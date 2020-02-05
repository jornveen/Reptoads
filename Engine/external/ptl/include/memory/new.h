#pragma once
#include <new>

void* operator new  (std::size_t count);
void* operator new[](std::size_t count);

void* operator new  (std::size_t count, const std::nothrow_t& tag) noexcept;
void* operator new[](std::size_t count, const std::nothrow_t& tag) noexcept;
//
 void operator delete  (void* ptr);
void operator delete[](void* ptr);
void operator delete  (void* ptr, const std::nothrow_t& tag);
 void operator delete[](void* ptr, const std::nothrow_t& tag);
void operator delete  (void* ptr, std::size_t sz);
void operator delete[](void* ptr, std::size_t sz);