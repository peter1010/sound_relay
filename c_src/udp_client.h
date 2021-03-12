#ifndef _UDP_CLIENT_H_
#define _UDP_CLIENT_H_

//#include "network.h"
#include "ip_address.h"

class EventLoop;
class UdpConnection;

typedef UdpConnection * (* ConnectionFactory)(void * pArg);

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


/*----------------------------------------------------------------------------*/
class UdpClient
{
public:
    UdpClient();
    virtual ~UdpClient() = 0;

    virtual unsigned get_max_recv_len() const = 0;

	// Called from destructor on the Connection
    void detach_connection(int, UdpConnection *) noexcept;

    // Should be called from sub-class to register a connection object creation
    // function.
    void register_connection_factory(ConnectionFactory, void *) noexcept;


protected:
    void init(uint16_t, const IpAddress &, uint16_t, const IpAddress &);
    
    void init(uint16_t, const IpAddress &);

    static int create_ipv4_socket(uint16_t, uint32_t);

    static int create_ipv6_socket(uint16_t, const struct in6_addr &, unsigned);

	void delete_connections() noexcept;
    UdpConnection * create_connection();

    static void bind(int sock, uint16_t port, const sockaddr * pAddr, socklen_t len);
    static void setsockopt_ipv6only(int);
    static void setsockopt_reuseaddr(int);


private:
    
    UdpConnection * mpConn;

    ConnectionFactory mpConnectionFactory;
    void * mpFactoryArg;

    UdpClient(const UdpClient &);
    UdpClient & operator=(const UdpClient &);
};




#endif
