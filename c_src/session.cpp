#include "session.h"
#include "rtp_client.h"
#include "rtcp_server.h"
#include "capture.h"
#include "logging.h"


/*----------------------------------------------------------------------------*/
Session::Session() : mpSource(0), mpRtp(0), mpRtcp(0)
{
    LOG_DEBUG("Session");
}


/*----------------------------------------------------------------------------*/
Session::~Session()
{
    LOG_DEBUG("~Session");
    disconnect();
}


/*----------------------------------------------------------------------------*/
void Session::set_our_address(const char * addr)
{
    mOurAddress = addr;;
}


/*----------------------------------------------------------------------------*/
void Session::set_our_rtp_port(const char * port) 
{
    mOurRtpPort = atoi(port);
}


/*----------------------------------------------------------------------------*/
void Session::set_our_rtcp_port(const char * port) 
{
    mOurRtcpPort = atoi(port);
}


/*----------------------------------------------------------------------------*/
void Session::set_peer_address(const char * addr)
{
    mPeerAddress = addr;;
}


/*----------------------------------------------------------------------------*/
void Session::set_peer_rtp_port(const char * port)
{
    mPeerRtpPort = atoi(port);
}


/*----------------------------------------------------------------------------*/
void Session::set_peer_rtcp_port(const char * port)
{
    mPeerRtcpPort = atoi(port);
}



/*----------------------------------------------------------------------------*/
void Session::play()
{
    mpSource = new Capture();
    mpRtp = new RtpClient(*this);
    mpRtcp = new RtcpServer(*this);
    
    mpSource->init();    
}


/*----------------------------------------------------------------------------*/
void Session::disconnect()
{
    LOG_DEBUG("Disconnect");

    if(mpSource) {
	delete mpSource;
	mpSource = 0;
    }
    if(mpRtp) {
	delete mpRtp;
	mpRtp = 0;
    }
    if(mpRtcp) {
	delete mpRtcp;
	mpRtcp = 0;
    }
}
