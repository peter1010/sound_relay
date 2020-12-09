#include "media.h"

Media * Media::mInstance = 0;


Session * Media::get_session(const char * pathname)
{
    if(!Media::mInstance) {
	Media::mInstance = new Media();
    }
    return Media::mInstance->get_session(pathname);
}


Session * Media::find_session(const char * pathname) const
{
    return 0;
}


Media::Media()
{
}

