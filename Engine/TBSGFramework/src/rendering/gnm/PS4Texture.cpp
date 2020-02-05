#include "core/Config.h"
#include "core/Defines.h"
#ifdef PLATFORM_PS
#include "rendering/gnm/PS4Texture.h"
#include <gnf.h>
#include <fstream>
#include "memory/freelist_resources.h"
#include "core/Assertion.h"
#include <stb_image.h>
#include <kernel.h>
#include <fios2.h>
#include <texture_tool.h>
#include "core/StringUtils.h"

const sce::Gnm::Texture* gfx::PS4Texture::GetGnmTexture() const
{
	return m_tex;
}

//create a buffer and apply padding when necessary
void gfx::PS4Texture::SetBufferWithPadding(size_t srcWidth, size_t paddingX, size_t srcHeight, size_t paddingY, const uint8_t* srcBuffer, uint8_t* destBuffer, size_t bitsPerChannel, size_t channelCount)
{
	const auto bytesPerChannel = bitsPerChannel / 8;
	const auto bytesPerPixel = bytesPerChannel * channelCount;
	const auto bytesPerSrcRow = bytesPerPixel * srcWidth;
	const auto bytesPerDestRow = bytesPerPixel * (srcWidth + paddingX);
	const auto bytesPerPaddingX = bytesPerPixel * paddingX;

	// set the bytes at the y padding to the destination buffer
	memset(destBuffer, 0, paddingY * bytesPerDestRow);
	destBuffer += paddingY * bytesPerDestRow;

	for(auto y = 0; y < srcHeight; ++y)
	{
		// add the current row's padding to the destination buffer
		void* startPtrDest = (void*)(destBuffer + y * bytesPerDestRow);
		memset(startPtrDest, 0, bytesPerPaddingX);

		// get the start ptr for copying a row of source data
		const void* startPtrSrc = (void*)(srcBuffer + y * srcWidth * bytesPerPixel);	// skip rows of src data only
		startPtrDest = (void*)(destBuffer + y * bytesPerDestRow + bytesPerPaddingX);	// skip rows of src data + padding data
		//memset(startPtrDest, 195, bytesPerSrcRow);
		memcpy(startPtrDest, startPtrSrc, bytesPerSrcRow);
	}

	uvMin.x = static_cast<float>(paddingX) / (srcWidth + paddingX);
	uvMin.y = static_cast<float>(paddingY) / (srcHeight + paddingY);
}

gfx::PS4Texture::PS4Texture(const ptl::string & a_path, const TextureId a_id, ptl::FreeListResource & memResource)
	: PS4Resource(a_id), alloc(&memResource)
{
	const auto bitsPerChannel = 8;
	const auto numChannels = 4;

	struct GnfHeader : sce::Gnf::Header
	{
		uint8_t	 m_version;
		uint8_t  m_count;
		uint8_t  m_alignment;
		uint8_t  m_unused;
		uint32_t m_streamSize;
	} header{};

	auto path = tbsg::Config::Get().MakeTexturePath(a_path);
	/*
	{
		std::ifstream file;
		file.open(path.c_str(), std::ios::binary | std::ios::ate);
		ASSERT_MSG(!file.fail(), "LOADING FILE FAILED...");
		file.close();
	}*/
	int width = 0, height = 0, channels = 0;
	uint8_t* const stbiData = stbi_load(path.c_str(), &width, &height, &channels, STBI_rgb_alpha);
	ASSERT(stbiData != nullptr);

	// handle paddings for weird texture dimensions
	auto paddingX = ptl::alignTo(width, 64) - width;
	auto paddingY = ptl::alignTo(height, 8) - height;

	const auto dataSize = (bitsPerChannel / 8) * (width + paddingX) * (height + paddingY) * numChannels;
	void* allocData = memResource.allocate(dataSize, 256);

	if(paddingX + paddingY > 0)
	{
		SetBufferWithPadding(width, paddingX, height, paddingY, stbiData, static_cast<uint8_t*>(allocData), bitsPerChannel, numChannels);
	}
	else
	{
		// allocate and copy plain data without paddings
		memcpy(allocData, stbiData, dataSize);
	}

	if (endsWith(path, ".dds"))
	{
		auto data = GetTextureFormat(bitsPerChannel, numChannels);
		m_tex = new sce::Gnm::Texture{};
		sce::Gnm::TextureSpec spec{};
		sce::Gnm::DataFormat form{};
		form.init(data.format, data.channelType, sce::Gnm::TextureChannel::kTextureChannelX, sce::Gnm::TextureChannel::kTextureChannelY, sce::Gnm::TextureChannel::kTextureChannelZ, sce::Gnm::TextureChannel::kTextureChannelW);
		spec.init();
		spec.m_depth = 1;
		spec.m_format = form;
		spec.m_width = width + paddingX;
		spec.m_height = (height + paddingY) / 6;
		spec.m_tileModeHint = sce::Gnm::kTileModeDisplay_LinearAligned;
		spec.m_textureType = sce::Gnm::TextureType::kTextureTypeCubemap;
		const auto ret = m_tex->init(&spec);
		ASSERT(ret == SCE_OK);
		m_tex->setBaseAddress(allocData);
		stbi_image_free(stbiData);
	}
	else
	{
		auto data = GetTextureFormat(bitsPerChannel, numChannels);
		m_tex = new sce::Gnm::Texture{};
		sce::Gnm::TextureSpec spec{};
		sce::Gnm::DataFormat form{};
		form.init(data.format, data.channelType, sce::Gnm::TextureChannel::kTextureChannelX, sce::Gnm::TextureChannel::kTextureChannelY, sce::Gnm::TextureChannel::kTextureChannelZ, sce::Gnm::TextureChannel::kTextureChannelW);
		spec.init();
		spec.m_depth = 1;
		spec.m_format = form;
		spec.m_width = width + paddingX;
		spec.m_height = height + paddingY;
		spec.m_tileModeHint = sce::Gnm::kTileModeDisplay_LinearAligned;
		spec.m_textureType = sce::Gnm::TextureType::kTextureType2d;
		const auto ret = m_tex->init(&spec);
		ASSERT(ret == SCE_OK);
		m_tex->setBaseAddress(allocData);
		stbi_image_free(stbiData);
	
	}

}

