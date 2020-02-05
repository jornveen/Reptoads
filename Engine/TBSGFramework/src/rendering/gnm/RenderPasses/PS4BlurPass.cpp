#ifdef PLATFORM_PS
#include "rendering/gnm/RenderPasses/PS4BlurPass.h"
#include "BlurShaderBuffers.h"
#include "rendering/gnm/Frame.h"
#include "rendering/gnm/PS4Renderer.h"
#include <toolkit.h>

gfx::PS4BlurPass::PS4BlurPass(uint8_t a_attributeBitMask, const ptl::vector<IShader*>& a_shaders, RenderPassId a_id)
	:PS4RenderPass(a_attributeBitMask, a_shaders, a_id)
	, m_startMipLevel(0)
{
	auto ret = m_alloc.initialize(
		sizeof(BlurData) * 10, SCE_KERNEL_WB_ONION,
		SCE_KERNEL_PROT_CPU_RW | SCE_KERNEL_PROT_GPU_ALL);
	ASSERT(ret == SCE_OK);

	for (auto i = 0; i < 16; ++i)
	{
		m_blurData[i] = static_cast<BlurData*>(m_alloc.allocate(sizeof(BlurData), 1));
		ASSERT(m_blurData[i] != nullptr);
		m_blurData[i]->blurHorizontal = true;
		m_blurData[i]->mipLevel = 0;
		m_blurData[i]->dimensions.x = 1;
		m_blurData[i]->dimensions.y = 1;
		m_blurDataBuffer[i].initAsConstantBuffer(m_blurData[i], sizeof(BlurData));
	}
}

void gfx::PS4BlurPass::ExecutePass(PS4Renderer& a_renderer, Camera& a_camera, sce::Gnmx::GnmxGfxContext& a_gfxc,
	Frame& a_frame)
{
	PrepareForRendering(a_gfxc);

	const uint32_t baseWidth = a_frame.blurInputTexture.getWidth();
	const uint32_t baseHeight = a_frame.blurInputTexture.getHeight();
	const auto lastMipLevel = a_frame.blurInputTexture.getLastMipLevel();
	float currWidth = baseWidth;
	float currHeight = baseHeight;

	// the full resolution might be too expensive
	for (auto i = 0; i < m_startMipLevel; ++i)
	{
		currWidth *= 0.5f;
		currHeight *= 0.5f;
	}

	int buferIndex = 0;
	// blur with downsampled mips
	for (auto i = m_startMipLevel; i <= lastMipLevel; ++i)
	{
		const auto groupsX = (currWidth + 7) / 8; // 64 threads = 8 wide
		const auto groupsY = (currHeight + 7) / 8;// 64 threads = 8 tall

		// j -> 0 = horizontal
		for (auto j = 0; j <= 1; ++j)
		{
			auto hor = (j == 0);
			m_blurData[buferIndex]->dimensions.x = currWidth;
			m_blurData[buferIndex]->dimensions.y = currHeight;
			m_blurData[buferIndex]->mipLevel = i;
			m_blurData[buferIndex]->blurHorizontal = hor;
			a_gfxc.setConstantBuffers(sce::Gnm::kShaderStageCs, 0, 1, &m_blurDataBuffer[buferIndex]);
			a_gfxc.setTextures(sce::Gnm::kShaderStageCs, 0, 1, hor ? &a_frame.blurInputTexture : &a_frame.HorizontalBlurResultTexture);
			a_gfxc.setRwTextures(sce::Gnm::kShaderStageCs, 0, 1, hor ? &a_frame.HorizontalBlurResultTexture : &a_frame.blurInputTexture);
			a_gfxc.dispatch(groupsX, groupsY, 1);
			++buferIndex;

			// wait on horizontal before vertical
			if(j == 0)
			{
				a_gfxc.waitForGraphicsWrites(a_frame.HorizontalBlurResultTexture.getBaseAddress256ByteBlocks(), a_frame.HorizontalBlurResultTexture.getSizeAlign().m_size, -1,
					sce::Gnm::kCacheActionWriteBackAndInvalidateL1andL2, sce::Gnm::kExtendedCacheActionFlushAndInvalidateCbCache, sce::Gnm::kStallCommandBufferParserDisable);
			}
		}

		// update values for the next mip
		currWidth *= 0.5f;
		currHeight *= 0.5f;
	}


	// wait on the final vertical
	a_gfxc.waitForGraphicsWrites(a_frame.blurInputTexture.getBaseAddress256ByteBlocks(), a_frame.blurInputTexture.getSizeAlign().m_size, -1,
		sce::Gnm::kCacheActionWriteBackAndInvalidateL1andL2, sce::Gnm::kExtendedCacheActionFlushAndInvalidateCbCache, sce::Gnm::kStallCommandBufferParserDisable);
}
#endif
