#ifdef PLATFORM_PS
#include "rendering/gnm/RenderPasses/PS4VignettePass.h"
#include "rendering/gnm/Frame.h"
#include "rendering/gnm/PS4Renderer.h"
#include "VignetteShaderBuffers.h"
#include "rendering/gnm/SimpleVertex.h"

#include <toolkit.h>
  

gfx::PS4VignettePass::PS4VignettePass(uint8_t a_attributeBitMask, const ptl::vector<IShader*>& a_shaders, float a_radiusZeroToOne, float a_smoothnessZeroToOne, glm::vec<2,size_t> a_screenSize, RenderPassId a_id)
	:PS4RenderPass(a_attributeBitMask,a_shaders,a_id)
{
	auto ret = m_alloc.initialize(
		sizeof(VignetteData) + sizeof(SimpleVertex) * 4, SCE_KERNEL_WB_ONION,
		SCE_KERNEL_PROT_CPU_RW | SCE_KERNEL_PROT_GPU_ALL);
	ASSERT(ret == SCE_OK);

	a_radiusZeroToOne = glm::clamp<float>(a_radiusZeroToOne, 0, 1);
	a_smoothnessZeroToOne = glm::clamp<float>(a_smoothnessZeroToOne, 0, 1);

	m_vignetteData = static_cast<VignetteData*>(m_alloc.allocate(sizeof(VignetteData), 1));
	m_vignetteData->aspectRatio = static_cast<float>(a_screenSize.x) / static_cast<float>(a_screenSize.y);
	m_vignetteData->vignetteRadius = glm::clamp<float>(a_radiusZeroToOne, 0, 1);
	m_vignetteData->vignetteSoftness = glm::clamp<float>(a_smoothnessZeroToOne, 0, 1);
	m_vignetteBuffer.initAsConstantBuffer(m_vignetteData, sizeof(VignetteData));

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

}

void gfx::PS4VignettePass::ExecutePass(PS4Renderer& a_renderer, Camera& a_camera, sce::Gnmx::GnmxGfxContext& a_gfxc,
	Frame& a_frame)
{
	sce::Gnm::BlendControl blendControl;
	blendControl.init();
	blendControl.setBlendEnable(true);
	blendControl.setColorEquation(sce::Gnm::BlendMultiplier::kBlendMultiplierSrcAlpha, sce::Gnm::kBlendFuncAdd, sce::Gnm::kBlendMultiplierInverseSrc1Alpha);

	sce::Gnm::AlphaToMaskControl alphaToMaskControl;
	alphaToMaskControl.init();
	alphaToMaskControl.setEnabled(sce::Gnm::kAlphaToMaskDisable);

	a_gfxc.setAlphaToMaskControl(alphaToMaskControl);
	a_gfxc.setDepthStencilDisable();
	a_gfxc.setBlendControl(0, blendControl);
	a_gfxc.setDepthRenderTarget(nullptr);
	sce::Gnmx::Toolkit::SurfaceUtil::clearDepthTarget(a_gfxc, &a_frame.depthTarget, 1.f);

	PrepareForRendering(a_gfxc);
	a_gfxc.setConstantBuffers(sce::Gnm::kShaderStagePs, 0, 1, &m_vignetteBuffer);
	a_gfxc.setTextures(sce::Gnm::kShaderStagePs, 0, 1, &a_frame.backBufferTexture);
	a_gfxc.setVertexBuffers(sce::Gnm::kShaderStageVs, 0, 2, &m_vtxBuffers[0]);
	a_gfxc.drawIndexAuto(6);
}
#endif
