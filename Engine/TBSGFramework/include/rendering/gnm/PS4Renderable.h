#pragma once
#ifdef PLATFORM_PS
#include "PS4Resource.h"
#include "ClassID.h"
#include "gameplay/Transform.h"

namespace gfx {
	class PS4RenderPass;
	class Material;
	class PS4Mesh;

	//#TODO support instancing
	class PS4Renderable : public gfx::PS4Resource, public ClassID<PS4Renderable>
	{
	public:
		PS4Renderable(core::Transform& a_transform, PS4Mesh& a_mesh, gfx::Material& a_material, MaterialId materialId, RenderableId a_id);
		~PS4Renderable();
		PS4Mesh* GetMesh() const { return mesh; }
		void SetMesh(gfx::PS4Mesh& a_mesh);
		Material* GetMaterial() const { return material; }
		MaterialId GetMaterialId() const { return materialId; }
		void SetMaterial(Material& a_material) { material = &a_material; }
		core::Transform& GetTransform() const { return *transform; }
        // compatibility with the DX12
        core::Transform* transform{ nullptr };
        gfx::PS4Mesh* mesh{ nullptr };
        gfx::Material* material{ nullptr };
		MaterialId materialId{ 0 };

	private:
		size_t m_meshHandle{};
	};
}
#endif