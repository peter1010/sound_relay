#include <sys/socket.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#include "udp_client.h"
#include "logging.h"
#include "connection.h"


/*----------------------------------------------------------------------------*/
UdpClient::UdpClient() : Network(1)
{
    LOG_DEBUG("UdpClient");
}


/*----------------------------------------------------------------------------*/
UdpClient::~UdpClient()
{
    LOG_DEBUG("~UdpClient");
}



/*----------------------------------------------------------------------------*/
bool UdpClient::init(unsigned short port, const IpAddress & address, 
		unsigned short localPort,
	      	const IpAddress & localAddress) 
{
    struct sockaddr_in addr;
    int status;

    // Start by opening a connection to the server
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if(sock < 0) {
        LOG_ERRNO_AS_ERROR("Failed to open UDP socket");
	return false;
    }

    if(localPort > 0) {
	memset(&addr, 0, sizeof(addr));

	addr.sin_family = AF_INET;
    	addr.sin_port = htons(localPort);
    	addr.sin_addr.s_addr = htonl(localAddress);

        status = bind(sock, reinterpret_cast<struct sockaddr *>(&addr), 
	    sizeof(addr));
        if(status != 0) {
       	    LOG_ERRNO_AS_ERROR("Bind to %hu failed", localPort);
	    close(sock);
	    return false;
        }
    }
    
    memset(&addr, 0, sizeof(addr));

    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = htonl(address);

    status = connect(sock, reinterpret_cast<struct sockaddr *>(&addr), 
	    sizeof(addr));
    if(status != 0) {
     	LOG_ERRNO_AS_ERROR("connect to %hu failed", port);
        close(sock);
        return false;
    }

    Connection * pConn = create_connection();
    if(pConn) {
	if(!pConn->attach(sock, *this, ntohl(addr.sin_addr.s_addr),
		ntohs(addr.sin_port))) {
            LOG_ERROR("Failed to attach connection");
	    delete pConn;
        }
    } else {
	close(sock);
    }	
    return true;
}

