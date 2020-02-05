project(LobbyServer CXX)

set(INCLUDE_FILES_PREFIX include)
set(SOURCE_FILES_PREFIX source)

find_package(OpenSSL REQUIRED)

include_directories(
    ${PROJECT_SOURCE_DIR}/include
)

add_definitions( -DCOF_USE_LOGGER=1 )  

set(LOBBY_SERVER_INCLUDE_FILES
    ${INCLUDE_FILES_PREFIX}/Lobby.h
    ${INCLUDE_FILES_PREFIX}/LobbyServer.h
)

set(LOBBY_SERVER_SOURCE_FILES
    ${SOURCE_FILES_PREFIX}/Lobby.cpp
    ${SOURCE_FILES_PREFIX}/LobbyServer.cpp
    ${SOURCE_FILES_PREFIX}/main.cpp
)

add_executable(LobbyServer
    ${LOBBY_SERVER_INCLUDE_FILES}
    ${LOBBY_SERVER_SOURCE_FILES}
)

target_link_libraries(LobbyServer
    enet
    mariadbclientpp
    ptl
    TBSGFramework
    DatabaseAPI
    NetworkLib
    BasicLogger
    fmt
    ${OPENSSL_LIBRARIES}
)