import socket

try:
	from . import media
	from . import network_interfaces
	from .session import ChangeNotAllowedError
except ImportError:
	import media
	import network_interfaces
	from session import ChangeNotAllowedError

class RtspConnection(object):

	def __init__(self, connfd, peer_address, listening_port):
		self.connfd = connfd
		self.read_buf = b''
		self.peer_address = peer_address
		self.listening_port = listening_port

	def fileno(self):
		return self.connfd.fileno()


	def read(self):
		new_data = self.connfd.recv(512)
		if not new_data:
			return False
		read_buf = self.read_buf + new_data
		if read_buf[-4:] != b'\r\n\r\n':
			self.read_buf = read_buf
			return True
		self.read_buf = b''
		lines = read_buf[:-4].splitlines()
		return self.parse_request(lines)

	def parse_request(self, lines):
		tokens = lines[0].split()
		request = tokens[0]
		version = tokens[-1]
		url = b" ".join(tokens[1:-1])
		if url.startswith(b"rtsp://"):
			url = url[7:]

		attributes = {}
		for line in lines[1:]:
			name, value = line.split(b":", 1)
			name = name.strip()
			value = value.strip()
			attributes[name.lower()] = value
		print("Recv: ", request)
		print(attributes)
		if request == b"OPTIONS":
			return self.parse_option_request(attributes)
		if request == b"DESCRIBE":
			return self.parse_describe_request(url, attributes)
		if request == b"SETUP":
			return self.parse_setup_request(url, attributes)
		if request == b"PLAY":
			return self.parse_play_request(url, attributes)
		if request == b"TEARDOWN":
			return self.parse_teardown_request(url, attributes)
		if request in (b"RECORD", b"ANNOUNCE",
				b"GET_PARAMETER", b"PAUSE", b"REDIRECT",
				b"SET_PARAMETER"):
			return self.Error(405, attributes)
		return self.Error(400, attributes)

	def keep_alive(self, attributes):
		if b"connection" in attributes:
			value = attributes[b"connection"]
			if value.lower() == b"close":
				return False
		return True

	def parse_option_request(self, attributes):
		if b"require" in attributes:
			return self.Error(551, attributes)
		if b"proxy-require" in attributes:
			return self.Error(551, attributes)
		lines = [
			b"RTSP/1.0 200 OK",
			b"CSeq: " + attributes[b"cseq"],
			b"Public: DESCRIBE, SETUP, PLAY, TEARDOWN",
			b"", b""]

		print(lines)
		self.connfd.send(b"\r\n".join(lines))
		return self.keep_alive(attributes)


	def parse_describe_request(self, url, attributes):
		if b"require" in attributes:
			return self.Error(551, attributes)
		if b"proxy-require" in attributes:
			return self.Error(551, attributes)

		session = media.get_session(url)

#    // Mandatory fields:
#    // CSeq:
#    // Content-type:
#    // Content-Length;

#    // SDP fields
#    // v= (protocol version)
#    // o = (origintator and session identifier)
#    // s = (session name)
#    // t = (time the session is active)
#    // m = (media name and transport address)

		pt = str(session.get_payload_type())
		sdp = [
			"v=0",
			"o=- {0} {1} IN IP4 {2}".format(session.get_sdp_id(),
			session.get_sdp_ver(), self.get_hostip(4)),
			"s=TV Session",
			"i=Sound from the TV",
			"t= 0 0",
			"a=recvonly",
			"a=control:rtsp://{0}:{1}/{2}".format(self.get_hostname(),
			self.listening_port, session.get_pathname()),
			"m=audio {0} RTP/AVP {1}".format(session.get_our_rtp_port(), pt),
			"a=rtpmap:{0} opus/{1}/{2}".format(pt, session.get_raw_bit_rate(),
			session.get_num_of_channels()),
			""]
		sdp = "\r\n".join(sdp)
		sdp = sdp.encode(encoding = "UTF-8")

