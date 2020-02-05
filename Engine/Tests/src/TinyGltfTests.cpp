#ifndef PLATFORM_PS
#include <catch/catch.hpp>
#include "core/Defines.h"

// Includes so that this code gets instantiated in the global namespace
#include <array>
#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <map>
#include <string>
#include <vector>

#include "stb_image.h"

#include <algorithm>
#include <fstream>
#include <sstream>

#include <functional>

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>  // include API for expanding a file path


#pragma region RapidJson TinyGltf Implementation
#include <rapidjson/rapidjson.h>
#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>
#include <rapidjson/encodedstream.h>
#include <rapidjson/encodings.h>


namespace rapidjson_tinygltf {
	//the implementation is slightly different, that is what we want to test. So we want this implementation too!
#undef TINY_GLTF_H_
#define TINYGLTF_NO_STB_IMAGE_WRITE
#define TINYGLTF_IMPLEMENTATION
#include <tinygltf/new_new_tiny_gltf.h>
}

namespace rapidjson_tinygltf
{
	static tinygltf::Model LoadModelFromGltf(const std::string& file)
	{
		tinygltf::Model model;
		tinygltf::TinyGLTF loader;
		std::string err;
		std::string warn;

		bool ret = loader.LoadASCIIFromFile(&model, &err, &warn, file, tinygltf::SectionCheck::REQUIRE_ALL);
		if (!ret)
			throw std::runtime_error(std::string("rapidjson_tinygltf::LoadModelFromGtf failed to load file: '")
				+ file + "' \nWith errors: '" + err + "'\nWith warnings: '" + warn + "'");

		return model;
	}

	static tinygltf::Model LoadModelFromGlb(const std::string& file)
	{
		tinygltf::Model model;
		tinygltf::TinyGLTF loader;
		std::string err;
		std::string warn;

		bool ret = loader.LoadBinaryFromFile(&model, &err, &warn, file, tinygltf::SectionCheck::REQUIRE_ALL);
		if (!ret)
			throw std::runtime_error(std::string("rapidjson_tinygltf::LoadModelFromGlb failed to load file: '")
				+ file + "' \nWith errors: '" + err + "'\nWith warnings: '" + warn + "'");

		return model;
	}
}
#pragma endregion

#pragma region Original(nlohmann) TinyGltf Implementation
#include <json.hpp>

namespace original_tinygltf {
	//the implementation is slightly different, that is what we want to test. So we want this implementation too!
#undef TINY_GLTF_H_
#define TINYGLTF_NO_STB_IMAGE_WRITE
#define TINYGLTF_IMPLEMENTATION
#include <tinygltf/tiny_gltf.h>
}


namespace original_tinygltf {
	static tinygltf::Model LoadModelFromGltf(const std::string& file)
	{
		tinygltf::Model model;
		tinygltf::TinyGLTF loader;
		std::string err;
		std::string warn;

		bool ret = loader.LoadASCIIFromFile(&model, &err, &warn, file, tinygltf::SectionCheck::REQUIRE_ALL);
		if (!ret)
			throw std::runtime_error(std::string("original_tinygltf::LoadModelFromGtf failed to load file: '")
				+ file + "' \nWith errors: '" + err + "'\nWith warnings: '" + warn + "'");


		return model;
	}

	static tinygltf::Model LoadModelFromGlb(const std::string& file)
	{
		tinygltf::Model model;
		tinygltf::TinyGLTF loader;
		std::string err;
		std::string warn;

		bool ret = loader.LoadBinaryFromFile(&model, &err, &warn, file, tinygltf::SectionCheck::REQUIRE_ALL);
		if (!ret)
			throw std::runtime_error(std::string("original_tinygltf::LoadModelFromGlb failed to load file: '")
				+ file + "' \nWith errors: '" + err + "'\nWith warnings: '" + warn + "'");

		return model;
	}
}
#pragma endregion


template<typename T, typename E, typename = typename std::enable_if<!std::is_same_v<T, E>, bool>::type>
static bool operator==(const std::vector<T>& original, const std::vector<E>& other)
{
	if (original.size() != other.size())
		return false;

	EmptyFunction();
	for (int i = 0; i < original.size(); ++i) {
		EmptyFunction();
		if (original[i] == other[i]) {
			EmptyFunction();
		} else {
			return false;
		}
	}
	return true;
}

template<typename T, typename E, typename = std::enable_if_t<!std::is_same_v<T, E>>>
static bool operator==(const std::map<std::string, T>& original,
	const std::map<std::string, E>& other)
{
	auto pred = [](auto a, auto b)
	{ return a.first == b.first && a.second == b.second; };

	return original.size() == other.size()
		&& std::equal(original.begin(), original.end(),
			other.begin(), pred);
}

