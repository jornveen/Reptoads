#include "memory/String.h"
#ifdef PLATFORM_WINDOWS
#include "rendering/dx/DX12Renderer.h"
#include "core/Defines.h"
#include <rendering/dx/DX12ResourceManager.h>
#include <rendering/dx/DX12Application.h>
#include <rendering/dx/CommandList.h>
#include <rendering/dx/CommandQueue.h>
#include <d3dcompiler.h>
#include "rendering/dx/Helpers.h"
#include "rendering/dx/DX12Shader.h"
#include "rendering/dx/render_passes/DxForwardPass.h"
#include "rendering/dx/ResourceStateTracker.h"

#include "DirectXTex.h"

#include <codecvt>
#include <string>
#include <experimental/filesystem>

#include <core/Config.h>

namespace gfx
{
	uint32_t DX12ResourceManager::textureIdCounter = 0;
	uint32_t DX12ResourceManager::meshIdCounter = 0;
	uint32_t DX12ResourceManager::materialIdCounter = 0;
	uint32_t DX12ResourceManager::shaderIdCounter = 0;

	
	DX12ResourceManager::DX12ResourceManager(DX12Renderer* renderer): renderer_(renderer){};


	TextureId DX12ResourceManager::CreateTexture(const ptl::string& path)
	{
		ptl::string fullPath = tbsg::Config::Get().GetTexturesFolder() + path;

		{
			if (!std::experimental::filesystem::exists(fullPath)) {
				std::cerr << "ERROR Texture could not be found: '" << fullPath << "'" << std::endl;
				auto defaultTexId = GetDefaultTextureId();
				textureNameMap.insert({ path, defaultTexId });
				return defaultTexId;
			}
		}

		auto commandQueue = DX12Application::Get().GetCommandQueue();
		auto commandList = commandQueue->GetCommandList();
        
		ptl::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;

		ptl::wstring widetexture = converter.from_bytes(fullPath);
		//ptl::wstring wide = converter.from_bytes(folder.c_str()) + widetexture;

		//ASSERT(false);

		Texture texture;
		//TODO: Add TextureUsage options
		commandList->LoadTextureFromFile(texture, widetexture, TextureUsage::Albedo);
		auto fence = commandQueue->ExecuteCommandList(commandList);
		commandQueue->WaitForFenceValue(fence);

		TextureId id{ ++textureIdCounter };
		textureMap.insert(std::pair<TextureId, Texture>(id, std::move(texture)));
		textureNameMap.insert(TextureNameMap::value_type{ path, id });
		return id;
	}

	// for now I need a pointer to a texture until the new shared architecture has been implemented
	// reason, the genericMultiMap currently only supports pointers
	TextureId DX12ResourceManager::AddTexture(gfx::Texture* inTexture)
	{
		TextureId id{ ++textureIdCounter };
		textureMap.insert({ id, *inTexture });
		return id;
	}

	TextureId DX12ResourceManager::CreateTextureFromMemory(unsigned char* texBuffer, uint32_t width, uint32_t height,
		uint32_t bitsPerChannel, uint32_t amountOfChannels, const glm::vec2 minUvs, const ptl::string& optionalName)
	{
		auto commandQueue = DX12Application::Get().GetCommandQueue();
		auto commandList = commandQueue->GetCommandList();


		//auto iter = textureMap_.find(id);
		//if (iter != textureMap_.end()) {
		if(false) {
			// texture.SetTextureUsage(textureUsage);
			// texture.SetD3D12Resource(iter->second);
			// texture.CreateViews();
			// texture.SetName(fileName);
		} else {
			// TexMetadata metadata;
			// ScratchImage scratchImage;
			//
			// if (filePath.extension() == ".dds") {
			// 	ThrowIfFailed(LoadFromDDSFile(fileName.c_str(), DDS_FLAGS_NONE, &metadata, scratchImage));
			// } else if (filePath.extension() == ".hdr") {
			// 	ThrowIfFailed(LoadFromHDRFile(fileName.c_str(), &metadata, scratchImage));
			// } else if (filePath.extension() == ".tga") {
			// 	ThrowIfFailed(LoadFromTGAFile(fileName.c_str(), &metadata, scratchImage));
			// } else {
			// 	ThrowIfFailed(LoadFromWICFile(fileName.c_str(), WIC_FLAGS_NONE, &metadata, scratchImage));
			// }
			//
			// if (textureUsage == TextureUsage::Albedo) {
			// 	metadata.format = MakeSRGB(metadata.format);
			// }

			ASSERT_MSG(bitsPerChannel == 8 && amountOfChannels == 4, "Currently only this configuration is supported");

			ptl::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
			ptl::wstring wideId = converter.from_bytes(optionalName);

			D3D12_RESOURCE_DESC textureDesc = {};
			textureDesc = CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, static_cast<UINT64>(width), static_cast<UINT>(height));


			auto device = DX12Application::Get().GetDevice();
			Microsoft::WRL::ComPtr<ID3D12Resource> textureResource;

			ThrowIfFailed(device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
				D3D12_HEAP_FLAG_NONE,
				&textureDesc,
				D3D12_RESOURCE_STATE_COMMON,
				nullptr,
				IID_PPV_ARGS(&textureResource)));

