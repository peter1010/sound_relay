#ifndef _SESSION_H_
#define _SESSION_H_

#include "ip_address.h"

class Capture;
class Replay;
//class RtpClient;
//class RtcpServer;
class UdpConnection;
class Sound;

class Session
{
public:
	Session();
	~Session();

	const IpAddress & get_our_address() const {return mOurAddress;};
	unsigned get_our_rtp_port() const { return mOurRtpPort; };
	unsigned get_our_rtcp_port() const { return mOurRtcpPort; };


	const IpAddress & get_peer_address() const {return mPeerAddress;};
	unsigned get_peer_rtp_port() const { return mPeerRtpPort; };
	unsigned get_peer_rtcp_port() const { return mPeerRtcpPort; };


	void set_our_address(const char * addr);
	void set_our_rtp_port(const char * port);
	void set_our_rtcp_port(const char * port);


	void set_peer_address(const char * addr);
	void set_peer_rtp_port(const char * port);
	void set_peer_rtcp_port(const char * port);

	Capture * get_source() const;
	Replay * get_sink() const;

	unsigned char get_payload_type() const { return 97; };
	unsigned short get_raw_bit_rate() const { return 48000; };
	unsigned char get_num_of_channels() const { return 2; };

	void capture();
	void play();

	void disconnect();
private:

	IpAddress mOurAddress;
	unsigned short mOurRtpPort;
	unsigned short mOurRtcpPort;

	IpAddress mPeerAddress;
	unsigned short mPeerRtpPort;
	unsigned short mPeerRtcpPort;

	Sound * mpSound;
	UdpConnection * mpRtp;
	UdpConnection * mpRtcp;
};

#endif
