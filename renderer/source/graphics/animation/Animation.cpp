#include "Animation.h"
#include "RendererState.h"
#include "assertions.h"
#include "maths/Quaternion.h"

namespace vrassimp
{
	void VertexBoneData::AddBoneData(int id, float weight)
	{
		for (int i = 0; i < MAX_BONES_PER_VERTX; ++i)
		{
			if (weights[i] == 0.0f)
			{
				ids[i] = id;
				weights[i] = weight;
				return;
			}
		}
	}

	void Animation::ProcessMesh(int meshIndex, aiMesh* mesh)
	{
		mBoneIdsAndWeights[meshIndex].resize(mesh->mNumVertices);

		// load bones
		for (int i = 0; i < mesh->mNumBones; ++i)
		{
			int boneIndex = 0;
			std::string boneName(mesh->mBones[i]->mName.data);

			if (mBoneMapping.find(boneName) == mBoneMapping.end())
			{
				boneIndex = mBoneCount;
				++mBoneCount;
				BoneMatrix matrix;
				matrix.offset = mesh->mBones[i]->mOffsetMatrix;
				mBoneMatrices.push_back(matrix);

				mBoneMapping[boneName] = boneIndex;
			}
			else
			{
				boneIndex = mBoneMapping[boneName];
			}

			/*
				Each bone influences certain number of vertices by a corresponding weight.
				This is what is sent into shader along with bone transformations.
			*/
			for (int j = 0; j < mesh->mBones[i]->mNumWeights; ++j)
			{
				int vertexId = mesh->mBones[i]->mWeights[j].mVertexId;
				float weight = mesh->mBones[i]->mWeights[j].mWeight;
				mBoneIdsAndWeights[meshIndex][vertexId].AddBoneData(boneIndex, weight);
			}
		}
	}

	void Animation::Animate(double seconds, std::vector<aiMatrix4x4>& transforms, std::vector<aiMatrix4x4>& boneTransforms)
	{
		double timeInTicks = seconds * mTicksPerSecond;
		float animationTime = fmod(timeInTicks, (float)mScene->mAnimations[settings.currentTrackIndex]->mDuration);

		aiMatrix4x4 identityMatrix = aiMatrix4x4();
		ReadNodeHierarchy(animationTime, mScene->mRootNode, identityMatrix);

		transforms.resize(mBoneCount);
		boneTransforms.resize(mBoneCount);

		for (int i = 0; i < mBoneCount; ++i)
		{
			transforms[i] = mBoneMatrices[i].finalWorldTransform;
			boneTransforms[i] = mBoneMatrices[i].finalBoneTransform;
		}
	}

	void Animation::ReadNodeHierarchy(float parentAnimationTime, const aiNode* parentNode, const aiMatrix4x4 parentTransform)
	{
		std::string nodeName(parentNode->mName.data);

		const aiAnimation* animation = mScene->mAnimations[settings.currentTrackIndex];
		aiMatrix4x4 nodeTransform(parentNode->mTransformation);

		const aiNodeAnim* nodeAnimation = FindNodeAnim(animation, nodeName);
		if (nodeAnimation)
		{
			aiMatrix4x4 matScale = InterpolateScale(parentAnimationTime, nodeAnimation);
			aiMatrix4x4 matRotation = InterpolateRotation(parentAnimationTime, nodeAnimation);
			aiMatrix4x4 matTranslation = InterpolateTranslation(parentAnimationTime, nodeAnimation);
			nodeTransform = matTranslation * matRotation * matScale; // works without scale as well
		}

		aiMatrix4x4 globalTransformation = parentTransform * nodeTransform;
		if (mBoneMapping.find(nodeName) != mBoneMapping.end())
		{
			unsigned int boneIndex = mBoneMapping[nodeName];
			mBoneMatrices[boneIndex].finalBoneTransform = (mGlobalInverseTransform * globalTransformation);
			mBoneMatrices[boneIndex].finalWorldTransform = (mGlobalInverseTransform * globalTransformation * mBoneMatrices[boneIndex].offset);
			boneEndpointPositions.push_back(BoneLine(mGlobalInverseTransform * parentTransform, mBoneMatrices[boneIndex].finalBoneTransform));
			//boneEndpointPositions.push_back(BoneLine(parentTransform, globalTransformation)); // works like this as well
		}

		for (int i = 0; i < parentNode->mNumChildren; ++i)
		{
			ReadNodeHierarchy(parentAnimationTime, parentNode->mChildren[i], globalTransformation);
		}
	}

