#ifndef _TCP_SERVER_H_
#define _TCP_SERVER_H_

#include <netinet/ip.h>

class EventLoop;
class TcpConnection;
class TcpServer;

typedef TcpConnection * (* ConnectionFactory)(int, TcpServer &);

/*----------------------------------------------------------------------------*/
class TcpServer
{
public:
    TcpServer(EventLoop & rEventLoop);

    virtual ~TcpServer() = 0;

    virtual unsigned get_max_recv_len() const = 0;

    void close_connection(TcpConnection & rConn);

    // Should be called from sub-class to register a connection object creation
    // function.
    void register_connection_factory(ConnectionFactory pFunc);

protected:
    bool init(in_port_t port);

    static void accept(void * arg);
    void accept();
 
private:
    EventLoop & mEventLoop;
    int mSock;
    TcpConnection * mpConn;
    ConnectionFactory mpConnectionFactory;

    
    TcpServer(const TcpServer &);
    TcpServer & operator=(const TcpServer &);
};

#endif
