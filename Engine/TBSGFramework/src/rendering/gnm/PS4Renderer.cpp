#ifdef PLATFORM_PS
#include "rendering/gnm/PS4ResourceManager.h"
#include "rendering/gnm/PS4Renderer.h"
#include "rendering/gnm/PS4RenderPass.h"
#include "rendering/gnm/PS4Mesh.h"
#include "rendering/gnm/PS4DebugRenderer.h"
#include "rendering/gnm/PS4Helpers.h"
#include "rendering/gnm/PS4UIRenderer.h"
#include <gnmx/context.h>
#include <kernel/equeue.h>
#include <video_out.h>
#include <toolkit.h>
#include "core/Assertion.h"
namespace gfx
{
    // For simplicity reasons the sample uses a single GfxContext for each
    // frame. Implementing more complex schemes where multipleGfxContext-s
    // are submitted in each frame is possible as well

    PS4Renderer::PS4Renderer(size_t a_texAllocByteSize)
        : IRenderer(std::make_unique<PS4DebugRenderer>(this), std::make_unique<PS4UIRenderer>(*this), std::make_unique<PS4ResourceManager>(*this))
        , GenericMultiMap<PS4Resource>([&](PS4Resource* x) { return Identifier(x->GetId()._id); })
        , m_screenSize(1920, 1080), m_gfxContext(nullptr)
		, m_blurMipLevels(8)
    {
    }

    PS4Renderer::~PS4Renderer()
    {
    }

    void PS4Renderer::Initialize()
    {
        InitAllocators(m_onionMemorySize, m_garlicMemorySize);
        InitHandles();
        InitializeGBuffers();
        m_uiRenderer->Initialize(*m_resourceManager);
        sce::Gnm::ClipControl control;
        control.init();
        control.setClipSpace(sce::Gnm::ClipControlClipSpace::kClipControlClipSpaceDX);
    }
	
    void gfx::PS4Renderer::InitAllocators(const size_t a_onionMemSize, const size_t a_garlicMemSize)
    {
        // Initialize the WB_ONION memory allocator
        auto ret = m_onionAllocator.initialize(
            a_onionMemSize, SCE_KERNEL_WB_ONION,
            SCE_KERNEL_PROT_CPU_RW | SCE_KERNEL_PROT_GPU_ALL);
        ThrowIfFailed(ret != SCE_OK, "Onion memory allocation failed...");

        // Initialize the WC_GARLIC memory allocator
        // NOTE: CPU reads from GARLIC write-combined memory have a very low
        // bandwidth so they are disabled for safety
        ret = m_garlicAllocator.initialize(
            a_garlicMemSize,
            SCE_KERNEL_WC_GARLIC,
            SCE_KERNEL_PROT_CPU_WRITE | SCE_KERNEL_PROT_GPU_ALL);
        ThrowIfFailed(ret != SCE_OK, "Garlic memory allocation failed...");

        // Initialize the Toolkit module
        m_onionAllocator.getIAllocator(m_toolkitAlloc.m_onion);
        m_garlicAllocator.getIAllocator(m_toolkitAlloc.m_garlic);
        initializeWithAllocators(&m_toolkitAlloc);
    }

    void PS4Renderer::InitHandles()
    {
        m_gpuMode = sce::Gnm::getGpuMode();

        // Open the video output port
        m_videoOutHandle = sceVideoOutOpen(0, SCE_VIDEO_OUT_BUS_TYPE_MAIN, 0, NULL);
        ThrowIfFailed(m_videoOutHandle < 0, "sceVideoOutOpen failed");

        // Initialize the flip rate: 0: 60Hz, 1: 30Hz or 2: 20Hz
        int ret = sceVideoOutSetFlipRate(m_videoOutHandle, 0);
        ThrowIfFailed(ret != SCE_OK, "sceVideoOutSetFlipRate failed");

        // Create the event queue for used to synchronize with end-of-pipe interrupts
        SceKernelEqueue eopEventQueue;
        ret = sceKernelCreateEqueue(&eopEventQueue, "EOP QUEUE");
        ThrowIfFailed(ret != SCE_OK, "sceKernelCreateEqueue failed");

        // Register for the end-of-pipe events
        ret = sce::Gnm::addEqEvent(eopEventQueue, sce::Gnm::kEqEventGfxEop, NULL);
        ThrowIfFailed(ret != SCE_OK, "Gnm::addEqEvent failed");

        // set the sampler data
        m_sampler.init();
        m_sampler.setMipFilterMode(sce::Gnm::kMipFilterModeNone);
        m_sampler.setXyFilterMode(sce::Gnm::kFilterModeBilinear, sce::Gnm::kFilterModePoint);
    }

