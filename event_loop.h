#ifndef _EVENT_LOOP_H_
#define _EVENT_LOOP_H_

#include <poll.h>

typedef void (*CallbackFunc)(void * arg);

#define MAX_FD_HANDLERS (10)

class EventLoop
{
public:
    static bool register_read_callback(int fd, CallbackFunc pFunc, void * arg);
    static bool register_write_callback(int fd, CallbackFunc pFunc, void * arg);
    static void unregister(int fd);

    static void main();
private:
    
    static bool register_callback(int fd, CallbackFunc pReadFunc, void * readArg,
        CallbackFunc pWriteFunc, void * writeArg);

    struct CallbackEntry {
        CallbackFunc pReadFunc;
        void * readArg;
        CallbackFunc pWriteFunc;
        void * writeArg;
    };
   
    static bool mPollListChanged;
    static unsigned mPollListSize;

    static struct pollfd mPollFdList[MAX_FD_HANDLERS];
    static CallbackEntry mPollCallbackList[MAX_FD_HANDLERS];
};

#endif