	const aiNodeAnim* Animation::FindNodeAnim(const aiAnimation* parentAnimation, const std::string parentNodeName)
	{
		for (int i = 0; i < parentAnimation->mNumChannels; ++i)
		{
			const aiNodeAnim* nodeAnim = parentAnimation->mChannels[i];
			if (std::string(nodeAnim->mNodeName.data) == parentNodeName)
			{
				return nodeAnim;
			}
		}

		return nullptr;
	}

	// Returns a 4x4 matrix with interpolated scaling between current and next frame
	aiMatrix4x4 Animation::InterpolateScale(float time, const aiNodeAnim* pNodeAnim)
	{
		aiVector3D scale;

		if (pNodeAnim->mNumScalingKeys == 1)
		{
			scale = pNodeAnim->mScalingKeys[0].mValue;
		}
		else
		{
			uint32_t frameIndex = 0;
			for (uint32_t i = 0; i < pNodeAnim->mNumScalingKeys - 1; i++)
			{
				if (time < (float)pNodeAnim->mScalingKeys[i + 1].mTime)
				{
					frameIndex = i;
					break;
				}
			}

			aiVectorKey currentFrame = pNodeAnim->mScalingKeys[frameIndex];
			aiVectorKey nextFrame = pNodeAnim->mScalingKeys[(frameIndex + 1) % pNodeAnim->mNumScalingKeys];

			float delta = (time - (float)currentFrame.mTime) / (float)(nextFrame.mTime - currentFrame.mTime);

			const aiVector3D& start = currentFrame.mValue;
			const aiVector3D& end = nextFrame.mValue;

			scale = (start + delta * (end - start));
		}

		aiMatrix4x4 mat;
		aiMatrix4x4::Scaling(scale, mat);
		return mat;
	}

	int Animation::FindScaling(float parentAnimationTime, const aiNodeAnim* parentNodeAnimation)
	{
		for (int i = 0; i < parentNodeAnimation->mNumScalingKeys - 1; i++)
		{
			if (parentAnimationTime < (float)parentNodeAnimation->mScalingKeys[i + 1].mTime)
			{
				return i;
			}
		}

		assert(0);
		return 0;
	}

	// Returns a 4x4 matrix with interpolated rotation between current and next frame
	aiMatrix4x4 Animation::InterpolateRotation(float time, const aiNodeAnim* pNodeAnim)
	{
		aiQuaternion rotation;

		if (pNodeAnim->mNumRotationKeys == 1)
		{
			rotation = pNodeAnim->mRotationKeys[0].mValue;

			// Quaternion
			vrmath::Quaternion result(glm::vec4(rotation.x, rotation.y, rotation.z, rotation.w));
			return result.ToAiRotationMatrix();
		}
		else
		{
			uint32_t frameIndex = 0;
			for (uint32_t i = 0; i < pNodeAnim->mNumRotationKeys - 1; i++)
			{
				if (time < (float)pNodeAnim->mRotationKeys[i + 1].mTime)
				{
					frameIndex = i;
					break;
				}
			}

			aiQuatKey currentFrame = pNodeAnim->mRotationKeys[frameIndex];
			aiQuatKey nextFrame = pNodeAnim->mRotationKeys[(frameIndex + 1) % pNodeAnim->mNumRotationKeys];

			float delta = (time - (float)currentFrame.mTime) / (float)(nextFrame.mTime - currentFrame.mTime);

			const aiQuaternion& start = currentFrame.mValue;
			const aiQuaternion& end = nextFrame.mValue;

			//aiQuaternion::Interpolate(rotation, start, end, delta);
			//rotation.Normalize();

			// Quaternion
			vrmath::Quaternion A(glm::vec4(start.x, start.y, start.z, start.w));
			vrmath::Quaternion B(glm::vec4(end.x, end.y, end.z, end.w));
			vrmath::Quaternion result = vrmath::Quaternion::Slerp(A, B, delta);
			return result.ToAiRotationMatrix();
		}

		//aiMatrix4x4 mat(rotation.GetMatrix());
		//return mat;
	}

	int Animation::FindRotation(float parentAnimationTime, const aiNodeAnim* parentNodeAnimation)
	{
		for (int i = 0; i < parentNodeAnimation->mNumRotationKeys - 1; i++)
		{
			if (parentAnimationTime < (float)parentNodeAnimation->mRotationKeys[i + 1].mTime)
			{
				return i;
			}
		}

		assert(0);
		return 0;
	}

