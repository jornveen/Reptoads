project(NetworkLib CXX)

include_directories(
    ${PROJECT_SOURCE_DIR}/include
)

set(INCLUDE_FILES_PREFIX include)
set(NETWORKLIB_INCLUDE_FILES
    ${INCLUDE_FILES_PREFIX}/Crypto/KeyChain.h
    ${INCLUDE_FILES_PREFIX}/Crypto/NetRSA.h
    ${INCLUDE_FILES_PREFIX}/Crypto/NetAES.h

    ${INCLUDE_FILES_PREFIX}/Net/Client.h
    ${INCLUDE_FILES_PREFIX}/Net/Connection.h
    ${INCLUDE_FILES_PREFIX}/Net/NetCommands.h
    ${INCLUDE_FILES_PREFIX}/Net/NetUtils.h
    ${INCLUDE_FILES_PREFIX}/Net/Packet.h
    ${INCLUDE_FILES_PREFIX}/Net/Server.h

    ${INCLUDE_FILES_PREFIX}/Utility/NetUtils.h
    ${INCLUDE_FILES_PREFIX}/Utility/Observable.h

)

set(SOURCE_FILES_PREFIX source)
set(NETWORKLIB_SOURCE_FILES
    ${SOURCE_FILES_PREFIX}/Crypto/NetRSA.cpp
    ${SOURCE_FILES_PREFIX}/Crypto/NetAES.cpp

    ${SOURCE_FILES_PREFIX}/Net/win/WINPacket.cpp
    ${SOURCE_FILES_PREFIX}/Net/Client.cpp
    ${SOURCE_FILES_PREFIX}/Net/Connection.cpp
    ${SOURCE_FILES_PREFIX}/Net/Server.cpp

    ${SOURCE_FILES_PREFIX}/Utility/Utils.cpp
)

add_library(NetworkLib STATIC
    ${NETWORKLIB_INCLUDE_FILES}
    ${NETWORKLIB_SOURCE_FILES}
)

target_link_libraries(NetworkLib enet)