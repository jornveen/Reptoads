#include "rendering/gnm/RenderPasses/PS4SkyboxPass.h"
#include "rendering/gnm/Frame.h"
#include "rendering/shader_headers/SkyboxBuffers.h"
#include "rendering/Camera.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

gfx::PS4SkyBoxPass::PS4SkyBoxPass(uint8_t a_attributeBitMask, const ptl::vector<IShader*>& a_shaders, sce::Gnm::Sampler a_sampler, RenderPassId a_id, Mesh& a_mesh, Texture& texture)
	: PS4RenderPass(a_attributeBitMask, a_shaders, a_id)
	, m_sampler(a_sampler)
	, mesh(static_cast<PS4Mesh*>(&a_mesh))
	, texture(static_cast<PS4Texture*>(&texture))
{
	m_depthStencilDesc.setDepthEnable(false);
	m_blendDesc.setBlendEnable(false);
	mesh->RegisterPass(*this);
}

void gfx::PS4SkyBoxPass::ExecutePass(PS4Renderer & a_renderer, Camera & a_camera, sce::Gnmx::GnmxGfxContext & a_gfxc, Frame & a_frame)
{
	sce::Gnm::Buffer buffer;

	PrepareForRendering(a_gfxc);

	a_gfxc.setRenderTarget(0, &a_frame.backBufferTarget);
	//a_gfxc.setDepthRenderTarget(nullptr);

	a_gfxc.setVertexBuffers(sce::Gnm::kShaderStageVs, 0, GetAttributeCount(), mesh->GetAttributeBuffers(*this));

	SkyboxBuffer* constants = static_cast<SkyboxBuffer*>(
		a_gfxc.allocateFromCommandBuffer(sizeof(SkyboxBuffer), sce::Gnm::kEmbeddedDataAlignment4));
	
	const auto& cameraPos = a_camera.get_Translation();
	glm::mat4 ProjMat = a_camera.get_ProjectionMatrix();
	glm::mat4 ViewMat = a_camera.get_ViewMatrix();
	glm::mat4 Modelmat = glm::translate(glm::mat4{ 1.0f }, cameraPos);
	
	constants->CameraPosition = cameraPos;
	constants->ModelMatrix = Modelmat;
	constants->MVPMatrix = ProjMat * ViewMat * Modelmat;

	buffer.initAsConstantBuffer(constants, sizeof(SkyboxBuffer));
	a_gfxc.setConstantBuffers(sce::Gnm::kShaderStageVs, 0, 1, &buffer);
	a_gfxc.setTextures(sce::Gnm::kShaderStagePs, 0, 1, texture->GetGnmTexture());
	a_gfxc.setSamplers(sce::Gnm::kShaderStagePs, 0, 1, &m_sampler);

	a_gfxc.setPrimitiveType(sce::Gnm::kPrimitiveTypeTriList);
	a_gfxc.setIndexSize(sce::Gnm::kIndexSize32);

	a_gfxc.drawIndex(mesh->GetIdxCount(), mesh->GetIdxBuffer());

}
