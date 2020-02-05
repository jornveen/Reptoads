#pragma once
#ifdef PLATFORM_PS
#include "rendering/gnm/PS4RenderPass.h"
#include "rendering/Mesh.h"
#include "rendering/gnm/PS4Mesh.h"
#include "rendering/Texture.h"
namespace gfx
{
	class PS4SkyBoxPass : public PS4RenderPass
	{
	public:
		PS4SkyBoxPass(uint8_t a_attributeBitMask, const ptl::vector<IShader*>& a_shaders, sce::Gnm::Sampler a_sampler, RenderPassId a_id, Mesh& mesh, Texture& texture);
		void ExecutePass(PS4Renderer& a_renderer, Camera& a_camera, sce::Gnmx::GnmxGfxContext& a_gfxc, Frame& a_frame) override;
	private:
		sce::Gnm::Sampler m_sampler;
		PS4Mesh* mesh;
		PS4Texture* texture;
	};

}
#endif