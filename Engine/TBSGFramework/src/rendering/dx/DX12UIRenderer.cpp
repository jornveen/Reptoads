#ifdef PLATFORM_WINDOWS
#include "memory/smart_ptr.h"
#include <rendering/dx/DX12UIRenderer.h>
#include "core/Assertion.h"
#include "core/Defines.h"
#include "core/config.h"
#include <ui/UIText.h>
#include <ui/UIImage.h>

namespace gfx
{
	DX12UIRenderer::DX12UIRenderer(DX12Renderer* renderer)
		: renderer_(renderer), uiPanelRenderer_(renderer), textRenderer_(renderer) {}


    void DX12UIRenderer::Initialize(IResourceManager& gfxResourceManager)
	{
		resourceManager = &gfxResourceManager;
		uiPanelRenderer_.Initialize();
        if(tbsg::Config::Get().enableTextRendering)
        {
		    textRenderer_.Initialize();
        }
	}

	void DX12UIRenderer::Render(Camera& camera)
	{
		UNUSED(camera);

		ptl::vector<std::pair<float, std::pair<bool, void*>>> orderedUI;
		for (CachedUIImage& cachedUiImage : m_uiImages)
			if(cachedUiImage.uiImage->IsVisible())
				orderedUI.push_back({ cachedUiImage.uiImage->GetGlobalDepth(), {true, &cachedUiImage} });
		for(CachedUIText& cachedUiText : m_uiTexts)
			if (cachedUiText.uiText->IsVisible())
				orderedUI.push_back({ cachedUiText.uiText->GetGlobalDepth(), {false, &cachedUiText} });


		std::sort(orderedUI.begin(), orderedUI.end());
		if(!orderedUI.empty()) {
			uiPanelRenderer_.StartFrame();
			for (auto& zDepthAndUiImageOrText : orderedUI) {
				if(zDepthAndUiImageOrText.second.first) {
					auto* uiElement = static_cast<CachedUIImage*>(zDepthAndUiImageOrText.second.second);
					uiPanelRenderer_.RenderPanel(uiElement);
				}else {
					auto* uiElement = static_cast<CachedUIText*>(zDepthAndUiImageOrText.second.second);
					uiPanelRenderer_.RenderPanel(uiElement);
				}
			}
			uiPanelRenderer_.EndFrame();
		}

		/*if (!m_uiImages.empty()) {
			uiPanelRenderer_.StartFrame();
			for (CachedUIImage& cachedUiImage : m_uiImages) {
				uiPanelRenderer_.RenderPanel(&cachedUiImage);
			}
			uiPanelRenderer_.EndFrame();
		}

		if (tbsg::Config::Get().enableTextRendering && !m_uiTexts.empty()) {
			textRenderer_.StartFrame();
			for (CachedUIText& cachedUiText : m_uiTexts) {
				textRenderer_.RenderUIText(cachedUiText);
			}
			textRenderer_.EndFrame();
		}*/
		// // DX12 Implementation needs to be separated like this because the hacky DX12TextRenderer
		// // Can't take depth into account, so depth is emulated by overriding pixels directly on the backbuffer.
		// // This is why the iterator uses Breadth First Search. 
		// // It makes sure that all child widgets are rendered after the correct depth level
		// enum class ActiveRenderer
		// {
		// 	Panel,
		// 	Text,
		// };
		// //By default start with the panel renderer because it's likely we have more UIImages are used than Texts
		// ActiveRenderer activeRenderer = ActiveRenderer::Panel;
		// uiPanelRenderer_.StartFrame();
  //
		// WidgetWithWorldPosVector currentLayer{ { root_.get(), root_->pos } };
  //
		// while (!currentLayer.empty()) {
  //
		// 	for (auto& pair : currentLayer) {
		// 		UIWidget* widget = pair.first;
		// 		glm::vec2 offset = pair.second;
		// 		if (widget->type == UIWidgetType::Panel) {
		// 			if (activeRenderer != ActiveRenderer::Panel) {
  //                       if (tbsg::Config::Get().enableTextRendering)
  //                       {
  //                           textRenderer_.EndFrame();
  //                       }
		// 				uiPanelRenderer_.StartFrame();
		// 				activeRenderer = ActiveRenderer::Panel;
		// 			}
  //
		// 			uiPanelRenderer_.RenderPanel(static_cast<UIPanel*>(widget), offset);
		// 		} else if (widget->type == UIWidgetType::Text) {
  //                   if (tbsg::Config::Get().enableTextRendering)
  //                   {
  //                       if (activeRenderer != ActiveRenderer::Text) {
  //                           uiPanelRenderer_.EndFrame();
  //
  //                           textRenderer_.StartFrame();
  //
  //                           activeRenderer = ActiveRenderer::Text;
  //                       }
  //
		// 				textRenderer_.RenderUIText(static_cast<UIText*>(widget), offset);
		// 			}
		// 		}
		// 	}
  //
		// 	currentLayer = GetDirectChildren(currentLayer);
		// }
  //
		// if (activeRenderer == ActiveRenderer::Panel){
		// 	uiPanelRenderer_.EndFrame();
  //       }
  //       else if (activeRenderer == ActiveRenderer::Text && tbsg::Config::Get().enableTextRendering)
  //               textRenderer_.EndFrame();
	}

