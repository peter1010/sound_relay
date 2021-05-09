#include "session.h"
#include "rtp_client.h"
#include "rtp_server.h"
#include "rtcp_client.h"
#include "rtcp_server.h"
#include "capture.h"
#include "replay.h"
#include "logging.h"


/*----------------------------------------------------------------------------*/
Session::Session() : mpSound(0), mpRtp(0), mpRtcp(0)
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


Capture * Session::get_source() const
{
	return dynamic_cast<Capture *>(mpSound);
};

Replay * Session::get_sink() const
{
	return dynamic_cast<Replay *>(mpSound);
};

/*----------------------------------------------------------------------------*/
void Session::capture()
{
	Capture * capture = new Capture();
	mpSound = capture;

	mpRtp = new RtpClient(*this);
	mpRtcp = new RtcpServer(*this);

	capture->init();

}

/*----------------------------------------------------------------------------*/
void Session::play()
{
	Replay * replay = new Replay();
	mpSound = replay;

	LOG_DEBUG("1");
	mpRtp = new RtpServer(*this);
	LOG_DEBUG("2");
	mpRtcp = new RtcpClient(*this);

	LOG_DEBUG("3");
	replay->init();
	LOG_DEBUG("4");

}



/*----------------------------------------------------------------------------*/
void Session::disconnect()
{
	LOG_DEBUG("Disconnect");

	if(mpSound) {
		delete mpSound;
		mpSound = 0;
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
