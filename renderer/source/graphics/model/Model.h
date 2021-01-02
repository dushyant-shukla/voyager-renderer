#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

#include <string>
#include <vector>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "Mesh.h"
#include "graphics/animation/Animation.h"

const std::string MODEL_PATH = "..\\..\\assets\\models\\";
const std::string TEXTURE_PATH = "..\\..\\assets\\textures\\";

namespace vrassimp
{
	struct Model
	{
		~Model();

		std::string mScreenName;

		struct
		{
			glm::vec3 position;
			glm::vec3 scale;
			glm::vec3 rotation;
		} mTransform;

		struct
		{
			glm::vec3 position;
			glm::vec3 scale;
			glm::vec3 rotation;
		} mAnimationTransform;

		struct
		{
			bool texturesAvailable = 1;
			glm::vec3 color = glm::vec3(1.0f, 0.0f, 0.0f);
		} modelSettings;

		const aiScene* mScene = nullptr;
		Animation* mAnimation = nullptr;
		bool isAnimationAvailable = false;

		// these things are actually drawn
		std::vector<Mesh*> meshes;
		std::vector<glm::vec4> jointPositions;
		std::vector<glm::vec4> linePositions;

		void LoadFromFile(std::string filename, std::string screename);
		void QueryAnimationData();
		void QueryMeshData(const unsigned int& meshIndex, const aiMesh* mesh);
		void QueryMeshMaterial(const unsigned int& meshIndex, const aiMesh* mesh);

	private:

		// utility constant
		static inline const int DEFAUTL_FLAGS = aiProcess_Triangulate | /*aiProcess_GenNormals | aiProcess_GenSmoothNormals | */aiProcess_CalcTangentSpace | aiProcess_ConvertToLeftHanded | aiProcess_GlobalScale;
		static inline const aiVector3D Zero3D = aiVector3D(0.0f, 0.0f, 0.0f);
	};
}
