#include <stdlib.h>
#include <poll.h>
#include <string.h>
#include <stdio.h>

#include "event_loop.h"
#include "logging.h"


bool EventLoop::mPollListChanged;
unsigned EventLoop::mPollListSize = 0;


EventLoop::CallbackEntry EventLoop::mPollCallbackList[MAX_FD_HANDLERS];
struct pollfd EventLoop::mPollFdList[MAX_FD_HANDLERS];

/*----------------------------------------------------------------------------*/
void EventLoop::register_read_callback(int fd, CallbackFunc pFunc, void * arg)
{
    register_callback(fd, pFunc, arg, NULL, NULL);
}


/*----------------------------------------------------------------------------*/
void EventLoop::register_write_callback(int fd, CallbackFunc pFunc, void * arg)
{
    register_callback(fd, NULL, NULL, pFunc, arg);
}


/*----------------------------------------------------------------------------*/
void EventLoop::register_callback(int fd, CallbackFunc pReadFunc, 
	void * readArg, CallbackFunc pWriteFunc, void * writeArg)
{
    unsigned i;
    if((pReadFunc == NULL) && (pWriteFunc == NULL)) {
	// nothing to do
        return;
    }

    // See if there is a match entry
    for(i = 0; i < mPollListSize; i++) {
        if(mPollFdList[i].fd == fd) {
            break;
        }
    }

    // no match so add to end
    if(i >= mPollListSize) {
        if( i < MAX_FD_HANDLERS) {
            mPollFdList[i].fd = fd;
            mPollFdList[i].events = 0;
            memset(&mPollCallbackList[i], 0, sizeof(EventLoop::CallbackEntry));
            ++mPollListSize;
        } else {
            LOG_ERROR("Too many handlers");
            exit(1);
        }
    }
    if(pReadFunc) {
        mPollCallbackList[i].pReadFunc = pReadFunc;
        mPollCallbackList[i].readArg = readArg;
	mPollFdList[i].events |= POLLIN;
    }
    if(pWriteFunc) {
        mPollCallbackList[i].pWriteFunc = pWriteFunc;
        mPollCallbackList[i].writeArg = writeArg;
        mPollFdList[i].events |= POLLOUT;
    }
    mPollListChanged = true;
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
    for(;i < mPollListSize; i++) {
	mPollFdList[i] = mPollFdList[i+1];
	mPollCallbackList[i] = mPollCallbackList[i+1];
    }
    --mPollListSize;
    mPollListChanged = true;
}


/*----------------------------------------------------------------------------*/
void EventLoop::main()
{
    printf("Starting loop\n");
    unsigned idx = 0;
    while(true) {
	mPollListChanged = false;

	if(mPollListSize > 0) {
	    int hitCnt = poll(mPollFdList, mPollListSize, 0);

	    while(hitCnt > 0) {
		idx = (idx >= mPollListSize) ? 0: idx+1;
	    	unsigned revents = mPollFdList[idx].revents;
		bool hit = false;

		if((revents & POLLIN) != 0) {
		    mPollCallbackList[idx].pReadFunc(mPollCallbackList[idx].readArg);
		    hit = true;
		    if(mPollListChanged) {
			break;
		    }
		}

		if((revents & POLLOUT) != 0) {
		    mPollCallbackList[idx].pWriteFunc(mPollCallbackList[idx].writeArg);
		    hit = true;
		    if(mPollListChanged) {
			break;
		    }
		}

		if(hit) {
		    --hitCnt;
		}
	    }
	}
    }
    printf("Ending loop\n");
}

