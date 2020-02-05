#pragma once
#if defined(PLATFORM_WINDOWS)
#include "dx/Texture.h"
// #include <rendering/dx/DX12Texture.h>
#elif defined(PLATFORM_PS)
#include <rendering/gnm/PS4Texture.h>
#endif


namespace gfx
{
#if defined(PLATFORM_WINDOWS)
	using Texture = ::Texture;
#elif defined(PLATFORM_PS)
	using Texture = PS4Texture;
#endif

#if defined(PLATFORM_WINDOWS)
	using TextureHandle = ::Texture*;
#elif defined(PLATFORM_PS)
	using TextureHandle = PS4Texture*;
#endif
}
