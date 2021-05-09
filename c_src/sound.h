#ifndef _SOUND_H_
#define _SOUND_H_

#include <poll.h>
#include <alsa/asoundlib.h>

/******************************************************************************/
class SoundException
{
public:
	SoundException(const char * fmt, ...) __attribute__((format (printf, 2, 3)));
};


/******************************************************************************/
class SndPcmHwParamsT
{
public:
	SndPcmHwParamsT();
	~SndPcmHwParamsT() noexcept;

	operator snd_pcm_hw_params_t *() const { return mPtr; };
private:

	snd_pcm_hw_params_t * mPtr;

	SndPcmHwParamsT(const SndPcmHwParamsT &);
	SndPcmHwParamsT & operator=(const SndPcmHwParamsT &);
};


/******************************************************************************/
class SndPcmSwParamsT
{
public:
	SndPcmSwParamsT();
	~SndPcmSwParamsT() noexcept;

	operator snd_pcm_sw_params_t *() const { return mPtr; };
private:

	snd_pcm_sw_params_t * mPtr;

	SndPcmSwParamsT(const SndPcmSwParamsT &);
	SndPcmSwParamsT & operator=(const SndPcmSwParamsT &);
};


/******************************************************************************/
class SndMixerT
{
public:
	SndMixerT();
	~SndMixerT() noexcept;

	operator snd_mixer_t * () const { return mHandle; };
private:
	snd_mixer_t * mHandle;

	SndMixerT(const SndMixerT &);
	SndMixerT & operator=(const SndMixerT &);
};


/******************************************************************************/
class SndMixerSelemIdT
{
public:
	SndMixerSelemIdT();
	~SndMixerSelemIdT() noexcept;

	operator snd_mixer_selem_id_t * () const { return mSid; };
private:
	snd_mixer_selem_id_t * mSid;

	SndMixerSelemIdT(const SndMixerSelemIdT &);
	SndMixerSelemIdT & operator=(const SndMixerSelemIdT &);
};


/******************************************************************************/
class SndDeviceNameHint
{
public:
	SndDeviceNameHint(int);
	~SndDeviceNameHint() noexcept;

	void * operator[](int idx) const { return mHints[idx]; };
private:
	void ** mHints = NULL;

	SndDeviceNameHint(const SndDeviceNameHint &);
	SndDeviceNameHint & operator=(const SndDeviceNameHint &);
};


/******************************************************************************/
class Sound
{
public:
	Sound();
	virtual ~Sound() = 0;

	virtual void init() = 0;
	void close();

protected:
	snd_pcm_t * mPcmHandle;
	struct pollfd mFd;

	uint8_t mSampleSize;
	uint8_t mChannels;
	uint16_t mSampleRate;

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

