#ifndef _VR_PLAYER_H_
#define _VR_PLAYER_H_

#include "player_interface.h"

extern bool vr_need_play_next;

extern int vr_player_init(void);
extern int vr_player_uninit(void);
extern int vr_player_pause(void);
extern int vr_player_seek(int);
extern int vr_player_stop(void);
extern int vr_player_volset(int volume);
extern player_status_t vr_player_getstatus(void);
extern int vr_player_resume(void);
extern int vr_player_volget(void);
extern int vr_player_getduration(void);
extern int vr_player_getpos(void);
extern int vr_player_playurl(char *file);
extern int vr_player_cacheurl(char *file);
extern void vr_player_playtts(char *word, bool sync);

#endif // _VR_PLAYER_H_
