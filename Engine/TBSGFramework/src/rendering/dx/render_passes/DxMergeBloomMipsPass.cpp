#ifdef PLATFORM_WINDOWS
#include "rendering/dx/render_passes/DxMergeBloomMipsPass.h"
#include "core/Defines.h"
#include "rendering/dx/DX12Application.h"
#include "rendering/gnm/SimpleVertex.h"
#include "rendering/dx/Helpers.h"
#include "rendering/ResourceIds.h"
#include "rendering/dx/DX12Shader.h"
#include "core/Config.h"
#include "rendering/dx/DX12Renderer.h"
#include "rendering/shader_headers/BloomShaderBuffers.h"
#include "rendering/dx/CommandQueue.h"
#include "rendering/dx/CommandList.h"


namespace gfx
{
	const D3D12_INPUT_ELEMENT_DESC fullscreenQuadInputElements[2] =
	{
		{ "POSITION",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD",   0, DXGI_FORMAT_R32G32_FLOAT,    0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	};

	DxMergeBloomMipsPass::DxMergeBloomMipsPass(DX12Renderer* renderer, Texture& textureToMerge) : renderer(renderer), textureToMerge(&textureToMerge)
	{
		Microsoft::WRL::ComPtr<ID3D12Device2> device = DX12Application::Get().GetDevice();

		auto& rm = renderer->GetGfxResourceManager();

		static ShaderId bloomMergeVertexShader = rm.CreateShader(tbsg::Config::Get().MakeShaderPath("Bloom_v.cso", 1));
		static ShaderId bloomMergePixelShader = rm.CreateShader(tbsg::Config::Get().MakeShaderPath("Bloom_p.cso", 1));

		DX12Shader* bloomMergeVertexShaderDx = static_cast<DX12Shader*>(&rm.GetShader(bloomMergeVertexShader));
		DX12Shader* bloomMergePixelShaderDx = static_cast<DX12Shader*>(&rm.GetShader(bloomMergePixelShader));

		D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData = {};
		featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;
		if (FAILED(device->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &featureData, sizeof(featureData)))) {
			featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
		}
		D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags =
			D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;

		CD3DX12_DESCRIPTOR_RANGE1 bloomMergeDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1);

		const uint32_t rootParamAmount = 2;
		CD3DX12_ROOT_PARAMETER1 rootParameters[rootParamAmount];
		rootParameters[0].InitAsConstantBufferView(0, 0, D3D12_ROOT_DESCRIPTOR_FLAG_NONE, D3D12_SHADER_VISIBILITY_PIXEL);
		rootParameters[1].InitAsDescriptorTable(1, &bloomMergeDescriptorRange);

		CD3DX12_STATIC_SAMPLER_DESC linearMirrorSampler(0, D3D12_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR, D3D12_TEXTURE_ADDRESS_MODE_MIRROR, D3D12_TEXTURE_ADDRESS_MODE_MIRROR);


		CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDescription;
		rootSignatureDescription.Init_1_1(rootParamAmount, rootParameters, 1, &linearMirrorSampler, rootSignatureFlags);

		bloomRootSignature.SetRootSignatureDesc(rootSignatureDescription.Desc_1_1, featureData.HighestVersion);

		// Setup the pipeline state.
		struct PipelineStateStream
		{
			CD3DX12_PIPELINE_STATE_STREAM_ROOT_SIGNATURE pRootSignature;
			CD3DX12_PIPELINE_STATE_STREAM_INPUT_LAYOUT InputLayout;
			CD3DX12_PIPELINE_STATE_STREAM_PRIMITIVE_TOPOLOGY PrimitiveTopologyType;
			CD3DX12_PIPELINE_STATE_STREAM_VS VS;
			CD3DX12_PIPELINE_STATE_STREAM_PS PS;
			CD3DX12_PIPELINE_STATE_STREAM_BLEND_DESC BlendDesc;
			CD3DX12_PIPELINE_STATE_STREAM_DEPTH_STENCIL_FORMAT DSVFormat;
			CD3DX12_PIPELINE_STATE_STREAM_RENDER_TARGET_FORMATS RTVFormats;
			CD3DX12_PIPELINE_STATE_STREAM_SAMPLE_DESC SampleDesc;
			CD3DX12_PIPELINE_STATE_STREAM_DEPTH_STENCIL DepthStencil;
			CD3DX12_PIPELINE_STATE_STREAM_RASTERIZER Rasterizer;
		} pipelineStateStream;

		CD3DX12_DEPTH_STENCIL_DESC dsd(D3D12_DEFAULT);
		dsd.DepthEnable = false;
		dsd.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;

		CD3DX12_RASTERIZER_DESC rd(D3D12_DEFAULT);
		rd.CullMode = D3D12_CULL_MODE_NONE;

		// sRGB formats provide free gamma correction!
		DXGI_FORMAT backBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
		DXGI_FORMAT depthBufferFormat = DXGI_FORMAT_D32_FLOAT;

		CD3DX12_BLEND_DESC blendDesc = CD3DX12_BLEND_DESC{ D3D12_DEFAULT };
		blendDesc.AlphaToCoverageEnable = false;
		blendDesc.IndependentBlendEnable = false;
		blendDesc.RenderTarget[0].BlendEnable = TRUE;
		blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
		blendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
		blendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_INV_SRC_ALPHA;
		blendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
		blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
		blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_ONE;
		blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_ONE;

		// Check the best multisample quality level that can be used for the given back buffer format.
		DXGI_SAMPLE_DESC sampleDesc = DX12Application::Get().GetMultisampleQualityLevels(backBufferFormat, D3D12_MAX_MULTISAMPLE_SAMPLE_COUNT);

		D3D12_RT_FORMAT_ARRAY rtvFormats = {};
		rtvFormats.NumRenderTargets = 1;
		rtvFormats.RTFormats[0] = backBufferFormat;

		pipelineStateStream.pRootSignature = bloomRootSignature.GetRootSignature().Get();
		pipelineStateStream.InputLayout = { fullscreenQuadInputElements, 2 };
		pipelineStateStream.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		pipelineStateStream.VS = CD3DX12_SHADER_BYTECODE(bloomMergeVertexShaderDx->getShaderBlob());
		pipelineStateStream.PS = CD3DX12_SHADER_BYTECODE(bloomMergePixelShaderDx->getShaderBlob());
		pipelineStateStream.DSVFormat = depthBufferFormat;
		pipelineStateStream.BlendDesc = blendDesc;
		pipelineStateStream.RTVFormats = rtvFormats;
		pipelineStateStream.SampleDesc = sampleDesc;
		pipelineStateStream.DepthStencil = dsd;
		pipelineStateStream.Rasterizer = rd;


		D3D12_PIPELINE_STATE_STREAM_DESC pipelineStateStreamDesc = {
			sizeof(PipelineStateStream), &pipelineStateStream
		};
		ThrowIfFailed(device->CreatePipelineState(&pipelineStateStreamDesc, IID_PPV_ARGS(&bloomPipelineState)));


		{
			SimpleVertex vertices[4]
			{
				{{-1.0f, -1.0f, 0.1f,}, {0.0f, 1.0f}},
				{{-1.0f,  1.0f, 0.1f,}, {0.0f, 0.0f}},
				{{ 1.0f,  1.0f, 0.1f,}, {1.0f, 0.0f}},
				{{ 1.0f, -1.0f, 0.1f,}, {1.0f, 1.0f}},
			};

			uint16_t indicies[6]{
				0, 1, 2, 2, 3, 0
			};

			auto commandQueue = DX12Application::Get().GetCommandQueue();
			auto commandList = commandQueue->GetCommandList();

			commandList->CopyVertexBuffer(vertexBuffer, 4, sizeof(SimpleVertex), vertices);
			commandList->CopyIndexBuffer(indexBuffer, 6, DXGI_FORMAT_R16_UINT, indicies);

			auto fence = commandQueue->ExecuteCommandList(commandList);
			commandQueue->WaitForFenceValue(fence);
		}
	}

