#include <string.h>
#include <string>

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
    const char * pBuf = reinterpret_cast<const char *>(rConn.getRecvBuf());
    unsigned len = rConn.getRecvBufLen();

    if((len >= 4) && (strcmp(&pBuf[len-4], "\r\n\r\n") == 0)) {
	LOG_INFO("%s\n", pBuf);
	parse_request(rConn);
    }	
    return true;
}


/*----------------------------------------------------------------------------*/
void RtspServer::parse_request(TcpConnection & rConn)
{	
    const char * p = reinterpret_cast<const char *>(rConn.getRecvBuf());
    unsigned left = rConn.getRecvBufLen();
    unsigned status = 200;
    RtspConnection * pData = dynamic_cast<RtspConnection*>(rConn.getAppData());

    if(!pData) {
       RtspConnection * pData = new RtspConnection;
       rConn.attachAppData(pData);
    }

    // Split up into lines to parse
    while((left > 0) && (status == 200)) {
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
    }	  
}


/*----------------------------------------------------------------------------*/
void RtspConnection::parse_line(std::string & str)
{
    switch(mState) {
	case PARSING_REQUEST_LINE:
	    parse_request_line(str);
	    break;
    }	    
}


/*----------------------------------------------------------------------------*/
void RtspConnection::parse_request_line(std::string & str)
{
    static const struct {
        const char * str;
	T_REQUEST_TYPE typ;
    } lookup[] = {
        {"OPTIONS ", OPTIONS},
    };

    std::size_t urlPos = 0;

    for(unsigned i = 0; i < sizeof(lookup)/sizeof(lookup[0]); i++) {
    	std::size_t idx = str.find(lookup[i].str);
    	if(idx == 0) {
            mRequestType = lookup[i].typ;
	    urlPos = strlen(lookup[i].str);
            break;
	}
    }
    if(urlPos > 0) {
        std::size_t urlEnd = str.rfind(" RTSP/");
	if(urlEnd != std::string::npos) {
	    mUrl.assign(str, urlPos, urlEnd - urlPos);
	    mState = PARSING_ENTITY_HEADERS;
	}
    }
}
