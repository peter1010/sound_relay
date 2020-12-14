#include "session.h"
#include "rtp_client.h"

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
//    pRtp = new RtpClient();
}
