#ifdef __cplusplus
#pragma once
#endif

#include "ShaderDefines.h"

SHADER_cbuffer BlurData
{
	// x = blur horizontally (1 OR 0)
	// y = mip-map level
	// z = textureDimensions.x
	// w = textureDimensions.y
	V2 dimensions;
	bool blurHorizontal;
	UINT mipLevel;
};