// Equals function for Value, for recursivity
static bool Equals(const original_tinygltf::tinygltf::Value &one, const rapidjson_tinygltf::tinygltf::Value &other) {
	if (one.Type() != other.Type()) return false;

	switch (one.Type()) {
	case rapidjson_tinygltf::tinygltf::NULL_TYPE:
		return true;
	case rapidjson_tinygltf::tinygltf::BOOL_TYPE:
		return one.Get<bool>() == other.Get<bool>();
	case rapidjson_tinygltf::tinygltf::NUMBER_TYPE:
		return TINYGLTF_DOUBLE_EQUAL(one.Get<double>(), other.Get<double>());
	case rapidjson_tinygltf::tinygltf::INT_TYPE:
		return one.Get<int>() == other.Get<int>();
	case rapidjson_tinygltf::tinygltf::OBJECT_TYPE: {
		auto oneObj = one.Get<original_tinygltf::tinygltf::Value::Object>();
		auto otherObj = other.Get<rapidjson_tinygltf::tinygltf::Value::Object>();
		if (oneObj.size() != otherObj.size()) return false;
		for (auto &it : oneObj) {
			auto otherIt = otherObj.find(it.first);
			if (otherIt == otherObj.end()) return false;

			if (!Equals(it.second, otherIt->second)) return false;
		}
		return true;
	}
	case rapidjson_tinygltf::tinygltf::ARRAY_TYPE: {
		if (one.Size() != other.Size()) return false;
		for (int i = 0; i < int(one.Size()); ++i)
			if (Equals(one.Get(i), other.Get(i))) return false;
		return true;
	}
	case rapidjson_tinygltf::tinygltf::STRING_TYPE:
		return one.Get<std::string>() == other.Get<std::string>();
	case rapidjson_tinygltf::tinygltf::BINARY_TYPE:
		return one.Get<std::vector<unsigned char> >() ==
			other.Get<std::vector<unsigned char> >();
	default: {
		// unhandled type
		return false;
	}
	}
}

static bool operator==(const original_tinygltf::tinygltf::Value &original, const rapidjson_tinygltf::tinygltf::Value &other) {
	return Equals(original, other);
}

// Equals function for std::vector<double> using TINYGLTF_DOUBLE_EPSILON
static bool Equals(const std::vector<double> &one,
	const std::vector<double> &other) {
	if (one.size() != other.size()) return false;
	for (int i = 0; i < int(one.size()); ++i) {
		if (!TINYGLTF_DOUBLE_EQUAL(one[size_t(i)], other[size_t(i)])) return false;
	}
	return true;
}

static bool operator==(const original_tinygltf::tinygltf::Accessor &original, const rapidjson_tinygltf::tinygltf::Accessor &other) {
	bool b = original.bufferView == other.bufferView &&
		original.byteOffset == other.byteOffset &&
		original.componentType == other.componentType &&
		original.count == other.count && original.extras == other.extras &&
		Equals(original.maxValues, other.maxValues) &&
		Equals(original.minValues, other.minValues) && original.name == other.name &&
		original.normalized == other.normalized && original.type == other.type;
	return b;
}
static bool operator==(const original_tinygltf::tinygltf::AnimationChannel &original, const rapidjson_tinygltf::tinygltf::AnimationChannel &other) {
	return original.extras == other.extras &&
		original.target_node == other.target_node &&
		original.target_path == other.target_path &&
		original.sampler == other.sampler;
}
static bool operator==(const original_tinygltf::tinygltf::OrthographicCamera &original, const rapidjson_tinygltf::tinygltf::OrthographicCamera &other) {
	return original.extensions == other.extensions && original.extras == other.extras &&
		TINYGLTF_DOUBLE_EQUAL(original.xmag, other.xmag) &&
		TINYGLTF_DOUBLE_EQUAL(original.ymag, other.ymag) &&
		TINYGLTF_DOUBLE_EQUAL(original.zfar, other.zfar) &&
		TINYGLTF_DOUBLE_EQUAL(original.znear, other.znear);
}

static bool operator==(const original_tinygltf::tinygltf::Animation &original, const rapidjson_tinygltf::tinygltf::Animation &other) {
	return original.channels == other.channels && original.extras == other.extras &&
		original.name == other.name && original.samplers == other.samplers;
}

static bool operator==(const original_tinygltf::tinygltf::AnimationSampler &original, const rapidjson_tinygltf::tinygltf::AnimationSampler &other) {
	return original.extras == other.extras && original.input == other.input &&
		original.interpolation == other.interpolation &&
		original.output == other.output;
}

