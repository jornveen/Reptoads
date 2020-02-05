#include "core/Config.h"
#include <glm/common.hpp>
#include "resource_loading/TtfHeaderParser.h"

#ifdef PLATFORM_PS
#include "rendering/gnm/PS4TextRenderer.h"
#include <mspace.h>
#include <libsysmodule.h>
#include <memory.h>
#include "core/Assertion.h"
#include <sce_font.h>
#include <ces.h>
#include <core/Defines.h>
#include "rendering/gnm/PS4Renderer.h"
#include <memory/Containers.h>
#include <kernel.h>
#include <fios2/fios2_all.h>


namespace gfx
{
	PS4TextRenderer::PS4TextRenderer(glm::vec<2, size_t> maxTextureDimensions, size_t maxSimultaneousTextEntities)
		: m_fontLibMallocInterface({
			FontMalloc, FontFree,
			FontRealloc, FontCalloc,
			(SceFontMspaceCreateFunction *)0,
			(SceFontMspaceDestroyFunction *)0
			})
		, resourceManager(nullptr)
		, m_scaleRatio(1.0f)
	{
	}

	void PS4TextRenderer::Initialize(IResourceManager& iResourceManager, glm::vec<2, size_t> a_screenSize)
	{
		resourceManager = &iResourceManager;
		// set the screen size, used to determine the relationship between font size and glyph pixel size on textures
		size_t size = MB(256);
		size_t alignment = MB(2);
		m_scaleRatio = a_screenSize.x / 1920;
		m_fontAllocator.initialize(size, SCE_KERNEL_WB_ONION, SCE_KERNEL_PROT_CPU_READ | SCE_KERNEL_PROT_CPU_WRITE | SCE_KERNEL_PROT_GPU_ALL);
		void* surfaceDirectMem = m_fontAllocator.allocate(size, alignment);
		ASSERT_MSG(surfaceDirectMem, "SURFACE DIRECT MEM FAILED");
		m_mspace = sceLibcMspaceCreate("surface mspace", surfaceDirectMem, size, 0);
		ASSERT_MSG(surfaceDirectMem, "MSPACE CREATION FAILED");
		CreateFontLibrary();

		// open the font(s)
		OpenFont(SCE_FONT_SET_SST_STD_EUROPEAN_JP_W1G);
	}


	void MemSet32(void* buffer, uint32_t value, uint32_t amount)
	{
		for (auto i = 0; i < amount; ++i) {
			*(static_cast<uint32_t*>(buffer) + i) = value;
		}
	}

	//#TODO, flip texture sizes of glyph and texture buffers???
	static uint64_t s_fontMemoryBuffer[MB(10) / sizeof(uint64_t)]; //10MiB

	sce::Gnm::TextureSpec PS4TextRenderer::CreateTextureSpec(glm::vec<2, size_t> a_textBounds)
	{
		sce::Gnm::TextureSpec spec{};
		sce::Gnm::DataFormat form{};
		form.init(sce::Gnm::kSurfaceFormat8_8_8_8, sce::Gnm::kTextureChannelTypeSrgb, sce::Gnm::TextureChannel::kTextureChannelX, sce::Gnm::TextureChannel::kTextureChannelY, sce::Gnm::TextureChannel::kTextureChannelZ, sce::Gnm::TextureChannel::kTextureChannelW);
		spec.init();
		spec.m_depth = 1;
		spec.m_format = form;
		spec.m_width = a_textBounds.x;
		spec.m_height = a_textBounds.y;
		spec.m_tileModeHint = sce::Gnm::kTileModeDisplay_LinearAligned;
		return spec;
	}

	float PS4TextRenderer::GetGlyphLineLengthFromString(ptl::string a_text, SceFontHandle font)
	{
		const uint8_t* utf8addr = reinterpret_cast<const uint8_t*>(a_text.c_str());

		uint32_t len;
		uint32_t ucode;
		float total = 0;

		for (int i = 0; i <= a_text.size() - 1; ++i) {
			SceFontGlyphMetrics metrics;
			int ret = sceCesUtf8ToUtf32(utf8addr, 4, &len, &ucode);
			ASSERT_MSG(ret == SCE_FONT_OK, "Converting Glyph failed.");
			sceFontGetCharGlyphMetrics(font, ucode, &metrics);
			utf8addr += len;
			total += metrics.Horizontal.advance;

			// return when newline
			if (ucode <= 0x0000001f)
				if (ucode == 0x0000000a)
					return total;
		}
		return total;
	}

