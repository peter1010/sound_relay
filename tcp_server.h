#ifndef _TCP_SERVER_H_
#define _TCP_SERVER_H_

#include <netinet/ip.h>
#include "network.h"

class EventLoop;


/*----------------------------------------------------------------------------*/
class TcpServer : public Network
{
public:
    TcpServer();
    virtual ~TcpServer() = 0;

    const char * get_hostip() const { return mHostIp; };
    in_port_t get_listening_port() const { return mListeningPort; };

protected:
    bool init(in_port_t port, in_addr_t addr = INADDR_ANY);

    static void accept(void * arg);
    void accept();
 
private:
    int mSock;
    char mHostIp[INET_ADDRSTRLEN];
    in_port_t mListeningPort;
 
    TcpServer(const TcpServer &);
    TcpServer & operator=(const TcpServer &);
};

#endif
