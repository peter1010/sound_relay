#include <string.h>
#include <string>
#include <algorithm>

#include "logging.h"
#include "rtsp_connection.h"

/*----------------------------------------------------------------------------*/
void RtspConnection::parse_line(const std::string & str)
{
    LOG_DEBUG("Line = %s", str.c_str());	

    switch(mState) {
	case PARSING_REQUEST_LINE:
	    parse_request_line(str);
	    break;
	case PARSING_ENTITY_HEADERS:
	    parse_entity_header(str);
	    break;
    }
}


std::string trim(const std::string str)
{
    std::size_t begin = str.find_first_not_of(" \t");
    std::size_t end = str.find_last_not_of(" \t");
    if(begin == std::string::npos) {
	return "";
    }
    return str.substr(begin, end - begin +1);
}


char ascii2lower(char in) 
{
    if ((in <= 'Z') && (in >= 'A')) {
	return in - 'Z' + 'z';
    }
    return in;
}

/*----------------------------------------------------------------------------*/
void RtspConnection::parse_request_line(const std::string & str)
{
    static const struct {
        const char * str;
	T_REQUEST_TYPE typ;
    } lookup[] = {
        {"OPTIONS", OPTIONS},
    };

    std::size_t urlPos = 0;

    for(unsigned i = 0; i < sizeof(lookup)/sizeof(lookup[0]); i++) {
    	std::size_t idx = str.find(lookup[i].str);
    	if(idx == 0) {
            mRequestType = lookup[i].typ;
	    LOG_DEBUG("Request type is %i", mRequestType);
	    urlPos = strlen(lookup[i].str);
            break;
	}
    }
    if(urlPos > 0) {
        std::size_t urlEnd = str.rfind("RTSP/");
	if(urlEnd != std::string::npos) {
	    mUrl = trim(str.substr(urlPos, urlEnd - urlPos));
	    LOG_DEBUG("URL is \"%s\"", mUrl.c_str());
	    LOG_DEBUG("RTSP Version is \"%s\"", std::string(str, urlEnd).c_str());
	    mState = PARSING_ENTITY_HEADERS;
	}
    }
}


/*----------------------------------------------------------------------------*/
void RtspConnection::parse_entity_header(const std::string & str)
{
    // Connection
    // Content-Encoding
    // Content-Length
    // Content-Type
    // CSeq
    // Proxy-Require
    // Require
    // Session
    // Transport
    std::size_t colon = str.find(":");
    if(colon != std::string::npos) {
	std::string name(str, 0, colon);
	std::transform(name.begin(), name.end(), name.begin(), ascii2lower);
	std::string value(trim(str.substr(colon+1)));
	LOG_DEBUG("%s => %s", name.c_str(), value.c_str());
    }
}
