#ifndef PLATFORM_PS
#include <RapidJsonGltf.h>
#include <IndependantTinyGltfStructures.h>
// #define TINYGLTF_IMPLEMENTATION
#include <tinygltf/new_new_tiny_gltf.h>

namespace rapidjson_tinygltf
{
	mock_tinygltf::Model LoadModelFromGltf(const std::string& file)
	{
		mock_tinygltf::Model gltfModel;

		return gltfModel;
	}

	mock_tinygltf::Model LoadModelFromGlb(const std::string& file)
	{
		mock_tinygltf::Model gltfModel;

		return gltfModel;
	}
}
#endif