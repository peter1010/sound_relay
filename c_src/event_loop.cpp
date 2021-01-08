#include <stdlib.h>
#include <poll.h>
#include <string.h>
#include <stdio.h>

#include "event_loop.h"
#include "logging.h"

EventLoop * EventLoop::mInstance = 0;


/*----------------------------------------------------------------------------*/
void EventLoop::create() 
{
    mInstance = new EventLoop();
}


/*----------------------------------------------------------------------------*/
EventLoop::EventLoop()
{
    LOG_DEBUG("EventLoop");
    mPollListSize = 0;
}


/*----------------------------------------------------------------------------*/
bool EventLoop::register_read_callback(int fd, CallbackFunc pFunc, void * arg)
{
    return register_callback(fd, pFunc, arg, NULL, NULL, NULL, NULL);
}


/*----------------------------------------------------------------------------*/
bool EventLoop::register_write_callback(int fd, CallbackFunc pFunc, void * arg)
{
    return register_callback(fd, NULL, NULL, pFunc, arg, NULL, NULL);
}


/*----------------------------------------------------------------------------*/
bool EventLoop::register_error_callback(int fd, CallbackFunc pFunc, void * arg)
{
    return register_callback(fd, NULL, NULL, NULL, NULL, pFunc, arg);
}

/*----------------------------------------------------------------------------*/
bool EventLoop::register_callback(int fd, CallbackFunc pReadFunc, 
	void * readArg, CallbackFunc pWriteFunc, void * writeArg,
	CallbackFunc pErrorFunc, void * errorArg)
{
    unsigned i;
    if(!pReadFunc && !pWriteFunc && !pErrorFunc) {
	// nothing to do
        return false;
    }

    // See if there is a match entry
    for(i = 0; i < mPollListSize; i++) {
        if(mPollFdList[i].fd == fd) {
            break;
        }
    }

    // no match so add to end
    if(i >= mPollListSize) {
        LOG_DEBUG("Register %i", fd);

        if( i < MAX_FD_HANDLERS) {
            mPollFdList[i].fd = fd;
            mPollFdList[i].events = 0;
            memset(&mPollCallbackList[i], 0, sizeof(EventLoop::CallbackEntry));
            ++mPollListSize;
        } else {
            LOG_ERROR("Too many handlers");
	    return false;
        }
    }
    if(pReadFunc) {
        mPollCallbackList[i].pReadFunc = pReadFunc;
        mPollCallbackList[i].readArg = readArg;
	mPollFdList[i].events |= (POLLIN | POLLRDNORM);
    }
    if(pWriteFunc) {
        mPollCallbackList[i].pWriteFunc = pWriteFunc;
        mPollCallbackList[i].writeArg = writeArg;
        mPollFdList[i].events |= (POLLOUT | POLLWRNORM);
    }
    if(pErrorFunc) {
        mPollCallbackList[i].pErrorFunc = pErrorFunc;
        mPollCallbackList[i].errorArg = errorArg;
    }
    mPollListChanged = true;
    return true;
}


/*----------------------------------------------------------------------------*/
void EventLoop::unregister(int fd)
{
    unsigned i;
    for(i = 0; i < mPollListSize; i++) {
        if(mPollFdList[i].fd == fd) {
            break;
        }
    }
    if(i >= mPollListSize) {
	return;
    }

    LOG_DEBUG("Unregister %i", fd);

    --mPollListSize;

    for(;i < mPollListSize; i++) {
	mPollFdList[i] = mPollFdList[i+1];
	mPollCallbackList[i] = mPollCallbackList[i+1];
    }
    mPollListChanged = true;
}


/*----------------------------------------------------------------------------*/
void EventLoop::main()
{
    LOG_DEBUG("Starting loop");
    unsigned idx = 0;
    while(true) {
	mPollListChanged = false;

	if(mPollListSize > 0) {
	    int hitCnt = poll(mPollFdList, mPollListSize, 0);

	    while(hitCnt > 0) {
		idx = (idx >= mPollListSize) ? 0: idx+1;
	    	unsigned revents = mPollFdList[idx].revents;
		bool hit = (revents != 0);

		if((revents & (POLLIN | POLLRDNORM)) != 0) {
		    CallbackFunc pFunc = mPollCallbackList[idx].pReadFunc;
		    if(pFunc) {
 			pFunc(mPollCallbackList[idx].readArg);
		        if(mPollListChanged) {
			    break;
		        }
		    }
		}

		if((revents & (POLLOUT | POLLWRNORM)) != 0) {
		    CallbackFunc pFunc = mPollCallbackList[idx].pWriteFunc;
		    if(pFunc) {
			pFunc(mPollCallbackList[idx].writeArg);
		    	if(mPollListChanged) {
			    break;
			}
		    }
		}
		if(revents & POLLERR) {
		    CallbackFunc pFunc = mPollCallbackList[idx].pErrorFunc;
		    if(pFunc) {
			pFunc(mPollCallbackList[idx].errorArg);
		    	if(mPollListChanged) {
			    break;
			}
		    }
		}

		if(hit) {
		    --hitCnt;
		}
	    }
	}
    }
    LOG_DEBUG("Ending loop");
}

