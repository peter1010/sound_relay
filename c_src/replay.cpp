#include <stdio.h>
#include <alsa/asoundlib.h>
#include <math.h>
#include <opus/opus.h>
#include <cstdint>

#include "replay.h"
#include "logging.h"
#include "event_loop.h"
#include "rtp_server.h"



/******************************************************************************/
Replay::Replay() : mpConn(0), mpBuffer(0), mpDecoder(0)
{
	// Opus supports only 16 bit sized int samples or floats
	mSampleSize = 16;     // 16 bit samples

	mChannels = 2;        // 2 channels

	// Opus supports 5 bands 8, 12, 16, 24 & 48
	mSampleRate = 48000;  // 48 kSamples/sec (initinal value)
}


/******************************************************************************/
Replay::~Replay()
{
	if(mpConn) {
		mpConn = 0;
	}
	if(mpBuffer) {
		delete [] mpBuffer;
		mpBuffer = 0;
	}
	if(mpDecoder) {
		opus_decoder_destroy(mpDecoder);
		mpDecoder = 0;
	}
}



/******************************************************************************/
void Replay::find_sink()
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
			if((!p) || (strcmp(p, "Output") == 0)) {
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
void Replay::find_mixer(const char * device)
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
void Replay::open(const char * alsa_dev)
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
				SND_PCM_STREAM_PLAYBACK, 0);
	if(status < 0) {
		LOG_ERROR("snd_pcm_open failed:%s", snd_strerror(status));
	} else {
		snd_pcm_poll_descriptors(handle, &mFd, 1);
		mPcmHandle = handle;
	}
}


/*****************************************************************************/
void Replay::set_hw_params()
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

	// test_formats(handle, params);
	// Select the format 16 bit signed
	snd_pcm_format_t fmt = SND_PCM_FORMAT_S16;
	assert(mSampleSize == 16);

	status = snd_pcm_hw_params_set_format(handle, params, fmt);
	if(status < 0) {
		throw SoundException("snd_pcm_hw_params_set_format failed:%s", snd_strerror(status));
	}
	LOG_INFO("format=%s", snd_pcm_format_name(fmt));

	// Select number of channels
	// test_channels(params);
	// Set number of channels
	status = snd_pcm_hw_params_set_channels(handle, params, mChannels);
	if(status < 0) {
		throw SoundException("Failed to set channels: %s\n", snd_strerror(status));
	}

	// Select sample rate
	// test_rates(params);
	unsigned int val = mSampleRate;
	int dir = 0;

	status = snd_pcm_hw_params_set_rate_near(handle, params, &val, &dir);
	if(status < 0) {
		throw SoundException("Failed to open PCM: %s\n", snd_strerror(status));
	}

	printf("Set sample rate to %i\n", val);
	mSampleRate = val;

	// Set period time (i.e. between interrupts)
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
void Replay::set_sw_params()
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
void Replay::read_callback(void * arg)
{
//    LOG_DEBUG("read_callback");

	Replay * pThis = reinterpret_cast<Replay *>(arg);
	unsigned short revents;

	pThis->mFd.revents = POLLIN | POLLRDNORM;
	snd_pcm_poll_descriptors_revents(pThis->mPcmHandle, &pThis->mFd, 1, &revents);
	if(revents) {
		pThis->do_loop();
	}
}


/*****************************************************************************/
void Replay::write_callback(void * arg)
{
//    LOG_DEBUG("write_callback");

	Replay * pThis = reinterpret_cast<Replay *>(arg);
	unsigned short revents;

	pThis->mFd.revents = POLLOUT | POLLWRNORM;
	snd_pcm_poll_descriptors_revents(pThis->mPcmHandle, &pThis->mFd, 1, &revents);
	if(revents) {
		pThis->do_loop();
	}
}



/*****************************************************************************/
void Replay::error_callback(void * arg)
{
//    LOG_DEBUG("error_callback");

	Replay * pThis = reinterpret_cast<Replay *>(arg);
	unsigned short revents;

	pThis->mFd.revents = POLLERR;
	snd_pcm_poll_descriptors_revents(pThis->mPcmHandle, &pThis->mFd, 1, &revents);
	if(revents) {
		pThis->do_loop();
	}
}

