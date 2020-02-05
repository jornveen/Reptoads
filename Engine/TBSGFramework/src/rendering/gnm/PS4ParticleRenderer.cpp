#pragma once
#ifdef PLATFORM_PS
#include "rendering/shader_headers/ParticleShaderBuffers.h"
#include "rendering/gnm/PS4ParticleRenderer.h"
#include "rendering/gnm/PS4Mesh.h"
#include "core/Config.h"

#define MAX_VERTICES 9999


namespace gfx
{
	PS4ParticleRenderer::PS4ParticleRenderer(PS4Renderer& renderer, sce::Gnm::Sampler& sampler, PS4RenderPass& pass)
		: sampler(sampler)
		, particleGPUData(nullptr)
	{
		Vertex vertices[MAX_VERTICES];
		uint32_t indices[MAX_VERTICES * 6];
		for(auto i = 0; i < MAX_VERTICES * 6; i+=6)
		{
			uint16_t start = static_cast<uint16_t>((i+1) / 6) * 4;
			indices[i + 0] = start + 0;
			indices[i + 1] = start + 2;
			indices[i + 2] = start + 1;
			indices[i + 3] = start + 1;
			indices[i + 4] = start + 2;
			indices[i + 5] = start + 3;
		}
		auto meshId = renderer.GetGfxResourceManager().CreateMesh(&vertices[0], MAX_VERTICES, &indices[0], MAX_VERTICES * 4);
		particleGPUData = &static_cast<PS4Mesh&>(renderer.GetGfxResourceManager().GetMesh(meshId));
		particleGPUData->RegisterPass(pass);
	}

	void PS4ParticleRenderer::RenderParticles(sce::Gnmx::GnmxGfxContext& gfxc, PS4RenderPass& pass, Camera& camera)
	{
		using core::Transform;

		SortParticles(camera);

		for (auto& punctualParticleEmitter : particleEmitters) {
			auto& transform = punctualParticleEmitter.transform;
			auto& particleEmitter = punctualParticleEmitter.particleEmitter;

			if (!particleEmitter.startParameters.renderLikeSkybox)
				RenderParticleEmitter(gfxc, pass, transform, particleEmitter, camera);
		}
	}

