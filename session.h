#ifndef _SESSION_H_
#define _SESSION_H_

#include <netinet/ip.h>

class Session
{
public:
    Session();
    ~Session();

    int get_sdp_id() const { return 12; };
    int get_sdp_ver() const { return  13; };
    const char * get_id() const { return "12345"; };

    unsigned get_our_rtp_port() const { return mOurRtpPort; };
    unsigned get_our_rtcp_port() const { return mOurRtcpPort; };
    unsigned get_peer_rtp_port() const { return mPeerRtpPort; };
    unsigned get_peer_rtcp_port() const { return mPeerRtcpPort; };

    void set_peer_rtp_port(in_port_t port) { mPeerRtpPort = port; };
    void set_peer_rtcp_port(in_port_t port) { mPeerRtcpPort = port; };

    const char * get_pathname() const { return "tv"; };
 
private:

    unsigned short mOurRtpPort;
    unsigned short mPeerRtpPort;

    unsigned short mOurRtcpPort;
    unsigned short mPeerRtcpPort;
};

#endif
