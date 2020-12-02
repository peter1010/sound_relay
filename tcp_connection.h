#ifndef _TCP_CONNECTION_H_
#define _TCP_CONNECTION_H_

class TcpServer;


/*----------------------------------------------------------------------------*/
class ConnectionAppData
{
public:
    virtual ~ConnectionAppData() {};
protected:
};


/*----------------------------------------------------------------------------*/
class TcpConnection
{
public:
    TcpConnection(int sock, TcpServer & parent);
    ~TcpConnection();

    const unsigned char * getRecvBuf() const { return mpRecvBuf; };
    unsigned getRecvBufLen() const { return mRecvPos; };
    int getSock() const { return mSock; };

    static void recv(void * arg);

    void attachAppData(ConnectionAppData * pData);

    ConnectionAppData * getAppData() const {return mpAppData;};
protected:

    void recv();

private:
    int mSock;
    TcpServer & mServer;
    ConnectionAppData * mpAppData;

    unsigned char *  mpRecvBuf;
    unsigned mMaxRecvLen;
    unsigned mRecvPos;
};

#endif
