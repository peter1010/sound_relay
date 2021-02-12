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

    unsigned short get_listening_port() const { return mListeningPort; };

protected:
    void init(unsigned short, const IpAddress &);

    static void accept(void *);
    static void alt_accept(void *);

    void accept(int);

    static int create_ipv4_socket(uint16_t, uint32_t);

    static int create_ipv6_socket(uint16_t, const struct in6_addr &, unsigned);

    static void bind_and_listen(int, uint16_t, const sockaddr *, socklen_t);


private:
    int mSock;
    int mAltSock;
    unsigned short mListeningPort;

    TcpServer(const TcpServer &);
    TcpServer & operator=(const TcpServer &);
};

#endif
