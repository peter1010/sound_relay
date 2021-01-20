#include <string.h>

#include "logging.h"
#include "rtsp_connection.h"
#include "rtsp_server.h"

/*----------------------------------------------------------------------------*/
RtspServer::RtspServer()
{
    LOG_DEBUG("RtspServer");

    register_connection_factory(RtspServer::connection_factory, this);

    // Start with port 554
    if(!init(554, IpAddress::AnyAddress())) {
	init(8554, IpAddress::AnyAddress());
    }
}


/*----------------------------------------------------------------------------*/
RtspServer::~RtspServer() 
{
    LOG_DEBUG("~RtspServer");
}


/*----------------------------------------------------------------------------*/
Connection * RtspServer::connection_factory(void *)
{
    return new RtspConnection();
}




