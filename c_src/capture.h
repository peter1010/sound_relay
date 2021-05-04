#ifndef _CAPTURE_H_
#define _CAPTURE_H_

#include "sound.h"
#include <cstdint>

class RtpClient;
struct OpusEncoder;

class Capture : public Sound
{
private:
	snd_pcm_uframes_t mPeriodSize;
	RtpClient * mpConn;
	int16_t * mpBuffer;
	OpusEncoder * mpEncoder;

	uint8_t mPeriodTime;	// in milliseconds

private:
	void open(const char * alsa_dev=0);

	void set_hw_params();
	void set_sw_params();

	void find_source();
	void find_mixer(const char *);
	void run();
	void do_loop();

	Capture(const Capture &);
	Capture & operator=(const Capture &);
public:

	Capture();
	~Capture();
	void init();
	void attach(RtpClient * conn);

	static void read_callback(void *);
	static void write_callback(void *);
	static void error_callback(void *);
};

#endif

