#include "session.h"
#include "rtp_client.h"
#include "capture.h"

/*----------------------------------------------------------------------------*/
Session::Session()
{
//    mSession = "123456";


    mOurRtpPort = 49176;
    mOurRtcpPort = mOurRtpPort + 1;;

}


/*----------------------------------------------------------------------------*/
void Session::play()
{
    mpSource = new Capture();
    mpSource->init();    
    mpRtp = new RtpClient(*this);
}