	// Returns a 4x4 matrix with interpolated translation between current and next frame
	aiMatrix4x4 Animation::InterpolateTranslation(float time, const aiNodeAnim* pNodeAnim)
	{
		aiVector3D translation;

		if (pNodeAnim->mNumPositionKeys == 1)
		{
			translation = pNodeAnim->mPositionKeys[0].mValue;
		}
		else
		{
			uint32_t frameIndex = 0;
			for (uint32_t i = 0; i < pNodeAnim->mNumPositionKeys - 1; i++)
			{
				if (time < (float)pNodeAnim->mPositionKeys[i + 1].mTime)
				{
					frameIndex = i;
					break;
				}
			}

			aiVectorKey currentFrame = pNodeAnim->mPositionKeys[frameIndex];
			aiVectorKey nextFrame = pNodeAnim->mPositionKeys[(frameIndex + 1) % pNodeAnim->mNumPositionKeys];

			float delta = (time - (float)currentFrame.mTime) / (float)(nextFrame.mTime - currentFrame.mTime);

			const aiVector3D& start = currentFrame.mValue;
			const aiVector3D& end = nextFrame.mValue;

			translation = (start + delta * (end - start));
		}

		aiMatrix4x4 mat;
		aiMatrix4x4::Translation(translation, mat);
		return mat;
	}

	int Animation::FindPosition(float parentAnimationTime, const aiNodeAnim* parentNodeAnimation)
	{
		for (int i = 0; i < parentNodeAnimation->mNumPositionKeys - 1; i++)
		{
			if (parentAnimationTime < (float)parentNodeAnimation->mPositionKeys[i + 1].mTime)
			{
				return i;
			}
		}

		assert(0);
		return 0;
	}

	glm::mat4 Animation::aiToGlm(aiMatrix4x4 ai_matr)
	{
		glm::mat4 result;
		result[0].x = ai_matr.a1; result[0].y = ai_matr.b1; result[0].z = ai_matr.c1; result[0].w = ai_matr.d1;
		result[1].x = ai_matr.a2; result[1].y = ai_matr.b2; result[1].z = ai_matr.c2; result[1].w = ai_matr.d2;
		result[2].x = ai_matr.a3; result[2].y = ai_matr.b3; result[2].z = ai_matr.c3; result[2].w = ai_matr.d3;
		result[3].x = ai_matr.a4; result[3].y = ai_matr.b4; result[3].z = ai_matr.c4; result[3].w = ai_matr.d4;

		return result;
	}

	void Animation::InitializeIKData()
	{
		aiMatrix4x4 identityMatrix = aiMatrix4x4();
		ReadIKBoneHierarchy(0.0f, mScene->mRootNode, identityMatrix);
	}

	void Animation::ReadIKBoneHierarchy(float animationTime, const aiNode* parentNode, const aiMatrix4x4 parentTransform)
	{
		std::string boneName(parentNode->mName.data);

		const aiAnimation* animation = mScene->mAnimations[settings.currentTrackIndex];
		aiMatrix4x4 nodeTransform(parentNode->mTransformation);

		const aiNodeAnim* nodeAnimation = FindNodeAnim(animation, boneName);
		if (nodeAnimation)
		{
			aiMatrix4x4 matScale = InterpolateScale(animationTime, nodeAnimation);
			aiMatrix4x4 matRotation = InterpolateRotation(animationTime, nodeAnimation);
			aiMatrix4x4 matTranslation = InterpolateTranslation(animationTime, nodeAnimation);
			nodeTransform = matTranslation * matRotation * matScale; // works without scale as well
		}

		aiMatrix4x4 globalTransformation = parentTransform * nodeTransform;
		if (mBoneMapping.find(boneName) != mBoneMapping.end())
		{
			unsigned int boneIndex = mBoneMapping[boneName];
			glm::mat4 localTransform = aiToGlm(nodeTransform);
			glm::mat4 globalTransform = aiToGlm(globalTransformation);
			glm::mat4 worldTransform = aiToGlm(mGlobalInverseTransform * globalTransformation * mBoneMatrices[boneIndex].offset);
			mCCDSolver.UpdateIkMatrices(boneName, boneIndex, localTransform, globalTransform, worldTransform);
		}

		for (int i = 0; i < parentNode->mNumChildren; ++i)
		{
			ReadIKBoneHierarchy(animationTime, parentNode->mChildren[i], globalTransformation);
		}
	}
}