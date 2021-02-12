#ifndef _UDP_CLIENT_H_
#define _UDP_CLIENT_H_

#include "network.h"
#include "ip_address.h"

class EventLoop;
class Connection;

/*----------------------------------------------------------------------------*/
class UdpClient : public Network
{
public:
    UdpClient();
    virtual ~UdpClient() = 0;

    const IpAddress & get_hostip() const { return IpAddress::NoAddress(); };

protected:
    void init(uint16_t, const IpAddress &, uint16_t, const IpAddress &);
    
    void init(uint16_t, const IpAddress &);

    static int create_ipv4_socket(uint16_t, uint32_t);

    static int create_ipv6_socket(uint16_t, const struct in6_addr &, unsigned);

private:
    
    UdpClient(const UdpClient &);
    UdpClient & operator=(const UdpClient &);
};

#endif
