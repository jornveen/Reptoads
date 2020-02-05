#pragma once

#ifdef  PLATFORM_WINDOWS
#include "rendering/RenderPass.h"
#include "rendering/dx/ResourceStateTracker.h"
#include "rendering/dx/RootSignature.h"
#include "rendering/dx/VertexBuffer.h"
#include "rendering/dx/IndexBuffer.h"
#include <wrl/client.h>
#include "rendering/Texture.h"


namespace gfx
{
	class DX12Renderer;

	class DxMergeBloomMipsPass : public IRenderPass
	{
	public:
		DxMergeBloomMipsPass(DX12Renderer* renderer, Texture& textureToMerge);

		void ExecutePass(Camera& camera, void* perFrameResources) override;

	private:
		DX12Renderer* renderer;
		Texture* textureToMerge;

		Microsoft::WRL::ComPtr<ID3D12PipelineState> bloomPipelineState;
		RootSignature bloomRootSignature;
		VertexBuffer vertexBuffer{ L"Merge Bloom Fullscreen vertex buffer" };
		IndexBuffer indexBuffer{ L"Merge Bloom Fullscreen index buffer" };
	};
}
#endif