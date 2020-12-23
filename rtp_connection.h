#ifndef _RTP_CONNECTION_H_
#define _RTP_CONNECTION_H_

#include <cstdint>
#include <string>
#include "udp_connection.h"


class RtpConnection : public UdpConnection
{
public:
    RtpConnection(unsigned char payloadType);
    ~RtpConnection();

    Byte * get_packet_buffer() const { return &mPacket[RTP_HEADER_LEN]; };
    unsigned get_packet_buffer_size() const { return MAX_PKT_SIZE - RTP_HEADER_LEN; };

    void send_packet(unsigned num_to_send, unsigned timeDuration);

protected:

    virtual bool parse_recv(const Byte * data, unsigned len);

private:
   enum {
	RTP_VERSION = 2,
	RTP_PADDING = 0,
	RTP_EXTENSION = 0,
	RTP_CSRC_COUNT = 0,

	MAX_PKT_SIZE = 2048,

	RTP_HEADER_LEN = 12 + RTP_CSRC_COUNT * 4
   };


    Byte * mPacket;
    uint16_t mSequenceNumber;
    uint32_t mTimeStamp;
};

#endif
