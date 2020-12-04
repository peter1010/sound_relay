#include <string.h>
#include <string>
#include <algorithm>

#include "logging.h"
#include "rtsp_connection.h"


/*----------------------------------------------------------------------------*/
RtspConnection::RtspConnection(int sock, TcpServer & parent) : TcpConnection(sock, parent),
       	mState(PARSING_REQUEST_LINE), mRtpPort(49176)
{
    mSession = "123456";
}


/*----------------------------------------------------------------------------*/
RtspConnection::T_RECV_STATE RtspConnection::parse_recv(const Byte * pData, unsigned len)
{
    T_RECV_STATE retVal = NEED_MORE;

    const char * p = reinterpret_cast<const char *>(pData);

    if((len >= 4) && (strcmp(&p[len-4], "\r\n\r\n") == 0)) {
    	unsigned left = len;
        
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
	        parse_line(str);
	    }
	    p = q;
        }	  
//	LOG_INFO("%s\n", pData);
	generate_response();
	retVal = ALL_DONE;
    }	
    return retVal;
}


/*----------------------------------------------------------------------------*/
void RtspConnection::generate_response()
{
     std::string response = get_response();
     send(reinterpret_cast<const unsigned char *>(response.c_str()),
		    response.size()); 
}



/*----------------------------------------------------------------------------*/
void RtspConnection::parse_line(const std::string & str)
{
    LOG_DEBUG("Line = %s", str.c_str());	

    switch(mState) {
	case PARSING_REQUEST_LINE:
	    parse_request_line(str);
	    break;
	case PARSING_OPTIONS_REQUEST:
	    parse_options_request(str);
	    break;
	case PARSING_DESCRIBE_REQUEST:
	    parse_describe_request(str);
	    break;
	case PARSING_SETUP_REQUEST:
	    parse_setup_request(str);
	    break;
    }
}


/*----------------------------------------------------------------------------*/
std::string RtspConnection::get_response()
{
    std::string retVal;

    switch(mState) {
	case PARSING_REQUEST_LINE:
	    break;
	case PARSING_OPTIONS_REQUEST:
	    retVal = generate_options_response();
	    break;
	case PARSING_DESCRIBE_REQUEST:
	    retVal = generate_describe_response();
	    break;
	case PARSING_SETUP_REQUEST:
	    retVal = generate_setup_response();
	    break;
    }
    mState = PARSING_REQUEST_LINE;
    LOG_DEBUG("%s", retVal.c_str());
    return retVal;
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
    std::string str1 = trim(str);
    std::size_t idx1 = str1.find_first_of(" \t");
    std::size_t idx2 = str1.find_last_of(" \t");

    if((idx1 != std::string::npos) && (idx2 != std::string::npos)) {
	std::string request = str1.substr(0, idx1);
	std::string url = trim(str1.substr(idx1, idx2-idx1));
	std::string version = str1.substr(idx2+1);

	LOG_DEBUG("Request = %s", request.c_str());
	LOG_DEBUG("URL = %s", url.c_str());
	LOG_DEBUG("Version = %s", version.c_str());

	if(request == "OPTIONS") {
	    mState = PARSING_OPTIONS_REQUEST;
	} else if(request == "DESCRIBE") {
	    mState = PARSING_DESCRIBE_REQUEST;
	} else if(request == "SETUP") {
	    mState = PARSING_SETUP_REQUEST;
	} else {
	    LOG_ERROR("Unknown request %s", request.c_str());
	    // TODO
	}
    }
}


bool split_name_value_pair(const std::string & str, std::string & name, std::string & value)
{
    bool retVal = false;

    std::size_t colon = str.find(":");
    if(colon != std::string::npos) {
	name = std::string(str, 0, colon);
	std::transform(name.begin(), name.end(), name.begin(), ascii2lower);
	value = trim(str.substr(colon+1));
	LOG_DEBUG("%s => %s", name.c_str(), value.c_str());
	retVal = true;
    }
    return retVal;
}


