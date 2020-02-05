#pragma once
#include "Camera.h"
#include "ParticleEmitter.h"
#include "ResourceIds.h"

namespace gfx
{
	class IParticleRenderer
	{
	public:
		virtual ~IParticleRenderer() = default;

		void UpdateParticleEmitters(float totalTime, float deltaTime);

		ParticleEmitterId AddEmitter(const core::Transform& position, ParticleEmitter&& emitter);
		//! NOTE: It is fully safe to call this function with an id which doesn't exist, it will silently ignore the call.
		void RemoveEmitter(ParticleEmitterId id);
		bool HasEmitter(ParticleEmitterId id);

	protected:
		void SortParticles(Camera& camera);

	private:
		void InitializeParticleEmitter(ParticleEmitter& emitter, float totalTime, float deltaTime);
		void UpdateParticleEmitters(ParticleEmitter& emitter, float totalTime, float deltaTime);

	protected:
		ptl::flat_value_map<PunctualParticleEmitter, /*SparseHandle=*/ParticleEmitterId> particleEmitters;
		ptl::vector<ParticleEmitterId> uninitializedEmitters;

#pragma pack(push, 1)
		struct ParticleVertex
		{
			glm::vec3 Position;
			glm::vec2 TexCoord;
		};
#pragma pack(pop)
	};
}
