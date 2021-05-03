#include <string.h>
#include <endian.h>

#include "logging.h"
#include "rtcp_client.h"
#include "session.h"
#include "capture.h"


/*----------------------------------------------------------------------------*/
RtcpClient::RtcpClient(const Session & session)
{
    LOG_DEBUG("RtcpServer");

//  mPayloadType = session.get_payload_type();
//  mpSource = session.get_source();
//    mSink = session.get_sink();

    init(session.get_peer_rtcp_port(), session.get_peer_address(),
		    session.get_our_rtcp_port(), session.get_our_address());
//    mpSource->attach(conn);

}


/*----------------------------------------------------------------------------*/
RtcpClient::~RtcpClient() 
{
    LOG_DEBUG("~RtpClient");
}


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
bool RtcpClient::parse_recv(const uint8_t * /* pData */, unsigned /* len */)
{
	return true;
}

#if 0
    unsigned pktLeft = len;
    while(pktLeft >= 4) {
	unsigned rc = *pData++ & 0x1F;	// Reception report count
        unsigned pt = *pData++;		// packet type
    
        const unsigned length = (get_uint16(pData) + 1) * 4;
	pData += 2;
	LOG_DEBUG("length %u", length);

	unsigned left = length - 4;

        switch(pt) {

	case 201: 	// RR
	{
            uint32_t ssrc = get_uint32(pData);
	    LOG_DEBUG("ssrc %u", ssrc);
	    pData += 4;
	    left -= 4;

	    while((rc > 0) && (left > 0)) {
        	uint32_t ssrc = get_uint32(pData);
		LOG_DEBUG("ssrc_1 %u", ssrc);
		unsigned fraction = pData[4];
		LOG_DEBUG("fraction %u", fraction);
		unsigned cumulative = get_uint24(&pData[5]);
		LOG_DEBUG("cumulative %u", cumulative);
		unsigned highest_seq_rcvd = get_uint24(&pData[8]);
		LOG_DEBUG("highest seq rcvd %u", highest_seq_rcvd);
		unsigned jitter = get_uint32(&pData[12]);
		LOG_DEBUG("jitter %u", jitter);
		unsigned lsr = get_uint32(&pData[16]);
		LOG_DEBUG("last SR %u", lsr);
		unsigned dlsr = get_uint32(&pData[20]);
		LOG_DEBUG("delay since last SR %u", dlsr);
		rc--;
		pData += 24;
		left -= 24;
	    }
	}
	break;

	case 202: 
	    while((rc > 0) && (left > 0)) {
        	uint32_t ssrc = get_uint32(pData);
		LOG_DEBUG("ssrc_1 %u", ssrc);
	        pData += 4;
	        left -= 4;
		unsigned n = 0;
		while(1) {
		    unsigned typ = pData[n++];
		    if(typ == 0) {
		        break;
		    }
		    unsigned cnt = pData[n++];
		    LOG_DEBUG("typ=%u cnt = %u \'%.*s\'", typ, cnt, cnt, pData);
		    n += cnt;
		}
		n = (n + 3) & ~0x3;
		rc--;
		pData += n;
		left -= n;
	    }
	    break;

	default:
	    LOG_DEBUG("pt = %u", pt);

	    break;
	}
	if(left > 0) {
	    pData += left;
	}
	pktLeft -= length;
    }
    return true;
}
#endif
