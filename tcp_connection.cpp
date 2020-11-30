#include <sys/socket.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#include "tcp_connection.h"
#include "logging.h"
#include "tcp_server.h"


/*----------------------------------------------------------------------------*/
TcpConnection::~TcpConnection()
{
    if(mSock >= 0) {
	close(mSock);
	mSock = -1;
    }
}


/*----------------------------------------------------------------------------*/
void TcpConnection::recv(void * arg)
{
    reinterpret_cast<TcpConnection *>(arg)->recv();
}


/*----------------------------------------------------------------------------*/
void TcpConnection::recv()
{
    int status = ::recv(mSock, &mRecvBuf[mRecvPos], MAX_RECV_BUFFER-mRecvPos,
		    0);
    if(status <= 0) {
	if(status < 0) {
	    LOG_ERRNO_AS_ERROR("Recv failed");
	} 
	LOG_ERROR("Closing connection");
	mServer.close_connection(*this);
    } else {
        mRecvPos += status;
        mRecvBuf[mRecvPos] = '\0';
	if( !mServer.parse_recv(*this)) {
	    LOG_ERROR("Closing connection");
	    mServer.close_connection(*this);
	}
    }
}
