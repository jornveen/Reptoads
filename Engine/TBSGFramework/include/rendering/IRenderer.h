#pragma once
#include <memory>
#include "IDebugRenderer.h"
#include "IUIRenderer.h"
#include "IResourceManager.h"
#include "Renderable.h"
#include "Camera.h"
#include "Lights.h"
#include "ResourceIds.h"

namespace gfx
{
	class Camera;

	/// Base for a renderer.
	/// Contains RenderContext directly
	class IRenderer
	{
	protected:
		std::unique_ptr<IDebugRenderer> m_debugRenderer;
		std::unique_ptr<IUIRenderer> m_uiRenderer;
		std::unique_ptr<IResourceManager> m_resourceManager;

	public:
		IRenderer(std::unique_ptr<IDebugRenderer>&& debugRenderer, std::unique_ptr<IUIRenderer>&& uiRenderer, std::unique_ptr<IResourceManager> a_rm) 
			: m_debugRenderer(std::move(debugRenderer)) , m_uiRenderer(std::move(uiRenderer)), m_resourceManager(std::move(a_rm)) {}
		virtual ~IRenderer() = default;

		// create a window and init the API.
		virtual void Initialize() = 0;
		// Render all meshes.
		virtual void RenderWorld(Camera& camera) = 0;
		// Render all UI elements.
		virtual void RenderUI(Camera& a_camera) = 0;
		virtual void Present() = 0;
		// Clear all data.
		virtual void Shutdown() = 0;
		virtual glm::vec<2, size_t> GetScreenSize() const = 0;

		RenderableId GenerateRenderableId() { return RenderableId{ ++m_renderableId }; }
		RenderPassId GenerateRenderPassId() { return RenderPassId{ ++m_renderPassId }; }
		IDebugRenderer* GetDebugRenderer() { return m_debugRenderer.get(); }
		IUIRenderer* GetUIRenderer() { return m_uiRenderer.get(); }
		// get reference to the resource manager
		IResourceManager& GetGfxResourceManager() const { return *m_resourceManager; }
		// add a renderable instance
		virtual RenderableId AddRenderable(core::Transform* transform, MeshId meshId, MaterialId materialId) = 0;
		virtual bool ContainsRenderable(RenderableId renderableId) = 0;
		virtual void RemoveRenderable(RenderableId renderableId) = 0;

		virtual RenderPassId AddRenderPass(IRenderPass* renderPass) = 0;
		virtual bool ContainsRenderPass(RenderPassId renderPassId) = 0;
		virtual void RemoveRenderPass(RenderPassId renderPassId) = 0;

	private:
		uint32_t m_renderableId{};
		uint32_t m_renderPassId{};
		uint32_t m_pointLightId{};
		uint32_t m_spotLightId{};
		uint32_t m_dirLightId{};
	};
}
