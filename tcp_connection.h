#ifndef _TCP_CONNECTION_H_
#define _TCP_CONNECTION_H_

#include "connection.h"

/*----------------------------------------------------------------------------*/
class TcpConnection : public Connection
{
public:
    TcpConnection();
    virtual ~TcpConnection() = 0;

protected:

    // Registered with the event loop
    bool recv();

    // -1 means issue > 0 means discard that number from the buffer
    virtual int parse_recv(const Byte *, unsigned len) = 0;

    void send(const Byte * pData, unsigned length);

private:

    // Receive buffer position
    unsigned mRecvPos;

    // Hide the default methods
    TcpConnection(const TcpConnection &);
    const TcpConnection & operator=(const TcpConnection &);
};

#endif
