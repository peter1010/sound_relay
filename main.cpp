#include <stdio.h>

#include "capture.h"
#include "rtsp_server.h"
#include "event_loop.h"
#include "logging.h"

int main(int argc, char * argv[])
{
    set_logging_level(5);

    RtspServer * p = new RtspServer();

    EventLoop::instance().main();
//    Capture cap;
//    cap.init();
}