static bool operator==(const original_tinygltf::tinygltf::Asset &original, const rapidjson_tinygltf::tinygltf::Asset &other) {
	return original.copyright == other.copyright &&
		original.extensions == other.extensions && original.extras == other.extras &&
		original.generator == other.generator &&
		original.minVersion == other.minVersion && original.version == other.version;
}
static bool operator==(const original_tinygltf::tinygltf::Buffer &original, const rapidjson_tinygltf::tinygltf::Buffer &other) {
	return original.data == other.data && original.extras == other.extras &&
		original.name == other.name && original.uri == other.uri;
}
static bool operator==(const original_tinygltf::tinygltf::BufferView & original, const rapidjson_tinygltf::tinygltf::BufferView &other) {
	return original.buffer == other.buffer && original.byteLength == other.byteLength &&
		original.byteOffset == other.byteOffset &&
		original.byteStride == other.byteStride && original.name == other.name &&
		original.target == other.target && original.extras == other.extras &&
		original.dracoDecoded == other.dracoDecoded;
}
static bool operator==(const original_tinygltf::tinygltf::PerspectiveCamera &original, const rapidjson_tinygltf::tinygltf::PerspectiveCamera &other) {
	return TINYGLTF_DOUBLE_EQUAL(original.aspectRatio, other.aspectRatio) &&
		original.extensions == other.extensions && original.extras == other.extras &&
		TINYGLTF_DOUBLE_EQUAL(original.yfov, other.yfov) &&
		TINYGLTF_DOUBLE_EQUAL(original.zfar, other.zfar) &&
		TINYGLTF_DOUBLE_EQUAL(original.znear, other.znear);
}
static bool operator==(const original_tinygltf::tinygltf::Camera &original, const rapidjson_tinygltf::tinygltf::Camera &other) {
	return original.name == other.name && original.extensions == other.extensions &&
		original.extras == other.extras &&
		original.orthographic == other.orthographic &&
		original.perspective == other.perspective && original.type == other.type;
}
static bool operator==(const original_tinygltf::tinygltf::Image &original, const rapidjson_tinygltf::tinygltf::Image &other) {
	return original.bufferView == other.bufferView &&
		original.component == other.component && original.extras == other.extras &&
		original.height == other.height && original.image == other.image &&
		original.mimeType == other.mimeType && original.name == other.name &&
		original.uri == other.uri && original.width == other.width;
}
static bool operator==(const original_tinygltf::tinygltf::Light &original, const rapidjson_tinygltf::tinygltf::Light &other) {
	return Equals(original.color, other.color) && original.name == other.name &&
		original.type == other.type;
}
static bool operator==(const original_tinygltf::tinygltf::Material &original, const rapidjson_tinygltf::tinygltf::Material &other) {
	return original.additionalValues == other.additionalValues &&
		original.extensions == other.extensions && original.extras == other.extras &&
		original.name == other.name && original.values == other.values;
}
static bool operator==(const original_tinygltf::tinygltf::Mesh &original, const rapidjson_tinygltf::tinygltf::Mesh &other) {
	return original.extensions == other.extensions && original.extras == other.extras &&
		original.name == other.name && original.primitives == other.primitives &&
		original.targets == other.targets && Equals(original.weights, other.weights);
}
static bool operator==(const original_tinygltf::tinygltf::Model &original, const rapidjson_tinygltf::tinygltf::Model &other) {
	bool accessorsSame = original.accessors == other.accessors;
	bool animSame = original.animations == other.animations;
	bool assetSame = original.asset == other.asset;
	bool bufferSame = original.buffers == other.buffers;
	bool bufferViewSame = original.bufferViews == other.bufferViews;
	bool cameraSame = original.cameras == other.cameras;
	bool defaultSceneSame = original.defaultScene == other.defaultScene;
	bool extensionsSame = original.extensions == other.extensions;
	bool extensionsRequiredSame = original.extensionsRequired == other.extensionsRequired;
	bool extensionUsedSame = original.extensionsUsed == other.extensionsUsed;
	bool extraSame = original.extras == other.extras;
	bool imagesSame = original.images == other.images;
	bool lightsSame = original.lights == other.lights;
	bool materialSame = original.materials == other.materials;
	bool meshesSame = original.meshes == other.meshes;
	bool nodesSame = original.nodes == other.nodes;
	bool samplersSame = original.samplers == other.samplers;
	bool scenesSame = original.scenes == other.scenes;
	bool skinsSame = original.skins == other.skins;
	bool textureSame = original.textures == other.textures;

	bool total = accessorsSame && animSame && assetSame && bufferSame && bufferViewSame && cameraSame && defaultSceneSame &&
		extensionsSame && extensionsRequiredSame && extensionUsedSame && extraSame && imagesSame && lightsSame && materialSame &&
		meshesSame && nodesSame && samplersSame && scenesSame && skinsSame && textureSame;

	return total;
}

