#include <unistd.h>
#include <string.h>

#include "network.h"
#include "logging.h"
#include "event_loop.h"
#include "connection.h"

/*----------------------------------------------------------------------------*/
Network::Network(): mpConn(0), mpConnectionFactory(0) 
{
    LOG_DEBUG("Network");
}


/*----------------------------------------------------------------------------*/
Network::~Network()
{
    LOG_DEBUG("~Network");
    delete_connection();
}


/*----------------------------------------------------------------------------*/
void Network::delete_connection()
{
    if(mpConn) {
        delete mpConn;
	mpConn = 0;
    }
}


/*----------------------------------------------------------------------------*/
bool Network::create_connection() 
{
    if(!mpConnectionFactory) {	
        LOG_ERROR("No registered Connection factory");
	return false;
    }
    if(mpConn) {
        LOG_ERROR("Too many active connections");
	return false;
    }

    mpConn = mpConnectionFactory();
    return true;
}


/*----------------------------------------------------------------------------*/
void Network::register_connection_factory(ConnectionFactory pFunc)
{
    mpConnectionFactory = pFunc;;
}


/*----------------------------------------------------------------------------*/
void Network::detach_connection(int sock, Connection * pConn)
{
    LOG_INFO("remove_connection");
    EventLoop::instance().unregister(sock);
    mpConn = 0;
}


/*----------------------------------------------------------------------------*/
const char * Network::get_hostname() const
{
    static char buf[64];
    static bool got_name = false;

    if(!got_name) {
	if(0 == gethostname(buf, sizeof(buf))) {
	    const unsigned len = strlen(buf);
	    buf[len] = '.';
	    if(0 != getdomainname(&buf[len+1], sizeof(buf) - len - 1)) {
		buf[len] = '\0';
	    }
	    got_name = true;	
	} else {
	    LOG_ERRNO_AS_ERROR("Failed to get domain name");
	    buf[0] = '\0';
        }
    }
    return buf;
}    

