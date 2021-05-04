#include <stdio.h>
#include <unistd.h>

#include "event_loop.h"
#include "logging.h"
#include "session.h"

/******************************************************************************/
void usage()
{
	fprintf(stderr, "Usage: \n"
		"a peer address\n"
		"b peer rtcp port\n"
		"c peer rtp port\n"
		"x our address\n"
		"y our rtcp port\n"
		"z our rtp port\n");
}

/******************************************************************************/
int main(int argc, char * const argv[])
{
	set_logging_level(5);
	Session * p = new Session();

	int opt;
	while ((opt = getopt(argc, argv, "a:b:c:x:y:z:h")) != -1) {
		switch(opt) {
			case 'a':
				p->set_peer_address(optarg);
				break;
			case 'b':
				p->set_peer_rtcp_port(optarg);
				break;
			case 'c':
				p->set_peer_rtp_port(optarg);
				break;
			case 'x':
				p->set_our_address(optarg);
				break;
			case 'y':
				p->set_our_rtcp_port(optarg);
				break;
			case 'z':
				p->set_our_rtp_port(optarg);
				break;
			case 'h':
			default:
				usage();
				return 0;
		}
	}

	p->play();
	EventLoop::instance().main();

	delete p;
}
