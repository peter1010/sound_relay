#ifndef _EVENT_LOOP_H_
#define _EVENT_LOOP_H_

#include <poll.h>

typedef void (*CallbackFunc)(void * arg);

#define MAX_FD_HANDLERS (10)

class EventLoop
{
public:
    static void register_read_callback(int fd, CallbackFunc pFunc, void * arg);
    static void register_write_callback(int fd, CallbackFunc pFunc, void * arg);

    static void main();
private:
    
    static void register_callback(int fd, CallbackFunc pReadFunc, void * readArg,
        CallbackFunc pWriteFunc, void * writeArg);

    struct CallbackEntry {
        CallbackFunc pReadFunc;
        void * readArg;
        CallbackFunc pWriteFunc;
        void * writeArg;
    };
   
    static unsigned mPollListSize;

    static struct pollfd mPollFdList[MAX_FD_HANDLERS];
    static CallbackEntry mPollCallbackList[MAX_FD_HANDLERS];
};

#endif