	void PS4ParticleRenderer::RenderParticleEmitter(sce::Gnmx::GnmxGfxContext& gfxc, PS4RenderPass& pass, core::Transform& transform, ParticleEmitter& emitter, Camera& camera)
	{
		if (emitter._particles.empty())
			return;

		struct ConstantBufferMatrices  // NOLINT(cppcoreguidelines-pro-type-member-init, hicpp-member-init)
		{
			glm::mat4 ModelMatrix;
			glm::mat4 ViewMatrix;
			glm::mat4 ProjectionMatrix;
			glm::mat4 MVPMatrix;
		} cbMats;

		cbMats.ViewMatrix = camera.get_ViewMatrix();
		cbMats.ProjectionMatrix = camera.get_ProjectionMatrix();

		//If we rotate the quads towards the camera then do it on the CPU
		if (emitter.startParameters.simulationSpace == SimulationSpace::Local) {
			cbMats.ModelMatrix = transform.GetWorldModelMatrix();
		}
		else if (emitter.startParameters.simulationSpace == SimulationSpace::LocalOnlyPosition) {
			glm::mat4 modelMat = transform.GetWorldModelMatrix();
			glm::vec4 translation = modelMat[3];
			cbMats.ModelMatrix = glm::translate(glm::mat4{ 1.0f }, glm::vec3{ translation });
		}
		else {
			cbMats.ModelMatrix = glm::mat4{ 1.0f };
		}

		//cbMats.MVPMatrix = DirectX::XMMatrixMultiply(DirectX::XMMatrixMultiply(cbMats.ModelMatrix, cbMats.ViewMatrix), cbMats.ProjectionMatrix);
		cbMats.MVPMatrix = cbMats.ProjectionMatrix * cbMats.ViewMatrix * cbMats.ModelMatrix;

		uint32_t vertexIndex = 0;
		for (uint32_t i = 0; i < static_cast<uint32_t>(emitter._particles.size()); ++i) {
			auto& particle = emitter._particles[i];
			const static glm::vec3 dirs[4]{
				{-1.f, 1.f, 0.f},
				{1.f, 1.f, 0.f},
				{-1.f, -1.f, 0.f},
				{1.f, -1.f, 0.f},
			};
			float billboardSize = emitter.startParameters.initialUniformScale.Value();

			{
				const auto minUVs = emitter.diffuseTexture->GetUVMin();
				particleGPUData->GetVertex(vertexIndex + 0).uv = { minUVs.x	, minUVs.y };
				particleGPUData->GetVertex(vertexIndex + 1).uv = { 1.0f		, minUVs.y };
				particleGPUData->GetVertex(vertexIndex + 2).uv = { minUVs.x	, 1.0f };
				particleGPUData->GetVertex(vertexIndex + 3).uv = { 1.0f		, 1.0f };
			}

			for (uint32_t j = 0; j < 4; ++j) {
				ParticleVertex vert;
				glm::vec3 vertpos{};
				if (emitter.startParameters.billboardType == BillboardType::WorldspaceQuad) {
					vertpos = particle.position + dirs[j] * billboardSize;
				}
				else if (emitter.startParameters.billboardType == BillboardType::QuadLookTowardsCamera) {
					glm::vec3 CameraRight_worldspace{ cbMats.ViewMatrix[0][0], cbMats.ViewMatrix[1][0], cbMats.ViewMatrix[2][0] };
					glm::vec3 CameraUp_worldspace{ cbMats.ViewMatrix[0][1], cbMats.ViewMatrix[1][1], cbMats.ViewMatrix[2][1] };

					glm::vec3 rightOffset = CameraRight_worldspace * dirs[j].x * billboardSize;
					glm::vec3 upOffset = CameraUp_worldspace * dirs[j].y * billboardSize;
					vertpos = particle.position
						+ rightOffset
						+ upOffset;
				}
				auto& vtx = particleGPUData->GetVertex(vertexIndex);
				vtx.pos = vertpos;
				++vertexIndex;
			}
		}

		// Allocate the vertex shader constants from the command buffer
		ParticleVSData* constantsVs = static_cast<ParticleVSData*>(
			gfxc.allocateFromCommandBuffer(sizeof(ParticleVSData), sce::Gnm::kEmbeddedDataAlignment4));
		ASSERT_MSG(constantsVs != nullptr, "ConstantPerObjectBufferAlloc failed..");
		constantsVs->ModelMatrix = cbMats.ModelMatrix;
		constantsVs->ViewMatrix = cbMats.ViewMatrix;
		constantsVs->ProjectionMatrix = cbMats.ProjectionMatrix;
		constantsVs->MVPMatrix = cbMats.MVPMatrix;

		sce::Gnm::Buffer bufferVs;
		bufferVs.initAsConstantBuffer(constantsVs, sizeof(ParticleVSData));

		gfxc.setPrimitiveType(sce::Gnm::kPrimitiveTypeTriList);
		gfxc.setIndexSize(sce::Gnm::kIndexSize32);
		gfxc.setConstantBuffers(sce::Gnm::ShaderStage::kShaderStageVs, 0, 1, &bufferVs);
		gfxc.setSamplers(sce::Gnm::ShaderStage::kShaderStagePs, 0, 1, &sampler);
		gfxc.setTextures(sce::Gnm::ShaderStage::kShaderStagePs, 0, 1, emitter.diffuseTexture->GetGnmTexture());
		gfxc.setTextures(sce::Gnm::ShaderStage::kShaderStagePs, 1, 1, emitter.emissiveTexture->GetGnmTexture());
		gfxc.setVertexBuffers(sce::Gnm::kShaderStageVs, 0, pass.GetAttributeCount(), particleGPUData->GetAttributeBuffers(pass));
		gfxc.drawIndex(emitter._particles.size() * 6, particleGPUData->GetIdxBuffer());
	}
}
#endif