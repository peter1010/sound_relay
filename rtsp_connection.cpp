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
	case PARSING_OPTIONS_REQUEST:
	    parse_options_request(str);
	    break;
	case PARSING_DESCRIBE_REQUEST:
	    parse_describe_request(str);
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
    }
    mState = PARSING_REQUEST_LINE;
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
	"Public: DESCRIBE, SETUP, TEARDOWN, PLAY\r\n\r\n";
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
        }
    }	
}


/*----------------------------------------------------------------------------*/
std::string RtspConnection::generate_describe_response()
{
    // v= (protocol version)
    // o = (origintator and session identifier)
    // s = (session name)
    // t = (time the session is active)
    // m = (media name and transport address)
    std::string sdp = "v=0\r\no=- SESS-ID SESS-VER IN IP4 10.0.0.1\r\n" \
	    "s=TV\r\nt=0 0\r\nm=audio 49170 RTP/AVP 31\r\n";

    return std::string("RTSP/1.0 200 OK\r\nCSeq: ") + mCseq + "\r\n" \
	"Content-Type: application/sdp\r\n" \
	"Content-Length: " + std::to_string(sdp.size()) + "\r\n\r\n" + sdp;
#if 0
    static const char templat[] = "RTSP/1.0 200 OK\r\nCSeq: %s\r\n"
	"Public: DESCRIBE, SETUP, TEARDOWN, PLAY\r\n\r\n";
    unsigned len = mCseq.size() + strlen(templat);
    unsigned char * p = new unsigned char[len];
    sprintf((char *)p, templat, mCseq.c_str());
    LOG_DEBUG("%s", p);
    return p;
#endif
}
