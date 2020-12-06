
#include "logging.h"
#include "rtp_connection.h"


/*----------------------------------------------------------------------------*/
RtpConnection::RtpConnection()
{
    LOG_DEBUG("RtpConnection");
}


/*----------------------------------------------------------------------------*/
bool RtpConnection::parse_recv(const Byte * pData, unsigned len)
{
    return true;
}


