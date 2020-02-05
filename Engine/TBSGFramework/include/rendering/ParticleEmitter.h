#pragma once
#include <glm/vec3.hpp>
#include "core/Assertion.h"
#include <random>
#include <glm/gtx/norm.hpp>
#include "Texture.h"
#include "gameplay/Transform.h"

namespace gfx
{
	struct ValueRange
	{
		float minDelay;
		float maxDelay;
		enum class DistributionType
		{
			Constant,
			Lineair,
			Exponentional
		}distribution;

		//Set @distribution type to Constant and set the value to @constantValue	
		void SetAsConstant(float constantValue);
		//Set @distribution type to Lineair and set the min and max values
		void SetAsLineair(float min, float max);

		float Value() const;
	};

	struct DirectionRange
	{
		enum class Direction
		{
			Any,
			Lineair,
		}direction;

		glm::vec3 min{-1, -1, -1};
		glm::vec3 max{1, 1, 1};

		// Set @direction to Any, so that Value will yield a vector in a unit sphere.
		void SetAsAny();

		void SetAsLineair(const glm::vec3& min, const glm::vec3& max);

		glm::vec3 Value() const;
	};

	struct VectorRange
	{
		ValueRange magnitudeRange;
		//Direction
		DirectionRange direction;

		//Call @direction.SetAsUnitSphere() and set ValueRange to a constant 1
		void SetAsUnitSphere();

		glm::vec3 Value() const;
	};

	//Defines whether the space is relative to the world, it's parent, or only it's parent's location and no rotation.
	enum class SimulationSpace
	{
		World = 0,
		Local = 1,
		LocalOnlyPosition = 2
	};

	enum class BillboardType
	{
		//The quad will be in worldspace and have a set rotation, thus you can move around it and see that it is a plane
		WorldspaceQuad,
		//The quad will always be rotated so it is perpendicular to the camera
		QuadLookTowardsCamera
	};

	struct ParticleEmitter
	{
		//Public parameters for artists to tweak
		struct StartParams
		{
			///Options for the emitter
			uint32_t maxParticleAmount = 50;
			bool useGravity = true;
			bool renderLikeSkybox = false;
			SimulationSpace simulationSpace = SimulationSpace::Local;
			BillboardType billboardType = BillboardType::QuadLookTowardsCamera;

			///Options Per particle
			ValueRange initialParticleAmount = { 10.0f, 10.0f, ValueRange::DistributionType::Constant };
			VectorRange initialVelocity = { ValueRange{0.0f, 0.0f, ValueRange::DistributionType::Constant }, DirectionRange::Direction::Any };
			ValueRange initialUniformScale = { 1.f, 1.f, ValueRange::DistributionType::Constant };
			ValueRange velocityChangeOverTime = { 0.0f, 0.0f, ValueRange::DistributionType::Constant };
			VectorRange spawnOffset = { ValueRange{-1.f, 1.f, ValueRange::DistributionType::Lineair}, DirectionRange::Direction::Any };
			//SpawnRate per second
			ValueRange particleSpawnRate{ 0.f, 0.f, ValueRange::DistributionType::Constant };
			ValueRange mass{ 1.0f, 1.0f, ValueRange::DistributionType::Constant };
			//If 0, then the particles are not killed
			ValueRange lifeTime{ 15.0f, 15.0f, ValueRange::DistributionType::Constant };
		}startParameters;

		//Currently only one texture is supported for simplicity
		gfx::Texture* diffuseTexture;
		gfx::Texture* emissiveTexture;

		glm::vec3 particleEmitterPosition;

		ParticleEmitter() : startParameters{}, diffuseTexture(nullptr), emissiveTexture(nullptr), particleEmitterPosition()
		{}

		ParticleEmitter(const StartParams& startParams, glm::vec3 position, Texture& diffuseTexture, Texture& emissiveTexture) 
			: startParameters(startParams), diffuseTexture(&diffuseTexture), emissiveTexture(&emissiveTexture), particleEmitterPosition(position)
		{}

