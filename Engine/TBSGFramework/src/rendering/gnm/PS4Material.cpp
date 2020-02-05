#ifdef PLATFORM_PS
#include "core/Assertion.h"
#include "rendering/gnm/PS4Material.h"
#include "rendering/gnm/PS4RenderPass.h"
#include <common/allocator.h>

gfx::PS4Material::PS4Material(const glm::vec3& a_emissive, const glm::vec4& a_diffuse, TextureId a_textureId, TextureId a_emissiveTextureId, MaterialId a_id, ptl::MemoryResource& a_alloc)
: PS4Resource(a_id), m_texId(a_textureId)
{
	m_data = static_cast<ShaderMaterial*>(a_alloc.allocate(sizeof(ShaderMaterial), sce::Gnm::kEmbeddedDataAlignment4));
	ASSERT_MSG(m_data != nullptr, "MATERIAL BUFFER ALLOCATION FAILED...");
	m_data->materialDiffuseColor = a_diffuse;
	m_data->materialEmissiveColor = glm::vec3(a_emissive);
	m_materialBufferPs.initAsConstantBuffer(m_data, sizeof(ShaderMaterial));
}

#endif
