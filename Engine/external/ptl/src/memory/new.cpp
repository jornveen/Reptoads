#include "memory/default_memresource.h"
#include "memory/new.h"
#include <iostream>


void* operator new(std::size_t size)
{
    //auto ptr = ptl::DefaultMemoryResource::get_default_allocator()->allocate(size);
    //std::cout << "new address:" << ptr << '\n';
    return std::malloc(size);
}

void* operator new [](size_t _Size)
{

    //auto ptr =  ptl::DefaultMemoryResource::get_default_allocator()->allocate(_Size);
    //std::cout << "new[] address:" << ptr << '\n';
    return  std::malloc(_Size);
}

void* operator new(size_t _Size, std::nothrow_t const&) noexcept
{
    //auto ptr = ptl::DefaultMemoryResource::get_default_allocator()->allocate(_Size);
    //std::cout << "new address:" << ptr << '\n';
    return std::malloc(_Size);
}

void* operator new [](size_t _Size, std::nothrow_t const&) noexcept
{
    //auto ptr = ptl::DefaultMemoryResource::get_default_allocator()->allocate(_Size);
    //std::cout << "new[] address:" << ptr << '\n';
    return std::malloc(_Size);
}


void operator delete(void* ptr)
{
    if (ptr == nullptr) return;
    //std::cout << "delete address:" << ptr << '\n';
     //ptl::DefaultMemoryResource::get_default_allocator()->deallocate(ptr);
    std::free(ptr);
}

void operator delete [](void* _Block)
{
    if (_Block == nullptr) return;
    //std::cout << "delete[] address:" << _Block << '\n';
    //ptl::DefaultMemoryResource::get_default_allocator()->deallocate(_Block);
    std::free(_Block);
}

void operator delete(void* Block, std::nothrow_t const&)
{
    if (Block == nullptr) return;
    //std::cout << "delete address:" << Block << '\n';
    //ptl::DefaultMemoryResource::get_default_allocator()->deallocate(Block);
    std::free(Block);
}

void operator delete [](void* Block, std::nothrow_t const&)
{
    if (Block == nullptr) return;
    //std::cout << "delete[] address:" << Block << '\n';
    //ptl::DefaultMemoryResource::get_default_allocator()->deallocate(Block);
    std::free(Block);
}

void operator delete(void* _Block, size_t _Size)
{
    //std::cout << "delete address:" << _Block << '\n';
    //ptl::DefaultMemoryResource::get_default_allocator()->deallocate(_Block, _Size);
    if (_Block == nullptr) return;
    std::free(_Block);
}

void operator delete [](void* _Block, size_t _Size)
{
    if (_Block == nullptr) return;
    //std::cout << "delete[] address:" << _Block << '\n';
    //ptl::DefaultMemoryResource::get_default_allocator()->deallocate(_Block, _Size);
    std::free(_Block);
}

