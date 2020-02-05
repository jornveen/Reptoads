project(GameServer CXX)

set(INCLUDE_FILES_PREFIX include)
set(SOURCE_FILES_PREFIX source)

find_package(OpenSSL REQUIRED)
find_package(Lua53 REQUIRED)

include_directories(
    ${PROJECT_SOURCE_DIR}/include
    ${LUA53_INCLUDE_DIR}
)

add_definitions( -DCOF_USE_LOGGER=1 )  

set(GAME_SERVER_INCLUDE_FILES
    ${INCLUDE_FILES_PREFIX}/gameplay/DataStructs.h
    ${INCLUDE_FILES_PREFIX}/gameplay/MatchHelper.h

    ${INCLUDE_FILES_PREFIX}/model/AISystem.h
    ${INCLUDE_FILES_PREFIX}/model/GameLogic.h

    ${INCLUDE_FILES_PREFIX}/scripting/LuaBindingFunc.h

    ${INCLUDE_FILES_PREFIX}/utils/CMDInput.h

    ${INCLUDE_FILES_PREFIX}/GameServer.h
    ${INCLUDE_FILES_PREFIX}/RTSARG.h
)

set(GAME_SERVER_SOURCE_FILES
    ${SOURCE_FILES_PREFIX}/gameplay/MatchHelper.cpp
  
    ${SOURCE_FILES_PREFIX}/model/AISystem.cpp
    ${SOURCE_FILES_PREFIX}/model/GameLogic.cpp

    ${SOURCE_FILES_PREFIX}/scripting/LuaBindingFunc.cpp

    ${SOURCE_FILES_PREFIX}/utils/CMDInput.cpp

    ${SOURCE_FILES_PREFIX}/GameServer
    ${SOURCE_FILES_PREFIX}/main.cpp
)

add_executable(GameServer
    ${GAME_SERVER_INCLUDE_FILES}
    ${GAME_SERVER_SOURCE_FILES}
)

target_link_libraries(GameServer
    enet
    mariadbclientpp
    ptl
    TBSGFramework
    NetworkLib
    BasicLogger
    fmt
    DatabaseAPI
    -lstdc++fs
    ${OPENSSL_LIBRARIES}
    ${LUA53_LIBRARIES}
)