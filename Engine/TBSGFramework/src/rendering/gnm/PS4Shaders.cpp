#ifdef PLATFORM_PS
#include "rendering/gnm/PS4Shaders.h"
#include <embedded_shader.h>
#include "core/Assertion.h"
#include <common/allocator.h>

gfx::PS4Shader::PS4Shader(ShaderType a_type, ptl::string a_path, ShaderId a_shaderId) 
	: IShader(a_type, a_path)
	, PS4Resource(a_shaderId)
{
}

ptl::string gfx::PS4Shader::CorrectPath(ptl::string a_path)
{
	ptl::string fileName(a_path);
	size_t pos = 0;
	while ((pos = fileName.find('/')) != ptl::string::npos)
	{
		fileName.erase(0, pos + 1);
		fileName = fileName.substr(0, fileName.size());
	}

	if ((pos = fileName.find('.')) != ptl::string::npos)
		fileName.erase(pos, fileName.size() - pos);

	fileName = ptl::string("/hostapp/data/Shaders/") + fileName;
	fileName.append(".sb");
	return fileName;
}

gfx::PS4VShader::PS4VShader(ptl::string a_path, ShaderId a_shaderId, ptl::MemoryResource& onionMemResource, ptl::MemoryResource& garlicMemResource)
	: PS4Shader(ShaderType::VS, a_path, a_shaderId)
{
	m_shader = loadShaderFromFile<sce::Gnmx::VsShader>(a_path, onionMemResource, garlicMemResource);
	ASSERT_MSG(m_shader, (ptl::string("Cannot load shader from path ") + a_path));

	fetchShader.memory = garlicMemResource.allocate(
		sce::Gnmx::computeVsFetchShaderSize(m_shader),
		sce::Gnm::kAlignmentOfFetchShaderInBytes);
	ASSERT_MSG(fetchShader.memory != nullptr, "Cannot allocate the fetch shader memory");

	sce::Gnmx::generateVsFetchShader(fetchShader.memory, &fetchShader.shaderModifier, m_shader, fetchShader.instancingData, fetchShader.instancingData != nullptr ? 256 : 0);

	sce::Gnmx::generateInputOffsetsCache(&m_data.inputOffsetsCache, sce::Gnm::ShaderStage::kShaderStageVs, m_shader);
}

gfx::PS4PShader::PS4PShader(ptl::string a_path, ShaderId a_shaderId, ptl::MemoryResource& onionMemResource, ptl::MemoryResource& garlicMemResource)
	: PS4Shader(ShaderType::PS, a_path, a_shaderId)
{
	m_shader = loadShaderFromFile<sce::Gnmx::PsShader>(a_path, onionMemResource, garlicMemResource);
	ASSERT_MSG(m_shader, (ptl::string("Cannot load shader from path ") + a_path));

	sce::Gnmx::generateInputOffsetsCache(&m_data.inputOffsetsCache, sce::Gnm::ShaderStage::kShaderStagePs, m_shader);
}

gfx::PS4CShader::PS4CShader(ptl::string a_path, ShaderId a_shaderId, ptl::MemoryResource& onionMemResource, ptl::MemoryResource& garlicMemResource)
	: PS4Shader(ShaderType::CS, a_path, a_shaderId)
{
	m_shader = loadShaderFromFile<sce::Gnmx::CsShader>(a_path, onionMemResource, garlicMemResource);
	ASSERT_MSG(m_shader, (ptl::string("Cannot load shader from path ") + a_path));

	sce::Gnmx::generateInputOffsetsCache(&m_data.inputOffsetsCache, sce::Gnm::ShaderStage::kShaderStageCs, m_shader);
}
#endif
