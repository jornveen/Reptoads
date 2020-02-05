#pragma once
#ifdef PLATFORM_PS
#include "rendering/gnm/PS4RenderPass.h"

namespace gfx
{
	class PS4Renderable;

	class PS4MainPass : public PS4RenderPass
	{
	public:
		PS4MainPass(uint8_t a_attributeBitMask, const ptl::vector<IShader*>& a_shaders, sce::Gnm::Sampler a_sampler, RenderPassId a_id);
		void ExecutePass(PS4Renderer& a_renderer, Camera& a_camera, sce::Gnmx::GnmxGfxContext& a_gfxc, Frame& a_frame) override;
	private:
		void RenderRenderable(sce::Gnmx::GfxContext& a_gfxc, gfx::PS4Renderable& a_renderable, sce::Gnm::Texture& a_emissiveTex, PS4Renderer& a_renderer) const;
		sce::Gnm::Sampler m_sampler;
	};
}
#endif