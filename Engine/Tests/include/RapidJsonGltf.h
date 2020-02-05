#pragma once
#include <string>

// forward declarations
namespace mock_tinygltf
{
	struct Model;
}

namespace rapidjson_tinygltf
{
	/// Attempts to load a ascii encoded .gltf file.
	/// \throws std::runtime_error when it could not load the file
	mock_tinygltf::Model LoadModelFromGltf(const std::string& file);
	/// Attempts to load a binary encoded .gltb file.
	/// \throws std::runtime_error when it could not load the file
	mock_tinygltf::Model LoadModelFromGlb(const std::string& file);
}
