#include "memory/details/mem_options.h"
#include "memory/monotonic_resource.h"
#include "memory/pool_resource.h"
#include "memory/details/mem_utils.h"
#include <catch/catch.hpp>
#include "gameplay/Transfrom.h"


TEST_CASE("Memory monotonic allocation and dealllocation")
{
    ptl::MonotonicResource monotonic{ptl::MemoryResourceOption{100*1024*1024,32}};
    CHECK(monotonic.GetNumberOfAlloc() == 0);

    auto oldFreePtr = monotonic.GetFreePtr();
    void* ptr = monotonic.allocate(sizeof(int), alignof(int));
    CHECK(monotonic.GetNumberOfAlloc() == 1);
    CHECK(monotonic.GetFreePtr() == sizeof(int) + ptl::alignTo(oldFreePtr, alignof(int)));
    *static_cast<int*>(ptr) = 10;
    CHECK(*static_cast<int*>(ptr) == 10);
    monotonic.deallocate(ptr);
    CHECK(*static_cast<int*>(ptr) != 10);
    oldFreePtr = monotonic.GetFreePtr();
    int* intPtr = monotonic.allocate_object<int>(alignof(int),9);
    CHECK(monotonic.GetFreePtr() == sizeof(int) + ptl::alignTo(oldFreePtr, alignof(int)));
    CHECK(monotonic.GetNumberOfAlloc() == 2);
    CHECK(*intPtr == 9);
    monotonic.deallocate(intPtr);
    monotonic.Release();
    CHECK(monotonic.GetNumberOfAlloc() == 0);
    CHECK(monotonic.GetFreePtr() == 0);
}

TEST_CASE("Memory monotonic multi allocation primitive types")
{
    ptl::MonotonicResource monotonic{ ptl::MemoryResourceOption{100 * 1024 * 1024,32} };

    std::vector<int*> vec;
    auto size = 1024;
    vec.reserve(size / sizeof(int));
    for(auto i = 0;i < size / sizeof(int);i++)
    {
        vec.push_back(monotonic.allocate_object<int>(alignof(int), i));
        CHECK(*vec[i] == i);
    }
    for (auto i = 0; i < size / sizeof(int); i++)
    {
        CHECK(*vec[i] == i);
    }
    monotonic.Release();
    for (auto i = 0; i < size / sizeof(int); i++)
    {
        CHECK_FALSE(*vec[i] == i);
    }
    for (auto i = 0; i < size / sizeof(int); i++)
    {
        vec[i]=monotonic.allocate_object<int>(alignof(int), i);
        CHECK(*vec[i] == i);
    }
    for (auto i = 0; i < size / sizeof(int); i++)
    {
        CHECK(*vec[i] == i);
    }
    for (auto i = 0; i < size / sizeof(int); i++)
    {
        monotonic.deallocate_object<int>(vec[i],alignof(int));
        CHECK_FALSE(*vec[i] == i);
    }
}

struct Test
{
    int i;
    bool b;
    double d;
};

bool operator==(const Test& lhs,const Test& rhs)
{
    return lhs.b == rhs.b && lhs.d == rhs.d && lhs.i == rhs.i;
}

TEST_CASE("Memory monotonic multi allocation custom types")
{
    ptl::MonotonicResource monotonic{ ptl::MemoryResourceOption{100 * 1024 * 1024,32} };
    std::vector<Test*> vec;
    auto size = 1024;
    vec.reserve(size / sizeof(int));
    for (int i = 0; i < size / sizeof(int); i++)
    {
        vec.push_back(monotonic.allocate_object<Test>(alignof(Test), i,true,0.5));
        CHECK(*vec[i] == Test{i,true,0.5});
    }
    for (auto i = 0; i < size / sizeof(Test); i++)
    {
        CHECK(*vec[i] == Test{ i,true,0.5 });
    }
    monotonic.Release();
    for (auto i = 0; i < size / sizeof(Test); i++)
    {
        CHECK_FALSE(*vec[i] == Test{ i,true,0.5 });
    }
    for (auto i = 0; i < size / sizeof(Test); i++)
    {
        vec[i] = monotonic.allocate_object<Test>(alignof(Test), Test{ i,true,0.5 });
        CHECK(*vec[i] == Test{ i,true,0.5 });
    }
    for (auto i = 0; i < size / sizeof(Test); i++)
    {
        CHECK(*vec[i] == Test{ i,true,0.5 });
    }
    for (auto i = 0; i < size / sizeof(Test); i++)
    {
        monotonic.deallocate_object<Test>(vec[i], alignof(int));
        CHECK_FALSE(*vec[i] == Test{ i,true,0.5 });
    }
}

TEST_CASE("pool monotonic allocation and dealllocation")
{
    ptl::PoolResource pool{ ptl::PoolResourceOption{100 * 1024 * 1024,sizeof(int)+alignof(int),32},ptl::nulloc() };
    CHECK(pool.GetNumberOfAlloc() == 0);
    auto ptr = pool.allocate(sizeof(int));
    CHECK(ptr != nullptr);
    CHECK(pool.GetNumberOfAlloc() == 1);
    *static_cast<int*>(ptr) = 9;
    CHECK(*static_cast<int*>(ptr) == 9);
    pool.deallocate(ptr);
    CHECK(pool.GetNumberOfDealloc() == 1);
    CHECK_FALSE(*static_cast<int*>(ptr) == 9);
}
TEST_CASE("pool monotonic multi allocation primitive types")
{
    ptl::PoolResource pool{ ptl::PoolResourceOption{100 * 1024 * 1024,sizeof(int) + alignof(int),32},ptl::nulloc() };;

    std::vector<int*> vec;
    auto size = 1024;
    vec.reserve(size / sizeof(int));
    for (auto i = 0; i < size / sizeof(int); i++)
    {
        vec.push_back(pool.allocate_object<int>(alignof(int), i));
        CHECK(*vec[i] == i);
    }
    for (auto i = 0; i < size / sizeof(int); i++)
    {
        CHECK(*vec[i] == i);
    }
    for (auto i = 0; i < size / sizeof(int); i++)
    {
        pool.deallocate(vec[i], sizeof(int));
        CHECK_FALSE(*vec[i] == i);
    }
    for (auto i = 0; i < size / sizeof(int); i++)
    {
        vec[i] = pool.allocate_object<int>(alignof(int), i);
        CHECK(*vec[i] == i);
    }
    for (auto i = 0; i < size / sizeof(int); i++)
    {
        CHECK(*vec[i] == i);
    }
    for (auto i = 0; i < size / sizeof(int); i++)
    {
        pool.deallocate_object<int>(vec[i], alignof(int));
        CHECK_FALSE(*vec[i] == i);
    }
}