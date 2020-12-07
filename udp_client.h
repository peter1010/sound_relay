#ifndef _UDP_CLIENT_H_
#define _UDP_CLIENT_H_

#include <netinet/ip.h>

#include "network.h"

class EventLoop;
class Connection;

/*----------------------------------------------------------------------------*/
class UdpClient : public Network
{
public:
    UdpClient();
    virtual ~UdpClient() = 0;


protected:
    bool init(in_port_t port, in_addr_t addr, in_port_t localPort = 0,
		    in_addr_t localAddr = INADDR_ANY);

private:
    
    UdpClient(const UdpClient &);
    UdpClient & operator=(const UdpClient &);
};

#endif