			// Update the global state tracker.
			ResourceStateTracker::AddGlobalResourceState(textureResource.Get(), D3D12_RESOURCE_STATE_COMMON);

			Texture texture{};
			texture.SetTextureUsage(TextureUsage::Albedo);
			texture.SetD3D12Resource(textureResource);
			texture.CreateViews();
			texture.SetName(wideId);

			const int imageCount = 1;
			std::vector<D3D12_SUBRESOURCE_DATA> subresources(imageCount);
			//const Image* pImages = scratchImage.GetImages();
			for (int i = 0; i < imageCount; ++i) {
				auto& subresource = subresources[i];
				subresource.RowPitch = width * (bitsPerChannel/8 * amountOfChannels);//pImages[i].rowPitch;
				subresource.SlicePitch = height * (bitsPerChannel / 8 * amountOfChannels);//pImages[i].slicePitch;
				subresource.pData = texBuffer;//pImages[i].pixels;
			}
			
			commandList->CopyTextureSubresource(texture, 0, static_cast<uint32_t>(subresources.size()), subresources.data());

			if (subresources.size() < textureResource->GetDesc().MipLevels) {
				commandList->GenerateMips(texture);
			}

			auto fence = commandQueue->ExecuteCommandList(commandList);
			commandQueue->WaitForFenceValue(fence);

