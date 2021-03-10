import socket

import media
import network_interfaces

class RtspConnection(object):

    def __init__(self, parent, connfd, client_address):
        self.connfd = connfd
        self.read_buf = b''
        self.parent = parent

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
        self.parse_request(lines)
        return True

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
            attributes[name.lower()] = value
        print(attributes)
        if request == b"OPTIONS":
            self.parse_option_request(attributes)
        elif request == b"DESCRIBE":
            self.parse_describe_request(url, attributes)
        elif request == b"SETUP":
             self.parse_setup_request(url, attributes)
        elif request == b"PLAY":
            self.parse_play_request(attributes)
        else:    
            print("Unknown request ", request);


    def parse_option_request(self, attributes):
        lines = [
            b"RTSP/1.0 200 OK",
	        b"CSeq: " + attributes[b"cseq"],
	        b"Public: DESCRIBE, SETUP, PLAY, TEARDOWN",
            b"", b""]

        print(lines)
        self.connfd.send(b"\r\n".join(lines))


    def parse_describe_request(self, url, attributes):
#	if(name == "cseq") {
#	    mCseq = value;
 #       } else if(name == "accept") {
#	    // Check that application/sdp is an option
#	    if(value.find("application/sdp") == std::string::npos) {
#		// TODO


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
	    	    self.get_rtsp_server_port(), session.get_pathname()),
	        "m=audio {0} RTP/AVP {1}".format(session.get_our_rtp_port(), pt),
	        "a=rtpmap:{0} opus/{1}/{2}".format(pt, session.get_raw_bit_rate(),
		        session.get_num_of_channels()),
            ""]
        sdp = "\r\n".join(sdp)
        sdp = sdp.encode(encoding = "UTF-8")

        lines = [
            b"RTSP/1.0 200 OK",
	        b"CSeq: " + attributes[b"cseq"],
	        b"Content-Type: application/sdp",
	        b"Content-Length: " + str(len(sdp)).encode(encoding="UTF-8"),
            b"", b""]
        self.connfd.send(b"\r\n".join(lines))
        self.connfd.send(sdp)


    def parse_setup_request(self, url, attributes):
        session = media.get_session(url)

        value = attributes[b"transport"]

        tokens = value.split(b";")
        proto = tokens[0].strip()

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
        if len(ports) > 1:
            session.set_peer_rtp_port(int(ports[0]))
            session.set_peer_rtcp_port(int(ports[1]))
        else:
            session.set_peer_rtp_port(int(ports[0]))
            session.set_peer_rtcp_port(int(ports[0])+1)

        peerAddress = self.get_peer_address();
#    if(peerAddress.is_ipv4()) {
#        pSession->set_our_address(get_hostip(IPv4));
#    } else {
#        pSession->set_our_address(get_hostip(IPv6));
#    }
        session.add_peer_address(peerAddress);

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
            b"Session: " + session.get_id(),
            b"Transport: " + transport,
            b"", b""]

        print(lines)
        self.connfd.send(b"\r\n".join(lines))


    def parse_play_request(self, url, attributes):
        session = media.get_session(url)

        session.play()

#    // Mandatory fields:
#    // CSeq:
        lines = [
            b"RTSP/1.0 200 OK",
	        b"CSeq: " + attributes[b"cseq"],
            b"", b""]

        print(lines)
        self.connfd.send(b"\r\n".join(lines))



    def get_hostip(self, ver):
        return network_interfaces.get_hostip(self.connfd, ver)

    def get_hostname(self):
        return socket.getfqdn()

    def get_rtsp_server_port(self):
        self.parent.get_listening_port()
