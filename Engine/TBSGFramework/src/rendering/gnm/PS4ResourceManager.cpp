#include "core/Config.h"
#ifdef PLATFORM_PS
#include "rendering/gnm/PS4Mesh.h"
#include "rendering/gnm/PS4Helpers.h"
#include "rendering/gnm/PS4Texture.h"
#include "rendering/gnm/PS4ResourceManager.h"
#include "rendering/gnm/PS4Material.h"
#include "rendering/gnm/PS4Shaders.h"
#include "memory/details/mem_utils.h"
#include <cassert>

bool EndsWith(const ptl::string& a_str, const ptl::string a_ext)
{
	return a_str.size() >= a_ext.size() &&
		a_str.compare(a_str.size() - a_ext.size(), a_ext.size(), a_ext) == 0;
}

gfx::PS4ResourceManager::PS4ResourceManager(PS4Renderer& a_core)
	: GenericMultiMap<PS4Resource>([&](const PS4Resource* x) { return Identifier(x->GetId()._id); }),
	meshMem
	{
		{ptl::AlignSize(1, 16u), 0, SCE_KERNEL_WC_GARLIC, SCE_KERNEL_PROT_CPU_WRITE | SCE_KERNEL_PROT_GPU_ALL}, new ptl::NullMemoryResource{}
	},
	texMem
	{
		{ptl::AlignSize(1 * 1024 * 1024 * 1024 + MB(600), 16u), 0, SCE_KERNEL_WC_GARLIC, SCE_KERNEL_PROT_CPU_WRITE| SCE_KERNEL_PROT_GPU_ALL}, new ptl::NullMemoryResource{}
	},
	shaderGarlicMem
	{
		{ptl::AlignSize(16 * 1024 * 1024, 16u), 0, SCE_KERNEL_WC_GARLIC, SCE_KERNEL_PROT_CPU_WRITE | SCE_KERNEL_PROT_GPU_ALL}, new ptl::NullMemoryResource{}
	},
	shaderOnionMem
	{
		{ptl::AlignSize(16 * 1024 * 1024, 16u), 0, SCE_KERNEL_WB_ONION, SCE_KERNEL_PROT_CPU_RW }, new ptl::NullMemoryResource{}
	},
	materialMem
	{
		{ptl::AlignSize(16 * 1024 * 1024 * 1024, 16u), 0, SCE_KERNEL_WC_GARLIC, SCE_KERNEL_PROT_CPU_WRITE | SCE_KERNEL_PROT_GPU_ALL }, new ptl::NullMemoryResource{}
	}
{

	OnItemAdded<PS4Material>([](auto ) {ASSERT_MSG(false, "THIS SHOULD NOT HAPPEN, WE USE IResourceManager::materialMap"); });
	OnItemRemoved<PS4Material>([](auto) {ASSERT_MSG(false, "THIS SHOULD NOT HAPPEN, WE USE IResourceManager::materialMap"); });
}

gfx::PS4ResourceManager::~PS4ResourceManager()
{
	DeleteAllResources();
}

TextureId PS4ResourceManager::AddTexture(Texture* inTexture)
{
	Add<PS4Texture>(static_cast<PS4Texture*>(inTexture));
	auto id = inTexture->GetId();
	TextureId* retId = static_cast<TextureId*>(&id); // this is weird, I know.
	return *retId;
}

TextureId PS4ResourceManager::CreateTexture(const ptl::string& filePath)
{
	auto id = GenerateTextureId();
	Add<PS4Texture>(filePath, id, texMem);
	textureNameMap.emplace(filePath, id);
	return id;
}

TextureId PS4ResourceManager::CreateTextureFromMemory(unsigned char* texBuffer, uint32_t width, uint32_t height, uint32_t bitsPerChannel, uint32_t amountOfChannels, const glm::vec2 minUvs, const ptl::string& optionalName)
{
	auto id = GenerateTextureId();
	Add<PS4Texture>(texBuffer, width, height, bitsPerChannel, amountOfChannels, id, texMem, minUvs);
	return id;
}

MeshId PS4ResourceManager::CreateMesh(const Vertex* vertices, size_t vtxCount, const uint32_t* indices, size_t idxCount, const ptl::string& optionalName)
{
	auto id = GenerateMeshId();
	Add<PS4Mesh>(vertices, vtxCount, indices, idxCount, id, texMem/*meshMem*/);
	return id;
}

