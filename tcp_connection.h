#ifndef _TCP_CONNECTION_H_
#define _TCP_CONNECTION_H_

class TcpServer;


/*----------------------------------------------------------------------------*/
class TcpConnection
{
public:
    TcpConnection(int sock, TcpServer & parent);
    virtual ~TcpConnection() = 0;

    typedef unsigned char Byte;

    int getSock() const { return mSock; };

    static void recv(void * arg);

    void send(const Byte * pData, unsigned length);

protected:

    void recv();

    // -1 means issue > 0 means discard that number from the buffer
    virtual int parse_recv(const Byte *, unsigned len) = 0;

private:
    int mSock;
    TcpServer & mServer;

    unsigned char *  mpRecvBuf;
    unsigned mMaxRecvLen;
    unsigned mRecvPos;

    TcpConnection(const TcpConnection &);
    const TcpConnection & operator=(const TcpConnection &);
};

#endif
