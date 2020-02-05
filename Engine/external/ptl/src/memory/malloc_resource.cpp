#include "memory/malloc_resource.h"
#include "memory/memory_resource.h"
#include <iostream>

void* ptl::MallocResource::DoAllocate(size_t size, const size_t )
{
    return std::malloc(size);
}

bool ptl::MallocResource::DoDeallocate(void*& ptr, size_t , size_t )
{
    //std::cout << "free: "<< ptr <<"\n";
    std::free(ptr);
    //ptr = nullptr;
    return true;
}

bool ptl::MallocResource::DoDeallocate(void*& ptr, size_t )
{
    //std::cout << "free()\n";
    //std::cout << ptr << std::endl;
    std::free(ptr);
    //ptr = nullptr;
    return true;
}

bool ptl::MallocResource::DoIsEqual(const MemoryResource&) const
{
    return true;
}
