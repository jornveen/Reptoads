#pragma once
#ifdef PLATFORM_PS
#include "rendering/gnm/PS4RenderPass.h"
#include "rendering/gnm/PS4ParticleRenderer.h"

namespace gfx
{
	class PS4ParticlePass : public PS4RenderPass
	{
	public:
		PS4ParticlePass(uint8_t a_attributeBitMask, const ptl::vector<IShader*>& a_shaders, sce::Gnm::Sampler& sampler, PS4Renderer& renderer, const RenderPassId& a_id);
		void ExecutePass(PS4Renderer& a_renderer, Camera& a_camera, sce::Gnmx::GnmxGfxContext& a_gfxc, Frame& a_frame) override;
		PS4ParticleRenderer& GetParticleRenderer() { return particleRenderer; }
	private:
		PS4ParticleRenderer particleRenderer;
	};
}
#endif