#ifdef PLATFORM_PS
#include "rendering/gnm/RenderPasses/PS4MipMapPass.h"
#include "rendering/gnm/PS4Renderer.h"
#include "rendering/gnm/Frame.h"
#include "core/Assertion.h"
#include "MipMapShaderBuffers.h"
#include <toolkit.h>

gfx::PS4MipMapPass::PS4MipMapPass(uint8_t a_attributeBitMask, const ptl::vector<IShader*>& a_shaders, RenderPassId a_id)
	: PS4RenderPass(a_attributeBitMask, a_shaders, a_id)
{
	// init the allocator
	auto ret = m_alloc.initialize(
		sizeof(MipMapData) * 2, SCE_KERNEL_WB_ONION,
		SCE_KERNEL_PROT_CPU_RW | SCE_KERNEL_PROT_GPU_ALL);
	ASSERT(ret == SCE_OK);

	// alloc and init the GPU data
	auto startMip = 0;
	for(auto i = 0; i < 2; ++i)
	{
		m_mipMapData[i] = static_cast<MipMapData*>(m_alloc.allocate(sizeof(MipMapData), 1));
		ASSERT(m_mipMapData[i] != nullptr);
		m_mipMapData[i]->dimensions = glm::vec2{ 0.0f,0.0f };
		m_mipMapData[i]->srcMipLevel = startMip;
		m_mipMapData[i]->numMipLevels = 1;
		m_mipMapData[i]->texDesc = 0;
		m_mipMapDataBuffer[i].initAsConstantBuffer(m_mipMapData[i], sizeof(MipMapData));
		startMip += 4;
	}

	// init the sampler
	m_linearClampSampler.init();
	m_linearClampSampler.setXyFilterMode(sce::Gnm::kFilterModeBilinear, sce::Gnm::kFilterModeBilinear);
	m_linearClampSampler.setWrapMode(sce::Gnm::kWrapModeClampBorder, sce::Gnm::kWrapModeClampBorder, sce::Gnm::kWrapModeClampBorder);
	m_linearClampSampler.setMipFilterMode(sce::Gnm::kMipFilterModeLinear);
}

void gfx::PS4MipMapPass::ExecutePass(PS4Renderer& a_renderer, Camera& a_camera, sce::Gnmx::GnmxGfxContext& a_gfxc,
	Frame& a_frame)
{
	PrepareForRendering(a_gfxc);

	auto& inTex = a_frame.emissiveTexture;
	auto& outTex = a_frame.blurInputTexture;
	auto width = outTex.getWidth();
	auto height = outTex.getHeight();

	// generate 8 mips
	for(int i = 0; i < 2; ++i)
	{
		m_mipMapData[i]->isSrgb = outTex.getTextureChannelType() == sce::Gnm::kTextureChannelTypeSrgb;

		const auto widthEven = (width % 2) == 0;
		const auto heightEven = (height % 2) == 0;
		if (widthEven && heightEven) m_mipMapData[i]->texDesc = TEX_DESC_WIDTH_HEIGHT_EVEN;
		else if (!widthEven && heightEven) m_mipMapData[i]->texDesc = TEX_DESC_WIDTH_ODD_HEIGHT_EVEN;
		else if (widthEven && !heightEven) m_mipMapData[i]->texDesc = TEX_DESC_WIDTH_EVEN_HEIGHT_ODD;
		else if (!widthEven && !heightEven) m_mipMapData[i]->texDesc = TEX_DESC_WIDTH_HEIGHT_ODD;

		m_mipMapData[i]->dimensions = glm::vec2{ width,height };
		m_mipMapData[i]->texelSize = glm::vec2{ 1.0f / m_mipMapData[i]->dimensions.x, 1.0f / m_mipMapData[i]->dimensions.y };
		m_mipMapData[i]->numMipLevels = 4;// glm::min<uint32_t>(4, (outTex.getLastMipLevel() + 1) - m_mipMapData->srcMipLevel);

		// return if there are no mip maps
		if (m_mipMapData[i]->numMipLevels == 0)
			return;

		const auto groupsX = (width + 7) / 8; // 64 threads = 8 wide
		const auto groupsY = (height + 7) / 8;// 64 threads = 8 tall
		a_gfxc.setConstantBuffers(sce::Gnm::kShaderStageCs, 0, 1, &m_mipMapDataBuffer[i]);
		a_gfxc.setSamplers(sce::Gnm::kShaderStageCs, 0, 1, &m_linearClampSampler);
		a_gfxc.setTextures(sce::Gnm::kShaderStageCs, 0, 1, &inTex);
		a_gfxc.setRwTextures(sce::Gnm::kShaderStageCs, 0, 1, &outTex);
		a_gfxc.dispatch(groupsX, groupsY, 1);


		a_gfxc.waitForGraphicsWrites(outTex.getBaseAddress256ByteBlocks(), outTex.getSizeAlign().m_size, -1,
			sce::Gnm::kCacheActionWriteBackAndInvalidateL1andL2, sce::Gnm::kExtendedCacheActionFlushAndInvalidateCbCache, sce::Gnm::kStallCommandBufferParserDisable);

		for (int i = 0; i < 4; ++i)
		{
			width *= .5;
			height *= .5;
		}
	}
}
#endif
