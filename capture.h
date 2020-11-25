#include <alsa/asoundlib.h>

class Capture
{
private:
    snd_pcm_t * m_captureHandle;

    static void test_access(snd_pcm_t * handle, snd_pcm_hw_params_t * params);
    static void test_formats(snd_pcm_t * handle, snd_pcm_hw_params_t * params);

private:
    void open_pcm(const char * alsa_dev=0);
    void set_hw_params();

    void find_source();
    void find_mixer(char * device);

public:
    Capture();
    void init();
};
