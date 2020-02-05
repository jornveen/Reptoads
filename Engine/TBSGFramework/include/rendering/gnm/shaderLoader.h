#pragma once
#include "core/Assertion.h"
#include <gnmx/shader_parser.h>
#include <gnmx/shaderbinary.h>
#include <gnmx/context.h>
#include <gnmx/fetchshaderhelper.h>
#ifdef PLATFORM_PS
bool acquireFileContents(void*& pointer, uint32_t& bytes, const ptl::string& filename);
bool readFileContents(void *data, const size_t size, FILE *fp);

template<typename TShader> 
inline TShader* loadShaderFromFile(const ptl::string& path, ptl::MemoryResource& onionMemResource, ptl::MemoryResource& garlicMemResource)
{
	void* binary;
	uint32_t size;

	if (!acquireFileContents(binary, size, path.c_str()))
	{
		ASSERT(false);
	}

	sce::Gnmx::ShaderInfo shaderInfo;
	sce::Gnmx::parseShader(&shaderInfo, binary);

	const auto* shaderStruct = static_cast<const TShader*>(shaderInfo.m_shaderStruct);

	TShader *shader = static_cast<TShader*>(onionMemResource.allocate(shaderStruct->computeSize(), sce::Gnm::kAlignmentOfBufferInBytes));
	void *shaderBinary = garlicMemResource.allocate(shaderInfo.m_gpuShaderCodeSize, sce::Gnm::kAlignmentOfShaderInBytes);

	memcpy(shaderBinary, shaderInfo.m_gpuShaderCode, shaderInfo.m_gpuShaderCodeSize);
	memcpy(shader, shaderStruct, shaderStruct->computeSize());

	shader->patchShaderGpuAddress(shaderBinary);
	free(binary);

	return shader;
}
#endif