gfx::PS4Texture::PS4Texture(const unsigned char * a_texBuffer, const uint32_t a_width, const uint32_t a_height, const uint32_t a_bitsPerChannel, const uint32_t a_amountOfChannels, const TextureId a_id, ptl::FreeListResource & memResource, glm::vec2 minUVs)
	: PS4Resource(a_id), alloc(&memResource), uvMin(minUVs)
{
	auto data = GetTextureFormat(a_bitsPerChannel, a_amountOfChannels);

	m_tex = new sce::Gnm::Texture{};
	sce::Gnm::TextureSpec spec{};
	sce::Gnm::DataFormat form{};
	spec.init();
	spec.m_depth = 1;
	spec.m_width = a_width;
	spec.m_height = a_height;
	spec.m_tileModeHint = sce::Gnm::kTileModeDisplay_LinearAligned;

	const auto paddingX = ptl::alignTo<int>(a_width, 64) - a_width;
	const auto paddingY = ptl::alignTo<int>(a_height, 8) - a_height;
	const auto dataSize = (a_bitsPerChannel / 8) * (a_width + paddingX) * (a_height + paddingY) * a_amountOfChannels;
	uint8_t* texData = static_cast<uint8_t*>(memResource.allocate(dataSize, 256));

	// covert grayscale to RGBA
	if (a_amountOfChannels < 4)
	{
		memcpy(texData, a_texBuffer, a_width * a_height);
		form.init(data.format, data.channelType, sce::Gnm::TextureChannel::kTextureChannelX, sce::Gnm::TextureChannel::kTextureChannelX, sce::Gnm::TextureChannel::kTextureChannelX, sce::Gnm::TextureChannel::kTextureChannelX);
	}
	else
	{
		memcpy(texData, a_texBuffer, (a_width * a_height * sizeof(uint32_t)));
		form.init(data.format, data.channelType, sce::Gnm::TextureChannel::kTextureChannelX, sce::Gnm::TextureChannel::kTextureChannelY, sce::Gnm::TextureChannel::kTextureChannelZ, sce::Gnm::TextureChannel::kTextureChannelW);
	}

	spec.m_format = form;
	int ret = m_tex->init(&spec);
	ASSERT(ret == SCE_GNM_OK);
	m_tex->setBaseAddress(texData);
}

gfx::PS4Texture::PS4Texture(sce::Gnm::Texture& textureRef, TextureId textureId) : PS4Resource(textureId)
{
	m_tex = &textureRef;
}


gfx::PS4Texture::FormatData gfx::PS4Texture::GetTextureFormat(uint32_t a_bitsPerChannel, uint32_t a_amountOfChannels) const
{
	if (a_amountOfChannels == 4 && a_bitsPerChannel == 8)
	{
		return { sce::Gnm::kSurfaceFormat8_8_8_8, sce::Gnm::kTextureChannelTypeSrgb };
	}
	else if(a_amountOfChannels == 1 && a_bitsPerChannel == 8)
	{
		return { sce::Gnm::kSurfaceFormat8, sce::Gnm::kTextureChannelTypeSrgb };
	}

	ASSERT(false);
}

struct membuf : std::streambuf
{
	membuf(char* begin, char* end) {
		this->setg(begin, begin, end);
	}
};

gfx::PS4Texture::~PS4Texture()
{
	alloc->deallocate(m_tex->getBaseAddress());
	delete m_tex;
	m_tex = nullptr;
}
#endif