	size_t GetNewLineCountFromString(ptl::string& a_text)
	{
		size_t newLineCount = 0;
		for (int i = 0; i < a_text.size() - 1; ++i) {
			if (a_text[i] == 0x0000001f || a_text[i] == 0x0000000a)
				++newLineCount;
		}
		return newLineCount;
	}

	void InvertString(ptl::string& a_text)
	{
		ptl::string v = a_text;
		int idx = 0;
		for (int i = v.size() - 1; i >= 0; --i) {
			a_text[idx] = v[i];
			++idx;
		}
	}

	uint32_t PS4TextRenderer::CharToUcode(const char* a_char)
	{
		ASSERT_MSG(m_systemFontHandle, "THE FONT HAS NOT BEEN SET YET!");
		uint32_t ucode;
		uint32_t offset;
		const auto ret = sceCesUtf8ToUtf32(reinterpret_cast<const uint8_t*>(a_char), 4, &offset, &ucode);
		ASSERT_MSG(ret == SCE_FONT_OK, "Converting Glyph failed.");
		return ucode;
	}

	uint32_t PS4TextRenderer::NextCharToUcode(const ptl::string a_text, const char* a_char, uint32_t& a_offset)
	{
		ASSERT_MSG(m_systemFontHandle, "THE FONT HAS NOT BEEN SET YET!");
		uint32_t ucode;
		const auto ret = sceCesUtf8ToUtf32(reinterpret_cast<const uint8_t*>(a_char), 4, &a_offset, &ucode);
		ASSERT_MSG(ret == SCE_FONT_OK, "Converting Glyph failed.");
		return ucode;
	}

	glm::vec2 PS4TextRenderer::GetStartPosition(TextProperties props, float lineHeight, SceFontHandle font)
	{
		glm::vec2 returnValue(0, 0);
		const float textBlockHeight = lineHeight * (GetNewLineCountFromString(props.str) + 1.);
		ASSERT_MSG(m_systemFontHandle, "THE FONT HAS NOT BEEN SET YET!");
		//auto invText = a_text;
		//InvertString(invText);

		// update the start x value
		// value represents center of glyph
		switch (props.ha) {
		case UIText::HorizontalAlignment::Leading:				returnValue.x = 0;																										break;
		case UIText::HorizontalAlignment::Center:				returnValue.x = static_cast<float>(props.texBounds.x) / 2.0f - GetGlyphLineLengthFromString(props.str, font) / 2.0f;	break;
		case UIText::HorizontalAlignment::Trailing:				returnValue.x = props.texBounds.x - GetGlyphLineLengthFromString(props.str, font);										break;
		default: ASSERT_MSG(false, "This alignment value has not been implemented yet.");
		}

		// update the start y value 
		switch (props.va) {
		case UIText::VerticalAlignment::Top:					returnValue.y = 0;													break;
		case UIText::VerticalAlignment::Center:					returnValue.y = props.texBounds.y * .5 - (textBlockHeight * .5);	break;
		case UIText::VerticalAlignment::Bottom:					returnValue.y = props.texBounds.y - textBlockHeight;				break;
		default: ASSERT_MSG(false, "This alignment value has not been implemented yet.");
		}

		return returnValue;
	}

