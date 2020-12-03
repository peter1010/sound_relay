#include <string.h>

#include "rtsp_server.h"
#include "logging.h"
#include "tcp_connection.h"
#include "rtsp_connection.h"

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
    const char * pBuf = reinterpret_cast<const char *>(rConn.getRecvBuf());
    unsigned len = rConn.getRecvBufLen();

    if((len >= 4) && (strcmp(&pBuf[len-4], "\r\n\r\n") == 0)) {
//	LOG_INFO("%s\n", pBuf);
	parse_request(rConn);
    }	
    return true;
}


/*----------------------------------------------------------------------------*/
void RtspServer::parse_request(TcpConnection & rConn)
{	
    const char * p = reinterpret_cast<const char *>(rConn.getRecvBuf());
    unsigned left = rConn.getRecvBufLen();
    RtspConnection * pData = dynamic_cast<RtspConnection*>(rConn.getAppData());

    if(!pData) {
       LOG_DEBUG("Creating a RTSP Connection");
       pData = new RtspConnection;
       rConn.attachAppData(pData);
    }

    // Split up into lines to parse
    while(left > 0) {
	while(((*p == '\r') || (*p == '\n')) && (left > 0)) {
	    ++p;
	    left--;
	}
	const char * q = p;
	while((*q != '\r') && (*q != '\n') && (left > 0)) {
	    ++q;
	    left--;
	}
	const unsigned lineLen = q - p;
	if(lineLen > 0) {
	    std::string str(p, lineLen);
	    pData->parse_line(str);
	}
	p = q;
    }	  
}