    void PS4Renderer::RenderWorld(Camera& camera)
    {
        //sce::Gnmx::GnmxGfxContext& gfxc = m_renderContext->gfxContext;
        Frame& frame = m_frames[m_backBufferIndex];
		m_currentFrame = &frame;
        // update the variable for the UIRenderer without
        // having to change the shared API
        m_gfxContext = &frame.gfxContext;
        auto& gfxc = frame.gfxContext;
        // Wait until the context label has been written to make sure that the
        // GPU finished parsing the command buffers before overwriting them

        while (frame.contextLabel[0] != renderContextFree)
        {
            // Wait for the EOP event
            SceKernelEvent eopEvent;
            int count;
            int ret = sceKernelWaitEqueue(m_eventQueue, &eopEvent, 1, &count, NULL);
            ThrowIfFailed(ret != SCE_OK, "sceKernelWaitEqueue failed");
        }
        // Reset the flip GPU label
        frame.contextLabel[0] = renderContextInUse;

        // Reset the graphical context and initialize the hardware state
        gfxc.reset();
        gfxc.initializeDefaultHardwareState();

        // non gfxc compute processing starts here

        // The waitUntilSafeForRendering stalls the GPU until the scan-out
        // operations on the current display buffer have been completed.
        // This command is not blocking for the CPU.
        // This command should be used right before writing the display buffer.
        gfxc.waitUntilSafeForRendering(m_videoOutHandle, m_backBufferIndex);

        // Setup the viewport to match the entire screen. (left, top, width, height, zscale, zoffset)
        // The z-scale and z-offset values are used to specify the transformation
        // from clip-space to screen-space
        gfxc.setupScreenViewport(0, 0, frame.backBufferTarget.getWidth(), frame.backBufferTarget.getHeight(), 0.5f, 0.5f);

        // Clear the color and the depth target
        sce::Gnmx::Toolkit::SurfaceUtil::clearRenderTarget(gfxc, &frame.backBufferTarget, Vector4(m_clearColor.x, m_clearColor.y, m_clearColor.z, m_clearColor.w)); //Vector4(m_clearColor.x, m_clearColor.y, m_clearColor.z, m_clearColor.w)
        sce::Gnmx::Toolkit::SurfaceUtil::clearRenderTarget(gfxc, &frame.emissiveTarget, Vector4(0));
        sce::Gnmx::Toolkit::SurfaceUtil::clearDepthTarget(gfxc, &frame.depthTarget, 1.f);
		//m_clearTextureFunctions.at(&frame.emissiveTexture)();

        for (RenderPassId pass : m_activeGraphicsPasses)
        {
			Get<PS4RenderPass>(pass).ExecutePass(*this, camera, gfxc, frame);
        }
    }

	RenderPassId PS4Renderer::AddRenderPass(IRenderPass* a_renderPass)
	{
		RenderPassId id = RenderPassId(a_renderPass->GetId()._id);
		ASSERT(!Contains<PS4RenderPass>(a_renderPass->GetId()));
		Add<PS4RenderPass>(a_renderPass);
		m_activeGraphicsPasses.push_back(id);
		return id;
	}

	bool PS4Renderer::ContainsRenderPass(RenderPassId a_renderPass)
	{
		return Contains<PS4RenderPass>(a_renderPass);
	}

	void PS4Renderer::RemoveRenderPass(RenderPassId a_renderPass)
	{
		ASSERT(Contains<PS4RenderPass>(a_renderPass));
		Remove<PS4RenderPass>(a_renderPass);
	}

    void PS4Renderer::RenderUI(Camera& a_camera)
    {
		m_uiRenderer->Render(a_camera);
    }

