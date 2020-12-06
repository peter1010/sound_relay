#ifndef _NETWORK_H_
#define _NETWORK_H_

class EventLoop;
class Connection;

typedef Connection * (* ConnectionFactory)();

/*----------------------------------------------------------------------------*/
class Network
{
public:
    Network(EventLoop & rEventLoop);

    virtual ~Network() = 0;

    virtual unsigned get_max_recv_len() const = 0;

    // Called from destructor on the Connection
    void detach_connection(int sock, Connection * pConn);

    // Should be called from sub-class to register a connection object creation
    // function.
    void register_connection_factory(ConnectionFactory pFunc);

protected:

    EventLoop & get_event_loop() const { return mEventLoop; };

    Connection * get_connection() const { return mpConn; };
    void delete_connection();
    bool create_connection();


private:
    
    EventLoop & mEventLoop;
    Connection * mpConn;
    ConnectionFactory mpConnectionFactory;

    Network(const Network &);
    Network & operator=(const Network &);
};

#endif
