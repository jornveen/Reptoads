#pragma once
#ifdef PLATFORM_WINDOWS
#include "RootSignature.h"
#include "IndexBuffer.h"
#include <glm/glm.hpp>
#include <array>

#include <rendering/dx/CommandQueue.h>
#include <rendering/dx/CommandList.h>
#include "rendering/IUIRenderer.h"

namespace gfx
{
	class DX12Renderer;

	class DX12UIPanelRenderer
	{
	public:
		explicit DX12UIPanelRenderer(DX12Renderer* renderer);

		void Initialize();
		void StartFrame();
		void RenderPanel(gfx::IUIRenderer::CachedUIImage* uiPanel);
		void RenderPanel(gfx::IUIRenderer::CachedUIText* uiPanel);
		void EndFrame();
		void Shutdown();

		void AddImage(gfx::IUIRenderer::CachedUIImage& uiImage);
		void UpdateImage(gfx::IUIRenderer::CachedUIImage& uiImage);
		void RemoveImage(ui::UIImage& uiImage);

	private:
		DX12Renderer* renderer;

		//DirectX state
		RootSignature uiOverlayRootSignature;
		Microsoft::WRL::ComPtr<ID3D12PipelineState> uiOverlayPipelineState;

		std::shared_ptr<CommandQueue> commandQueue_ {};
		std::shared_ptr<CommandList> commandList_ {};

		IndexBuffer indexBuffer;
		// ptl::unordered_map<ui::UIImage*, VertexBuffer> uiImageCache;

		// const std::array<uint32_t, 6> quadIndices_{ 0, 1, 2, 1, 3, 2};

	private:

// #pragma pack(push, 1)
// 			struct UIVertex
// 			{
// 				glm::vec3 Position;
// 				glm::vec2 TexCoord;
// 			};
// #pragma pack(pop)
// 			std::array<UIVertex, 4> CalculatePanelQuad(::gfx::UIPanel* panel, glm::vec2 additionalOffset);
	};
}
#endif
