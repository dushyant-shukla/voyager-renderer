#include "Model.h"
#include "assertions.h"

namespace vrassimp
{
	Model::~Model()
	{
		for (auto& mesh : meshes)
		{
			if (mesh != nullptr)
			{
				delete mesh;
			}
		}
	}

	void Model::LoadFromFile(std::string filename, std::string screename)
	{
		Assimp::Importer Importer;
		std::string fullpath(MODEL_PATH + filename);
		Importer.ReadFile(fullpath.c_str(), Model::DEFAUTL_FLAGS);

		/*
			Importer above will go out of scope when LoadFromFile() ends.
			To take control of the scene we call GetOrphanedScene()
		*/
		mScene = Importer.GetOrphanedScene();

		ASSERT_SUCCESS_AND_THROW(mScene, "FAILED TO LOAD MODEL: " + fullpath);

		mScreenName = screename;

		if (mScene)
		{
			if (mScene->HasAnimations())
			{
				QueryAnimationData();
			}

			this->meshes.resize(mScene->mNumMeshes);
			for (int meshIndex = 0; meshIndex < mScene->mNumMeshes; ++meshIndex)
			{
				QueryMeshData(meshIndex, mScene->mMeshes[meshIndex]);
				QueryMeshMaterial(meshIndex, mScene->mMeshes[meshIndex]);
			}
		}
	}

	void Model::QueryAnimationData()
	{
		isAnimationAvailable = true;
		mAnimation = new Animation();
		mAnimation->mScene = mScene;
		mAnimation->mGlobalInverseTransform = mScene->mRootNode->mTransformation;
		mAnimation->mGlobalInverseTransform.Inverse(); // animation runs without inverse as well

		// extract information about animation tracks available in the file
		for (size_t i = 0; i < mScene->mNumAnimations; ++i)
		{
			double start = (i == 0 ? 0 : mAnimation->availableTracks[i - 1].end);
			double end = (start + mScene->mAnimations[i]->mDuration / mScene->mAnimations[i]->mTicksPerSecond);
			mAnimation->availableTracks.emplace_back(mScene->mAnimations[i]->mDuration, std::string(mScene->mAnimations[i]->mName.data), mScene->mAnimations[i]->mTicksPerSecond, start, end);
			mAnimation->settings.tracks += mAnimation->availableTracks[i].name + '\0';
		}
		mAnimation->settings.tracks += '\0';

		if (mScene->mAnimations[0]->mTicksPerSecond != 0.0)
		{
			mAnimation->mTicksPerSecond = mScene->mAnimations[0]->mTicksPerSecond;
		}
		else
		{
			mAnimation->mTicksPerSecond = 25.0f;
		}

		/*
			Process animation data for all meshes here
		*/
		mAnimation->mBoneIdsAndWeights.resize(mScene->mNumMeshes);
		for (int meshIndex = 0; meshIndex < mScene->mNumMeshes; ++meshIndex)
		{
			aiMesh* mesh = mScene->mMeshes[meshIndex];
			if (mesh->mNumBones > 0)
			{
				mAnimation->ProcessMesh(meshIndex, mesh);
			}
		}
	}

