import ctypes.util
import socket
import struct

class sockaddr(ctypes.Structure):
	_fields_ = [
		('family', ctypes.c_uint16),
		('data', ctypes.c_uint8 * 14)]

class sockaddr_ipv6(ctypes.Structure):
	_fields_ = [
		('family', ctypes.c_uint16),
		('data', ctypes.c_uint8 * 26)]


class ifaddrs(ctypes.Structure):
	pass

ifaddrs._fields_ = [
		('next', ctypes.POINTER(ifaddrs)),
		('name', ctypes.c_char_p),
		('flags', ctypes.c_uint), 
		('addr', ctypes.POINTER(sockaddr)),
		('netmask', ctypes.POINTER(sockaddr))]

IPv4 = 4
IPv6 = 6

def extract_address(sockaddr_p):
	if sockaddr_p:
		if sockaddr_p[0].family == socket.AF_INET:
			data = bytes(bytearray(sockaddr_p[0].data))
			port, addr, pad = struct.unpack("!H4s8s", data)
			addr = socket.inet_ntop(socket.AF_INET, addr)
			return(addr, port)
		if sockaddr_p[0].family == socket.AF_INET6:
			sockaddr_p = ctypes.cast(sockaddr_p, ctypes.POINTER(sockaddr_ipv6))
			data = bytes(bytearray(sockaddr_p[0].data))
			port = struct.unpack("!H", data[:2])
			flow, addr, scope = struct.unpack("=I16sI", data[2:])
			addr = socket.inet_ntop(socket.AF_INET6, addr)
			return (addr, port, flow, scope)
	return None


def get_hostip(sock, ver):
	local_address = sock.getsockname()
	if (ver == IPv4) and (len(local_address) == 2):
		return local_address[0]

	if (ver == IPv6) and (len(local_address) == 4):
		return local_address[0] + "%" + socket.if_indextoname(local_address[3])

	libc = ctypes.CDLL(ctypes.util.find_library("c"))
	ifaddr = ctypes.POINTER(ifaddrs)()
	retval = libc.getifaddrs(ctypes.byref(ifaddr))
	if retval != 0:
		raise RuntimeError()

	ifaddr_saved = ifaddr

	# Find the interface name for the local address
	while ifaddr:
		addr = extract_address(ifaddr[0].addr)
		if addr and (len(addr) == len(local_address)):
			if addr[0] == local_address[0]:
				name = ifaddr[0].name.decode(encoding='UTF-8')
				break
		ifaddr = ifaddr[0].next

	ifaddr = ifaddr_saved

	while ifaddr:
		_name = ifaddr[0].name.decode(encoding='UTF-8')
		if name == _name:
			addr = extract_address(ifaddr[0].addr)
			if addr and (len(addr) != len(local_address)):
				host = addr[0]
				if len(addr) == 4: # IPv6
					host += "%" + socket.if_indextoname(addr[3])
				break
		ifaddr = ifaddr[0].next

	libc.freeifaddrs(ifaddr_saved)
	return host


if __name__ == "__main__":
	sock = socket.socket(socket.AF_INET6, socket.SOCK_STREAM, 0)
	sock.bind(("fe80::ba27:ebff:fe7f:e822", 8000, 0, 2))
	print(get_hostip(sock, IPv4))
	print(get_hostip(sock, IPv6))
	sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM, 0)
	sock.bind(("192.168.11.51", 8000))
	print(get_hostip(sock, IPv4))
	print(get_hostip(sock, IPv6))