	void PS4TextRenderer::DrawGlyphs(TextProperties props, uint8_t* glyphBuffer, SceFontHandle font, glm::vec<2,size_t> padding)
	{
		const float sizeToPCFontSize = .9f;
		const float scaleX = props.fontSize * sizeToPCFontSize;
		const float scaleY = props.fontSize * sizeToPCFontSize;

		float lineH;
		// set pixel scale and horizontal layout
		{
			SceFontHorizontalLayout HorizLayout;
			sceFontSetScalePixel(font, scaleX, scaleY);
			sceFontGetHorizontalLayout(font, &HorizLayout);
			//baseY = HorizLayout.baseLineY;
			lineH = HorizLayout.lineHeight;
		}

		int ret = sceFontBindRenderer(font, m_fontRenderer);
		ASSERT_MSG(ret == SCE_FONT_OK, "BINDING FONT RENDERER FAILED.");

		ret = sceFontSetupRenderScalePixel(font, scaleX, scaleY);
		ASSERT(ret == SCE_OK);

		// make sure the font's pixel size is set
		SceFontRenderSurface renderSurface;
		sceFontRenderSurfaceInit(&renderSurface, static_cast<void*>(glyphBuffer), props.texBounds.x + padding.x, 1, props.texBounds.x + padding.x, props.texBounds.y + padding.y);

		// reinterpret the c string to loop through the bytes
		const uint8_t* utf8addr = reinterpret_cast<const uint8_t*>(props.str.c_str());

		ptl::vector<float> offsets;

		// set the positioning values
		auto startValues = GetStartPosition(props, lineH, font);
		startValues.x += padding.x;
		startValues.y += padding.y;
		float x = startValues.x;
		float y = startValues.y;

		// reinterpret the c string to loop through the bytes
		utf8addr = reinterpret_cast<const uint8_t*>(props.str.c_str());

		// the data to track progress for the glyph renderer
		SceFontGlyphMetrics metrics;
		SceFontRenderResult result;

		// render the glyphs
		int charIdx = 0;
		while (charIdx < props.str.size()) {
			uint32_t len;
			uint32_t ucode;

			// get the next utf32 character
			int ret = sceCesUtf8ToUtf32(utf8addr, 4, &len, &ucode);
			ASSERT_MSG(ret == SCE_FONT_OK, "Converting Glyph failed.");
			// break when done
			if (ucode != 0x00000000) {
				utf8addr += len;

				// move downwards when encountering a newline char
				if (ucode <= 0x0000001f) {
					if (ucode == 0x0000000a) {
						if (props.ha == UIText::HorizontalAlignment::Center) {
							const auto total = GetGlyphLineLengthFromString(props.str.substr(++charIdx), font);
							x = static_cast<float>(props.texBounds.x) / 2.0f - total / 2.0f + padding.x;
						} else if (props.ha == UIText::HorizontalAlignment::Trailing) {
							const auto total = GetGlyphLineLengthFromString(props.str.substr(++charIdx), font);
							x = props.texBounds.x + padding.x - total;
						} else {
							++charIdx;
							x = startValues.x;
						}
						y += lineH;
					}
					continue;
				}

				if (y < props.texBounds.y + padding.y) {
					ret = sceFontRenderCharGlyphImage(font, ucode, &renderSurface, x, y, &metrics, &result);
					ASSERT_MSG(ret == SCE_FONT_OK, "Rendering glyph failed.");
					x += metrics.Horizontal.advance;
				}
			}
			++charIdx;
		}

		ret = sceFontUnbindRenderer(font);
		ASSERT_MSG(ret == SCE_FONT_OK, "UNBINDING FONT RENDERER FAILED.");
	}

	void PS4TextRenderer::CreateTextTexture(ui::UIText* text)
	{
		const glm::vec<2, size_t> textBounds(text->GetSize().x * m_scaleRatio, text->GetSize().y * m_scaleRatio);

		TextProperties texProps;
		texProps.fontSize = text->GetFontSize() * m_scaleRatio;
		texProps.texBounds = textBounds;
		texProps.str = text->GetOldText();
		texProps.ha = text->GetHorizontalAlignment();
		texProps.va = text->GetVerticalAlignment();
		texProps.font = text->GetFontPath();

		SceFontHandle fontHandle = GetFontHandle(texProps.font);
		const auto paddingX = ptl::alignTo<int>(texProps.texBounds.x, 64) - texProps.texBounds.x;
		const auto paddingY = ptl::alignTo<int>(texProps.texBounds.y, 8) - texProps.texBounds.y;
		const size_t texSizeInBytes = alignTo<size_t>((texProps.texBounds.x + paddingX) * (texProps.texBounds.y + paddingY), 256);
		uint8_t* texBuffer = new uint8_t[texSizeInBytes];

		// DEBUG SURFACES
		//memset((void*)texBuffer, 195, texSizeInBytes);

		// DEBUG PADDING
		memset((void*)texBuffer, 255, (texProps.texBounds.x + paddingX) * paddingY);
		for(int i = 0; i < texProps.texBounds.y + paddingY; ++i)
		{
			memset((void*)(texBuffer + (texProps.texBounds.x + paddingX) * i), 255, paddingX);
		}
		// END DEBUG PADDING

		if(!texProps.str.empty()) DrawGlyphs(texProps, texBuffer, fontHandle, glm::vec<2,size_t>(paddingX, paddingY));
		glm::vec2 minUvs = { 0.0f, 0.0f };
		if(paddingX + paddingY > 0) minUvs = { (float)paddingX / (texProps.texBounds.x + paddingX), (float)paddingY / (texProps.texBounds.y + paddingY) };
		text->SetTextTextureId(resourceManager->CreateTextureFromMemory(texBuffer, texProps.texBounds.x + paddingX, texProps.texBounds.y + paddingY, 8, 1, minUvs));
		delete[] texBuffer;
	}

