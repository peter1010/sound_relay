#include <string.h>

#include "rtsp_server.h"
#include "logging.h"
#include "tcp_connection.h"
#include "rtsp_connection.h"

/*----------------------------------------------------------------------------*/
RtspServer::RtspServer(EventLoop & event_loop) : TcpServer(event_loop)
{
    LOG_DEBUG("RtspServer");

    // Start with port 554
    if(!init(554)) {
	init(8554);
    }
    register_connection_factory(RtspServer::connection_factory);
}


/*----------------------------------------------------------------------------*/
RtspServer::~RtspServer() 
{
    LOG_DEBUG("~RtspServer");
}


/*----------------------------------------------------------------------------*/
TcpConnection * RtspServer::connection_factory(int sock, TcpServer & parent)
{
    return new RtspConnection(sock, parent);
}




