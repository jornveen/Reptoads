#pragma once
#ifdef PLATFORM_PS
#include "rendering/gnm/PS4UIRenderer.h"
class PS4UIPass : public gfx::PS4RenderPass
{
public:
	PS4UIPass(uint8_t a_attributeBitMask, ptl::vector<IShader*> a_shaders, sce::Gnm::Sampler a_sampler, RenderPassId a_id);
	void ExecutePass(PS4Renderer& a_renderer, Camera& a_camera, sce::Gnmx::GnmxGfxContext& a_gfxc, Frame& a_frame) override;

private:
	void RenderPanel(sce::Gnmx::GnmxGfxContext& a_gfxc, glm::mat4 a_mm, const sce::Gnm::Texture* a_texture, PS4Mesh& mesh, glm::vec4 a_color = glm::vec4(1,1,1,1)) const;
	sce::Gnm::Sampler m_sampler;
};
#endif