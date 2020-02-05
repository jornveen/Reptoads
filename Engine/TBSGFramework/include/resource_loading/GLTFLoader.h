#pragma once
#pragma warning(push, 0)   
#define TINYGLTF_NO_STB_IMAGE_WRITE
#include <tinygltf/new_new_tiny_gltf.h>
#pragma warning(pop)
#include "memory/String.h"
#include "rendering/Mesh.h"
#include "rendering/IResourceManager.h"
#include "memory/Containers.h"
#include "gameplay/Transform.h"


namespace scene
{
    class SceneNode;
}


namespace gfx {
	/// A collections of indices and vertices, which can be rendered with a single material
	struct RenderableMesh
	{
		MeshId meshId;
		gfx::Mesh* mesh;
		MaterialId materialId;
		gfx::Material* material;
	};
    class IRenderer;
}

class GLTFLoader
{
    gfx::IRenderer& renderer_;
	gfx::IResourceManager* gpuResourceManager_;
	ptl::string gltfFile_ = "";
	scene::SceneNode* rootSceneNode_ = nullptr;
	tinygltf::Model model_;
	ptl::unordered_map<int /*materialIndex*/, MaterialId> materialMap_;
	ptl::unordered_map<int /*materialIndex*/, ptl::vector<gfx::RenderableMesh> > renderableMeshMap_;

public:
	explicit GLTFLoader(gfx::IRenderer& gpuResourceManager);

	/*
	 * Load a full scene from .gltf or .glb file.
	 *
	 * \Param gltfFile: path to the file eg. "data/gltfs/DamagedHelmet.gltf" or "data/gltfs/Duck.glb"
	 * \Param parentNode (NOT OPTIONAL): The root node, where all models found in the gltf scene will be added to.
	 *                               When nullptr is given, this argument is ignored
	 */
	bool LoadModelFromGltf(const ptl::string& gltfFile, scene::SceneNode* rootSceneNode = nullptr);

private:
	bool LoadModelFromGltfOrGlb(const ptl::string &filename, tinygltf::Model *model);

	/// Recursive function for adding the node and all it's children to sceneRoot
	void AddNodeToScene(int nodeIndex, scene::SceneNode* parentSceneNode);

    /// Adds a Camera Scene Node to the root of the SceneGraph
    void AddCameraToScene(tinygltf::Node& camera);
	
	/// Create the Mesh, Material and Texture resources from the mesh and add it to the sceneNode
	void CreateMeshMaterialTexture(int meshIndex, scene::SceneNode* sceneNode);
    void CreateLights();
	/// Create material in gpuResourceManager, and return the id
	MaterialId CreateMaterial(int materialIndex);
	ptl::string GetNameForMaterial(int materialIndex);
	ptl::string GetNameForTexture(int textureIndex);
	ptl::string GetNameForMesh(int meshIndex, int primitiveIndex);
	static core::Transform GetLocalTransformFromNode(tinygltf::Node& node, scene::SceneNode* parentNode);
};