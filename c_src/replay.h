#ifndef _REPLAY_H_
#define _REPLAY_H_

#include "sound.h"
#include <cstdint>

class RtpServer;
struct OpusDecoder;

class Replay : public Sound
{
private:
	snd_pcm_uframes_t m_periodSize;
	RtpServer * mpConn;
	int16_t * mpBuffer;
	OpusDecoder * mpDecoder;

private:
	void open(const char * alsa_dev=0);

	void set_hw_params();
	void set_sw_params();

	void find_sink();
	void find_mixer(const char *);
	void run();
	void do_loop();

public:

	Replay();
	~Replay();
	void init();
	void attach(RtpServer * conn);

	void write(const uint8_t * pData, unsigned len);

	static void read_callback(void *);
	static void write_callback(void *);
	static void error_callback(void *);
};

#endif

