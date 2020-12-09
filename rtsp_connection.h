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
    int get_our_rtp_port() const { return mServerRtpPort; };
    int get_our_rtcp_port() const { return mServerRtcpPort; };
    int get_peer_rtp_port() const { return mClientRtpPort; };
    int get_peer_rtcp_port() const { return mClientRtcpPort; };
    void set_peer_rtp_port(in_port_t port) { mClientRtpPort = port; };
    void set_peer_rtcp_port(in_port_t port) { mClientRtcpPort = port; };
    const char * get_pathname() const { return "tv"; };
    in_port_t get_rtsp_server_port() const;

private:

    enum T_STATE
    {
	PARSING_REQUEST_LINE,
	PARSING_OPTIONS_REQUEST,
	PARSING_DESCRIBE_REQUEST,
	PARSING_SETUP_REQUEST,
	PARSING_PLAY_REQUEST,
    };
    T_STATE mParsingState;
    std::string mUrl;
    std::string mCseq;


    std::string mSession;

    unsigned short mClientRtpPort;
    unsigned short mServerRtpPort;

    unsigned short mClientRtcpPort;
    unsigned short mServerRtcpPort;
};

#endif
