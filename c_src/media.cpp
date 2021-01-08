#include "media.h"
#include "session.h"

Media * Media::mInstance = 0;


/*----------------------------------------------------------------------------*/
Session * Media::get_session(const char * pathname)
{
    if(!Media::mInstance) {
	Media::mInstance = new Media();
    }
    return Media::mInstance->find_session(pathname);
}


/*----------------------------------------------------------------------------*/
Session * Media::find_session(const char * pathname) const
{
    return pSession;
}


/*----------------------------------------------------------------------------*/
Media::Media()
{
    pSession = new Session();
}

