#ifndef _EVENT_LOOP_H_
#define _EVENT_LOOP_H_

#include <poll.h>

typedef void (*CallbackFunc)(void * arg);

#define MAX_FD_HANDLERS (10)

class EventLoop
{
public:

    static EventLoop & instance() { if(!mInstance) { create(); } return *mInstance;};

    bool register_read_callback(int, CallbackFunc, void *);
    bool register_write_callback(int, CallbackFunc, void *);
    bool register_error_callback(int, CallbackFunc, void *);

    void unregister(int);

    void main();
private:
    
    static void create();
    EventLoop();

    bool register_callback(int, CallbackFunc, void *, CallbackFunc, void *,
        	CallbackFunc, void *);

    struct CallbackEntry {
        CallbackFunc pReadFunc;
        void * readArg;
        CallbackFunc pWriteFunc;
        void * writeArg;
        CallbackFunc pErrorFunc;
        void * errorArg;
    };
   
    static EventLoop * mInstance;
    bool mPollListChanged;
    unsigned mPollListSize;

    struct pollfd mPollFdList[MAX_FD_HANDLERS];
    CallbackEntry mPollCallbackList[MAX_FD_HANDLERS];
};

#endif
