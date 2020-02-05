//#include "rendering/gnm/PS4Texture.h"
//#ifdef PLATFORM_PS
//#include "rendering/gnm/TextureLoader.h"
//#include <texture_tool.h>
//#include <fstream>
//#include "rendering/gnm/PS4Helpers.h"
//#include <common/allocator.h>
//
////temp
//#include <string>
//
//gfx::TextureLoader::TextureLoader(LinearAllocator& a_garlicAlloc)
//	: m_allocator(&a_garlicAlloc)
//{
//}
//
//sce::Gnm::Texture gfx::TextureLoader::LoadGnf(const char* a_path)
//{
//	struct GnfHeader : public sce::Gnf::Header
//	{
//		uint8_t	 m_version;
//		uint8_t  m_count;
//		uint8_t  m_alignment;
//		uint8_t  m_unused;
//		uint32_t m_streamSize;
//	} header{};
//
//	std::ifstream file;
//	file.open(a_path, std::ios::binary | std::ios::ate);
//	ThrowIfFailed(file.fail(), "LOADING FILE FAILED...");
//
//	int32_t fileSize = file.tellg();
//	ThrowIfFailed(fileSize <= 0, "LOADED FILE IS EMPTY...");
//	file.seekg(0, std::ios::beg);
//
//	// Read header
//	file.read(reinterpret_cast<char *>(&header), sizeof(GnfHeader));
//	ThrowIfFailed(header.m_magicNumber != sce::Gnf::kMagic, "ERROR, file verification failed...");
//
//	// Read T#
//	sce::Gnm::Texture tsharp;
//	file.seekg(0, std::ios::cur);
//	file.read(reinterpret_cast<char *>(&tsharp), sizeof(tsharp));
//	file.seekg(sizeof(sce::Gnm::Texture) * (header.m_count - 1), std::ios::cur); // Skip remaining T#s
//
//	// Check if there are user data
//	if ((sizeof(GnfHeader) - sizeof(sce::Gnf::Header) + sizeof(sce::Gnm::Texture) * header.m_count) != header.m_contentsSize)
//	{
//		// Read user data
//		uint32_t userDataMagic;
//		uint32_t userDataSize;
//		file.read(reinterpret_cast<char *>(&userDataMagic), sizeof(userDataMagic));
//		file.read(reinterpret_cast<char *>(&userDataSize), sizeof(userDataSize));
//	}
//
//
//	// Read texel data
//	auto texelBuf = m_allocator->allocate(tsharp.getSizeAlign().m_size, tsharp.getSizeAlign().m_size);
//	ThrowIfFailed(texelBuf == nullptr, "TEXTURE ALLOCATION FAILED...");
//	std::cout << "TEX CONTENT SIZE: " << std::endl;
//	std::cout << tsharp.getSizeAlign().m_size << std::endl;
//
//	int32_t dataOffset = header.m_contentsSize + sizeof(sce::Gnf::Header);
//	file.seekg(dataOffset, std::ios::beg);
//	file.read(static_cast<char *>(texelBuf), tsharp.getSizeAlign().m_size);
//	tsharp.setBaseAddress(texelBuf);
//
//	file.clear();
//	file.close();
//
//	return tsharp;
//}
//#endif