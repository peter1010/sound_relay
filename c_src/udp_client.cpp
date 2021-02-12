#include <sys/socket.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#include "udp_client.h"
#include "logging.h"
#include "connection.h"


/******************************************************************************/
/**
 * Constuct a UDP client
 *
 * Given we support IPv6 & IPv4 there may be 2 connections per client (IPv4
 * & IPv6)
 */
UdpClient::UdpClient() : Network(2)
{
    LOG_DEBUG("UdpClient");
}


/******************************************************************************/
/**
 * Deconstuct a UDP client
 */
UdpClient::~UdpClient()
{
    LOG_DEBUG("~UdpClient");
}


/******************************************************************************/
/**
 * Initialise the client connection to the peer
 */
void UdpClient::init(unsigned short port, const IpAddress & address)
{
    init(port, address, 0, IpAddress::AnyAddress());
}


/******************************************************************************/
void UdpClient::init(uint16_t remotePort, const IpAddress & remoteAddress,
		uint16_t localPort, const IpAddress & localAddress)
{
    int sock = -1;
    int alt_sock = -1;

    if(remoteAddress.is_any() || remoteAddress.is_none()) {
        if(localAddress.is_ipv4()) {
    	    sock = create_ipv4_socket(localPort, localAddress.get_raw_ipv4());
        } else if (localAddress.is_ipv6()) {
     	    sock = create_ipv6_socket(localPort, localAddress, localAddress.get_scope_id());
	} else {
	    try {
	  	sock = create_ipv4_socket(localPort, IpAddress::AnyIpv4Address().get_raw_ipv4());
	    } catch (SocketException e) {
		sock = -1;
	    }
	    try {
    	        alt_sock = create_ipv6_socket(localPort, IpAddress::AnyIpv6Address(), 0);
	        if(sock < 0) {
	            sock = alt_sock;
	            alt_sock = -1;
	        }
	    } catch (SocketException e) {
		alt_sock = -1;
    	        if(sock < 0) {
	            throw NetworkException("Cannot create an all listening socket");
	        }
            }
	}
    } else if(remoteAddress.is_ipv4() && (localAddress.is_any() || localAddress.is_ipv4())) {
    	sock = create_ipv4_socket(localPort, localAddress.get_raw_ipv4());
    } else if (remoteAddress.is_ipv6() && (localAddress.is_any() || localAddress.is_ipv6())) {
    	sock = create_ipv6_socket(localPort, localAddress, localAddress.get_scope_id());
    } else {
        LOG_DEBUG("Remote %s @ %u", remoteAddress.c_str(), remotePort);
        LOG_DEBUG("Local %s @ %u", localAddress.c_str(), localPort);
	throw NetworkException("UDP Client socket can not be created no valid IP address");
    }

    socklen_t len = 0;
    struct sockaddr_storage addr;

    if(remoteAddress.is_ipv4()) {

        struct sockaddr_in * pAddr = reinterpret_cast<struct sockaddr_in *>(&addr);
        memset(pAddr, 0, sizeof(*pAddr));

        pAddr->sin_family = AF_INET;
        pAddr->sin_port = htons(remotePort);
        pAddr->sin_addr.s_addr = htonl(remoteAddress.get_raw_ipv4());
        len = sizeof(*pAddr);

    } else {

	struct sockaddr_in6 *pAddr = reinterpret_cast<struct sockaddr_in6 *>(&addr);;
        memset(pAddr, 0, sizeof(*pAddr));

        pAddr->sin6_family = AF_INET6;
        pAddr->sin6_port = htons(remotePort);
        pAddr->sin6_addr = remoteAddress;
        len = sizeof(*pAddr);
    }
 
    if(len > 0) {
        int status = connect(sock, reinterpret_cast<struct sockaddr *>(&addr), len);
        if(status != 0) {
    	    throw SocketException(sock, true, "connect to %hu failed", remotePort);
        }
    }

    Connection * pConn = create_connection();
    if(pConn) {
	pConn->attach(sock, *this, remoteAddress, remotePort);
    } else {
	::close(sock);
    }

    if(alt_sock >= 0) {
        Connection * pConn = create_connection();
        if(pConn) {
	    pConn->attach(alt_sock, *this, remoteAddress, remotePort);
        } else {
   	    ::close(sock);
        }
    }
}


/******************************************************************************/
int UdpClient::create_ipv4_socket(uint16_t localPort, uint32_t localAddress)
{
    int sock = socket(AF_INET, SOCK_DGRAM, 0);

    if(sock < 0) {
        SocketException(sock, true, "Failed to open UDPv4 socket");
    }

    socklen_t len = 0;
    struct sockaddr_in addr;

    addr.sin_family = AF_INET;
    addr.sin_port = htons(localPort);
    addr.sin_addr.s_addr = htonl(localAddress);
    len = sizeof(addr);

    bind(sock, localPort, reinterpret_cast<const struct sockaddr *>(&addr), len);
    return sock;
}


/******************************************************************************/
int UdpClient::create_ipv6_socket(uint16_t localPort,
	       const struct in6_addr & localAddress, unsigned scope_id)
{
    int sock = socket(AF_INET6, SOCK_DGRAM, 0);

    if(sock < 0) {
        SocketException(sock, true, "Failed to open UDPv6 socket");
    }

    setsockopt_ipv6only(sock);

    socklen_t len = 0;
    struct sockaddr_in6 addr;

    addr.sin6_family = AF_INET6;
    addr.sin6_port = htons(localPort);
    addr.sin6_addr = localAddress;
    addr.sin6_flowinfo = 0;
    addr.sin6_scope_id = scope_id;
    len = sizeof(addr);

    bind(sock, localPort, reinterpret_cast<const struct sockaddr *>(&addr), len);
    return sock;
}


