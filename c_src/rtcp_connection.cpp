
#include <sys/random.h>

#include "logging.h"
#include "rtcp_connection.h"
#include "capture.h"


/*----------------------------------------------------------------------------*/
RtcpConnection::RtcpConnection()
{
    LOG_DEBUG("RtcpConnection");
    
//    mSource = new Capture();
//    mSource.init();

//    mPacket = new Byte[MAX_PKT_SIZE];
}


/*----------------------------------------------------------------------------*/
RtcpConnection::~RtcpConnection()
{
//    if(mPacket) {
//	delete [] mPacket;
//	mPacket = 0;
//    }
}


/*----------------------------------------------------------------------------*/
bool RtcpConnection::parse_recv(const Byte * pData, unsigned len)
{
    LOG_DEBUG("RTCP message received %u", len);	
#if 0
    unsigned rc = pData[0] & 0x1F;
    unsigned pt = pData[1];
    switch(pt) {
	case 201; 	// RR
	    if(rc > 0) {
		unsigned fraction = pt[12];
		unsigned cumulative = (pt[13] << 16) | (pt[14] << 8) | pt[15];
#endif
    return true;
}


/*----------------------------------------------------------------------------*/
void RtcpConnection::send_packet()
{
}
