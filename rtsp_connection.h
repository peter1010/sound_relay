#ifndef _RTSP_CONNECTION_H_
#define _RTSP_CONNECTION_H_

#include <string>
#include "tcp_connection.h"


class RtspConnection : public ConnectionAppData
{
public:
    RtspConnection() : mState(PARSING_REQUEST_LINE) {};

    void parse_line(const std::string & str);

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

    void parse_request_line(const std::string & str);

    void parse_entity_header(const std::string & str);
};

#endif
