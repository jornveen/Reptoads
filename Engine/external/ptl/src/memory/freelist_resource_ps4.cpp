#include "memory/freelist_resources.h"
#include "memory/details/mem_options.h"
#include "memory/details/mem_utils.h"
#include <sys/_defines/_sce_ok.h>
#include <_kernel.h>
#include <mat.h>

#define MATGROUP_FREELIST_ALLOCATOR MatGroup(1)

//https://www.gamedev.net/articles/programming/general-and-gameplay-programming/c-custom-memory-allocation-r3010/
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
    void* alloc(
        size_t blob_size,
        size_t alignment,
        SceKernelMemoryType mem_type,
        int mem_proect,
        uint8_t* baseAddr, size_t* buffer_size, off_t* dmemOffset)
    {
        int32_t ret{};
        const size_t memoryAlignment = 64UL * 1024UL;
        *buffer_size = ptl::alignToPow2(blob_size, memoryAlignment);

        ret = sceKernelAllocateDirectMemory(
            0,//start
            sceKernelGetDirectMemorySize() - 1,//end
            *buffer_size,//size
            memoryAlignment,//
            mem_type,
            dmemOffset);

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
            *buffer_size,
            mem_proect,
            0,
            *dmemOffset,
            memoryAlignment);
        if (ret != SCE_OK)
        {
            assert(false);//sceKernelMapDirectMemory failed"
        }
        sceMatAllocPhysicalMemory((uint64_t)baseAddress, *buffer_size, memoryAlignment, mem_type);
        return baseAddress;
    }

}

void* ptl::FreeListResource::DoAllocate(size_t size, const size_t alignment)
{
    assert(size != 0 && alignment != 0);
    FreeBlock* prev_free_block = nullptr;
    FreeBlock* free_block = m_freeBlocks;

    while (free_block != nullptr)
    {
        uintptr_t adjustment = alignForwardAdjustmentWithHeader<Header>(free_block, alignment);
        size_t total_size = size + adjustment;
        if (free_block->size < total_size)
        {
            prev_free_block = free_block;
            free_block = free_block->next;
            continue;
        }
        assert(sizeof(Header) >= sizeof(FreeBlock)&& "sizeof(Header) < sizeof(FreeBlock)");
        //If allocations in the remaining memory will be impossible 
        if (free_block->size - total_size <= sizeof(Header))
        {
            //Increase allocation size instead of creating a new FreeBlock 
            total_size = free_block->size;

            if (prev_free_block != nullptr)
                prev_free_block->next = free_block->next;
            else
                m_freeBlocks = free_block->next;
        }
        else
        {
            //Else create a new FreeBlock containing remaining memory 
            FreeBlock* next_block = (FreeBlock*)(add(free_block, total_size));

            next_block->size = free_block->size - total_size;
            next_block->next = free_block->next;

            if (prev_free_block != nullptr)
                prev_free_block->next = next_block;
            else
                m_freeBlocks = next_block;
        }
        uintptr_t aligned_address = (uintptr_t)free_block + adjustment;
        Header* header = (Header*)(aligned_address - sizeof(Header));
        header->size = total_size;
        header->adjustment = adjustment;
        m_usedMemory += total_size;
        m_numAllocations++;

        assert(alignForwardAdjustment((void*)aligned_address, alignment) == 0);

		sceMatAlloc((void*)aligned_address, header->size, header->adjustment, MATGROUP_FREELIST_ALLOCATOR);
        return (void*)aligned_address;
    }
    return nullptr;
}

bool ptl::FreeListResource::DoDeallocate(void*& ptr, size_t size, size_t aligment)
{
    return DoDeallocate(ptr, aligment);
}

bool ptl::FreeListResource::DoDeallocate(void*& ptr, size_t aligment)
{

    assert(ptr != nullptr);
    Header* header = (Header*)subtract(ptr, sizeof(Header));
    uintptr_t block_start = reinterpret_cast<uintptr_t>(ptr) - header->adjustment;
    size_t block_size = header->size;
    uintptr_t block_end = block_start + block_size;
    FreeBlock* prev_free_block = nullptr;
    FreeBlock* free_block = m_freeBlocks;

    while (free_block != nullptr)
    {
        if ((uintptr_t)free_block >= block_end) break;
        prev_free_block = free_block;
        free_block = free_block->next;
    }

    if (prev_free_block == nullptr)
    {
        prev_free_block = (FreeBlock*)block_start;
        prev_free_block->size = block_size;
        prev_free_block->next = m_freeBlocks;
        m_freeBlocks = prev_free_block;
    }
    else if ((uintptr_t)prev_free_block + prev_free_block->size == block_start)
    {
        prev_free_block->size += block_size;
    }
    else
    {
        FreeBlock* temp = (FreeBlock*)block_start;
        temp->size = block_size;
        temp->next = prev_free_block->next;
        prev_free_block->next = temp;
        prev_free_block = temp;
    }

    if (free_block != nullptr && (uintptr_t)free_block == block_end)
    {
        prev_free_block->size += free_block->size;
        prev_free_block->next = free_block->next;
    }

    m_numAllocations--;
    m_usedMemory -= block_size;
	sceMatFree(ptr);
    return true;

}

bool ptl::FreeListResource::DoIsEqual(const MemoryResource& rhs) const
{
    return false;
}

ptl::FreeListResource::FreeListResource(MemoryResourceOption&& option, MemoryResource* upstream):
MemoryResource(upstream), m_blobSize(option.blobSize),m_align(option.memoryAlingment)
{
    assert(m_blobSize % 16 == 0);
    auto ret = terminate(m_baseAddress, &m_blobSize, &m_dmemOffset);
    if (ret != SCE_OK)
        assert(false);
    //Allocate memmory

    void* addr = alloc(m_blobSize, m_align, option.mem_type, option.mem_protect, m_baseAddress, &m_blobSize, &m_dmemOffset);
    m_baseAddress = (uint8_t*)(addr);
    m_freeBlocks = (FreeBlock*)(m_baseAddress);
    m_freeBlocks->next = nullptr;
    m_freeBlocks->size = m_blobSize;
}
