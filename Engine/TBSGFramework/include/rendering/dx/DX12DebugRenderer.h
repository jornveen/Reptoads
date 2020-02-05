#pragma once
#include <rendering/IDebugRenderer.h>

namespace gfx
{
	class DX12Renderer;

	class DX12DebugRenderer : public IDebugRenderer
	{
		DX12Renderer* m_renderer;

	public:
		DX12DebugRenderer(DX12Renderer* renderer);
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
