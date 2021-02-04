
#include <arpa/inet.h>
#include <string.h>

#include "ip_address.h"
#include "logging.h"

/******************************************************************************/
/**
 * Construct a empty (null) address
 */
IpAddress::IpAddress() : mpAddr(0)
{
}


/******************************************************************************/
/**
 * Construct an IPv4 address based on 32 bit unsigned value
 */
IpAddress::IpAddress(uint32_t ipv4Addr) : mpAddr(0)
{
    update_ipv4_address(ipv4Addr);
}


/******************************************************************************/
/**
 * Construct an IPv4 address based on the contents of struct in_addr
 */
IpAddress::IpAddress(const struct in_addr & ipv4Addr) : mpAddr(0)
{
    update_ipv4_address(ipv4Addr);
}


/******************************************************************************/
/** 
 * Construct an Ipv6 address based on the contents of struct in6_addr
 */
IpAddress::IpAddress(const struct in6_addr & ipv6Addr) : mpAddr(0)
{
    update_ipv6_address(ipv6Addr);
}


/******************************************************************************/
/**
 * Copy constructor
 */
IpAddress::IpAddress(const IpAddress & addr)
{
    mpAddr = addr.mpAddr;
    if(mpAddr) {
	mpAddr->refCnt++;
    }
}


/******************************************************************************/
/**
 * Construct an IP address based on the IP address found in a struct sockaddr_storage
 */
IpAddress::IpAddress(const struct sockaddr_storage & addr) : mpAddr(0)
{
    update_address(addr);
}


/******************************************************************************/
/**
 * Construct an IP address based on the IP address in the struct sockaddr *
 */
IpAddress::IpAddress(const struct sockaddr * addr) : mpAddr(0)
{
    update_address(reinterpret_cast<const struct sockaddr_storage &>(*addr));
}



/******************************************************************************/
/**
 * Return instance of a Any Address, used for creating listening sockets
 * on both Ipv6 and Ipv4
 */
const IpAddress & IpAddress::AnyAddress()
{
    static IpAddress * addr = 0;
    if(!addr) {
	addr = new IpAddress;
	addr->mpAddr = new Address;
    }
    return *addr;
}


/******************************************************************************/
/**
 * Return instance of a Any Ipv6 Address, used for creating listening sockets
 * on Ipv6
 */
const IpAddress & IpAddress::AnyIpv6Address()
{
    static IpAddress addr(in6addr_any);
    return addr;
}


/******************************************************************************/
/**
 * Return instance of a Any Ipv4 Address, used for creating listening sockets
 * on Ipv6
 */
const IpAddress & IpAddress::AnyIpv4Address()
{
    static IpAddress addr(INADDR_ANY);
    return addr;
}


/******************************************************************************/
/**
 * Return instance of a No Address
 */
const IpAddress & IpAddress::NoAddress()
{
    static IpAddress addr;
    return addr;
}


/******************************************************************************/
IpAddress::~IpAddress()
{
    remove_address();
}


/******************************************************************************/
bool IpAddress::is_version(IpVersion_t ver) const
{
    return mpAddr ? (mpAddr->ver == ver) : false;
}


/******************************************************************************/
RawIpv4_t IpAddress::get_raw_ipv4() const
{
    if(is_ipv4()) {
        return mpAddr->v4Addr;
    }
    LOG_ERROR("Cannot convert IP Address to an Int");
    throw 1;
}


/******************************************************************************/
IpAddress::operator struct in6_addr &() const
{
    if(is_ipv6()) {
        return reinterpret_cast<struct in6_addr &>(mpAddr->v6Addr);
    }
    LOG_ERROR("Cannot convert IP Address to an Int");
    throw 2;
}



/******************************************************************************/
IpAddress & IpAddress::operator=(RawIpv4_t ipv4Addr)
{
    update_ipv4_address(ipv4Addr);
    return *this;
}


