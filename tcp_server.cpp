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
    if(mSock) {
        ::close(mSock);
	mSock = -1;
    }
}



/*----------------------------------------------------------------------------*/
bool TcpServer::init(unsigned short port, IpAddress address) 
{
    struct sockaddr_in addr;

    // Start by opening a connection to the server
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if(sock < 0) {
        LOG_ERRNO_AS_ERROR("Failed to open TCP socket");
	return false;
    }

    // setsockopt();

    memset(&addr, 0, sizeof(addr));

    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = htonl(address); // htonl(address);

    int status = bind(sock, reinterpret_cast<struct sockaddr *>(&addr), 
	    sizeof(addr));
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
    struct sockaddr_in client;
    socklen_t len = sizeof(client);

    int connfd = ::accept(pThis->mSock, reinterpret_cast<struct sockaddr *>(&client),
		&len);
    if(connfd < 0) {
	LOG_ERRNO_AS_ERROR("Accept failed");
    }
    char buf[INET_ADDRSTRLEN];
    LOG_INFO("Incomming request from %s", inet_ntop(AF_INET, &client.sin_addr,
		buf, INET_ADDRSTRLEN));

    Connection * pConn = pThis->create_connection();
    if(pConn) {
    	struct sockaddr_in server;
	if(!pConn->attach(connfd, *pThis,
			client.sin_addr, ntohs(client.sin_port))) {
            LOG_ERROR("Failed to attach connection");
	    delete pConn;
        }
    	socklen_t len = sizeof(client);
    	if(0 == getsockname(connfd, reinterpret_cast<struct sockaddr *>(&server)
		, &len)) {
	   pThis->mHostIp = server.sin_addr;
	}
    } else {
	::close(connfd);
    }	
}

