#pragma once
#ifdef PLATFORM_PS
#include "rendering/gnm/PS4RenderPass.h"
#include <common/allocator.h>

struct BloomData;

namespace gfx
{
	class PS4BloomPass : public PS4RenderPass
	{
	public:
		PS4BloomPass(uint8_t a_attributeBitMask, const ptl::vector<IShader*>& a_shaders, RenderPassId a_id);
		void ExecutePass(PS4Renderer& a_renderer, Camera& a_camera, sce::Gnmx::GnmxGfxContext& a_gfxc, Frame& a_frame) override;

	private:
		sce::Gnm::Sampler sampler;
		LinearAllocator m_alloc;
		BloomData* m_bloomData{};
		sce::Gnm::Buffer m_bloomBuffer{};
		sce::Gnm::Buffer m_vtxBuffers[2]{};
	};
}
#endif