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

    enum T_RECV_STATE {
        BAD_RECV,
	NEED_MORE,
	ALL_DONE
    };

    virtual T_RECV_STATE parse_recv(const Byte *, unsigned len) = 0;

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
