#ifndef GCWSOUND_H_
#define GCWSOUND_H_

void init_sdlaudio();

// Modified by the audio thread to match VideoFastForwarded after it finds
// that the video thread has skipped a frame. The audio thread must deal
// with the condition as soon as possible.
extern volatile unsigned int AudioFastForwarded;

// OUTPUT_SOUND_FREQUENCY should be a power-of-2 fraction of SOUND_FREQUENCY;
// if not, gcwsound.c's feed_buffer() needs to resample the output.
#define OUTPUT_SOUND_FREQUENCY 44100

#ifdef RG350
//#define AUDIO_OUTPUT_BUFFER_SIZE (((OUTPUT_SOUND_FREQUENCY / 50) + 0x0Fu) & ~0x0Fu)
#define AUDIO_OUTPUT_BUFFER_SIZE 2048
#else
#define AUDIO_OUTPUT_BUFFER_SIZE 2048
#endif

#define OUTPUT_FREQUENCY_DIVISOR ((int) (SOUND_FREQUENCY) / (OUTPUT_SOUND_FREQUENCY))

#endif /* GCWSOUND_H_ */
