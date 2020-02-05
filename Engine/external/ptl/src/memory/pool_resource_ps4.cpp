#include "memory/pool_resource.h"
#include "memory/pool_resource.h"
#include "memory/details/mem_options.h"

#include <sys/_defines/_sce_ok.h>
#include <_kernel.h>
#include <system_service.h>
#include "memory/details/mem_utils.h"
#include <iostream>
#include <mat.h>

#define MATGROUP_POOL_ALLOCATOR MatGroup(2)

namespace
{
    int32_t terminate(uint8_t* baseAddr, size_t* maxSize, off_t* dmemOffset)
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
        maxSize = 0;

        return ret;
    }
}
namespace ptl
{
    void* PoolResource::DoAllocate(size_t size, const size_t aligment)
    {
        if (m_maxChuncks <= m_usedChuncks)
        {
            return nullptr;
        };
        if (size >= m_chunckSize)
        {
            return nullptr;
        };
        if (aligment > m_chunckAligment)
        {
            return nullptr;
        }
        void* p = _free_list;
        _free_list = (void**)(*_free_list);
        ++m_usedChuncks;

		sceMatAlloc(p, m_chunckSize, 0, MATGROUP_POOL_ALLOCATOR);
        return p;
    }

    bool PoolResource::DoDeallocate(void*& ptr, size_t, size_t)
    {
        *((void**)ptr) = _free_list;
        _free_list = (void**)ptr;
        --m_usedChuncks;

		sceMatFree(ptr);
        return true;
    }

    bool PoolResource::DoDeallocate(void*& ptr, size_t)
    {
        *((void**)ptr) = _free_list;
        _free_list = (void**)ptr;
        --m_usedChuncks;
        return true;
    }

    bool PoolResource::DoIsEqual(const MemoryResource& rhs) const
    {
        return this == &rhs;
    }

    PoolResource::PoolResource(PoolResourceOption& option, MemoryResource* upstream) : MemoryResource(upstream), m_chunckSize(option.chunckSize), m_blobSize(option.blobSize)
    {
        Init(option);
    }
    PoolResource::PoolResource(PoolResourceOption&& option, MemoryResource* upstream) : MemoryResource(upstream), m_chunckSize(option.chunckSize), m_blobSize(option.blobSize)
    {
        Init(option);
    }

    void PoolResource::Init(PoolResourceOption& option)
    {
        ///////////////////////// ALLOCATE MEMEORY /////////////////////////////////////
        auto ret = terminate(m_blob, &m_blobSize, &m_dmemOffset);
        if (ret != SCE_OK)
            assert(false && "terminate does not work");
        option.memoryAlingment = 64UL * 1024UL;
        m_blobSize = alignToPow2(option.blobSize, option.memoryAlingment);
        ret = sceKernelAllocateDirectMemory(
            0,//start
            sceKernelGetDirectMemorySize() - 1,//end
            m_blobSize,//size
            option.memoryAlingment,//
            option.mem_type,
            &m_dmemOffset);

        if (ret != SCE_OK) {
            if (SCE_KERNEL_ERROR_EAGAIN == ret) {
                sceSystemServiceReportAbnormalTermination(nullptr);
            }
            else {
                sceSystemServiceReportAbnormalTermination(nullptr);
            }
        }

        void *baseAddress = nullptr;
        ret = sceKernelMapDirectMemory(
            &baseAddress,
            m_blobSize,
            option.mem_protect,
            0,
            m_dmemOffset,
            option.memoryAlingment);
        if (ret != SCE_OK)
        {
            sceSystemServiceReportAbnormalTermination(nullptr);
        }
        sceMatAlloc(baseAddress, m_blobSize, 0, 0);
        m_chunckAligment = option.memoryAlingment;
        m_blob  = static_cast<uint8_t*>(baseAddress);
        auto space = m_blobSize;
        auto adjustment = align_offset(m_chunckAligment, m_chunckSize, m_blob, space);

        _free_list = (void**)add(m_blob, adjustment);
        m_maxChuncks = (m_blobSize - adjustment) / m_chunckSize;
        void** p = _free_list;
        //Initialize free blocks list 
        for (size_t i = 0; i < m_maxChuncks - 1; i++)
        {
            *p = add(p, m_chunckSize);
            p = (void**)*p;
        }
        *p = nullptr;


		sceMatAllocPhysicalMemory((uint64_t)m_blob, m_blobSize, option.memoryAlingment, option.mem_type);
    }
 
    PoolResource::~PoolResource()
    {
        sceMatFree(m_blob);
        auto ret = terminate(m_blob, &m_blobSize, &m_dmemOffset);
        if (ret != SCE_OK)
            assert(false && "terminate does not work");
    }
}
