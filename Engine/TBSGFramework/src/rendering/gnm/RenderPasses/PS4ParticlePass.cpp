#ifdef PLATFORM_PS
#include "rendering/gnm/RenderPasses/PS4ParticlePass.h"

PS4ParticlePass::PS4ParticlePass(uint8_t a_attributeBitMask, const ptl::vector<IShader*>& a_shaders, sce::Gnm::Sampler& sampler, PS4Renderer& renderer, const RenderPassId & a_id)
	: PS4RenderPass(a_attributeBitMask, a_shaders, a_id), particleRenderer(renderer, sampler,  *this)
{
}

void PS4ParticlePass::ExecutePass(PS4Renderer& a_renderer, Camera& a_camera, sce::Gnmx::GnmxGfxContext& a_gfxc,
	Frame& a_frame)
{
	PrepareForRendering(a_gfxc);
	particleRenderer.RenderParticles(a_gfxc, *this, a_camera);
}
#endif