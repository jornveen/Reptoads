#ifdef PLATFORM_PS
#pragma once
#include "PS4Resource.h"
#include "ClassID.h"
#include <gnm/texture.h>
#include "memory/string.h"

namespace ptl {
	class FreeListResource;
}

class LinearAllocator;

namespace gfx
{
	class PS4Texture : public PS4Resource, public ClassID<PS4Texture>
	{
	private:
		void SetBufferWithPadding(size_t srcWidth, size_t paddingX, size_t srcHeight, size_t paddingY,
			const uint8_t* srcBuffer, uint8_t* targetBuffer, size_t bitsPerChannel, size_t channelCount);
		struct FormatData { sce::Gnm::SurfaceFormat format; sce::Gnm::TextureChannelType channelType; };
		sce::Gnm::Texture* m_tex;
		ptl::FreeListResource* alloc;

	public:
		~PS4Texture();
		PS4Texture(	const ptl::string& a_path, const TextureId a_id, ptl::FreeListResource& memResource);
		PS4Texture(const unsigned char * a_texBuffer, const uint32_t a_width, const uint32_t a_height, const uint32_t a_bitsPerChannel, 
			const uint32_t a_amountOfChannels, const TextureId a_id, ptl::FreeListResource & memResource, glm::vec2 minUVs = {0.0f,0.0f});

		PS4Texture(sce::Gnm::Texture& textureRef, TextureId textureId);
		const glm::vec2& GetUVMin() const { return uvMin; }
		FormatData GetTextureFormat(uint32_t a_bitsPerChannel, uint32_t a_amountOfChannels) const;
		const sce::Gnm::Texture* GetGnmTexture() const;
		glm::vec2 uvMin{0,0};
	};
}
#endif
