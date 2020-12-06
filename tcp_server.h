#ifndef _TCP_SERVER_H_
#define _TCP_SERVER_H_

#include <netinet/ip.h>
#include "network.h"

class EventLoop;


/*----------------------------------------------------------------------------*/
class TcpServer : public Network
{
public:
    TcpServer(EventLoop & rEventLoop);
    virtual ~TcpServer() = 0;


protected:
    bool init(in_port_t port, in_addr_t addr = INADDR_ANY);

    static void accept(void * arg);
    void accept();
 
private:
    int mSock;
    
    TcpServer(const TcpServer &);
    TcpServer & operator=(const TcpServer &);
};

#endif
