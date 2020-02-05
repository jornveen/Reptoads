#pragma once
#ifdef PLATFORM_PS
#include "rendering/IResourceManager.h"
#include "GenericMultiMap.h"
#include "PS4Resource.h"
#include "PS4Renderer.h"
#include "PS4Mesh.h"
#include "memory/freelist_resources.h"
#include "memory/monotonic_resource.h"
#include <glm/vec3.hpp>

namespace gfx
{
	class PS4Mesh;
	class PS4Renderer;
	class TextureLoader;

	class PS4ResourceManager : public IResourceManager, public GenericMultiMap<PS4Resource>
	{
	public:
		PS4ResourceManager(PS4Renderer& a_core);
		~PS4ResourceManager();

		TextureId AddTexture(Texture* inTexture) override;
		TextureId CreateTexture(const ptl::string& filePath) override;
		TextureId CreateTextureFromMemory(
			unsigned char* texBuffer, 
			const uint32_t width, 
			const uint32_t height,
			const uint32_t bitsPerChannel, 
			const uint32_t amountOfChannels,
			const glm::vec2 minUvs,
			const ptl::string& optionalName) override;

		MeshId CreateMesh(
			const Vertex* vertices, 
			size_t vtxCount, 
			const uint32_t* indices, 
			size_t idxCount,
			const ptl::string& optionalName) override;

		MaterialId CreateMaterial(
			const BlendMode blendMode, 
			const glm::vec4 diffuseFactor, 
			const TextureId diffuseTextureId,
			const glm::vec3 emissiveFactor, 
			const TextureId emissiveTextureId, 
			float alphaCutoff, 
			const ptl::string& optionalName) override;

		ShaderId CreateShader(const ptl::string& shaderPath) override;
		Texture& GetDefaultTexture() override;
		TextureId GetDefaultTextureId() override;

		bool ContainsTexture(TextureId a_id) override;
		bool ContainsMesh(MeshId a_id) override;
		//bool ContainsMaterial(MaterialId a_id) override;
		bool ContainsShader(ShaderId a_id) override;

		PS4Texture& GetTexture(TextureId a_id) override;
		PS4Mesh& GetMesh(MeshId a_id) override;
		IShader& GetShader(ShaderId a_id) override;

		void DeleteTexture(TextureId a_id) override;
		void DeleteMesh(MeshId a_id) override;
		void DeleteMaterial(MaterialId a_id) override;
		void DeleteShader(ShaderId a_id) override;
		void DeleteAllResources() override;

		private:
		ptl::FreeListResource meshMem;
		ptl::FreeListResource texMem;
		ptl::MonotonicResource shaderGarlicMem;
		ptl::MonotonicResource shaderOnionMem;
		ptl::MonotonicResource materialMem;
	};
}
#endif