static bool operator==(const original_tinygltf::tinygltf::Node &original, const rapidjson_tinygltf::tinygltf::Node &other) {
	return original.camera == other.camera && original.children == other.children &&
		original.extensions == other.extensions && original.extras == other.extras &&
		Equals(original.matrix, other.matrix) && original.mesh == other.mesh &&
		original.name == other.name && Equals(original.rotation, other.rotation) &&
		Equals(original.scale, other.scale) && original.skin == other.skin &&
		Equals(original.translation, other.translation) &&
		Equals(original.weights, other.weights);
}
static bool operator==(const original_tinygltf::tinygltf::Parameter &original, const rapidjson_tinygltf::tinygltf::Parameter &other) {
	if (original.bool_value != other.bool_value ||
		original.has_number_value != other.has_number_value)
		return false;

	if (!TINYGLTF_DOUBLE_EQUAL(original.number_value, other.number_value))
		return false;

	if (original.json_double_value.size() != other.json_double_value.size())
		return false;
	for (auto &it : original.json_double_value) {
		auto otherIt = other.json_double_value.find(it.first);
		if (otherIt == other.json_double_value.end()) return false;

		if (!TINYGLTF_DOUBLE_EQUAL(it.second, otherIt->second)) return false;
	}

	if (!Equals(original.number_array, other.number_array)) return false;

	if (original.string_value != other.string_value) return false;

	return true;
}

static bool operator==(const original_tinygltf::tinygltf::Primitive &original, const rapidjson_tinygltf::tinygltf::Primitive &other) {
	return original.attributes == other.attributes && original.extras == other.extras &&
		original.indices == other.indices && original.material == other.material &&
		original.mode == other.mode && original.targets == other.targets;
}
static bool operator==(const original_tinygltf::tinygltf::Sampler &original, const rapidjson_tinygltf::tinygltf::Sampler &other) {
	return original.extras == other.extras && original.magFilter == other.magFilter &&
		original.minFilter == other.minFilter && original.name == other.name &&
		original.wrapR == other.wrapR && original.wrapS == other.wrapS &&
		original.wrapT == other.wrapT;
}
static bool operator==(const original_tinygltf::tinygltf::Scene &original, const rapidjson_tinygltf::tinygltf::Scene &other) {
	return original.extensions == other.extensions && original.extras == other.extras &&
		original.name == other.name && original.nodes == other.nodes;
	;
}
static bool operator==(const original_tinygltf::tinygltf::Skin &original, const rapidjson_tinygltf::tinygltf::Skin &other) {
	return original.inverseBindMatrices == other.inverseBindMatrices &&
		original.joints == other.joints && original.name == other.name &&
		original.skeleton == other.skeleton;
}
static bool operator==(const original_tinygltf::tinygltf::Texture & original, const rapidjson_tinygltf::tinygltf::Texture &other) {
	return original.extensions == other.extensions && original.extras == other.extras &&
		original.name == other.name && original.sampler == other.sampler &&
		original.source == other.source;
}


TEST_CASE("Gltf version compare: 'data/gltfs/unit_test_models/CesiumMilkTruck.gltf'")
{
	std::string file = "data/gltfs/unit_test_models/CesiumMilkTruck.gltf";

	original_tinygltf::tinygltf::Model originalGltfModel;
	rapidjson_tinygltf::tinygltf::Model rapidjsonGltfModel;
	REQUIRE_NOTHROW(originalGltfModel = original_tinygltf::LoadModelFromGltf(file));
	REQUIRE_NOTHROW(rapidjsonGltfModel = rapidjson_tinygltf::LoadModelFromGltf(file));

	CHECK(originalGltfModel == rapidjsonGltfModel);
}

TEST_CASE("Glb version compare: 'data/gltfs/unit_test_models/CesiumMilkTruck.glb'")
{
	std::string fileGlb = "data/gltfs/unit_test_models/CesiumMilkTruck.glb";
	original_tinygltf::tinygltf::Model originalGlbModel;
	rapidjson_tinygltf::tinygltf::Model rapidjsonGlbModel;
	REQUIRE_NOTHROW(originalGlbModel = original_tinygltf::LoadModelFromGlb(fileGlb));
	REQUIRE_NOTHROW(rapidjsonGlbModel = rapidjson_tinygltf::LoadModelFromGlb(fileGlb));
	
	CHECK(originalGlbModel == rapidjsonGlbModel);
}

TEST_CASE("Glb version compare: 'data/gltfs/unit_test_models/GameObject.glb'")
{
	std::string fileGlb = "data/gltfs/unit_test_models/GameObject.glb";
	original_tinygltf::tinygltf::Model originalGlbModel;
	rapidjson_tinygltf::tinygltf::Model rapidjsonGlbModel;
	REQUIRE_NOTHROW(originalGlbModel = original_tinygltf::LoadModelFromGlb(fileGlb));
	REQUIRE_NOTHROW(rapidjsonGlbModel = rapidjson_tinygltf::LoadModelFromGlb(fileGlb));

	CHECK(originalGlbModel == rapidjsonGlbModel);
}
#endif