/******************************************************************************/
IpAddress & IpAddress::operator=(const IpAddress & addr)
{
    if(addr.mpAddr != mpAddr) {
	remove_address();
	mpAddr = addr.mpAddr;
	if(mpAddr) {
	    mpAddr->refCnt++;
	}
    }
    return *this;
}


/******************************************************************************/
IpAddress & IpAddress::operator=(const struct in_addr & ipv4Addr)
{
    update_ipv4_address(ipv4Addr);
    return *this;
}


/******************************************************************************/
bool IpAddress::operator==(const IpAddress & other) const
{
    if(mpAddr == other.mpAddr) {
	return true;
    }
    if(mpAddr && other.mpAddr) {
	if(mpAddr->ver == other.mpAddr->ver) {
	    if(mpAddr->ver == other.mpAddr->ver) {
		switch(mpAddr->ver) {
		    case ANY:
		    default:
		        return true;

		    case IPv4:
			return mpAddr->v4Addr == other.mpAddr->v4Addr;

		    case IPv6:
			return memcmp(mpAddr->v6Addr, other.mpAddr->v6Addr, sizeof(RawIpv6_t)) == 0;
		}
	    }
	}
    }
    return false;
}



/******************************************************************************/
const char * IpAddress::c_str() const
{
    static char buf[INET6_ADDRSTRLEN];
    if(is_ipv4()) {
        struct in_addr addr;
        addr.s_addr = htonl(mpAddr->v4Addr);
        inet_ntop(AF_INET, &addr, buf, sizeof(buf));
    } else if(is_ipv6()) {
	struct in6_addr addr;
        memcpy(&addr, mpAddr->v6Addr, sizeof(addr));
        inet_ntop(AF_INET6, &addr, buf, sizeof(buf));
    } else {
	return "Not an IP Address";
    }
    return buf;
}


/******************************************************************************/
void IpAddress::remove_address()
{
    if(mpAddr) {
	if(--mpAddr->refCnt <= 0) {
	    delete mpAddr;
	    mpAddr = 0;
	}
    }
}


/******************************************************************************/
void IpAddress::update_ipv4_address(RawIpv4_t ipv4Address)
{
    if(mpAddr) {
	if(mpAddr->refCnt == 1) {
	    mpAddr->ver = IPv4;
	    mpAddr->v4Addr = ipv4Address;
	} else {
	    remove_address();
	}
    }
    if(!mpAddr) {
	mpAddr = new Address;
	mpAddr->ver = IPv4;
	mpAddr->v4Addr = ipv4Address;
    }
}


/******************************************************************************/
void IpAddress::update_ipv4_address(const struct in_addr & addr)
{
    update_ipv4_address(ntohl(addr.s_addr));
}


/******************************************************************************/
void IpAddress::update_ipv6_address(const RawIpv6_t ipv6Address)
{
    if(mpAddr) {
	if(mpAddr->refCnt == 1) {
	    mpAddr->ver = IPv6;
	    memcpy(mpAddr->v6Addr, ipv6Address, sizeof(mpAddr->v6Addr));
	} else {
	    remove_address();
	}
    }
    if(!mpAddr) {
        mpAddr = new Address;
        mpAddr->ver = IPv6;
	memcpy(mpAddr->v6Addr, ipv6Address, sizeof(mpAddr->v6Addr));
    }
}


/******************************************************************************/
void IpAddress::update_address(const struct sockaddr_storage & addr)
{
    if(addr.ss_family == AF_INET) {
	const struct sockaddr_in * pAddr
	       	= reinterpret_cast<const struct sockaddr_in *>(&addr);
	update_ipv4_address(pAddr->sin_addr);
    } else if(addr.ss_family == AF_INET6) {
	const struct sockaddr_in6 * pAddr
		= reinterpret_cast<const struct sockaddr_in6 *>(&addr);
	update_ipv6_address(pAddr->sin6_addr);
    }
}

