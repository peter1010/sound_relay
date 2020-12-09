#ifndef _SESSION_H_
#define _SESSION_H_


class Session
{
public:
    Session();
    ~Session();

    int get_session_id() const { return 12; };
    int get_session_ver() const { return  13; };

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
