#pragma once
#ifdef PLATFORM_PS
#include <rendering/IUIRenderer.h>
#include "PS4ResourceManager.h"
#include "PS4TextRenderer.h"


namespace gfx
{
	class PS4Renderer;

	class PS4UIRenderer : public IUIRenderer
	{
	public:
		PS4UIRenderer(PS4Renderer& a_renderer);
		void Initialize(IResourceManager& gfxResourceManager) override;
		void Render(Camera& a_camera) override;
		void Shutdown() override;

		const ptl::vector<CachedUIImage>& GetUIImages() const { return m_uiImages; }
		const ptl::vector<CachedUIText>& GetUITexts() const { return m_uiTexts; }
		const ptl::vector<CachedUIImage>& GetUIImageList() const { return m_uiImages; }
		const ptl::vector<CachedUIText>& GetUITextList() const { return m_uiTexts; }
		const PS4TextRenderer& GetTextRenderer() const { return m_textRenderer; }

	private:
		void RenderPanel(sce::Gnmx::GnmxGfxContext& a_gfxc, glm::mat4 a_mm, glm::vec2 a_panelSize, sce::Gnm::Buffer* a_buffers, const sce::Gnm::Texture* a_texture, float depth, glm::vec4 a_color = glm::vec4(1, 1, 1, 1));
		void AddImage(ui::UIImage* uiImage, glm::mat4 modelMatrix) override;
		void AddText(ui::UIText* uiText, glm::mat4 modelMatrix) override;
		auto UpdateImage(ui::UIImage* uiImage, glm::mat4 newModelMatrix)->CachedUIImage* override;
		auto UpdateText(ui::UIText* uiText, glm::mat4 newModelMatrix)->CachedUIText* override;
		void RemoveImage(ui::UIImage* uiImage) override;
		void RemoveText(ui::UIText* uiText) override;

	public:
		TextureId CreateTextTexture(const UITextOptionsForTexture& options) override;
	    void RemoveTextTexture(gfx::TextureId texture) override;
	private:
		LinearAllocator m_alloc;
		PS4TextRenderer m_textRenderer;
		PS4Renderer* m_renderer;
		IResourceManager* m_resourceManager;
		PS4RenderPass* m_renderPass;
		glm::mat4 m_projMatrix;
		sce::Gnm::Buffer m_vtxBuffer[2]{};
		//size_t m_textCount;
	};
}

#endif