/*----------------------------------------------------------------------------*/
void RtspConnection::parse_options_request(const std::string & str)
{
    std::string name;
    std::string value;

    if(split_name_value_pair(str, name, value)) {
	// CSeq
	if(name == "cseq") {
	    mCseq = value;
        }
    }	
}


/*----------------------------------------------------------------------------*/
std::string RtspConnection::generate_options_response()
{
    return std::string("RTSP/1.0 200 OK\r\nCSeq: ") + mCseq + "\r\n"
	"Public: DESCRIBE, SETUP, PLAY, TEARDOWN\r\n\r\n";
}

/*----------------------------------------------------------------------------*/
void RtspConnection::parse_describe_request(const std::string & str)
{
    std::string name;
    std::string value;

    if(split_name_value_pair(str, name, value)) {
	// CSeq
	if(name == "cseq") {
	    mCseq = value;
        } else if(name == "accept") {
	    // Check that application/sdp is an option
	    if(value.find("application/sdp") == std::string::npos) {
		// TODO
            }
	}
    }	
}


/*----------------------------------------------------------------------------*/
std::string RtspConnection::generate_describe_response()
{
    // Mandatory fields:
    // CSeq:
    // Content-type:
    // Content-Length;

    // SDP fields
    // v= (protocol version)
    // o = (origintator and session identifier)
    // s = (session name)
    // t = (time the session is active)
    // m = (media name and transport address)
    std::string sdp = "v=0\r\n" \
	    "o=- SESS-ID SESS-VER IN IP4 10.0.0.1\r\n" \
	    "s=TV\r\n" \
	    "t=0 0\r\n" \
	    "m=audio " + std::to_string(mRtpPort) +" RTP/AVP 31\r\n";

    return std::string("RTSP/1.0 200 OK\r\n"\
	    "CSeq: ") + mCseq + "\r\n" \
	    "Content-Type: application/sdp\r\n" \
	    "Content-Length: " + std::to_string(sdp.size()) + "\r\n\r\n" + sdp;
}


/*----------------------------------------------------------------------------*/
void RtspConnection::parse_setup_request(const std::string & str)
{
    std::string name;
    std::string value;

    if(split_name_value_pair(str, name, value)) {
	// CSeq
	if(name == "cseq") {
	    mCseq = value;
        } else if(name == "transport") {
  	    std::size_t idx = value.find(";");
	    std::string proto;

	    if(idx != std::string::npos) {
		proto = value.substr(0, idx);
	    } else {
		proto = value;
	    }
	    LOG_DEBUG("proto=%s", proto.c_str());
	
	    // Transport fields are separated with semi-colons.
	    while(idx != std::string::npos) {
		std::size_t idx2 = value.find(";", idx+1);
		std::string token;
		if(idx != std::string::npos) {
		    token = value.substr(idx+1, idx2-idx);
		} else {
		    token = value.substr(idx+1);
		}
		// Break up the token
		std::size_t idx3 = token.find("=", idx+1);
		std::string subName;
		std::string subValue;
		if(idx3 != std::string::npos) {
		    subName = value.substr(idx, idx3-idx);
		    subValue = token.substr(idx3+1, idx2-idx3-1);
		} else {
		    subName = value.substr(idx, idx2-idx);
		}
		idx = idx2;
		LOG_DEBUG("%s=%s", subName.c_str(), subValue.c_str());
            }
	}
    }	
}


/*----------------------------------------------------------------------------*/
std::string RtspConnection::generate_setup_response()
{
    // Mandatory fields:
    // CSeq:
    // Session:
    // Transport:
    std::string transport = "RTP/AVP;unicast;client_port=4688-4589;server_port=6356-6257";

    return std::string("RTSP/1.0 200 OK\r\n" \
        "CSeq: ") + mCseq + "\r\n" \
	"Session: " + mSession + "\r\n" \
	"Transport: " + transport + "\r\n\r\n";
}


