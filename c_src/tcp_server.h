#ifndef _TCP_SERVER_H_
#define _TCP_SERVER_H_

#include "network.h"
#include "ip_address.h"

class EventLoop;


/*----------------------------------------------------------------------------*/
class TcpServer : public Network
{
public:
    TcpServer();
    virtual ~TcpServer() = 0;

    const IpAddress & get_hostip() const { return mHostIp; };
    unsigned short get_listening_port() const { return mListeningPort; };

protected:
    bool init(unsigned short port, IpAddress addr = AnyAddress);

    static void accept(void * arg);
 
private:
    int mSock;
    IpAddress mHostIp;
    unsigned short mListeningPort;
 
    TcpServer(const TcpServer &);
    TcpServer & operator=(const TcpServer &);
};

#endif
