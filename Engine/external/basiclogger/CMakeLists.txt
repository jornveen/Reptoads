project(BasicLogger CXX)

add_subdirectory(${PROJECT_SOURCE_DIR}/external/fmt)

include_directories(
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/external/fmt/include
)

add_definitions( -DCOF_USE_LOGGER=1 )  

set(INCLUDE_FILES_PREFIX include)
set(LOGGER_INCLUDE_FILES
    ${INCLUDE_FILES_PREFIX}/Details/GlobalLoggerInstance.h

    ${INCLUDE_FILES_PREFIX}/Sinks/File.h
    ${INCLUDE_FILES_PREFIX}/Sinks/Sink.h
    ${INCLUDE_FILES_PREFIX}/Sinks/StdOut.h

    ${INCLUDE_FILES_PREFIX}/BasicLogger.h
    ${INCLUDE_FILES_PREFIX}/Logger.h
    ${INCLUDE_FILES_PREFIX}/LoggingFunction.h
    ${INCLUDE_FILES_PREFIX}/LoggingMacros.h

)

set(SOURCE_FILES_PREFIX src)
set(LOGGER_SOURCE_FILES
    ${SOURCE_FILES_PREFIX}/Details/LoggingFunction.cpp

    ${SOURCE_FILES_PREFIX}/Sinks/File.cpp
    ${SOURCE_FILES_PREFIX}/Sinks/Sink.cpp
    ${SOURCE_FILES_PREFIX}/Sinks/StdOut.cpp
)

add_library(BasicLogger STATIC
    ${LOGGER_INCLUDE_FILES}
    ${LOGGER_SOURCE_FILES}
)

