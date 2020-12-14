#include <string.h>

#include "logging.h"
#include "rtp_connection.h"
#include "rtp_client.h"
#include "session.h"

/*----------------------------------------------------------------------------*/
RtpClient::RtpClient(const Session & session)
{
    LOG_DEBUG("RtpClient");

    init(session.get_peer_rtp_port(), session.get_peer_address(),
		    session.get_our_rtp_port(), session.get_our_address());

//    mSource = session.get_source();
//    mSink = session.get_sink();

    register_connection_factory(RtpClient::connection_factory);
}


/*----------------------------------------------------------------------------*/
RtpClient::~RtpClient() 
{
    LOG_DEBUG("~RtpClient");
}


/*----------------------------------------------------------------------------*/
Connection * RtpClient::connection_factory()
{
    RtpConnection * conn = new RtpConnection();
//    mSource->attach(conn);
    return conn;
}




