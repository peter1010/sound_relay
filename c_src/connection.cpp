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
    bool retVal = true;

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
    LOG_INFO("Host IPv4 is %s", get_hostip(IPv4).c_str());
    LOG_INFO("Host IPv6 is %s", get_hostip(IPv6).c_str());
}


/******************************************************************************/
IpAddress Connection::get_hostip(IpVersion_t ver) const
{
    struct sockaddr_storage server;
    socklen_t len = sizeof(server);

    if(0 == getsockname(mSock, reinterpret_cast<struct sockaddr *>(&server)
		, &len)) {

        IpAddress srvAddr = IpAddress(server);

	if ((ver == IPv4) && srvAddr.is_ipv4()) {
	    return srvAddr;
	}
	if ((ver == IPv6) && srvAddr.is_ipv6()) {
	    return srvAddr;
	}

	struct ifaddrs * ifaddr;
	if(getifaddrs(&ifaddr) == 0) {
	    char * intf = NULL;
	    for(struct ifaddrs * ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
		if(ifa->ifa_addr == NULL) {
		    continue;
		}
		if(ifa->ifa_addr != NULL) {
		    try {
		        IpAddress tmpAddr = IpAddress(ifa->ifa_addr);
		        if (tmpAddr == srvAddr) {
			    intf = ifa->ifa_name;
			    LOG_INFO("Interface name is %s", intf);
			}
		    } catch (IpAddressException e) {}
		}
	    }
	    for(struct ifaddrs * ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
		if(ifa->ifa_addr == NULL) {
		    continue;
		}
		if(ifa->ifa_addr != NULL) {
		    if (strcmp(ifa->ifa_name, intf) == 0) {
		        try {
			    IpAddress tmpAddr = IpAddress(ifa->ifa_addr);
   			    if ((ver == IPv4) && tmpAddr.is_ipv4()) {
	                        srvAddr = tmpAddr;
			        break;
			    }
	                    if ((ver == IPv6) && tmpAddr.is_ipv6()) {
	    		        srvAddr = tmpAddr;
			        break;
	                    }   
		        } catch (IpAddressException e) {}
		    }
		}
	    }
	    freeifaddrs(ifaddr);
	    return srvAddr;
	}
    }
    return IpAddress::NoAddress();
}


/******************************************************************************/
const char * Connection::get_hostname() const
{
    static char buf[64];
    static bool got_name = false;

    if(!got_name) {
	if(0 == gethostname(buf, sizeof(buf))) {
	    const unsigned len = strlen(buf);
	    buf[len] = '.';
	    if(0 != getdomainname(&buf[len+1], sizeof(buf) - len - 1)) {
		buf[len] = '\0';
	    }
	    got_name = true;
	} else {
	    LOG_ERRNO_AS_ERROR("Failed to get domain name");
	    buf[0] = '\0';
        }
    }
    return buf;
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


