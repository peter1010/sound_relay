#ifndef _UDP_CONNECTION_H_
#define _UDP_CONNECTION_H_

#include "udp_client.h"
#include "ip_address.h"

class EventLoop;

/*----------------------------------------------------------------------------*/
class UdpConnection
{
public:
    UdpConnection();
    virtual ~UdpConnection() = 0;

    void send(const uint8_t * pData, unsigned length);

    void attach(int, UdpClient &);

protected:

    bool recv();

    virtual bool parse_recv(const uint8_t *, unsigned len) = 0;

    typedef unsigned char Byte;

    int get_sock() const { return mSock; };

    // Registered with the event loop
    static void recv(void * arg);

    Byte * get_recv_buf(unsigned & maxLen) const { maxLen = mMaxRecvLen; return mpRecvBuf; };

//    virtual void send(const uint8_t *, unsigned) = 0;

    UdpClient * get_network() const { return mpNetwork; };


private:

    int mSock;
    UdpClient * mpNetwork;

    uint8_t * mpRecvBuf;
    unsigned mMaxRecvLen;

    UdpConnection(const UdpConnection &);
    const UdpConnection & operator=(const UdpConnection &);
};


#endif
