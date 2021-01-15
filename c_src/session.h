#ifndef _SESSION_H_
#define _SESSION_H_

#include "ip_address.h"

class Capture;
class RtpClient;
class RtcpClient;

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

    unsigned char get_payload_type() const { return 97; };
    unsigned short get_raw_bit_rate() const { return 48000; };
    unsigned char get_num_of_channels() const { return 2; };

    void play();

    void disconnect();
private:

    IpAddress mOurAddress;
    unsigned short mOurRtpPort;
    unsigned short mOurRtcpPort;

    IpAddress mPeerAddress;
    unsigned short mPeerRtpPort;
    unsigned short mPeerRtcpPort;

    Capture * mpSource;
    RtpClient * mpRtp;	
    RtcpClient * mpRtcp;
};

#endif