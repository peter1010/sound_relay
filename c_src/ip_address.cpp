
#include <arpa/inet.h>
#include <string.h>

#include "ip_address.h"
#include "logging.h"

/******************************************************************************/
IpAddress::IpAddress() : mpAddr(0) 
{	 
}


/******************************************************************************/
IpAddress::IpAddress(unsigned int ipv4Address) : mpAddr(0)
{
    update_ipv4_address(ipv4Address);
}


/******************************************************************************/
IpAddress::IpAddress(const struct in_addr & ipv4Address) : mpAddr(0)
{
    update_ipv4_address(ntohl(ipv4Address.s_addr));
}


/******************************************************************************/
IpAddress::IpAddress(const struct in6_addr & ipv6Address) : mpAddr(0)
{
    update_ipv6_address(ipv6Address);
}


/******************************************************************************/
IpAddress::IpAddress(const IpAddress & address) : mpAddr(0)
{
    update_address(address);
}


/******************************************************************************/
IpAddress::IpAddress(const struct sockaddr & addr) : mpAddr(0)
{
    if(addr.sa_family == AF_INET) {
	const struct sockaddr_in * pAddr
	       	= reinterpret_cast<const struct sockaddr_in *>(&addr);
	update_ipv4_address(ntohl(pAddr->sin_addr.s_addr));
    } else if(addr.sa_family == AF_INET6) {
	const struct sockaddr_in6 * pAddr 
		= reinterpret_cast<const struct sockaddr_in6 *>(&addr);
	update_ipv6_address(pAddr->sin6_addr);
    }
}


/******************************************************************************/
IpAddress::IpAddress(const struct sockaddr_storage & addr) : mpAddr(0)
{
    if(addr.ss_family == AF_INET) {
	const struct sockaddr_in * pAddr
	       	= reinterpret_cast<const struct sockaddr_in *>(&addr);
	update_ipv4_address(ntohl(pAddr->sin_addr.s_addr));
    } else if(addr.ss_family == AF_INET6) {
	const struct sockaddr_in6 * pAddr 
		= reinterpret_cast<const struct sockaddr_in6 *>(&addr);
	update_ipv6_address(pAddr->sin6_addr);
    }
}


/******************************************************************************/
IpAddress::~IpAddress()
{
    remove_address();
}


/******************************************************************************/
bool IpAddress::is_ipv4() const
{
    return mpAddr ? mpAddr->ipv4 : false;
}


/******************************************************************************/
bool IpAddress::is_ipv6() const
{
    return mpAddr ? !mpAddr->ipv4 : false;
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
IpAddress & IpAddress::operator=(unsigned int ipv4Address)
{
    update_ipv4_address(ipv4Address);
    return *this;
}


/******************************************************************************/
IpAddress & IpAddress::operator=(const IpAddress & address)
{
    update_address(address);
    return *this;
}


/******************************************************************************/
IpAddress & IpAddress::operator=(const struct in_addr & ipv4Address)
{
    *this = ntohl(ipv4Address.s_addr);
    return *this;
}


/******************************************************************************/
const char * IpAddress::c_str() const
{
    static char buf[INET_ADDRSTRLEN];
    if(is_ipv4()) {
        struct in_addr addr;
        addr.s_addr = htonl(mpAddr->v4Addr);
        inet_ntop(AF_INET, &addr, buf, sizeof(buf));
    } else if(is_ipv6()) {
	struct in6_addr addr;
	uint8_t * p = reinterpret_cast<uint8_t *>(&addr);
        memcpy(&addr, mpAddr->v6Addr, sizeof(addr));
    	LOG_INFO("%02x %02x %02x %02x %02x %02x", p[0], p[1],
		    p[2], p[3], p[4], 
		    p[5]);
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
void IpAddress::update_ipv4_address(uint32_t ipv4Address)
{
    if(mpAddr) {
	if(mpAddr->refCnt == 1) {
	    mpAddr->ipv4 = true;
	    mpAddr->v4Addr = ipv4Address;
	} else {
	    remove_address();
	}
    }
    if(!mpAddr) {
	mpAddr = new Address;
	mpAddr->ipv4 = true;
	mpAddr->v4Addr = ipv4Address;
	mpAddr->refCnt = 1;
    }
}


/******************************************************************************/
void IpAddress::update_ipv6_address(const uint8_t * ipv6Address)
{
    if(mpAddr) {
	if(mpAddr->refCnt == 1) {
	    mpAddr->ipv4 = false;
	    memcpy(mpAddr->v6Addr, ipv6Address, sizeof(mpAddr->v6Addr));
	} else {
	    remove_address();
	}	
    }
    if(!mpAddr) {
        mpAddr = new Address;
        mpAddr->ipv4 = false;
	memcpy(mpAddr->v6Addr, ipv6Address, sizeof(mpAddr->v6Addr));
	mpAddr->refCnt = 1;
    }
}



/******************************************************************************/
void IpAddress::update_address(const IpAddress & address)
{
    if(address.is_ipv4()) {
	update_ipv4_address(address.mpAddr->v4Addr);
    } else if(address.is_ipv6()) {
	update_ipv6_address(address.mpAddr->v6Addr);
    } else {
	remove_address();
    }
}
