#include <rendering/dx/DX12Material.h>

namespace gfx
{
	DX12Material::DX12Material(const glm::vec3& /*a_ambient*/, const glm::vec3& /*a_diffuse*/, const glm::vec3& /*a_specular*/,
		float /*a_specularPower*/, const char* /*a_id*/)
		: m_data(nullptr),
		m_hasTexture(false), m_texId{}
	{
	}

	DX12Material::DX12Material(const glm::vec3& /*a_ambient*/, const glm::vec3& /*a_diffuse*/,
		const glm::vec3& /*a_specular*/, float /*a_specularPower*/, const char* a_textureId, const char* /*a_id*/)
		: m_data(nullptr),
		m_hasTexture(true), m_texId(a_textureId)
	{
	}
}