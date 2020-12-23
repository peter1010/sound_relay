#ifndef _CAPTURE_H_
#define _CAPTURE_H_

#include <poll.h>
#include <alsa/asoundlib.h>

class RtpConnection;
class Connection;
struct OpusEncoder;

class Capture
{
private:
    snd_pcm_t * m_captureHandle;
    snd_output_t * m_stdout;
    snd_pcm_uframes_t m_periodSize;
    RtpConnection * mpConn;
    struct pollfd mFd;
    short * mBuffer;
    OpusEncoder * mEncoder;

    static void test_access(snd_pcm_t *, snd_pcm_hw_params_t *);
    static void test_formats(snd_pcm_t *, snd_pcm_hw_params_t *);
    static void test_channels(snd_pcm_hw_params_t *);
    static void test_rates(snd_pcm_hw_params_t *);

private:
    void open_pcm(const char * alsa_dev=0);
    void set_hw_params();

    void find_source();
    void find_mixer(const char *);
    void run();
    void do_loop();

public:
    Capture();
    void init();
    void attach(Connection * conn);

    static void read_callback(void *);
    static void write_callback(void *);
    static void error_callback(void *);

};

#endif

