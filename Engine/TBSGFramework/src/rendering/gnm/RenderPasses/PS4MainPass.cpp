#ifdef PLATFORM_PS
#include "rendering/gnm/RenderPasses/PS4MainPass.h"
#include "rendering/gnm/PS4ResourceManager.h"

PS4MainPass::PS4MainPass(uint8_t a_attributeBitMask, const ptl::vector<IShader*>& a_shaders, sce::Gnm::Sampler a_sampler, RenderPassId a_id)
	: PS4RenderPass(a_attributeBitMask, a_shaders, a_id)
	, m_sampler(a_sampler)
{}

void PS4MainPass::ExecutePass(PS4Renderer& a_renderer, Camera& a_camera, sce::Gnmx::GnmxGfxContext& a_gfxc,
	Frame& a_frame)
{
	PrepareForRendering(a_gfxc);
	a_gfxc.setRenderTarget(0, &a_frame.backBufferTarget);
	a_gfxc.setRenderTarget(1, &a_frame.emissiveTarget);
	a_gfxc.setDepthRenderTarget(&a_frame.depthTarget);
	a_frame.gfxContext.setRenderTargetMask(0xFFFFFFFF);
	const auto& cache = a_renderer.GetTexCache();


	// set vertex shader constant buffer, data automatically cleared after command execution
	PerFrameConstantsVs* frameConstants = static_cast<PerFrameConstantsVs*>(
	a_gfxc.allocateFromCommandBuffer(sizeof(PerFrameConstantsVs), sce::Gnm::kEmbeddedDataAlignment4));
	ASSERT_MSG(frameConstants != nullptr, "ConstantPerFrameBufferAlloc failed..");

	frameConstants->vpMatrix = a_camera.get_ProjectionMatrix() * a_camera.get_ViewMatrix();
	frameConstants->viewMatrix = a_camera.get_ViewMatrix();
	frameConstants->camPosWs = glm::vec4(a_camera.get_Translation(), 1.0);

	//// set pixel shader constant buffers
	//sce::Gnm::Buffer bufferPs;
	//bufferPs.initAsConstantBuffer(m_lightDataGpu, sizeof(LightDataPs));
	//gfxc.setConstantBuffers(sce::Gnm::kShaderStagePs, 1, 1, &bufferPs);

	sce::Gnm::Buffer bufferVs;
	bufferVs.initAsConstantBuffer(frameConstants, sizeof(PerFrameConstantsVs));
	a_gfxc.setConstantBuffers(sce::Gnm::kShaderStageVs, 0, 1, &bufferVs);

	// set the sampler
	a_gfxc.setSamplers(sce::Gnm::kShaderStagePs, 0, 1, &m_sampler);
	sce::Gnm::AlphaToMaskControl alphaToMaskControl;
	alphaToMaskControl.init();
	alphaToMaskControl.setEnabled(sce::Gnm::AlphaToMaskMode::kAlphaToMaskEnable);
	a_gfxc.setAlphaToMaskControl(alphaToMaskControl);

	// render objects with textures
	if (!cache.empty())
	{
		for (auto texMap = cache.begin(); texMap != cache.end(); ++texMap)
		{
			const auto texId = texMap->first;
			
			// Setup the texture and its sampler on the PS stage
			a_gfxc.setTextures(sce::Gnm::kShaderStagePs, 0, 1, a_renderer.GetGfxResourceManager().GetTexture(TextureId{ texId }).GetGnmTexture());
			for (PS4Renderable* renderable : texMap->second)
			{
				auto materialId = renderable->GetMaterialId();
				auto& material = a_renderer.GetGfxResourceManager().GetMaterial(static_cast<gfx::MaterialId&>(materialId));
				ASSERT(texId == material.diffuseTextureId._id);
				auto& emissiveInputTexture = a_renderer.GetGfxResourceManager().GetTexture(material.emissiveTextureId);

				a_gfxc.setTextures(sce::Gnm::kShaderStagePs, 1, 1, emissiveInputTexture.GetGnmTexture());
				RenderRenderable(a_gfxc, *renderable, a_frame.emissiveTexture, a_renderer);
			}
		}

		// wait until writing to the emissive texture has completed
		a_gfxc.waitForGraphicsWrites(a_frame.emissiveTarget.getBaseAddress256ByteBlocks(),a_frame.emissiveTarget.getColorSizeAlign().m_size,-1,
			sce::Gnm::kCacheActionWriteBackAndInvalidateL1andL2, sce::Gnm::kExtendedCacheActionFlushAndInvalidateCbCache, sce::Gnm::kStallCommandBufferParserDisable);
	}
}

void PS4MainPass::RenderRenderable(sce::Gnmx::GfxContext& a_gfxc, gfx::PS4Renderable& a_renderable, sce::Gnm::Texture& a_emissiveTex, PS4Renderer& a_renderer) const
{
	// assert when material buffer has not been created
	//ASSERT_MSG(a_renderable.GetMaterial()->IsBufferCreated(), "ERROR, the material data buffer has not been created yet...");

	sce::Gnm::Buffer buffer;
	sce::Gnm::Buffer materialBuffer;
	//#TODO make sure LShaders work too, don't assume StageVs
	a_gfxc.setVertexBuffers(sce::Gnm::kShaderStageVs, 0, GetAttributeCount(), a_renderable.GetMesh()->GetAttributeBuffers(*this));

	// Allocate the vertex shader constants from the command buffer
	PerObjectConstantsVs* constants = static_cast<PerObjectConstantsVs*>(
		a_gfxc.allocateFromCommandBuffer(sizeof(PerObjectConstantsVs), sce::Gnm::kEmbeddedDataAlignment4));
	ASSERT_MSG(constants, "ConstantPerObjectBufferAlloc failed..");

	const auto& modelMat = a_renderable.GetTransform().GetWorldModelMatrix();
	constants->modelMatrix = a_renderable.GetTransform().GetWorldModelMatrix();
	constants->normalMatrix = glm::transpose(glm::inverse(modelMat));

	buffer.initAsConstantBuffer(constants, sizeof(PerObjectConstantsVs));
	a_gfxc.setPrimitiveType(sce::Gnm::kPrimitiveTypeTriList);
	a_gfxc.setIndexSize(sce::Gnm::kIndexSize32);

	materialBuffer.initAsConstantBuffer(&a_renderable.GetMaterial()->GetShaderMaterialData(), sizeof(ShaderMaterial));
	// set the constant buffers
	a_gfxc.setConstantBuffers(sce::Gnm::kShaderStageVs, 1, 1, &buffer);
	a_gfxc.setConstantBuffers(sce::Gnm::kShaderStagePs, 2, 1, &materialBuffer);
	a_gfxc.setRwTextures(sce::Gnm::kShaderStagePs, 0, 1, &a_emissiveTex);

	// Submit a draw call
	a_gfxc.drawIndex(a_renderable.GetMesh()->GetIdxCount(), a_renderable.GetMesh()->GetIdxBuffer());
}
#endif