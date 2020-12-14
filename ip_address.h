#ifndef _IP_ADDRESS_H_
#define _IP_ADDRESS_H_

#include <netinet/ip.h>

class IpAddress
{
public:
    IpAddress(unsigned int addr);
    IpAddress();
    IpAddress(const IpAddress &);
    IpAddress(const struct in_addr &);

    // Assumption is int = 32 bits?
    operator unsigned int() const;
    operator const char *() const;
     
    IpAddress & operator=(const IpAddress &);
    IpAddress & operator=(unsigned int);
    IpAddress & operator=(const struct in_addr &);
    const char * c_str() const;

private:
    unsigned int mAddr;
};

const IpAddress AnyAddress(INADDR_ANY);

const IpAddress NoAddress;

#endif