    void PS4Renderer::Present()
    {
		// Submit the command buffers, request a flip of the display buffer and
	   // write the GPU label that determines the render context state (free)
	   // and trigger a software interrupt to signal the EOP event queue
	   // NOTE: for this basic sample we are submitting a single GfxContext
	   // per frame. Submitting multiple GfxContext-s per frame is allowed.
	   // Multiple contexts are processed in order, i.e.: they start in
	   // submission order and end in submission order.
		int ret = m_currentFrame->gfxContext.submitAndFlipWithEopInterrupt(
			m_videoOutHandle,
			m_backBufferIndex,
			SCE_VIDEO_OUT_FLIP_MODE_VSYNC,
			0,
			sce::Gnm::kEopFlushCbDbCaches,
			const_cast<uint32_t*>(m_currentFrame->contextLabel),
			renderContextFree,
			sce::Gnm::kCacheActionWriteBackAndInvalidateL1andL2);
		ASSERT(ret == sce::Gnm::kSubmissionSuccess)

			// Signal the system that every draw for this frame has been submitted.
			// This function gives permission to the OS to hibernate when all the
			// currently running GPU tasks (graphics and compute) are done.
			ret = sce::Gnm::submitDone();
		ASSERT(ret == SCE_GNM_OK)

			// Rotate the display buffers
			m_backBufferIndex = (m_backBufferIndex + 1) % m_displayBufferCount;
		//m_backBuffer = m_displayBuffers + m_backBufferIndex;

		// Rotate the render contexts
		m_renderContextIndex = (m_renderContextIndex + 1) % m_renderContextCount;
    }

    void PS4Renderer::Shutdown()
    {
        // Wait for the GPU to be idle before deallocating its resources
        for (uint32_t i = 0; i < m_renderContextCount; ++i)
        {
            if (m_frames[i].contextLabel)
            {
                while (m_frames[i].contextLabel[0] != renderContextFree)
                {
                    sceKernelUsleep(1000);
                }
            }
        }

        // Unregister the EOP event queue
        int ret = sce::Gnm::deleteEqEvent(m_eventQueue, sce::Gnm::kEqEventGfxEop);
        if (ret != SCE_OK)
        {
            printf("Gnm::deleteEqEvent failed: 0x%08X\n", ret);
        }

        // Destroy the EOP event queue
        ret = sceKernelDeleteEqueue(m_eventQueue);
        if (ret != SCE_OK)
        {
            printf("sceKernelDeleteEqueue failed: 0x%08X\n", ret);
        }

        // Terminate the video output
        ret = sceVideoOutClose(m_videoOutHandle);
        if (ret != SCE_OK)
        {
            printf("sceVideoOutClose failed: 0x%08X\n", ret);
        }

        // Releasing manually each allocated resource is not necessary as we are
        // terminating the linear allocators for ONION and GARLIC here.
        m_onionAllocator.terminate();
        m_garlicAllocator.terminate();
        m_gfxContext = nullptr;
    }

    sce::Gnm::GpuMode PS4Renderer::GetGPUMode() const
    {
        return m_gpuMode;
    }

    void PS4Renderer::CreateCommandBuffer(sce::Gnmx::GnmxGfxContext * a_ptr)
    {
        const auto cueHeapSize = sce::Gnmx::ConstantUpdateEngine::computeHeapSize(m_cueRingEntries);
        void* cueData = m_garlicAllocator.allocate(cueHeapSize, 4);
        void* drawCmdData = m_onionAllocator.allocate(m_dcbSizeInBytes, 4);
        void* constCmdBuffer = m_onionAllocator.allocate(m_ccbSizeInBytes, 4);
        a_ptr->init(cueData, m_cueRingEntries, drawCmdData, m_dcbSizeInBytes, constCmdBuffer, m_ccbSizeInBytes);
    }

