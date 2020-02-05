#ifdef PLATFORM_PS
#include "rendering/shader_headers/UIOverlayShaderBuffers.h"
#include "rendering/gnm/RenderPasses/PS4UIPass.h"
#include "rendering/gnm/SimpleVertex.h"
#include <ui/UIImage.h>
#include <ui/UIText.h>

PS4UIPass::PS4UIPass(uint8_t a_attributeBitMask, ptl::vector<IShader*> a_shaders, sce::Gnm::Sampler a_sampler, RenderPassId a_id)
	: PS4RenderPass(a_attributeBitMask, a_shaders, a_id), m_sampler(a_sampler)
{
}

void PS4UIPass::ExecutePass(PS4Renderer& a_renderer, Camera& a_camera, sce::Gnmx::GnmxGfxContext& a_gfxc,
	Frame& a_frame)
{
	PrepareForRendering(a_gfxc);

	a_gfxc.setRenderTarget(0, &a_frame.backBufferTarget);
	a_gfxc.setDepthRenderTarget(nullptr);

	a_gfxc.setPrimitiveType(sce::Gnm::kPrimitiveTypeTriList);
	a_gfxc.setIndexSize(sce::Gnm::kIndexSize32);
	a_gfxc.setSamplers(sce::Gnm::kShaderStagePs, 0, 1, &m_sampler);

	glm::mat4 ortho = glm::orthoLH(0.0f, 1920.0f,
		0.0f, 1080.0f,
		a_camera.get_ZNear(), a_camera.get_ZFar());

	const auto& uiImages = static_cast<PS4UIRenderer*>(a_renderer.GetUIRenderer())->GetUIImages();
	const auto& uiTexts = static_cast<PS4UIRenderer*>(a_renderer.GetUIRenderer())->GetUITexts();
	auto& rm = a_renderer.GetGfxResourceManager();

	ptl::vector<std::pair<float, std::pair<bool, void*>>> orderedUI;
	for (const IUIRenderer::CachedUIImage& cachedUiImage : uiImages)
		if(cachedUiImage.uiImage->IsVisible())
			orderedUI.push_back(std::pair<float, std::pair<bool, void*>>{ cachedUiImage.uiImage->GetGlobalDepth(), std::pair<bool, void*>{true, (void*)&cachedUiImage} });
	for (const IUIRenderer::CachedUIText& cachedUiText : uiTexts)
		if(cachedUiText.uiText->IsVisible())
			orderedUI.push_back(std::pair<float, std::pair<bool, void*>>{ cachedUiText.uiText->GetGlobalDepth(), std::pair<bool, void*>{false, (void*)&cachedUiText} });

	std::sort(orderedUI.begin(), orderedUI.end());
	if (!orderedUI.empty()) {
		for (auto& zDepthAndUiImageOrText : orderedUI) {
			if (zDepthAndUiImageOrText.second.first) {
				const IUIRenderer::CachedUIImage* uiElement = static_cast<IUIRenderer::CachedUIImage*>(zDepthAndUiImageOrText.second.second);
				RenderPanel(a_gfxc, ortho, rm.GetTexture(rm.GetTextureId(uiElement->uiImage->GetTextureId())).GetGnmTexture(), static_cast<PS4Mesh&>(rm.GetMesh(uiElement->meshId)));
			}
			else {
				const IUIRenderer::CachedUIText* uiElement = static_cast<IUIRenderer::CachedUIText*>(zDepthAndUiImageOrText.second.second);
				RenderPanel(a_gfxc, ortho, a_renderer.GetGfxResourceManager().GetTexture(uiElement->uiText->GetTextTextureId()).GetGnmTexture(), static_cast<PS4Mesh&>(rm.GetMesh(uiElement->meshId)), uiElement->uiText->GetColor());
			}
		}
	}
}

void PS4UIPass::RenderPanel(sce::Gnmx::GnmxGfxContext& a_gfxc, glm::mat4 a_mm, const sce::Gnm::Texture* a_texture, PS4Mesh& mesh, glm::vec4 a_color) const
{
	a_gfxc.setVertexBuffers(sce::Gnm::kShaderStageVs, 0, GetAttributeCount(), mesh.GetAttributeBuffers(*this));

	// allocate the vertex shader constants
	UIObjectConstantsVs* objectConstants = static_cast<UIObjectConstantsVs*>(
		a_gfxc.allocateFromCommandBuffer(sizeof(UIObjectConstantsVs), sce::Gnm::kEmbeddedDataAlignment4));
	ASSERT_MSG(objectConstants != nullptr, "ConstantPerObjectBufferAlloc failed..");

	// allocate the pixel shader constants
	UIObjectConstantsPs* psConstants = static_cast<UIObjectConstantsPs*>(
		a_gfxc.allocateFromCommandBuffer(sizeof(UIObjectConstantsPs), sce::Gnm::kEmbeddedDataAlignment4));
	ASSERT_MSG(psConstants != nullptr, "ConstantPixelBufferAlloc failed..");

	// set the shader constants
	objectConstants->projectionMatrix = a_mm;
	psConstants->textColor = a_color;

	// submit the shader data
	sce::Gnm::Buffer buffers[2];
	buffers[0].initAsConstantBuffer(objectConstants, sizeof(UIObjectConstantsVs));
	buffers[1].initAsConstantBuffer(psConstants, sizeof(UIObjectConstantsPs));
	a_gfxc.setConstantBuffers(sce::Gnm::kShaderStageVs, 0, 1, &buffers[0]);
	a_gfxc.setConstantBuffers(sce::Gnm::kShaderStagePs, 1, 1, &buffers[1]);
	a_gfxc.setTextures(sce::Gnm::kShaderStagePs, 0, 1, a_texture);
	a_gfxc.drawIndex(mesh.GetIdxCount(), mesh.GetIdxBuffer());
} 
#endif