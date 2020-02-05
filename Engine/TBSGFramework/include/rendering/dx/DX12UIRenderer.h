#pragma once
#include "rendering/Texture.h"
#ifdef PLATFORM_WINDOWS
#include "rendering/IUIRenderer.h"
#include <rendering/dx/DX12UIPanelRenderer.h>
#include <rendering/dx/DX12TextRenderer.h>


namespace gfx
{
	class DX12Renderer;

	class DX12UIRenderer : public IUIRenderer
	{
		DX12Renderer* renderer_;
		DX12UIPanelRenderer uiPanelRenderer_;
		DX12TextRenderer textRenderer_;

	public:
		DX12UIRenderer(DX12Renderer* renderer);
		~DX12UIRenderer() override = default;

		void Initialize(IResourceManager& gfxResourceManager) override;
		void Render(Camera& camera) override;
		void Shutdown() override;

		// UIPanel* AddPanel(glm::vec2 pos, glm::vec2 size, gfx::TextureHandle texture, UIWidget* parent = nullptr) override;
		// void RemovePanel(UIPanel* panel) override;
		// UIText* AddText(glm::vec2 pos, glm::vec2 size, const std::string& text, UIText::HorizontalAlignment textAlignment,
		// 	UIText::VerticalAlignment verticalAlignment, float textSize, glm::vec4 textColor, UIWidget* parent) override;
		// void RemoveUIText(UIText* text) override;

		TextureId CreateTextTexture(const UITextOptionsForTexture& options) override;
		void RemoveTextTexture(gfx::TextureId texture) override;

		void AddImage(ui::UIImage* uiImage, glm::mat4 modelMatrix) override;
		void AddText(ui::UIText* uiText, glm::mat4 modelMatrix) override;
		auto UpdateImage(ui::UIImage* uiImage, glm::mat4 newModelMatrix)->CachedUIImage* override;
		auto UpdateText(ui::UIText* uiText, glm::mat4 newModelMatrix)->CachedUIText* override;
		void RemoveImage(ui::UIImage* uiImage) override;
		void RemoveText(ui::UIText* uiText) override;

		DX12UIPanelRenderer& GetUIPanelRenderer() { return uiPanelRenderer_; }
		DX12TextRenderer& GetUITextRenderer() { return textRenderer_; }
	private:
		// using WidgetWithWorldPosVector = ptl::vector<std::pair<UIWidget*, glm::vec2>>;
		// WidgetWithWorldPosVector GetDirectChildren(const WidgetWithWorldPosVector& currentLayer);
		// void RenderIfPanel(UIWidget* widget);
		// void RenderIfText(UIWidget* widget);
	};
}
#endif
