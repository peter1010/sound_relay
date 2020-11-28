#include "sender.h"


#if 0

int rtpSock = socket(AF_INET, SOCK_DGRAM, 0);

int rtcpSock = socket(AF_INET, SOCK_DGRAM. 0);


Sender::init() {

    struct sockaddr_in rtsp_addr;

    // Start by opening a connection to the server
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if(sock == -1) {
        LOG_ERROR("Failed to open RTSP socket");
    }

    memset(rtsp_addr, 0, sizeof(their_addr);
    rtsp_addr.sin_family = AF_INET;
    rtsp_addr.sin_port = htons(mRtspPort);
    rtsp_addr.sin_addr = mRtspIpAddr;

    int status = connect(sock, reinterpret_cast<struct sockaddr *> &rtsp_addr, sizeof(rtsp_addr));
}
    
#endif
