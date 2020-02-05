#pragma once
#include <cstddef>
#include <cstdint>
#include "memory/monotonic_resource.h"
#include "details/mem_options.h"

/// A rapidjson concept wrapper around our own memory allocator.
/// \note implements rapidjson::allocator concept
class RapidJsonAllocator
{
	struct Header
	{
		uint32_t size{};
	};
public:
    ~RapidJsonAllocator();
	// ReSharper disable once CppInconsistentNaming

	//!< Whether this allocator needs to call Free().
	static const bool kNeedFree = true;

	// Allocate a memory block.
	// \param size of the memory block in bytes.
	// \returns pointer to the memory block.
	void* Malloc(std::size_t size);

	// Resize a memory block.
	// \param originalPtr The pointer to current memory block. Null pointer is permitted.
	// \param originalSize The current size in bytes. (Design issue: since some allocator may not book-keep this, explicitly pass to it can save memory.)
	// \param newSize the new size in bytes.
	void* Realloc(void* originalPtr, size_t originalSize, size_t newSize);

	// Free a memory block.
	// \param pointer to the memory block. Null pointer is permitted.
	static void Free(void *ptr);

private:
	static void* InternalMallocImpl(std::size_t size, void* hint = nullptr);
	static Header& GetHeader(void* ptr);
};