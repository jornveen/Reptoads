#pragma once
#ifdef PLATFORM_PS
#include <rendering/IDebugRenderer.h>

namespace gfx
{
	class PS4Renderer;

	class PS4DebugRenderer : public IDebugRenderer
	{
		//PS4Renderer* m_renderer; #eliminate warning  "member not being used"

	public:
		PS4DebugRenderer(PS4Renderer* renderer);
		// init the API.
		void Initialize() override;
		// add a debug line
		void AddLine() override;
		// render all debug lines
		void Render() override;
		// clear all debug lines
		void Clear() override;
	};
}
#endif