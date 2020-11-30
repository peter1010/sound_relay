#ifndef _TCP_SERVER_H_
#define _TCP_SERVER_H_

#include <netinet/ip.h>

class TcpConnection;
class EventLoop;

/*----------------------------------------------------------------------------*/
class TcpServer
{
public:
    TcpServer(EventLoop & event_loop) : mEventLoop(event_loop), mSock(-1),
       		mpConn(NULL) {};
    virtual ~TcpServer();

    void close_connection(TcpConnection & rConn);

    virtual bool parse_recv(TcpConnection & rConn) = 0;
protected:
    bool init(in_port_t port);

    static void accept(void * arg);
    void accept();

 
private:
    EventLoop & mEventLoop;
    int mSock;
    TcpConnection * mpConn;
};

#endif
