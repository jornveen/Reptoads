#pragma once
#include "ResourceIds.h"
#include "memory/smart_ptr.h"
#include "shader_headers/DefaultShaderBuffers.h"
#include "memory/default_memresource.h"

#if defined(PLATFORM_WINDOWS)
#elif defined(PLATFORM_PS)
#include "rendering/gnm/PS4Material.h"
#endif

namespace gfx
{
	enum class BlendMode {
		/// Fully opaque objects. (alpha==1.0)
		Opaque,
		/// Objects with only fully transparency (alpha==1.0 OR alpha==0.0)
		Masked,
		/// Objects with semi transparency (0 <= alpha <= 1)
		Blending,
	};
	

	class Material
	{
	public:
		Material(BlendMode blendMode, float alphaCutoff, glm::vec4 diffuseFactor, TextureId diffuseTextureId, 
			glm::vec3 emissiveFactor, TextureId emissiveTextureId)
			: blendMode(blendMode), diffuseTextureId(diffuseTextureId), emissiveTextureId(emissiveTextureId), 
			heapShaderMaterial{new ShaderMaterial{}}
		{
			heapShaderMaterial->materialDiffuseColor = diffuseFactor;
			heapShaderMaterial->materialEmissiveColor = emissiveFactor;
			heapShaderMaterial->alphaCutoff = alphaCutoff;
		}

		const BlendMode blendMode;
		const TextureId diffuseTextureId;
		const TextureId emissiveTextureId;
		ptl::string name;


		// ReSharper disable once CppMemberFunctionMayBeConst
		
		ShaderMaterial& GetShaderMaterialData() { return *heapShaderMaterial; }

	private:
		ptl::unique_ptr<ShaderMaterial> heapShaderMaterial;
	};
}