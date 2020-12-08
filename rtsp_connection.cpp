#include <string.h>
#include <time.h>

#include <string>
#include <algorithm>

#include "logging.h"
#include "rtsp_connection.h"


/*----------------------------------------------------------------------------*/
RtspConnection::RtspConnection() :  mState(PARSING_REQUEST_LINE)
{
    mSession = "123456";

    mServerRtpPort = 49176;
    mServerRtcpPort = mServerRtpPort + 1;
}


/*----------------------------------------------------------------------------*/
int RtspConnection::parse_recv(const Byte * pData, unsigned len)
{
    int retVal = 0;

    const char * p = reinterpret_cast<const char *>(pData);
    const char * end = reinterpret_cast<const char *>(memmem(reinterpret_cast<const void *>(p), len, "\r\n\r\n", 4));

    if(end) {
	const unsigned size = (end - p);
    	unsigned left = size;
        
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
	retVal = size + 4;
    } else {
        LOG_DEBUG("%i %i %i %i", pData[len-4], pData[len-3], pData[len-2], pData[len-1]);
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
	case PARSING_PLAY_REQUEST:
	    parse_play_request(str);
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
	case PARSING_PLAY_REQUEST:
	    retVal = generate_play_response();
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

//	LOG_DEBUG("Request = %s", request.c_str());
//	LOG_DEBUG("URL = %s", url.c_str());
//	LOG_DEBUG("Version = %s", version.c_str());

	if(request == "OPTIONS") {
	    mState = PARSING_OPTIONS_REQUEST;
	} else if(request == "DESCRIBE") {
	    mState = PARSING_DESCRIBE_REQUEST;
	} else if(request == "SETUP") {
	    mState = PARSING_SETUP_REQUEST;
	} else if(request == "PLAY") {
	    mState = PARSING_PLAY_REQUEST;
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
    return std::string("RTSP/1.0 200 OK\r\n" \
	"CSeq: ") + mCseq + "\r\n" \
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
	    "o=- " + std::to_string(get_session_id()) + " " 
	    	+ std::to_string(get_session_ver()) + " IN IP4 " + get_hostip() 
		+ "\r\n" \
	    "s=TV Session\r\n" \
	    "i=Sound from the TV\r\n" \
	    "t= 0 0\r\n" \
	    "a=recvonly\r\n" \
	    "a=control:rtsp://" + get_hostname() + ":" 
	    	+ std::to_string(get_rtsp_server_port()) + "/" + get_pathname() 
		+ "\r\n" \
	    "m=audio " + std::to_string(get_our_rtp_port()) +" RTP/AVP 31\r\n" \
	    "a=rtpmap:31 OPUS/48000/2\r\n";

    return std::string("RTSP/1.0 200 OK\r\n"\
	    "CSeq: ") + mCseq + "\r\n" \
	    "Content-Type: application/sdp\r\n" \
	    "Content-Length: " + std::to_string(sdp.size()) + "\r\n\r\n" 
	    + sdp;
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
		const std::size_t idx2 = value.find(";", idx+1);
		const std::size_t idx3 = value.find("=", idx+1);

		std::string subName;
		std::string subValue;
		if(idx2 != std::string::npos) {
		    if((idx3 != std::string::npos) && (idx3 < idx2)) {
 		        subName = value.substr(idx+1, idx3-idx-1);
			subValue = value.substr(idx3+1, idx2-idx3-1);
		    } else {
			subName = value.substr(idx+1, idx2-idx-1);
		    }
		} else if(idx3 != std::string::npos) {
 		    subName = value.substr(idx+1, idx3-idx-1);
		    subValue = value.substr(idx3+1);
		} else {
		    subName = value.substr(idx+1);
		}
		idx = idx2;
		LOG_DEBUG("%s=%s", subName.c_str(), subValue.c_str());
		if(subName == "client_port") {
		    const std::size_t idx4 = subValue.find("-");
		    if(idx4 != std::string::npos) {
			mClientRtpPort = std::stoi(subValue.substr(0, idx4));
			mClientRtcpPort = std::stoi(subValue.substr(idx4+1));
		    } else {
			mClientRtpPort = std::stoi(subValue);
			mClientRtcpPort = mClientRtpPort + 1;
		    }
		}
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
    std::string transport = "RTP/AVP/UDP;unicast;" \
	"client_port=" + std::to_string(mClientRtpPort) + "-"
       	+ std::to_string(mClientRtcpPort) + ";"\
	"server_port=" + std::to_string(mServerRtpPort) + "-"
	+ std::to_string(mServerRtcpPort);

    return std::string("RTSP/1.0 200 OK\r\n" \
        "CSeq: ") + mCseq + "\r\n" \
	"Session: " + mSession + "\r\n" \
	"Transport: " + transport + "\r\n\r\n";
}


/*----------------------------------------------------------------------------*/
void RtspConnection::parse_play_request(const std::string & str)
{
    std::string name;
    std::string value;

    if(split_name_value_pair(str, name, value)) {
	// CSeq
	if(name == "cseq") {
	    mCseq = value;
        } else if(name == "range") {
	}
    }	
}


/*----------------------------------------------------------------------------*/
std::string RtspConnection::generate_play_response()
{
    // Mandatory fields:
    // CSeq:
    
    return std::string("RTSP/1.0 200 OK\r\n" \
        "CSeq: ") + mCseq + "\r\n\r\n";
}



