#ifndef _IP_ADDRESS_H_
#define _IP_ADDRESS_H_

#include <netinet/ip.h>

typedef uint32_t RawIpv4_t;
typedef uint8_t RawIpv6_t[16];

class IpAddress
{
public:
    IpAddress(RawIpv4_t addr);
    IpAddress();
    IpAddress(const IpAddress &);
    IpAddress(const struct in_addr &);
    IpAddress(const struct in6_addr &);
    IpAddress(const struct sockaddr_storage &);

    ~IpAddress();

    // Get Ipv4 as a uint32_t
    operator RawIpv4_t() const;

    operator struct in6_addr &() const;

    IpAddress & operator=(const IpAddress &);
    IpAddress & operator=(uint32_t);
    IpAddress & operator=(const struct in_addr &);

//    bool operator==(const IpAddress &) const;
//    bool operator!=(const IpAddress &) const;

    const char * c_str() const;

    bool is_ipv6() const;
    bool is_ipv4() const;
    bool is_any() const;

    static const IpAddress & AnyAddress();

    static const IpAddress & AnyIpv6Address();

    static const IpAddress & AnyIpv4Address();
    
    static const IpAddress & NoAddress();
protected:
    void update_ipv4_address(RawIpv4_t);

    void update_ipv4_address(const struct in_addr &);

    void update_ipv6_address(const RawIpv6_t);

    inline void update_ipv6_address(const struct in6_addr & addr) 
    {
    	update_ipv6_address(reinterpret_cast<const uint8_t *>(&addr)); 
    };
	
    void update_address(const struct sockaddr_storage & addr);

    void remove_address();

private:
    enum IpVersion {
	ANY,
	IPv4,
	IPv6
    };
 
    struct Address {
	union {
	    RawIpv4_t v4Addr;
	    RawIpv6_t v6Addr;
	};
	IpVersion ver;
	int refCnt;

	Address() : ver(ANY), refCnt(1) {};
    };
    Address * mpAddr;
};



#endif
