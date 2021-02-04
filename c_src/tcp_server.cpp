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
bool TcpServer::init(unsigned short localPort, const IpAddress & localAddr)
{
    int sock = -1;
    int alt_sock = -1;

    if(localAddr.is_ipv4()) {
    	sock = create_ipv4_socket(localPort, localAddr.get_raw_ipv4());
    } else if (localAddr.is_ipv6()) {
    	sock = create_ipv6_socket(localPort, localAddr);
    } else if (localAddr.is_any()) {
    	sock = create_ipv4_socket(localPort, IpAddress::AnyIpv4Address().get_raw_ipv4());
    	alt_sock = create_ipv6_socket(localPort, IpAddress::AnyIpv6Address());
	if(sock < 0) {
	    sock = alt_sock;
	    alt_sock = -1;
	}
    } else {
	LOG_ERROR("No IP address");
	return false;
    }

    if(sock < 0) {
	return false;
    }


    EventLoop::instance().register_read_callback(sock, TcpServer::accept, this);
    if(alt_sock >= 0) {
        EventLoop::instance().register_read_callback(alt_sock, TcpServer::alt_accept, this);
    }
    mSock = sock;
    mAltSock = alt_sock;
    mListeningPort = localPort;
    return true;
}


/*----------------------------------------------------------------------------*/
int TcpServer::create_ipv4_socket(uint16_t localPort, uint32_t localAddress)
{
    int sock = socket(AF_INET, SOCK_STREAM, 0);

    if(sock < 0) {
        LOG_ERRNO_AS_ERROR("Failed to open TCPv4 socket");
	return -1;
    }

    setsockopt_reuseaddr(sock);

    socklen_t len = 0;
    struct sockaddr_in addr;

    addr.sin_family = AF_INET;
    addr.sin_port = htons(localPort);
    addr.sin_addr.s_addr = htonl(localAddress);
    len = sizeof(addr);

    return bind_and_listen(sock, localPort, reinterpret_cast<const struct sockaddr *>(&addr), len);
}


/*----------------------------------------------------------------------------*/
int TcpServer::create_ipv6_socket(uint16_t localPort, const struct in6_addr & localAddress)
{
    int sock = socket(AF_INET6, SOCK_STREAM, 0);

    if(sock < 0) {
        LOG_ERRNO_AS_ERROR("Failed to open TCPv6 socket");
	return -1;
    }

    setsockopt_reuseaddr(sock);
    setsockopt_ipv6only(sock);

    socklen_t len = 0;
    struct sockaddr_in6 addr;

    addr.sin6_family = AF_INET6;
    addr.sin6_port = htons(localPort);
    addr.sin6_addr = localAddress;
    len = sizeof(addr);

    return bind_and_listen(sock, localPort, reinterpret_cast<const struct sockaddr *>(&addr), len);
}


/*----------------------------------------------------------------------------*/
int TcpServer::bind_and_listen(int sock, uint16_t localPort, const sockaddr * pAddr,
		socklen_t len)
{
    sock = bind(sock, localPort, pAddr, len);

    if(sock >= 0) {
        int status = listen(sock, 5);
        if(status != 0) {
    	    LOG_ERRNO_AS_ERROR("listen failed");
	    close(sock);
	    return -1;
        }
   }
   return sock;
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

	if(!pConn->attach(connfd, *this, clientAddr, port)) {
            LOG_ERROR("Failed to attach connection");
	    delete pConn;
        }
    } else {
	::close(connfd);
    }
}

