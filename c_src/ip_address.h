#ifndef _IP_ADDRESS_H_
#define _IP_ADDRESS_H_

#include <netinet/ip.h>
#include "ip_version.h"

typedef uint32_t RawIpv4_t;
typedef uint8_t RawIpv6_t[16];

class IpAddressException
{
public:
     IpAddressException(const char *, bool useErrno = false);
     ~IpAddressException();
private:
};


class IpAddress
{
public:
    IpAddress(RawIpv4_t addr);
    IpAddress();
    IpAddress(const IpAddress &);
    IpAddress(const struct in_addr &);
    IpAddress(const struct in6_addr &, const char * intf = 0);
    IpAddress(const struct sockaddr_storage &);
    IpAddress(const struct sockaddr *);

    ~IpAddress();

    // Get Ipv4 as a uint32_t
    RawIpv4_t get_raw_ipv4() const;

    operator struct in6_addr &() const;

    IpAddress & operator=(const IpAddress &);
    IpAddress & operator=(RawIpv4_t);
    IpAddress & operator=(const struct in_addr &);

    bool operator==(const IpAddress &) const;
    bool operator!=(const IpAddress & other) const { return !(*this==other);};

    const char * c_str() const;

    bool is_version(IpVersion_t ver) const;
    bool is_ipv6() const { return is_version(IPv6); };
    bool is_ipv4() const { return is_version(IPv4); };
    bool is_any() const { return is_version(ANY); };
    bool is_none() const { return mpAddr == 0; };

    static const IpAddress & AnyAddress();

    static const IpAddress & AnyIpv6Address();

    static const IpAddress & AnyIpv4Address();

    static const IpAddress & NoAddress();

    unsigned get_scope_id() const;

protected:
    void update_ipv4_address(RawIpv4_t);

    void update_ipv4_address(const struct in_addr &);

    void update_ipv6_address(const RawIpv6_t, unsigned scope_id);

    inline void update_ipv6_address(const struct in6_addr & addr, unsigned scope_id)
    {
    	update_ipv6_address(reinterpret_cast<const uint8_t *>(&addr), scope_id);
    };

    void update_address(const struct sockaddr_storage &);

    void remove_address();

private:

    struct Address {
	union {
	    RawIpv4_t v4Addr;
	    RawIpv6_t v6Addr;
	};
	IpVersion_t ver;
	unsigned scopeId;
	int refCnt;

	Address() : ver(ANY), refCnt(1) {};
    };
    Address * mpAddr;
};



#endif
