#pragma once
#include "memory/Containers.h"

#ifdef PLATFORM_WINDOWS
#include <rendering/Lights.h>
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include "core/Defines.h"
#include <vector>


namespace gfx
{
	struct PointLightData
	{
		// ReSharper disable once CppPossiblyUninitializedMember
		PointLightData() // NOLINT(cppcoreguidelines-pro-type-member-init, hicpp-member-init)
			: PositionWS(0.0f, 0.0f, 0.0f, 1.0f)
			, PositionVS(0.0f, 0.0f, 0.0f, 1.0f)
			, Color(1.0f, 1.0f, 1.0f, 1.0f)
			, ConstantAttenuation(1.0f)
			, LinearAttenuation(0.0f)
			, QuadraticAttenuation(0.0f)
		{
		}

		glm::vec4    PositionWS; // Light position in world space.
		//----------------------------------- (16 byte boundary)
		glm::vec4    PositionVS; // Light position in view space.
		//----------------------------------- (16 byte boundary)
		glm::vec4    Color;
		//----------------------------------- (16 byte boundary)
		float       ConstantAttenuation;
		float       LinearAttenuation;
		float       QuadraticAttenuation;
		// Add some padding to align to 16 bytes.
		float       _Padding;
		//----------------------------------- (16 byte boundary)
		// Total:                              16 * 4 = 64 bytes
	};

	class PointLight final : public IPointLight
	{
		friend class DX12Renderer;

	public:
		PointLight(std::vector<PointLightData>& pointLights, uint32_t pointLightIndex) : pointLights_(&pointLights), pointLightIndex_(pointLightIndex)
		{
			SetAttenuation(LightDist::d3250);
		}

		void SetPosition(const glm::vec3& a_position) override { GetData().PositionWS = glm::vec4(a_position.x, a_position.y, a_position.z, 1.0f); };
		void SetColor(const glm::vec3& a_color) override { GetData().Color = glm::vec4(a_color.x, a_color.y, a_color.z, 1.0f); }
		void SetConstantAttenuation(const float a_ca) override { GetData().ConstantAttenuation = a_ca; }
		void SetLinearAttenuation(const float a_la) override { GetData().LinearAttenuation = a_la; }
		void SetQuadraticAttenuation(const float a_qa) override { GetData().QuadraticAttenuation = a_qa; }

		float GetConstantAttenuation() const override { return GetData().ConstantAttenuation; }
		float GetLinearAttenuation() const override { return GetData().LinearAttenuation; }
		float GetQuadraticAttenuation() const override { return GetData().QuadraticAttenuation; }

		const PointLightData& GetData() const { return (*pointLights_)[pointLightIndex_]; }
		PointLightData& GetData() { return (*pointLights_)[pointLightIndex_]; }

		std::vector<PointLightData>& GetDataVector() { return *pointLights_; }
		const std::vector<PointLightData>& GetDataVector() const { return *pointLights_; }
		uint32_t GetPointLightIndex() const { return pointLightIndex_; }

	private:
		void SetPointLightIndex(uint32_t newIndex) { pointLightIndex_ = newIndex; }


		std::vector<PointLightData>* pointLights_;
		uint32_t pointLightIndex_;
	};

	struct SpotLightData
	{
		SpotLightData()
			: PositionWS(0.0f, 0.0f, 0.0f, 1.0f)
			, PositionVS(0.0f, 0.0f, 0.0f, 1.0f)
			, DirectionWS(0.0f, 0.0f, 1.0f, 0.0f)
			, DirectionVS(0.0f, 0.0f, 1.0f, 0.0f)
			, Color(1.0f, 1.0f, 1.0f, 1.0f)
			, SpotAngle(glm::half_pi<float>())
			, ConstantAttenuation(1.0f)
			, LinearAttenuation(0.0f)
			, QuadraticAttenuation(0.0f)
		{}

		glm::vec4    PositionWS; // Light position in world space.
		//----------------------------------- (16 byte boundary)
		glm::vec4    PositionVS; // Light position in view space.
		//----------------------------------- (16 byte boundary)
		glm::vec4    DirectionWS; // Light direction in world space.
		//----------------------------------- (16 byte boundary)
		glm::vec4    DirectionVS; // Light direction in view space.
		//----------------------------------- (16 byte boundary)
		glm::vec4    Color;
		//----------------------------------- (16 byte boundary)
		float       SpotAngle;
		float       ConstantAttenuation;
		float       LinearAttenuation;
		float       QuadraticAttenuation;
		//----------------------------------- (16 byte boundary)
		// Total:                              16 * 6 = 96 bytes
	};