	void DxMergeBloomMipsPass::ExecutePass(Camera& camera, void* perFrameResources)
	{
		UNUSED(camera);
		UNUSED(perFrameResources);

		auto commandQueue = DX12Application::Get().GetCommandQueue();
		auto commandList = commandQueue->GetCommandList();

		commandList->SetPipelineState(bloomPipelineState);
		commandList->SetGraphicsRootSignature(bloomRootSignature);

		commandList->SetViewport(renderer->GetViewport());
		commandList->SetScissorRect(renderer->GetScissorsRect());

		commandList->SetRenderTarget(renderer->GetRenderTarget());

		BloomData bloomData;
		bloomData.bloomIntensity = 1.0f;
		bloomData.startMipLevel = 0;
		bloomData.finalBlurMip = 7;
		commandList->SetGraphicsDynamicConstantBuffer(0, sizeof(BloomData), &bloomData);
		commandList->SetShaderResourceView(1, 0, *textureToMerge, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, 0, D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES);

		commandList->SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		commandList->SetVertexBuffer(0, vertexBuffer);
		commandList->SetIndexBuffer(indexBuffer);
		commandList->DrawIndexed(6);

		auto fence = commandQueue->ExecuteCommandList(commandList);
		commandQueue->WaitForFenceValue(fence);
	}
}
#endif