	void DX12UIRenderer::Shutdown()
	{
        if (tbsg::Config::Get().enableTextRendering)
        {
            textRenderer_.Shutdown();
        }
		uiPanelRenderer_.Shutdown();
	}

	// UIPanel* DX12UIRenderer::AddPanel(glm::vec2 pos, glm::vec2 size, gfx::TextureHandle texture, UIWidget* parent)
	// {
	// 	UIPanel* panel = IUIRenderer::AddPanel(pos, size, texture, parent);
	// 	// Potentially do some custom work for DX12
	// 	return panel;
	// }
	//
	// void DX12UIRenderer::RemovePanel(UIPanel* panel)
	// {
	// 	IUIRenderer::RemovePanel(panel);
	// }
	// UIText* DX12UIRenderer::AddText(glm::vec2 pos, glm::vec2 size, const std::string& text, UIText::HorizontalAlignment textAlignment,
	// 	UIText::VerticalAlignment verticalAlignment, float textSize, glm::vec4 textColor, UIWidget* parent)
	// {
	// 	UIText* uiText = IUIRenderer::AddText(pos, size, text, textAlignment, verticalAlignment, textSize, textColor, parent);
	// 	return uiText;
	// }
	// void DX12UIRenderer::RemoveUIText(UIText* uiText)
	// {
	// 	IUIRenderer::RemoveUIText(uiText);
	// }

	TextureId DX12UIRenderer::CreateTextTexture(const UITextOptionsForTexture& options)
	{
		auto& textTextures = textRenderer_.CreateTextureFromText(options);
		return textTextures.textureId;
	}

	void DX12UIRenderer::RemoveTextTexture(gfx::TextureId textureId)
	{
		textRenderer_.RemoveTextTexture(textureId);
	}

	void DX12UIRenderer::AddImage(ui::UIImage* uiImage, glm::mat4 modelMatrix)
	{
		IUIRenderer::AddImage(uiImage, modelMatrix);
		auto& cached = IUIRenderer::GetCachedImage(uiImage);
		uiPanelRenderer_.AddImage(cached);
	}

	void DX12UIRenderer::AddText(ui::UIText* uiText, glm::mat4 modelMatrix)
	{
		textRenderer_.AddUIText(uiText);
		IUIRenderer::AddText(uiText, modelMatrix);
	}

	auto DX12UIRenderer::UpdateImage(ui::UIImage* uiImage, glm::mat4 newModelMatrix)->CachedUIImage*
	{
		auto ptr = IUIRenderer::UpdateImage(uiImage, newModelMatrix);
		auto& cached = IUIRenderer::GetCachedImage(uiImage);
		uiPanelRenderer_.UpdateImage(cached);
		return ptr;
	}

	auto DX12UIRenderer::UpdateText(ui::UIText* uiText, glm::mat4 newModelMatrix)->CachedUIText*
	{
		textRenderer_.AddUIText(uiText);
		textRenderer_.RemoveUIText(*uiText);
		auto ptr = IUIRenderer::UpdateText(uiText, newModelMatrix);
		return ptr;
	}

	void DX12UIRenderer::RemoveImage(ui::UIImage* uiImage)
	{
		auto& cached = IUIRenderer::GetCachedImage(uiImage);
		uiPanelRenderer_.RemoveImage(*cached.uiImage);

		IUIRenderer::RemoveImage(uiImage);
	}

	void DX12UIRenderer::RemoveText(ui::UIText* uiText)
	{
		textRenderer_.RemoveUIText(*uiText);

		IUIRenderer::RemoveText(uiText);
	}

	// DX12UIRenderer::WidgetWithWorldPosVector DX12UIRenderer::GetDirectChildren(
	// 	const DX12UIRenderer::WidgetWithWorldPosVector& currentLayer)
	// {
	// 	WidgetWithWorldPosVector newLayer;
	//
	// 	for (auto& pair : currentLayer)
	// 	{
	// 		for (auto& uiWidgetPtr : pair.first->children)
	// 		{
	// 			newLayer.push_back({ uiWidgetPtr.get(), pair.second + uiWidgetPtr.get()->parent->pos });
	// 		}
	// 	}
	//
	// 	return newLayer;
	// }

	// void DX12UIRenderer::RenderIfPanel(UIWidget* widget)
	// {
	// 	// if(widget->type == UIWidgetType::Panel) {
	// 	// 	UIPanel* panel = static_cast<UIPanel*>(widget);
	// 	// 	uiPanelRenderer_.RenderPanel(panel);
	// 	// }
	// 	//
	// 	// for (ptl::unique_ptr<UIWidget>& child : widget->children) {
	// 	// 	RenderIfPanel(child.get());
	// 	// }
	// }

// 	void DX12UIRenderer::RenderIfText(UIWidget* widget)
// 	{
// #ifndef DISABLE_TEXT_RENDERER
// 		// if (widget->type == UIWidgetType::Text) {
// 		// 	UIText* text = static_cast<UIText*>(widget);
// 		// 	textRenderer_.RenderUIText(text);
// 		// }
// 		// for (ptl::unique_ptr<UIWidget>& child : widget->children) {
// 		// 	RenderIfText(child.get());
// 		// }
// #endif
// 	}
}
#endif
