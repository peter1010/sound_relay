#include <alsa/asoundlib.h>

class Capture
{
private:
    snd_pcm_t * m_captureHandle;
    snd_output_t * m_stdout;
    snd_pcm_uframes_t m_periodSize;


    static void test_access(snd_pcm_t * handle, snd_pcm_hw_params_t * params);
    static void test_formats(snd_pcm_t * handle, snd_pcm_hw_params_t * params);
    static void test_channels(snd_pcm_hw_params_t * params);
    static void test_rates(snd_pcm_hw_params_t * params);

private:
    void open_pcm(const char * alsa_dev=0);
    void set_hw_params();

    void find_source();
    void find_mixer(const char * device);
    void run();

public:
    Capture();
    void init();
};
