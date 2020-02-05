/** 
 @file  ps4.h
 @brief ENet Unix header
*/
#ifndef __ENET_UNIX_H__
#define __ENET_UNIX_H__

#include <sys/types.h>
#include <sys/socket.h>
#include <libnetctl.h>
#include <kernel.h>

#include <stdlib.h>
#include <stdint.h>
#include <scetypes.h>
#include <sys/time.h>

#ifdef MSG_MAXIOVLEN
#define ENET_BUFFER_MAXIMUM MSG_MAXIOVLEN
#endif

typedef SceNetId ENetSocket;

#define ENET_SOCKET_NULL -1

#define ENET_HOST_TO_NET_16(value) (sceNetHtons (value)) /**< macro that converts host to net byte-order of a 16-bit value */
#define ENET_HOST_TO_NET_32(value) (sceNetHtonl (value)) /**< macro that converts host to net byte-order of a 32-bit value */

#define ENET_NET_TO_HOST_16(value) (sceNetNtohs (value)) /**< macro that converts net to host byte-order of a 16-bit value */
#define ENET_NET_TO_HOST_32(value) (sceNetNtohl (value)) /**< macro that converts net to host byte-order of a 32-bit value */

typedef struct
{
    void * data;
    size_t dataLength;
} ENetBuffer;

#define ENET_CALLBACK

#define ENET_API extern

typedef fd_set ENetSocketSet;

#define ENET_SOCKETSET_EMPTY(sockset)          FD_ZERO (& (sockset))
#define ENET_SOCKETSET_ADD(sockset, socket)    FD_SET (socket, & (sockset))
#define ENET_SOCKETSET_REMOVE(sockset, socket) FD_CLR (socket, & (sockset))
#define ENET_SOCKETSET_CHECK(sockset, socket)  FD_ISSET (socket, & (sockset))
    
#endif /* __ENET_UNIX_H__ */

