try:
	from . import session
except ImportError:
	import session

tv_session = session.Session()


def get_session(url):
    return tv_session
