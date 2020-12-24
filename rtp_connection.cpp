
#include <sys/random.h>

#include "logging.h"
#include "rtp_connection.h"
#include "capture.h"


/*----------------------------------------------------------------------------*/
RtpConnection::RtpConnection(unsigned char payloadType)
{
    LOG_DEBUG("RtpConnection");
    
//    mSource = new Capture();
//    mSource.init();

    getrandom(&mSequenceNumber, sizeof(mSequenceNumber), 0);
    getrandom(&mTimeStamp, sizeof(mTimeStamp), 0);

    mPacket = new Byte[MAX_PKT_SIZE];

 
    // First byte is version, P, X, CC
    mPacket[0] = (RTP_VERSION << 6) | (RTP_PADDING << 5) | (RTP_EXTENSION << 4) 
	    | (RTP_CSRC_COUNT);
    mPacket[1] = payloadType;
    // SSRC
    getrandom(&mPacket[8], 4, 0);
}


/*----------------------------------------------------------------------------*/
RtpConnection::~RtpConnection()
{
    if(mPacket) {
	delete [] mPacket;
	mPacket = 0;
    }
}


/*----------------------------------------------------------------------------*/
bool RtpConnection::parse_recv(const Byte * pData, unsigned len)
{
    return true;
}


/*----------------------------------------------------------------------------*/
void RtpConnection::send_packet(unsigned num_to_send, unsigned timeDuration)
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
