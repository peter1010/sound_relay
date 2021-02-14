#include <stdio.h>
//#include <math.h>

#include "sound.h"
#include "logging.h"
#include "event_loop.h"


snd_output_t * Sound::mStdout = 0;


/******************************************************************************/
SoundException::SoundException(const char * fmt, ...) 
{
    va_list ap;
    va_start(ap, fmt);

    VLOG_ERROR(fmt, ap);
    va_end(ap);
}


/******************************************************************************/
SndPcmHwParamsT::SndPcmHwParamsT()
{
    const int status = snd_pcm_hw_params_malloc(&mPtr);
    if(status < 0) {
        throw SoundException("snd_pcm_hw_params_malloc() failed => %s",
		       	snd_strerror(status));
    }
}


/******************************************************************************/
SndPcmHwParamsT::~SndPcmHwParamsT()
{
    snd_pcm_hw_params_free(mPtr);
}


/******************************************************************************/
SndPcmSwParamsT::SndPcmSwParamsT()
{
    const int status = snd_pcm_sw_params_malloc(&mPtr);
    if(status < 0) {
        throw SoundException("snd_pcm_sw_params_malloc() failed => %s",
		       	snd_strerror(status));
    }
}


/******************************************************************************/
SndPcmSwParamsT::~SndPcmSwParamsT()
{
    snd_pcm_sw_params_free(mPtr);
}



/******************************************************************************/
SndMixerT::SndMixerT()
{
    const int status = snd_mixer_open(&mHandle, 0);
    if(status < 0) {
	throw SoundException("snd_mixer_open() failed => %s",
                    snd_strerror(status));
    }
}


/******************************************************************************/
SndMixerT::~SndMixerT()
{
    snd_mixer_close(mHandle);
}


/******************************************************************************/
SndMixerSelemIdT::SndMixerSelemIdT()
{
    const int status = snd_mixer_selem_id_malloc(&mSid);
    if(status < 0) {
        throw SoundException("snd_mixer_selem_id_malloc() failed => %s",
                    snd_strerror(status));
    }
}


/******************************************************************************/
SndMixerSelemIdT::~SndMixerSelemIdT()
{
    snd_mixer_selem_id_free(mSid);
}


/******************************************************************************/
SndDeviceNameHint::SndDeviceNameHint(int card)
{
    const int status = snd_device_name_hint(card, "pcm", &mHints);
    if(status < 0) {
        throw SoundException("snd_device_name_hint(%i) failed => %s", card,
                    snd_strerror(status));
    }
}
        

/******************************************************************************/
SndDeviceNameHint::~SndDeviceNameHint()
{	
    snd_device_name_free_hint(mHints);
}
 

/******************************************************************************/
Sound::Sound() : mPcmHandle(0)
{
    if(!mStdout) {
	const int status = snd_output_stdio_attach(&mStdout, stdout, 0);
        if(status < 0) {
            LOG_ERROR("Failed to attach to stdout");
        }
    }
    mFd.fd = -1;
}


/******************************************************************************/
void Sound::dump_hw_params(snd_pcm_hw_params_t * params)
{
    snd_pcm_hw_params_dump(params, mStdout);
}


/******************************************************************************/
void Sound::dump_sw_params(snd_pcm_sw_params_t * params)
{
    snd_pcm_sw_params_dump(params, mStdout);
}



/*****************************************************************************/
void Sound::test_access(snd_pcm_t * handle, snd_pcm_hw_params_t * params)
{
    static const snd_pcm_access_t accesses[] = {
    	SND_PCM_ACCESS_MMAP_INTERLEAVED,
	SND_PCM_ACCESS_MMAP_NONINTERLEAVED,
	SND_PCM_ACCESS_MMAP_COMPLEX,
	SND_PCM_ACCESS_RW_INTERLEAVED,
	SND_PCM_ACCESS_RW_NONINTERLEAVED,
    };
    unsigned i;
    for(i = 0; i < sizeof(accesses)/sizeof(snd_pcm_access_t); i++) {
        const int status = snd_pcm_hw_params_test_access(handle, params,
                accesses[i]);
        if(status == 0) {
            printf("Access type %s is supported\n",
                    snd_pcm_access_name(accesses[i]));
        }
    }
}


