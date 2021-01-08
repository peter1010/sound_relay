#ifndef _MEDIA_H_
#define _MEDIA_H_

class Session;


class Media
{
public:

    static Session * get_session(const char * pathname);

protected:

    Session * find_session(const char * pathname) const;

private:
    static Media * mInstance;

    Session * pSession;

    Media();
    Media(const Media &);
    const Media & operator=(const Media &);
};

#endif
