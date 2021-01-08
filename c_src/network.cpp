#include <unistd.h>
#include <string.h>

#include "network.h"
#include "logging.h"
#include "event_loop.h"
#include "connection.h"

/*----------------------------------------------------------------------------*/
Network::Network(unsigned maxNumOfConns): mMaxNumOfConns(maxNumOfConns),
	mpConnectionFactory(0), mpFactoryArg(0)
{
    LOG_DEBUG("Network");
    mpConns = new Connection * [maxNumOfConns];
    for(unsigned i = 0; i < maxNumOfConns; i++) {
	mpConns[i] = 0;
    }
}


/*----------------------------------------------------------------------------*/
Network::~Network()
{
    LOG_DEBUG("~Network");
    delete_connections();
}


/*----------------------------------------------------------------------------*/
void Network::delete_connections()
{
    for(unsigned i = 0; i < mMaxNumOfConns; i++) {
	Connection * pConn = mpConns[i];
	if(pConn) {
            delete pConn;
	    mpConns[i] = 0;
	}
    }
}


/*----------------------------------------------------------------------------*/
Connection * Network::create_connection() 
{
    Connection * pConn = 0;

    if(!mpConnectionFactory) {	
        LOG_ERROR("No registered Connection factory");
	return 0;
    }
    for(unsigned i = 0; i < mMaxNumOfConns; i++) {
	if(mpConns[i] == 0) {
    	    pConn = mpConnectionFactory(mpFactoryArg);
    	    mpConns[i] = pConn;
	    break;
	}
    }
    if(!pConn) {
        LOG_ERROR("Too many active connections");
    }

    return pConn;
}


/*----------------------------------------------------------------------------*/
void Network::register_connection_factory(ConnectionFactory pFunc, void * pArg)
{
    mpConnectionFactory = pFunc;
    mpFactoryArg = pArg;
}


/*----------------------------------------------------------------------------*/
void Network::detach_connection(int sock, Connection * pConn)
{
    LOG_INFO("remove_connection");
    EventLoop::instance().unregister(sock);

    for(unsigned i = 0; i < mMaxNumOfConns; i++) {
	if(mpConns[i] == pConn) {
    	    mpConns[i] = 0;
	}
    }
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

