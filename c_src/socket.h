#ifndef _SOCKET_H_
#define _SOCKET_H_

//#include "ip_address.h"

class IpAddress;
class Socket;
struct in6_addr;

/******************************************************************************/
class SocketException 
{
public:
	SocketException(Socket * , bool, const char * fmt, ...) __attribute__((format (printf, 4, 5)));
	~SocketException();
private:
	Socket * mpSock;
};

struct SocketDetails {
	int mSockV4;
	int mSockV6;
	int mType;
};

/*----------------------------------------------------------------------------*/
class Socket
{
public:
	Socket(int);  // SOCK_STREAM or SOCK_DATAGRAM
	~Socket();

	void bind(const IpAddress &, uint16_t port);

	void listen(int);

	void accept(IpAddress * peer, uint16_t * port);

	void connect(const IpAddress &, uint16_t port);

	int recv(uint8_t * pBuf, int len);

	int send(const uint8_t * pData, unsigned length);

	int fileNo1() const { return mSockV4 >= 0 ? mSockV4 : mSockV6; };
	int fileNo2() const { return mSockV6 >= 0 ? mSockV6 : mSockV4; };

protected:

	enum FollowUp_t {
		BIND,
		CONNECT
	};

	void ipv4_socket(uint16_t, uint32_t, FollowUp_t);
	void ipv6_socket(uint16_t, const struct in6_addr &, unsigned, FollowUp_t);

	void bind(int sock, uint16_t port, const sockaddr * pAddr, socklen_t len);
	void connect(int sock, uint16_t port, const sockaddr * pAddr, socklen_t len);

	void setsockopt_ipv6only(int sock);

private:
	int mSockV4;
	int mSockV6;
	int mType;
	
	Socket(const Socket &);
	Socket & operator=(const Socket &);
};


#endif
