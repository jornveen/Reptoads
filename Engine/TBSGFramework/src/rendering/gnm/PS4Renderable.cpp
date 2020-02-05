#ifdef PLATFORM_PS
#include "rendering/gnm/PS4Renderable.h"
#include "rendering/gnm/PS4Mesh.h"
#include "rendering/gnm/PS4Material.h"

gfx::PS4Renderable::PS4Renderable(core::Transform& a_transform, gfx::PS4Mesh& a_mesh, gfx::Material& a_material, MaterialId materialId, RenderableId a_id)
	: PS4Resource(a_id), transform(&a_transform), m_meshHandle(-1), mesh(&a_mesh), material(&a_material), materialId(materialId)
{
}

gfx::PS4Renderable::~PS4Renderable()
{
	mesh->RemoveOnMeshUpdated(m_meshHandle);
	m_meshHandle = -1;
}

void gfx::PS4Renderable::SetMesh(gfx::PS4Mesh& a_mesh)
{
	mesh->RemoveOnMeshUpdated(m_meshHandle);
	mesh = &a_mesh;
}

#endif
