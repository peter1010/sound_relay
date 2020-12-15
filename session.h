#ifndef _SESSION_H_
#define _SESSION_H_

#include "ip_address.h"

class Capture;
class RtpClient;

class Session
{
public:
    Session();
    ~Session();

    int get_sdp_id() const { return 12; };
    int get_sdp_ver() const { return  13; };
    const char * get_id() const { return "12345"; };

    const IpAddress & get_our_address() const {return mOurAddress;};
    unsigned get_our_rtp_port() const { return mOurRtpPort; };
    unsigned get_our_rtcp_port() const { return mOurRtcpPort; };
 

    const IpAddress & get_peer_address() const {return mPeerAddress;};
    unsigned get_peer_rtp_port() const { return mPeerRtpPort; };
    unsigned get_peer_rtcp_port() const { return mPeerRtcpPort; };

    void set_our_address(const IpAddress & addr) {mOurAddress = addr;};
    void add_peer_address(const IpAddress & addr) {mPeerAddress = addr;};

    void set_peer_rtp_port(unsigned short port) { mPeerRtpPort = port; };
    void set_peer_rtcp_port(unsigned short port) { mPeerRtcpPort = port; };

    const char * get_pathname() const { return "tv"; };

    Capture * get_source() const { return mpSource; };

    void play();

private:

    IpAddress mOurAddress;
    unsigned short mOurRtpPort;
    unsigned short mOurRtcpPort;

    IpAddress mPeerAddress;
    unsigned short mPeerRtpPort;
    unsigned short mPeerRtcpPort;

    Capture * mpSource;
    RtpClient * mpRtp;	
};

#endif
