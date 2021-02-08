
#include <sys/random.h>
#include <endian.h>

#include "logging.h"
#include "rtcp_connection.h"
#include "capture.h"

/******************************************************************************/
static uint32_t get_uint32(const uint8_t * pData)
{
    return (static_cast<uint32_t>(pData[0]) << 24) |
    	  		(static_cast<uint32_t>(pData[1]) << 16) |
    				(static_cast<uint16_t>(pData[2]) << 8) | pData[3];
}


/******************************************************************************/
static uint32_t get_uint24(const uint8_t * pData)
{
    return (static_cast<uint32_t>(pData[0]) << 16) |
    				(static_cast<uint16_t>(pData[1]) << 8) | pData[2];
}


/******************************************************************************/
static uint32_t get_uint16(const uint8_t * pData)
{
    return (static_cast<uint16_t>(pData[0]) << 8) | pData[1];
}


/******************************************************************************/
RtcpConnection::RtcpConnection()
{
    LOG_DEBUG("RtcpConnection");
    
//    mSource = new Capture();
//    mSource.init();

//    mPacket = new Byte[MAX_PKT_SIZE];
}


/******************************************************************************/
RtcpConnection::~RtcpConnection()
{
//    if(mPacket) {
//	delete [] mPacket;
//	mPacket = 0;
//    }
}


/******************************************************************************/
bool RtcpConnection::parse_recv(const Byte * pData, unsigned len)
{
    LOG_DEBUG("RTCP message received %u", len);	

    unsigned left =  len;
    while(left >= 8) {
	unsigned rc = pData[0] & 0x1F;	// Reception report count
        unsigned pt = pData[1];		// packet type
    
        unsigned length = get_uint16(&pData[2]);
	LOG_DEBUG("length %u", length);
        uint32_t ssrc = get_uint32(&pData[4]);
	LOG_DEBUG("ssrc %u", ssrc);
	

	pData += 8;
	left -= 8;

        switch(pt) {
	case 201: 	// RR
	    while(rc > 0) {
        	uint32_t ssrc = get_uint32(pData);
		LOG_DEBUG("ssrc %u", ssrc);
		unsigned fraction = pData[4];
		LOG_DEBUG("fraction %u", fraction);
		unsigned cumulative = get_uint24(&pData[5]);
		LOG_DEBUG("cumulative %u", cumulative);
		unsigned highest_seq_rcvd = get_uint24(&pData[8]);
		LOG_DEBUG("higest seq rcvd %u", highest_seq_rcvd);
		unsigned jitter = get_uint32(&pData[12]);
		LOG_DEBUG("jitter %u", jitter);
		unsigned lsr = get_uint32(&pData[16]);
		LOG_DEBUG("lsr %u", lsr);
		unsigned dlsr = get_uint32(&pData[20]);
		LOG_DEBUG("dlsr %u", dlsr);
		rc--;
		pData += 24;
		left -= 24;
	    }
	    break;

	default:
	    LOG_DEBUG("pt = %u", pt);

	    break;
	}
    }
    return true;
}


/*----------------------------------------------------------------------------*/
void RtcpConnection::send_packet()
{
}
