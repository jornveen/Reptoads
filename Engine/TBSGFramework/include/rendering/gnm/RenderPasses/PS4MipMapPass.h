#pragma once
#ifdef PLATFORM_PS
#include "rendering/gnm/PS4RenderPass.h"
#include <common/allocator.h>


struct MipMapData;

namespace gfx
{
	class PS4MipMapPass : public PS4RenderPass
	{
	public:
		PS4MipMapPass(uint8_t a_attributeBitMask, const ptl::vector<IShader*>& a_shaders, RenderPassId a_id);
		void ExecutePass(PS4Renderer& a_renderer, Camera& a_camera, sce::Gnmx::GnmxGfxContext& a_gfxc, Frame& a_frame) override;

	private:
		LinearAllocator m_alloc;
		sce::Gnm::Sampler m_linearClampSampler;
		MipMapData* m_mipMapData[2]{};
		sce::Gnm::Buffer m_mipMapDataBuffer[2]{};
	};
}
#endif