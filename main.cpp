#include <stdio.h>

#include "capture.h"
#include "rtsp_server.h"
#include "event_loop.h"
#include "logging.h"

int main(int argc, char * argv[])
{
    set_logging_level(5);

    EventLoop loop;
    RtspServer rtsp(loop);

    loop.main();
//    Capture cap;
//    cap.init();
}
