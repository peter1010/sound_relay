#ifndef _TCP_CONNECTION_H_
#define _TCP_CONNECTION_H_

class TcpServer;

/*----------------------------------------------------------------------------*/
class TcpConnection
{
public:
    TcpConnection(int sock, TcpServer & parent) : mSock(sock), mServer(parent),
       		mRecvPos(0) {};
    ~TcpConnection();

    const char * getRecvBuf() const { return mRecvBuf; };
    unsigned getRecvBufLen() const { return mRecvPos; };
    int getSock() const { return mSock; };

    static void recv(void * arg);
protected:

    void recv();

private:
    int mSock;
    TcpServer & mServer;

    static const unsigned MAX_RECV_BUFFER = 1024;
    char mRecvBuf[MAX_RECV_BUFFER];
    unsigned mRecvPos;
};

#endif
