#ifndef _CONNECTION_H_
#define _CONNECTION_H_

#include <netinet/in.h>		// Definition of sockaddr_in
#include "network.h"

class EventLoop;

/*----------------------------------------------------------------------------*/
class Connection
{
public:
    Connection();
    virtual ~Connection() = 0;

    // Called from the TcpServer after creating the connection object
    bool attach(int, Network &, struct sockaddr_in &);

protected:
    typedef unsigned char Byte;

    int get_sock() const { return mSock; };

    // Registered with the event loop
    static void recv(void * arg);
    virtual bool recv() = 0;

    Byte * get_recv_buf(unsigned & maxLen) const { maxLen = mMaxRecvLen; return mpRecvBuf; };


    virtual void send(const Byte *, unsigned) = 0;

    const char * get_hostname() const { return mpNetwork->get_hostname(); };
    const char * get_hostip() const { return mpNetwork->get_hostip(); };
    Network * get_network() const { return mpNetwork; };

private:
    // Socket details
    int mSock;
    Network * mpNetwork;
    struct sockaddr_in  mPeerAddress;

    Byte * mpRecvBuf;
    unsigned mMaxRecvLen;

    // Hide the default methods
    Connection(const Connection &);
    const Connection & operator=(const Connection &);
};

#endif
