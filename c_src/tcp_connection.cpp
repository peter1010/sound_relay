#include <sys/socket.h>
#include <string.h>
#include <arpa/inet.h>

#include "tcp_connection.h"
#include "logging.h"
#include "event_loop.h"

/*----------------------------------------------------------------------------*/
TcpConnection::TcpConnection() :  mRecvPos(0)
{
}


/*----------------------------------------------------------------------------*/
TcpConnection::~TcpConnection()
{
    LOG_DEBUG("~TcpConnection");
}


/*----------------------------------------------------------------------------*/
bool TcpConnection::recv()
{
    bool retVal = true;
    unsigned maxLen = 0; 
    Byte * pBuf = get_recv_buf(maxLen);

    const int status = ::recv(get_sock(), &pBuf[mRecvPos], maxLen-mRecvPos,
		    0);
    if(status <= 0) {
	if(status < 0) {
	    LOG_ERRNO_AS_ERROR("Recv failed");
	} 
	LOG_ERROR("Closing connection");
	retVal = false;
    } else {
        mRecvPos += status;

	const int discard = parse_recv(pBuf, mRecvPos);
	if(discard < 0) {
	    LOG_ERROR("Closing connection");
	    retVal = false;
	} else if(discard > 0) {
	    mRecvPos -= discard;
	    memmove(pBuf, &pBuf[discard], mRecvPos);
	}
    }
    return retVal;
}


/*----------------------------------------------------------------------------*/
void TcpConnection::send(const unsigned char * pData, unsigned length)
{
    ::send(get_sock(), pData, length, 0);
}

