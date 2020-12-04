#include <sys/socket.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#include "tcp_server.h"
#include "logging.h"
#include "event_loop.h"
#include "tcp_connection.h"

#if 0

int rtpSock = socket(AF_INET, SOCK_DGRAM, 0);

int rtcpSock = socket(AF_INET, SOCK_DGRAM. 0);
#endif


/*----------------------------------------------------------------------------*/
TcpServer::TcpServer(EventLoop & rEventLoop) 
	: mEventLoop(rEventLoop), mSock(-1)
	, mpConn(NULL), mpConnectionFactory(NULL) 
{
    LOG_DEBUG("TcpServer");
}


/*----------------------------------------------------------------------------*/
TcpServer::~TcpServer()
{
    LOG_DEBUG("~TcpServer");
    if(mpConn) {
        close_connection(*mpConn);
	mpConn = NULL;
    }
    if(mSock) {
        close(mSock);
	mSock = -1;
    }
}



/*----------------------------------------------------------------------------*/
bool TcpServer::init(uint16_t port) 
{
    struct sockaddr_in addr;

    // Start by opening a connection to the server
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if(sock < 0) {
        LOG_ERRNO_AS_ERROR("Failed to open TCP socket");
    }

    // setsockopt();

    memset(&addr, 0, sizeof(addr));

    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

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
   mEventLoop.register_read_callback(sock, TcpServer::accept, this);
   mSock = sock;
   return true;
}


/*----------------------------------------------------------------------------*/
void TcpServer::register_connection_factory(ConnectionFactory pFunc)
{
    mpConnectionFactory = pFunc;;
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

    if(mpConnectionFactory) {
	if(mpConn) {
	    close(connfd);
        } else {
	    mpConn = mpConnectionFactory(connfd, *this);
   	    mEventLoop.register_read_callback(connfd, TcpConnection::recv, mpConn);
	}
    } else {
        LOG_ERROR("No registered Connection factory");
    }	
}


/*----------------------------------------------------------------------------*/
void TcpServer::close_connection(TcpConnection & rConn)
{
    LOG_INFO("close_connection");
    mEventLoop.unregister(rConn.getSock());
    delete &rConn;
    mpConn = NULL;
}

