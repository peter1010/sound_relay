#include <sys/socket.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#include "tcp_server.h"
#include "logging.h"
#include "event_loop.h"
#include "tcp_connection.h"


/*----------------------------------------------------------------------------*/
TcpServer::TcpServer() : Network(1), mSock(-1), mAltSock(-1)
{
    LOG_DEBUG("TcpServer");
}


/*----------------------------------------------------------------------------*/
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


/*----------------------------------------------------------------------------*/
bool TcpServer::init(unsigned short port, const IpAddress & addr) 
{
    int sock = -1;
    int alt_sock = -1;

    if(addr.is_ipv4()) {
    	sock = create_ipv4_socket(port, addr);
    } else if (addr.is_ipv6()) {
    	sock = create_ipv6_socket(port, addr);
    } else if (addr.is_any()) {
    	sock = create_ipv4_socket(port, IpAddress::AnyIpv4Address());
    	alt_sock = create_ipv6_socket(port, IpAddress::AnyIpv6Address());
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
    mListeningPort = port;
    return true;
}


/*----------------------------------------------------------------------------*/
int TcpServer::create_ipv4_socket(uint16_t port, uint32_t address)
{
    int sock = socket(AF_INET, SOCK_STREAM, 0);

    if(sock < 0) {
        LOG_ERRNO_AS_ERROR("Failed to open TCPv6 socket");
	return -1;
    }

    int on = 1;
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (void *)&on, sizeof(on));

    socklen_t len = 0;
    struct sockaddr_in addr;

    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = htonl(address);
    len = sizeof(addr);

    return bind_and_listen(sock, port, reinterpret_cast<const struct sockaddr *>(&addr), len);
}


/*----------------------------------------------------------------------------*/
int TcpServer::create_ipv6_socket(uint16_t port, const struct in6_addr & address)
{
    int sock = socket(AF_INET6, SOCK_STREAM, 0);

    if(sock < 0) {
        LOG_ERRNO_AS_ERROR("Failed to open TCPv6 socket");
	return -1;
    }

    int on = 1;
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (void *)&on, sizeof(on));

    on = 1;
    setsockopt(sock, IPPROTO_IPV6, IPV6_V6ONLY, (void *)&on, sizeof(on));

    socklen_t len = 0;
    struct sockaddr_in6 addr;

    addr.sin6_family = AF_INET6;
    addr.sin6_port = htons(port);
    addr.sin6_addr = address;
    len = sizeof(addr);

    return bind_and_listen(sock, port, reinterpret_cast<const struct sockaddr *>(&addr), len);
}


/*----------------------------------------------------------------------------*/
int TcpServer::bind_and_listen(int sock, uint16_t port, const sockaddr * pAddr,
		socklen_t len) 
{	
    int status = bind(sock, pAddr, len);
    if(status != 0) {
    	LOG_ERRNO_AS_ERROR("Bind to %hu failed", port);
	close(sock);
	return -1;
    }

    status = listen(sock, 5);
    if(status != 0) {
    	LOG_ERRNO_AS_ERROR("listen failed");
	close(sock);
	return -1;
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
    	struct sockaddr_storage server;

	if(!pConn->attach(connfd, *this, clientAddr, port)) {
            LOG_ERROR("Failed to attach connection");
	    delete pConn;
        }
    	socklen_t len = sizeof(server);
    	if(0 == getsockname(connfd, reinterpret_cast<struct sockaddr *>(&server)
		, &len)) {
	   mHostIp = IpAddress(server);
	}
    } else {
	::close(connfd);
    }	
}

