#ifndef _NETWORK_H_
#define _NETWORK_H_

class Connection;
class IpAddress;

typedef Connection * (* ConnectionFactory)(void * pArg);

/*----------------------------------------------------------------------------*/
class Network
{
public:
    Network(unsigned);
    virtual ~Network() = 0;

    virtual unsigned get_max_recv_len() const = 0;

    // Called from destructor on the Connection
    void detach_connection(int, Connection *);

    // Should be called from sub-class to register a connection object creation
    // function.
    void register_connection_factory(ConnectionFactory, void *);

    const char * get_hostname() const;
    virtual const IpAddress & get_hostip() const = 0;

protected:

//    Connection * get_connection() const { return mpConn; };
    void delete_connections();
    Connection * create_connection();

private:
    
    unsigned mMaxNumOfConns;
    Connection ** mpConns;

    ConnectionFactory mpConnectionFactory;
    void * mpFactoryArg;

    Network(const Network &);
    Network & operator=(const Network &);
};

#endif
