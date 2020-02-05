project(TBSGFramework CXX)

find_package(Lua53 REQUIRED)

include_directories(
    ${PROJECT_SOURCE_DIR}/include
    ${LUA53_INCLUDE_DIR}

)

set(INCLUDE_FILES_PREFIX include)
set(TBSG_FRAMEWORK_INCLUDE_FILES
    ${INCLUDE_FILES_PREFIX}/core/Config.h
    ${INCLUDE_FILES_PREFIX}/resource_loading/ConfigLoader.h

)

set(SOURCE_FILES_PREFIX src)
set(TBSG_FRAMEWORK_SOURCE_FILES
    ${SOURCE_FILES_PREFIX}/core/Config.cpp
    ${SOURCE_FILES_PREFIX}/core/StrHash.cpp

    ${SOURCE_FILES_PREFIX}/resource_loading/ConfigLoader.cpp
    ${SOURCE_FILES_PREFIX}/resource_loading/ArgumentParsing.cpp

    ${SOURCE_FILES_PREFIX}/gameplay/scripting/LuaContext.cpp
    ${SOURCE_FILES_PREFIX}/gameplay/GamePlayHelper.cpp
)

add_library(TBSGFramework STATIC
    ${TBSG_FRAMEWORK_INCLUDE_FILES}
    ${TBSG_FRAMEWORK_SOURCE_FILES}
)