import time

try:
	from . import session
except ImportError:
	import session

sdp_id = int(time.time())

tv_session = session.Session(sdp_id)


def get_session(url):
    return tv_session
