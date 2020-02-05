/**
 @file  unix.c
 @brief ENet Unix system specific functions
*/
#ifdef __ORBIS__

#include <sys/time.h>
#include <netinet/tcp.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <time.h>

#define ENET_BUILDING_LIB 1
#include "enet/enet.h"

#define SOCKET_OPEN_NAME	__FUNCTION__

static enet_uint32 timeBase = 0;

int
enet_initialize(void)
{
	return sceNetCtlInit();
}

void
enet_deinitialize(void)
{
	sceNetCtlTerm();
}


int doResolverNtoa(const char *hostname, SceNetInAddr *addr)
{
	SceNetId rid = -1;
	int memid = -1;
	int ret;

	ret = sceNetPoolCreate(__FUNCTION__, 4 * 1024, 0);
	if (ret < 0) {
		printf("sceNetPoolCreate() failed (0x%x errno=%d)\n",
			ret, sce_net_errno);
		goto failed;
	}
	memid = ret;
	ret = sceNetResolverCreate("resolver", memid, 0);
	if (ret < 0) {
		printf("sceNetResolverCreate() failed (0x%x errno=%d)\n",
			ret, sce_net_errno);
		goto failed;
	}
	rid = ret;
	ret = sceNetResolverStartNtoa(rid, hostname, addr, 0, 0, 0);
	if (ret < 0) {
		printf("sceNetResolverStartNtoa() failed (0x%x errno=%d)\n",
			ret, sce_net_errno);
		goto failed;
	}
	ret = sceNetResolverDestroy(rid);
	if (ret < 0) {
		printf("sceNetResolverDestroy() failed (0x%x errno=%d)\n",
			ret, sce_net_errno);
		goto failed;
	}
	ret = sceNetPoolDestroy(memid);
	if (ret < 0) {
		printf("sceNetPoolDestroy() failed (0x%x errno=%d)\n",
			ret, sce_net_errno);
		goto failed;
	}
	return 0;

failed:
	sceNetResolverDestroy(rid);
	sceNetPoolDestroy(memid);
	return ret;
}

SceKernelTimeval netGetTimeWide(void)
{
	SceKernelTimeval tv;

	sceKernelGettimeofday(&tv);
	return tv;
}

enet_uint32
enet_host_random_seed(void)
{
	return (enet_uint32)time(NULL);
}

enet_uint32
enet_time_get(void)
{
	SceKernelTimeval timeVal = netGetTimeWide();

	return timeVal.tv_sec * 1000 + timeVal.tv_usec / 1000 - timeBase;
}

void
enet_time_set(enet_uint32 newTimeBase)
{
	SceKernelTimeval timeVal = netGetTimeWide();

	timeBase = timeVal.tv_sec * 1000 + timeVal.tv_usec / 1000 - newTimeBase;
}

int
enet_address_set_host_ip(ENetAddress * address, const char * name)
{
	if (sceNetInetPton(SCE_NET_AF_INET, name, &address->host) != 0)
		return -1;

	return 0;
}

int
enet_address_set_host(ENetAddress * address, const char * name)
{
	return enet_address_set_host_ip(address, name);
}

char *netStrcpy(char *dest, const char *src, size_t n)
{
	char *p;

	p = strncpy(dest, src, n);
	dest[n - 1] = '\0';
	return p;
}

int
enet_address_get_host_ip(const ENetAddress * address, char * name, size_t nameLength)
{
	char tmp[SCE_NET_INET_ADDRSTRLEN];
	if (sceNetInetNtop(SCE_NET_AF_INET, &address->host, tmp, sizeof(tmp)) == NULL) {
		return -1;
	}

	size_t addrLen = strlen(tmp);
	if (addrLen >= nameLength)
		return -1;

	netStrcpy(name, tmp, addrLen);

	return 0;
}

int
enet_address_get_host(const ENetAddress * address, char * name, size_t nameLength)
{
	return enet_address_get_host_ip(address, name, nameLength);
}

int
enet_socket_bind(ENetSocket socket, const ENetAddress * address)
{
	SceNetSockaddrIn sin;

	memset(&sin, 0, sizeof(SceNetSockaddrIn));

	sin.sin_len = sizeof(sin);
	sin.sin_family = SCE_NET_AF_INET;

	if (address != NULL)
	{
		sin.sin_port = ENET_HOST_TO_NET_16(address->port);
		sin.sin_addr.s_addr = address->host;
	}
	else
	{
		sin.sin_port = 0;
		sin.sin_addr.s_addr = (u_int32_t)0x00000000;
	}

	int ret = sceNetBind(socket, (SceNetSockaddr *)&sin, sizeof(sin));
	if (ret < 0) {
		return -1;
	}

	return 0;
}

int
enet_socket_get_address(ENetSocket socket, ENetAddress * address)
{
	SceNetSockaddrIn sin;
	SceNetSocklen_t sinLength = sizeof(SceNetSockaddrIn);

	if (sceNetGetsockname(socket, (SceNetSockaddr*) & sin, &sinLength) == -1)
		return -1;

	address->host = (enet_uint32)sin.sin_addr.s_addr;
	address->port = ENET_NET_TO_HOST_16(sin.sin_port);

	return 0;
}

