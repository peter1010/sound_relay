#include <string.h>

#include "logging.h"
#include "rtp_connection.h"
#include "rtp_client.h"
#include "session.h"
#include "capture.h"


/*----------------------------------------------------------------------------*/
RtpClient::RtpClient(const Session & session)
{
    LOG_DEBUG("RtpClient");

    init(session.get_peer_rtp_port(), session.get_peer_address(),
		    session.get_our_rtp_port(), session.get_our_address());

    mpSource = session.get_source();
//    mSink = session.get_sink();

    register_connection_factory(RtpClient::connection_factory, this);
}


/*----------------------------------------------------------------------------*/
RtpClient::~RtpClient() 
{
    LOG_DEBUG("~RtpClient");
}


/*----------------------------------------------------------------------------*/
Connection * RtpClient::connection_factory(void * pArg)
{
    RtpClient * pThis = reinterpret_cast<RtpClient *>(pArg);
    RtpConnection * conn = new RtpConnection();
    pThis->mpSource->attach(conn);
    return conn;
}




