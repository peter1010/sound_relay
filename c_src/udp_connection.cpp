#include <sys/socket.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#include "udp_connection.h"
#include "logging.h"
#include "event_loop.h"
#include "socket.h"

/******************************************************************************/
/**
 * Constuct a UDP client
 *
 * Given we support IPv6 & IPv4 there may be 2 connections per client (IPv4
 * & IPv6)
 */
UdpConnection::UdpConnection() : mSock(SOCK_DGRAM), mpRecvBuf(NULL)
{
	LOG_DEBUG("UdpConnection");
}


/******************************************************************************/
/**
 * Deconstuct a UDP client
 */
UdpConnection::~UdpConnection()
{
	if(mpRecvBuf) {
		delete [] mpRecvBuf;
		mpRecvBuf = NULL;
	}

	LOG_DEBUG("~UdpConnection");
}


/******************************************************************************/
/**
 * Initialise the client connection to the peer
 */
void UdpConnection::init(unsigned short port, const IpAddress & address)
{
	init(port, address, 0, IpAddress::AnyAddress());
}


/******************************************************************************/
void UdpConnection::init(uint16_t remotePort, const IpAddress & remoteAddress,
		uint16_t localPort, const IpAddress & localAddress)
{

	if(remoteAddress.is_any() || remoteAddress.is_none()) {
		mSock.bind(localAddress, localPort);
	} else if(remoteAddress.is_ipv4() && (localAddress.is_any() || localAddress.is_ipv4())) {
		mSock.bind(localAddress, localPort);
	} else if (remoteAddress.is_ipv6() && (localAddress.is_any() || localAddress.is_ipv6())) {
		mSock.bind(localAddress, localPort);
	} else {
		LOG_DEBUG("Remote %s @ %u", remoteAddress.c_str(), remotePort);
		LOG_DEBUG("Local %s @ %u", localAddress.c_str(), localPort);
		throw "TODO"; //NetworkException("UDP Client socket can not be created no valid IP address");
	}

	mSock.connect(remoteAddress, remotePort);
    mMaxRecvLen  = get_max_recv_len();
	if(mMaxRecvLen > 0) {
		mpRecvBuf = new uint8_t[mMaxRecvLen];
		EventLoop::instance().register_read_callback(mSock.fileNo1(), recv, this);

	}
}


bool UdpConnection::recv()
{
	bool retVal = true;
	unsigned maxLen = 0; 
	uint8_t * pBuf = get_recv_buf(maxLen);

	const int status = mSock.recv(pBuf, maxLen);

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



/******************************************************************************/
void UdpConnection::recv(void * arg)
{
	UdpConnection * pThis = reinterpret_cast<UdpConnection *>(arg);
	if(!pThis->recv()) {
		delete pThis;
	}
}

