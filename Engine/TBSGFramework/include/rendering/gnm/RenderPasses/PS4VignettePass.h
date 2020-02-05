#pragma once
#ifdef PLATFORM_PS
#include "VignetteShaderBuffers.h"
#include "rendering/gnm/PS4RenderPass.h"
#include <common/allocator.h>

struct VignetteData;

namespace gfx
{
	class PS4VignettePass : public PS4RenderPass
	{
	public:
		PS4VignettePass(uint8_t a_attributeBitMask, const ptl::vector<IShader*>& a_shaders, float a_radiusZeroToOne, float a_smoothnessZeroToOne, glm::vec<2, size_t> a_screenSize, RenderPassId a_id);
		void ExecutePass(PS4Renderer& a_renderer, Camera& a_camera, sce::Gnmx::GnmxGfxContext& a_gfxc, Frame& a_frame) override;
		void SetRadiusZeroToOne(float a_radius) const {m_vignetteData->vignetteRadius = glm::clamp<float>(a_radius, 0, 1);}
		void SetSoftneZeroToOne(float a_softness) const { m_vignetteData->vignetteSoftness = glm::clamp<float>(a_softness, 0, 1); }
	private:
		LinearAllocator m_alloc;
		VignetteData* m_vignetteData{};
		sce::Gnm::Buffer m_vignetteBuffer{};
		sce::Gnm::Buffer m_vtxBuffers[2]{};
	};
}
#endif