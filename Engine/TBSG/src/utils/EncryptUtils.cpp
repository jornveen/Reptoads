#include "utils/EncryptUtils.h"
#pragma once
#ifdef PLATFORM_PS
#define SCE_LIBSECURE_MEMORY_CONTAINER_SIZE	10*1024	

#include <kernel.h>
#include <libsecure.h>


SceLibSecureBlock g_libMemoryBlock = { nullptr, 0 };
#include "memory/default_memresource.h"
#endif

void utils::InitEncrypt()
{
#ifdef PLATFORM_PS
    g_libMemoryBlock.pointer = ptl::DefaultMemoryResource::get_default_monotonic()->allocate(SCE_LIBSECURE_MEMORY_CONTAINER_SIZE);
    g_libMemoryBlock.length = SCE_LIBSECURE_MEMORY_CONTAINER_SIZE;

    SceKernelUuid *p;
    SceKernelUuid *pBuf = static_cast<SceKernelUuid *>(g_libMemoryBlock.pointer);
    SceKernelUuid *pBufEnd = pBuf + g_libMemoryBlock.length;
    for (p = pBuf; p < pBufEnd; p++) {
        sceKernelUuidCreate(p);
    }
    p = (SceKernelUuid*)pBufEnd - sizeof(*p);
    sceKernelUuidCreate(p);

    sceLibSecureInit(SCE_LIBSECURE_FLAGS_ALL, &g_libMemoryBlock);

#endif
}

void utils::DeinitEncrypt()
{
#ifdef PLATFORM_PS
    if (g_libMemoryBlock.pointer) {
        memset(g_libMemoryBlock.pointer, 0, sizeof(char)*g_libMemoryBlock.length);
        free(g_libMemoryBlock.pointer);
    }
    g_libMemoryBlock.pointer = nullptr;
    g_libMemoryBlock.length = 0;

#endif
}
