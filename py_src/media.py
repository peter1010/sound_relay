class Session(object):

    def get_sdp_id(self):
        return 12

    def get_sdp_ver(self):
        return  13

    def get_id(self):
        return b"12345"


    #const IpAddress & get_our_address() const {return mOurAddress;};
    #unsigned get_our_rtp_port() const { return mOurRtpPort; };
    #unsigned get_our_rtcp_port() const { return mOurRtcpPort; };
 

    #const IpAddress & get_peer_address() const {return mPeerAddress;};
    #unsigned get_peer_rtp_port() const { return mPeerRtpPort; };
    #unsigned get_peer_rtcp_port() const { return mPeerRtcpPort; };

    #void set_our_address(const IpAddress & addr) {mOurAddress = addr;};
    #void add_peer_address(const IpAddress & addr) {mPeerAddress = addr;};

    #void set_peer_rtp_port(unsigned short port) { mPeerRtpPort = port; };
    #void set_peer_rtcp_port(unsigned short port) { mPeerRtcpPort = port; };

    def get_pathname(self):
        return "tv"

#    Capture * get_source() const { return mpSource; };

    def get_payload_type(self):
        return 97
    
    def get_raw_bit_rate(self):
        return 48000

    def get_num_of_channels():
        return 2

#    void play();

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


session = Session()


def get_session(url):
    return session
