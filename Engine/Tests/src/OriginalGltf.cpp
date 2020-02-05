#ifndef PLATFORM_PS
#include <OriginalGltf.h>
#include <IndependantTinyGltfStructures.h>

// We need to include the stl headers from tinygltf so we can get is outside of our custom namespace
// ReSharper disable CppUnusedIncludeDirective
#include <array>
#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <map>
#include <string>
#include <vector>

#include <algorithm>
#include <fstream>
#include <sstream>

#include <functional>

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>  // include API for expanding a file path

// And these are the stl headers from nlohmann/json
#include <algorithm> // all_of, find, for_each
#include <cassert> // assert
#include <ciso646> // and, not, or
#include <cstddef> // nullptr_t, ptrdiff_t, size_t
#include <functional> // hash, less
#include <initializer_list> // initializer_list
#include <iosfwd> // istream, ostream
#include <iterator> // random_access_iterator_tag
#include <numeric> // accumulate
#include <string> // string, stoi, to_string
#include <utility> // declval, forward, move, pair, swap

#include <cstdint> // int64_t, uint64_t
#include <map> // map
#include <memory> // allocator
#include <string> // string
#include <vector> // vector

#include <ciso646> // not
#include <cstddef> // size_t
#include <type_traits> // conditional, enable_if, false_type, integral_constant, is_constructible, is_integral, is_same, remove_cv, remove_reference, true_type

#include <iterator> // random_access_iterator_tag

#include <exception> // exception
#include <stdexcept> // runtime_error
#include <string> // to_string

#include <cstddef> // size_t

#include <array> // array
#include <ciso646> // and
#include <cstddef> // size_t
#include <cstdint> // uint8_t

#include <algorithm> // transform
#include <array> // array
#include <ciso646> // and, not
#include <forward_list> // forward_list
#include <iterator> // inserter, front_inserter, end
#include <map> // map
#include <string> // string
#include <tuple> // tuple, make_tuple
#include <type_traits> // is_arithmetic, is_same, is_enum, underlying_type, is_convertible
#include <unordered_map> // unordered_map
#include <utility> // pair, declval
#include <valarray> // valarray

#include <ciso646> // or, and, not
#include <iterator> // begin, end
#include <tuple> // tuple, get
#include <type_traits> // is_same, is_constructible, is_floating_point, is_enum, underlying_type
#include <utility> // move, forward, declval, pair
#include <valarray> // valarray
#include <vector> // vector

#include <cassert> // assert
#include <cstddef> // size_t
#include <cstring> // strlen
#include <istream> // istream
#include <iterator> // begin, end, iterator_traits, random_access_iterator_tag, distance, next
#include <memory> // shared_ptr, make_shared, addressof
#include <numeric> // accumulate
#include <string> // string, char_traits
#include <type_traits> // enable_if, is_base_of, is_pointer, is_integral, remove_pointer
#include <utility> // pair, declval
#include <cstdio> //FILE *

#include <clocale> // localeconv
#include <cstddef> // size_t
#include <cstdlib> // strtof, strtod, strtold, strtoll, strtoull
#include <cstdio> // snprintf
#include <initializer_list> // initializer_list
#include <string> // char_traits, string
#include <vector> // vector

#include <cassert> // assert
#include <cmath> // isfinite
#include <cstdint> // uint8_t
#include <functional> // function
#include <string> // string
#include <utility> // move

#include <utility> // declval

#include <cstddef>
#include <string>
#include <vector>

#include <json.hpp>
// ReSharper restore CppUnusedIncludeDirective


namespace original_tinygltf {
// #define TESTS_ALLOW_NLOHMANN_JSON

#define TINYGLTF_NO_STB_IMAGE_WRITE
#define TINYGLTF_IMPLEMENTATION
#include <tinygltf/tiny_gltf.h>
}


namespace original_tinygltf {
	static ::mock_tinygltf::Value ConvertTo(tinygltf::Value& value)
	{
		switch (value.Type()) {
		case mock_tinygltf::NULL_TYPE:
			return ::mock_tinygltf::Value{};
		case mock_tinygltf::NUMBER_TYPE:
			return ::mock_tinygltf::Value{ value.Get<double>()};
		case mock_tinygltf::INT_TYPE:
			return ::mock_tinygltf::Value{value.Get<int>()};
		case mock_tinygltf::BOOL_TYPE:
			return ::mock_tinygltf::Value{value.Get<bool>()};
		case mock_tinygltf::STRING_TYPE:
			return ::mock_tinygltf::Value{value.Get<std::string>()};
		case mock_tinygltf::ARRAY_TYPE: {
			auto& arr = value.Get<tinygltf::Value::Array>();
			std::vector<::mock_tinygltf::Value> convertedArr(arr.size());
			for (size_t i = 0; i < arr.size(); ++i) {
				convertedArr[i] = ConvertTo(arr[i]);
			}
			return ::mock_tinygltf::Value{ convertedArr };
		}
		case mock_tinygltf::BINARY_TYPE: {
			auto& arr = value.Get<std::vector<unsigned char>>();
			return ::mock_tinygltf::Value{ arr.data(), arr.size()};
		}
		case mock_tinygltf::OBJECT_TYPE: {
			auto& obj = value.Get<tinygltf::Value::Object>();
			mock_tinygltf::Value::Object convertedObj{};
			for (auto& pair : obj) {
				convertedObj.emplace(pair.first, ConvertTo(pair.second));
			}
			return ::mock_tinygltf::Value{ convertedObj };
		}
		default: 
			ASSERT_MSG(false, "UNKNOWN TYPE for mock_tinygltf::Value");
			return ::mock_tinygltf::Value{};
		}
	}