MaterialId PS4ResourceManager::CreateMaterial(BlendMode blendMode, glm::vec4 diffuseFactor, TextureId diffuseTextureId,
	glm::vec3 emissiveFactor, TextureId emissiveTextureId, float alphaCutoff, const ptl::string& optionalName)
{

	auto id = GenerateMaterialId();
	materialMap.insert(MaterialMap::value_type{ id, Material{blendMode, alphaCutoff, diffuseFactor, diffuseTextureId, emissiveFactor, emissiveTextureId} });
	return id;
}

ShaderId PS4ResourceManager::CreateShader(const ptl::string& a_shaderPath)
{
	auto path = tbsg::Config::Get().MakeShaderPath(a_shaderPath,1);
	auto id = GenerateShaderId();
	if (EndsWith(path, "_v.sb") || EndsWith(path, "_v.pssl"))
	{
		Add<PS4VShader>(path, id, shaderOnionMem, texMem/*shaderGarlicMem*/);
	}
	else if (EndsWith(path, "_p.sb") || EndsWith(path, "_p.pssl"))
	{
		Add<PS4PShader>(path, id, shaderOnionMem, texMem/*shaderGarlicMem*/);
	}
	else if(EndsWith(path, "_c.sb") || EndsWith(path, "_p.pssl"))
	{
		Add<PS4CShader>(path, id, shaderOnionMem, texMem/*shaderGarlicMem*/);
	}
	else ASSERT_MSG(false, "SHADER TYPE IS NOT SUPPORTED YET...");
	return id;
}

Texture& PS4ResourceManager::GetDefaultTexture()
{
	return Get<PS4Texture>(static_cast<Identifier>(GetDefaultTextureId()));
}

TextureId PS4ResourceManager::GetDefaultTextureId()
{
	return TextureId(1); // 0 = null, return first created
}

bool PS4ResourceManager::ContainsTexture(TextureId a_id)
{
	return Contains<PS4Texture>(static_cast<Identifier>(a_id));
}

bool PS4ResourceManager::ContainsMesh(MeshId a_id)
{
	return Contains<PS4Mesh>(a_id);
}

bool PS4ResourceManager::ContainsShader(ShaderId a_id)
{
	return Contains<PS4VShader>(a_id) || Contains<PS4PShader>(a_id) || Contains<PS4CShader>(a_id);
}

PS4Texture& PS4ResourceManager::GetTexture(TextureId a_id)
{
	ASSERT(Contains<PS4Texture>(a_id));
	return Get<PS4Texture>(a_id);
}

PS4Mesh& PS4ResourceManager::GetMesh(MeshId a_id)
{
	ASSERT(Contains<PS4Mesh>(a_id));
	return Get<PS4Mesh>(a_id);
}


IShader& PS4ResourceManager::GetShader(ShaderId a_id)
{
	ASSERT(Contains<PS4VShader>(a_id) || Contains<PS4PShader>(a_id) || Contains<PS4CShader>(a_id));
	if (Contains<PS4VShader>(a_id)) return Get<PS4VShader>(a_id);
	if (Contains<PS4PShader>(a_id)) return Get<PS4PShader>(a_id);
	return Get<PS4CShader>(a_id);
}

void PS4ResourceManager::DeleteTexture(TextureId a_id)
{
	ASSERT(Contains<PS4Texture>(a_id));
	auto& tex = Get<PS4Texture>(a_id);
	Remove<PS4Texture>(a_id);
	delete &tex;
}

void PS4ResourceManager::DeleteMesh(MeshId a_id)
{
	ASSERT(Contains<PS4Mesh>(a_id));
	auto& mesh = Get<PS4Mesh>(a_id);
	Remove<PS4Mesh>(a_id);
	delete &mesh;
}

void PS4ResourceManager::DeleteMaterial(MaterialId id)
{
	auto it = materialMap.find(id);
	ASSERT_MSG(it != materialMap.end(), "Attempting to delete a material which cannot be found");
	if(it != materialMap.end()) {
		materialMap.erase(it);
	}
}

void PS4ResourceManager::DeleteShader(ShaderId a_id)
{
	if (Contains<PS4VShader>(a_id)) Remove<PS4VShader>(a_id);
	else if (Contains<PS4PShader>(a_id)) Remove<PS4PShader>(a_id);
	else if (Contains<PS4CShader>(a_id)) Remove<PS4CShader>(a_id);
	ASSERT(false);
}

void PS4ResourceManager::DeleteAllResources()
{
	DeleteAll();
	shaderGarlicMem.Release();
	shaderOnionMem.Release();
	materialMem.Release();
}
#endif
