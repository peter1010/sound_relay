#include <string.h>

#include "rtsp_server.h"
#include "logging.h"
#include "tcp_connection.h"

/*----------------------------------------------------------------------------*/
RtspServer::RtspServer(EventLoop & event_loop) : TcpServer(event_loop)
{
    // Start with port 554
    if(!init(554)) {
	init(8554);
    }
}


/*----------------------------------------------------------------------------*/
bool RtspServer::parse_recv(TcpConnection & rConn)
{
    const char * pBuf = rConn.getRecvBuf();
    unsigned len = rConn.getRecvBufLen();

    if((len >= 4) && (strcmp(&pBuf[len-4], "\r\n\r\n") == 0)) {
	LOG_INFO("%s\n", pBuf);
	parse_request(pBuf, len);
	return true;
    } else {
        return true;
    }	
}


void RtspServer::parse_request(pBuf, len)
{
    char * p = skip_spaces(pBuf);
    char * p2 = skip_nonspaces(p);

}
