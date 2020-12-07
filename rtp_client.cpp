#include <string.h>

#include "logging.h"
#include "rtp_connection.h"
#include "rtp_client.h"

/*----------------------------------------------------------------------------*/
RtpClient::RtpClient()
{
    LOG_DEBUG("RtpClient");

//    // Start with port 554
//    if(!init(554)) {
//	init(8554);
//   }
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
    return new RtpConnection();
}




