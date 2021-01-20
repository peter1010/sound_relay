
#include <arpa/inet.h>
#include <string.h>

#include "ip_address.h"
#include "logging.h"

/******************************************************************************/
IpAddress::IpAddress() : mpAddr(0) 
{	 
}


/******************************************************************************/
IpAddress::IpAddress(uint32_t ipv4Addr) : mpAddr(0)
{
    update_ipv4_address(ipv4Addr);
}


/******************************************************************************/
IpAddress::IpAddress(const struct in_addr & ipv4Addr) : mpAddr(0)
{
    update_ipv4_address(ipv4Addr);
}


/******************************************************************************/
IpAddress::IpAddress(const struct in6_addr & ipv6Addr) : mpAddr(0)
{
    update_ipv6_address(ipv6Addr);
}


/******************************************************************************/
IpAddress::IpAddress(const IpAddress & addr)
{
    mpAddr = addr.mpAddr;
    if(mpAddr) {
	mpAddr->refCnt++;
    }
}


/******************************************************************************/
IpAddress::IpAddress(const struct sockaddr_storage & addr) : mpAddr(0)
{
    update_address(addr);
}


/******************************************************************************/
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
const IpAddress & IpAddress::AnyIpv6Address()
{
    static IpAddress addr(in6addr_any);
    return addr;
}


/******************************************************************************/
const IpAddress & IpAddress::AnyIpv4Address()
{
    static IpAddress addr(INADDR_ANY);
    return addr;
}


/******************************************************************************/
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
bool IpAddress::is_ipv4() const
{
    return mpAddr ? (mpAddr->ver == IPv4) : false;
}


/******************************************************************************/
bool IpAddress::is_ipv6() const
{
    return mpAddr ? (mpAddr->ver == IPv6) : false;
}


/******************************************************************************/
bool IpAddress::is_any() const
{
    return mpAddr ? (mpAddr->ver == ANY) : false;
}

/******************************************************************************/
IpAddress::operator unsigned int() const
{
    if(is_ipv4()) {
        return mpAddr->v4Addr;
    }
    throw "Cannot convert IP Address to an Int";
}


/******************************************************************************/
IpAddress::operator struct in6_addr &() const
{
    if(is_ipv6()) {
        return reinterpret_cast<struct in6_addr &>(mpAddr->v6Addr);
    }
    throw "Cannot convert IP Address to an Int";
}



/******************************************************************************/
IpAddress & IpAddress::operator=(unsigned int ipv4Addr)
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

