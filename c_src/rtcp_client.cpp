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


    register_connection_factory(RtcpClient::connection_factory, this);

    // Note init will call RtpClient::connection_factory!
    init(session.get_peer_rtcp_port(), session.get_peer_address(),
		    session.get_our_rtcp_port(), session.get_our_address());
}


/*----------------------------------------------------------------------------*/
RtcpClient::~RtcpClient() 
{
    LOG_DEBUG("~RtpClient");
}


/*----------------------------------------------------------------------------*/
UdpConnection * RtcpClient::connection_factory(void * pArg)
{
    LOG_DEBUG("rtcp connection factory");

    RtcpClient * pThis = reinterpret_cast<RtcpClient *>(pArg);
    RtcpConnection * conn = new RtcpConnection();
//    pThis->mpSource->attach(conn);
    return conn;
}




