#ifndef _UDP_CONNECTION_H_
#define _UDP_CONNECTION_H_

#include "connection.h"


/*----------------------------------------------------------------------------*/
class UdpConnection : public Connection
{
public:
    UdpConnection();
    virtual ~UdpConnection() = 0;

    void send(const Byte * pData, unsigned length);

protected:

    bool recv();

    virtual bool parse_recv(const Byte *, unsigned len) = 0;

private:

    UdpConnection(const UdpConnection &);
    const UdpConnection & operator=(const UdpConnection &);
};

#endif
