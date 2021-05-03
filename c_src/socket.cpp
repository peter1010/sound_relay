#include <sys/socket.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#include "logging.h"
#include "socket.h"
#include "ip_address.h"
#include "event_loop.h"

/*----------------------------------------------------------------------------*/
SocketException::SocketException(Socket * pSock, bool includeErrno, const char * fmt, ...) 
{
	mpSock = pSock;
	va_list ap;
	va_start(ap, fmt);

	if(includeErrno) {
		VLOG_ERRNO_AS_ERROR(fmt, ap);
	} else {
		VLOG_ERROR(fmt, ap);
	}
	va_end(ap);
}


/******************************************************************************/
SocketException::~SocketException() 
{
//	if(mpSock) {
//		delete mpSock;
//		mpSock = NULL;
//	}
}

/******************************************************************************/
/**
 * Construct a socket
 *
 * Given we support IPv6 & IPv4 there may be 2 connections per socket (IPv4
 * & IPv6)
 */
Socket::Socket(int type) : mSockV4(-1), mSockV6(-1), mType(type), mSockCreated(false)
{
	LOG_DEBUG("Socket");
}


/******************************************************************************/
/**
 * Deconstruct a socket
 */
Socket::~Socket()
{
	if(mSockV4 >= 0) {
		EventLoop::instance().unregister(mSockV4);
		::close(mSockV4);
		mSockV4 = -1;
	}
	if(mSockV6 >= 0) {
		EventLoop::instance().unregister(mSockV6);
		::close(mSockV6);
		mSockV6 = -1;
	}
	LOG_DEBUG("~Socket");
}


/******************************************************************************/
void Socket::bind(const IpAddress & addr, uint16_t port)
{
	if(addr.is_ipv4()) {
		ipv4_socket(port, addr.get_raw_ipv4(), BIND);
	} else if (addr.is_ipv6()) {
		ipv6_socket(port, addr, addr.get_scope_id(), BIND);
	} else {
		throw SocketException(this, false, "Cannot bind socket");
	}
}


/******************************************************************************/
void Socket::connect(const IpAddress & addr, uint16_t port)
{
	if(addr.is_ipv4()) {
		ipv4_socket(port, addr.get_raw_ipv4(), CONNECT);
	} else if (addr.is_ipv6()) {
		ipv6_socket(port, addr, addr.get_scope_id(), CONNECT);
	} else {
		throw SocketException(this, false, "Cannot connect socket");
	}
}


/******************************************************************************/
void Socket::listen(int backlog)
{
	if((mSockCreated) && (mType == SOCK_STREAM)) {
		if (mSockV4 > 0) {
			::listen(mSockV4, backlog);
		}
		if (mSockV6 > 0) {
			::listen(mSockV6, backlog);
		}
	} else {
		throw SocketException(this, false, "Cannot listen on this socket");
	}
}


/******************************************************************************/
void Socket::ipv4_socket(uint16_t port, uint32_t address, FollowUp_t action)
{
	if(!mSockCreated) {
		mSockV4 = socket(AF_INET, mType, 0);

		if(mSockV4 < 0) {
			SocketException(this, true, "Failed to open v4 socket");
		}
	}

	socklen_t len = 0;
	struct sockaddr_in addr;

	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = htonl(address);
	len = sizeof(addr);

	switch(action) {
		case BIND:
			bind(mSockV4, port, reinterpret_cast<const struct sockaddr *>(&addr), len);
			break;
		case CONNECT:
			connect(mSockV4, port, reinterpret_cast<const struct sockaddr *>(&addr), len);
			break;
	}
}


/******************************************************************************/
void Socket::ipv6_socket(uint16_t port,
		   const struct in6_addr & address, unsigned scope_id, FollowUp_t action)
{
	if(!mSockCreated) {
		mSockV6 = socket(AF_INET6, mType, 0);

		if(mSockV6 < 0) {
			SocketException(this, true, "Failed to open v6 socket");
		}
		setsockopt_ipv6only(mSockV6);
	}

	socklen_t len = 0;
	struct sockaddr_in6 addr;

	addr.sin6_family = AF_INET6;
	addr.sin6_port = htons(port);
	addr.sin6_addr = address;
	addr.sin6_flowinfo = 0;
	addr.sin6_scope_id = scope_id;
	len = sizeof(addr);

	switch(action) {
		case BIND:
			bind(mSockV6, port, reinterpret_cast<const struct sockaddr *>(&addr), len);
			break;
		case CONNECT:
			connect(mSockV6, port, reinterpret_cast<const struct sockaddr *>(&addr), len);
			break;
	}
}


/******************************************************************************/
void Socket::bind(int sock, uint16_t port, const sockaddr * pAddr,
		socklen_t len)
{
	int status = ::bind(sock, pAddr, len);
	if(status != 0) {
		throw SocketException(this, true, "Bind to %hu failed", port);
	}
}


/******************************************************************************/
void Socket::connect(int sock, uint16_t port, const sockaddr * pAddr,
		socklen_t len)
{
	int status = ::connect(sock, pAddr, len);
	if(status != 0) {
		throw SocketException(this, true, "Connect to %hu failed", port);
	}
}


/******************************************************************************/
void Socket::setsockopt_ipv6only(int sock)
{
	int on = 1;
	int rc = ::setsockopt(sock, IPPROTO_IPV6, IPV6_V6ONLY, (void *)&on, sizeof(on));
	if(rc != 0) {
		throw SocketException(this, true, "Failed to set IPPROTO_IPV6");
	}
}


/*----------------------------------------------------------------------------*/
int Socket::recv(uint8_t * pBuf, int len)
{
	int rc = -1;
	if(mSockV4 > 0) {
		rc = ::recv(mSockV4, pBuf, len, 0);
	} else if(mSockV6 > 0) {
		rc = ::recv(mSockV6, pBuf, len, 0);
	}
	if(rc < 0) {
		throw SocketException(this, true, "Failed to read");
	}
	return rc;
}


/*----------------------------------------------------------------------------*/
int Socket::send(const uint8_t * pData, unsigned length)
{
	int rc = -1;
	if(mSockV4 > 0) {
		rc = ::send(mSockV4, pData, length, 0);
	} else if(mSockV6 > 0) {
		rc = ::send(mSockV6, pData, length, 0);
	}
	if(rc < 0) {
		throw SocketException(this, true, "Failed to write");
	}
	return rc;
}

