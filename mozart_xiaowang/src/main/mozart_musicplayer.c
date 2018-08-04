#include <stdio.h>
#include <string.h>

#include "sharememory_interface.h"
#include "event_interface.h"
#include "utils_interface.h"

#include "mozart_config.h"
#include "ingenicplayer.h"
#include "mozart_musicplayer.h"
#include "mozart_key_function.h"
#include "mozart_face.h"
#include "vr_interface.h"
#include "json-c/json.h"
/* #define MOZART_MUSICPLAYER_DEBUG */
#ifdef MOZART_MUSICPLAYER_DEBUG
#define pr_debug(fmt, args...)				\
	printf("[MUSICPLAYER] [DEBUG] {%s, %d}: "fmt, __func__, __LINE__, ##args)
#else  /* MOZART_MUSICPLAYER_DEBUG */
#define pr_debug(fmt, args...)			\
	do {} while (0)
#endif	/* MOZART_MUSICPLAYER_DEBUG */

#define pr_info(fmt, args...)						\
	printf("[MUSICPLAYER] [INFO] "fmt, ##args)

#define pr_err(fmt, args...)						\
	fprintf(stderr, "[MUSICPLAYER] [ERROR] {%s, %d}: "fmt, __func__, __LINE__, ##args)

#define musicplayer_lock(lock)						\
	do {                                                            \
		int i = 0;                                              \
									\
		while (pthread_mutex_trylock(lock)) {                   \
			if (i++ >= 100) {                               \
				pr_err("#### {%s, %s, %d} dead lock (last: %d)####\n", \
				       __FILE__, __func__, __LINE__, last_lock_line); \
				i = 0;                                  \
			}                                               \
			usleep(100 * 1000);                             \
		}                                                       \
		last_lock_line = __LINE__;				\
	} while (0)

#define musicplayer_unlock(lock)					\
	do {								\
		pthread_mutex_unlock(lock);				\
	} while (0)

#define VOLUME_VARIATION 10

musicplayer_handler_t *mozart_musicplayer_handler;
extern snd_source_t snd_source;
static bool autoplay_flag;
static int in_musicplayer;
static int last_lock_line;
static player_handler_t current_player_handler;
static player_status_t musicplayer_status;
static struct music_list *musicplayer_list;
static pthread_mutex_t musicplayer_mutex = PTHREAD_MUTEX_INITIALIZER;
int g_playing_flag = 0;
int g_alarm_cnt = 0;
char g_tmp[256] = {0};
static mozart_event mozart_musicplayer_event = {
	.event = {
		.misc = {
			.name = "musicplayer",
		},
	},
	.type = EVENT_MISC,
};

static int __mozart_musicplayer_play_music_info(musicplayer_handler_t *handler,
						struct music_info *info);

static int check_handler_uuid(musicplayer_handler_t *handler)
{
	if (handler == NULL || handler->player_handler == NULL) {
		pr_err("handler is null\n");
		return -1;
	}

	if (strcmp(current_player_handler.uuid, handler->player_handler->uuid)) {
		pr_debug("%s can't control, controler is %s\n", handler->player_handler->uuid,
			 current_player_handler.uuid);
		return -1;
	}

	return 0;
}

bool mozart_musicplayer_is_active(musicplayer_handler_t *handler)
{
	int ret;

	musicplayer_lock(&musicplayer_mutex);
	ret = check_handler_uuid(handler);
	musicplayer_unlock(&musicplayer_mutex);

	return ret ? false : true;
}
/**********************************************************************************************
 * music player list
 *********************************************************************************************/
static void free_data(void *arg)
{
	return;
}

int mozart_musicplayer_musiclist_insert(musicplayer_handler_t *handler, struct music_info *info)
{
	int ret;

	musicplayer_lock(&musicplayer_mutex);
	if (check_handler_uuid(handler) < 0) {
		pr_err("check handler uuid failed\n");
		musicplayer_unlock(&musicplayer_mutex);
		return -1;
	}

	ret = mozart_musiclist_insert(musicplayer_list, info);
	musicplayer_unlock(&musicplayer_mutex);

	/* TODO-tjiang: notify song queue ? */

	return ret;
}

int mozart_musicplayer_musiclist_delete_index(musicplayer_handler_t *handler, int index)
{
	int ret;

	musicplayer_lock(&musicplayer_mutex);
	if (check_handler_uuid(handler) < 0) {
		pr_err("check handler uuid failed\n");
		musicplayer_unlock(&musicplayer_mutex);
		return -1;
	}

	ret = mozart_musiclist_delete_index(musicplayer_list, index);
	musicplayer_unlock(&musicplayer_mutex);

	/* TODO-tjiang: notify song queue ? */

	return ret;
}

enum play_mode mozart_musicplayer_musiclist_get_play_mode(musicplayer_handler_t *handler)
{
	enum play_mode mode;

	musicplayer_lock(&musicplayer_mutex);
	if (check_handler_uuid(handler) < 0) {
		pr_debug("check handler uuid failed\n");
		musicplayer_unlock(&musicplayer_mutex);
		return play_mode_order;
	}

	mode = mozart_musiclist_get_play_mode(musicplayer_list);
	musicplayer_unlock(&musicplayer_mutex);

	return mode;
}

int mozart_musicplayer_musiclist_set_play_mode(musicplayer_handler_t *handler, enum play_mode mode)
{
	int ret;

	musicplayer_lock(&musicplayer_mutex);
	if (check_handler_uuid(handler) < 0) {
		pr_err("check handler uuid failed\n");
		musicplayer_unlock(&musicplayer_mutex);
		return -1;
	}

	ret = mozart_musiclist_set_play_mode(musicplayer_list, mode);
	musicplayer_unlock(&musicplayer_mutex);

	mozart_ingenicplayer_notify_play_mode(mode);

	return ret;
}

int mozart_musicplayer_musiclist_get_length(musicplayer_handler_t *handler)
{
	int ret;

	musicplayer_lock(&musicplayer_mutex);
	if (check_handler_uuid(handler) < 0) {
		pr_debug("check handler uuid failed\n");
		musicplayer_unlock(&musicplayer_mutex);
		return -1;
	}

	ret = mozart_musiclist_get_length(musicplayer_list);
	musicplayer_unlock(&musicplayer_mutex);

	return ret;
}

int mozart_musicplayer_musiclist_get_current_index(musicplayer_handler_t *handler)
{
	int ret;

	musicplayer_lock(&musicplayer_mutex);
	if (check_handler_uuid(handler) < 0) {
		pr_debug("check handler uuid failed\n");
		musicplayer_unlock(&musicplayer_mutex);
		return -1;
	}

	ret = mozart_musiclist_get_current_index(musicplayer_list);
	musicplayer_unlock(&musicplayer_mutex);

	return ret;
}

struct music_info *mozart_musicplayer_musiclist_get_current_info(musicplayer_handler_t *handler)
{
	struct music_info *info;

	musicplayer_lock(&musicplayer_mutex);
	if (check_handler_uuid(handler) < 0) {
		pr_debug("check handler uuid failed\n");
		musicplayer_unlock(&musicplayer_mutex);
		return NULL;
	}

	info = mozart_musiclist_get_current(musicplayer_list);
	musicplayer_unlock(&musicplayer_mutex);

	return info;
}

struct music_info *mozart_musicplayer_musiclist_get_index_info(musicplayer_handler_t *handler,
							       int index)
{
	struct music_info *info;

	musicplayer_lock(&musicplayer_mutex);
	if (check_handler_uuid(handler) < 0) {
		pr_debug("check handler uuid failed\n");
		musicplayer_unlock(&musicplayer_mutex);
		return NULL;
	}

	info = mozart_musiclist_get_index(musicplayer_list, index);
	musicplayer_unlock(&musicplayer_mutex);

	return info;
}

int mozart_musicplayer_musiclist_clean(musicplayer_handler_t *handler)
{
	musicplayer_lock(&musicplayer_mutex);
	if (check_handler_uuid(handler) < 0) {
		pr_err("check handler uuid failed\n");
		musicplayer_unlock(&musicplayer_mutex);
		return -1;
	}

	mozart_musiclist_clean(musicplayer_list);
	musicplayer_unlock(&musicplayer_mutex);

	return 0;
}

int mozart_musicplayer_play_shortcut(musicplayer_handler_t *handler, int index)
{
	char shortcut_name[20] = {};

	musicplayer_lock(&musicplayer_mutex);
	if (check_handler_uuid(handler) < 0) {
		pr_err("check handler uuid failed\n");
		musicplayer_unlock(&musicplayer_mutex);
		return -1;
	}

	sprintf(shortcut_name, "shortcut_%d", index);
	mozart_ingenicplayer_notify_play_shortcut(shortcut_name);
	musicplayer_unlock(&musicplayer_mutex);

	return 0;
}
#define LIGHT_BLUE    "\033[1;34m"
#define LIGHT_RED     "\033[1;31m"
#define LIGHT_PURPLE "\033[1;35m"
#define NONE          "\033[m"
#define my_pr(fmt, args...)		printf(LIGHT_BLUE fmt NONE,##args)
int luobo_fanyi_flag = 0;
int mozart_musicplayer_playfanyi_index(musicplayer_handler_t *handler, int index)
{	my_pr("-------------mozart_musicplayer_playfanyi_index------------ index = %d\n",index);
	int ret;
	struct music_info *info = NULL;

	//musicplayer_lock(&musicplayer_mutex);
	if (check_handler_uuid(handler) < 0) {
		pr_err("check handler uuid failed\n");
		//musicplayer_unlock(&musicplayer_mutex);
		return -1;
	}

	info = mozart_musiclist_set_index(musicplayer_list, index);
	if (info == NULL) {
		//musicplayer_unlock(&musicplayer_mutex);
		return -1;
	}
	luobo_fanyi_flag = 1;
	mozart_player_cacheurl(handler->player_handler, info->music_url);
	my_pr("---------------mozart_player_cacheurl--------------------\n");
	

	player_status_t status;
	while(1){
		usleep(100*1000);
		status = mozart_player_getstatus(handler->player_handler);
		//my_pr("qqqqq--->>>> status = %d\n",status);// 2 playing   3 pause
		if(status == 1){
			while(1){
				status = mozart_player_getstatus(handler->player_handler);
				//my_pr("QQQQQ--->>>> status = %d\n",status);
				if(status == 2){
					mozart_musicplayer_play_pause(handler);
					break;
				}
				usleep(100*1000);
			}
			break;
		}
		usleep(100*1000);

	}
	usleep(500*1000);
	luobo_fanyi_flag = 0;
	//mozart_musicplayer_play_pause(handler);
	//mozart_module_pause();
	mozart_musiclist_delete_index(musicplayer_list, index);
	

	return ret;
}

int mozart_musicplayer_play_index(musicplayer_handler_t *handler, int index)
{	my_pr("-------------mozart_musicplayer_play_index------------ index = %d\n",index);
	int ret;
	struct music_info *info = NULL;

	musicplayer_lock(&musicplayer_mutex);
	if (check_handler_uuid(handler) < 0) {
		pr_err("check handler uuid failed\n");
		musicplayer_unlock(&musicplayer_mutex);
		return -1;
	}

	info = mozart_musiclist_set_index(musicplayer_list, index);
	if (info == NULL) {
		musicplayer_unlock(&musicplayer_mutex);
		return -1;
	}

	ret = __mozart_musicplayer_play_music_info(handler, info);
	musicplayer_unlock(&musicplayer_mutex);

	return ret;
}

int mozart_musicplayer_prev_music(musicplayer_handler_t *handler)
{	printf("-------------mozart_musicplayer_prev_music------------\n");
	int ret;
	struct music_info *info = NULL;

	musicplayer_lock(&musicplayer_mutex);
	if (check_handler_uuid(handler) < 0) {
		pr_err("check handler uuid failed\n");
		musicplayer_unlock(&musicplayer_mutex);
		return -1;
	}

	info = mozart_musiclist_set_prev(musicplayer_list);
	if (info == NULL) {
		musicplayer_unlock(&musicplayer_mutex);
		return -1;
	}

	ret = __mozart_musicplayer_play_music_info(handler, info);
	musicplayer_unlock(&musicplayer_mutex);

	return ret;
}

static int __mozart_musicplayer_next_music(musicplayer_handler_t *handler, bool force)
{	printf("-------------__mozart_musicplayer_next_music------------\n");
	struct music_info *info = NULL;

	info = mozart_musiclist_set_next(musicplayer_list, force);
	if (info == NULL)
		return -1;

	return __mozart_musicplayer_play_music_info(handler, info);
}

/* for stop */
int mozart_musicplayer_next_music_false(musicplayer_handler_t *handler)
{	printf("------------mozart_musicplayer_next_music_false-----------------\n");
	int ret;

	musicplayer_lock(&musicplayer_mutex);
	if (check_handler_uuid(handler) < 0) {
		pr_err("check handler uuid failed\n");
		musicplayer_unlock(&musicplayer_mutex);
		return -1;
	}

	ret = __mozart_musicplayer_next_music(handler, false);
	musicplayer_unlock(&musicplayer_mutex);

	return ret;
}

int mozart_musicplayer_next_music(musicplayer_handler_t *handler)
{
	int ret;

	musicplayer_lock(&musicplayer_mutex);
	if (check_handler_uuid(handler) < 0) {
		pr_err("check handler uuid failed\n");
		musicplayer_unlock(&musicplayer_mutex);
		return -1;
	}

	ret = __mozart_musicplayer_next_music(handler, true);
	musicplayer_unlock(&musicplayer_mutex);

	return ret;
}

/**********************************************************************************************
 * music player handler
 *********************************************************************************************/
int mozart_musicplayer_get_duration(musicplayer_handler_t *handler)
{
	int ret;

	musicplayer_lock(&musicplayer_mutex);
	if (check_handler_uuid(handler) < 0) {
		musicplayer_unlock(&musicplayer_mutex);
		return PLAYER_PAUSED;
	}

	ret = mozart_player_getduration(handler->player_handler);
	musicplayer_unlock(&musicplayer_mutex);

	return ret;
}

static int __mozart_musicplayer_get_pos(musicplayer_handler_t *handler)
{
	return mozart_player_getpos(handler->player_handler);
}

int mozart_musicplayer_get_pos(musicplayer_handler_t *handler)
{
	int ret;

	musicplayer_lock(&musicplayer_mutex);
	if (check_handler_uuid(handler) < 0) {
		musicplayer_unlock(&musicplayer_mutex);
		return PLAYER_PAUSED;
	}

	ret = __mozart_musicplayer_get_pos(handler);
	musicplayer_unlock(&musicplayer_mutex);

	return ret;
}

int mozart_musicplayer_set_seek(musicplayer_handler_t *handler, int seek)
{
	int ret;

	musicplayer_lock(&musicplayer_mutex);
	if (check_handler_uuid(handler) < 0) {
		musicplayer_unlock(&musicplayer_mutex);
		return PLAYER_PAUSED;
	}

	ret = mozart_player_seek(handler->player_handler, seek);
	musicplayer_unlock(&musicplayer_mutex);

	if (ret == 0)
		mozart_ingenicplayer_notify_pos();

	return ret;
}

player_status_t mozart_musicplayer_get_status(musicplayer_handler_t *handler)
{
	player_status_t status;

	musicplayer_lock(&musicplayer_mutex);
	if (check_handler_uuid(handler) < 0) {
		musicplayer_unlock(&musicplayer_mutex);
		return PLAYER_PAUSED;
	}

	status = mozart_player_getstatus(handler->player_handler);
	musicplayer_unlock(&musicplayer_mutex);

	return status;
}

static int __mozart_musicplayer_get_volume(musicplayer_handler_t *handler)
{
	return mozart_player_volget(handler->player_handler);
}

int mozart_musicplayer_get_volume(musicplayer_handler_t *handler)
{
	int ret;

	musicplayer_lock(&musicplayer_mutex);
	if (check_handler_uuid(handler) < 0) {
		musicplayer_unlock(&musicplayer_mutex);
		return 0;
	}

	ret = __mozart_musicplayer_get_volume(handler);
	musicplayer_unlock(&musicplayer_mutex);

	return ret;
}

static int __mozart_musicplayer_set_volume(musicplayer_handler_t *handler, int volume)
{
	int ret;

	ret = mozart_player_volset(handler->player_handler, volume);

	if (ret == 0)
		mozart_ingenicplayer_notify_volume(__mozart_musicplayer_get_volume(handler));

	return ret;
}

int mozart_musicplayer_set_volume(musicplayer_handler_t *handler, int volume)
{
	int ret;

	musicplayer_lock(&musicplayer_mutex);
	if (check_handler_uuid(handler) < 0) {
		musicplayer_unlock(&musicplayer_mutex);
		return -1;
	}

	ret = __mozart_musicplayer_set_volume(handler, volume);
	musicplayer_unlock(&musicplayer_mutex);

	return ret;
}

int mozart_musicplayer_volume_up(musicplayer_handler_t *handler)
{	printf("########################mozart_musicplayer_volume_up###############################\n");
	int ret, volume;

	musicplayer_lock(&musicplayer_mutex);
	if (check_handler_uuid(handler) < 0) {
		musicplayer_unlock(&musicplayer_mutex);
		return -1;
	}

	volume = __mozart_musicplayer_get_volume(handler) + VOLUME_VARIATION;
	if (volume > 100)
		volume = 100;
	else if (volume < 0)
		volume = 0;
	ret = __mozart_musicplayer_set_volume(handler, volume);
	musicplayer_unlock(&musicplayer_mutex);

	return ret;
}

int mozart_musicplayer_volume_down(musicplayer_handler_t *handler)
{	printf("########################mozart_musicplayer_volume_down###############################\n");
	int ret, volume;

	musicplayer_lock(&musicplayer_mutex);
	if (check_handler_uuid(handler) < 0) {
		musicplayer_unlock(&musicplayer_mutex);
		return -1;
	}

	volume = __mozart_musicplayer_get_volume(handler) - VOLUME_VARIATION;
	if (volume > 100)
		volume = 100;
	else if (volume < 0)
		volume = 0;
	ret = __mozart_musicplayer_set_volume(handler, volume);
	musicplayer_unlock(&musicplayer_mutex);

	return ret;
}

 extern int wake_flage ;
 extern int wakebefore_status ;

int mozart_musicplayer_play_pause(musicplayer_handler_t *handler)
{	
	player_status_t status;
	usleep(200*1000);

	status = mozart_player_getstatus(handler->player_handler);
	printf("#######mozart_musicplayer_play_pause################## status = %d\n",status);
	int ret = -1;
	
	musicplayer_lock(&musicplayer_mutex);
	if (check_handler_uuid(handler) < 0) {
		musicplayer_unlock(&musicplayer_mutex);
		return PLAYER_PAUSED;
	}
	printf("####wake_flage = %d,wakebefore_status = %d\n",wake_flage,wakebefore_status);
	if(wake_flage == 0){
		if (status == PLAYER_PLAYING) {
			ret = mozart_player_pause(handler->player_handler);
			snprintf(mozart_musicplayer_event.event.misc.type, 16, "pause");
		} else if (status == PLAYER_PAUSED) {
			usleep(100*1000);
			mozart_player_resume(handler->player_handler);
			snprintf(mozart_musicplayer_event.event.misc.type, 16, "resume");
		}
	}else{
		if (wakebefore_status== PLAYER_PAUSED) {
			ret = mozart_player_pause(handler->player_handler);
			snprintf(mozart_musicplayer_event.event.misc.type, 16, "pause");
		} else if (wakebefore_status == PLAYER_PLAYING) {
			usleep(100*1000);
			mozart_player_resume(handler->player_handler);
			snprintf(mozart_musicplayer_event.event.misc.type, 16, "resume");
		}
		wake_flage = 0;
		wakebefore_status = 0;
	}
	
	if (mozart_event_send(mozart_musicplayer_event) < 0)
		pr_err("%s: Send event fail\n", __func__);

	musicplayer_unlock(&musicplayer_mutex);
	
	return ret;
}

int mozart_musicplayer_stop_playback(musicplayer_handler_t *handler)
{	printf("------------mozart_musicplayer_stop_playback-----------\n");
	autoplay_flag = false;
	if (mozart_player_stop(handler->player_handler)) {
		pr_err("send stop cmd failed\n");
		return -1;
	}

	if (AUDIO_OSS == get_audio_type()) {
		int cnt = 500;
		while (cnt--) {
			if (musicplayer_status == PLAYER_STOPPED)
				break;
			usleep(10 * 1000);
		}
		if (cnt < 0) {
			pr_err("waiting for stopped status timeout.\n");
			return -1;
		}
	}
	if (share_mem_set(MUSICPLAYER_DOMAIN, STATUS_RUNNING))
		pr_err("share_mem_set failure.\n");

	return 0;
}
////mozart_next_song();
pthread_t time_alarm_pthread;
int timeout_cnt = 0;

void *time_alarm_func(void *args)
{
	int cnt = *(int *)args;
	printf("---->>>[time_alarm_func]<<----------cnt = %d\n",cnt);	
	int time = 0;
	if(snd_source == SND_SRC_LOCALPLAY||snd_source == SND_SRC_NONE){
		g_alarm_cnt = 0;
		led_music_playing();
		return NULL;
	}
	if(luobo_fanyi_flag){
		printf("luobo_fanyi_flag = 1\n");
		g_alarm_cnt = 0;
		return 0;
	}
	if(cnt == 1&&strlen(g_tmp)){
		char *tmp = g_tmp;
		char *result = NULL;
		
		mozart_module_pause();
		json_object *reply_object = NULL;
		json_object *songs = NULL;
		json_object *songs_object = NULL;
		reply_object = json_object_new_object();
		songs = json_object_new_array();
		songs_object = json_object_new_object();
		
		json_object_object_add(reply_object, "command", json_object_new_string("add_queue_to_tail"));
		json_object_object_add(reply_object, "timestamp", json_object_new_int(12345));
		result = strsep(&tmp,"\n");
		if(result)
			json_object_object_add(songs_object, "songurl", json_object_new_string(result));
		result = NULL;
		result = strsep(&tmp,"\n");
		if(result)
			json_object_object_add(songs_object, "song_name", json_object_new_string(result));
		result = NULL;
		result = strsep(&tmp,"\n");
		if(result)
			json_object_object_add(songs_object, "artists_name", json_object_new_string(result));
		result = NULL;
		result = strsep(&tmp,"\n");
		printf("tmp = %s,result = %s\n",tmp,result);
		if(result)
			json_object_object_add(songs_object, "picture", json_object_new_string(result));
		json_object_object_add(songs_object, "duration", json_object_new_int(300));
		json_object_object_add(songs_object, "song_id", json_object_new_int(-1));
		json_object_array_add(songs, songs_object);
		json_object_object_add(reply_object, "songs", songs);
		char *send_json = strdup(json_object_get_string(reply_object));

		ingenicplayer_add_queue(send_json);
		return NULL;
	}

	player_status_t status;
	status = mozart_player_getstatus(mozart_musicplayer_handler->player_handler);	
	my_pr("time_alarm_func status = %d\n",status);
	while(status != PLAYER_PLAYING){//status != PLAYER_PLAYING
		my_pr("time = %d , mozart_vr_get_status = %d\n",time++,mozart_vr_get_status());
		if((snd_source != SND_SRC_CLOUD)&&(snd_source != SND_SRC_INGENICPLAYER))
			return NULL;
		if(time >= 15){
			printf("--------timeout-------\n");
			timeout_cnt++;
			if(timeout_cnt >= 3){
				timeout_cnt = 0;
				mozart_play_key_sync("badnet");
				mozart_tts_wait();
				mozart_next_song();
				
			}else{
				mozart_play_key_sync("next");
				mozart_tts_wait();
				mozart_next_song();
			}
				
			return NULL;
		}
		sleep(1);
		status = mozart_player_getstatus(mozart_musicplayer_handler->player_handler);		
	}
	timeout_cnt = 0;
	if(time < 15){
		g_alarm_cnt = 0; //
		while(!g_playing_flag){
			usleep(500*1000);
		}
		g_playing_flag = 0;
		led_music_playing();
	}
	return NULL;
}

int create_alarm_pthread(int cnt)
{	
	printf("----------create_alarm_pthread-------------cnt = %d\n",cnt);
	g_alarm_cnt = cnt;
	if (pthread_create(&time_alarm_pthread, NULL, time_alarm_func, (void *)&g_alarm_cnt) == -1) {
		printf("create time_alarm_pthread failed\n");
		return -1;
	}
	pthread_detach(time_alarm_pthread);

	return 0;
}

static int __mozart_musicplayer_play_music_info(musicplayer_handler_t *handler,	struct music_info *info)
{
	my_pr("-------------__mozart_musicplayer_play_music_info------------snd_source = %d\n",snd_source);
	my_pr("mozart_vr_get_status = %d\n",mozart_vr_get_status());
	int i, ret;
	char r = '\n';
	if(in_musicplayer) {
		my_pr(" in_musicplayer = %d\n",in_musicplayer);
		my_pr(" info->music_url = %s\n",info->music_url);
		my_pr(" info->music_name = %s\n",info->music_name);
		my_pr(" info->artists_name = %s\n",info->artists_name);
		my_pr(" info->music_picture = %s\n",info->music_picture);
		if(snd_source == SND_SRC_CLOUD || snd_source == SND_SRC_INGENICPLAYER){  //语音播放模式才去保存
			FILE *fp = NULL;
			fp = fopen("/usr/data/pre_url.txt","wb");
			if(fp){
				if(info->music_url){
					fwrite(info->music_url,1,strlen(info->music_url),fp);
					fwrite(&r,1,1,fp);
				}
				if(info->music_name){
					fwrite(info->music_name,1,strlen(info->music_name),fp);
					fwrite(&r,1,1,fp);
				}
				if(info->artists_name){
					fwrite(info->artists_name,1,strlen(info->artists_name),fp);
					fwrite(&r,1,1,fp);
				}
				if(info->music_picture){
					fwrite(info->artists_name,1,strlen(info->music_picture),fp);
					fwrite(&r,1,1,fp);
				}
				

			}
			fclose(fp);
			fp = NULL;
		}
		ret = mozart_player_playurl(handler->player_handler, info->music_url);
		if (ret) {
			pr_err("player play url failed\n");
			return -1;
		}
		//create_alarm_pthread(2);
	} else {
		my_pr(" in_musicplayer = %d\n",in_musicplayer);
		//查看pre_url文件中是否有上次的播放地址
		FILE *fp = NULL;
		memset(g_tmp,0,sizeof(g_tmp));
		if(access("/usr/data/pre_url.txt",F_OK) == 0)
		{	
			
			fp = fopen("/usr/data/pre_url.txt","rb");
			if(fread(g_tmp,1,sizeof(g_tmp),fp))
				printf("g_tmp = %s\n",g_tmp);
			
			fclose(fp);
	
		}
		memory_domain domain;
	 	ret = share_mem_get_active_domain(&domain);


		if (ret) {
			printf("get active domain error in %s:%s:%d, do nothing.\n",
			       __FILE__, __func__, __LINE__);
		
		}
		__attribute__((unused)) struct music_info *dump;
		module_status domain_status;

		share_mem_set(MUSICPLAYER_DOMAIN, WAIT_RESPONSE);
		snprintf(mozart_musicplayer_event.event.misc.type,
				sizeof(mozart_musicplayer_event.event.misc.type), "play");
		if (mozart_event_send(mozart_musicplayer_event))
			pr_err("send musicplayer play event failure\n");

		/* wait RENDER_DOMAIN become to RESPONSE_DONE status for 10 seconds. */
		for (i = 0; i < 1000; i++) {
			ret = share_mem_get(MUSICPLAYER_DOMAIN, &domain_status);
			pr_debug("domain_status: %s.\n", module_status_str[domain_status]);
			if (!ret)
				if (domain_status == RESPONSE_DONE ||
						domain_status == RESPONSE_PAUSE ||
						domain_status == RESPONSE_CANCEL)
					break;

			usleep(10 * 1000);
		}

		if (i == 1000) {
			pr_err("Wait musicplayer play event reponse done timeout, will not play music");
			return -1;
		}

		my_pr("========================================================================\n");

		for (i = 0; i < mozart_musiclist_get_length(musicplayer_list); i++)
		{
			dump = mozart_musiclist_get_index(musicplayer_list, i);
			my_pr("%d: %s\n", i, dump->music_name);
		}
		my_pr("========================================================================domain_status = %d\n",domain_status);
		switch (domain_status) {
			case RESPONSE_DONE:
				if (mozart_player_playurl(handler->player_handler, info->music_url)) {
					pr_err("player play url failed\n");
					return -1;
				}
				//create_alarm_pthread(1);
				break;
			case RESPONSE_PAUSE:
				if (mozart_player_cacheurl(handler->player_handler, info->music_url)) {
					pr_err("player play url failed\n");
					return -1;
				}
				break;
			case RESPONSE_CANCEL:
			default:
				pr_err("musicplayer response cancel\n");
				return -1;
		}
		in_musicplayer = 1;
	}

	autoplay_flag = true;

	snprintf(mozart_musicplayer_event.event.misc.type, 16, "playing");
	if (mozart_event_send(mozart_musicplayer_event) < 0)
		pr_err("%s: Send event fail\n", __func__);
	return 0;
}

int mozart_musicplayer_play_music_info(musicplayer_handler_t *handler, struct music_info *info)
{	printf("-------------mozart_musicplayer_play_music_info-----------------\n");
	int ret;

	musicplayer_lock(&musicplayer_mutex);
	if (check_handler_uuid(handler) < 0) {
		pr_err("check handler uuid failed\n");
		musicplayer_unlock(&musicplayer_mutex);
		return -1;
	}

	ret = __mozart_musicplayer_play_music_info(handler, info);
	musicplayer_unlock(&musicplayer_mutex);

	return ret;
}

static int musicplayer_status_change_callback(player_snapshot_t *snapshot,
					      struct player_handler *handler, void *param)
{
	musicplayer_handler_t *musicplayer_handler = (musicplayer_handler_t *)param;

	if (strcmp(handler->uuid, snapshot->uuid)) {
		pr_debug("%s can't control, controler is %s\n", handler->uuid, snapshot->uuid);
		if (share_mem_set(MUSICPLAYER_DOMAIN, STATUS_RUNNING))
			pr_err("share_mem_set failure.\n");
		musicplayer_status = PLAYER_STOPPED;
		return 0;
	}

	musicplayer_status = snapshot->status;

	pr_debug("mozart musicplayer recv player status: %d, %s.\n",
		 snapshot->status, player_status_str[snapshot->status]);

	switch (snapshot->status) {
	case PLAYER_TRANSPORT:
	case PLAYER_PLAYING:
		if (share_mem_set(MUSICPLAYER_DOMAIN, STATUS_PLAYING))
			pr_err("share_mem_set failure.\n");
		mozart_ingenicplayer_notify_song_info();
		break;
	case PLAYER_PAUSED:
		if (share_mem_set(MUSICPLAYER_DOMAIN, STATUS_PAUSE))
			pr_err("share_mem_set failure.\n");
		mozart_ingenicplayer_notify_song_info();
		break;
	case PLAYER_STOPPED:
		if (share_mem_set(MUSICPLAYER_DOMAIN, STATUS_RUNNING))
			pr_err("share_mem_set failure.\n");
		if (autoplay_flag)
			mozart_musicplayer_next_music_false(musicplayer_handler);
		else
			mozart_ingenicplayer_notify_song_info();
		break;
	case PLAYER_UNKNOWN:
		musicplayer_status = PLAYER_STOPPED;
		if (share_mem_set(MUSICPLAYER_DOMAIN, STATUS_RUNNING))
			pr_err("share_mem_set failure.\n");
		break;
	default:
		musicplayer_status = PLAYER_STOPPED;
		pr_err("Unsupported status: %d.\n", snapshot->status);
		break;
	}
	return 0;
}

int mozart_musicplayer_start(musicplayer_handler_t *handler)
{	printf("########################mozart_musicplayer_start###############################\n");
	if (handler == NULL || handler->player_handler == NULL) {
		pr_err("handler is null\n");
		return -1;
	}

	musicplayer_lock(&musicplayer_mutex);

	if (strcmp(current_player_handler.uuid, "invalid")) {
		pr_err("current handler isn't invalid\n");
		musicplayer_unlock(&musicplayer_mutex);
		return -1;
	}

	memcpy(current_player_handler.uuid, handler->player_handler->uuid,
	       sizeof(current_player_handler.uuid));
	pr_debug("current is %s\n", current_player_handler.uuid);

	musicplayer_unlock(&musicplayer_mutex);

	return 0;
}

int mozart_musicplayer_stop(musicplayer_handler_t *handler)
{	printf("########################mozart_musicplayer_stop###############################\n");
	musicplayer_lock(&musicplayer_mutex);
	in_musicplayer = 0;
	if (!strcmp(current_player_handler.uuid, "invalid")) {
		musicplayer_unlock(&musicplayer_mutex);
		return 0;
	}
	if (strcmp(current_player_handler.uuid, handler->player_handler->uuid)) {
		musicplayer_unlock(&musicplayer_mutex);
		return -1;
	}

	pr_debug("stop %s\n", current_player_handler.uuid);
	if (mozart_player_getstatus(handler->player_handler) != PLAYER_STOPPED)
		mozart_musicplayer_stop_playback(handler);
	mozart_musiclist_clean(musicplayer_list);
	strcpy(current_player_handler.uuid, "invalid");
	musicplayer_unlock(&musicplayer_mutex);

	return 0;
}

int mozart_musicplayer_startup(void)
{	printf("########################mozart_musicplayer_startup###############################\n");
	pr_debug("...\n");

	musicplayer_lock(&musicplayer_mutex);
	if (mozart_musicplayer_handler) {
		pr_info("musicplayer is running\n");
		musicplayer_unlock(&musicplayer_mutex);
		return 0;
	}

	musicplayer_list = mozart_musiclist_create(free_data);

	if (share_mem_init() != 0)
		pr_err("share_mem_init failed\n");

	if (share_mem_set(MUSICPLAYER_DOMAIN, STATUS_RUNNING))
		pr_err("share_mem_set MUSICPLAYER_DOMAIN failed\n");

	mozart_musicplayer_handler = calloc(sizeof(*mozart_musicplayer_handler), 1);
	if (mozart_musicplayer_handler == NULL) {
		pr_err("calloc musicplayer handler failed\n");
		musicplayer_unlock(&musicplayer_mutex);
		return -1;
	}
	mozart_musicplayer_handler->player_handler = mozart_player_handler_get(
		"musicplayer", musicplayer_status_change_callback, mozart_musicplayer_handler);
	if (mozart_musicplayer_handler->player_handler == NULL) {
		pr_err("get player handler failed\n");
		free(mozart_musicplayer_handler);
		mozart_musicplayer_handler = NULL;
		musicplayer_unlock(&musicplayer_mutex);
		return -1;
	}

	strcpy(current_player_handler.uuid, "invalid");
	musicplayer_unlock(&musicplayer_mutex);

	return 0;
}

int mozart_musicplayer_shutdown(void)
{	printf("########################mozart_musicplayer_shutdown###############################\n");
	pr_debug("...\n");

	musicplayer_lock(&musicplayer_mutex);
	if (mozart_musicplayer_handler == NULL) {
		pr_info("ingenic player not running\n");
		musicplayer_unlock(&musicplayer_mutex);
		return 0;
	}

	if (check_handler_uuid(mozart_musicplayer_handler) == 0 &&
	    mozart_player_getstatus(mozart_musicplayer_handler->player_handler) != PLAYER_STOPPED)
		mozart_musicplayer_stop_playback(mozart_musicplayer_handler);
	strcpy(current_player_handler.uuid, "invalid");

	mozart_player_handler_put(mozart_musicplayer_handler->player_handler);
	memset(mozart_musicplayer_handler, 0, sizeof(*mozart_musicplayer_handler));
	free(mozart_musicplayer_handler);
	mozart_musicplayer_handler = NULL;

	mozart_musiclist_destory(musicplayer_list);

	if (share_mem_set(MUSICPLAYER_DOMAIN, STATUS_SHUTDOWN))
		pr_err("share_mem_set failed\n");

	musicplayer_unlock(&musicplayer_mutex);

	return 0;
}
