#include <unistd.h>
#include <ifaddrs.h>
#include <string.h>

#include "connection.h"
#include "logging.h"
#include "network.h"
#include "event_loop.h"


/******************************************************************************/
Connection::Connection() :  mpRecvBuf(NULL)
{
    LOG_DEBUG("Connection");
}


/******************************************************************************/
void Connection::attach(int sock, Network & network,
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
			Connection::recv, this);
    }
}


/******************************************************************************/
Connection::~Connection()
{
    LOG_DEBUG("~Connection");

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
}


/******************************************************************************/
void Connection::recv(void * arg)
{
    Connection * pThis = reinterpret_cast<Connection *>(arg);
    if(!pThis->recv()) {
        delete pThis;
    }
}


