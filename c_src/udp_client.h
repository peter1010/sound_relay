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

    const IpAddress & get_hostip() const { return NoAddress; };

protected:
    bool init(unsigned short port, const IpAddress & addr, 
		    unsigned short localPort = 0,
		    const IpAddress & localAddr = AnyAddress);

private:
    
    UdpClient(const UdpClient &);
    UdpClient & operator=(const UdpClient &);
};

#endif
