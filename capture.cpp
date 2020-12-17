#include <stdio.h>
#include <alsa/asoundlib.h>
#include <math.h>
#include <dns_sd.h>

#include "capture.h"
#include "logging.h"
#include "event_loop.h"

Capture::Capture()
    : m_captureHandle(0)
{
    int status = snd_output_stdio_attach(&m_stdout, stdout, 0);
    if(status < 0) {
        LOG_ERROR("Failed to attach to stdout");
    }
}

/******************************************************************************/
void Capture::find_source()
{
    int card = -1;

    // Scan through the hardware cards
    do {
        int status = snd_card_next(&card);
        if(status < 0) {
            LOG_ERROR("snd_card_next(%i) failed => %s", card,
                    snd_strerror(status));
            break;
        }
        if( card == -1) {
            // No more cards
            break;
        }

        printf("Card %i\n", card);

        char * name = NULL;
        status = snd_card_get_name(card, &name);
        if(status < 0) {
            LOG_ERROR("snd_card_get_name(%i) failed => %s", card,
                    snd_strerror(status));
        } else {
            if(name) {
                printf("!!! Card name %s\n", name);
                free(name);
            }
        }

        void ** hints = NULL;
        status = snd_device_name_hint(card, "pcm", &hints);
        if(status < 0) {
            LOG_ERROR("snd_device_name_hint(%i) failed => %s", card,
                    snd_strerror(status));
            break;
        }

        for(int i = 0;;i++) {
            if (hints[i] == NULL) {
                break;
            }
            char * p = NULL;
            p = snd_device_name_get_hint(hints[i], "IOID");
            if((!p) || (strcmp(p, "Input") == 0)) {
                if(p) {
                    free(p);
                }
                p = snd_device_name_get_hint(hints[i], "NAME");
                if (p) {
                     printf("hw:%i,%i => %s\n", card, i, p);
                     free(p);
                }
                p =  snd_device_name_get_hint(hints[i], "DESC");
                if(p) {
                    printf("%s\n", p);
                    free(p);
                }
                printf("---\n");
            }
        }
        snd_device_name_free_hint(hints);
    } while(true);
}


/*****************************************************************************/
void Capture::find_mixer(const char * device)
{
    snd_mixer_t * handle = NULL;
    snd_mixer_selem_id_t * sid = NULL;

    int status = snd_mixer_open(&handle, 0);
    if(status < 0) {
        LOG_ERROR("snd_mixer_open() failed => %s",
                    snd_strerror(status));
        return;
    }

    do {
        status = snd_mixer_attach(handle, device);
        if(status < 0) {
            LOG_ERROR("snd_mixer_attach(%s) failed => %s", device,
                    snd_strerror(status));
            break;
        }
        /* Although no one knows what this does, its required */
        status = snd_mixer_selem_register(handle, NULL, NULL);
        if(status < 0) {
            LOG_ERROR("snd_mixer_selem_register() failed => %s",
                    snd_strerror(status));
            break;
        }
        status = snd_mixer_load(handle);
        if(status < 0) {
            LOG_ERROR("snd_mixer_load() failed => %s", snd_strerror(status));
            break;
        }

        status = snd_mixer_selem_id_malloc(&sid);
        if(status < 0) {
            LOG_ERROR("snd_mixer_selem_id_malloc() failed => %s",
                    snd_strerror(status));
            break;
        }

        snd_mixer_elem_t * elem;
        for(elem = snd_mixer_first_elem(handle); elem; elem = snd_mixer_elem_next(elem))
        {
            const int idx = snd_mixer_selem_get_index(elem);
//            printf("Active %i\n", snd_mixer_selem_is_active(elem));
            printf("NAME: %s:%d\n", snd_mixer_selem_get_name(elem), idx);
            for(int i = 0; i < 20; i++) {
                snd_mixer_selem_channel_id_t cid = static_cast<snd_mixer_selem_channel_id_t>(i);
                int sw = 0;
                if(snd_mixer_selem_get_capture_switch(elem, cid, &sw) >= 0)
                {
//                    printf("%s:%d\n", snd_mixer_selem_get_name(elem), idx);
                    printf("- Capture switch %i for %i\n", sw, i);
                }
                long value;
                if(snd_mixer_selem_get_capture_volume(elem, cid, &value) == 0)
                {
                    printf("- Capture volume %li for %i\n", value, i );
                }
            }

            long min, max;
            if(snd_mixer_selem_get_capture_volume_range(elem, &min, &max) >= 0) {
                long minDb = -99, maxDb = -99;
                snd_mixer_selem_ask_capture_vol_dB (elem, min, &minDb);
                snd_mixer_selem_ask_capture_vol_dB (elem, max, &maxDb);

                printf("-- Min %li (%li dB), Max %li (%li db)\n", min, minDb, max, maxDb);
            }
        }
    }
    while(0);


    if(sid) {
        snd_mixer_selem_id_free(sid);
    }
    if(handle) {
        snd_mixer_close(handle);
    }
}



