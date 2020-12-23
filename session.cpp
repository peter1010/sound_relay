#include "session.h"
#include "rtp_client.h"
#include "capture.h"
#include "logging.h"


/*----------------------------------------------------------------------------*/
Session::Session() : mpSource(0), mpRtp(0)
{
    LOG_DEBUG("Session");

//    mSession = "123456";


    mOurRtpPort = 49176;
    mOurRtcpPort = mOurRtpPort + 1;;

}


/*----------------------------------------------------------------------------*/
Session::~Session()
{
    LOG_DEBUG("~Session");

    if(mpSource) {
	delete mpSource;
	mpSource = 0;
    }
    if(mpRtp) {
	delete mpRtp;
	mpRtp = 0;
    }
}


/*----------------------------------------------------------------------------*/
void Session::play()
{
    mpSource = new Capture();
    mpRtp = new RtpClient(*this);

    mpSource->init();    
}


