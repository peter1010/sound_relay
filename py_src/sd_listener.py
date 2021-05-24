import os
import fcntl

# See man sd_listen_fds
# See http://cgit.freedesktop.org/systemd/systemd/plain/src/libsystemd-daemon/sd-daemon.c

SD_LISTEN_FDS_START=3

def sd_listen_fds(unset_environment):
	"""Almost but not quite the same as the C version. The python version returns
    a list of fds as opposed to the number of fds"""
	try:
		listen_pid = int(os.environ["LISTEN_PID"])
	except (KeyError, ValueError):
		print("env[LISTEN_PID] doesn't exist")
		return ()
	our_pid = os.getpid()
	if our_pid != listen_pid:
		print("os.getpid() (%i) != listen_pid (%i)" % (our_pid, listen_pid))
		return ()
	try:
		num_fds = int(os.environ["LISTEN_FDS"])
	except (KeyError, ValueError):
		print("env[LISTEN_FDS] doesnt exist")
		return ()
#    print("LISTEN_FDS=%i" % num_fds)
	ret_vals = []
	for fd in range(SD_LISTEN_FDS_START, SD_LISTEN_FDS_START + num_fds):
		flags = fcntl.fcntl(fd, fcntl.F_GETFD)
		if not (flags & fcntl.FD_CLOEXEC):
			fcntl.fcntl(fd, fcntl.F_SETFD, flags | fcntl.FD_CLOEXEC)
		ret_vals.append(fd)
	if unset_environment:
		del os.environ["LISTEN_PID"]
		del os.environ["LISTEN_FDS"]
	return tuple(ret_vals)


def sd_is_fifo(fd, path):
	st_fd = os.fstat(fd)
	if not stat.S_ISFIFO(st_fd):
		return False
	if path:
		st_path = os.stat(path)
		return (st_path.st_dev == st_fd.st_dev) and (st_path.st_ino == st_fd.st_ino)
	return True


def sd_is_special(fd, path):
	st_fd = os.fstat(fd)
	if not stat.S_ISREG(st_fd) && not stat.S_ISCHR(st_fd):
		return False
	if path:
		st_path = os.stat(path)
		if stat.S_ISREG(st_path) && stat.S_ISREG(st_fd):
			return (st_path.st_dev == st_fd.st_dev) and (st_path.st_ino == st_fd.st_ino)
		elif stat.S_ISCHR(st_path) && stat.S_ISCHR(st_fd):
			return st_path.st_rdev == st_fd.st_rdev
		else:
			return False
	return True


def sd_is_socket(fd, family, typ, listening):
	pass

def sd_is_socket_inet():
	pass

def sd_is_socket_unix():
	pass

def sd_is_mq():
	pass

def sd_notify():
	pass

def sd_notifyf():
	pass

def sd_booted():
	pass
