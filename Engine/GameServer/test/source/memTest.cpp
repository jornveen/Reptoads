//#define CATCH_CONFIG_MAIN

//#include "catch/catch.hpp"
/*
#include "memory/monotonic_resourceh.h"
#include "memory/Allocator.h"

ptl::MonotonicResource g_mr(4 * 1024);

int alloc(size_t count)
{
    for (auto i = 0; i < count; ++i)
        g_mr.allocate(sizeof(int), alignof(int));
    return g_mr.GetNumberOfAlloc();
}


TEST_CASE("Memory Test", "[multi-file:2]")
{
    REQUIRE(1024 == alloc(1024));
}
*/