	class SpotLight final : public ISpotLight
	{
		friend class DX12Renderer;

	public:
		SpotLight(std::vector<SpotLightData>& spotLights, uint32_t spotLightIndex) : spotLights_(&spotLights), spotLightIndex_(spotLightIndex) {}

		void SetPosition(const glm::vec3& a_position) override { GetData().PositionWS = glm::vec4(a_position.x, a_position.y, a_position.z, 1.0f); };
		void SetColor(const glm::vec3& a_color) override { GetData().Color = glm::vec4(a_color.x, a_color.y, a_color.z, 1.0f); }
		
		void SetConstantAttenuation(const float a_ca) override { GetData().ConstantAttenuation = a_ca; }
		void SetLinearAttenuation(const float a_la) override { GetData().LinearAttenuation = a_la; }
		void SetQuadraticAttenuation(const float a_qa) override { GetData().QuadraticAttenuation = a_qa; }

		float GetConstantAttenuation() const override { return GetData().ConstantAttenuation; }
		float GetLinearAttenuation() const override { return GetData().LinearAttenuation; }
		float GetQuadraticAttenuation() const override { return GetData().QuadraticAttenuation; }

		void SetDirection(const glm::vec3& a_position) override { GetData().DirectionWS = glm::vec4(a_position, 1.0f); }
		void SetInnerCutOffAngle(float a_angleInDegrees) override { UNUSED(a_angleInDegrees); } //TODO: Figure out how both PS4 and DX12 systems work
		void SetOuterCutOffAngle(float a_angleInDegrees) override { UNUSED(a_angleInDegrees); }; //TODO: Figure out how both PS4 and DX12 systems work

		const SpotLightData& GetData() const { return (*spotLights_)[spotLightIndex_]; }
		SpotLightData& GetData() { return (*spotLights_)[spotLightIndex_]; }

		std::vector<SpotLightData>& GetDataVector() { return *spotLights_; }
		const std::vector<SpotLightData>& GetDataVector() const { return *spotLights_; }
		uint32_t GetSpotLightIndex() const { return spotLightIndex_; }

	private:
		void SetSpotLightIndex(uint32_t newIndex) { spotLightIndex_ = newIndex; }

		std::vector<SpotLightData>* spotLights_;
		uint32_t spotLightIndex_;
	};

	struct DirectionalLightData
	{
		DirectionalLightData()
			: DirectionWS(0.0f, 0.0f, 1.0f, 0.0f)
			, DirectionVS(0.0f, 0.0f, 1.0f, 0.0f)
			, Color(1.0f, 1.0f, 1.0f, 1.0f)
		{}

		glm::vec4    DirectionWS; // Light direction in world space.
		//----------------------------------- (16 byte boundary)
		glm::vec4    DirectionVS; // Light direction in view space.
		//----------------------------------- (16 byte boundary)
		glm::vec4	 Color;
		//----------------------------------- (16 byte boundary)
		// Total:                              16 * 3 = 48 bytes 
	};

	class DirectionalLight : public IDirectionalLight
	{
		friend class DX12Renderer;

	public:
		DirectionalLight(std::vector<DirectionalLightData>& directionalLights, uint32_t directionalLightIndex)
			: directionalLights_(&directionalLights), directionalLightIndex_(directionalLightIndex)
		{}

		void SetDirection(const glm::vec3& a_direction) override { GetData().DirectionWS = glm::vec4(a_direction, 1.0f); }
		void SetColor(const glm::vec3& a_color) override { GetData().Color = glm::vec4(a_color, 1.0f); }
		void SetStrength(float a_strength) override { UNUSED(a_strength); } //TODO: Figure out how to make strength the same as ps4

		const DirectionalLightData& GetData() const { return (*directionalLights_)[directionalLightIndex_]; }
		DirectionalLightData& GetData() { return (*directionalLights_)[directionalLightIndex_]; }

		std::vector<DirectionalLightData>& GetDataVector() { return *directionalLights_; }
		const std::vector<DirectionalLightData>& GetDataVector() const { return *directionalLights_; }
		uint32_t GetDirectionalLightIndex() const { return directionalLightIndex_; }

	private:
		void SetDirectionalLightIndex(uint32_t newIndex) { directionalLightIndex_ = newIndex; }

	private:
		std::vector<DirectionalLightData>* directionalLights_;
		uint32_t directionalLightIndex_;
	};
}
#endif
