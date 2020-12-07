
#include <opus/opus.h>
#include "logging.h"
#include "rtp_connection.h"
#include "capture.h"

#define RTP_VERSION (2)
#define RTP_PADDING (0)
#define RTP_EXTENSION (0)
#define RTP_CSRC_COUNT (0)


/*----------------------------------------------------------------------------*/
RtpConnection::RtpConnection()
{
    LOG_DEBUG("RtpConnection");
    
//    mSource = new Capture();
//    mSource.init();


#if 0
    // First byte is version, P, X, CC
    pData[0] = (RTP_VERSION << 6) | (RTP_PADDING << 5) | (RTP_EXTENSION << 4) 
	    | (RTP_CSRC_COUNT);
    pData[1] = 
#endif
}


/*----------------------------------------------------------------------------*/
bool RtpConnection::parse_recv(const Byte * pData, unsigned len)
{
    return true;
}

#if 0
RtpConnection::send()
{

}
#endif