/**
 * Open the PCM device (default)
 *
 */
void Capture::open_pcm(const char * alsa_dev) 
{
    if(alsa_dev == NULL) {
        alsa_dev = "default";
    }
    int status = snd_pcm_open(&m_captureHandle, alsa_dev,
            SND_PCM_STREAM_CAPTURE, 0);
    if(status < 0) {
        LOG_ERROR("snd_pcm_open failed:%s", snd_strerror(status));
    }
}

/*****************************************************************************/
void Capture::test_access(snd_pcm_t * handle, snd_pcm_hw_params_t * params)
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
void Capture::test_formats(snd_pcm_t * handle, snd_pcm_hw_params_t * params)
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
void Capture::test_channels(snd_pcm_hw_params_t * params)
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
void Capture::test_rates(snd_pcm_hw_params_t * params)
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
void Capture::set_hw_params()
{
    snd_pcm_hw_params_t * params;
    snd_pcm_t * handle = m_captureHandle;

    int status = snd_pcm_hw_params_malloc(&params);
    if(status < 0) {
        LOG_ERROR("snd_pcm_hw_params_malloc() failed => %s", snd_strerror(status));
        return;
    }

    do {
	// Fill in the params structure will full possibilities of the all
	// configurations for this device
        status = snd_pcm_hw_params_any(handle, params);
        if(status < 0) {
            LOG_ERROR("snd_pcm_hw_params_any failed:%s", snd_strerror(status));
            break;
        }

        // Disable any resampling we want a clean input
        status = snd_pcm_hw_params_set_rate_resample(handle, params, 0);
        if(status < 0) {
            LOG_ERROR("Failed to disable resampling: %s\n", snd_strerror(status));
            break;
        }

	// Select the access method
        test_access(handle, params);
        // Set Access format
        status = snd_pcm_hw_params_set_access(handle, params, SND_PCM_ACCESS_RW_INTERLEAVED);
        if(status < 0) {
            LOG_ERROR("Failed to set access mode: %s\n", snd_strerror(status));
            break;
        }

      	// Select the format
        test_formats(handle, params);
        // Set format
        snd_pcm_format_t fmt = SND_PCM_FORMAT_S16;
        status = snd_pcm_hw_params_set_format(handle, params, fmt);
        if(status < 0) {
            LOG_ERROR("snd_pcm_hw_params_set_format failed:%s", snd_strerror(status));
        }
        else {
            LOG_INFO("format=%s", snd_pcm_format_name(fmt));
        }
        
	// Select number of channels
        test_channels(params);
        // Set number of channels
        status = snd_pcm_hw_params_set_channels(handle, params, 2);
        if(status < 0) {
            LOG_ERROR("Failed to set channels: %s\n", snd_strerror(status));
        }

	// Select sample rate
        test_rates(params);
        unsigned int val = 44100;
        int dir = 0;

        status = snd_pcm_hw_params_set_rate_near(handle, params, &val, &dir);
        if(status < 0) {
            LOG_ERROR("Failed to open PCM: %s\n", snd_strerror(status));
        }

        printf("Set sample rate to %i\n", val);

        // Set period time (i.e. between interrupts)
        snd_pcm_uframes_t frames = 32;
        val = 10000;   // 10 ms
        status = snd_pcm_hw_params_set_period_time_near(handle, params, &val, &dir);
	if(status < 0) {
	    LOG_ERROR("Failed to set period: %s\n", snd_strerror(status));
	}
        printf("Set period to %i us\n", val);

	snd_pcm_hw_params_get_period_size(params, &m_periodSize, &dir);

	// Set the buffer size
	snd_pcm_uframes_t bufSize = m_periodSize * 3;
	status = snd_pcm_hw_params_set_buffer_size_near(handle, params, &bufSize);
	if(status < 0) {
	    LOG_ERROR("Failed to set buffer size: %s\n", snd_strerror(status));
	}
	printf("Buffer size is %lu\n", bufSize);


	status = snd_pcm_hw_params(handle, params);
        if(status < 0) {
            LOG_ERROR("Failed to set HW parameters: %s\n", snd_strerror(status));
        }

        snd_pcm_hw_params_dump(params, m_stdout);
    } while(false);

    snd_pcm_hw_params_free(params);
}        