	static uint64_t fontMemoryBuffer[10 * 1024 * 1024 / sizeof(uint64_t)]; //10MiB

	void PS4TextRenderer::CreateFontLibrary()
	{
		int ret = sceSysmoduleLoadModule(SCE_SYSMODULE_FONT);
		ASSERT_MSG(ret == SCE_OK, "MODULE LOADING FAILED");
		ret = sceSysmoduleLoadModule(SCE_SYSMODULE_FONT_FT);
		ASSERT_MSG(ret == SCE_OK, "MODULE LOADING FAILED");
		ret = sceSysmoduleLoadModule(SCE_SYSMODULE_FREETYPE_OT);
		ASSERT_MSG(ret == SCE_OK, "MODULE LOADING FAILED");

		m_moduleFont = SCE_SYSMODULE_FONT;
		m_moduleFont = SCE_SYSMODULE_FONT_FT;
		m_moduleFont = SCE_SYSMODULE_FREETYPE_OT;

		m_mspace = sceLibcMspaceCreate("font mspace", &fontMemoryBuffer[0], sizeof(fontMemoryBuffer), 0);
		ASSERT_MSG(m_mspace, "MSPACE CREATION FAILED");
		m_fontMemory = (SceFontMemory*)sceLibcMspaceCalloc(m_mspace, 1, sizeof(SceFontMemory));
		ASSERT_MSG(m_fontMemory, "FONT MEMORY ALLOCATION FAILED");
		ret = sceFontMemoryInit(m_fontMemory, &fontMemoryBuffer[0], sizeof(fontMemoryBuffer), &m_fontLibMallocInterface, (void*)m_mspace, FontMemoryDestroyCallback, (void*)0);
		ASSERT_MSG(ret == SCE_FONT_OK, "FONT MEMORY INIT FAILED");
		ret = sceFontCreateLibrary(m_fontMemory, sceFontSelectLibraryFt(0), &m_fontLib);
		ASSERT_MSG(ret == SCE_FONT_OK, "FONT LIBRARY CREATION FAILED");
		ret = sceFontSupportSystemFonts(m_fontLib);
		ASSERT_MSG(ret == SCE_FONT_OK, "SYSTEM FONT INIT FAILED");
		ret = sceFontSupportExternalFonts(m_fontLib, 32, SCE_FONT_FORMAT_OPENTYPE);
		ASSERT_MSG(ret == SCE_FONT_OK, "EXTERNAL FONT INIT FAILED");
		ret = sceFontAttachDeviceCacheBuffer(m_fontLib, NULL, 1 * 1024 * 1024);
		ASSERT_MSG(ret == SCE_FONT_OK, "DEVICE CACHE BUFFER INIT FAILED");
		ret = sceFontCreateRenderer(m_fontMemory, sceFontSelectRendererFt(0), &m_fontRenderer);
		ASSERT_MSG(ret == SCE_FONT_OK, "FONT RENDERER CREATION FAILED");
	}

