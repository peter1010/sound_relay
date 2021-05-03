#include <string.h>
#include <sys/random.h>

#include "logging.h"
#include "rtp_server.h"
#include "session.h"
#include "replay.h"


/*----------------------------------------------------------------------------*/
RtpServer::RtpServer(const Session & session)
{
    LOG_DEBUG("RtpServer");

    const uint8_t payloadType = session.get_payload_type();
    mpSink = session.get_sink();
//    mSink = session.get_sink();


    // Note init will call RtpClient::connection_factory!
    init(session.get_peer_rtp_port(), session.get_peer_address(),
		    session.get_our_rtp_port(), session.get_our_address());

	mpSink->attach(this);

//	getrandom(&mSequenceNumber, sizeof(mSequenceNumber), 0);
//    getrandom(&mTimeStamp, sizeof(mTimeStamp), 0);

    mPacket = new uint8_t[MAX_PKT_SIZE];

    // First byte is version, P, X, CC
    mPacket[0] = (RTP_VERSION << 6) | (RTP_PADDING << 5) | (RTP_EXTENSION << 4)
	    | (RTP_CSRC_COUNT);
    mPacket[1] = payloadType;
    // SSRC
//    getrandom(&mPacket[8], 4, 0);
}


/*----------------------------------------------------------------------------*/
RtpServer::~RtpServer() 
{
    if(mPacket) {
		delete [] mPacket;
		mPacket = 0;
	}
    LOG_DEBUG("~RtpServer");
}


/******************************************************************************/
bool RtpServer::parse_recv(const uint8_t * pData, unsigned len)
{
	const uint16_t sequenceNumber = (static_cast<uint16_t>(pData[2]) << 8) | pData[3];

	const uint32_t timeStamp = (static_cast<uint32_t>(pData[4]) << 24) |
						 (static_cast<uint32_t>(pData[5]) << 16) |
						 (static_cast<uint16_t>(pData[6]) << 8) | pData[7];
//	LOG_DEBUG("%u", sequenceNumber);

	mpSink->write(&pData[RTP_HEADER_LEN], len - RTP_HEADER_LEN);
    return true;
}

#if 0
/******************************************************************************/
void RtpClient::send_packet(unsigned num_to_send, unsigned timeDuration)
{
    ++mSequenceNumber;
    mTimeStamp += timeDuration;

    mPacket[2] = (mSequenceNumber >> 8) & 0xFF;
    mPacket[3] = mSequenceNumber & 0xFF;
    mPacket[4] = (mTimeStamp >> 24) & 0xFF;
    mPacket[5] = (mTimeStamp >> 16) & 0xFF;
    mPacket[6] = (mTimeStamp >> 8) & 0xFF;
    mPacket[7] = mTimeStamp & 0xFF;

    send(mPacket, num_to_send + RTP_HEADER_LEN);
}
#endif
