#ifndef PLATFORM_PS
#include <IndependantTinyGltfStructures.h>

namespace mock_tinygltf
{
	// Equals function for Value, for recursivity
	static bool Equals(const mock_tinygltf::Value &one, const mock_tinygltf::Value &other) {
		if (one.Type() != other.Type()) return false;

		switch (one.Type()) {
		case NULL_TYPE:
			return true;
		case BOOL_TYPE:
			return one.Get<bool>() == other.Get<bool>();
		case NUMBER_TYPE:
			return MOCK_TINYGLTF_DOUBLE_EQUAL(one.Get<double>(), other.Get<double>());
		case INT_TYPE:
			return one.Get<int>() == other.Get<int>();
		case OBJECT_TYPE: {
			auto oneObj = one.Get<mock_tinygltf::Value::Object>();
			auto otherObj = other.Get<mock_tinygltf::Value::Object>();
			if (oneObj.size() != otherObj.size()) return false;
			for (auto &it : oneObj) {
				auto otherIt = otherObj.find(it.first);
				if (otherIt == otherObj.end()) return false;

				if (!Equals(it.second, otherIt->second)) return false;
			}
			return true;
		}
		case ARRAY_TYPE: {
			if (one.Size() != other.Size()) return false;
			for (int i = 0; i < int(one.Size()); ++i)
				if (Equals(one.Get(i), other.Get(i))) return false;
			return true;
		}
		case STRING_TYPE:
			return one.Get<std::string>() == other.Get<std::string>();
		case BINARY_TYPE:
			return one.Get<std::vector<unsigned char> >() ==
				other.Get<std::vector<unsigned char> >();
		default: {
			// unhandled type
			return false;
		}
		}
	}

	// Equals function for std::vector<double> using TINYGLTF_DOUBLE_EPSILON
	static bool Equals(const std::vector<double> &one,
		const std::vector<double> &other) {
		if (one.size() != other.size()) return false;
		for (int i = 0; i < int(one.size()); ++i) {
			if (!MOCK_TINYGLTF_DOUBLE_EQUAL(one[size_t(i)], other[size_t(i)])) return false;
		}
		return true;
	}

