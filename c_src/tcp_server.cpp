#include <sys/socket.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#include "tcp_server.h"
#include "logging.h"
#include "event_loop.h"
#include "tcp_connection.h"


/*----------------------------------------------------------------------------*/
TcpServer::TcpServer() : Network(1), mSock(-1)
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
}


/*----------------------------------------------------------------------------*/
bool TcpServer::init(unsigned short port, IpAddress address) 
{

    int sock;
    // Start by opening a connection to the server
    if(address.is_ipv4()) {
    	sock = socket(AF_INET, SOCK_STREAM, 0);
    } else {
    	sock = socket(AF_INET6, SOCK_STREAM, 0);
    }

    if(sock < 0) {
        LOG_ERRNO_AS_ERROR("Failed to open TCP socket");
	return false;
    }

    int on = 1;
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char *)&on, sizeof(on));

    struct sockaddr_storage addr;
    socklen_t addrLen = 0;

    if(address.is_ipv4()) {
    	struct sockaddr_in * pAddr = reinterpret_cast<struct sockaddr_in *>(&addr);
	memset(pAddr, 0, sizeof(*pAddr));

	pAddr->sin_family = AF_INET;
 	pAddr->sin_port = htons(port);
	pAddr->sin_addr.s_addr = htonl(address);
	addrLen = sizeof(*pAddr);

    } else if(address.is_ipv6()) {
	struct sockaddr_in6 * pAddr = reinterpret_cast<struct sockaddr_in6 *>(&addr);
	memset(pAddr, 0, sizeof(*pAddr));

	pAddr->sin6_family = AF_INET6;
	pAddr->sin6_port = htons(port);
	pAddr->sin6_addr = address;
	addrLen = sizeof(*pAddr);
    }

    int status = bind(sock, reinterpret_cast<struct sockaddr *>(&addr), addrLen);
    if(status != 0) {
    	LOG_ERRNO_AS_ERROR("Bind to %hu failed", port);
	close(sock);
	return false;
    }

    status = listen(sock, 5);
    if(status != 0) {
    	LOG_ERRNO_AS_ERROR("listen failed");
	close(sock);
	return false;
   }
   EventLoop::instance().register_read_callback(sock, TcpServer::accept, this);
   mSock = sock;
   mListeningPort = port;
   return true;
}


/*----------------------------------------------------------------------------*/
void TcpServer::accept(void * arg)
{
    TcpServer * pThis = reinterpret_cast<TcpServer *>(arg);
    struct sockaddr_storage client;
    struct sockaddr * pClient = reinterpret_cast<struct sockaddr *>(&client);
    socklen_t len = sizeof(client);

    int connfd = ::accept(pThis->mSock, pClient, &len);
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

    Connection * pConn = pThis->create_connection();
    if(pConn) {
    	struct sockaddr_storage server;

	if(!pConn->attach(connfd, *pThis, clientAddr, port)) {
            LOG_ERROR("Failed to attach connection");
	    delete pConn;
        }
    	socklen_t len = sizeof(server);
    	if(0 == getsockname(connfd, reinterpret_cast<struct sockaddr *>(&server)
		, &len)) {
	   pThis->mHostIp = IpAddress(server);
	}
    } else {
	::close(connfd);
    }	
}

