import time

try:
	from . import session
except ImportError:
	import session

sdp_id = int(time.time())

tv_session = session.Session(sdp_id)

def get_base_session(url):
	# TODO base session has no ID or ports defined and should be const
	sess = tv_session
	return sess

def create_session(url):
	# TODO create a session with unique rtp_id
	sess = tv_session
	return sess


def get_session(url, session_id):
	# TODO return a created session
	sess = tv_session

	if sess.get_rtp_id() != session_id:
		return None
	return sess

