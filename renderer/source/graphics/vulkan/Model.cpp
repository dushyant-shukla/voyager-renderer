#include "Model.h"
#include "RendererState.h"
#include "assertions.h"

namespace vrassimp
{
	Texture::Texture() {}

	Texture::Texture(Type _type, std::string _path)
		: type(_type), path(_path)
	{}

	Mesh::Mesh() {}

	Mesh::Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<Texture> textures)
		: vertices(vertices), indices(indices), textures(textures)
	{
	}

	void Mesh::Draw()
	{
	}

	VertexLayout::VertexLayout(std::vector<VertexLayoutComponentType> components) : layoutComponents(std::move(components)) {}

	unsigned int VertexLayout::Stride()
	{
		unsigned int stride = 0;
		for (auto& component : layoutComponents)
		{
			switch (component)
			{
			case VertexLayoutComponentType::VERTEX_COMPONENT_UV:
				stride += 2 * sizeof(float);
				break;
			case VertexLayoutComponentType::VERTEX_COMPONENT_DUMMY_FLOAT:
				stride += sizeof(float);
				break;
			case VertexLayoutComponentType::VERTEX_COMPONENT_DUMMY_VEC4:
				stride += 4 * sizeof(float);
				break;
			case VertexLayoutComponentType::VERTEX_COMPONENT_BONE_WEIGHT:
				stride += 4 * sizeof(float);
				break;
			case VertexLayoutComponentType::VERTEX_COMPONENT_BONE_IDS:
				stride += 4 * sizeof(float);
				break;
			default:
				// All components except the ones listed above are made up of 3 floats
				stride += 3 * sizeof(float);
			}
		}

		return stride;
	}

	void Model::LoadFromFile(std::string filename, ModelCreateInfo* createInfo)
	{
		Assimp::Importer Importer;

		// Load file
		//char buffer[MAX_PATH];
		//GetModuleFileName(NULL, buffer, MAX_PATH);
		//std::string::size_type pos = std::string(buffer).find_last_of("\\/");

		//std::string fileName = std::string(buffer).substr(0, pos);
		//std::size_t found = fileName.find("PC Visual Studio");
		//if (found != std::string::npos)
		//{
		//	fileName.erase(fileName.begin() + found, fileName.end());
		//}

		//fileName = fileName + "Resources\\Models\\" + filename;
		std::string fullpath(MODEL_PATH + filename);
		const aiScene* scene = Importer.ReadFile(fullpath.c_str(), Model::DEFAUTL_FLAGS);
		ASSERT_SUCCESS_AND_THROW(scene, "FAILED TO LOAD MODEL: " + fullpath);

		if (scene)
		{
			glm::vec3 scale(1.0f);
			glm::vec3 center(0.0f);
			glm::vec2 uvScale(1.0f);
			if (createInfo)
			{
				scale = createInfo->scale;
				center = createInfo->center;
				uvScale = createInfo->uvScale;
			}

			if (scene->HasAnimations())
			{
				QueryAnimationData(Importer, scene);
			}

			this->meshes.resize(scene->mNumMeshes);
			for (int meshIndex = 0; meshIndex < scene->mNumMeshes; ++meshIndex)
			{
				QueryMeshData(meshIndex, scene->mMeshes[meshIndex], scene);
				QueryMeshMaterial(meshIndex, scene->mMeshes[meshIndex], scene);
			}
		}
	}

	/*
		TODO: may be the scene is not needed as the second para here
	*/
	void Model::QueryAnimationData(Assimp::Importer& Importer, const aiScene* scene)
	{
		isAnimationAvailable = true;
		mAnimation = new Animation();
		mAnimation->mScene = Importer.GetOrphanedScene();
		mAnimation->SetAnimation(0);
		mAnimation->mGlobalInverseTransform = scene->mRootNode->mTransformation;
		if (scene->mAnimations[0]->mTicksPerSecond != 0.0)
		{
			mAnimation->mTicksPerSecond = scene->mAnimations[0]->mTicksPerSecond;
		}
		else
		{
			mAnimation->mTicksPerSecond = 25.0f;
		}

		mAnimation->ProcessNode(scene->mRootNode, scene);
	}

	void Model::QueryMeshData(const unsigned int& meshIndex, const aiMesh* mesh, const aiScene* scene)
	{
		meshes[meshIndex].vertices.resize(mesh->mNumVertices);

		// vertex attributes
		for (size_t vertIndex = 0; vertIndex < mesh->mNumVertices; ++vertIndex)
		{
			meshes[meshIndex].vertices[vertIndex].position = { mesh->mVertices[vertIndex].x, mesh->mVertices[vertIndex].y , mesh->mVertices[vertIndex].z };

			meshes[meshIndex].vertices[vertIndex].normal = { mesh->mNormals[vertIndex].x, mesh->mNormals[vertIndex].y, mesh->mNormals[vertIndex].z };

			const aiVector3D* pTexCoord = (mesh->HasTextureCoords(0)) ? &(mesh->mTextureCoords[0][vertIndex]) : &Zero3D;
			meshes[meshIndex].vertices[vertIndex].uv = { pTexCoord->x, pTexCoord->y };

			const aiVector3D* pTangent = (mesh->HasTangentsAndBitangents()) ? &(mesh->mTangents[vertIndex]) : &Zero3D;
			meshes[meshIndex].vertices[vertIndex].tangent = { pTangent->x, pTangent->y, pTangent->z };

			if (isAnimationAvailable)
			{
				for (unsigned int boneIndex = 0; boneIndex < MAX_BONES_PER_VERTX; ++boneIndex)
				{
					meshes[meshIndex].vertices[vertIndex].boneWeights[boneIndex] = mAnimation->mBones[meshIndex][vertIndex].weights[boneIndex];
				}

				for (unsigned int boneIndex = 0; boneIndex < MAX_BONES_PER_VERTX; ++boneIndex)
				{
					meshes[meshIndex].vertices[vertIndex].boneIds[boneIndex] = mAnimation->mBones[meshIndex][vertIndex].ids[boneIndex];
				}
			}
			else
			{
				for (unsigned int boneIndex = 0; boneIndex < MAX_BONES_PER_VERTX; ++boneIndex)
				{
					meshes[meshIndex].vertices[vertIndex].boneWeights[boneIndex] = 0.0f;
				}

				for (unsigned int boneIndex = 0; boneIndex < MAX_BONES_PER_VERTX; ++boneIndex)
				{
					meshes[meshIndex].vertices[vertIndex].boneIds[boneIndex] = 0.0f;
				}
			}

			aiColor3D pColor(0.f, 0.f, 0.f);
			scene->mMaterials[mesh->mMaterialIndex]->Get(AI_MATKEY_COLOR_DIFFUSE, pColor);
			meshes[meshIndex].vertices[vertIndex].color = { pColor.r, pColor.g, pColor.b };
		}

		// mesh indices
		for (size_t i = 0; i < mesh->mNumFaces; ++i)
		{
			aiFace face = mesh->mFaces[i];
			for (size_t j = 0; j < face.mNumIndices; ++j)
			{
				meshes[meshIndex].indices.push_back(face.mIndices[j]);
			}
		}
	}

	void Model::QueryMeshMaterial(const unsigned int& meshIndex, const aiMesh* mesh, const aiScene* scene)
	{
		if (mesh->mMaterialIndex >= 0)
		{
			aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

			aiString path;
			if (material->GetTextureCount(aiTextureType_DIFFUSE))
			{
				if (material->GetTexture(aiTextureType_DIFFUSE, 0, &path) == AI_SUCCESS)
				{
					int index = std::string(path.data).rfind("\\");
					if (index < 0)
					{
						index = std::string(path.data).rfind("/");
					}
					std::string filename = std::string(path.data).substr(index + 1);

					Texture texture(Texture::Type::DIFFUSE, path.data);
					meshes[meshIndex].textures.push_back(texture);
				}
			}

			if (material->GetTextureCount(aiTextureType_EMISSIVE))
			{
				if (material->GetTexture(aiTextureType_EMISSIVE, 0, &path) == AI_SUCCESS)
				{
					int index = std::string(path.data).rfind("\\");
					if (index < 0)
					{
						index = std::string(path.data).rfind("/");
					}
					std::string filename = std::string(path.data).substr(index + 1);

					Texture texture(Texture::Type::EMISSIVE, path.data);
					meshes[meshIndex].textures.push_back(texture);
				}
			}
		}
	}

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

	Model::ModelCreateInfo::ModelCreateInfo()
		: center(glm::vec3(0.0f)),
		scale(glm::vec3(1.0f)),
		uvScale(glm::vec2(1.0f))
	{}

	Model::ModelCreateInfo::ModelCreateInfo(glm::vec3 _center, glm::vec3 _scale, glm::vec2 _uvScale)
		: center(_center),
		scale(_scale),
		uvScale(_uvScale)
	{}

	Model::ModelCreateInfo::ModelCreateInfo(float _center, float _scale, float _uvScale)
		: center(glm::vec3(_center)),
		scale(glm::vec3(_scale)),
		uvScale(glm::vec2(_uvScale))
	{}

	void Animation::SetAnimation(unsigned int animationIndex)
	{
		if (animationIndex < mScene->mNumAnimations)
		{
			mActiveAnimation = mScene->mAnimations[animationIndex];
			return;
		}
		THROW("ANIMATION FAILURE: INVALID INDEX PASSED");
	}

	void Animation::ProcessNode(aiNode* node, const aiScene* scene)
	{
		mBones.resize(scene->mNumMeshes);
		for (int i = 0; i < scene->mNumMeshes; ++i)
		{
			aiMesh* mesh = scene->mMeshes[i];
			ProcessMesh(i, mesh, scene);
		}
	}

	void Animation::ProcessMesh(int meshIndex, aiMesh* mesh, const aiScene* scene)
	{
		mBones[meshIndex].resize(mesh->mNumVertices);

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
				Each bone influences certain number of vertices
			*/
			for (int j = 0; j < mesh->mBones[i]->mNumWeights; ++j)
			{
				int vertexId = mesh->mBones[i]->mWeights[j].mVertexId;
				float weight = mesh->mBones[i]->mWeights[j].mWeight;
				mBones[meshIndex][vertexId].AddBoneData(boneIndex, weight);
			}
		}
	}

	void Animation::BoneTransform(double seconds, std::vector<aiMatrix4x4>& transforms)
	{
		aiMatrix4x4 identityMatrix;

		double timeInTicks = seconds * mTicksPerSecond;
		float animationTime = fmod(timeInTicks, mScene->mAnimations[0]->mDuration);

		tempCount.clear();
		tempCount.resize(53);
	}

	void Animation::ReadNodeHierarchy(float parentAnimationTime, const aiNode* parentNode, const aiMatrix4x4 parentTransform)
	{
		std::string nodeName(parentNode->mName.data);

		const aiAnimation* animation = mScene->mAnimations[0];
		aiMatrix4x4 nodeTransform(parentNode->mTransformation);

		const aiNodeAnim* nodeAnimation = FindNodeAnim(animation, nodeName);

		bool found = false;
		int foundCount = -1;

		if (nodeAnimation)
		{
			aiMatrix4x4 matScale = InterpolateScale(parentAnimationTime, nodeAnimation);
			aiMatrix4x4 matRotation = InterpolateRotation(parentAnimationTime, nodeAnimation);
			aiMatrix4x4 matTranslation = InterpolateTranslation(parentAnimationTime, nodeAnimation);

			nodeTransform = matTranslation * matRotation * matScale;
		}

		aiMatrix4x4 globalTransformation = parentTransform * nodeTransform;
		if (mBoneMapping.find(nodeName) != mBoneMapping.end())
		{
			unsigned int boneIndex = mBoneMapping[nodeName];
			mBoneMatrices[boneIndex].finalBoneTransform = mGlobalInverseTransform * globalTransformation;
			mBoneMatrices[boneIndex].finalWorldTransform = mGlobalInverseTransform * globalTransformation * mBoneMatrices[boneIndex].offset;

			for (int i = 0; i < parentNode->mNumChildren; i++)
			{
				std::string nodeName(parentNode->mChildren[i]->mName.data);
				if (mBoneMapping.find(nodeName) != mBoneMapping.end())
				{
					int boneIndex = mBoneMapping[nodeName];
					tempCount[boneIndex].push_back(boneIndex);
				}
				else
				{
					tempCount[boneIndex].push_back(boneIndex);
				}
			}
		}

		for (int i = 0; i < parentNode->mNumChildren; ++i)
		{
			ReadNodeHierarchy(parentAnimationTime, parentNode->mChildren[i], globalTransformation);
		}
	}

	const aiNodeAnim* Animation::FindNodeAnim(const aiAnimation* parentAnimation, const std::string parentNodeName)
	{
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

			aiQuaternion::Interpolate(rotation, start, end, delta);
			rotation.Normalize();
		}

		aiMatrix4x4 mat(rotation.GetMatrix());
		return mat;
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

	Quaternions Animation::nlerp(Quaternions a, Quaternions b, float blend)
	{
		a.Normalize();
		b.Normalize();

		Quaternions result;
		float dot_product = a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
		float one_minus_blend = 1.0f - blend;

		if (dot_product < 0.0f)
		{
			result.x = a.x * one_minus_blend + blend * -b.x;
			result.y = a.y * one_minus_blend + blend * -b.y;
			result.z = a.z * one_minus_blend + blend * -b.z;
			result.w = a.w * one_minus_blend + blend * -b.w;
		}
		else
		{
			result.x = a.x * one_minus_blend + blend * b.x;
			result.y = a.y * one_minus_blend + blend * b.y;
			result.z = a.z * one_minus_blend + blend * b.z;
			result.w = a.w * one_minus_blend + blend * b.w;
		}

		return result.Normalized();
	}
}