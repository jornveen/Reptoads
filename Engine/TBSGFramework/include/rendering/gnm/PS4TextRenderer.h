#include "PS4ResourceManager.h"
#ifdef PLATFORM_PS
#pragma once
#include <cstdint>
#include <sce_font/libfont.h>
#include <common/allocator.h>
#include <mspace.h>
#include <glm/vec2.hpp>
#include <queue>
#include <unordered_map>
#include <string>
#include "rendering/IUIRenderer.h"
#include "ui/UIText.h"
namespace gfx 
{
	class PS4Renderer;


	class PS4TextRenderer
	{
		struct InitResult { TextureId textureId; void* glyphBuffer; };
		struct TextProperties { ptl::string str; float fontSize; glm::vec<2, size_t> texBounds;  UIText::HorizontalAlignment ha;  UIText::VerticalAlignment va; ptl::string font; };
		struct TextureEntity { TextureId tex; LinearAllocator* alloc; };

	public:
		PS4TextRenderer(glm::vec<2, size_t> maxTextureDimensions = { 1920,1080 }, size_t maxSimultaneousTextEntities = 2);
		void Initialize(IResourceManager& iResourceManager, glm::vec<2, size_t> a_screenSize);
		float GetGlyphLineLengthFromString(ptl::string a_text, SceFontHandle font);
		void CreateTextTexture(ui::UIText* text);

		~PS4TextRenderer();
	private:
		static void* FontMalloc(void* a_object, uint32_t a_size);
		static void FontFree(void* a_object, void* a_p);
		static void* FontRealloc(void* a_object, void* a_p, uint32_t a_size);
		static void* FontCalloc(void* a_object, uint32_t a_nBlock, uint32_t a_size);
		static void FontMemoryDestroyCallback(SceFontMemory* fontMemory, void* object, void* destroyArg);
		SceFontHandle GetFontHandle(const ptl::string& fontName);
		uint32_t CharToUcode(const char* a_char);
		uint32_t NextCharToUcode(const ptl::string a_text, const char* a_char, uint32_t& a_offset);
		glm::vec2 GetStartPosition(TextProperties props, float lineHeight, SceFontHandle font);
		void DrawGlyphs(TextProperties props, uint8_t* glyphBuffer, SceFontHandle font, glm::vec<2, size_t> padding);
		sce::Gnm::TextureSpec CreateTextureSpec(glm::vec<2, size_t> a_textBounds);
		void CreateFontLibrary();
		void OpenFont(uint32_t a_fontSetType);
		IResourceManager* resourceManager;
		SceLibcMspace m_mspace;
		SceFontLibrary m_fontLib;
		SceFontMemory* m_fontMemory;
		LinearAllocator m_fontAllocator;
		LinearAllocator m_allocAllocator;
		const SceFontMemoryInterface m_fontLibMallocInterface;
		SceFontRenderer m_fontRenderer;
		SceFontHandle m_systemFontHandle;
		float m_scaleRatio;
		int m_moduleFont;
		int m_moduleFontFt;
		int m_moduleFreeType;

		ptl::unordered_map<ptl::string, SceFontHandle> fontNameToFontHandle{};
	};
}
#endif