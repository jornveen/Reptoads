#pragma once
#if defined(PLATFORM_WINDOWS)
#include "dx/DX12Renderable.h"
#elif defined(PLATFORM_PS)
#include "rendering/gnm/PS4Renderable.h"
#endif
#include "ResourceIds.h"

namespace gfx
{
	struct Renderable
	{
		// for now I have to add the ID back duntil we have the unified architecture.
		// reason: IDCompare function in GenericMultiMap
		Renderable(MeshId meshId, MaterialId materialId) :
			meshId(meshId), materialId(materialId)
		{}

		MeshId meshId;
		MaterialId materialId;
		RenderableId renderableRegisteredId;
	};

	struct PunctualRenderable
	{
		core::Transform* transform;
		MeshId meshId;
		MaterialId materialId;
		RenderableId registeredId;
	};
}
