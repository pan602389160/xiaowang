#ifndef __MOZART_MODULE_LOCAL_MUSIC_H__
#define __MOZART_MODULE_LOCAL_MUSIC_H__

#ifdef  __cplusplus
extern "C" {
#endif

#if (SUPPORT_LOCALPLAYER == 1)
	extern int mozart_module_local_music_startup(void);
#else
	static inline  int mozart_module_local_music_startup(void) { return 0; }
#endif

#ifdef  __cplusplus
}
#endif

#endif	/* __MOZART_MODULE_LOCAL_MUSIC_H__ */
