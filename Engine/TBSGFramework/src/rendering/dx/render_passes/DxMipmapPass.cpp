
#ifdef PLATFORM_WINDOWS
#include "rendering/dx/render_passes/DxMipmapPass.h"
#include "core/Defines.h"
#include "rendering/dx/DX12Application.h"
#include "rendering/dx/CommandQueue.h"
#include "rendering/dx/CommandList.h"
#include "rendering/dx/DX12Renderer.h"


namespace gfx
{
	DxMipmapPass::DxMipmapPass(DX12Renderer* renderer, Texture* texture) : renderer(renderer), textureToMipmap(texture)
	{
		
	}

	void DxMipmapPass::ExecutePass(Camera& camera, void* perFrameResources)
	{
		UNUSED(perFrameResources);
		UNUSED(camera);
		

		if(renderer->justResizedDuringFrame) {
			nonMsaaTextureToMipmap.Resize(renderer->windowClientWidth_, renderer->windowClientHeight_);
			nonMsaaTextureToMipmap.CreateViews();
		}

		bool isFirstFrame = false;
		if (!isNonMsaaTextureToMipmapCreated) {
			CreateNonMSAATexture();
			isFirstFrame = true;
		}

		auto commandQueue = DX12Application::Get().GetCommandQueue();
		auto resolveCommandList = commandQueue->GetCommandList();
		auto bugFixCommandList = commandQueue->GetCommandList();
		auto mipCommandList = commandQueue->GetCommandList();

		resolveCommandList->ResolveSubresource(nonMsaaTextureToMipmap, *textureToMipmap);
		//resolveCommandList->TransitionBarrier(nonMsaaTextureToMipmap, )

		if (!isFirstFrame && !renderer->justResizedDuringFrame) {
			//HACK: Because the ResourceStateTracker fucks up, we have to transition it into the state the ResourceStateTracker THINKS the resources are in.
			/*D3D12_RESOURCE_BARRIER res1 = CD3DX12_RESOURCE_BARRIER::Transition(nonMsaaTextureToMipmap.GetD3D12Resource().Get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COMMON, 1);
			D3D12_RESOURCE_BARRIER res2 = CD3DX12_RESOURCE_BARRIER::Transition(nonMsaaTextureToMipmap.GetD3D12Resource().Get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COMMON, 2);
			D3D12_RESOURCE_BARRIER res3 = CD3DX12_RESOURCE_BARRIER::Transition(nonMsaaTextureToMipmap.GetD3D12Resource().Get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COMMON, 3);
			D3D12_RESOURCE_BARRIER res4 = CD3DX12_RESOURCE_BARRIER::Transition(nonMsaaTextureToMipmap.GetD3D12Resource().Get(), D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_COMMON, 4);
			D3D12_RESOURCE_BARRIER res5 = CD3DX12_RESOURCE_BARRIER::Transition(nonMsaaTextureToMipmap.GetD3D12Resource().Get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COMMON, 5);
			D3D12_RESOURCE_BARRIER res6 = CD3DX12_RESOURCE_BARRIER::Transition(nonMsaaTextureToMipmap.GetD3D12Resource().Get(), D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_COMMON, 6);
			D3D12_RESOURCE_BARRIER res7 = CD3DX12_RESOURCE_BARRIER::Transition(nonMsaaTextureToMipmap.GetD3D12Resource().Get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COMMON, 7);

			bugFixCommandList->GetGraphicsCommandList()->ResourceBarrier(1, &res1);
			bugFixCommandList->GetGraphicsCommandList()->ResourceBarrier(1, &res2);
			bugFixCommandList->GetGraphicsCommandList()->ResourceBarrier(1, &res3);
			bugFixCommandList->GetGraphicsCommandList()->ResourceBarrier(1, &res4);
			bugFixCommandList->GetGraphicsCommandList()->ResourceBarrier(1, &res5);
			bugFixCommandList->GetGraphicsCommandList()->ResourceBarrier(1, &res6);
			bugFixCommandList->GetGraphicsCommandList()->ResourceBarrier(1, &res7);*/
		}

		mipCommandList->GenerateMips(nonMsaaTextureToMipmap);

		commandQueue->ExecuteCommandList(resolveCommandList);
		commandQueue->ExecuteCommandList(bugFixCommandList);
		auto fence = commandQueue->ExecuteCommandList(mipCommandList);
		commandQueue->WaitForFenceValue(fence);
	}

	void DxMipmapPass::CreateNonMSAATexture()
	{
		auto desc = textureToMipmap->GetD3D12ResourceDesc();
		desc.Alignment = 65536;
		desc.Flags &= ~(D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET);
		desc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
		desc.MipLevels = 8;
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;

		nonMsaaTextureToMipmap = Texture(desc, nullptr, TextureUsage::Albedo, L"NonMsaa Texture to Mipmap");
		nonMsaaTextureToMipmap.CreateViews();


		isNonMsaaTextureToMipmapCreated = true;
	}
}
#endif