#pragma once
#ifdef PLATFORM_PS
#include <gnmx/context.h>
#include <cstdint>

enum ComputeJobState
{
	computeJobInProgress = 0,
	computeJobFinished,
};

namespace gfx
{
	struct Frame
	{
		sce::Gnmx::GnmxGfxContext gfxContext;					// the command buffer for the current frame
		sce::Gnm::RenderTarget backBufferTarget;				// the back buffer - the final output of the program (USED AS DISPLAY BUFFER)
		sce::Gnm::RenderTarget emissiveTarget;					// the back buffer - the final output of the program (USED AS DISPLAY BUFFER)
		sce::Gnm::Texture backBufferTexture;
		sce::Gnm::Texture emissiveTexture;
		sce::Gnm::Texture blurInputTexture;
		sce::Gnm::Texture HorizontalBlurResultTexture;
		volatile uint32_t *contextLabel;						// [VOLATILE] DO NOT OPTIMIZE AWAY, changed by GPU

		sce::Gnm::DepthRenderTarget depthTarget;
		sce::Gnm::Texture depthTargetTexture;
	};
}
#endif