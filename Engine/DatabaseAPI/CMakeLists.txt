project(DatabaseAPI CXX)

include_directories(
    ${PROJECT_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/external/mariadb/include
)

add_definitions( -DCOF_USE_LOGGER=1 )  

set(INCLUDE_FILES_PREFIX include)
set(DATABASE_INCLUDE_FILES
    ${INCLUDE_FILES_PREFIX}/DatabaseAPI.h
    ${INCLUDE_FILES_PREFIX}/DatabaseCredentials.h
    ${INCLUDE_FILES_PREFIX}/GameDataDatabase.h
    ${INCLUDE_FILES_PREFIX}/Payloads.h
    ${INCLUDE_FILES_PREFIX}/ProfileDatabase.h
)

set(SOURCE_FILES_PREFIX source)
set(DATABASE_SOURCE_FILES
    ${SOURCE_FILES_PREFIX}/DatabaseAPI.cpp
    ${SOURCE_FILES_PREFIX}/GameDataDatabase.cpp
    ${SOURCE_FILES_PREFIX}/ProfileDatabase.cpp
)

add_library(DatabaseAPI STATIC
    ${DATABASE_INCLUDE_FILES}
    ${DATABASE_SOURCE_FILES}
)

target_link_libraries(DatabaseAPI
    mariadbclientpp
    ptl
)