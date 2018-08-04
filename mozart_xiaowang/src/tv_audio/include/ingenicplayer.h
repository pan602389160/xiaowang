#ifndef __INGENICPLAYER_H__
#define __INGENICPLAYER_H__

#include "mozart_config.h"
#include "appserver.h"
#include "musiclist_interface.h"

#if (SUPPORT_INGENICPLAYER == 1)
extern int mozart_ingenicplayer_notify_play_shortcut(char *shortcut_name);
extern int mozart_ingenicplayer_notify_pos(void);
extern int mozart_ingenicplayer_notify_volume(int volume);
extern int mozart_ingenicplayer_notify_play_mode(enum play_mode mode);
extern int mozart_ingenicplayer_notify_song_info(void);
extern int mozart_ingenicplayer_response_cmd(char *command, char *data, struct appserver_cli_info *owner);
extern int mozart_ingenicplayer_startup(void);
extern int mozart_ingenicplayer_shutdown(void);
#else
static inline int mozart_ingenicplayer_notify_play_shortcut(char *shortcut_name) { return 0; }
static inline int mozart_ingenicplayer_notify_pos(void) { return 0; }
static inline int mozart_ingenicplayer_notify_volume(int volume) { return 0; }
static inline int mozart_ingenicplayer_notify_play_mode(enum play_mode mode) { return 0; }
static inline int mozart_ingenicplayer_notify_song_info(void) { return 0; }
static inline int mozart_ingenicplayer_response_cmd(char *command,
						    char *data, struct appserver_cli_info *owner) { return 0; }
static inline int mozart_ingenicplayer_startup(void) { return 0; }
static inline int mozart_ingenicplayer_shutdown(void) { return 0; }
#endif

#endif /* __INGENICPLAYER_H__ */
