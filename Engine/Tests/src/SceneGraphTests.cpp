#ifndef PLATFORM_PS
#include <scene/SceneGraph.h>
#include <catch/catch.hpp>
#include <glm/glm.hpp>
#include <resource_loading/GLTFLoader.h>
#include "rendering/dx/DX12Mesh.h"
#include "rendering/dx/DX12Shader.h"
#include "rendering/IRenderer.h"


TEST_CASE("[SceneGraph] SetParent")
{
	using namespace scene;
	SceneNode root;

	REQUIRE(root.GetChildren().empty());

	auto* child0 = root.AddSceneNode(core::Transform{ });
	child0->name = "Child_0";
	REQUIRE(root.GetChildren().size() == 1);

	auto* child1 = root.AddSceneNode(core::Transform{ });
	child1->name = "Child_1";
	REQUIRE(root.GetChildren().size() == 2);

	child0->SetParent(child1);
	REQUIRE(root.GetChildren().size() == 1);
	REQUIRE(root.GetChildren()[0].get() == child1);
	REQUIRE(child1->GetParent() == &root);
	REQUIRE(child0->GetParent() == child1);
	REQUIRE(child1->GetChildren().size() == 1);
	REQUIRE(child1->GetChildren()[0].get() == child0);
}

TEST_CASE("[SceneGraph] Calculate WorldModelMatrix with multiple children")
{
	using namespace scene;

	// Sanity test for me, prerequisite for testing commutative matrix/rotation multiplication in core::Transform::GetWorldModelMatrix()
	glm::quat rot_90_degrees_x{ glm::vec3{glm::radians(90.0f), 0, 0} };
	glm::quat rot_90_degrees_y{ glm::vec3{0.0f, glm::radians(90.0f), 0} };
	glm::quat rot90x90y{/*Euler:*/glm::vec3{glm::radians(90.0f), glm::radians(90.0f), 0.0f} };

	REQUIRE_FALSE(rot_90_degrees_x * rot_90_degrees_y == rot_90_degrees_y * rot_90_degrees_x);
	REQUIRE_FALSE(rot_90_degrees_x * rot_90_degrees_y == rot90x90y);
	REQUIRE(rot_90_degrees_y * rot_90_degrees_x == rot90x90y);

	SceneNode root;

	core::Transform child0_tr{ glm::vec3{10.f, 0.f, 0.f}, rot_90_degrees_x, glm::vec3{1,1,1} };
	glm::mat4 child0_local_mat = child0_tr.GetLocalModelMatrix();
	auto* child0 = root.AddSceneNode( std::move(child0_tr));
	child0->name = "Child_0";
	CHECK(child0->transform.GetWorldModelMatrix() == child0_local_mat);

	core::Transform child1_tr{ glm::vec3{20.f, 15.f, 0.f}, rot_90_degrees_y, glm::vec3{1,1,1} };
	glm::mat4 child1_local_mat = child1_tr.GetLocalModelMatrix();
	auto* child1 = root.AddSceneNode(std::move(child1_tr));
	child1->name = "Child_1";
	CHECK(child1->transform.GetWorldModelMatrix() == child1_local_mat);

	child0->SetParent(child1);
	glm::mat4 child0_tr_new_mat = child0->transform.GetWorldModelMatrix();
	CHECK(child0_tr_new_mat == child1_local_mat * child0_local_mat);

	glm::vec3 pos;
	glm::quat rot;
	glm::vec3 scale;
	glm::vec3 skew;
	glm::vec4 perspective;
	glm::decompose(child0_tr_new_mat, scale, rot, pos, skew, perspective);
	CHECK(glm::all(glm::epsilonEqual(pos, glm::vec3{ 20, 15, -10 }, 0.1f)));
	CHECK(glm::all(glm::epsilonEqual(rot, rot90x90y, 0.2f)));
	CHECK(glm::all(glm::epsilonEqual( scale, glm::vec3{ 1,1,1 }, 0.1f )));
}