		//Private fields for the renderer to keep it's state
	private: friend class IParticleRenderer; friend class DxParticleRenderer; friend class PS4ParticleRenderer;

		 struct ParticleInstance
		 {
			 glm::vec3 position;
			 float mass;
			 glm::vec3 velocity;
			 float birthTime;
		 };
		 std::vector<ParticleInstance> _particles{};
	};

	struct PunctualParticleEmitter
	{
		core::Transform transform;
		ParticleEmitter particleEmitter;
	};
}


//=====================================================
// Implementation:

namespace gfx
{
	inline void ValueRange::SetAsConstant(float constantValue)
	{
		distribution = DistributionType::Constant;
		minDelay = constantValue;
		maxDelay = constantValue;
	}

	inline void ValueRange::SetAsLineair(float min, float max)
	{
		distribution = DistributionType::Lineair;
		minDelay = min;
		maxDelay = max;
	}

	inline float ValueRange::Value() const
	{
		static std::mt19937 mersenneTwister{ static_cast<unsigned>(time(0)) };

		if (distribution == DistributionType::Constant) {
			ASSERT_MSG(std::abs(maxDelay - minDelay) < 0.001, "when distribution is constant, minDelay is returned, but maxDelay is different.");
			return minDelay;
		} else if (distribution == DistributionType::Lineair) {
            //ASSERT(minDelay < maxDelay);
            ASSERT(minDelay != maxDelay);
			std::uniform_real_distribution<float> lineairDistribution{ glm::min(minDelay, maxDelay), glm::max(maxDelay, minDelay) };
			return lineairDistribution(mersenneTwister);
		} else {
			// ReSharper disable once CppZeroConstantCanBeReplacedWithNullptr
			std::exponential_distribution<float> exponentialDistribution{ 0.5 };
			return exponentialDistribution(mersenneTwister);
		}
	}

	inline void DirectionRange::SetAsAny()
	{
		direction = Direction::Any;
	}

	inline void DirectionRange::SetAsLineair(const glm::vec3& min, const glm::vec3& max)
	{
		direction = Direction::Lineair;
		this->min = min;
		this->max = max;
	}

	inline glm::vec3 DirectionRange::Value() const
	{
		static std::mt19937 mersenneTwister{ static_cast<unsigned>(time(0)) };


		if (direction == Direction::Any) {
			std::uniform_real_distribution<float> lineairDistribution{ -1, 1 };
			glm::vec3 randomDirection{
				lineairDistribution(mersenneTwister),
				lineairDistribution(mersenneTwister),
				lineairDistribution(mersenneTwister)
			};
			randomDirection = glm::normalize(randomDirection);
			return randomDirection;
		} else if (direction == Direction::Lineair) {
            //ASSERT(min.x < max.x);
            //ASSERT(min.y < max.y);
            //ASSERT(min.z < max.z);
            //ASSERT(min.x != max.x);
            //ASSERT(min.y != max.y);
            //ASSERT(min.z != max.z);

			glm::vec3 randomDirection {
				std::uniform_real_distribution<float> {glm::min(min.x, max.x), glm::max(min.x, max.x)}(mersenneTwister),
				std::uniform_real_distribution<float> {glm::min(min.y, max.y), glm::max(min.y, max.y)}(mersenneTwister),
				std::uniform_real_distribution<float> {glm::min(min.z, max.z), glm::max(min.z, max.z)}(mersenneTwister)
			};
			randomDirection = glm::normalize(randomDirection);
			return randomDirection;
		}
		else {
			ASSERT_MSG(false, "Not implemented yet");
			return { 0, 0, 0 };
		}
	}

	inline void VectorRange::SetAsUnitSphere()
	{
		direction.SetAsAny();
		magnitudeRange.SetAsConstant(1.0f);
	}

	inline glm::vec3 VectorRange::Value() const
	{
		glm::vec3 randomDirection = direction.Value();
		randomDirection *= magnitudeRange.Value();

		return randomDirection;
	}
}
