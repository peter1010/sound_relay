#ifndef _NETWORK_H_
#define _NETWORK_H_

#include <stdint.h>
#include <sys/socket.h>

class IpAddress;
class Connection;

typedef Connection * (* ConnectionFactory)(void * pArg);

/******************************************************************************/
class SocketException 
{
public:
    SocketException(int, bool, const char * fmt, ...) __attribute__((format (printf, 4, 5)));
    ~SocketException();
private:
    int mSock;
};


/******************************************************************************/
class NetworkException 
{
public:
    NetworkException(const char * msg);
    ~NetworkException();
private:
};



/******************************************************************************/
class Network
{
public:
    Network(unsigned);
    virtual ~Network() = 0;

    virtual unsigned get_max_recv_len() const = 0;

    // Called from destructor on the Connection
    void detach_connection(int, Connection *);

    // Should be called from sub-class to register a connection object creation
    // function.
    void register_connection_factory(ConnectionFactory, void *);

protected:

    void delete_connections();
    Connection * create_connection();

    static void bind(int sock, uint16_t port, const sockaddr * pAddr, socklen_t len);
    static void setsockopt_ipv6only(int);
    static void setsockopt_reuseaddr(int);

private:

    unsigned mMaxNumOfConns;
    Connection ** mpConns;

    ConnectionFactory mpConnectionFactory;
    void * mpFactoryArg;

    Network(const Network &);
    Network & operator=(const Network &);
};

#endif
