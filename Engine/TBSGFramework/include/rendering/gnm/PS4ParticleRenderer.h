#pragma once
#ifdef PLATFORM_PS
#include "rendering/IParticleRenderer.h"
#include "rendering/gnm/PS4Renderer.h"

namespace gfx
{
	class PS4ParticleRenderer : public IParticleRenderer
	{
	public:
		explicit PS4ParticleRenderer(PS4Renderer& renderer, sce::Gnm::Sampler& sampler, PS4RenderPass& pass);
		void RenderParticles(sce::Gnmx::GnmxGfxContext& gfxc, PS4RenderPass& pass, gfx::Camera& camera);
		void RenderParticleEmitter(sce::Gnmx::GnmxGfxContext& gfxc, PS4RenderPass& pass, core::Transform& transform, ParticleEmitter& emitter, Camera& camera);
	private:
		sce::Gnm::Sampler sampler;
		PS4Mesh* particleGPUData;
	};
}
#endif	