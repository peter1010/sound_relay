#include <sys/socket.h>
#include <string.h>
#include <arpa/inet.h>

#include "udp_connection.h"
#include "logging.h"
#include "event_loop.h"

/*----------------------------------------------------------------------------*/
UdpConnection::UdpConnection()
{
}


/*----------------------------------------------------------------------------*/
UdpConnection::~UdpConnection()
{
    LOG_DEBUG("~UdpConnection");
}


/*----------------------------------------------------------------------------*/
bool UdpConnection::recv()
{
    bool retVal = true;
    unsigned maxLen = 0; 
    Byte * pBuf = get_recv_buf(maxLen);

    const int status = ::recv(get_sock(), &pBuf, maxLen, 0);

    if(status <= 0) {
	if(status < 0) {
	    LOG_ERRNO_AS_ERROR("Recv failed");
	} 
	LOG_ERROR("Closing connection");
	retVal = false;
    } else {

	const int ok = parse_recv(pBuf, status);
	if(!ok) {
	    LOG_ERROR("Closing connection");
	    retVal = false;
	}
    }
    return retVal;
}


/*----------------------------------------------------------------------------*/
void UdpConnection::send(const unsigned char * pData, unsigned length)
{
    ::send(get_sock(), pData, length, 0);
}

