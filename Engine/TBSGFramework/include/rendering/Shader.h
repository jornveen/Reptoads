#pragma once
#include "memory/String.h"

namespace gfx
{
	enum ShaderType
	{
		//Pixel shader
		PS = 1 << 0,
		//Vertex shader
		VS = 1 << 1,
		//Compute shader
		CS = 1 << 2,
		// end bit, used to obtain the count
		SHADER_END_BIT = 1 << 3
	};

	class IShader
	{
	public:
		IShader(ShaderType type, ptl::string a_name) : type(type), name(a_name) {}
		virtual ~IShader() = default;

		const ShaderType type;
		const ptl::string name;
	};
}