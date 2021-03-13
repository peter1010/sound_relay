#include <string.h>

#include "logging.h"
#include "rtcp_connection.h"
#include "rtcp_client.h"
#include "session.h"
#include "capture.h"


/*----------------------------------------------------------------------------*/
RtcpClient::RtcpClient(const Session & session)
{
    LOG_DEBUG("RtcpClient");

//  mPayloadType = session.get_payload_type();
//  mpSource = session.get_source();
//    mSink = session.get_sink();

    int sock = init(session.get_peer_rtcp_port(), session.get_peer_address(),
		    session.get_our_rtcp_port(), session.get_our_address());
    mpConn = new RtcpConnection();
	mpConn->attach(sock, *this);
//    mpSource->attach(conn);
}


/*----------------------------------------------------------------------------*/
RtcpClient::~RtcpClient() 
{
    LOG_DEBUG("~RtpClient");
}