	void PS4Renderer::CreateTexture(sce::Gnm::Texture& a_tex, sce::Gnm::DataFormat a_dataFormat, sce::Gnm::ResourceMemoryType a_resourceMemoryType, size_t a_mipLevels, float a_resolutionScale)
	{
		sce::Gnm::TextureSpec spec{};
		spec.init();
		spec.m_width = m_screenSize.x * a_resolutionScale;
		spec.m_height = m_screenSize.y * a_resolutionScale;
		spec.m_pitch = 0;
		spec.m_numSlices = 1;
		spec.m_numMipLevels = a_mipLevels;
		spec.m_format = a_dataFormat;
		auto ret = a_tex.init(&spec);
		ASSERT(ret == SCE_GNM_OK);
		const auto sizeAlign = a_tex.getSizeAlign();
		void* data = m_garlicAllocator.allocate(sizeAlign);
		ASSERT(data != nullptr);
		a_tex.setBaseAddress(data);
		a_tex.setResourceMemoryType(a_resourceMemoryType);
		m_clearTextureFunctions.emplace(&a_tex, [&]() { memset(data, 0, sizeAlign.m_size); });
	}

	void PS4Renderer::CreateRenderTarget(sce::Gnm::RenderTarget& a_rt, sce::Gnm::DataFormat a_dataFormat)
    {
		// Compute the tiling mode for the render target
		sce::Gnm::TileMode tileMode;
		sce::Gnm::DataFormat format = sce::Gnm::kDataFormatB8G8R8A8UnormSrgb;
		int ret = sce::GpuAddress::computeSurfaceTileMode(m_gpuMode, &tileMode, sce::GpuAddress::kSurfaceTypeColorTargetDisplayable, format, 1);
		ASSERT_MSG(ret == SCE_OK, "GpuAddress::computeSurfaceTileMode");

		sce::Gnm::RenderTargetSpec spec{};
		spec.init();
		spec.m_width = m_screenSize.x;
		spec.m_height = m_screenSize.y;
		spec.m_pitch = 0;
		spec.m_numSlices = 1;
		spec.m_colorFormat = a_dataFormat;
		spec.m_colorTileModeHint = tileMode;
		spec.m_minGpuMode = sce::Gnm::getGpuMode();
		spec.m_numSamples = sce::Gnm::kNumSamples4;
		spec.m_numFragments = sce::Gnm::kNumFragments1;
		spec.m_flags.enableCmaskFastClear = 0;
		spec.m_flags.enableFmaskCompression = 0;
		ret = a_rt.init(&spec);
		ASSERT(ret == SCE_GNM_OK);
		const auto sizeAlign = a_rt.getColorSizeAlign();
		a_rt.setAddresses(m_garlicAllocator.allocate(sizeAlign), nullptr, nullptr);
    }

    void PS4Renderer::CreateRenderTargetAndTexture(sce::Gnm::RenderTarget& a_rt, sce::Gnm::Texture& a_tex, sce::Gnm::DataFormat a_dataFormat, sce::Gnm::ResourceMemoryType a_resourceMemoryType)
    {
        // Compute the tiling mode for the render target
        sce::Gnm::TileMode tileMode;
        sce::Gnm::DataFormat format = sce::Gnm::kDataFormatB8G8R8A8UnormSrgb;
        int ret = sce::GpuAddress::computeSurfaceTileMode(m_gpuMode, &tileMode, sce::GpuAddress::kSurfaceTypeColorTargetDisplayable, format, 1);
        ASSERT_MSG(ret == SCE_OK, "GpuAddress::computeSurfaceTileMode");

        sce::Gnm::RenderTargetSpec spec{};
        spec.init();
        spec.m_width = m_screenSize.x;
        spec.m_height = m_screenSize.y;
        spec.m_pitch = 0;
        spec.m_numSlices = 1;
        spec.m_colorFormat = a_dataFormat;
        spec.m_colorTileModeHint = tileMode;
        spec.m_minGpuMode = sce::Gnm::getGpuMode();
        spec.m_numSamples = sce::Gnm::kNumSamples4;
        spec.m_numFragments = sce::Gnm::kNumFragments1;
        spec.m_flags.enableCmaskFastClear = 0;
        spec.m_flags.enableFmaskCompression = 0;
        ret = a_rt.init(&spec);
        ASSERT(ret == SCE_GNM_OK);
        const auto sizeAlign = a_rt.getColorSizeAlign();
        a_rt.setAddresses(m_garlicAllocator.allocate(sizeAlign), nullptr, nullptr);
        a_tex.initFromRenderTarget(&a_rt, false);
        a_tex.setResourceMemoryType(a_resourceMemoryType);
    }