/*****************************************************************************/
void Sound::test_formats(snd_pcm_t * handle, snd_pcm_hw_params_t * params)
{
   static const snd_pcm_format_t formats[] = {
	SND_PCM_FORMAT_S8,
	SND_PCM_FORMAT_U8,
	SND_PCM_FORMAT_S16_LE,
	SND_PCM_FORMAT_S16_BE,
	SND_PCM_FORMAT_U16_LE,
	SND_PCM_FORMAT_U16_BE,
	SND_PCM_FORMAT_S24_LE,
	SND_PCM_FORMAT_S24_BE,
	SND_PCM_FORMAT_U24_LE,
	SND_PCM_FORMAT_U24_BE,
	SND_PCM_FORMAT_S32_LE,
	SND_PCM_FORMAT_S32_BE,
	SND_PCM_FORMAT_U32_LE,
	SND_PCM_FORMAT_U32_BE,
	SND_PCM_FORMAT_FLOAT_LE,
	SND_PCM_FORMAT_FLOAT_BE,
	SND_PCM_FORMAT_FLOAT64_LE,
	SND_PCM_FORMAT_FLOAT64_BE,
	SND_PCM_FORMAT_IEC958_SUBFRAME_LE,
	SND_PCM_FORMAT_IEC958_SUBFRAME_BE,
	SND_PCM_FORMAT_MU_LAW,
	SND_PCM_FORMAT_A_LAW,
	SND_PCM_FORMAT_IMA_ADPCM,
	SND_PCM_FORMAT_MPEG,
	SND_PCM_FORMAT_GSM,
	SND_PCM_FORMAT_SPECIAL,
	SND_PCM_FORMAT_S24_3LE,
	SND_PCM_FORMAT_S24_3BE,
	SND_PCM_FORMAT_U24_3LE,
	SND_PCM_FORMAT_U24_3BE,
	SND_PCM_FORMAT_S20_3LE,
	SND_PCM_FORMAT_S20_3BE,
	SND_PCM_FORMAT_U20_3LE,
	SND_PCM_FORMAT_U20_3BE,
	SND_PCM_FORMAT_S18_3LE,
	SND_PCM_FORMAT_S18_3BE,
	SND_PCM_FORMAT_U18_3LE,
	SND_PCM_FORMAT_U18_3BE,
    }; 
    unsigned i;
    for(i = 0; i < sizeof(formats)/sizeof(snd_pcm_format_t); i++) {
        const int status = snd_pcm_hw_params_test_format(handle, params,
                formats[i]);
        if(status == 0) {
            printf("Format type %s is supported\n",
                    snd_pcm_format_name(formats[i]));
        }
    }
}


/*****************************************************************************/
void Sound::test_channels(snd_pcm_hw_params_t * params)
{
    unsigned int min = 0;
    unsigned int max = 0;

    int status = snd_pcm_hw_params_get_channels_min(params, &min);
    if(status < 0) {
	LOG_ERROR("cannot get minimum channels count: %s",
                snd_strerror(status));
	return;
    }
    status = snd_pcm_hw_params_get_channels_max(params, &max);
    if(status < 0) {
	LOG_ERROR("cannot get maximum channels count: %s",
                snd_strerror(status));
	return;
    }
    printf("Channels: %i - %i\n", min, max);
#if 0
    // snd_pcm_hw_params_test_channels(handle, params, i) == 0)
#endif
    putchar('\n');
}


/*****************************************************************************/
void Sound::test_rates(snd_pcm_hw_params_t * params)
{
    unsigned int min = 0;
    unsigned int max = 0;
    int status = snd_pcm_hw_params_get_rate_min(params, &min, NULL);
    if(status < 0) {
	LOG_ERROR("cannot get minimum rate: %s\n", snd_strerror(status));
	return;
    }
    status = snd_pcm_hw_params_get_rate_max(params, &max, NULL);
    if(status < 0) {
	LOG_ERROR("cannot get maximum rate: %s\n", snd_strerror(status));
	return;
    }

    printf("Sample rates: %i - %i\n", min, max);
    
    // snd_pcm_hw_params_test_rate(handle, params, min + 1, 0))
}


/*****************************************************************************/
void Sound::close()
{
    if(mPcmHandle) {
	EventLoop::instance().unregister(mFd.fd);
	snd_pcm_close(mPcmHandle);
	mPcmHandle = 0;
	mFd.fd = -1;
    }
}


/*****************************************************************************/
Sound::~Sound()
{
    LOG_DEBUG("~Sound");
    close();
}

