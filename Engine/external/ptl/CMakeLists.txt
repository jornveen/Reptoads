project(ptl CXX)

include_directories(
    ${PROJECT_SOURCE_DIR}/include
)

set(INCLUDE_FILES_PREFIX include)
set(PTL_INCLUDE_FILES
    ${INCLUDE_FILES_PREFIX}/core/SparseSet.h

    ${INCLUDE_FILES_PREFIX}/memory/details/mem_defines.h
    ${INCLUDE_FILES_PREFIX}/memory/details/mem_linked_list.h
    ${INCLUDE_FILES_PREFIX}/memory/details/mem_options.h
    ${INCLUDE_FILES_PREFIX}/memory/details/mem_utils.h

    ${INCLUDE_FILES_PREFIX}/memory/Allocator.h
    ${INCLUDE_FILES_PREFIX}/memory/allocator_impl.h
    ${INCLUDE_FILES_PREFIX}/memory/Containers.h
    ${INCLUDE_FILES_PREFIX}/memory/default_memresource.h
    ${INCLUDE_FILES_PREFIX}/memory/freelist_resources.h
    ${INCLUDE_FILES_PREFIX}/memory/malloc_resource.h
    ${INCLUDE_FILES_PREFIX}/memory/memory_resource.h
    ${INCLUDE_FILES_PREFIX}/memory/monotonic_resource.h
    ${INCLUDE_FILES_PREFIX}/memory/new.h
    ${INCLUDE_FILES_PREFIX}/memory/pool_resource.h
    ${INCLUDE_FILES_PREFIX}/memory/ps2_utils.h
    ${INCLUDE_FILES_PREFIX}/memory/RapidJsonAllocator.h
    ${INCLUDE_FILES_PREFIX}/memory/smart_ptr.h
    ${INCLUDE_FILES_PREFIX}/memory/String.h


)

set(SOURCE_FILES_PREFIX src)
set(PTL_SOURCE_FILES
    ${SOURCE_FILES_PREFIX}/memory/Allocator.cpp
    ${SOURCE_FILES_PREFIX}/memory/default_memresource.cpp
    ${SOURCE_FILES_PREFIX}/memory/freelist_resources_win.cpp
    ${SOURCE_FILES_PREFIX}/memory/malloc_resource.cpp
    ${SOURCE_FILES_PREFIX}/memory/monotonic_resource_win.cpp
    ${SOURCE_FILES_PREFIX}/memory/new.cpp
    ${SOURCE_FILES_PREFIX}/memory/pool_resource.cpp
    ${SOURCE_FILES_PREFIX}/memory/RapidJsonAllocator.cpp
    ${SOURCE_FILES_PREFIX}/memory/String.cpp

)

add_library(ptl STATIC
    ${PTL_INCLUDE_FILES}
    ${PTL_SOURCE_FILES}
)