import socket
import select

import rtsp_connection

class RtspServer(object):

	def __init__(self):
		self.connections = []
		try:
			self.init_socket(554);
		except PermissionError:
			self.init_socket(8554);
		self.wait_and_process()


	def init_socket(self, port):
		self.sock = self.create_ipv4_socket(port, '')
		self.alt_sock = self.create_ipv6_socket(port, '', 0)
		self.listening_port = port

	def create_ipv4_socket(self, port, address):
		sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM, 0)
		sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
		sock.bind((address, port))
		sock.listen(1)
		return sock

	def create_ipv6_socket(self, port, address, scope_id):
		sock = socket.socket(socket.AF_INET6, socket.SOCK_STREAM, 0)
		sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
		sock.setsockopt(socket.IPPROTO_IPV6, socket.IPV6_V6ONLY, 1)
		sock.bind((address, port, 0, scope_id))
		sock.listen(1)
		return sock


	def wait_and_process(self):
		poller = select.poll()
		poller.register(self.sock, select.POLLIN)
		poller.register(self.alt_sock, select.POLLIN)

		while True:
			events = poller.poll()
			for event, mask in events:
				if event == self.sock.fileno():
					poller.register(self.create_connection(self.sock), select.POLLIN)
				elif event == self.alt_sock.fileno():
					poller.register(self.create_connection(self.alt_sock), select.POLLIN)
				else:
					found_idx = None
					for idx, conn in enumerate(self.connections):
						if event == conn.fileno():
							found_idx = idx
							conn_awake = conn
							break

					if found_idx is not None:
						status = conn_awake.read()
						if status is False:
							poller.unregister(conn_awake)
							del self.connections[found_idx]
							del conn_awake


	def create_connection(self, sock):
		connfd, address = sock.accept()
		if len(address) == 2:
			address, port = address
			scope_id = None
		else:
			address, port, flow_id, scope_id = address
			if scope_id > 0:
				address += "%" + socket.if_indextoname(scope_id)

		print("incomming connection request from ", address)
		new_conn = rtsp_connection.RtspConnection(self, connfd, address)
		self.connections.append(new_conn)
		return new_conn

	def get_listening_port(self):
		return self.listening_port

if __name__ == "__main__":
	rtsp = RtspServer();

