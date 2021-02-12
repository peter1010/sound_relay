#include <sys/socket.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#include "tcp_server.h"
#include "logging.h"
#include "event_loop.h"
#include "tcp_connection.h"


/******************************************************************************/
/**
 * Construct a TCP server
 */
TcpServer::TcpServer() : Network(1), mSock(-1), mAltSock(-1)
{
    LOG_DEBUG("TcpServer");
}


/******************************************************************************/
/**
 * Deconstruct a TCP server
 */
TcpServer::~TcpServer()
{
    LOG_DEBUG("~TcpServer");
    if(mSock) {
   	EventLoop::instance().unregister(mSock);
        ::close(mSock);
	mSock = -1;
    }
    if(mAltSock) {
   	EventLoop::instance().unregister(mAltSock);
        ::close(mAltSock);
	mAltSock = -1;
    }
}


/******************************************************************************/
/**
 * Create a listening socket
 */
void TcpServer::init(unsigned short localPort, const IpAddress & localAddr)
{
    int sock = -1;
    int alt_sock = -1;

    if(localAddr.is_ipv4()) {
    	sock = create_ipv4_socket(localPort, localAddr.get_raw_ipv4());
    } else if (localAddr.is_ipv6()) {
    	sock = create_ipv6_socket(localPort, localAddr, localAddr.get_scope_id());
    } else if (localAddr.is_any()) {
	try {
    	    sock = create_ipv4_socket(localPort, IpAddress::AnyIpv4Address().get_raw_ipv4());
	} catch (SocketException e) {
	    sock = -1;
	}
	try {
    	    alt_sock = create_ipv6_socket(localPort, IpAddress::AnyIpv6Address(), 0);
	    if(sock < 0) {
	        sock = alt_sock;
	        alt_sock = -1;
	    }
	} catch (SocketException e) {
	    alt_sock = -1;
    	    if(sock < 0) {
	        throw NetworkException("Cannot create an all listening socket");
	    }
	}
    } else {
        LOG_DEBUG("Local %s @ %u", localAddr.c_str(), localPort);
	throw NetworkException("TCP socket cannot be created, no valid IP address");
    }

    EventLoop::instance().register_read_callback(sock, TcpServer::accept, this);
    if(alt_sock >= 0) {
        EventLoop::instance().register_read_callback(alt_sock, TcpServer::alt_accept, this);
    }
    mSock = sock;
    mAltSock = alt_sock;
    mListeningPort = localPort;
}


/******************************************************************************/
int TcpServer::create_ipv4_socket(uint16_t localPort, uint32_t localAddress)
{
    int sock = socket(AF_INET, SOCK_STREAM, 0);

    if(sock < 0) {
	SocketException(sock, true, "Failed to open TCPv4 socket");
    }

    setsockopt_reuseaddr(sock);

    socklen_t len = 0;
    struct sockaddr_in addr;

    addr.sin_family = AF_INET;
    addr.sin_port = htons(localPort);
    addr.sin_addr.s_addr = htonl(localAddress);
    len = sizeof(addr);

    bind_and_listen(sock, localPort, reinterpret_cast<const struct sockaddr *>(&addr), len);
    return sock;
}


/******************************************************************************/
int TcpServer::create_ipv6_socket(uint16_t localPort, const struct in6_addr & localAddress, unsigned scope_id)
{
    int sock = socket(AF_INET6, SOCK_STREAM, 0);

    if(sock < 0) {
	SocketException(sock, true, "Failed to open TCPv6 socket");
    }

    setsockopt_reuseaddr(sock);
    setsockopt_ipv6only(sock);

    socklen_t len = 0;
    struct sockaddr_in6 addr;

    addr.sin6_family = AF_INET6;
    addr.sin6_port = htons(localPort);
    addr.sin6_addr = localAddress;
    addr.sin6_flowinfo = 0;
    addr.sin6_scope_id = scope_id;
    len = sizeof(addr);

    bind_and_listen(sock, localPort, reinterpret_cast<const struct sockaddr *>(&addr), len);
    return sock;
}


/*----------------------------------------------------------------------------*/
void TcpServer::bind_and_listen(int sock, uint16_t localPort, const sockaddr * pAddr,
		socklen_t len)
{
    bind(sock, localPort, pAddr, len);

    int status = listen(sock, 5);
    if(status != 0) {
	throw SocketException(sock, true, "listen failed");
    }
}


/*----------------------------------------------------------------------------*/
void TcpServer::accept(void * arg)
{
    TcpServer * pThis = reinterpret_cast<TcpServer *>(arg);
    pThis->accept(pThis->mSock);
}



/*----------------------------------------------------------------------------*/
void TcpServer::alt_accept(void * arg)
{
    TcpServer * pThis = reinterpret_cast<TcpServer *>(arg);
    pThis->accept(pThis->mAltSock);
}


/*----------------------------------------------------------------------------*/
void TcpServer::accept(int sock)
{
    struct sockaddr_storage client;
    struct sockaddr * pClient = reinterpret_cast<struct sockaddr *>(&client);
    socklen_t len = sizeof(client);

    int connfd = ::accept(sock, pClient, &len);
    if(connfd < 0) {
	LOG_ERRNO_AS_ERROR("Accept failed");
    }

    IpAddress clientAddr(client);
    LOG_INFO("Incomming request from %s", clientAddr.c_str());

    uint16_t port;
    if(pClient->sa_family == AF_INET) {
	port = ntohs(reinterpret_cast<struct sockaddr_in *>(pClient)->sin_port);
    } else {
	port = ntohs(reinterpret_cast<struct sockaddr_in6 *>(pClient)->sin6_port);
    }

    Connection * pConn = create_connection();
    if(pConn) {
	pConn->attach(connfd, *this, clientAddr, port);
    } else {
	::close(connfd);
    }
}

