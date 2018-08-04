/*
	tv_audio.h
 */
#ifndef __TV_AUDIO_H__
#define __TV_AUDIO_H__

typedef enum {
	M_EMPTY = 0,
	M_PLANET,
	M_MOON,
} device_mode_t;

extern int tv_audio_start(char *app_name, device_mode_t mode);

extern void tv_audio_terminate(void);

#endif /* __TV_AUDIO_H__ */
