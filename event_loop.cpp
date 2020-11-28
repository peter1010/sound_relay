#include <stdlib.h>
#include <poll.h>
#include <string.h>

#include "event_loop.h"
#include "logging.h"

unsigned EventLoop::mPollListSize = 0;


EventLoop::CallbackEntry EventLoop::mPollCallbackList[MAX_FD_HANDLERS];
struct pollfd EventLoop::mPollFdList[MAX_FD_HANDLERS];


void EventLoop::register_read_callback(int fd, CallbackFunc pFunc, void * arg)
{
    register_callback(fd, pFunc, arg, NULL, NULL);
}


void EventLoop::register_write_callback(int fd, CallbackFunc pFunc, void * arg)
{
    register_callback(fd, NULL, NULL, pFunc, arg);
}


void EventLoop::register_callback(int fd, CallbackFunc pReadFunc, void * readArg,
        CallbackFunc pWriteFunc, void * writeArg)
{
    unsigned i;
    if((pReadFunc == NULL) && (pWriteFunc == NULL)) {
        return;
    }

    for(i = 0; i < mPollListSize; i++) {
        if(mPollFdList[i].fd == fd) {
            break;
        }
    }
    if(i >= mPollListSize) {
        if( i < MAX_FD_HANDLERS) {
            mPollFdList[i].fd = fd;
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
    }
    if(pWriteFunc) {
        mPollCallbackList[i].pWriteFunc = pWriteFunc;
        mPollCallbackList[i].writeArg = writeArg;
    }
}


void EventLoop::main()
{
}

