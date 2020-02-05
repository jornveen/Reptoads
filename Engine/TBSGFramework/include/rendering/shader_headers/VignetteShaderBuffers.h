#ifdef __cplusplus
#pragma once
#endif

#include "ShaderDefines.h"

SHADER_cbuffer VignetteData
{
	float vignetteRadius; // (0-1)
	float vignetteSoftness; // (0-1)
	float aspectRatio;
};

#ifndef __cplusplus
struct VS_OUTPUT
{
	float4 Position : SEM_POSITION;
	float2 UV : TEXCOORD0;
};
#endif