	void PS4TextRenderer::OpenFont(uint32_t a_fontSetType)
	{
		SceFontOpenDetail* openDetail;
		uint32_t openFlag;
		int ret;

		openFlag = SCE_FONT_OPEN_FILE_STREAM;
		openDetail = (SceFontOpenDetail*)0;
		m_systemFontHandle = SCE_FONT_HANDLE_INVALID;
		ret = sceFontOpenFontSet(m_fontLib, a_fontSetType, openFlag, openDetail, &m_systemFontHandle);
		ASSERT_MSG(ret == SCE_FONT_OK, "OPENING FONT FAILED.");

		{
			/*E Determine how much memory is required to open the "/hostapp/data/fonts directory. */
			char directory[] = "/hostapp/data/fonts";
			SceFiosDH dh = SCE_FIOS_DH_INVALID;
			SceFiosBuffer buffer = SCE_FIOS_BUFFER_INITIALIZER;
			SceFiosSize bufferSize = 0;

			SceFiosOp op = sceFiosDHOpen(NULL, &dh, directory, buffer);
			int err = sceFiosOpWait(op);
			if (err == SCE_FIOS_ERROR_BAD_SIZE)
				bufferSize = sceFiosOpGetActualCount(op);
			else if (err != SCE_FIOS_OK) {
				printf("Unable to open directory %s\n", directory);
				ASSERT(false);
			}

			sceFiosOpDelete(op);

			/*E Now allocate the buffer (if needed) and traverse the directory. */
			if (bufferSize != 0) {
				buffer.set(ptl::DefaultMemoryResource::get_default_allocator()->allocate(bufferSize), bufferSize);

				err = sceFiosDHOpenSync(NULL, &dh, directory, buffer);
				assert(err == SCE_FIOS_OK);
			}

			for (;;) {
				SceFiosDirEntry entry = SCE_FIOS_DIRENTRY_INITIALIZER;
				err = sceFiosDHReadSync(NULL, dh, &entry);

				if (err == SCE_FIOS_ERROR_EOF)
					break;

				if (err != SCE_FIOS_OK) {
					printf("Error reading directory %s\n", directory);
					break;
				}

				printf("Read directory entry %s\n", entry.fullPath);

				if (entry.statFlags & SCE_FIOS_STATUS_READABLE) {
					//SceFontOpenDetail fontOpenDetail;
					SceFontHandle fontHandle = SCE_FONT_HANDLE_INVALID;
					int loadedFont = sceFontOpenFontFile(m_fontLib, entry.fullPath, SCE_FONT_OPEN_FILE_STREAM, SCE_FONT_OPEN_DETAIL_NONE, &fontHandle);
					if (loadedFont != SCE_OK) {
						std::cout << "CANNOT LOAD FONT!: " << entry.fullPath << "\n";
					} else {
						std::cout << "Opened font: " << entry.fullPath << "\n";

						ptl::string fontName = tbsg::GetFontNameFromTtf(entry.fullPath);
						fontNameToFontHandle.insert({ std::move(fontName), fontHandle });
						//SceFontLibrary whee;
						//
						//sceFontGetLibrary(m_systemFontHandle, &whee);
						
						//sceFontCloseFont(fontHandle);
					}
				}
			}

			sceFiosDHCloseSync(NULL, dh);

			if (buffer.pPtr) {
				ptl::DefaultMemoryResource::get_default_allocator()->deallocate(buffer.pPtr);
			}
		}
	}

	PS4TextRenderer::~PS4TextRenderer()
	{
		sceSysmoduleUnloadModule(m_moduleFont);
		sceSysmoduleUnloadModule(m_moduleFontFt);
		sceSysmoduleUnloadModule(m_moduleFreeType);
		sceFontUnbindRenderer(m_systemFontHandle);
		for(auto& nameAndFont : fontNameToFontHandle) {
			int closeSuccess = sceFontCloseFont(nameAndFont.second);
			ASSERT(closeSuccess == SCE_OK);
		}
	}

	void* PS4TextRenderer::FontMalloc(void* a_object, uint32_t a_size)
	{
		(void)a_object; // param not used, suppress warnings
		return malloc(a_size);
	}

	void PS4TextRenderer::FontFree(void* a_object, void* a_p)
	{
		(void)a_object; // param not used, suppress warnings
		free(a_p);
	}

	void* PS4TextRenderer::FontRealloc(void* a_object, void* a_p, uint32_t a_size)
	{
		(void)a_object; // param not used, suppress warnings
		return realloc(a_p, a_size);
	}

	void* PS4TextRenderer::FontCalloc(void* a_object, uint32_t a_nBlock, uint32_t a_size)
	{
		(void)a_object; // param not used, suppress warnings
		return calloc(a_nBlock, a_size);
	}

	void PS4TextRenderer::FontMemoryDestroyCallback(SceFontMemory* a_fontMemory, void* a_object, void* a_destroyArg)
	{
		(void)a_object;  // param not used, suppress warnings
		(void)a_destroyArg; // param not used, suppress warnings
		free((void*)a_fontMemory);
	}

	SceFontHandle PS4TextRenderer::GetFontHandle(const ptl::string& fontName)
	{
		auto it = fontNameToFontHandle.find(fontName);
		if(it == fontNameToFontHandle.end()) {
			return m_systemFontHandle;
		}

		return it->second;
	}
}
#endif