			// Add the texture resource to the texture cache.
			TextureId id{ ++textureIdCounter };
			textureMap.insert({ id, texture });
			return id;
		}
	}

	MeshId DX12ResourceManager::CreateMesh(const Vertex* vertices, size_t vtxCount, const uint32_t* indices,
		size_t idxCount, const ptl::string& optionalName)
	{

		ptl::unique_ptr<gfx::Mesh> meshPtr{ new gfx::DX12Mesh{ vertices, vtxCount, indices, idxCount, optionalName.c_str() } };

		MeshId id{ ++meshIdCounter };
		meshMap.insert(MeshMap::value_type{ id, std::move(meshPtr) });
		return id;
	}

	MaterialId DX12ResourceManager::CreateMaterial(BlendMode blendMode, glm::vec4 diffuseFactor, TextureId diffuseTextureId, 
		glm::vec3 emissiveFactor, TextureId emissiveTextureId, float alphaCutoff, const ptl::string& optionalName)
	{
		UNUSED(optionalName);

		Material mat{ blendMode, alphaCutoff, diffuseFactor, diffuseTextureId, emissiveFactor, emissiveTextureId };

		MaterialId id{ ++materialIdCounter };
		materialMap.insert(MaterialMap::value_type(id, std::move(mat)));
		return id;
	}

	//void DX12ResourceManager::CreateMaterial(glm::vec3, glm::vec3 a_ambient, glm::vec3 a_diffuse,
	//	glm::vec3 a_specular, float a_shinyness, const char* a_textureId, const char* a_id)
	//{
	//	Material mat{ a_ambient, a_diffuse, a_specular, a_shinyness, a_textureId, a_id };
	//	materialMap_.insert(std::pair<std::string, Material>(a_id, std::move(mat)));
	//}

	template<typename T>
	static bool hasEnding(std::basic_string<T> const &fullString, std::basic_string<T> const &ending) {
		if (fullString.length() >= ending.length()) {
			return (0 == fullString.compare(fullString.length() - ending.length(), ending.length(), ending));
		} else {
			return false;
		}
	}

	ShaderId DX12ResourceManager::CreateShader(const ptl::string& shaderPath)
	{
		ptl::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
		ptl::wstring wide = converter.from_bytes(shaderPath);

		Microsoft::WRL::ComPtr<ID3DBlob> shaderBlob;
		ThrowIfFailed(D3DReadFileToBlob(wide.c_str(), &shaderBlob));

		namespace fs = std::experimental::filesystem;

		fs::path path{ shaderPath };
		auto filename = path.stem();

		ShaderType type;
		if (hasEnding<wchar_t>(std::wstring(filename.c_str()), L"_v"))
			type = ShaderType::VS;
		else if (hasEnding<wchar_t>(std::wstring(filename.c_str()), L"_p"))
			type = ShaderType::PS;
		else if (hasEnding<wchar_t>(std::wstring(filename.c_str()), L"_c"))
			type = ShaderType::CS;
		else
		{
			ASSERT_MSG(false, "Cannot find the type of shader by the stuffix of the filename");
			throw std::exception();
		}
		

		std::string shaderName;
		switch (type) { 
			case ShaderType::PS: shaderName = " (Pixel Shader)"; break;
			case ShaderType::VS: shaderName = " (Vertex Shader)"; break;
			case ShaderType::CS: shaderName = " (Compute Shader)"; break;
			default: shaderName = " (Unknown shader type)";
		}

		auto shader = ptl::make_unique<DX12Shader>(shaderBlob, type, std::string(shaderPath) + shaderName);
		auto baseClassShader = ptl::static_pointer_cast<gfx::IShader>(std::move(shader));

		ShaderId id{ ++shaderIdCounter };
		shaderMap.insert(ShaderMap::value_type{ id, std::move(baseClassShader) });
		return id;
	}

	Texture& DX12ResourceManager::GetDefaultTexture()
	{
		return renderer_->m_EarthTexture;
	}

	TextureId DX12ResourceManager::GetDefaultTextureId()
	{
		return defaultTextureId;
	}

	/*
	//Texture& DX12ResourceManager::GetTexture(TextureId textureId)
	//{
	//	auto it = textureMap_.find(std::string(texturePath));
	//	if(it != textureMap_.end())
	//	{
	//		return it->second;
	//	}

	//	throw std::exception();
	//}

	////Texture& DX12ResourceManager::GetTexture(ptl::string texturePath)
	////{
	////	auto id_it = textureNameMap_.find()

	////	auto it = textureMap_.find(std::string(texturePath));
	////	if (it != textureMap_.end()) {
	////		return it->second;
	////	}

	////	throw std::exception();
	////}

	//Mesh& DX12ResourceManager::GetMesh(MeshId meshId)
	//{
	//	auto it = meshMap_.find(std::string(meshPath));
	//	if (it != meshMap_.end()) {
	//		return it->second;
	//	}

	//	throw std::exception();
	//}

	//Material& DX12ResourceManager::GetMaterial(MaterialId materialId)
	//{
	//	auto it = materialMap_.find(std::string(materialPath));
	//	if (it != materialMap_.end()) {
	//		return it->second;
	//	}

	//	throw std::exception();
	//}

	//IShader& DX12ResourceManager::GetShader(ShaderId shaderId)
	//{
	//	auto it = shaderMap_.find(std::string(shaderPath));
	//	if (it != shaderMap_.end()) {
	//		return *it->second;
	//	}

	//	throw std::exception();
	//}

	//bool DX12ResourceManager::ContainsTexture(TextureId textureId)
	//{
	//	return textureMap_.find(std::string(id)) != textureMap_.end();
	//}

	//bool DX12ResourceManager::ContainsMesh(MeshId meshId)
	//{
	//	return meshMap_.find(std::string(id)) != meshMap_.end();
	//}

	//bool DX12ResourceManager::ContainsMaterial(MaterialId materialId)
	//{
	//	return materialMap_.find(std::string(id)) != materialMap_.end();
	//}

	//bool DX12ResourceManager::ContainsShader(ShaderId shaderId)
	//{
	//	return shaderMap_.find(std::string(id)) != shaderMap_.end();
	//}
	*/

	void DX12ResourceManager::DeleteTexture(TextureId textureId)
	{
		textureMap.erase(textureId);
		for(auto it = textureNameMap.begin(); it != textureNameMap.end(); ++it) {
			if (it->second == textureId) {
				textureNameMap.erase(it);
				break;
			}
		}
	}
	void DX12ResourceManager::DeleteMesh(MeshId meshId)
	{
		meshMap.erase(meshId);
	}
	void DX12ResourceManager::DeleteMaterial(MaterialId materialId)
	{
		materialMap.erase(materialId);
	}
	void DX12ResourceManager::DeleteShader(ShaderId shaderId)
	{
		shaderMap.erase(shaderId);
	}
	void DX12ResourceManager::DeleteAllResources()
	{
		textureMap.clear();
		textureNameMap.clear();
		meshMap.clear();
		materialMap.clear();
		shaderMap.clear();
	}
}

#endif