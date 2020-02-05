
#ifdef PLATFORM_PS
#include"rendering/gnm/RenderPasses/PS4BloomPass.h"
#include "rendering/gnm/SimpleVertex.h"
#include "rendering/shader_headers/BloomShaderBuffers.h"
#include "rendering/gnm/Frame.h"
#include "core/Assertion.h"
#include <toolkit.h>


gfx::PS4BloomPass::PS4BloomPass(uint8_t a_attributeBitMask, const ptl::vector<IShader*>& a_shaders, RenderPassId a_id)
	: PS4RenderPass(a_attributeBitMask, a_shaders, a_id)
{
	auto ret = m_alloc.initialize(
		sizeof(BloomData) + sizeof(SimpleVertex) * 4, SCE_KERNEL_WB_ONION,
		SCE_KERNEL_PROT_CPU_RW | SCE_KERNEL_PROT_GPU_ALL);
	ASSERT(ret == SCE_OK);

	m_bloomData = static_cast<BloomData*>(m_alloc.allocate(sizeof(BloomData), 1));
	m_bloomData->startMipLevel = 0;
	m_bloomData->bloomIntensity = 1.0f;
	m_bloomData->finalBlurMip = 7;
	m_bloomBuffer.initAsConstantBuffer(m_bloomData, sizeof(BloomData));

	SimpleVertex vertices[4]
	{
		{{-1.0f, -1.0f, 0.1f,}, {0.0f, 1.0f}},
		{{-1.0f,  1.0f, 0.1f,}, {0.0f, 0.0f}},
		{{ 1.0f,  1.0f, 0.1f,}, {1.0f, 0.0f}},
		{{ 1.0f, -1.0f, 0.1f,}, {1.0f, 1.0f}},
	};

	const auto size = sizeof(SimpleVertex) * 4;
	float* vdata = static_cast<float*>(m_alloc.allocate(size, 4));
	ASSERT(vdata != nullptr);
	memcpy(vdata, vertices, size);
	m_vtxBuffers[0].initAsVertexBuffer(vdata, sce::Gnm::kDataFormatR32G32B32Float, sizeof(SimpleVertex), 4);
	m_vtxBuffers[1].initAsVertexBuffer(vdata + 3, sce::Gnm::kDataFormatR32G32Float, sizeof(SimpleVertex), 4);

	sampler.init();
	sampler.setMipFilterMode(sce::Gnm::kMipFilterModeLinear);
	sampler.setXyFilterMode(sce::Gnm::kFilterModeBilinear, sce::Gnm::kFilterModeBilinear);
}

void gfx::PS4BloomPass::ExecutePass(PS4Renderer& a_renderer, Camera& a_camera, sce::Gnmx::GnmxGfxContext& a_gfxc,
	Frame& a_frame)
{
	//make sure to wait for the result of the blur pass
	a_gfxc.setDepthRenderTarget(nullptr);
	a_gfxc.setRenderTarget(1, nullptr);

	PrepareForRendering(a_gfxc);
	a_gfxc.setConstantBuffers(sce::Gnm::kShaderStagePs, 0, 1, &m_bloomBuffer);
	a_gfxc.setSamplers(sce::Gnm::kShaderStagePs, 0, 1, &sampler);
	a_gfxc.setTextures(sce::Gnm::kShaderStagePs, 0, 1, &a_frame.backBufferTexture);
	a_gfxc.setTextures(sce::Gnm::kShaderStagePs, 1, 1, &a_frame.blurInputTexture);
	a_gfxc.setVertexBuffers(sce::Gnm::kShaderStageVs, 0, 2, &m_vtxBuffers[0]);
	a_gfxc.drawIndexAuto(6);
}
#endif