    void PS4Renderer::CreateDepthTargetAndTexture(sce::Gnm::DepthRenderTarget& a_dt, sce::Gnm::Texture& a_tex, sce::Gnm::DataFormat a_depthFormat, sce::Gnm::StencilFormat a_stencilFormat, sce::Gnm::ResourceMemoryType a_resourceMemoryType)
    {
        // Compute the tiling mode for the depth render target
        sce::Gnm::TileMode tileMode;
        int ret = computeSurfaceTileMode(m_gpuMode, &tileMode, sce::GpuAddress::kSurfaceTypeColorTargetDisplayable, a_depthFormat, 1);
        ASSERT(ret == SCE_OK);

        // Initialize the depth buffer descriptor
        sce::Gnm::DepthRenderTargetSpec spec;
        spec.init();
        spec.m_width = m_screenSize.x;
        spec.m_height = m_screenSize.y;
        spec.m_pitch = 0;
        spec.m_numSlices = 1;
        spec.m_zFormat = a_depthFormat.getZFormat();
        spec.m_stencilFormat = a_stencilFormat;
        spec.m_minGpuMode = m_gpuMode;
        spec.m_numFragments = sce::Gnm::kNumFragments1;
        spec.m_flags.enableHtileAcceleration = m_htileEnabled ? 1 : 0;
        ret = a_dt.init(&spec);
        ASSERT(ret == SCE_GNM_OK);
        const sce::Gnm::SizeAlign depthTargetSizeAlign = a_dt.getZSizeAlign();

        // Initialize the HTILE buffer, if enabled
        if (m_htileEnabled)
        {
            const sce::Gnm::SizeAlign htileSizeAlign = a_dt.getHtileSizeAlign();
            void* htileMemory = m_garlicAllocator.allocate(htileSizeAlign);
            ASSERT(htileMemory != nullptr);
            a_dt.setHtileAddress(htileMemory);
        }

        const sce::Gnm::SizeAlign stencilSizeAlign = a_dt.getStencilSizeAlign();
        ASSERT(a_stencilFormat != sce::Gnm::kStencilInvalid)
            void* stencilMemory = m_garlicAllocator.allocate(stencilSizeAlign);
        void *depthMemory = m_garlicAllocator.allocate(depthTargetSizeAlign);
        ASSERT(stencilMemory != nullptr || depthMemory != nullptr);
        a_dt.setAddresses(depthMemory, stencilMemory);

        a_tex.initFromDepthRenderTarget(&a_dt, false);
        a_tex.setResourceMemoryType(a_resourceMemoryType);
    }

    void PS4Renderer::InitializeGBuffers()
    {
        SceVideoOutResolutionStatus status;
        if (SCE_OK == sceVideoOutGetResolutionStatus(m_videoOutHandle, &status) && status.fullHeight > 1080)
        {
            m_screenSize.x = status.fullWidth;
            m_screenSize.y = status.fullHeight;
        }

        const auto depthFormat = sce::Gnm::DataFormat::build(sce::Gnm::kZFormat32Float);
        void* displaySurfaceData[m_displayBufferCount];
        for (auto i = 0; i < m_displayBufferCount; ++i)
        {
            Frame& frame = m_frames[i];
            CreateCommandBuffer(&frame.gfxContext);
			frame.gfxContext.setRenderTargetMask(0xFFFFFFFF);
            CreateRenderTargetAndTexture(frame.backBufferTarget, frame.backBufferTexture, sce::Gnm::kDataFormatB8G8R8A8UnormSrgb, sce::Gnm::kResourceMemoryTypeRO);
            CreateRenderTargetAndTexture(frame.emissiveTarget, frame.emissiveTexture, sce::Gnm::kDataFormatB8G8R8A8Unorm, sce::Gnm::kResourceMemoryTypeRO);
            CreateDepthTargetAndTexture(frame.depthTarget, frame.depthTargetTexture, depthFormat, sce::Gnm::kStencil8, sce::Gnm::kResourceMemoryTypeRO); // this texture is never bound as an RWTexture, so it can be marked read-only.
			CreateTexture(frame.blurInputTexture, sce::Gnm::kDataFormatR8G8B8A8Unorm, sce::Gnm::kResourceMemoryTypeGC, m_blurMipLevels, .5f);
			CreateTexture(frame.HorizontalBlurResultTexture, sce::Gnm::kDataFormatR8G8B8A8Unorm, sce::Gnm::kResourceMemoryTypeGC, m_blurMipLevels, .5f);
        	// GPU fence value, volatile to prevent it from being optimized away
            frame.contextLabel = (volatile uint32_t*)m_onionAllocator.allocate(4, 8);
			ASSERT(frame.contextLabel != nullptr);

            displaySurfaceData[i] = frame.backBufferTarget.getBaseAddress();
			frame.gfxContext.setPrimitiveType(sce::Gnm::kPrimitiveTypeTriList);
			frame.gfxContext.setIndexSize(sce::Gnm::kIndexSize32);
        }

        SceVideoOutBufferAttribute attribute;
        sceVideoOutSetBufferAttribute(&attribute,
            SCE_VIDEO_OUT_PIXEL_FORMAT_B8_G8_R8_A8_SRGB,
            SCE_VIDEO_OUT_TILING_MODE_TILE,
            SCE_VIDEO_OUT_ASPECT_RATIO_16_9,
            m_screenSize.x,
            m_screenSize.y,
            m_frames[0].backBufferTarget.getPitch());
        int res = sceVideoOutRegisterBuffers(m_videoOutHandle, 0, displaySurfaceData, m_displayBufferCount, &attribute);
        ASSERT(res == SCE_VIDEO_OUT_OK)
    }

