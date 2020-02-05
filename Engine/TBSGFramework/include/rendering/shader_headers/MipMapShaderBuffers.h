#pragma once
#include "ShaderDefines.h"

#define TEX_DESC_WIDTH_HEIGHT_EVEN 0
#define TEX_DESC_WIDTH_ODD_HEIGHT_EVEN 1
#define TEX_DESC_WIDTH_EVEN_HEIGHT_ODD 2
#define TEX_DESC_WIDTH_HEIGHT_ODD 3

SHADER_cbuffer MipMapData
{
	V2 dimensions;
	V2 texelSize;
	UINT srcMipLevel;
	UINT numMipLevels;
	UINT texDesc;
	bool isSrgb;
};