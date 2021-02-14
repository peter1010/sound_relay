#include <stdio.h>
#include <alsa/asoundlib.h>
#include <math.h>
//#include <dns_sd.h>
#include <opus/opus.h>
#include <cstdint>

#include "capture.h"
#include "logging.h"
#include "event_loop.h"
#include "rtp_connection.h"



/******************************************************************************/
Capture::Capture() : mpConn(0), mpBuffer(0), mpEncoder(0)
{
}


/******************************************************************************/
Capture::~Capture()
{
    if(mpConn) {
	mpConn = 0;
    }
    if(mpBuffer) {
	delete [] mpBuffer;
	mpBuffer = 0;
    }
    if(mpEncoder) {
	opus_encoder_destroy(mpEncoder);
	mpEncoder = 0;
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

        LOG_DEBUG("Card %i", card);

        char * name = NULL;
        status = snd_card_get_name(card, &name);
        if(status < 0) {
            LOG_ERROR("snd_card_get_name(%i) failed => %s", card,
                    snd_strerror(status));
        } else {
            if(name) {
                LOG_DEBUG("Card name %s", name);
                free(name);
            }
        }

        SndDeviceNameHint hints(card);

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
                     LOG_DEBUG("hw:%i,%i => %s", card, i, p);
                     free(p);
                }
                p =  snd_device_name_get_hint(hints[i], "DESC");
                if(p) {
                    LOG_DEBUG("%s", p);
                    free(p);
                }
            }
        }
    } while(true);
}


