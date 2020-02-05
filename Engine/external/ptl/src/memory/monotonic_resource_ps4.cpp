#include "memory/monotonic_resource.h"
#include "memory/details/mem_options.h"
#include "memory/details/mem_utils.h"

#include <gnm.h>
#include <kernel.h>
#include <scebase.h>
#include <mat.h>

namespace
{
    int32_t terminate(uint8_t* baseAddr,size_t* maxSize,off_t* dmemOffset,size_t* freeptr)
    {
        int32_t tmpRet, ret = SCE_OK;

        if (baseAddr)
        {
            tmpRet = sceKernelMunmap(baseAddr, *maxSize);
            if (tmpRet != SCE_OK)
            {
                printf("sceKernelMunmap failed: 0x%08X\n", tmpRet);
                ret = tmpRet;
            }
        }

        if (*dmemOffset)
        {
            tmpRet = sceKernelReleaseDirectMemory(*dmemOffset, *maxSize);
            if (tmpRet != SCE_OK)
            {
                printf("sceKernelReleaseDirectMemory failed: 0x%08X\n", tmpRet);
                ret = tmpRet;
            }
        }

        baseAddr = nullptr;
        dmemOffset = 0;
        freeptr = 0;
        maxSize = 0;

        return ret;
    }
}
namespace ptl
{

    MonotonicResource::MonotonicResource(const MemoryResourceOption& option, MemoryResource* upstream) : MemoryResource(upstream)
    {
        Init(option, upstream);
    }
    MonotonicResource::MonotonicResource(MemoryResourceOption&& option, MemoryResource* upstream) : MemoryResource(upstream)
    {
        Init(option, upstream);
    }

    void MonotonicResource::Init(const MemoryResourceOption& option, MemoryResource* upstream)
    {
        assert(option.blobSize % 16 == 0);
        auto ret = terminate(m_buffer, &m_buffer_size, &m_dmemOffset, &m_freeptr);
        if (ret != SCE_OK)
            assert(false);

        const size_t memoryAlignment = 64UL * 1024UL;
        m_buffer_size = alignToPow2(option.blobSize, memoryAlignment);


        ret = sceKernelAllocateDirectMemory(
            0,//start
            sceKernelGetDirectMemorySize() - 1,//end
            m_buffer_size,//size
            memoryAlignment,//
            option.mem_type,
            &m_dmemOffset);

        if (ret != SCE_OK) {
            if (SCE_KERNEL_ERROR_EAGAIN == ret) {
                assert(false);//"sceKernelAllocateDirectMemory failed SCE_KERNEL_ERROR_EINVAL"
            }
            else {
                assert(false);//"sceKernelAllocateDirectMemory failed SCE_KERNEL_ERROR_EAGAIN"
            }
        }
        void *baseAddress = nullptr;
        ret = sceKernelMapDirectMemory(
            &baseAddress,
            m_buffer_size,
            option.mem_protect,
            0,
            m_dmemOffset,
            memoryAlignment);
        if (ret != SCE_OK)
        {
            assert(false);//sceKernelMapDirectMemory failed"
        }
        sceMatAlloc(baseAddress, m_buffer_size, 0, 0);
        m_buffer = static_cast<uint8_t*>(baseAddress);
    }
    void* MonotonicResource::DoAllocate(size_t size, const size_t aligment)
    {
        const size_t offset = alignTo(m_freeptr, aligment);
        const size_t newSize = offset + size;
        if (newSize > m_buffer_size)
            return nullptr;
        m_freeptr = newSize;
        return m_buffer + offset;
    }

    bool MonotonicResource::DoDeallocate(void*& ptr, size_t size, size_t)
    {
        memset(ptr, 0xCC, size);// invalidates pointer
        return true; // does nothing!
    }
    bool MonotonicResource::DoDeallocate(void*& ptr, size_t size)
    {
        memset(ptr, 0xCC, size);// invalidates pointer
        return true; // does nothing!
    }
    bool MonotonicResource::DoIsEqual(const MemoryResource& rhs) const
    {
        return this == &rhs;
    }
    MonotonicResource::~MonotonicResource()
    {
        if (m_dmemOffset)
        {
            sceMatFree(m_buffer);
            auto tmpRet = sceKernelReleaseDirectMemory(m_dmemOffset, m_buffer_size);
            if (tmpRet != SCE_OK)
            {
                assert(false);//"Cannot release Memeory: One of the arguments is invalid"
            }
        }
    }
    void MonotonicResource::Release()
    {
        m_freeptr = 0;
    }
}
