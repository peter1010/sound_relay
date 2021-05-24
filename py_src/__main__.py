import sys

from . import rtsp_server
from . import sd_listener

def main():
	fds = sd_listener.sd_listen_fds(False)
	if len(fds) > 1:
		print("Only one FD please")
		sys.exit(1)
	print("starting..")
	if len(fds) == 1:
		sock = socket.fromfd(fds[0], socket.AF_INET, socket.SOCK_STREAM)
	else:
		sock = None
	rtsp = rtsp_server.RtspServer(sock);


if __name__ == "__main__":
	main()
