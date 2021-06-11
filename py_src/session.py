import os
import time
import signal

class ChangeNotAllowedError(Exception):
	pass

class Session(object):

	def __init__(self, sdp_id):
		self.OurRtpPort = 49176;
		self.OurRtcpPort = self.OurRtpPort + 1
		self.sdp_id = sdp_id
		self.rtp_id = int(time.time())
		self.pid = None

	def get_sdp_id(self):
		"""SDP Session ID, A numeric string such that tuple of <username>, <sess_id>, <nettype>,
		<addrype> and <address> is globally unique for this session"""
		return self.sdp_id

	def get_sdp_ver(self):
		"""SDP Session version, Version number of the SDP, must increment on change to SDP"""
		return  13

	def get_rtp_id(self):
		"""RTP Session ID used in the RTSP SETUP Response"""
		return str(self.rtp_id).encode('utf-8')

	def get_our_rtp_port(self):
		return self.OurRtpPort

	def get_our_rtcp_port(self):
		return self.OurRtcpPort
 
	def get_peer_rtp_port(self):
		return self.PeerRtpPort

	def get_peer_rtcp_port(self):
		return self.PeerRtcpPort

	def set_our_address(self, addr):
		if hasattr(self, "OurAddress") and (addr != self.OurAddress):
			if self.pid:
				raise ChangeNotAllowedError()
		self.OurAddress = addr;

	def add_peer_address(self, addr):
		self.PeerAddress = addr

	def set_peer_rtp_port(self, port):
		if hasattr(self, "PeerRtpPort") and (port != self.PeerRtpPort):
			if self.pid:
				raise ChangeNotAllowedError()
		self.PeerRtpPort = port

	def set_peer_rtcp_port(self, port):
		if hasattr(self, "PeerRtcpPort") and (port != self.PeerRtcpPort):
			if self.pid:
				raise ChangeNotAllowedError()
		self.PeerRtcpPort = port

	def get_pathname(self):
		return "tv"

	def get_payload_type(self):
		return 97
    
	def get_raw_bit_rate(self):
		return 48000

	def get_num_of_channels(self):
		return 2

	def get_player(self):
		poss = [
			("..", "c_src", "__armv6l__", "sound_relay"),
		    ("c_src", "__armv6l__", "sound_relay"),
			("bin", "sound_relay")
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
		else:
			self.pid = pid
		print("play...")

	def stop(self):
		if self.pid:
			os.kill(self.pid, signal.SIGINT)
			self.pid = None