	bool Accessor::operator==(const Accessor &other) const {
		return this->bufferView == other.bufferView &&
			this->byteOffset == other.byteOffset &&
			this->componentType == other.componentType &&
			this->count == other.count && this->extras == other.extras &&
			Equals(this->maxValues, other.maxValues) &&
			Equals(this->minValues, other.minValues) && this->name == other.name &&
			this->normalized == other.normalized && this->type == other.type;
	}
	bool Animation::operator==(const Animation &other) const {
		return this->channels == other.channels && this->extras == other.extras &&
			this->name == other.name && this->samplers == other.samplers;
	}
	bool AnimationChannel::operator==(const AnimationChannel &other) const {
		return this->extras == other.extras &&
			this->target_node == other.target_node &&
			this->target_path == other.target_path &&
			this->sampler == other.sampler;
	}
	bool AnimationSampler::operator==(const AnimationSampler &other) const {
		return this->extras == other.extras && this->input == other.input &&
			this->interpolation == other.interpolation &&
			this->output == other.output;
	}
	bool Asset::operator==(const Asset &other) const {
		return this->copyright == other.copyright &&
			this->extensions == other.extensions && this->extras == other.extras &&
			this->generator == other.generator &&
			this->minVersion == other.minVersion && this->version == other.version;
	}
	bool Buffer::operator==(const Buffer &other) const {
		return this->data == other.data && this->extras == other.extras &&
			this->name == other.name && this->uri == other.uri;
	}
	bool BufferView::operator==(const BufferView &other) const {
		return this->buffer == other.buffer && this->byteLength == other.byteLength &&
			this->byteOffset == other.byteOffset &&
			this->byteStride == other.byteStride && this->name == other.name &&
			this->target == other.target && this->extras == other.extras &&
			this->dracoDecoded == other.dracoDecoded;
	}
	bool Camera::operator==(const Camera &other) const {
		return this->name == other.name && this->extensions == other.extensions &&
			this->extras == other.extras &&
			this->orthographic == other.orthographic &&
			this->perspective == other.perspective && this->type == other.type;
	}
	bool Image::operator==(const Image &other) const {
		return this->bufferView == other.bufferView &&
			this->component == other.component && this->extras == other.extras &&
			this->height == other.height && this->image == other.image &&
			this->mimeType == other.mimeType && this->name == other.name &&
			this->uri == other.uri && this->width == other.width;
	}
	bool Light::operator==(const Light &other) const {
		return Equals(this->color, other.color) && this->name == other.name &&
			this->type == other.type;
	}
	bool Material::operator==(const Material &other) const {
		return this->additionalValues == other.additionalValues &&
			this->extensions == other.extensions && this->extras == other.extras &&
			this->name == other.name && this->values == other.values;
	}
	bool Mesh::operator==(const Mesh &other) const {
		return this->extensions == other.extensions && this->extras == other.extras &&
			this->name == other.name && this->primitives == other.primitives &&
			this->targets == other.targets && Equals(this->weights, other.weights);
	}
	bool Model::operator==(const Model &other) const {
		return this->accessors == other.accessors &&
			this->animations == other.animations && this->asset == other.asset &&
			this->buffers == other.buffers &&
			this->bufferViews == other.bufferViews &&
			this->cameras == other.cameras &&
			this->defaultScene == other.defaultScene &&
			this->extensions == other.extensions &&
			this->extensionsRequired == other.extensionsRequired &&
			this->extensionsUsed == other.extensionsUsed &&
			this->extras == other.extras && this->images == other.images &&
			this->lights == other.lights && this->materials == other.materials &&
			this->meshes == other.meshes && this->nodes == other.nodes &&
			this->samplers == other.samplers && this->scenes == other.scenes &&
			this->skins == other.skins && this->textures == other.textures;
	}
	bool Node::operator==(const Node &other) const {
		return this->camera == other.camera && this->children == other.children &&
			this->extensions == other.extensions && this->extras == other.extras &&
			Equals(this->matrix, other.matrix) && this->mesh == other.mesh &&
			this->name == other.name && Equals(this->rotation, other.rotation) &&
			Equals(this->scale, other.scale) && this->skin == other.skin &&
			Equals(this->translation, other.translation) &&
			Equals(this->weights, other.weights);
	}
	bool OrthographicCamera::operator==(const OrthographicCamera &other) const {
		return this->extensions == other.extensions && this->extras == other.extras &&
			MOCK_TINYGLTF_DOUBLE_EQUAL(this->xmag, other.xmag) &&
			MOCK_TINYGLTF_DOUBLE_EQUAL(this->ymag, other.ymag) &&
			MOCK_TINYGLTF_DOUBLE_EQUAL(this->zfar, other.zfar) &&
			MOCK_TINYGLTF_DOUBLE_EQUAL(this->znear, other.znear);
	}
	bool Parameter::operator==(const Parameter &other) const {
		if (this->bool_value != other.bool_value ||
			this->has_number_value != other.has_number_value)
			return false;

		if (!MOCK_TINYGLTF_DOUBLE_EQUAL(this->number_value, other.number_value))
			return false;

		if (this->json_double_value.size() != other.json_double_value.size())
			return false;
		for (auto &it : this->json_double_value) {
			auto otherIt = other.json_double_value.find(it.first);
			if (otherIt == other.json_double_value.end()) return false;

			if (!MOCK_TINYGLTF_DOUBLE_EQUAL(it.second, otherIt->second)) return false;
		}

		if (!Equals(this->number_array, other.number_array)) return false;

		if (this->string_value != other.string_value) return false;

		return true;
	}
	bool PerspectiveCamera::operator==(const PerspectiveCamera &other) const {
		return MOCK_TINYGLTF_DOUBLE_EQUAL(this->aspectRatio, other.aspectRatio) &&
			this->extensions == other.extensions && this->extras == other.extras &&
			MOCK_TINYGLTF_DOUBLE_EQUAL(this->yfov, other.yfov) &&
			MOCK_TINYGLTF_DOUBLE_EQUAL(this->zfar, other.zfar) &&
			MOCK_TINYGLTF_DOUBLE_EQUAL(this->znear, other.znear);
	}
	bool Primitive::operator==(const Primitive &other) const {
		return this->attributes == other.attributes && this->extras == other.extras &&
			this->indices == other.indices && this->material == other.material &&
			this->mode == other.mode && this->targets == other.targets;
	}
	bool Sampler::operator==(const Sampler &other) const {
		return this->extras == other.extras && this->magFilter == other.magFilter &&
			this->minFilter == other.minFilter && this->name == other.name &&
			this->wrapR == other.wrapR && this->wrapS == other.wrapS &&
			this->wrapT == other.wrapT;
	}
	bool Scene::operator==(const Scene &other) const {
		return this->extensions == other.extensions && this->extras == other.extras &&
			this->name == other.name && this->nodes == other.nodes;
		;
	}
	bool Skin::operator==(const Skin &other) const {
		return this->inverseBindMatrices == other.inverseBindMatrices &&
			this->joints == other.joints && this->name == other.name &&
			this->skeleton == other.skeleton;
	}
	bool Texture::operator==(const Texture &other) const {
		return this->extensions == other.extensions && this->extras == other.extras &&
			this->name == other.name && this->sampler == other.sampler &&
			this->source == other.source;
	}
	bool Value::operator==(const Value &other) const {
		return Equals(*this, other);
	}
}
#endif