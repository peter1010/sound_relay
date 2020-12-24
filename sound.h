#ifndef _SOUND_H_
#define _SOUND_H_

#include <poll.h>
#include <alsa/asoundlib.h>

class Sound
{
public:
    Sound();
    ~Sound();

    void close();

protected:
    snd_pcm_t * mPcmHandle;
    struct pollfd mFd;

    static void test_access(snd_pcm_t *, snd_pcm_hw_params_t *);
    static void test_formats(snd_pcm_t *, snd_pcm_hw_params_t *);
    static void test_channels(snd_pcm_hw_params_t *);
    static void test_rates(snd_pcm_hw_params_t *);

    static void dump_hw_params(snd_pcm_hw_params_t * params);
    static void dump_sw_params(snd_pcm_sw_params_t * params);

private:
    static snd_output_t * mStdout;

};

#endif