    void PS4Renderer::RemoveRenderable(RenderableId a_id)
    {
        auto inst = Get<PS4Renderable>();
        for (auto v = inst.begin(); v != inst.end(); ++v)
        {
			auto renderable = static_cast<PS4Renderable*>(*v);
			auto id = renderable->GetId();
            if (id == a_id)
            {
                TextureId texId = static_cast<const TextureId>(renderable->GetMaterial()->diffuseTextureId);
	            ptl::vector<PS4Renderable*> list = m_texCache.at(texId._id);
	            std::_Vector_iterator<std::_Vector_val<std::_Simple_types<PS4Renderable*>>> iterator = std::find(list.begin(), list.end(), renderable);
                m_texCache.at(texId._id).erase(iterator);
                if (m_texCache.at(texId._id).empty())
                    m_texCache.erase(m_texCache.find(texId._id));
            }
        }

		ASSERT(!Contains<PS4Renderable>(a_id));
		Remove<PS4Renderable>(a_id);
    }

	RenderableId PS4Renderer::AddRenderable(core::Transform* transform, MeshId meshId, MaterialId materialId)
	{
		auto id = GenerateRenderableId();
		auto& mesh = m_resourceManager->GetMesh(meshId);
		auto& material = m_resourceManager->GetMaterial(materialId);
		Add<PS4Renderable>(*transform, static_cast<PS4Mesh&>(mesh), static_cast<Material&>(material), materialId,  id);

		auto texId = m_resourceManager->GetMaterial(materialId).diffuseTextureId;
		if (m_texCache.find(texId._id) == m_texCache.end())
			m_texCache.emplace(texId._id, ptl::vector<PS4Renderable*>());

		m_texCache.at(texId._id).push_back(&Get<PS4Renderable>(id));

		return id;
	}

	//void PS4Renderer::EnableRenderableAccess(RenderPassId a_pass)
 //   {
	//	auto& pass = Get<PS4RenderPass>(a_pass);
	//	auto meshes = static_cast<PS4ResourceManager*>(m_resourceManager.get())->Get<PS4Mesh>();
	//	for(auto i = 0; i < meshes.size(); ++i)
	//	{
	//		meshes[i]->RegisterPass(pass);
	//	}
 //   }

	//IRenderPass& PS4Renderer::GetRenderPass(RenderPassId renderPassId)
 //   {
	//	ASSERT(Contains<PS4RenderPass>(renderPassId));
	//	return Get<PS4RenderPass>(renderPassId);
 //   }

	bool PS4Renderer::ContainsRenderable(RenderableId a_renderableId)
	{
		return Contains<PS4Renderable>(a_renderableId);
	}
}
#endif