// A resource manager which voids all calls and returns static instances of resources
class MockResourceManager : public gfx::IResourceManager
{
public:
	TextureId CreateTextureFromMemory(unsigned char* , uint32_t , uint32_t ,
		uint32_t , uint32_t , const ptl::string& ) override{
		return { 0 };
	}
	MeshId CreateMesh(const Vertex* , size_t , const uint32_t* , size_t ,
		const ptl::string& , size_t ) override{
		return { 0 };
	}
	MaterialId CreateMaterial(BlendMode , glm::vec4 , TextureId ,
		glm::vec3 , TextureId , float, const ptl::string& ) override{
		return { 0 };
	}
	ShaderId CreateShader(const ptl::string& ) override{ return { 0 }; }
	gfx::Texture& GetDefaultTexture() override { static gfx::Texture tex; return tex; }
	TextureId GetDefaultTextureId() override{ return { 0 }; }
	void DeleteTexture(TextureId ) override{}
	void DeleteMesh(MeshId ) override{}
	void DeleteMaterial(MaterialId ) override{}
	void DeleteShader(ShaderId ) override{}
	void DeleteAllResources() override{}
	inline gfx::Texture& GetTexture(TextureId) override { static gfx::Texture tex; return tex; }
	inline gfx::Texture& GetTexture(const ptl::string& ) override{ static gfx::Texture tex; return tex; }
	inline TextureId GetTextureId(const ptl::string&) override { return { 0 }; }
	inline Mesh& GetMesh(MeshId ) override{ static gfx::Mesh& tex = *reinterpret_cast<gfx::Mesh*>(123); return tex; }
	inline Material& GetMaterial(MaterialId ) override{ static gfx::Material tex{BlendMode::Opaque, 0, {}, {0}, {}, {0}}; return tex; }
	inline IShader& GetShader(ShaderId ) override{ static gfx::DX12Shader tex{{}, ShaderType::SHADER_END_BIT, "NONE"}; return tex; }
	inline bool ContainsTexture(TextureId) override { return false; }
	inline bool ContainsTexture(const ptl::string& ) override{ return false; }
	inline bool ContainsMesh(MeshId ) override{ return false; }
	inline bool ContainsMaterial(MaterialId ) override{ return false; }
	inline bool ContainsShader(ShaderId ) override{ return false; }
	TextureId AddTexture(gfx::Texture*) override { return { 0 }; }
	TextureId CreateTexture(const ptl::string&) override { return { 0 }; }
	//	void CreateTexture(const char* , const ptl::string& ) override {}
//	void CreateTextureFromMemory(const char* , unsigned char* , uint32_t , uint32_t ,
//		uint32_t , uint32_t ) override {}
//	void CreateMesh(const gfx::Vertex* , size_t , const uint32_t* , size_t ,
//		const char* , size_t ) override {}
//	void CreateMaterial(glm::vec3 , glm::vec3 , glm::vec3 , glm::vec3 ,
//		float , const char* ) override {}
//	void CreateMaterial(glm::vec3 , glm::vec3 , glm::vec3 , glm::vec3 ,
//		float , const char* , const char* ) override {}
//	void CreateRenderPass(uint8_t , ptl::vector<gfx::IShader*> , const char* ) override {}
//	void CreateShader(const char* , bool ) override {}
//	void CreateRenderable(core::Transform& , gfx::Mesh& , gfx::Material& , const char* )
//	override {}
//	Texture& GetTexture(const char* ) override { static Texture mockTexture; return mockTexture; }
//	gfx::Mesh& GetMesh(const char* ) override
//	{
//#ifdef PLATFORM_WINDOWS
//		// ReSharper disable once CppMsExtReinterpretCastFromNullptr
//		static gfx::DX12Mesh& mockMesh = *reinterpret_cast<gfx::DX12Mesh*>(1);
//		return mockMesh;
//#else
//		throw std::exception();
//#endif
//	}
//	gfx::Material& GetMaterial(const char* ) override
//	{
//		static gfx::Material mockMaterial{ {}, {}, {}, 0.0f, "MockEmpty" };
//		return mockMaterial;
//	}
//	gfx::RenderPass& GetRenderPass(const char* ) override
//	{
//		static gfx::RenderPass mockRenderPass{ 0, "MockPass", {} };
//		return mockRenderPass;
//	}
//	gfx::IShader& GetShader(const char* ) override
//	{
//#ifdef PLATFORM_WINDOWS
//		// ReSharper disable once CppMsExtReinterpretCastFromNullptr
//		static gfx::DX12Shader& mockShader = *reinterpret_cast<gfx::DX12Shader*>(1);
//		return mockShader;
//#else
//		throw std::exception();
//#endif 
//	}
//	gfx::Renderable& GetRenderable(const char* ) override
//	{
//		static gfx::Renderable mockRenderable{ nullptr, nullptr, nullptr, "" };
//		return mockRenderable;
//	}
//	void DeleteTexture(const char* ) override {}
//	void DeleteMesh(const char* ) override {}
//	void DeleteMaterial(const char* ) override {}
//	void DeleteRenderPass(const char* ) override {}
//	void DeleteShader(const char* ) override {}
//	void DeleteRenderable(const char* ) override {}
//	void DeleteAllResources() override {}
//	bool ContainsTexture(const char* ) override { return true; }
//	bool ContainsMesh(const char* ) override { return true; }
//	bool ContainsMaterial(const char* ) override { return true; }
//	bool ContainsRenderPass(const char* ) override { return true; }
//	bool ContainsShader(const char* ) override { return true; }
//	bool ContainsRenderable(const char* ) override { return true; }
//	gfx::Texture& AddTexture(const char*, gfx::Texture&&) override
//	{
//		static gfx::Texture mockTexture{};
//		return mockTexture;
//	}
};