/*****************************************************************************/
void Replay::write(const uint8_t * pData, unsigned len)
{
	static int cnt = 0;
	snd_pcm_t * handle = mPcmHandle;

	//
	int frames = (mSampleRate * 120UL) / 1000;

	int opus_status = opus_decode(mpDecoder, pData,
				len, mpBuffer, frames, 0);

	if(opus_status <= 0) {
		LOG_ERROR("Opus decode failed %s", opus_strerror(opus_status));
		return;
	}

	frames = opus_status;

	int status = snd_pcm_writei(handle, mpBuffer, frames);
	if(status < 0)
	{
		if(status == -EPIPE)
		{
			LOG_ERROR("Under run occurred");
//	snd_pcm_state_t state = snd_pcm_state(handle);
//	LOG_DEBUG("State is %s\n", snd_pcm_state_name(state));

			// Restart
			snd_pcm_prepare(handle);
			status = snd_pcm_writei(handle, mpBuffer, frames);
		}
		else
		{
			LOG_ERROR("Write error:%s", snd_strerror(status));
		}
	}
	else if(status != (int) frames)
	{
		LOG_ERROR("Short write");
	}
	cnt++;
	if(cnt > 100) {
		snd_pcm_sframes_t delay = snd_pcm_avail(handle);
		LOG_DEBUG("%lu", delay);
		cnt = 0;
	}
}


/*****************************************************************************/
void Replay::run()
{
	int error;

	const int frames = (mSampleRate * 120UL) / 1000;  // Opus says allow for 120 ms
	mpBuffer = new int16_t[frames * 2];

	snd_pcm_t * handle = mPcmHandle;

	mpDecoder = opus_decoder_create(mSampleRate, mChannels, &error);
	if(mpDecoder == 0) {
		LOG_ERROR("Failed to create opus decoder: %s", opus_strerror(error));
		return;
	}

	opus_decoder_ctl(mpDecoder, OPUS_SET_BITRATE(128000));
	opus_decoder_ctl(mpDecoder, OPUS_SET_COMPLEXITY(9));


//	int status = snd_pcm_start(handle);
//	if(status < 0) {
//		LOG_ERROR("Failed to start");
//	}

	snd_pcm_state_t state = snd_pcm_state(handle);
	printf("State is %s\n", snd_pcm_state_name(state));

	LOG_DEBUG("Play started");

#if 0
	const int count = snd_pcm_poll_descriptors_count(handle);
	if(count != 1) {
		LOG_ERROR("Alsa using more that one file descriptor");
		return;
	}

	int fd = mFd.fd;
	unsigned events = mFd.events;
	if(events & (POLLIN | POLLRDNORM)) {
		EventLoop::instance().register_read_callback(fd, Replay::read_callback, this);
	}
	if(events & (POLLOUT | POLLWRNORM)) {
	EventLoop::instance().register_write_callback(fd, Replay::write_callback, this);
	}
	EventLoop::instance().register_error_callback(fd, Replay::error_callback, this);
#endif
}


/*****************************************************************************/
void Replay::do_loop()
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



	unsigned size = mpConn->get_packet_buffer_size();

	if(size > 0) {
		int opus_status = opus_decode(mpDecoder, mpConn->get_packet_buffer(),
				size, mpBuffer, frames, 0);

		if(opus_status <= 0) {
			LOG_ERROR("Opus decode failed %s", opus_strerror(opus_status));
		}

		int status = snd_pcm_writei(handle, mpBuffer, frames);
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
			LOG_ERROR("Short write");
		}
	}
}


/*****************************************************************************/
void Replay::attach(RtpServer * conn)
{
	mpConn = conn;
}


/*****************************************************************************/
/**
 * Initialise the Video capture
 */
void Replay::init()
{
	find_sink();
//    find_mixer("hw:0");
	open();
	set_hw_params();
	set_sw_params();
	run();
}
