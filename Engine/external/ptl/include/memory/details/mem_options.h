#pragma once
#ifdef __ORBIS__
#include <kernel/memory.h>
namespace ptl
{
    struct MemoryResourceOption
    {
        size_t blobSize{};
        SceKernelMemoryType mem_type{};
        int mem_protect{};
        size_t memoryAlingment{};


        MemoryResourceOption(size_t blobSize, size_t alignment, SceKernelMemoryType mem_type ,int memProtect)
            : blobSize(blobSize),
            mem_type(mem_type),
              mem_protect(memProtect),
            memoryAlingment(alignment)
        {
        }

        MemoryResourceOption(const MemoryResourceOption& other) = default;
        MemoryResourceOption(MemoryResourceOption&& other) noexcept = default;
        MemoryResourceOption& operator=(const MemoryResourceOption& other) = default;
        MemoryResourceOption& operator=(MemoryResourceOption&& other) noexcept = default;
    };
    struct PoolResourceOption : MemoryResourceOption
    {
        size_t chunckSize{};

        PoolResourceOption(size_t blobSize, size_t chunckSize, size_t chunckAlingment,SceKernelMemoryType mem_type, int memProtect)
            : MemoryResourceOption(blobSize, chunckAlingment, mem_type,memProtect),
            chunckSize(chunckSize)
        {
        }

        PoolResourceOption(const MemoryResourceOption& other, size_t chunckSize)
            : MemoryResourceOption(other),
            chunckSize(chunckSize)
        {
        }

        PoolResourceOption(MemoryResourceOption&& other, size_t chunckSize)
            : MemoryResourceOption(other),
            chunckSize(chunckSize)
        {
        }

        PoolResourceOption(const PoolResourceOption& other) = default;
        PoolResourceOption(PoolResourceOption&& other) noexcept = default;
        PoolResourceOption& operator=(const PoolResourceOption& other) = default;
        PoolResourceOption& operator=(PoolResourceOption&& other) noexcept = default;
    };
}
#else
namespace ptl
{
    struct MemoryResourceOption
    {
        size_t blobSize{};
        size_t memoryAlingment{};

        explicit MemoryResourceOption(size_t blobSize, size_t alignment)
            : blobSize(blobSize),
            memoryAlingment(alignment)
        {
        }


        MemoryResourceOption(const MemoryResourceOption& other) = default;
        MemoryResourceOption(MemoryResourceOption&& other) noexcept = default;
        MemoryResourceOption& operator=(const MemoryResourceOption& other) = default;
        MemoryResourceOption& operator=(MemoryResourceOption&& other) noexcept = default;
    };
    struct PoolResourceOption : MemoryResourceOption
    {
        size_t chunckSize{};
        PoolResourceOption(size_t blobSize, size_t chunckSize, size_t chunckAlingment)
            : MemoryResourceOption(blobSize, chunckAlingment),
            chunckSize(chunckSize)
        {
        }
        /*
        PoolResourceOption(size_t&& blobSize, size_t&& chunckSize, size_t&& chunckAlingment)
            : MemoryResourceOption(blobSize, chunckAlingment),
            chunckSize(chunckSize)
        {
        }*/

        PoolResourceOption(const MemoryResourceOption& other)
            : MemoryResourceOption(other),
            chunckSize(chunckSize)
        {
        }

        PoolResourceOption(MemoryResourceOption&& other)
            : MemoryResourceOption(other),
            chunckSize(chunckSize)
        {
        }

        PoolResourceOption(const PoolResourceOption& other) = default;
        PoolResourceOption(PoolResourceOption&& other) noexcept = default;
        PoolResourceOption& operator=(const PoolResourceOption& other) = default;
        PoolResourceOption& operator=(PoolResourceOption&& other) noexcept = default;
    };
}
#endif