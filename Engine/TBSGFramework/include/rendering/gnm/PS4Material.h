#pragma once
#ifdef PLATFORM_PS
#include <gnm/buffer.h>
#include <glm/vec3.hpp>
#include "rendering/gnm/PS4Resource.h"
#include "rendering/gnm/ClassID.h"
#include "DefaultShaderBuffers.h"
#include "memory/memory_resource.h"

class LinearAllocator;

namespace gfx
{
	// bitmask should be obtained from the shader
	class PS4Material : public PS4Resource, public ClassID<PS4Material>
	{
	public:
		PS4Material(const glm::vec3& a_emissive, const glm::vec4& a_diffuse, TextureId a_textureId, TextureId a_emissiveTextureId, MaterialId a_id, ptl::MemoryResource& a_alloc);
		bool IsBufferCreated() const { return m_data != nullptr; }
		const sce::Gnm::Buffer* GetBufferPs() const { return &m_materialBufferPs; }
		void SetTexture(TextureId a_texId) { m_texId = a_texId; }
		TextureId GetTextureId() const { return m_texId; }
		TextureId GetEmissiveTextureId() const { return m_emissiveTexId; }
	private:
		sce::Gnm::Buffer m_materialBufferPs{};
		// stack allocated data is thread specific
		ShaderMaterial* m_data{};
		TextureId m_texId{0};
		TextureId m_emissiveTexId{0};
		//gfx::BlendMode m_blendMode;
	};
}
#endif
