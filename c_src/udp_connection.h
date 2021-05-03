#ifndef _UDP_CONNECTION_H_
#define _UDP_CONNECTION_H_

#include "ip_address.h"
#include "socket.h"

class EventLoop;

/*----------------------------------------------------------------------------*/
class UdpConnection
{
public:
    UdpConnection();
    virtual ~UdpConnection() = 0;

    virtual unsigned get_max_recv_len() const = 0;

protected:
    void init(uint16_t, const IpAddress &, uint16_t, const IpAddress &);
    
    void init(uint16_t, const IpAddress &);

    void send(const uint8_t * pData, unsigned length) {mSock.send(pData, length);};

    virtual bool parse_recv(const uint8_t *, unsigned len) = 0;

	bool recv();

    // Registered with the event loop
    static void recv(void * arg);

    uint8_t * get_recv_buf(unsigned & maxLen) const { maxLen = mMaxRecvLen; return mpRecvBuf; };

    Socket mSock;
private:

    uint8_t * mpRecvBuf;
    unsigned mMaxRecvLen;
   
    UdpConnection(const UdpConnection &);
    UdpConnection & operator=(const UdpConnection &);
};


#endif
