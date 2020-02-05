#pragma once
#if defined(PLATFORM_WINDOWS)
#include "rendering/Camera.h"
#elif defined(PLATFORM_PS)
#include "rendering/gnm/PS4RenderPass.h"
#endif

namespace gfx
{
#if defined(PLATFORM_WINDOWS)
//using RenderPass = DX12RenderPass;
	class IRenderPass
	{
	public:
		IRenderPass() = default;
		virtual ~IRenderPass() = default;

		IRenderPass(const IRenderPass& other) = delete;
		IRenderPass(IRenderPass&& other) noexcept = delete;
		IRenderPass& operator=(const IRenderPass& other) = delete;
		IRenderPass& operator=(IRenderPass&& other) noexcept = delete;

		virtual void ExecutePass(Camera& camera, void* perFrameResources) = 0;
	};
#elif defined(PLATFORM_PS)
using IRenderPass = PS4RenderPass;
#endif
}