	void Model::QueryMeshData(const unsigned int& meshIndex, const aiMesh* aiMesh)
	{
		Mesh* mesh = new Mesh();
		mesh->vertices.resize(aiMesh->mNumVertices);

		// vertex attributes
		for (size_t vertIndex = 0; vertIndex < aiMesh->mNumVertices; ++vertIndex)
		{
			mesh->vertices[vertIndex].position = { aiMesh->mVertices[vertIndex].x, aiMesh->mVertices[vertIndex].y , aiMesh->mVertices[vertIndex].z };

			mesh->vertices[vertIndex].normal = { aiMesh->mNormals[vertIndex].x, aiMesh->mNormals[vertIndex].y, aiMesh->mNormals[vertIndex].z };

			const aiVector3D* pTexCoord = (aiMesh->HasTextureCoords(0)) ? &(aiMesh->mTextureCoords[0][vertIndex]) : &Zero3D;
			mesh->vertices[vertIndex].uv = { pTexCoord->x, pTexCoord->y };

			const aiVector3D* pTangent = (aiMesh->HasTangentsAndBitangents()) ? &(aiMesh->mTangents[vertIndex]) : &Zero3D;
			mesh->vertices[vertIndex].tangent = { pTangent->x, pTangent->y, pTangent->z };

			/*
				Add bone-id(s) and weight(s) for each mesh vertex.
			*/
			if (mAnimation && !mAnimation->mBoneIdsAndWeights[meshIndex].empty())//isAnimationAvailable)
			{
				for (unsigned int boneIndex = 0; boneIndex < MAX_BONES_PER_VERTX; ++boneIndex)
				{
					mesh->vertices[vertIndex].boneWeights[boneIndex] = mAnimation->mBoneIdsAndWeights[meshIndex][vertIndex].weights[boneIndex];
				}

				for (unsigned int boneIndex = 0; boneIndex < MAX_BONES_PER_VERTX; ++boneIndex)
				{
					mesh->vertices[vertIndex].boneIds[boneIndex] = mAnimation->mBoneIdsAndWeights[meshIndex][vertIndex].ids[boneIndex];
				}
			}
			else
			{
				for (unsigned int boneIndex = 0; boneIndex < MAX_BONES_PER_VERTX; ++boneIndex)
				{
					mesh->vertices[vertIndex].boneWeights[boneIndex] = 0.0f;
				}

				for (unsigned int boneIndex = 0; boneIndex < MAX_BONES_PER_VERTX; ++boneIndex)
				{
					mesh->vertices[vertIndex].boneIds[boneIndex] = 0.0f;
				}
			}

			aiColor3D pColor(0.f, 0.f, 0.f);
			mScene->mMaterials[aiMesh->mMaterialIndex]->Get(AI_MATKEY_COLOR_DIFFUSE, pColor);
			mesh->vertices[vertIndex].color = { pColor.r, pColor.g, pColor.b };
		}

		// mesh indices
		for (size_t i = 0; i < aiMesh->mNumFaces; ++i)
		{
			aiFace face = aiMesh->mFaces[i];
			for (size_t j = 0; j < face.mNumIndices; ++j)
			{
				mesh->indices.push_back(face.mIndices[j]);
			}
		}

		this->meshes[meshIndex] = mesh;
	}

	void Model::QueryMeshMaterial(const unsigned int& meshIndex, const aiMesh* mesh)
	{
		if (mesh->mMaterialIndex >= 0)
		{
			aiMaterial* material = mScene->mMaterials[mesh->mMaterialIndex];

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

					Texture* texture = new Texture(Texture::Type::DIFFUSE, filename);
					meshes[meshIndex]->textures.push_back(texture);
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

					Texture* texture = new Texture(Texture::Type::EMISSIVE, filename);
					meshes[meshIndex]->textures.push_back(texture);
				}
			}

			if (material->GetTextureCount(aiTextureType_NORMALS))
			{
				if (material->GetTexture(aiTextureType_NORMALS, 0, &path) == AI_SUCCESS)
				{
					int index = std::string(path.data).rfind("\\");
					if (index < 0)
					{
						index = std::string(path.data).rfind("/");
					}
					std::string filename = std::string(path.data).substr(index + 1);

					Texture* texture = new Texture(Texture::Type::NORMALS, filename);
					meshes[meshIndex]->textures.push_back(texture);
				}
			}

			/*if (material->GetTextureCount(aiTextureType_SPECULAR))
			{
				if (material->GetTexture(aiTextureType_SPECULAR, 0, &path) == AI_SUCCESS)
				{
					int index = std::string(path.data).rfind("\\");
					if (index < 0)
					{
						index = std::string(path.data).rfind("/");
					}
					std::string filename = std::string(path.data).substr(index + 1);

					Texture* texture = new Texture(Texture::Type::SPECULAR, filename);
					meshes[meshIndex]->textures.push_back(texture);
				}
			}

			if (material->GetTextureCount(aiTextureType_OPACITY))
			{
				if (material->GetTexture(aiTextureType_OPACITY, 0, &path) == AI_SUCCESS)
				{
					int index = std::string(path.data).rfind("\\");
					if (index < 0)
					{
						index = std::string(path.data).rfind("/");
					}
					std::string filename = std::string(path.data).substr(index + 1);

					Texture* texture = new Texture(Texture::Type::OPACITY, filename);
					meshes[meshIndex]->textures.push_back(texture);
				}
			}*/
		}
	}
}