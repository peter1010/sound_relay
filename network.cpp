
#include "network.h"
#include "logging.h"
#include "event_loop.h"
#include "connection.h"

/*----------------------------------------------------------------------------*/
Network::Network(EventLoop & rEventLoop) 
	: mEventLoop(rEventLoop), mpConn(0), mpConnectionFactory(0) 
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
    mEventLoop.unregister(sock);
    mpConn = 0;
}