class MockRenderer : public gfx::IRenderer
{
public:
	MockRenderer(): IRenderer(
		{ nullptr }, 
		{ nullptr }, 
		std::unique_ptr<gfx::IResourceManager>{ new MockResourceManager() }) 
	{}

	void Initialize() override{}
	void RenderWorld(gfx::Camera& ) override{}
	void Present() override {}
	void Shutdown() override {}
	glm::vec<2, size_t> GetScreenSize() const override
	{
		return { 1920.f, 1080.f };
	}
	RenderableId AddRenderable(core::Transform* , MeshId , MaterialId ) override{ return { 0 }; }
	bool ContainsRenderable(RenderableId ) override{ return false; }
	void RemoveRenderable(RenderableId ) override {}
	bool ContainsRenderPass(RenderPassId) override { return false; }
	void RemoveRenderPass(RenderPassId ) override {}
	void RenderUI(Camera&) override {}
	RenderPassId AddRenderPass(IRenderPass*) override { return { 0 }; }
};


TEST_CASE("[SceneGraph] 'data/gltfs/unit_test_models/BlenderGameObject.glb' hierarchy test, check if parent/child relation is correct")
{
	MockRenderer renderer{};
	scene::SceneNode root{};

	GLTFLoader loader{ renderer };
	bool succesfull_load;
	REQUIRE_NOTHROW(succesfull_load = loader.LoadModelFromGltf("data/gltfs/unit_test_models/BlenderGameObject.glb", &root));
	REQUIRE(succesfull_load);

	// Check if the parent reference corresponds with the children reference.
	// I don't use the iterator to tests ONLY the parent-child relation, and not depend on iterator correctness
	ptl::stack<scene::SceneNode*> remainingNodes;
	remainingNodes.push(&root);
	while (!remainingNodes.empty()) {
		auto* currentParent = remainingNodes.top();
		remainingNodes.pop();

		for(auto& nodePtr : currentParent->GetChildren()) {
			CHECK(nodePtr->GetParent() == currentParent);
			CHECK(nodePtr->transform.GetParent() == &currentParent->transform);

			remainingNodes.push(nodePtr.get());
		}
	}
}
#endif