int
enet_socket_listen(ENetSocket socket, int backlog)
{
	return sceNetListen(socket, backlog);
}

ENetSocket
enet_socket_create(ENetSocketType type)
{
	return sceNetSocket(SOCKET_OPEN_NAME, SCE_NET_AF_INET, type == ENET_SOCKET_TYPE_DATAGRAM ? SCE_NET_SOCK_DGRAM : SCE_NET_SOCK_STREAM, 0);
}

int
enet_socket_set_option(ENetSocket socket, ENetSocketOption option, int value)
{
	int result = -1;
	switch (option)
	{
	case ENET_SOCKOPT_NONBLOCK:
		result = sceKernelFcntl(socket, SCE_KERNEL_F_SETFL, (value ? SCE_KERNEL_O_NONBLOCK : 0) | (fcntl(socket, SCE_KERNEL_F_GETFL) & ~SCE_KERNEL_O_NONBLOCK));
		break;

	case ENET_SOCKOPT_BROADCAST:
		result = sceNetSetsockopt(socket, SCE_NET_SOL_SOCKET, SCE_NET_SO_BROADCAST, (char *)& value, sizeof(int));
		break;

	case ENET_SOCKOPT_REUSEADDR:
		result = sceNetSetsockopt(socket, SCE_NET_SOL_SOCKET, SCE_NET_SO_REUSEADDR, (char *)& value, sizeof(int));
		break;

	case ENET_SOCKOPT_RCVBUF:
		result = sceNetSetsockopt(socket, SCE_NET_SOL_SOCKET, SCE_NET_SO_RCVBUF, (char *)& value, sizeof(int));
		break;

	case ENET_SOCKOPT_SNDBUF:
		result = sceNetSetsockopt(socket, SCE_NET_SOL_SOCKET, SCE_NET_SO_SNDBUF, (char *)& value, sizeof(int));
		break;

	case ENET_SOCKOPT_RCVTIMEO:
	{
		struct timeval timeVal;
		timeVal.tv_sec = value / 1000;
		timeVal.tv_usec = (value % 1000) * 1000;
		result = sceNetSetsockopt(socket, SCE_NET_SOL_SOCKET, SCE_NET_SO_RCVTIMEO, (char *)& timeVal, sizeof(struct timeval));
		break;
	}

	case ENET_SOCKOPT_SNDTIMEO:
	{
		struct timeval timeVal;
		timeVal.tv_sec = value / 1000;
		timeVal.tv_usec = (value % 1000) * 1000;
		result = sceNetSetsockopt(socket, SCE_NET_SOL_SOCKET, SO_SNDTIMEO, (char *)& timeVal, sizeof(struct timeval));
		break;
	}

	case ENET_SOCKOPT_NODELAY:
		result = sceNetSetsockopt(socket, SCE_NET_IPPROTO_TCP, SCE_NET_TCP_NODELAY, (char *)& value, sizeof(int));
		break;

	default:
		break;
}
	return result == -1 ? -1 : 0;
}

int
enet_socket_get_option(ENetSocket socket, ENetSocketOption option, int * value)
{
	int result = -1;
	socklen_t len;
	switch (option)
	{
	case ENET_SOCKOPT_ERROR:
		len = sizeof(int);
		result = sceNetGetsockopt(socket, SCE_NET_SOL_SOCKET, SCE_NET_SO_ERROR, value, &len);
		break;

	default:
		break;
	}
	return result == -1 ? -1 : 0;
}

int
enet_socket_connect(ENetSocket socket, const ENetAddress * address)
{
	SceNetSockaddrIn sin;
	int result;

	memset(&sin, 0, sizeof(SceNetSockaddrIn));

	sin.sin_family = SCE_NET_AF_INET;
	sin.sin_port = ENET_HOST_TO_NET_16(address->port);
	sin.sin_addr.s_addr = address->host;

	result = sceNetConnect(socket, (SceNetSockaddr*) & sin, sizeof(SceNetSockaddrIn));
	if (result == -1 && errno == SCE_NET_EINPROGRESS)
		return 0;

	return result;
}

ENetSocket
enet_socket_accept(ENetSocket socket, ENetAddress * address)
{
	int result;
	SceNetSockaddrIn sin;
	SceNetSocklen_t sinLength = sizeof(SceNetSockaddrIn);

	result = sceNetAccept(socket,
		address != NULL ? (SceNetSockaddr*) & sin : NULL,
		address != NULL ? &sinLength : NULL);

	if (result == -1)
		return ENET_SOCKET_NULL;

	if (address != NULL)
	{
		address->host = (enet_uint32)sin.sin_addr.s_addr;
		address->port = ENET_NET_TO_HOST_16(sin.sin_port);
	}

	return result;
}

int
enet_socket_shutdown(ENetSocket socket, ENetSocketShutdown how)
{
	return sceNetShutdown(socket, (int)how);
}

void
enet_socket_destroy(ENetSocket socket)
{
	if (socket != -1)
		sceNetSocketClose(socket);
}

