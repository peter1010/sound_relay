#include <sys/socket.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>

#include "udp_connection.h"
#include "logging.h"
#include "event_loop.h"

/*----------------------------------------------------------------------------*/
UdpConnection::UdpConnection() : mpRecvBuf(NULL)
{
    LOG_DEBUG("UdpConnection");
}


/*----------------------------------------------------------------------------*/
UdpConnection::~UdpConnection()
{
    if(mpNetwork) {
        mpNetwork->detach_connection(mSock, this);
	mpNetwork = NULL;
    }

    if(mSock >= 0) {
	::close(mSock);
	mSock = -1;
    }
    if(mpRecvBuf) {
	delete [] mpRecvBuf;
	mpRecvBuf = NULL;
    }
    LOG_DEBUG("~UdpConnection");
}


/*----------------------------------------------------------------------------*/
bool UdpConnection::recv()
{
    bool retVal = true;
    unsigned maxLen = 0; 
    Byte * pBuf = get_recv_buf(maxLen);

    const int status = ::recv(get_sock(), pBuf, maxLen, 0);

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


/******************************************************************************/
void UdpConnection::attach(int sock, UdpClient & network,
		const IpAddress & peerAddress, unsigned short /*peerPort*/)
{
    mpNetwork = &network;
    mSock = sock;
    mPeerAddress = peerAddress;
//  mPeerPort = port;

    mMaxRecvLen = network.get_max_recv_len();
    if(mMaxRecvLen > 0) {
	mpRecvBuf = new unsigned char[mMaxRecvLen];

        EventLoop::instance().register_read_callback(sock,
			UdpConnection::recv, this);
    }
}


/******************************************************************************/
void UdpConnection::recv(void * arg)
{
    UdpConnection * pThis = reinterpret_cast<UdpConnection *>(arg);
    if(!pThis->recv()) {
        delete pThis;
    }
}


