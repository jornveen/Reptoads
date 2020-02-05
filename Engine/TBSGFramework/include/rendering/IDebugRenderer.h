#pragma once
#include <memory>

namespace gfx
{
	class IDebugRenderer
	{
	public:
		virtual ~IDebugRenderer() = default;
		// init the API.
		virtual void Initialize() = 0;
		// add a debug line
		virtual void AddLine() = 0;
		// render all debug lines
		virtual void Render() = 0;
		// clear all debug lines
		virtual void Clear() = 0;
	};
}