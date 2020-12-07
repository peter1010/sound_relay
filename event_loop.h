#ifndef _EVENT_LOOP_H_
#define _EVENT_LOOP_H_

#include <poll.h>

typedef void (*CallbackFunc)(void * arg);

#define MAX_FD_HANDLERS (10)

class EventLoop
{
public:

    static EventLoop & instance() { if(!mInstance) { create(); } return *mInstance;};

    bool register_read_callback(int fd, CallbackFunc pFunc, void * arg);
    bool register_write_callback(int fd, CallbackFunc pFunc, void * arg);
    void unregister(int fd);

    void main();
private:
    
    static void create();
    EventLoop();

    bool register_callback(int fd, CallbackFunc pReadFunc, void * readArg,
        CallbackFunc pWriteFunc, void * writeArg);

    struct CallbackEntry {
        CallbackFunc pReadFunc;
        void * readArg;
        CallbackFunc pWriteFunc;
        void * writeArg;
    };
   
    static EventLoop * mInstance;
    bool mPollListChanged;
    unsigned mPollListSize;

    struct pollfd mPollFdList[MAX_FD_HANDLERS];
    CallbackEntry mPollCallbackList[MAX_FD_HANDLERS];
};

#endif
