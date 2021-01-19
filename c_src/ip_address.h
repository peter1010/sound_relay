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
    IpAddress(const struct in6_addr &);
    IpAddress(const struct sockaddr &);
    IpAddress(const struct sockaddr_storage &);

    ~IpAddress();

    // Assumption is int = 32 bits?
    operator unsigned int() const;
    operator const char *() const;
    operator struct in6_addr &() const;

    IpAddress & operator=(const IpAddress &);
    IpAddress & operator=(unsigned int);
    IpAddress & operator=(const struct in_addr &);

//    bool operator==(const IpAddress &) const;
//    bool operator!=(const IpAddress &) const;

    const char * c_str() const;

    bool is_ipv6() const;
    bool is_ipv4() const;

private:
    struct Address {
	union {
	    uint32_t v4Addr;
	    uint8_t v6Addr[16];
	};
	bool ipv4;
	int refCnt;
    };
    Address * mpAddr;

    void update_ipv4_address(uint32_t);
    void update_ipv6_address(const uint8_t *);

    void update_ipv6_address(const struct in6_addr & addr) {
    	update_ipv6_address(reinterpret_cast<const uint8_t *>(&addr)); };
	
    void update_address(const IpAddress &);
    void remove_address();
};


const IpAddress AnyAddress(in6addr_any);

const IpAddress NoAddress;

#endif