	/*
		struct {
			int count;
			bool isSparse;
			struct {
				int byteOffset;
				int bufferView;
				int componentType;  // a TINYGLTF_COMPONENT_TYPE_ value
			} indices;
			struct {
				int bufferView;
				int byteOffset;
			} values;
		} sparse;
	 */
	using mock_tinygltf_accessor_sparse_t = decltype(std::declval<mock_tinygltf::Accessor>().sparse);
	using original_tinygltf_accessor_sparse_t = decltype(std::declval<tinygltf::Accessor>().sparse);

	static mock_tinygltf_accessor_sparse_t ConvertTo(original_tinygltf_accessor_sparse_t& sparse)
	{
		mock_tinygltf_accessor_sparse_t convertedSparse{};
		convertedSparse.count = sparse.count;
		convertedSparse.isSparse = sparse.isSparse;
		convertedSparse.values.bufferView = sparse.values.bufferView;
		convertedSparse.values.byteOffset = sparse.values.byteOffset;
		convertedSparse.indices.bufferView = sparse.indices.bufferView;
		convertedSparse.indices.byteOffset = sparse.indices.byteOffset;
		convertedSparse.indices.componentType = sparse.indices.componentType;
		return convertedSparse;
	}

	static ::mock_tinygltf::Accessor ConvertTo(tinygltf::Accessor& accessor)
	{
		::mock_tinygltf::Accessor convertedAccessor{};

		convertedAccessor.bufferView = accessor.bufferView;
		convertedAccessor.byteOffset = accessor.byteOffset;
		convertedAccessor.componentType = accessor.componentType;
		convertedAccessor.count = accessor.count;
		convertedAccessor.extras = ConvertTo(accessor.extras);
		convertedAccessor.type = accessor.type;
		convertedAccessor.maxValues = accessor.maxValues;
		convertedAccessor.minValues = accessor.minValues;
		convertedAccessor.name = accessor.name;
		convertedAccessor.normalized = accessor.normalized;
		convertedAccessor.sparse = ConvertTo(accessor.sparse);

		return convertedAccessor;
	}

	static ::mock_tinygltf::Model ConvertToIndependantStructure(::original_tinygltf::tinygltf::Model& model)
	{
		::mock_tinygltf::Model independantModel{};

		// reserve the size for each vector
		independantModel.accessors.reserve(model.accessors.size());
		for(auto& accessor : model.accessors) {
			independantModel.accessors.push_back(ConvertTo(accessor));
		}


		return independantModel;
	}

	mock_tinygltf::Model LoadModelFromGltf(const std::string& file)
	{
		tinygltf::Model model;
		tinygltf::TinyGLTF loader;
		std::string err;
		std::string warn;

		bool ret = loader.LoadASCIIFromFile(&model, &err, &warn, file, mock_tinygltf::SectionCheck::REQUIRE_ALL);
		if (!ret)
			throw std::runtime_error(std::string("original_tinygltf::LoadModelFromGtf failed to load file: '") 
				+ file + "' \nWith errors: '" + err + "'\nWith warnings: '" + warn + "'");


		mock_tinygltf::Model gltfModel = ConvertToIndependantStructure(model);

		return gltfModel;
	}

	mock_tinygltf::Model LoadModelFromGlb(const ::std::string& file)
	{
		tinygltf::Model model;
		tinygltf::TinyGLTF loader;
		std::string err;
		std::string warn;

		bool ret = loader.LoadBinaryFromFile(&model, &err, &warn, file, mock_tinygltf::SectionCheck::REQUIRE_ALL);
		if (!ret)
			throw std::runtime_error(std::string("original_tinygltf::LoadModelFromGlb failed to load file: '")
				+ file + "' \nWith errors: '" + err + "'\nWith warnings: '" + warn + "'");


		mock_tinygltf::Model gltfModel = ConvertToIndependantStructure(model);

		return gltfModel;
	}
}
#endif