/*****************************************************************************/
void Capture::find_mixer(const char * device)
{
    SndMixerT handle;

    int status = snd_mixer_attach(handle, device);
    if(status < 0) {
        throw SoundException("snd_mixer_attach(%s) failed => %s", device,
                    snd_strerror(status));
    }

    /* Although no one knows what this does, its required */
    status = snd_mixer_selem_register(handle, NULL, NULL);
    if(status < 0) {
        throw SoundException("snd_mixer_selem_register() failed => %s",
                    snd_strerror(status));
    }

    status = snd_mixer_load(handle);
    if(status < 0) {
        throw SoundException("snd_mixer_load() failed => %s", snd_strerror(status));
    }

    SndMixerSelemIdT sid;

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



/*****************************************************************************/
/**
 * Open the PCM device (default)
 *
 */
void Capture::open(const char * alsa_dev) 
{
    if(mPcmHandle) {
	LOG_ERROR("PCM Handle already exists");
	return;
    }

    snd_pcm_t * handle;
    if(alsa_dev == NULL) {
        alsa_dev = "default";
    }
    const int status = snd_pcm_open(&handle, alsa_dev,
		SND_PCM_STREAM_CAPTURE, 0);
    if(status < 0) {
        LOG_ERROR("snd_pcm_open failed:%s", snd_strerror(status));
    } else {
	snd_pcm_poll_descriptors(handle, &mFd, 1);
	mPcmHandle = handle;
    }
}


/*****************************************************************************/
void Capture::set_hw_params()
{
    snd_pcm_t * handle = mPcmHandle;

    if(!handle) {
	LOG_ERROR("Set HW params failed as no PCM handle defined");
	return;
    }

    SndPcmHwParamsT params;

    // Fill in the params structure will full possibilities of the all
    // configurations for this device
    int status = snd_pcm_hw_params_any(handle, params);
    if(status < 0) {
        throw SoundException("snd_pcm_hw_params_any failed:%s", snd_strerror(status));
    }

    // Disable any resampling we want a clean input
    status = snd_pcm_hw_params_set_rate_resample(handle, params, 0);
    if(status < 0) {
	throw SoundException("Failed to disable resampling: %s\n", snd_strerror(status));
    }

    // Select the access method
    test_access(handle, params);
    // Set Access format
    status = snd_pcm_hw_params_set_access(handle, params, SND_PCM_ACCESS_RW_INTERLEAVED);
    if(status < 0) {
        throw SoundException("Failed to set access mode: %s\n", snd_strerror(status));
    }

    // Select the format
    test_formats(handle, params);
    // Set format
    snd_pcm_format_t fmt = SND_PCM_FORMAT_S16;
    status = snd_pcm_hw_params_set_format(handle, params, fmt);
    if(status < 0) {
        throw SoundException("snd_pcm_hw_params_set_format failed:%s", snd_strerror(status));
    }
    LOG_INFO("format=%s", snd_pcm_format_name(fmt));
        
    // Select number of channels
    test_channels(params);
    // Set number of channels
    status = snd_pcm_hw_params_set_channels(handle, params, 2);
    if(status < 0) {
	throw SoundException("Failed to set channels: %s\n", snd_strerror(status));
    }

    // Select sample rate
    test_rates(params);
    unsigned int val = 48000;
    int dir = 0;

    status = snd_pcm_hw_params_set_rate_near(handle, params, &val, &dir);
    if(status < 0) {
        throw SoundException("Failed to open PCM: %s\n", snd_strerror(status));
    }

    printf("Set sample rate to %i\n", val);

    // Set period time (i.e. between interrupts)
//        snd_pcm_uframes_t frames = 32;
    val = 10000;   // 10 ms
    status = snd_pcm_hw_params_set_period_time_near(handle, params, &val, &dir);
    if(status < 0) {
	throw SoundException("Failed to set period: %s\n", snd_strerror(status));
    }
    printf("Set period to %i us\n", val);

    snd_pcm_hw_params_get_period_size(params, &m_periodSize, &dir);

    // Set the buffer size
    snd_pcm_uframes_t bufSize = m_periodSize * 3;
    status = snd_pcm_hw_params_set_buffer_size_near(handle, params, &bufSize);
    if(status < 0) {
	throw SoundException("Failed to set buffer size: %s\n", snd_strerror(status));
    }
    printf("Buffer size is %lu\n", bufSize);


    status = snd_pcm_hw_params(handle, params);
    if(status < 0) {
        throw SoundException("Failed to set HW parameters: %s\n", snd_strerror(status));
    }

    dump_hw_params(params);
}        


/*****************************************************************************/
void Capture::set_sw_params()
{
    snd_pcm_t * handle = mPcmHandle;

    if(!handle) {
	LOG_ERROR("Set SW params failed as no PCM handle defined");
	return;
    }

    SndPcmSwParamsT params;

    // Fill in the params structure will full possibilities of the all
    // configurations for this device
    int status = snd_pcm_sw_params_current(handle, params);
    if(status < 0) {
        throw SoundException("snd_pcm_sw_params_current failed:%s", snd_strerror(status));
    }

    dump_sw_params(params);
}        



/*****************************************************************************/
void Capture::read_callback(void * arg)
{
//    LOG_DEBUG("read_callback");

    Capture * pThis = reinterpret_cast<Capture *>(arg);
    unsigned short revents;

    pThis->mFd.revents = POLLIN | POLLRDNORM;
    snd_pcm_poll_descriptors_revents(pThis->mPcmHandle, &pThis->mFd, 1, &revents);
    if(revents) {
	pThis->do_loop();
    }
}


/*****************************************************************************/
void Capture::write_callback(void * arg)
{
//    LOG_DEBUG("write_callback");

    Capture * pThis = reinterpret_cast<Capture *>(arg);
    unsigned short revents;

    pThis->mFd.revents = POLLOUT | POLLWRNORM;
    snd_pcm_poll_descriptors_revents(pThis->mPcmHandle, &pThis->mFd, 1, &revents);
    if(revents) {
	pThis->do_loop();
    }
}



/*****************************************************************************/
void Capture::error_callback(void * arg)
{
//    LOG_DEBUG("error_callback");

    Capture * pThis = reinterpret_cast<Capture *>(arg);
    unsigned short revents;

    pThis->mFd.revents = POLLERR;
    snd_pcm_poll_descriptors_revents(pThis->mPcmHandle, &pThis->mFd, 1, &revents);
    if(revents) {
	pThis->do_loop();
    }
}


/*****************************************************************************/
void Capture::run()
{
    int error;

    const int frames = 480;  // 48000 / 120 => 1/100 sec
    mpBuffer = new int16_t[frames * 2]; // 4);

    snd_pcm_t * handle = mPcmHandle;

    mpEncoder = opus_encoder_create(48000, 2, OPUS_APPLICATION_AUDIO, &error);
    if(mpEncoder == 0) {
	LOG_ERROR("Failed to create opus encoder: %s", opus_strerror(error));
	return;
    }
    
    opus_encoder_ctl(mpEncoder, OPUS_SET_BITRATE(128000));
    opus_encoder_ctl(mpEncoder, OPUS_SET_COMPLEXITY(9));


    int status = snd_pcm_start(handle);
    if(status < 0) {	
	LOG_ERROR("Failed to start");
    }

    snd_pcm_state_t state = snd_pcm_state(handle);
    printf("State is %s\n", snd_pcm_state_name(state));
    
    LOG_DEBUG("Record started");

    const int count = snd_pcm_poll_descriptors_count(handle);
    if(count != 1) {
	LOG_ERROR("Alsa using more that one file descriptor");
	return;
    }
	

    int fd = mFd.fd;
    unsigned events = mFd.events;
    if(events & (POLLIN | POLLRDNORM)) {
        EventLoop::instance().register_read_callback(fd, Capture::read_callback, this);
    }
    if(events & (POLLOUT | POLLWRNORM)) {
       EventLoop::instance().register_write_callback(fd, Capture::write_callback, this);
    }
    EventLoop::instance().register_error_callback(fd, Capture::error_callback, this);
}


/*****************************************************************************/
void Capture::do_loop()
{
    static snd_pcm_state_t PrevState = SND_PCM_STATE_SETUP;
	
    snd_pcm_t * handle = mPcmHandle;

    snd_pcm_state_t state = snd_pcm_state(handle);
	if(state != PrevState) {
	    LOG_DEBUG("State is %s\n", snd_pcm_state_name(state));
	    PrevState = state;
	}

    // frames can only be 120,240,480 or 960 @ 48000
    const int frames = 480;

        int status = snd_pcm_readi(handle, mpBuffer, frames);
        if(status < 0)
        {
            if(status == -EPIPE)
            {
                LOG_ERROR("Over run occurred");
                snd_pcm_prepare(handle);
            }
            else
            {
                LOG_ERROR("Read error:%s", snd_strerror(status));
            }
        }
        else if(status != (int) frames)
        {
            LOG_ERROR("Short read");
        }
       

	int opus_status = opus_encode(mpEncoder, mpBuffer, frames,
			mpConn->get_packet_buffer(), 
			mpConn->get_packet_buffer_size());

	if(opus_status <= 0) {
	    LOG_ERROR("Opus encode failed %s", opus_strerror(opus_status));
	}
	
	mpConn->send_packet(opus_status, 480);
}


/*****************************************************************************/
void Capture::attach(Connection * conn) 
{ 
    mpConn = dynamic_cast<RtpConnection *>(conn); 
}


/*****************************************************************************/
/**
 * Initialise the Video capture
 */
void Capture::init()
{
    find_source();
//    find_mixer("hw:0");
    open();
    set_hw_params();
    set_sw_params();
    run();
}
