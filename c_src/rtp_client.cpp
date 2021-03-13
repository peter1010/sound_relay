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

    mPayloadType = session.get_payload_type();
    mpSource = session.get_source();
//    mSink = session.get_sink();


    // Note init will call RtpClient::connection_factory!
    int sock = init(session.get_peer_rtp_port(), session.get_peer_address(),
		    session.get_our_rtp_port(), session.get_our_address());

    mpConn = new RtpConnection(mPayloadType);
	mpConn->attach(sock, *this);
	mpSource->attach(mpConn);

}


/*----------------------------------------------------------------------------*/
RtpClient::~RtpClient() 
{
    LOG_DEBUG("~RtpClient");
}


