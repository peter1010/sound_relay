import sys
import socket

from . import rtsp_server
from . import sd_listener

def main():
	fds = sd_listener.sd_listen_fds(False)
	if len(fds) > 2:
		print("Max 2 FD please")
		sys.exit(1)
	print("starting..")
		
	if len(fds) == 2:
		sockV6 = socket.socket(fileno = fds[0])
		sockV4 = socket.socket(fileno = fds[1])
		if sockV6.family == socket.AF_INET:
			sockV6, sockV4 = sockV4, sockV6
	elif len(fds) == 1:
		sockV6 = socket.socket(fileno = fds[0])
		sockV4 = None
		if sockV6.family == socket.AF_INET:
			sockV6, sockV4 = sockV4, sockV6
	else:
		sockV4 = None
		sockV6 = None
	rtsp = rtsp_server.RtspServer(sockV4, sockV6);


if __name__ == "__main__":
	main()