int
enet_socket_send(ENetSocket socket,
	const ENetAddress * address,
	const ENetBuffer * buffers,
	size_t bufferCount)
{
	SceNetMsghdr msgHdr;
	SceNetSockaddrIn sin;
	int sentLength;

	memset(&msgHdr, 0, sizeof(SceNetMsghdr));

	if (address != NULL)
	{
		memset(&sin, 0, sizeof(SceNetSockaddrIn));

		sin.sin_family = AF_INET;
		sin.sin_port = ENET_HOST_TO_NET_16(address->port);
		sin.sin_addr.s_addr = address->host;

		msgHdr.msg_name = &sin;
		msgHdr.msg_namelen = sizeof(SceNetSockaddrIn);
	}

	msgHdr.msg_iov = (SceNetIovec*) buffers;
	msgHdr.msg_iovlen = bufferCount;

	sentLength = sceNetSendmsg(socket, &msgHdr, 0);

	if (sentLength == -1)
	{
		if (errno == EWOULDBLOCK)
			return 0;

		return -1;
	}

	return sentLength;
}

int
enet_socket_receive(ENetSocket socket,
	ENetAddress * address,
	ENetBuffer * buffers,
	size_t bufferCount)
{
	SceNetMsghdr msgHdr;
	SceNetSockaddrIn sin;
	int recvLength;

	memset(&msgHdr, 0, sizeof(SceNetMsghdr));

	if (address != NULL)
	{
		msgHdr.msg_name = &sin;
		msgHdr.msg_namelen = sizeof(SceNetSockaddrIn);
	}

	msgHdr.msg_iov = (SceNetIovec*) buffers;
	msgHdr.msg_iovlen = bufferCount;

	recvLength = sceNetRecvmsg(socket, &msgHdr, SCE_NET_MSG_DONTWAIT);

	if (recvLength == -1)
	{
		if (errno == SCE_NET_EWOULDBLOCK)
			return 0;

		return -1;
	}

#ifdef HAS_MSGHDR_FLAGS
	if (msgHdr.msg_flags & MSG_TRUNC)
		return -1;
#endif

	if (address != NULL)
	{
		address->host = (enet_uint32)sin.sin_addr.s_addr;
		address->port = ENET_NET_TO_HOST_16(sin.sin_port);
	}

	return recvLength;
	}

int
enet_socketset_select(ENetSocket maxSocket, ENetSocketSet * readSet, ENetSocketSet * writeSet, enet_uint32 timeout)
{
	struct timeval timeVal;

	timeVal.tv_sec = timeout / 1000;
	timeVal.tv_usec = (timeout % 1000) * 1000;

	return select(maxSocket + 1, readSet, writeSet, NULL, &timeVal);
}

int
enet_socket_wait(ENetSocket socket, enet_uint32 * condition, enet_uint32 timeout)
{
#ifdef HAS_POLL
	struct pollfd pollSocket;
	int pollCount;

	pollSocket.fd = socket;
	pollSocket.events = 0;

	if (*condition & ENET_SOCKET_WAIT_SEND)
		pollSocket.events |= POLLOUT;

	if (*condition & ENET_SOCKET_WAIT_RECEIVE)
		pollSocket.events |= POLLIN;

	pollCount = poll(&pollSocket, 1, timeout);

	if (pollCount < 0)
	{
		if (errno == EINTR && * condition & ENET_SOCKET_WAIT_INTERRUPT)
		{
			*condition = ENET_SOCKET_WAIT_INTERRUPT;

			return 0;
		}

		return -1;
	}

	*condition = ENET_SOCKET_WAIT_NONE;

	if (pollCount == 0)
		return 0;

	if (pollSocket.revents & POLLOUT)
		* condition |= ENET_SOCKET_WAIT_SEND;

	if (pollSocket.revents & POLLIN)
		* condition |= ENET_SOCKET_WAIT_RECEIVE;

	return 0;
#else
	fd_set readSet, writeSet;
	struct timeval timeVal;
	int selectCount;

	timeVal.tv_sec = timeout / 1000;
	timeVal.tv_usec = (timeout % 1000) * 1000;

	FD_ZERO(&readSet);
	FD_ZERO(&writeSet);

	if (*condition & ENET_SOCKET_WAIT_SEND)
		FD_SET(socket, &writeSet);

	if (*condition & ENET_SOCKET_WAIT_RECEIVE)
		FD_SET(socket, &readSet);

	selectCount = select(socket + 1, &readSet, &writeSet, NULL, &timeVal);

	if (selectCount < 0)
	{
		if (errno == EINTR && * condition & ENET_SOCKET_WAIT_INTERRUPT)
		{
			*condition = ENET_SOCKET_WAIT_INTERRUPT;

			return 0;
		}

		return -1;
	}

	*condition = ENET_SOCKET_WAIT_NONE;

	if (selectCount == 0)
		return 0;

	if (FD_ISSET(socket, &writeSet))
		* condition |= ENET_SOCKET_WAIT_SEND;

	if (FD_ISSET(socket, &readSet))
		* condition |= ENET_SOCKET_WAIT_RECEIVE;

	return 0;
#endif
}

#endif

