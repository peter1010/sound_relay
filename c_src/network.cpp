#include <unistd.h>
#include <string.h>
#include <sys/socket.h>

#include "network.h"
#include "logging.h"
#include "event_loop.h"
#include "connection.h"


/******************************************************************************/
Network::Network(unsigned maxNumOfConns): mMaxNumOfConns(maxNumOfConns),
	mpConnectionFactory(0), mpFactoryArg(0)
{
    LOG_DEBUG("Network");
    mpConns = new Connection * [maxNumOfConns];
    for(unsigned i = 0; i < maxNumOfConns; i++) {
	mpConns[i] = 0;
    }
}


/******************************************************************************/
Network::~Network()
{
    LOG_DEBUG("~Network");
    delete_connections();
}


/******************************************************************************/
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


/******************************************************************************/
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


/******************************************************************************/
void Network::register_connection_factory(ConnectionFactory pFunc, void * pArg)
{
    mpConnectionFactory = pFunc;
    mpFactoryArg = pArg;
}


/******************************************************************************/
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


/******************************************************************************/
int Network::bind(int sock, uint16_t port, const sockaddr * pAddr,
		socklen_t len)
{
    int status = ::bind(sock, pAddr, len);
    if(status != 0) {
    	LOG_ERRNO_AS_ERROR("Bind to %hu failed", port);
	::close(sock);
	return -1;
    }
    return sock;
}


/******************************************************************************/
void Network::setsockopt_ipv6only(int sock)
{
    int on = 1;
    ::setsockopt(sock, IPPROTO_IPV6, IPV6_V6ONLY, (void *)&on, sizeof(on));
}


/******************************************************************************/
void Network::setsockopt_reuseaddr(int sock)
{
    int on = 1;
    ::setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (void *)&on, sizeof(on));
}