/*****************************************************************************/
void Capture::read_callback(void * arg)
{
    Capture * pThis = reinterpret_cast<Capture *>(arg);

    struct pollfd ufd;
    unsigned short revents;

    ufd.fd = pThis->mFd;
    ufd.events = POLLIN | POLLRDNORM;
    snd_pcm_poll_descriptors_revents(pThis->m_captureHandle, &ufd, 1, &revents);
    if(revents) {
	pThis->do_loop();
    }
}


/*****************************************************************************/
void Capture::write_callback(void * arg)
{
    Capture * pThis = reinterpret_cast<Capture *>(arg);

    struct pollfd ufd;
    unsigned short revents;

    ufd.fd = pThis->mFd;
    ufd.events = POLLOUT | POLLWRNORM;
    snd_pcm_poll_descriptors_revents(pThis->m_captureHandle, &ufd, 1, &revents);
    if(revents) {
	pThis->do_loop();
    }
}



/*****************************************************************************/
void Capture::error_callback(void * arg)
{
    Capture * pThis = reinterpret_cast<Capture *>(arg);

    struct pollfd ufd;
    unsigned short revents;

    ufd.fd = pThis->mFd;
    ufd.events = POLLERR;
    snd_pcm_poll_descriptors_revents(pThis->m_captureHandle, &ufd, 1, &revents);
    if(revents) {
	pThis->do_loop();
    }
}


/*****************************************************************************/
void Capture::run()
{
    const int frames = 100;
    short * buffer = (short *) malloc(frames * 4);
    double y = 0.0;

    snd_pcm_t * handle = m_captureHandle;

    int status = snd_pcm_start(handle);
    if(status < 0) {	
	LOG_ERROR("Failed to start\n");
    }

    snd_pcm_state_t PrevState = SND_PCM_STATE_SETUP;

    const int count = snd_pcm_poll_descriptors_count(handle);
    if(count != 1) {
	LOG_ERROR("Alsa using more that one file descriptor");
	return;
    }

    struct pollfd ufd;
    snd_pcm_poll_descriptors(handle, &ufd, 1);

    int fd = ufd.fd;
    unsigned events = ufd.events;
    if(events & (POLLIN | POLLRDNORM)) {
        EventLoop::instance().register_read_callback(fd, Capture::read_callback, this);
    }
    if(events & (POLLOUT | POLLWRNORM)) {
       EventLoop::instance().register_write_callback(fd, Capture::write_callback, this);
    }
    EventLoop::instance().register_error_callback(fd, Capture::error_callback, this);
    mFd = fd;
}


void Capture::do_loop()
{
#if 0
	snd_pcm_state_t state = snd_pcm_state(handle);
	if(state != PrevState) {
	    printf("State is %s\n", snd_pcm_state_name(state));
	    PrevState = state;
	}

        int status = snd_pcm_readi(handle, buffer, frames);
        if(status < 0)
        {
            if(status == -EPIPE)
            {
                LOG_ERROR("Over run occurred\n");
                snd_pcm_prepare(handle);
            }
            else
            {
                LOG_ERROR("Read error:%s\n", snd_strerror(status));
            }
        }
        else if(status != (int) frames)
        {
            fprintf(stderr, "Short read\n");
        }
        int i;
        double value;
        for(i = 0; i < status; i++)
        {
            value = buffer[i];
//            printf("%f\n", value);
            y = (9999.0 * y + value * value)/10000.0;
        }
//        fprintf(stderr, "\r%015.2f\n", 10 * log10(y));
#endif
}



/**
 * Initialise the Video capture
 */
void Capture::init()
{
    find_source();
    find_mixer("hw:0");
    open_pcm();
    set_hw_params();
    run();
}
