#pragma once
#ifdef PLATFORM_PS
#include "GenericMultiMap.h"
#include "PS4Renderable.h"
#include <rendering/IRenderer.h>
#include <gnmx/context.h>
#include <map>
#include <common/allocator.h>
#include "Frame.h"

#define MB(x) x * 1024 * 1024

struct LightDataPs;

namespace gfx
{
	class Camera;
	struct Vertex;
	class TextureLoader;
	class PS4ResourceManager;

	enum RenderContextState
	{
		renderContextFree = 0,
		renderContextInUse,
	};

	class PS4Renderer final : public IRenderer, public GenericMultiMap<PS4Resource>
	{
	public:
		PS4Renderer(size_t a_texAllocByteSize = MB(512));
		~PS4Renderer();
		// create a window and init the API.
		void Initialize() override;
		// Render all meshes.
		void RenderWorld(Camera& camera) override;
		// Render all UI elements.
		void RenderUI(Camera& camera) override;
		void Present() override;

		// Clear all data.
		void Shutdown() override;
		sce::Gnm::GpuMode GetGPUMode() const;
		void InitializeGBuffers();
        glm::vec<2, size_t> GetScreenSize() const override { return m_screenSize; }
		ptl::map<uint32_t, ptl::vector<PS4Renderable*>>& GetTexCache() { return m_texCache; }
		Frame& GetCurrentFrame() const { if (!m_currentFrame) ASSERT(false); return *m_currentFrame; }
		size_t GetMaxBlurMipLevels() const { return m_blurMipLevels; }
		sce::Gnmx::GnmxGfxContext& GetGfxContext() const { if (!m_gfxContext) ASSERT(false); return *m_gfxContext; }
		const sce::Gnm::Sampler& GetSampler() const { return m_sampler; }
		
		RenderableId AddRenderable(core::Transform* transform, MeshId meshId, MaterialId materialId) override;
		bool ContainsRenderable(RenderableId renderableId) override;
		void RemoveRenderable(RenderableId renderableId) override;
		RenderPassId AddRenderPass(IRenderPass* renderPass) override;
		bool ContainsRenderPass(RenderPassId renderPassId) override;
		void RemoveRenderPass(RenderPassId renderPassId) override;

	private:
		ptl::vector<RenderPassId> m_activeGraphicsPasses;
		//ptl::unordered_map<PS4RenderPass*, ptl::vector<ptl::string>> m_renderables;
		ptl::map<uint32_t, ptl::vector<PS4Renderable*>> m_texCache;
		void InitAllocators(size_t a_onionMemSize, size_t a_garlicMemSize);
		void InitHandles();
		sce::Gnmx::Toolkit::Allocators m_toolkitAlloc;
		sce::Gnm::GpuMode m_gpuMode; // neo or base
		LinearAllocator m_onionAllocator;
		LinearAllocator m_garlicAllocator;
		int m_videoOutHandle;
		_SceKernelEqueue* m_eventQueue;
		//LightDataPs* m_lightDataGpu;
		glm::vec<2,size_t> m_screenSize;
		sce::Gnmx::GnmxGfxContext* m_gfxContext;

		const glm::vec4 m_clearColor = glm::vec4(59. / 256., 110. / 256., 165. / 256., 1);
		const size_t m_onionMemorySize = MB(16);
		const size_t m_garlicMemorySize = MB(1024) + MB(256);
		//const sce::Gnm::ZFormat m_depthFormat = sce::Gnm::kZFormat32Float;
		//const sce::Gnm::StencilFormat m_stencilFormat = sce::Gnm::kStencil8;
		//sce::Gnm::DepthRenderTarget m_depthTarget;
		const bool m_htileEnabled = true;
		std::map<sce::Gnm::Texture*, std::function<void()>> m_clearTextureFunctions;

		// constant update engine ring entries
		const size_t m_cueRingEntries = 64;
		// draw command buffer size
		const size_t m_dcbSizeInBytes = MB(2);
		// constants command buffer size
		const size_t m_ccbSizeInBytes = 256 * 1024;

		const static uint32_t m_displayBufferCount = 3;

		//DisplayBuffer m_displayBuffers [3]; // CHANGE TO m_displayBufferCount!!
		//DisplayBuffer* m_backBuffer;
		size_t m_backBufferIndex = 0;

		const static uint32_t m_renderContextCount = 2;
		//RenderContext* m_renderContext;
		//RenderContext m_renderContexts [2]; // CHANGE TO m_renderContextCount!!
		size_t m_renderContextIndex = 0;

		//sce::Gnm::Texture m_texture;
		sce::Gnm::Sampler m_sampler;

		const size_t m_blurMipLevels = 1; // set in constructor
		void CreateCommandBuffer(sce::Gnmx::GnmxGfxContext* a_ptr);
		void CreateTexture(sce::Gnm::Texture& a_tex, sce::Gnm::DataFormat a_dataFormat, sce::Gnm::ResourceMemoryType a_resourceMemoryType, size_t a_mipLevels = 1, float a_resolutionScale = 1);
		void CreateRenderTarget(sce::Gnm::RenderTarget& a_rt, sce::Gnm::DataFormat a_dataFormat);
		void CreateRenderTargetAndTexture(sce::Gnm::RenderTarget& a_rt, sce::Gnm::Texture& a_tex, sce::Gnm::DataFormat a_dataFormat, sce::Gnm::ResourceMemoryType a_resourceMemoryType);
		void CreateDepthTargetAndTexture(sce::Gnm::DepthRenderTarget& a_dt, sce::Gnm::Texture& a_tex, sce::Gnm::DataFormat a_depthFormat, sce::Gnm::StencilFormat a_stencilFormat, sce::Gnm::ResourceMemoryType a_resourceMemoryType);
		Frame m_frames[m_displayBufferCount];
		Frame* m_currentFrame;
	};
}
#endif