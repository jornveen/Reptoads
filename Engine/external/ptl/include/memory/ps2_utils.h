#pragma once
#ifndef _WIN32
#include <system_service.h>

#define USER_NEW()\
\
\
void *user_new(std::size_t size) throw(std::bad_alloc); \
void *user_new(std::size_t size, const std::nothrow_t& x) throw(); \
void *user_new_array(std::size_t size) throw(std::bad_alloc); \
void *user_new_array(std::size_t size, const std::nothrow_t& x) throw(); \
void user_delete(void *ptr) throw(); \
void user_delete(void *ptr, const std::nothrow_t& x) throw(); \
void user_delete_array(void *ptr) throw(); \
void user_delete_array(void *ptr, const std::nothrow_t& x) throw(); \
\
void *user_new(std::size_t size) throw(std::bad_alloc) \
{ \
void *ptr; \
\
if (size == 0) \
size = 1; \
\
ptr = ptl::DefaultMemoryResource::get_default_allocator()->allocate(size); \
return ptr; \
}\
\
void *user_new(std::size_t size, const std::nothrow_t& x) throw() \
{ \
void *ptr; \
\
(void)x; \
\
if (size == 0) \
size = 1; \
\
ptr = ptl::DefaultMemoryResource::get_default_allocator()->allocate(size); \
return ptr; \
}\
\
\
void *user_new_array(std::size_t size) throw(std::bad_alloc) \
{ \
return user_new(size); \
}\
\
\
void *user_new_array(std::size_t size, const std::nothrow_t& x) throw() \
{ \
return user_new(size, x); \
}\
\
\
void user_delete(void *ptr) throw() \
{\
\
if (ptr != nullptr) \
ptl::DefaultMemoryResource::get_default_allocator()->deallocate(ptr); \
}\
\
\
void user_delete(void *ptr, const std::nothrow_t& x) throw() \
{ \
(void)x; \
\
user_delete(ptr); \
}\
\
\
void user_delete_array(void *ptr) throw() \
{ \
user_delete(ptr); \
}\
\
void user_delete_array(void *ptr, const std::nothrow_t& x) throw() \
{ \
user_delete(ptr, x); \
} \




#define USER_MALLOC()\
extern "C" {\
int user_malloc_init(void);\
int user_malloc_finalize(void);\
void *user_malloc(size_t size);\
void user_free(void *ptr);\
void *user_calloc(size_t nelem, size_t size);\
void *user_realloc(void *ptr, size_t size);\
void *user_memalign(size_t boundary, size_t size);\
int user_posix_memalign(void **ptr, size_t boundary, size_t size);\
void *user_reallocalign(void *ptr, size_t size, size_t boundary);\
int user_malloc_stats(SceLibcMallocManagedSize *mmsize);\
int user_malloc_stats_fast(SceLibcMallocManagedSize *mmsize);\
size_t user_malloc_usable_size(void *ptr);\
}\
\
struct Header\
{\
    uint32_t size{};\
};\
\
Header& get_header(void* ptr)\
{\
    return static_cast<Header*>(ptr)[-1];\
}\
\
void* alloc(size_t size)\
{\
    void* ptr = ptl::DefaultMemoryResource::get_default_allocator()->allocate(size + sizeof(Header));\
    ptr = &static_cast<Header*>(ptr)[1];\
    get_header(ptr).size = static_cast<uint32_t>(size);\
    return ptr;\
}\
\
int user_malloc_init(void)\
{\
    ptl::DefaultMemoryResource::get_default_allocator();\
    return 0;\
}\
\
int user_malloc_finalize(void)\
{\
    sceSystemServiceReportAbnormalTermination(nullptr);\
}\
\
void *user_malloc(size_t size)\
{\
    return alloc(size);\
}\
void user_free(void *ptr)\
{\
    ptl::DefaultMemoryResource::get_default_allocator()->deallocate(ptr);\
}\
void *user_calloc(size_t nelem, size_t size)\
{\
    return alloc(nelem * size);\
}\
void *user_realloc(void *ptr, size_t size)\
{\
    void* newPtr = alloc(size);\
    \
    if (ptr) {\
        memcpy(newPtr, ptr, get_header(newPtr).size);\
        user_free(ptr);\
    }\
    \
    return newPtr;\
}\
void *user_memalign(size_t boundary, size_t size)\
{\
    sceSystemServiceReportAbnormalTermination(nullptr);\
}\
int user_posix_memalign(void **ptr, size_t boundary, size_t size)\
{\
    sceSystemServiceReportAbnormalTermination(nullptr);\
}\
void *user_reallocalign(void *ptr, size_t size, size_t boundary)\
{\
    sceSystemServiceReportAbnormalTermination(nullptr);\
}\
int user_malloc_stats(SceLibcMallocManagedSize *mmsize)\
{\
    sceSystemServiceReportAbnormalTermination(nullptr);\
}\
int   user_malloc_stats_fast(SceLibcMallocManagedSize *mmsize)\
{\
    sceSystemServiceReportAbnormalTermination(nullptr);\
}\
size_t user_malloc_usable_size(void *ptr)\
{\
    sceSystemServiceReportAbnormalTermination(nullptr);\
}\

#else

#define USER_NEW()
#define USER_MALLOC()

#endif