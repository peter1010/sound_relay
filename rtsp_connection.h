#ifndef _RTSP_CONNECTION_H_
#define _RTSP_CONNECTION_H_

#include <string>
#include "tcp_connection.h"


class RtspConnection : public TcpConnection
{
public:
    RtspConnection(int sock, TcpServer & parent);

    void parse_line(const std::string & str);

    std::string get_response();

    void generate_response();

protected:

    virtual int parse_recv(const Byte * data, unsigned len);

    void parse_request_line(const std::string & str);

    void parse_options_request(const std::string & str);

    std::string generate_options_response();

    void parse_describe_request(const std::string & str);
   
    std::string generate_describe_response();

    void parse_setup_request(const std::string & str);
    
    std::string generate_setup_response();

    void parse_play_request(const std::string & str);
    
    std::string generate_play_response();

private:
    enum T_STATE
    {
	PARSING_REQUEST_LINE,
	PARSING_OPTIONS_REQUEST,
	PARSING_DESCRIBE_REQUEST,
	PARSING_SETUP_REQUEST,
	PARSING_PLAY_REQUEST,
    };
    T_STATE mState;

    std::string mCseq;
       	
    std::string mSession;

    unsigned short mClientRtpPort;
    unsigned short mServerRtpPort;

    unsigned short mClientRtcpPort;
    unsigned short mServerRtcpPort;
};

#endif
