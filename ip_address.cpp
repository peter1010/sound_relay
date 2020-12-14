
#include <arpa/inet.h>

#include "ip_address.h"


IpAddress::IpAddress() 
{
    mAddr = 0xFFFFFFFFU;
}


IpAddress::IpAddress(unsigned int ipv4Address)
{
    mAddr = ipv4Address;
}


IpAddress::IpAddress(const struct in_addr & ipv4Address)
{
    mAddr = ntohl(ipv4Address.s_addr);
}


IpAddress::operator unsigned int() const
{
    return mAddr;
}


IpAddress::IpAddress(const IpAddress & address)
{
    mAddr = address.mAddr;
}


IpAddress & IpAddress::operator=(unsigned int ipv4Address)
{
    mAddr = ipv4Address;
    return *this;
}


IpAddress & IpAddress::operator=(const IpAddress & ipv4Address)
{
    mAddr = ipv4Address.mAddr;
    return *this;
}


IpAddress & IpAddress::operator=(const struct in_addr & ipv4Address)
{
    mAddr = ntohl(ipv4Address.s_addr);
    return *this;
}

const char * IpAddress::c_str() const
{
    static char buf[INET_ADDRSTRLEN];
    struct in_addr addr;
    addr.s_addr = htonl(mAddr);
    inet_ntop(AF_INET, &addr, buf, sizeof(buf));
    return buf;
}
