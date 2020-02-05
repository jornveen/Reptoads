#pragma once
#ifdef PLATFORM_PS
#include "rendering/gnm/PS4RenderPass.h"
#include <common/allocator.h>

struct BlurData;

namespace gfx
{
	class PS4BlurPass : public PS4RenderPass
	{
	public:
		PS4BlurPass(uint8_t a_attributeBitMask, const ptl::vector<IShader*>& a_shaders, RenderPassId a_id);
		void ExecutePass(PS4Renderer& a_renderer, Camera& a_camera, sce::Gnmx::GnmxGfxContext& a_gfxc, Frame& a_frame) override;
	private:
		LinearAllocator m_alloc;
		BlurData* m_blurData[16]{};
		sce::Gnm::Buffer m_blurDataBuffer[16]{};
		const size_t m_startMipLevel;
	};
}
#endif