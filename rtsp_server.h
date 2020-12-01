#ifndef _RTSP_SERVER_H_
#define _RTSP_SERVER_H_

#include <string>
#include "tcp_server.h"
#include "tcp_connection.h"


class RtspConnection : public ConnectionAppData
{
public:
    RtspConnection() : mState(PARSING_REQUEST_LINE) {};

    void parse_line(std::string & str);

private:
    enum T_STATE
    {
	PARSING_REQUEST_LINE,
        PARSING_ENTITY_HEADERS,	    
    };
    T_STATE mState;

    enum T_REQUEST_TYPE
    {	
	DESCRIBE,
	//ANNOUNCE,
	//GET_PARAMETER,
	OPTIONS,
	PAUSE,
	PLAY,
	//RECORD,
	//REDIRECT,
	SETUP,
	//SET_PARAMETER,
	TEARDOWN
    };
    T_REQUEST_TYPE mRequestType;

    std::string mUrl;

    void parse_request_line(std::string & str);
};


class RtspServer : public TcpServer
{
public:
    RtspServer(EventLoop & event_loop);

private:

    bool parse_recv(TcpConnection & rConn);

    unsigned getMaxRecvBufLen() const {return 1024;};

    void parse_request(TcpConnection & rConn);

    void parse_line(TcpConnection & rConn, const char * pBuf, unsigned len);
};

#endif