# Content-Encoding not needed as entity is in plain text
# Content-Language not needed as entity is applicabel to all
		lines = [
			b"RTSP/1.0 200 OK",
			b"CSeq: " + attributes[b"cseq"],
			b"Content-Type: application/sdp",
			b"Content-Length: " + str(len(sdp)).encode(encoding="UTF-8"),
			b"", b""]
		self.connfd.send(b"\r\n".join(lines))
		self.connfd.send(sdp)
		return self.keep_alive(attributes)


	def Error(self, num, attributes):
		msgs = {
			400 : "Bad Request",
			405 : "Method Not Allowed",
			454 : "Session Not Found",
			455 : "Method Not Valid In This State",
			459 : "Aggregate Operation Not Allowed",
			461 : "Unsupported Transport",
			551 : "Option not supported",
		}
		lines = [
			"RTSP/1.0 {0} {1}".format(num, msgs[num]).encode(encoding = "UTF-8"),
			b"CSeq: " + attributes[b"cseq"],
		]
		if num == 405:
			lines.append(b"Allow: DESCRIBE, SETUP, PLAY, TEARDOWN")
		if num == 551:
			if b"require" in attributes:
				unsupported = attributes[b"require"]
			else:
				unsupported = attributes[b"proxy-require"]
			lines.append(b"Unsupported: {0}".format(unsupported).encode(encoding = "UTF-8"))
		lines += [
			b"", 
			b""]
		print(lines)
		self.connfd.send(b"\r\n".join(lines))
		return self.keep_alive(attributes)


	def parse_setup_request(self, url, attributes):
		if b"require" in attributes:
			return self.Error(551, attributes)
		if b"proxy-require" in attributes:
			return self.Error(551, attributes)
		session = media.get_session(url)

		value = attributes[b"transport"]
		# We dont do aggregated sessions...
		if b"session" in attributes:
			return self.Error(459, attributes)

		tokens = value.split(b";")
		for token in tokens:
			token = token.strip()

		proto = tokens[0].strip()

		if proto.upper() not in (b"RTP/AVP", b"RTP/AVP/UDP"):
			return self.Error(461, attributes)

		print("proto=", proto)
#        print(tokens)

		values = {}
		for token in tokens[1:]:
			idx = token.find(b"=")
			if idx > 0:
				name, value = token[:idx].strip().lower(), token[idx+1:].strip()
			else:
				name = token.strip().lower()
				value = True
			values[name] = value

		ports = values[b"client_port"].split(b"-")
		try:
			if len(ports) > 1:
				session.set_peer_rtp_port(int(ports[0]))
				session.set_peer_rtcp_port(int(ports[1]))
			else:
				session.set_peer_rtp_port(int(ports[0]))
				session.set_peer_rtcp_port(int(ports[0])+1)

			if self.peer_address.find(".") >= 0:
				session.set_our_address(self.get_hostip(4))
			else:
				session.set_our_address(self.get_hostip(6))

			session.add_peer_address(self.peer_address);
		except ChangeNotAllowedError as err:
			return self.Error(455, attributes)

#    // Mandatory fields:
#    // CSeq:
#    // Session:
#    // Transport:
		transport = "RTP/AVP/UDP;unicast;client_port=" \
            + str(session.get_peer_rtp_port()) + "-" + str(session.get_peer_rtcp_port()) \
            + ";server_port=" + str(session.get_our_rtp_port()) + "-" \
            + str(session.get_our_rtcp_port())
		transport = transport.encode(encoding="UTF-8")

		lines = [
			b"RTSP/1.0 200 OK",
			b"CSeq: " + attributes[b"cseq"],
			b"Session: " + session.get_rtp_id(),
			b"Transport: " + transport,
			b"", b""]

		print(lines)
		self.connfd.send(b"\r\n".join(lines))
		return self.keep_alive(attributes)


	def parse_play_request(self, url, attributes):
		if b"require" in attributes:
			return self.Error(551, attributes)
		if b"proxy-require" in attributes:
			return self.Error(551, attributes)
		session_id = None
		if b"session" in attributes:
			session_id = attributes[b"session"]
		print(session_id)
		session = media.get_session(url)
		if session.get_rtp_id() != session_id:
			return self.Error(454, attributes)

		session.play()

#    // Mandatory fields:
#    // CSeq:
		lines = [
			b"RTSP/1.0 200 OK",
			b"CSeq: " + attributes[b"cseq"],
			b"Session: " + session.get_rtp_id(),
			b"", b""]

		print(lines)
		self.connfd.send(b"\r\n".join(lines))
		return self.keep_alive(attributes)


	def parse_teardown_request(self, url, attributes):
		if b"require" in attributes:
			return self.Error(551, attributes)
		if b"proxy-require" in attributes:
			return self.Error(551, attributes)
		session_id = None
		if b"session" in attributes:
			session_id = attributes[b"session"]
		session = media.get_session(url)
		if session.get_rtp_id() != session_id:
			return self.Error(454, attributes)
		session.stop()
		return self.keep_alive(attributes)


	def get_hostip(self, ver):
		return network_interfaces.get_hostip(self.connfd, ver)

	def get_hostname(self):
		return socket.getfqdn()
