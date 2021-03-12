#ifndef _CONNECTION_H_
#define _CONNECTION_H_

#include "network.h"
#include "ip_address.h"

class EventLoop;

/*----------------------------------------------------------------------------*/
class Connection
{
public:
    Connection();
    virtual ~Connection() = 0;

    // Called from the TcpServer after creating the connection object
    void attach(int, Network &, const IpAddress &, unsigned short);

protected:
    typedef unsigned char Byte;

    int get_sock() const { return mSock; };

    // Registered with the event loop
    static void recv(void * arg);
    virtual bool recv() = 0;

    Byte * get_recv_buf(unsigned & maxLen) const { maxLen = mMaxRecvLen; return mpRecvBuf; };

    virtual void send(const Byte *, unsigned) = 0;

    const IpAddress & get_peer_address() const { return mPeerAddress; };

    Network * get_network() const { return mpNetwork; };

private:
    // Socket details
    int mSock;
    Network * mpNetwork;
    IpAddress mPeerAddress;

    Byte * mpRecvBuf;
    unsigned mMaxRecvLen;

    // Hide the default methods
    Connection(const Connection &);
    const Connection & operator=(const Connection &);
};

#endif
