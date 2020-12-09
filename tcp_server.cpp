#include <sys/socket.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#include "tcp_server.h"
#include "logging.h"
#include "event_loop.h"
#include "tcp_connection.h"


/*----------------------------------------------------------------------------*/
TcpServer::TcpServer() : mSock(-1)
{
    LOG_DEBUG("TcpServer");
    strcpy(mHostIp,"Unknown");
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
bool TcpServer::init(in_port_t port, in_addr_t address) 
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
    addr.sin_addr.s_addr = htonl(address);

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
    reinterpret_cast<TcpServer *>(arg)->accept();
}


/*----------------------------------------------------------------------------*/
void TcpServer::accept()
{
    struct sockaddr_in client;
    socklen_t len = sizeof(client);

    int connfd = ::accept(mSock, reinterpret_cast<struct sockaddr *>(&client),
		&len);
    if(connfd < 0) {
	LOG_ERRNO_AS_ERROR("Accept failed");
    }
    char buf[INET_ADDRSTRLEN];
    LOG_INFO("Incomming request from %s", inet_ntop(AF_INET, &client.sin_addr,
		buf, INET_ADDRSTRLEN));

    if(create_connection()) {
    	struct sockaddr_in server;
	if(!get_connection()->attach(connfd, *this, client)) {
            LOG_ERROR("Failed to attach connection");
	    delete_connection();
        }
    	socklen_t len = sizeof(client);
    	if(0 == getsockname(connfd, reinterpret_cast<struct sockaddr *>(&server)
		, &len)) {
	   inet_ntop(AF_INET, &server.sin_addr, mHostIp, sizeof(mHostIp));
	}
    } else {
	::close(connfd);
    }	
}

