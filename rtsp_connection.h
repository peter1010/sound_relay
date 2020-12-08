#ifndef _RTSP_CONNECTION_H_
#define _RTSP_CONNECTION_H_

#include <string>
#include "tcp_connection.h"


class RtspConnection : public TcpConnection
{
public:
    RtspConnection();

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


    int get_session_id() const { return 12; };
    int get_session_ver() const { return  13; };
    int get_rtsp_server_port() const { return 554; };
    int get_our_rtp_port() const { return mServerRtpPort; };
    const char * get_pathname() const { return "tv"; };

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
