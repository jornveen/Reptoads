#pragma once
#ifdef PLATFORM_PS
#include "rendering/Shader.h"
#include "PS4Resource.h"
#include <gnmx/context.h>
#include "rendering/gnm/shaderLoader.h"
#include "ClassID.h"
#include "core/Assertion.h"


namespace sce {
	namespace Gnmx {
		class PsShader;
		class VsShader;
	}
}

namespace gfx
{
	struct FetchShader
	{
		sce::Gnm::FetchShaderInstancingMode *instancingData;
		void *memory{};
		uint32_t shaderModifier{};
	};

	struct ShaderData
	{
		sce::Gnmx::InputOffsetsCache inputOffsetsCache{};
	};

	class PS4Shader : public gfx::IShader, public gfx::PS4Resource
	{
	public:
		PS4Shader(ShaderType a_type, ptl::string a_path, ShaderId a_shaderId);
		static ptl::string CorrectPath(ptl::string a_path);
		const ShaderData& GetData() const { return m_data; }

	protected:
		ShaderData m_data;
	};

	class PS4VShader : public PS4Shader, public ClassID<PS4VShader>
	{
	public:
		PS4VShader(ptl::string a_path, ShaderId a_shaderId, ptl::MemoryResource& onionMemResource, ptl::MemoryResource& garlicMemResource);
		const sce::Gnmx::VsShader& GetShader() const { return *m_shader; }
		const FetchShader& GetFetchShader() const { return fetchShader; }
	private:
		sce::Gnmx::VsShader *m_shader;
		FetchShader fetchShader;
	};

	class PS4PShader : public PS4Shader,  public ClassID<PS4PShader>
	{
	public:
		PS4PShader(ptl::string a_path, ShaderId a_shaderId, ptl::MemoryResource& onionMemResource, ptl::MemoryResource& garlicMemResource);
		const sce::Gnmx::PsShader& GetShader() const { return *m_shader; }
	private:
		sce::Gnmx::PsShader* m_shader;
	};

	class PS4CShader : public PS4Shader,  public ClassID<PS4PShader>
	{
	public:
		PS4CShader(ptl::string a_path, ShaderId a_shaderId, ptl::MemoryResource& onionMemResource, ptl::MemoryResource& garlicMemResource);
		const sce::Gnmx::CsShader& GetShader() const { return *m_shader; }
	private:
		sce::Gnmx::CsShader* m_shader;
	};
}
#endif
