#pragma once
#include "rendering/Shader.h"
#ifdef PLATFORM_PS
#include "ClassID.h"
#include "PS4Resource.h"
#include <unordered_map>
#include <gnmx/context.h>
#include "memory/Containers.h"

namespace sce {
	namespace Gnmx {
		class PsShader;
		class VsShader;
	}
}

class LinearAllocator;

namespace gfx
{
	struct Frame;
	class Camera;
	class PS4Renderer;

	class PS4RenderPass : public PS4Resource, public ClassID<PS4RenderPass>
	{
	public:
		struct ShaderData
		{
			explicit ShaderData(const bool a_useFetch) : useFetch(a_useFetch) {}
			const bool useFetch;
			sce::Gnmx::InputOffsetsCache inputOffsetsCache{};
			uint32_t shaderModifier{};
			void * fetchShaderMemory{};
		};

		/**
		 * \brief
		 * \param a_attributeBitMask 1 << 1 = position, 1 << 2 = color, 1 << 3 uv.
		 * \param a_alloc a reference to a garlic allocator that can be used to allocate the shaders.
		 * \param a_id the identifier of the RenderPass itself
		 * \param a_vsId The name of the vertex shader without the file extension.
		 * \param a_psId The name of the pixel shader without the file extension.
		 */ 
		PS4RenderPass(uint8_t a_attributeBitMask, ptl::vector<IShader*> a_shaders, RenderPassId a_id);
		virtual void ExecutePass(PS4Renderer& a_renderer, 
			Camera& a_camera, sce::Gnmx::GnmxGfxContext& a_gfxc, Frame& a_frame) = 0;
		uint8_t GetAttributeBitMask() const;
		uint8_t GetAttributeCount() const { return m_attributeCount; }

	protected:
		void PrepareForRendering(sce::Gnmx::GnmxGfxContext& a_gfxc) const;
		void SetAttributeBitmask(uint8_t a_bitmask);
		void SetShaderBitmask();

		ptl::vector<IShader*> m_shaders;
		ptl::unordered_map<size_t, ShaderData> m_shaderData;
		uint8_t m_attributeBitMask{};
		uint8_t m_attributeCount{};
		uint8_t m_shaderBitMask{};
		sce::Gnm::ActiveShaderStages m_activeShaderStages;
		sce::Gnm::DepthStencilControl m_depthStencilDesc;
		sce::Gnm::PrimitiveSetup m_primitiveDesc;
		sce::Gnm::BlendControl m_blendDesc;
	};
}
#endif