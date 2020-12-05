#include <sys/socket.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#include "tcp_connection.h"
#include "logging.h"
#include "tcp_server.h"


/*----------------------------------------------------------------------------*/
TcpConnection::TcpConnection(int sock, TcpServer & parent) : mSock(sock)
	, mServer(parent), mRecvPos(0)
{
   mMaxRecvLen = mServer.get_max_recv_len();
   mpRecvBuf = new unsigned char[mMaxRecvLen];
}


/*----------------------------------------------------------------------------*/
TcpConnection::~TcpConnection()
{
    LOG_DEBUG("~TcpConnection");

    if(mSock >= 0) {
	close(mSock);
	mSock = -1;
    }
    if(mpRecvBuf) {
	delete [] mpRecvBuf;
	mpRecvBuf = NULL;
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
    int status = ::recv(mSock, &mpRecvBuf[mRecvPos], mMaxRecvLen-mRecvPos,
		    0);
    if(status <= 0) {
	if(status < 0) {
	    LOG_ERRNO_AS_ERROR("Recv failed");
	} 
	LOG_ERROR("Closing connection");
	mServer.close_connection(*this);
    } else {
        mRecvPos += status;

	const int discard = parse_recv(mpRecvBuf, mRecvPos);
	if(discard < 0) {
	    LOG_ERROR("Closing connection");
	    mServer.close_connection(*this);
	} else if(discard > 0) {
	    mRecvPos -= discard;
	    memmove(mpRecvBuf, &mpRecvBuf[discard], mRecvPos);
	}
    }
}


/*----------------------------------------------------------------------------*/
void TcpConnection::send(const unsigned char * pData, unsigned length)
{
    ::send(mSock, pData, length, 0);
}

