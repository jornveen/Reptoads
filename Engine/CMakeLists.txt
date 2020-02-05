cmake_minimum_required (VERSION 3.5)
project (TBSGServer CXX)

set(CMAKE_CXX_FLAGS "-Wall --std=c++17 -O2")
set(CMAKE_C_COMPILER "/usr/bin/clang-6.0" CACHE PATH "" FORCE)
set(CMAKE_CXX_COMPILER "/usr/bin/clang++-6.0" CACHE PATH "" FORCE)


set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/CmakeModules)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

include_directories(
    ${OPENSSL_INCLUDE_DIR}

    ${CMAKE_SOURCE_DIR}/NetworkLib/include
    ${CMAKE_SOURCE_DIR}/DatabaseAPI/include
    ${CMAKE_SOURCE_DIR}/TBSGFramework/include
    ${CMAKE_SOURCE_DIR}/external/Enet/include
    ${CMAKE_SOURCE_DIR}/external/Cereal/include
    ${CMAKE_SOURCE_DIR}/external/Catch2
    ${CMAKE_SOURCE_DIR}/external/eventpp/include
    ${CMAKE_SOURCE_DIR}/external/cxxopts
    ${CMAKE_SOURCE_DIR}/external/FlatValueMap/include
    ${CMAKE_SOURCE_DIR}/external/ptl/include
    ${CMAKE_SOURCE_DIR}/external/rapidjson/include
    ${CMAKE_SOURCE_DIR}/external/mariadb/include
    ${CMAKE_SOURCE_DIR}/external/mariadb/include/mysql
    ${CMAKE_SOURCE_DIR}/external/sol2-develop
    ${CMAKE_SOURCE_DIR}/external/basiclogger/include
    ${CMAKE_SOURCE_DIR}/external/basiclogger/external/fmt/include
)

add_subdirectory(${CMAKE_SOURCE_DIR}/DatabaseAPI)
add_subdirectory(${CMAKE_SOURCE_DIR}/external/Enet)
add_subdirectory(${CMAKE_SOURCE_DIR}/external/mariadb)
add_subdirectory(${CMAKE_SOURCE_DIR}/external/ptl)
add_subdirectory(${CMAKE_SOURCE_DIR}/external/basiclogger)
add_subdirectory(${CMAKE_SOURCE_DIR}/NetworkLib)
add_subdirectory(${CMAKE_SOURCE_DIR}/TBSGFramework)

add_subdirectory(${CMAKE_SOURCE_DIR}/LobbyServer)
add_subdirectory(${CMAKE_SOURCE_DIR}/GameServer)

message("Copying assets to bin...")

file(
	COPY ${CMAKE_SOURCE_DIR}/../Projects/froggy/Server/config.json
	DESTINATION ${CMAKE_BINARY_DIR}/bin
)

file(
	COPY ${CMAKE_SOURCE_DIR}/../Projects/froggy/Server/data
	DESTINATION ${CMAKE_BINARY_DIR}/bin
)

message("Assets Copied")
