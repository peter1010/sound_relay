import socket
import select

try:
	from . import rtsp_connection
except ImportError:
	import rtsp_connection

class RtspServer(object):

	def __init__(self, sockV4 = None, sockV6 = None):
		self.connections = []
		if sockV4 is None and sockV6 is None:
			try:
				print("Trying port 554")
				self.init_socket(554);
			except PermissionError:
				print("Trying port 8554")
				self.init_socket(8554);
		else:
			self.sockV4 = sockV4
			self.sockV6 = sockV6
			if sockV4:
				self.listening_portV4 = sockV4.getsockname()[1]
			if sockV6:
				self.listening_portV6 = sockV6.getsockname()[1]
		self.wait_and_process()

	def __del__(self):
		for conn in self.connections:
			conn.close()
		self.connections = []

	def init_socket(self, port):
		self.create_ipv4_socket(port, '')
		self.create_ipv6_socket(port, '', 0)

	def create_ipv4_socket(self, port, address):
		sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM, 0)
		sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
		self.listening_portV4 = port
		sock.bind((address, port))
		sock.listen(1)
		self.sockV4 = sock

	def create_ipv6_socket(self, port, address, scope_id):
		sock = socket.socket(socket.AF_INET6, socket.SOCK_STREAM, 0)
		sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
		sock.setsockopt(socket.IPPROTO_IPV6, socket.IPV6_V6ONLY, 1)
		self.listening_portV6 = port
		sock.bind((address, port, 0, scope_id))
		sock.listen(1)
		self.sockV6 = sock


	def wait_and_process(self):
		poller = select.poll()
		if self.sockV4 is not None:
			poller.register(self.sockV4, select.POLLIN)
		if self.sockV6 is not None:
			poller.register(self.sockV6, select.POLLIN)

		while True:
			conn_mask = select.POLLIN
			events = poller.poll()
			for event, mask in events:
				if self.sockV4 and event == self.sockV4.fileno():
					poller.register(self.create_connection(self.sockV4, self.listening_portV4), conn_mask)
				elif self.sockV6 and event == self.sockV6.fileno():
					poller.register(self.create_connection(self.sockV6, self.listening_portV6), conn_mask)
				else:
					found_idx = None
					for idx, conn in enumerate(self.connections):
						if event == conn.fileno():
							found_idx = idx
							break

					if found_idx is not None:
						status = conn.read()
						if status is False:
							poller.unregister(conn)
							del self.connections[found_idx]
							conn.close()
							del conn


	def create_connection(self, sock, listening_port):
		connfd, address = sock.accept()
		if len(address) == 2:
			address, port = address
			scope_id = None
		else:
			address, port, flow_id, scope_id = address
			if scope_id > 0:
				print("DEBUG:", address, port, flow_id, scope_id)
				try:
					address += "%" + socket.if_indextoname(scope_id)
				except OSError:
					pass

		print("incomming connection request from ", address)
		new_conn = rtsp_connection.RtspConnection(connfd, address, listening_port)
		self.connections.append(new_conn)
		return new_conn

if __name__ == "__main__":
	rtsp = RtspServer();

