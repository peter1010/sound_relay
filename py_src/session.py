import os

class Session(object):

	def __init__(self):
		self.OurRtpPort = 49176;
		self.OurRtcpPort = self.OurRtpPort + 1

	def get_sdp_id(self):
		return 12

	def get_sdp_ver(self):
		return  13

	def get_id(self):
		return b"12345"


	#const IpAddress & get_our_address() const {return mOurAddress;};
	def get_our_rtp_port(self):
		return self.OurRtpPort

	def get_our_rtcp_port(self):
		return self.OurRtcpPort
 

	#const IpAddress & get_peer_address() const {return mPeerAddress;};
	def get_peer_rtp_port(self):
		return self.PeerRtpPort

	def get_peer_rtcp_port(self):
		return self.PeerRtcpPort

	def set_our_address(self, addr):
		self.OurAddress = addr;

	def add_peer_address(self, addr):
		self.PeerAddress = addr

	def set_peer_rtp_port(self, port):
		self.PeerRtpPort = port

	def set_peer_rtcp_port(self, port):
		self.PeerRtcpPort = port

	def get_pathname(self):
		return "tv"

#    Capture * get_source() const { return mpSource; };

	def get_payload_type(self):
		return 97
    
	def get_raw_bit_rate(self):
		return 48000

	def get_num_of_channels(self):
		return 2

	def get_player(self):
		poss = [
			("..", "c_src", "__armv6l__", "sound_relay"),
		    ("c_src", "__armv6l__", "sound_relay")
		]
		for parts in poss:
			filepath = os.path.join(*parts)
			print(filepath)
			if os.path.exists(filepath):
				return filepath

	def play(self):
		pid = os.fork()
		if pid == 0:
			# Child
			player = self.get_player()
			args = [os.path.basename(player),
					"-c", str(self.PeerRtpPort),
					"-b", str(self.PeerRtcpPort),
					"-a", str(self.PeerAddress),
					"-z", str(self.OurRtpPort),
					"-y", str(self.OurRtcpPort),
					"-x", str(self.OurAddress)]
			print(args)
			os.execvp(player, args)

		print("play...")

#    void disconnect();

#    IpAddress mOurAddress;
#    unsigned short mOurRtpPort;
#    unsigned short mOurRtcpPort;

#    IpAddress mPeerAddress;
#    unsigned short mPeerRtpPort;
#    unsigned short mPeerRtcpPort;

#    Capture * mpSource;
#    RtpClient * mpRtp;	
#    RtcpClient * mpRtcp;

