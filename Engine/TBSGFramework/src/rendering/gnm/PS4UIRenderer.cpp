#ifdef PLATFORM_PS
#include <toolkit.h>
#include "rendering/gnm/SimpleVertex.h"
#include "rendering/gnm/RenderPasses/PS4UIPass.h"
#include <rendering/gnm/PS4UIRenderer.h>
#include <rendering/gnm/PS4Renderer.h>
#include <array>
#include <ui/UIText.h>
#include <ui/UIImage.h>

namespace gfx
{
	PS4UIRenderer::PS4UIRenderer(PS4Renderer& a_renderer)
		: m_renderer(&a_renderer), m_resourceManager(nullptr)
		, m_renderPass(nullptr)
	{
	}


	void PS4UIRenderer::Initialize(IResourceManager& gfxResourceManager)
	{
		IUIRenderer::Initialize(gfxResourceManager);
		m_resourceManager = &gfxResourceManager;
		//#TODO make sure the path is correct to prevent shaders from being loaded multiple times
		auto vsId = m_resourceManager->CreateShader("UIOverlayShader_v.sb");
		auto psId = m_resourceManager->CreateShader("UIOverlayShader_p.sb");
		ptl::vector<IShader*> shaders;
		shaders.push_back(&m_resourceManager->GetShader(vsId));
		shaders.push_back(&m_resourceManager->GetShader(psId));

		// create a sampler
		sce::Gnm::Sampler sampler;
		sampler.init();
		sampler.setWrapMode(sce::Gnm::WrapMode::kWrapModeClampBorder, sce::Gnm::WrapMode::kWrapModeClampBorder, sce::Gnm::WrapMode::kWrapModeClampBorder);
		sampler.setMipFilterMode(sce::Gnm::kMipFilterModeNone);
		sampler.setXyFilterMode(sce::Gnm::kFilterModeBilinear, sce::Gnm::kFilterModeBilinear);

		m_renderPass = new PS4UIPass(vertexPosition + vertexUv, shaders, sampler, m_renderer->GenerateRenderPassId());
		m_textRenderer.Initialize(*m_resourceManager, m_renderer->GetScreenSize());
	}

	void PS4UIRenderer::Render(Camera& a_camera)
	{
		ASSERT_MSG(m_renderPass, "ERROR, RENDER PASS NOT CREATED!");
		auto& gfxc = m_renderer->GetGfxContext();
		auto& frame = m_renderer->GetCurrentFrame();

		m_renderPass->ExecutePass(*m_renderer, a_camera, gfxc, frame);
	}

	void PS4UIRenderer::AddImage(ui::UIImage * uiImage, glm::mat4 modelMatrix)
	{
		IUIRenderer::AddImage(uiImage, modelMatrix);
		auto& image = GetCachedImage(uiImage);
	}

	void PS4UIRenderer::AddText(ui::UIText * uiText, glm::mat4 modelMatrix)
	{
		m_textRenderer.CreateTextTexture(uiText);
		IUIRenderer::AddText(uiText, modelMatrix);
		auto& text = GetCachedText(uiText);
	}

	auto PS4UIRenderer::UpdateImage(ui::UIImage * uiImage, glm::mat4 newModelMatrix) -> CachedUIImage*
	{
		auto ptr = IUIRenderer::UpdateImage(uiImage, newModelMatrix);
		auto& image = GetCachedImage(uiImage);
		if (ptr == nullptr) return ptr;

		auto& mesh = static_cast<PS4Mesh&>(resourceManager->GetMesh(GetCachedImage(uiImage).meshId));
		mesh.GetVertex(0) = ptr->vertices[0];
		mesh.GetVertex(1) = ptr->vertices[1];
		mesh.GetVertex(2) = ptr->vertices[2];
		mesh.GetVertex(3) = ptr->vertices[3];
		return ptr;
	}

	auto PS4UIRenderer::UpdateText(ui::UIText * uiText, glm::mat4 newModelMatrix) -> CachedUIText*
	{
		auto ptr = IUIRenderer::UpdateText(uiText, newModelMatrix);
		if (ptr == nullptr) return ptr;

		auto& mesh = static_cast<PS4Mesh&>(resourceManager->GetMesh(GetCachedText(uiText).meshId));
		mesh.GetVertex(0) = ptr->vertices[0];
		mesh.GetVertex(1) = ptr->vertices[1];
		mesh.GetVertex(2) = ptr->vertices[2];
		mesh.GetVertex(3) = ptr->vertices[3];
		return ptr;
	}

	void PS4UIRenderer::RemoveImage(ui::UIImage * uiImage)
	{
		IUIRenderer::RemoveImage(uiImage);
	}

	void PS4UIRenderer::RemoveText(ui::UIText * uiText)
	{
		m_resourceManager->DeleteTexture(uiText->GetTextTextureId());
		IUIRenderer::RemoveText(uiText);
	}

	TextureId PS4UIRenderer::CreateTextTexture(const UITextOptionsForTexture& options)
	{
		ui::UIText txt;
		txt.SetText(options.text);
		txt.SetFontSize(options.fontSize);
		txt.SetSize(options.size);
		txt.SetHorizontalAlignment(options.textAlignment);
		txt.SetVerticalAlignment(options.verticalAlignment);
		m_textRenderer.CreateTextTexture(&txt);
		return txt.GetTextTextureId();
	}

	void PS4UIRenderer::RemoveTextTexture(gfx::TextureId texture)
	{
		m_resourceManager->DeleteTexture(texture);
	}

	void PS4UIRenderer::Shutdown()
	{
		m_alloc.terminate();
		m_renderPass = nullptr;
		m_resourceManager = nullptr;
		m_renderer = nullptr;
	}
}

#endif
