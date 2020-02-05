#include "memory/RapidJsonAllocator.h"
#include "memory/default_memresource.h"
#include <cstring>
#include <iostream>
#include "memory/Allocator.h"

RapidJsonAllocator::~RapidJsonAllocator()
{
    printf("relase memeory!\n");
    //ptl::DefaultMemoryResource::get_default_monotonic()->Release();
}

void* RapidJsonAllocator::Malloc(std::size_t size)
{
	//return malloc(size);
	return InternalMallocImpl(size);
}

void* RapidJsonAllocator::Realloc(void* originalPtr, size_t originalSize, size_t newSize)
{
	//void* newPtr = malloc(newSize);
	void* newPtr = InternalMallocImpl(newSize, originalPtr);

	memcpy(newPtr, originalPtr, originalSize);

	Free(originalPtr);

	return newPtr;
}

void RapidJsonAllocator::Free(void* ptr)
{
	//free(ptr);
	ptl::DefaultMemoryResource::get_default_allocator()->deallocate(ptr);
	//ptr = nullptr;
}

void* RapidJsonAllocator::InternalMallocImpl(std::size_t size, void* hint)
{
	void* ptr;
    if (hint == nullptr)
        ptr = ptl::DefaultMemoryResource::get_default_allocator()->allocate(size);
	else
        ptr = ptl::DefaultMemoryResource::get_default_allocator()->allocate(size,hint);
/*
	ptr = &static_cast<Header*>(ptr)[1];
	GetHeader(ptr).size = static_cast<uint32_t>(size);*/
	return ptr;
}

RapidJsonAllocator::Header& RapidJsonAllocator::GetHeader(void* ptr)
{
	assert(false && "this is no longer used!!!!!");
	return static_cast<Header*>(ptr)[-1];
}
