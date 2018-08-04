#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <pthread.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <sys/types.h>
#include <dirent.h>


#include <signal.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>


#include "json-c/json.h"
#include "utils_interface.h"
#include "linklist_interface.h"
#include "localplayer_interface.h"

#include "mozart_config.h"
#include "ingenicplayer.h"
#include "mozart_musicplayer.h"
#include "mozart_key_function.h"
#include "nvrw_interface.h"

/* #define MOZART_INGENICPLAYER_DEBUG */
#ifdef MOZART_INGENICPLAYER_DEBUG
#define pr_debug(fmt, args...)				\
	printf("[INGENICPLAYER] %s %d: "fmt, __func__, __LINE__, ##args)
#else  /* MOZART_INGENICPLAYER_DEBUG */
#define pr_debug(fmt, args...)			\
	do {} while (0)
#endif	/* MOZART_INGENICPLAYER_DEBUG */

#define pr_info(fmt, args...)						\
	printf("[INGENICPLAYER] [INFO] "fmt, ##args)

#define pr_err(fmt, args...)						\
	fprintf(stderr, "[INGENICPLAYER] [ERROR] %s %d: "fmt, __func__, __LINE__, ##args)

#define DEVICENAME "/usr/data/ingenicplayer/devicename"
#define DEVICE_INGENICPLAYER_DIR "/usr/data/ingenicplayer/"

enum {
	INGENICPLAYER_PLAY_PAUSE = 0,
	INGENICPLAYER_PREV,
	INGENICPLAYER_NEXT,
	INGENICPLAYER_PLAY_MODE,
	INGENICPLAYER_INC_VOLUME,
	INGENICPLAYER_DEC_VOLUME,
	INGENICPLAYER_SET_VOLUME,
	INGENICPLAYER_GET_VOLUME,
	INGENICPLAYER_GET_SONG_INFO,
	INGENICPLAYER_GET_SONG_QUEUE,
	INGENICPLAYER_ADD_QUEUE,
	INGENICPLAYER_REPLACE_QUEUE,
	INGENICPLAYER_DEL_QUEUE,
	INGENICPLAYER_PLAY_QUEUE,
	INGENICPLAYER_GET_DEVICE_INFO,
	INGENICPLAYER_HEART_PULSATE,
	INGENICPLAYER_SET_SEEK,
	INGENICPLAYER_GET_POSITION,
	INGENICPLAYER_GET_SD_MUSIC,
	INGENICPLAYER_SET_SHORTCUT,
	INGENICPLAYER_GET_SHORTCUT,
	INGENICPLAYER_PLAY_SHORTCUT,
	INGENICPLAYER_ADD_QUEUE_TO_TAIL,
	INGENICPLAYER_UPGRADE,
	INGENICPLAYER_RS_READ,
	INGENICPLAYER_RS_WRITE,
	INGENICPLAYER_RS_WRITE_FORCE,
	INGENICPLAYER_RS_LED_RGB,
	INGENICPLAYER_CLEAR_WPACFG,
};

char *ingenicplayer_cmd[] = {
	[INGENICPLAYER_PLAY_PAUSE] = "toggle", /* play/pause */
	[INGENICPLAYER_PREV] = "prefix", /* previous */
	[INGENICPLAYER_NEXT] = "skip", /* next */
	[INGENICPLAYER_PLAY_MODE] = "set_queue_mode", /* play mode */
	[INGENICPLAYER_INC_VOLUME] = "increase_volume",
	[INGENICPLAYER_DEC_VOLUME] = "decrease_volume",
	[INGENICPLAYER_SET_VOLUME] = "set_volume",
	[INGENICPLAYER_GET_VOLUME] = "get_volume",
	[INGENICPLAYER_GET_SONG_INFO] = "get_song_info",
	[INGENICPLAYER_GET_SONG_QUEUE] = "get_song_queue",
	[INGENICPLAYER_ADD_QUEUE] = "add_queue_to_play",
	[INGENICPLAYER_REPLACE_QUEUE] = "replace_queue",
	[INGENICPLAYER_DEL_QUEUE] = "delete_from_queue",
	[INGENICPLAYER_PLAY_QUEUE] = "play_from_queue",
	[INGENICPLAYER_GET_DEVICE_INFO] = "get_device_info",
	[INGENICPLAYER_HEART_PULSATE] = "heart_pulsate",
	[INGENICPLAYER_SET_SEEK] = "set_seek",
	[INGENICPLAYER_GET_POSITION] = "get_position",
	[INGENICPLAYER_GET_SD_MUSIC] = "get_sd_music",
	[INGENICPLAYER_SET_SHORTCUT] = "set_shortcut",
	[INGENICPLAYER_GET_SHORTCUT] = "get_shortcut",
	[INGENICPLAYER_PLAY_SHORTCUT] = "play_shortcut",
	[INGENICPLAYER_ADD_QUEUE_TO_TAIL] = "add_queue_to_tail",
	[INGENICPLAYER_UPGRADE] = "upgrade",
	[INGENICPLAYER_RS_READ] = "rs_read",
	[INGENICPLAYER_RS_WRITE] = "rs_write",
	[INGENICPLAYER_RS_WRITE_FORCE] = "rs_write_force",
	[INGENICPLAYER_RS_LED_RGB] = "rs_led_rgb",
	[INGENICPLAYER_CLEAR_WPACFG] = "clear_wpacfg",
};

struct cmd_info_c {
	struct appserver_cli_info *owner;
	char *command;
	char *data;
};

static List cmd_list;
static bool die_out = false;
static pthread_t ingenicplayer_thread;
static pthread_mutex_t cmd_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t cond_var = PTHREAD_COND_INITIALIZER;

typedef struct voice  
{  
	int num;
	char url[128];  
	struct voice  *next;  
} ST_VOICE;  

ST_VOICE *voice_head = NULL;
ST_VOICE *voice_tmp = NULL;
ST_VOICE *voice_keep = NULL;
static int voice_cnt = 0;
static int sing_flag = 0;



/* TODO: snd_source need lock */
static int mozart_module_ingenicplayer_start(void)
{	printf("------------------mozart_module_ingenicplayer_start--------------\n");
	if (snd_source != SND_SRC_INGENICPLAYER) {
		if (mozart_module_stop())
			return -1;
		if (mozart_musicplayer_start(mozart_musicplayer_handler))
			return -1;
		snd_source = SND_SRC_INGENICPLAYER;
	} else if (!mozart_musicplayer_is_active(mozart_musicplayer_handler)) {
		if (mozart_musicplayer_start(mozart_musicplayer_handler))
			return -1;
	} else {
		mozart_musicplayer_musiclist_clean(mozart_musicplayer_handler);
	}

	return 0;
}

static void free_cmd_list(void *cmd_list_info)
{
	struct cmd_info_c *cmd_info = (struct cmd_info_c *)cmd_list_info;
	if (cmd_info) {
		if (cmd_info->owner) {
			free(cmd_info->owner);
			cmd_info->owner = NULL;
		}
		if (cmd_info->command) {
			free(cmd_info->command);
			cmd_info->command = NULL;
		}
		if (cmd_info->data) {
			free(cmd_info->data);
			cmd_info->data = NULL;
		}
		free(cmd_info);
	}
}

static int get_object_int(char *object, char *cmd_json)
{
	int ret = -1;
	json_object *cmd_object = NULL;
	json_object *tmp = NULL;

	cmd_object = json_tokener_parse(cmd_json);
	if (cmd_object) {
		if (json_object_object_get_ex(cmd_object, object, &tmp))
			ret = json_object_get_int(tmp);
	}
	json_object_put(cmd_object);
	return ret;
}

static char *get_object_string(char *object, char *content, char *cmd_json)
{
	char *content_object = NULL;
	json_object *cmd_object = NULL;
	json_object *tmp = NULL;

	cmd_object = json_tokener_parse(cmd_json);
	if (cmd_object) {
		if (json_object_object_get_ex(cmd_object, object, &tmp)) {
			if (content)
				json_object_object_get_ex(tmp, content, &tmp);
			content_object = strdup(json_object_get_string(tmp));
		}
	}
	json_object_put(cmd_object);
	return content_object;
}

static char *sync_devicename(char *newname)
{
	int namelen = 0;
	char *devicename = NULL;
	FILE *fd = NULL;

	if ((fd = fopen(DEVICENAME, "a+")) == NULL) {
		pr_err("fopen failed %s\n", strerror(errno));
		return NULL;
	}
	if (newname) {
		truncate(DEVICENAME, 0);
		fwrite(newname, sizeof(char), strlen(newname), fd);
	} else {
		fseek(fd, 0, SEEK_END);
		namelen = ftell(fd);
		fseek(fd, 0, SEEK_SET);
		if (namelen) {
			devicename = (char *)calloc(namelen, sizeof(char) + 1);
			if (devicename) {
				fread(devicename, sizeof(char), namelen, fd);
				pr_debug("get devicename %s\n", devicename);
			}
		}
	}
	fclose(fd);

	return devicename;
}

static int ingenicplayer_play_pause(void)
{
	mozart_play_pause();
	//return mozart_musicplayer_play_pause(mozart_musicplayer_handler);
}

static void get_song_json_object(json_object *object, struct music_info *info)
{
	json_object *reply_object = object;

	if (object == NULL || info == NULL)
		return ;

	if (info->id)
		json_object_object_add(reply_object, "song_id",
				json_object_new_int(info->id));
	if (info->music_name)
		json_object_object_add(reply_object, "song_name",
				json_object_new_string(info->music_name));
	if (info->music_url)
		json_object_object_add(reply_object, "songurl",
				json_object_new_string(info->music_url));
	if (info->music_picture)
		json_object_object_add(reply_object, "picture",
				json_object_new_string(info->music_picture));
	if (info->albums_name)
		json_object_object_add(reply_object, "albumsname",
				json_object_new_string(info->albums_name));
	if (info->artists_name)
		json_object_object_add(reply_object, "artists_name",
				json_object_new_string(info->artists_name));
	if (info->data)
		json_object_object_add(reply_object, "data",
				json_object_new_string(info->data));
}

static void get_player_status(json_object *object)
{
	json_object *reply_object = object;
	player_status_t status = mozart_musicplayer_get_status(mozart_musicplayer_handler);

	if (status == PLAYER_PLAYING)
		json_object_object_add(reply_object, "status", json_object_new_string("play"));
	else if (status == PLAYER_PAUSED || status == PLAYER_TRANSPORT)
		json_object_object_add(reply_object, "status", json_object_new_string("pause"));
	else if (status == PLAYER_STOPPED)
		json_object_object_add(reply_object, "status", json_object_new_string("stop"));
}

char *ingenicplayer_get_song_info(char *command,char *cur_time)
{	
	int index = -1;
	int length = -1;
	int duration = 0;
	enum play_mode mode = -1;
	char *reply_json = NULL;
	json_object *reply_object;
	struct music_info *info = NULL;

	reply_object = json_object_new_object();
	if (!reply_object)
		return NULL;

	get_player_status(reply_object);

	mode = mozart_musicplayer_musiclist_get_play_mode(mozart_musicplayer_handler);
	length = mozart_musicplayer_musiclist_get_length(mozart_musicplayer_handler);
	index = mozart_musicplayer_musiclist_get_current_index(mozart_musicplayer_handler);
	info = mozart_musicplayer_musiclist_get_current_info(mozart_musicplayer_handler);
	if (info) {
		get_song_json_object(reply_object, info);
		duration = mozart_musicplayer_get_duration(mozart_musicplayer_handler);
		json_object_object_add(reply_object, "get_duration", json_object_new_int(duration));
	}

	int pos = 0;
	pos = mozart_musicplayer_get_pos(mozart_musicplayer_handler);
	json_object_object_add(reply_object, "get_current_position", json_object_new_int(pos));
	
	json_object_object_add(reply_object, "queue_mode", json_object_new_int(mode));
	json_object_object_add(reply_object, "queue_index", json_object_new_int(index));
	json_object_object_add(reply_object, "queue_num", json_object_new_int(length));
	if(command){
		json_object_object_add(reply_object, "command", json_object_new_string(command));
	}
	if(cur_time)
		json_object_object_add(reply_object, "timestamp", json_object_new_string(cur_time));

	reply_json = strdup(json_object_get_string(reply_object));
	json_object_put(reply_object);

	return reply_json;
}

char *ingenicplayer_get_position(char *command,char *cur_time)
{
	int i = 0;
	int pos = 0;
	int duration = 0;
	char *reply_json = NULL;
	json_object *reply_object = json_object_new_object();
	if (!reply_object)
		return NULL;

	for (i = 0; i < 30; i++) {
		pos = mozart_musicplayer_get_pos(mozart_musicplayer_handler);
		if (pos != -1 && pos != 0)
			break;
		usleep(100000);
	}
	duration = mozart_musicplayer_get_duration(mozart_musicplayer_handler);

	json_object_object_add(reply_object, "get_current_position",
			json_object_new_int(pos));
	json_object_object_add(reply_object, "get_duration",
			json_object_new_int(duration));
	if(command)
		json_object_object_add(reply_object, "command", json_object_new_string(command));
	if(cur_time)
		json_object_object_add(reply_object, "timestamp", json_object_new_string(cur_time));
	reply_json = strdup(json_object_get_string(reply_object));
	json_object_put(reply_object);

	return reply_json;
}

int h5_play_mode(char *data)
{	printf("=======h5_play_mode=============\n");
	json_object *object = NULL;
	enum play_mode set_mode = -1;
	enum play_mode mode = -1;

	set_mode = get_object_int("value", data);
	mode = mozart_musicplayer_musiclist_get_play_mode(mozart_musicplayer_handler);
	printf("AAA set_mode = %d,mode = %d\n",set_mode,mode);


	if(set_mode != mode){
		printf("SET MODE ====\n");
		mozart_musicplayer_musiclist_set_play_mode(mozart_musicplayer_handler, mode);
	}
	mode = mozart_musicplayer_musiclist_get_play_mode(mozart_musicplayer_handler);
	printf("BBB set_mode = %d,mode = %d\n",set_mode,mode);

	return set_mode;

}

int ingenicplayer_play_mode(struct cmd_info_c *cmd_info)
{
	enum play_mode mode;

	mode = get_object_int("mode", cmd_info->data);
	mozart_musicplayer_musiclist_set_play_mode(mozart_musicplayer_handler, mode);

	mode = mozart_musicplayer_musiclist_get_play_mode(mozart_musicplayer_handler);
	mozart_ingenicplayer_notify_play_mode(mode);

	return 0;
}

char *ingenicplayer_get_volume(char *command,char *cur_time)
{	
	json_object *reply_object;
	char *reply_json = NULL;
	int volume = mozart_musicplayer_get_volume(mozart_musicplayer_handler);

	if (volume < 0)
		return NULL;

	reply_object = json_object_new_object();
	if (reply_object == NULL)
		return NULL;

	json_object_object_add(reply_object, "volume", json_object_new_int(volume));
	
	if(command){
		json_object_object_add(reply_object, "command", json_object_new_string(command));
	}
	if(cur_time){
		json_object_object_add(reply_object, "timestamp", json_object_new_string(cur_time));
	}
	reply_json = strdup(json_object_get_string(reply_object));
	json_object_put(reply_object);

	return reply_json;
}

char *h5_get_queue(void)
{
	int i = 0;
	int index = -1;
	int length = -1;
	char *reply_json = NULL;
	json_object *reply_object;
	json_object *songs;
	struct music_info *info = NULL;

	reply_object = json_object_new_object();
	if (!reply_object)
		goto object_err;
	songs = json_object_new_array();
	if (!songs)
		goto object_err;
	index = mozart_musicplayer_musiclist_get_current_index(mozart_musicplayer_handler);
	json_object_object_add(reply_object, "index", json_object_new_int(index));
	json_object_object_add(reply_object, "songs", songs);
	length = mozart_musicplayer_musiclist_get_length(mozart_musicplayer_handler);
	for (i = 0; i < length; i++) {
		json_object *songs_object = json_object_new_object();
		if (!songs_object)
			goto object_err;

		info = mozart_musicplayer_musiclist_get_index_info(mozart_musicplayer_handler, i);
		if (info != NULL) {
			get_song_json_object(songs_object, info);
			json_object_array_add(songs, songs_object);
		}
	}
	reply_json = (char *)calloc(strlen(json_object_get_string(reply_object))+ 1, sizeof(char));
	if (!reply_json) {
		pr_err("calloc %s\n", strerror(errno));
		goto object_err;
	}
	json_object_object_add(reply_object, "command", json_object_new_string("get_song_queue"));
	reply_json = strdup(json_object_get_string(reply_object));


	return reply_json;
object_err:
	json_object_put(reply_object);
	return -1;

}
void ingenicplayer_get_queue(char *command, struct appserver_cli_info *owner)
{
	int i = 0;
	int index = -1;
	int length = -1;
	char *reply_json = NULL;
	json_object *reply_object;
	json_object *songs;
	struct music_info *info = NULL;

	reply_object = json_object_new_object();
	if (!reply_object)
		goto object_err;
	songs = json_object_new_array();
	if (!songs)
		goto object_err;

	index = mozart_musicplayer_musiclist_get_current_index(mozart_musicplayer_handler);
	json_object_object_add(reply_object, "index", json_object_new_int(index));
	json_object_object_add(reply_object, "songs", songs);

	length = mozart_musicplayer_musiclist_get_length(mozart_musicplayer_handler);
	
	for (i = 0; i < length; i++) {
		json_object *songs_object = json_object_new_object();
		if (!songs_object)
			goto object_err;

		info = mozart_musicplayer_musiclist_get_index_info(mozart_musicplayer_handler, i);
		if (info != NULL) {
			get_song_json_object(songs_object, info);
			json_object_array_add(songs, songs_object);
		}
	}
	reply_json = (char *)calloc(strlen(json_object_get_string(reply_object))
			+ 1, sizeof(char));
	if (!reply_json) {
		pr_err("calloc %s\n", strerror(errno));
		goto object_err;
	}
	strcpy(reply_json, (const char *)json_object_get_string(reply_object));
	mozart_appserver_response(command, reply_json, owner);
	free(reply_json);

object_err:
	json_object_put(reply_object);
	return;
}

static struct music_info *create_music_info_by_json(json_object *object)
{
	int id = 0;
	char *music_name = NULL;
	char *music_url = NULL;
	char *music_picture = NULL;
	char *albums_name = NULL;
	char *artists_name = NULL;
	char *data = NULL;
	json_object *tmp = NULL;
	struct music_info *info = NULL;

	if (object == NULL)
		return NULL;

	if (json_object_object_get_ex(object, "song_id", &tmp))
		id = json_object_get_int(tmp);
	if (json_object_object_get_ex(object, "song_name", &tmp))
		music_name = strdup(json_object_get_string(tmp));
	if (json_object_object_get_ex(object, "songurl", &tmp))
		music_url = strdup(json_object_get_string(tmp));
	if (json_object_object_get_ex(object, "picture", &tmp))
		music_picture = strdup(json_object_get_string(tmp));
	if (json_object_object_get_ex(object, "albumsname", &tmp))
		albums_name = strdup(json_object_get_string(tmp));
	if (json_object_object_get_ex(object, "artists_name", &tmp))
		artists_name = strdup(json_object_get_string(tmp));
	if (json_object_object_get_ex(object, "data", &tmp))
		data = strdup(json_object_get_string(tmp));

	info = mozart_musiclist_get_info(id, music_name, music_url,
					 music_picture, albums_name, artists_name, data);

	free(music_name);
	free(music_url);
	free(music_picture);
	free(albums_name);
	free(artists_name);

	return info;
}

static int ingenicplayer_insert_song(json_object *array)
{	printf("---------------ingenicplayer_insert_song---------------\n");
	int index = -1;
	struct music_info *info = NULL;

	if (array == NULL)
		return index;

	info = create_music_info_by_json(array);
	printf("info->music_url = %s\n",info->music_url);
	if (info)
		index = mozart_musicplayer_musiclist_insert(mozart_musicplayer_handler, info);

	return index;
}

void *h5_musiclist_delete_index(char *data)
{
	int index = -1;
	json_object *object = NULL;

	object = json_tokener_parse(data);
	if (!object) {
		pr_err("json parse failed %s\n", strerror(errno));
		return NULL;
	}
	index = get_object_int("index", data);
	mozart_musicplayer_musiclist_delete_index(mozart_musicplayer_handler, index);
	return NULL;
}

#define SERVER_ADDR	"119.23.207.229" 
#define SERVER_PORT	82 
#define POSTto "api.rayshine.cc"
char upload_head[] =
	"POST /talk/voice/dev HTTP/1.1\r\n"
	"host: %s\r\n"
	"content-Type: multipart/form-data; boundary=%s\r\n"
	"content-Length: %d\r\n"
	"Connection: keep-alive\r\n\r\n"
;
char upload_duration[] = 
	"--%s\r\n"
	"content-Disposition: form-data; name=\"duration\"\r\n\r\n%d\r\n"
;

char upload_devid[] =
	"--%s\r\n"
	"content-Disposition: form-data; name=\"deviceId\"\r\n\r\n%s\r\n"
;
char upload_file[] =
	"--%s\r\n"
	"content-Disposition: form-data; name=\"file\"; filename=\"%s\"\r\n"
	"content-Type: audio/wav\r\n\r\n"
;
	
char upload_request[] = 
	"--%s"
	"content-Disposition: form-data; name=\"duration\"; \r\n\r\n%d\r\n"
	"\r\n--%s\r\n"
	"content-Disposition: form-data; name=\"deviceId\"\r\n\r\n%s\r\n"
	"--%s\r\n\r\n"
	"content-Disposition: form-data; name=\"file\"; filename=\"%s\"\r\n"
	"content-Type: audio/mp3\r\n\r\n"
	"--%s\r\n"
;

int Connect_Server(char *severip,int port)
{
	int sock = -1;
	struct sockaddr_in addr;
	
	if(severip == NULL || port <= 0)
		return -1;
		
	memset(&addr,0,sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	inet_pton(AF_INET,severip,&addr.sin_addr.s_addr);
	
	sock = socket(AF_INET,SOCK_STREAM,0);
	if(-1 == sock)
		return -1;

	if(connect(sock,(struct sockaddr*)&addr,sizeof(addr)) < 0)
	{
		close(sock);
		return -1;
	}
		
		return sock;	
	
}

typedef enum {
		/**
		 * @brief not in vr mode.
		 */
		VR_IDLE,
		/**
		 * @brief on aec mode
		 */
		VR_AEC,
		/**
		 * @brief on asr mode
		 */
		VR_ASR,
		/**
		 * @brief requesting content
		 */
		VR_CONTENT_GET,
	} vr_status_t;
int h5_post_msg()
{	
	printf("=========h5_post_msg============\n");
	
	char filename[256] = {0};
	int sock = -1;
	char send_date[4096] = {0};
	char send_end[128] = {0};
	char send_request[2048] = {0};
	FILE *fp = NULL;
	char *userid = "00000000199";
	char boundary[64] = {0};
	int ret = -1,already = -1,ContentLength = -1;
	long long int timestamp;
	struct timeval tv;
	int duration  = 5;
	int file_size = 0;

	char macaddr[] = "255.255.255.255";
	memset(macaddr, 0, sizeof(macaddr));
	get_mac_addr("wlan0", macaddr, "");

	char devid[32] = {0};
	sprintf(devid,"NS-%s",&macaddr[6]);
	
	strcpy(filename,"/tmp/speak_8.amr");

	printf("filename = %s\n",filename);
	if(access(filename,F_OK) != 0)
	{
		printf("file %s not exsit!\n",filename);
		return -1;
	}

	sock = Connect_Server(SERVER_ADDR,SERVER_PORT);
	if(sock < 0 )
	{
		printf("connect server error!\n");
		return -1;
	}

	fp = fopen(filename,"rb");
	if(fp == NULL)
	{
		printf("open file %s error!\n",filename);
		close(sock);
		return -1;
	}

	fseek(fp,0,SEEK_END);
	ContentLength = ftell(fp);
	file_size = ContentLength;
	rewind(fp);

	gettimeofday(&tv,NULL);
	timestamp = (long long int)tv.tv_sec * 1000 + tv.tv_usec;
	snprintf(boundary,64,"---------------------------%lld",timestamp);



	int len1 = 0,len2 = 0;
	len1 = snprintf(send_request,2048,upload_request,boundary,duration,boundary,devid,boundary,"speak_8.amr",boundary);
	len2 = snprintf(send_end,2048,"\r\n--%s--\r\n",boundary);
	ContentLength += len2 + len1;

	int send_cnt = 0;
	ret = snprintf(send_date,4096,upload_head,POSTto,boundary,ContentLength);
	send_cnt = send(sock,send_date,ret,0);

	
	if( send_cnt < ret)
	{
		printf("send head error!\n");
		close(sock);
		return -1;
	}
	
	fclose(fp);
// ------------duration
	memset(send_date,0,sizeof(send_date));
	ret = snprintf(send_date,4096,upload_duration,boundary,duration);
	send_cnt = send(sock,send_date,ret,0);


	if(send_cnt < ret)
	{
			close(sock);
			return -1;
	}
	printf("%s\n",send_date);
// ------------devid
	memset(send_date,0,sizeof(send_date));
	ret = snprintf(send_date,4096,upload_devid,boundary,devid);
	send_cnt = send(sock,send_date,ret,0);

	if(send_cnt < ret)
	{
			close(sock);
			return -1;
	}
	printf("%s\n",send_date);
// ------------file
	memset(send_date,0,sizeof(send_date));
	ret = snprintf(send_date,4096,upload_file,boundary,filename);
	send_cnt = send(sock,send_date,ret,0);

	if(send_cnt < ret)
	{
			close(sock);
			return -1;
	}
	printf("%s\n",send_date);


	fp = NULL;
	fp = fopen(filename,"rb");
	if(fp == NULL)
	{
		printf("open file %s error!\n",filename);
		close(sock);
		return -1;
	}

	while(1)
	{		
			memset(send_date,0,sizeof(send_date));
			ret = fread(send_date,1,4096,fp);
			sprintf(send_date,"%s%c%c",send_date,'\r','\n');
			printf("<%d>",ret);
			if(ret != 4096)
			{
					if(!ferror(fp))
					{
						int last_send_cnt = send(sock,send_date,ret,0);
						if(last_send_cnt != ret)
						{
								printf("send the end date error!\n");
								close(sock);
								fclose(fp);
								return -1;
						}
						memset(send_date,0,sizeof(send_date));

						fclose(fp);
						break;
					}
					else
					{
						printf("read file error!\n");
						close(sock);
						fclose(fp);
						return -1;
					}
						
			}
			
			if(send(sock,send_date,4096,0) != 4096)
			{
					printf("send date error\n");
					close(sock);
					fclose(fp);
					return -1;
			}
			
			
	}
	printf("\r\n\r\n");


#if 0
	send_cnt = 0;			
	memset(send_date,0,sizeof(send_date));
	send_cnt = snprintf(send_date,4096,upload_devid,devid);
	ret = send(sock,send_date,send_cnt,0);

	printf("<send:%d,send_cnt = %d>\n",ret,send_cnt);
	if(send_cnt != ret)
	{
		close(sock);
		return -1;
	}
	printf("%s\n",send_date);
#endif
//发送最后的boundary结束文件上传。
SEND_END:		
	memset(send_date,0,sizeof(send_date));
	ret = snprintf(send_date,4096,"\r\n--%s--\r\n",boundary);
	if(send(sock,send_date,ret,0) != ret)
	{
			close(sock);
			return -1;
	}
	
	printf("%s\n",send_date);
//接收返回值，用于判断是否上传成功
	printf("#########################################\n");
	memset(send_date,0,sizeof(send_date));
	if(recv(sock,send_date,4096,0) < 0)
		printf("recv error!\n");
		
	printf("recv:%s\n",send_date);
	close(sock);
	system("rm /tmp/speak_16.wav");
	system("rm /tmp/speak_8.amr");
	system("rm /tmp/speak_8.wav");
	return 0;

}
int calculation_file(char *file)//计算文件的时间先后
{
	char my_file[32] = {0};
	strncpy(my_file,file,8);
	char *tmp = NULL;
	char *result = NULL;
	tmp = my_file;
	result = strsep(&tmp,".");
	return atoi(result);
}
#if 1
void *check_msg()
{	printf("============check_msg=========\n");
	
	char cmd[256] = {0};
	DIR * dir;
	struct dirent * ptr;
	int first = 1;
	ST_VOICE *voice_current = NULL;
	ST_VOICE *voice_tail = NULL;
	ST_VOICE *voice_tmp = NULL;
	ST_VOICE *voice_tmp_next = NULL;
	ST_VOICE *voice_tmp_pre = NULL;
	dir = opendir("/usr/data/voice/");
	
	while((ptr = readdir(dir)) != NULL)
	{	
		printf("ptr->d_name = %s\n",ptr->d_name);
		if((strcmp(ptr->d_name,".")!=0)&&((strcmp(ptr->d_name,"..")!=0)))
		{	
			
			printf("-----check_msg ptr->d_name = %s\n",ptr->d_name);
			{
				voice_current = NULL;
				voice_current = (ST_VOICE *)calloc(1, sizeof(ST_VOICE));
				sprintf(voice_current->url,"%s",ptr->d_name);
				voice_current->num = calculation_file(ptr->d_name);
				voice_current->next = NULL;
				if(!voice_head){
					voice_head = voice_current;
					voice_tail = voice_head->next;
					voice_cnt++;
				}else{
					//printf("44444444 voice_head->num = %d,voice_current->num = %d\n",voice_head->num,voice_current->num);
					if(voice_head->num > voice_current->num){//当前音频时间最早

						voice_tmp = voice_head;
						voice_head = voice_current;
						voice_head->next = voice_tmp;
						voice_cnt++;
					}else{ //当前比头的时间晚
						voice_tmp_pre = voice_head;
						voice_tmp = voice_head->next;
						printf("AAAAAAAA\n");
						while(voice_tmp != voice_tail){
							//printf("ccccccc voice_current->num = %d\n",voice_current->num);
							//printf("ccccccc voice_tmp->num = %d\n",voice_tmp->num);
							if((voice_tmp->num) > (voice_current->num)){
								voice_tmp_next = voice_tmp;
								voice_tmp = voice_current;
								voice_tmp_pre->next = voice_tmp;
								voice_tmp->next = voice_tmp_next;
								voice_cnt++;
								break;
							}
							voice_tmp_pre = voice_tmp;
							voice_tmp = voice_tmp_pre->next;
						}
						if(voice_tmp == voice_tail){
							voice_tmp_pre->next = voice_current;
							voice_current->next = voice_tail;
							voice_cnt++;
						}
					}
				}
			}
		}
	}

	closedir(dir);
	//printf("voice_head->url = %s\n",voice_head->url);

	//voice_tmp = voice_head;

	//while(strlen(voice_tmp->next->url)){
	//	voice_tmp = voice_tmp->next;
	//	printf("voice_tmp = %s\n",voice_tmp->url);
	//}
	return NULL;
}
#else 
void *check_msg()//开机检查是否有留言，
{	printf("============check_msg=========\n");
	
	char cmd[256] = {0};
	char file_amr[128] = {0};
	DIR * dir;
	struct dirent * ptr;
	int first = 1;
	ST_VOICE *voice_tmp = NULL;
	ST_VOICE *voice_tail = NULL;
	dir = opendir("/usr/data/voice/");//这里的存放留言文件夹后面要改

	while((ptr = readdir(dir)) != NULL)
	{	
		printf("ptr->d_name = %s\n",ptr->d_name);
		if((strcmp(ptr->d_name,".")!=0)&&((strcmp(ptr->d_name,"..")!=0)))
		{
			memset(cmd,0,sizeof(cmd));
			memset(file_amr,0,sizeof(file_amr));
			
			sprintf(file_amr,"/usr/data/voice/%s",ptr->d_name);
			printf("-----check_msg ptr->d_name = %s\n",ptr->d_name);

			if(voice_head){
				voice_tmp = voice_tail->next;
				voice_tmp = (ST_VOICE *)calloc(1, sizeof(ST_VOICE));
				if(voice_tmp){		
					voice_tmp->next = NULL;
					memset(cmd,0,sizeof(cmd));
					voice_cnt++;
					sprintf(voice_tmp->url,"%s",ptr->d_name);
					printf("##### voice_tmp = %s\n",voice_head);
				}else{
					printf("voice_head calloc failed\n");
					continue;
				}
				voice_tail = voice_tmp;		
			}

			if(!voice_head){
				voice_head = (ST_VOICE *)calloc(1, sizeof(ST_VOICE));

				if(voice_head){		
					voice_head->next = NULL;
					if(first){
						voice_tail = voice_head;
						voice_keep = voice_head;
						first = 0;
					}
					memset(cmd,0,sizeof(cmd));
					voice_cnt++;
					sprintf(voice_head->url,"%s",ptr->d_name);
				
					printf("##### voice_head = %s\n",voice_head);
				}else{
					printf("voice_head calloc failed\n");
					continue;
				}
			}
		}
	}
	closedir(dir);

	return NULL;
}

#endif
void *h5_play_keep_msg()//播放保存下来的最近 留言，最多5条
{
	printf("=========h5_play_keep_msg============\n");
	printf("voice_keep->url = %s\n",voice_keep->url);
	mozart_play_pause();
	//int pre_pos = mozart_musicplayer_get_pos(mozart_musicplayer_handler);
	//int pre_index = mozart_musicplayer_musiclist_get_current_index(mozart_musicplayer_handler);

	mozart_play_pause();
	ST_VOICE *voice_play = voice_keep;
	char cmd[256] = {0};
	char file_amr[128] = {0};
	while(voice_play){	

		//播放留言
		memset(cmd,0,sizeof(cmd));
		memset(file_amr,0,sizeof(file_amr));
		sprintf(cmd,"mplayer /usr/data/voice/%s",voice_play->url);
		sprintf(file_amr,"/usr/data/voice/%s",voice_play->url);
		printf("@@@  voice_play->url = %s\n",voice_play->url);
		if(!access(file_amr,0))
			system(cmd);
		//播完一句
		voice_play = voice_play->next;
	}
	printf("zzzz voice_head->url = %s\n",voice_head->url);
	//mozart_musicplayer_play_index(mozart_musicplayer_handler, pre_index);   
	//mozart_musicplayer_set_seek(mozart_musicplayer_handler,pre_pos);
	mozart_play_pause();
	return NULL;
}


void *h5_play_msg()
{
	printf("=========h5_play_msg============voice_cnt = %d,voice_head= %p\n",voice_cnt,voice_head);
	if(voice_head){
//	int pre_pos = mozart_musicplayer_get_pos(mozart_musicplayer_handler);
//	int pre_index = mozart_musicplayer_musiclist_get_current_index(mozart_musicplayer_handler);

	mozart_play_pause();

	
		ST_VOICE *voice_play = voice_head;	//第一次播放的话 肯定是没听过的，要从头开始 播放
		if(!sing_flag){
			sing_flag = 1;
		}else if((sing_flag)&&(voice_head->next)){//没有新留言就播放最近的留言，有的话播放新来的留言
			if(voice_head->next)
				voice_play = voice_head->next;
		}
		printf("ccc\n");
		ST_VOICE *voice_pre_play = voice_head;
		
		char cmd[256] = {0};
		char file_amr[128] = {0};
		printf("dddd\n");

		while(voice_play){	

			//播放留言
			system("echo \"0\" > /sys/class/leds/led_red/brightness");
			system("echo \"0\" > /sys/class/leds/led_green/brightness");

			memset(cmd,0,sizeof(cmd));
			memset(file_amr,0,sizeof(file_amr));
			printf("3333 voice_play->url = %s\n",voice_play->url);
			sprintf(cmd,"mplayer /usr/data/voice/%s",voice_play->url);

			sprintf(file_amr,"/usr/data/voice/%s",voice_play->url);
			printf("@@@  voice_play->url = %s\n",voice_play->url);
			if(!access(file_amr,0))
				system(cmd);
			//播完一句

			if(voice_head->next){
				voice_pre_play = voice_head;
				voice_head = voice_head->next;
			}
			voice_play = voice_play->next;
			printf("CCCCCCCCC\n");
			if(voice_cnt > 1){
				printf("DDDDDDD\n");
				voice_cnt--;
				memset(cmd,0,sizeof(cmd));
				sprintf(cmd,"rm /usr/data/voice/%s",voice_pre_play->url);
				system(cmd);
				printf("??? cmd = %s\n",cmd);
				free(voice_pre_play);
				printf("??? AAA ??????\n");
				voice_pre_play = NULL;
			}
		}
		printf("zzzz voice_head->url = %s\n",voice_head->url);
		system("echo \"1\" > /sys/class/leds/led_red/brightness");
		system("echo \"1\" > /sys/class/leds/led_green/brightness");
		mozart_play_pause();
		//mozart_musicplayer_play_index(mozart_musicplayer_handler, pre_index);   
		//mozart_musicplayer_set_seek(mozart_musicplayer_handler,pre_pos);
	}
	
}


void *h5_insert_voice(char *url)//如果是在唤醒状态，不能提醒，要等唤醒过后才能进行
{
	printf("======h5_insert_voice===========url = %s\n",url);
	if(!url)
		return NULL;
	if(access("/usr/data/voice/",0)){
		system("mkdir /usr/data/voice");
	}
	while(1){
		if (mozart_vr_get_status() != VR_ASR)
			break;
		printf("receive newmsg!!!!!!");
		sleep(3);
	}
	char cmd[256] = {0};
	struct tm *tms;
	time_t curtime_sec = time(0); 
	tms = localtime(&curtime_sec);//time_p->tm_hour, time_p->tm_min, time_p->tm_sec
	
	int pre_pos = mozart_musicplayer_get_pos(mozart_musicplayer_handler);
	int pre_index = mozart_musicplayer_musiclist_get_current_index(mozart_musicplayer_handler);

	player_status_t status;
	status = mozart_player_getstatus(mozart_musicplayer_handler->player_handler);
	if(status == 2)
		mozart_play_pause();     //暂停下不需要再暂停

	usleep(200*1000);
	ST_VOICE *voice_x = NULL;
	mozart_play_key_sync("newmsg");
	usleep(200*1000);
	if(!voice_head){
		voice_head = (ST_VOICE *)calloc(1, sizeof(ST_VOICE));
		if(voice_head){	
			voice_head->next = NULL;
//			voice_tmp = voice_head;
			voice_keep = voice_head;
			memset(cmd,0,sizeof(cmd));
			sprintf(cmd,"wget http://file.rayshine.cc/media/%s -O /usr/data/voice/%02d%02d%02d%02d.amr",url,(tms->tm_mon + 1),tms->tm_hour, tms->tm_min, tms->tm_sec);
			printf("------>cmd_download_song_URL = %s\n",cmd);
			system(cmd);
			voice_cnt++;
			sprintf(voice_head->url,"%02d_%02d_%02d.amr",tms->tm_hour, tms->tm_min, tms->tm_sec);

		}else{
			return NULL;
		}
	}else{
		voice_tmp = voice_head;
		while(voice_tmp->next){
			usleep(100);
			printf("SSSSSSSSSSSSSSSS\n");
			voice_tmp = voice_tmp->next;
		}
		voice_tmp->next = (ST_VOICE *)calloc(1, sizeof(ST_VOICE));
		if(voice_tmp->next){
			memset(cmd,0,sizeof(cmd));
			sprintf(cmd,"wget http://file.rayshine.cc/media/%s -O /usr/data/voice/%02d%02d%02d%02d.amr",url,(tms->tm_mon + 1),tms->tm_hour, tms->tm_min, tms->tm_sec);
			printf("------>cmd_download_song_URL = %s\n",cmd);
			system(cmd);
			voice_cnt++;		
			sprintf(voice_tmp->next->url,"%02d%02d%02d%02d.amr",(tms->tm_mon + 1),tms->tm_hour, tms->tm_min, tms->tm_sec);
		}else{
			return NULL;
		}
	}


	printf("voice_head->url = %s\n",voice_head->url);
	if(voice_tmp)
		printf("voice_tmp->url = %s\n",voice_tmp->next->url);

	printf("@@@@@------------>voice_head->url = %s,voice_cnt = %d\n",voice_head->url,voice_cnt);
	
	if(voice_cnt == 2)
		printf("voice_head->next->url = %s\n",voice_head->next->url);
	if(voice_cnt == 3)
		printf("voice_head->next->next->url = %s\n",voice_head->next->next->url);


	mozart_musicplayer_play_index(mozart_musicplayer_handler, pre_index);   
	mozart_musicplayer_set_seek(mozart_musicplayer_handler,pre_pos);
	//mozart_play_pause();
}


char *ingenicplayer_add_queue(char *data)
{	printf("?????????????ingenicplayer_add_queue???????\n");
	int index = -1;
	json_object *object = NULL;
	json_object *array = NULL;
	json_object *songs = NULL;

	object = json_tokener_parse(data);
	if (!object) {
		pr_err("json parse failed %s\n", strerror(errno));
		return NULL;
	}
	
	if (json_object_object_get_ex(object, "songs", &songs)) {
		printf("##songs = %s\n",json_object_to_json_string(songs));
		if (json_object_array_length(songs) > 0) {
			
			array = json_object_array_get_idx(songs, 0);
			printf("##array = %s\n",json_object_to_json_string(array));
			
			index = ingenicplayer_insert_song(array);
		}
	}
	

	mozart_musicplayer_play_index(mozart_musicplayer_handler, index);
	json_object_put(object);

	return ingenicplayer_get_song_info(NULL,NULL);
}

char *ingenicplayer_replace_queue(char *data, struct appserver_cli_info *owner)
{	printf("???ingenicplayer_replace_queue????  data =\n %s\n",data);
	
	int i = 0;
	int index = -1;
	int length = -1;
	json_object *cmd_object = NULL;
	json_object *array = NULL;
	json_object *songs = NULL;
	json_object *tmp = NULL;

	cmd_object = json_tokener_parse(data);
	if (!cmd_object) {
		pr_err("json parse failed %s\n", strerror(errno));
		return NULL;
	}

	if (json_object_object_get_ex(cmd_object, "songs", &songs))
		length = json_object_array_length(songs);

	if(length == 0){
		mozart_musicplayer_musiclist_clean(mozart_musicplayer_handler);
		return NULL;
	}

	for (i = 0; i < length ; i++) {
		array = json_object_array_get_idx(songs, i);
		ingenicplayer_insert_song(array);
	}

	if (mozart_musicplayer_musiclist_get_length(mozart_musicplayer_handler) <= 0) {

		json_object_put(cmd_object);
		ingenicplayer_get_queue("delete_from_queue", owner);
		mozart_musicplayer_stop_playback(mozart_musicplayer_handler);
		return NULL;
	} else {
		if (json_object_object_get_ex(cmd_object, "index", &tmp) &&
				json_object_get_int(tmp) >= 0)
			index = json_object_get_int(tmp);
		else
			index = 0;
		index = mozart_musicplayer_musiclist_get_length(mozart_musicplayer_handler) - 1;
		mozart_musicplayer_play_index(mozart_musicplayer_handler, index);
	}

	json_object_put(cmd_object);

	return ingenicplayer_get_song_info("add_queue_to_play",NULL);
}

static void ingenicplayer_delete_queue(char *data, struct appserver_cli_info *owner)
{
	int idx = -1;
	int index = -1;
	int length = -1;

	if ((idx = get_object_int("index", data)) >= 0)
		mozart_musicplayer_musiclist_delete_index(mozart_musicplayer_handler, idx);

	ingenicplayer_get_queue("delete_from_queue", owner);

	index = mozart_musicplayer_musiclist_get_current_index(mozart_musicplayer_handler);
	length = mozart_musicplayer_musiclist_get_length(mozart_musicplayer_handler);

	if (index == idx && length > 0)
		mozart_musicplayer_play_index(mozart_musicplayer_handler, index);
	else if (length <= 0)
		mozart_musicplayer_stop_playback(mozart_musicplayer_handler);

	return;
}

char *ingenicplayer_play_queue(char *data)
{
	int index = -1;

	if ((index = get_object_int("index", data)) >= 0)
		mozart_musicplayer_play_index(mozart_musicplayer_handler, index);

	return ingenicplayer_get_song_info(NULL,NULL);
}

char *ingenicplayer_get_device_info(void)
{	printf("*******************ingenicplayer_get_device_info*********************************%s\n",__FUNCTION__);
	int volume;
	char *devicename = NULL;
	char *reply_json = NULL;
	json_object *reply_object = NULL;
	json_object *song_info = NULL;
	struct music_info *info = NULL;

	volume = mozart_musicplayer_get_volume(mozart_musicplayer_handler);
	if (volume < 0)
		return NULL;

	reply_object = json_object_new_object();
	if (!reply_object)
		return NULL;
	song_info = json_object_new_object();
	if (!song_info)
		goto get_device_info_err;

	json_object_object_add(reply_object, "volume", json_object_new_int(volume));

	if ((devicename = sync_devicename(NULL)) != NULL) {
		json_object_object_add(reply_object, "name",
				json_object_new_string(devicename));
		free(devicename);
	}

	json_object_object_add(reply_object, "song_info", song_info);

	get_player_status(song_info);

	info = mozart_musicplayer_musiclist_get_current_info(mozart_musicplayer_handler);
	if (info)
		get_song_json_object(song_info, info);
	reply_json = strdup(json_object_get_string(reply_object));

get_device_info_err:
	json_object_put(reply_object);
	return reply_json;
}

static char *ingenicplayer_heart_pulsate(void)
{
	char *reply_json = NULL;
	json_object *reply_object = json_object_new_object();
	if (!reply_object)
		return NULL;

	reply_json = strdup(json_object_get_string(reply_object));
	json_object_put(reply_object);

	return reply_json;
}

void ingenicplayer_set_seek(char *cmd_json)
{
	int ret = -1;
	char *reply_json = NULL;

	if ((ret = get_object_int("set_seek_position", cmd_json)) >= 0){
		mozart_musicplayer_set_seek(mozart_musicplayer_handler, ret);
	}else if((ret = get_object_int("value", cmd_json)) >= 0){
		mozart_musicplayer_set_seek(mozart_musicplayer_handler, ret);
	}
	pr_debug("seek %d\n", ret);

	reply_json = ingenicplayer_get_position(NULL,NULL);
	printf("reply_json = %s\n",reply_json);
	if (reply_json) {
		mozart_appserver_notify("get_position", reply_json);
		free(reply_json);
	}

	return;
}

char *ingenicplayer_get_sd_music(void)
{	
	char *sdmusic = NULL;
	char *reply_json = NULL;
	json_object *songs = NULL;
	json_object *reply_object = json_object_new_object();
	if (!reply_object)
		return NULL;

	if ((sdmusic = mozart_localplayer_get_musiclist()) == NULL) {
		
		json_object_object_add(reply_object, "insert",
				json_object_new_int(0));
		json_object_object_add(reply_object, "songs",
				json_object_new_array());
	} else {
		
		json_object_object_add(reply_object, "insert",
				json_object_new_int(1));
		if ((songs = json_tokener_parse(sdmusic)) != NULL)
			json_object_object_add(reply_object, "songs", songs);
		free(sdmusic);
	}
	reply_json = strdup(json_object_get_string(reply_object));

	json_object_put(reply_object);

	return reply_json;
}

static char *ingenicplayer_set_shortcut(char *data)
{
	FILE *fp = NULL;
	char *name = NULL;
	char shortcut_name[64] = {};

	if (!data)
		return NULL;

	name = get_object_string("name", NULL, data);
	if (name == NULL)
		return NULL;

	sprintf(shortcut_name, "%s%s", DEVICE_INGENICPLAYER_DIR, name);

	fp = fopen(shortcut_name, "w+");
	if (fp == NULL) {
		pr_err("fopen %s\n", strerror(errno));
		free(name);
		return NULL;
	}

	fwrite(data, strlen(data), sizeof(char), fp);
	fclose(fp);
	free(name);

	return NULL;
}

static char *ingenicplayer_get_shortcut(char *data)
{
	FILE *fp = NULL;
	char *name = NULL;
	char shortcut_name[64] = {};
	char *shortcut = NULL;
	int length = 0;
	json_object *object = NULL;

	if (!data)
		return NULL;

	name = get_object_string("name", NULL, data);
	if (name == NULL)
		return NULL;

	sprintf(shortcut_name, "%s%s", DEVICE_INGENICPLAYER_DIR, name);

	if ((fp = fopen(shortcut_name, "r")) != NULL) {
		fseek(fp, 0, SEEK_END);
		length = ftell(fp);
		fseek(fp, 0, SEEK_SET);
		if (length) {
			shortcut = (char *)calloc(length, sizeof(char) + 1);
			if (shortcut) {
				fread(shortcut, sizeof(char), length, fp);
				pr_debug("get shortcut %s\n", shortcut);
			}
		}
		fclose(fp);
	}
	if (shortcut == NULL) {
		if ((object = json_tokener_parse(data)) != NULL) {
			json_object_object_add(object, "data", json_object_new_array());
			shortcut = strdup(json_object_get_string(object));
			json_object_put(object);
		}
	}
	free(name);

	return shortcut;
}

static char *ingenicplayer_play_shortcut(char *data)
{
	FILE *fp = NULL;
	char shortcut_name[64] = {};
	char *shortcut = NULL;
	char *songs = NULL;
	char *reply_json = NULL;
	int length = 0;

	if (data == NULL)
		return NULL;

	sprintf(shortcut_name, "%s%s", DEVICE_INGENICPLAYER_DIR, data);

	if ((fp = fopen(shortcut_name, "r")) != NULL) {
		fseek(fp, 0, SEEK_END);
		length = ftell(fp);
		fseek(fp, 0, SEEK_SET);
		if (length) {
			shortcut = (char *)calloc(length, sizeof(char) + 1);
			if (shortcut) {
				fread(shortcut, sizeof(char), length, fp);
				pr_debug("get shortcut %s\n", shortcut);

				if ((songs = get_object_string("data", NULL, shortcut)) != NULL)
					reply_json = ingenicplayer_replace_queue(songs, NULL);

				free(shortcut);
			}
		}
		fclose(fp);
	}

	return reply_json;
}

static int ingenicplayer_handle_command(struct cmd_info_c *cmd_info)
{	
	int index = -1;
	int volume = 0;
	char *name = NULL;
	char *command = NULL;
	char *reply_json = NULL;
	char cmd[64] = {0};
	char ID[32] = {0};
	int fd = -1;
	int ret = -1;
	char buf[16] = {0};
	char num_r[16] = {0};
	char num_g[16] = {0};
	char num_b[16] = {0};
	char *tmp;

	json_object *data = NULL;
	json_object *object = NULL;

	nvinfo_t *nvinfo = NULL;

	for (index = 0; index < sizeof(ingenicplayer_cmd) /
			sizeof(ingenicplayer_cmd[0]); index++) {
		if (strcmp(ingenicplayer_cmd[index], cmd_info->command) == 0)
			break;
	}
	
	switch (index) {
	case INGENICPLAYER_PLAY_PAUSE:
		ingenicplayer_play_pause();
		break;
	case INGENICPLAYER_PREV:
		mozart_musicplayer_prev_music(mozart_musicplayer_handler);
		break;
	case INGENICPLAYER_NEXT:
		mozart_musicplayer_next_music(mozart_musicplayer_handler);
		break;
	case INGENICPLAYER_PLAY_MODE:
		ingenicplayer_play_mode(cmd_info);
		break;
	case INGENICPLAYER_INC_VOLUME:
		mozart_musicplayer_volume_up(mozart_musicplayer_handler);
		break;
	case INGENICPLAYER_DEC_VOLUME:
		mozart_musicplayer_volume_down(mozart_musicplayer_handler);
		break;
	case INGENICPLAYER_SET_VOLUME:
		volume = get_object_int("volume", cmd_info->data);
		mozart_musicplayer_set_volume(mozart_musicplayer_handler, volume);
		break;
	case INGENICPLAYER_GET_VOLUME:
		reply_json = ingenicplayer_get_volume(NULL,NULL);
		command = strdup("get_volume");
		break;
	case INGENICPLAYER_GET_SONG_INFO:
		reply_json = ingenicplayer_get_song_info(NULL,NULL);
		command = strdup("get_song_info");
		break;
	case INGENICPLAYER_GET_SONG_QUEUE:
		ingenicplayer_get_queue("get_song_queue", cmd_info->owner);
		break;
	case INGENICPLAYER_ADD_QUEUE:
		//mozart_module_ingenicplayer_start();
		reply_json = ingenicplayer_add_queue(cmd_info->data);
		command = strdup("get_song_info");
		break;
	case INGENICPLAYER_REPLACE_QUEUE:
		//mozart_module_ingenicplayer_start();
		reply_json = ingenicplayer_replace_queue(cmd_info->data, cmd_info->owner);
		command = strdup("get_song_info");
		break;
	case INGENICPLAYER_DEL_QUEUE:
		ingenicplayer_delete_queue(cmd_info->data, cmd_info->owner);
		break;
	case INGENICPLAYER_PLAY_QUEUE:
		reply_json = ingenicplayer_play_queue(cmd_info->data);
		command = strdup("get_song_info");
		break;
	case INGENICPLAYER_GET_DEVICE_INFO:
		reply_json = ingenicplayer_get_device_info();
		command = strdup("get_device_info");
		break;
	case INGENICPLAYER_HEART_PULSATE:
		reply_json = ingenicplayer_heart_pulsate();
		command = strdup("heart_pulsate");
		break;
	case INGENICPLAYER_SET_SEEK:
		ingenicplayer_set_seek(cmd_info->data);
		break;
	case INGENICPLAYER_GET_POSITION:
		reply_json = ingenicplayer_get_position(NULL,NULL);
		command = strdup("get_position");
		break;
	case INGENICPLAYER_GET_SD_MUSIC:
		reply_json = ingenicplayer_get_sd_music();
		ingenicplayer_get_sd_music();
		command = strdup("get_sd_music");
		break;
	case INGENICPLAYER_SET_SHORTCUT:
		reply_json = ingenicplayer_set_shortcut(cmd_info->data);
		command = strdup("set_shortcut");
		break;
	case INGENICPLAYER_GET_SHORTCUT:
		reply_json = ingenicplayer_get_shortcut(cmd_info->data);
		command = strdup("get_shortcut");
		break;
	case INGENICPLAYER_PLAY_SHORTCUT:
		//mozart_module_ingenicplayer_start();
		name = get_object_string("name", NULL, cmd_info->data);
		if (name) {
			mozart_ingenicplayer_notify_play_shortcut(name);
			free(name);
		}
		break;
	case INGENICPLAYER_ADD_QUEUE_TO_TAIL:
		reply_json = ingenicplayer_add_queue(cmd_info->data);
		command = strdup("get_song_info");
		break;
	case INGENICPLAYER_UPGRADE:
		printf("%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%  \n");
		nvinfo = mozart_updater_chkver();
		if (strcmp(nvinfo->update_version,nvinfo->current_version) != 0) {
			system("rm /usr/data/voice/*");
			system("echo \"154\" > /sys/class/leds/led_red/brightness");
			system("echo \"50\" > /sys/class/leds/led_green/brightness");
			system("echo \"205\" > /sys/class/leds/led_blue/brightness");
			mozart_system("updater -p");
		}else{
			player_status_t status;
			status = mozart_player_getstatus(mozart_musicplayer_handler->player_handler);
			if (status == PLAYER_PLAYING) 
				mozart_play_pause();
			mozart_tts("已经是最新版本");
			mozart_tts_wait();
			if (status == PLAYER_PLAYING) 
				mozart_play_pause();
		}
		free(nvinfo);
		break;
	case INGENICPLAYER_RS_READ:
		fd = open("/usr/data/mac.txt",O_RDONLY);
		if(fd){
			memset(ID,0,sizeof(ID));
			ret = read(fd,ID,sizeof(ID));
			if(ret)
				sprintf(cmd,"{\"scope\":\"%s\",\"data\":\"%s\"}","rs_read",ID);
			else
				sprintf(cmd,"{\"scope\":\"%s\",\"data\":\"%s\"}","rs_read","null");
			close(fd);
		}else
			printf("open /usr/data/mac.txt error\n");
		reply_json = strdup(json_object_get_string(json_tokener_parse(cmd)));
		command = strdup("usercommand");
		break;
	case INGENICPLAYER_RS_WRITE:
		if(access("/usr/data/mac.txt",F_OK)){
			memset(cmd,0,sizeof(cmd));
		
			object = json_tokener_parse(cmd_info->data);
			json_object_object_get_ex(object,"data",&data);
			sprintf(cmd,"%s %s","rs_write",json_object_get_string(data));

			system(cmd);
		}
		reply_json = strdup("{OK}");
		command = strdup("usercommand");
		break;
	case INGENICPLAYER_RS_WRITE_FORCE:
		memset(cmd,0,sizeof(cmd));
		object = json_tokener_parse(cmd_info->data);
		json_object_object_get_ex(object,"data",&data);
		sprintf(cmd,"%s %s","rs_write",json_object_get_string(data));
		system(cmd);
		reply_json = strdup("{OK}");
		command = strdup("usercommand");
		break;
	case INGENICPLAYER_RS_LED_RGB:
		printf("---------> INGENICPLAYER_RS_LED_RGB\n");
		object = json_tokener_parse(cmd_info->data);
		json_object_object_get_ex(object,"data",&data);
		if(data)
			sprintf(buf,"%s",json_object_get_string(data));

		snprintf(num_r,3,"%s",&buf[0]);
		snprintf(num_g,3,"%s",&buf[2]);
		snprintf(num_b,3,"%s",&buf[4]);

		sprintf(cmd,"echo %d > /sys/class/leds/led_red/brightness",(int)strtol(num_r,&tmp,16));
		printf("cmd = %s\n",cmd);
		system(cmd);
		sprintf(cmd,"echo %d > /sys/class/leds/led_green/brightness",(int)strtol(num_g,&tmp,16));
		printf("cmd = %s\n",cmd);
		system(cmd);
		sprintf(cmd,"echo %d > /sys/class/leds/led_blue/brightness",(int)strtol(num_b,&tmp,16));
		printf("cmd = %s\n",cmd);
		system(cmd);
		
		break;
	case INGENICPLAYER_CLEAR_WPACFG:
		system("> /usr/data/wpa_supplicant.conf");
		break;
	default:
		pr_err("Unknow ingenic player cmd %d %s\n", index, cmd_info->command);
		break;
	}

	if (reply_json && command){
		int tmp = -1;
		tmp = mozart_appserver_response(command, reply_json, cmd_info->owner);
		//printf("%s:tmp = %d\n",cmd_info->command,tmp);
	}

	free(reply_json);
	free(command);

	free_cmd_list(cmd_info);

	return index;
}

int mozart_ingenicplayer_notify_play_shortcut(char *shortcut_name)
{
	char *reply_json = NULL;

	if ((reply_json = ingenicplayer_play_shortcut(shortcut_name)) == NULL)
		return -1;

	mozart_appserver_notify("get_song_info", reply_json);
	free(reply_json);

	return 0;
}

int mozart_ingenicplayer_notify_volume(int volume)
{
	json_object *reply_object = NULL;

	if ((reply_object = json_object_new_object()) == NULL)
		return -1;

	json_object_object_add(reply_object, "volume", json_object_new_int(volume));
	mozart_appserver_notify("set_volume", (char *)json_object_get_string(reply_object));
	json_object_put(reply_object);

	return 0;
}

int mozart_ingenicplayer_notify_play_mode(enum play_mode mode)
{
	json_object *reply_object = NULL;

	if ((reply_object = json_object_new_object()) == NULL)
		return -1;

	json_object_object_add(reply_object, "mode", json_object_new_int(mode));
	mozart_appserver_notify("set_queue_mode", (char *)json_object_get_string(reply_object));
	json_object_put(reply_object);

	return 0;
}

int mozart_ingenicplayer_notify_pos(void)
{
	char *reply_json = NULL;

	reply_json = ingenicplayer_get_position(NULL,NULL);
	if (reply_json) {
		mozart_appserver_notify("get_position", reply_json);
		free(reply_json);
	}

	return 0;
}

int mozart_ingenicplayer_notify_song_info(void)
{
	char *reply_json = NULL;

	reply_json = ingenicplayer_get_song_info(NULL,NULL);
	if (reply_json) {
		mozart_appserver_notify("get_song_info", reply_json);
		free(reply_json);
	}

	return 0;
}

int mozart_ingenicplayer_response_cmd(char *command, char *data, struct appserver_cli_info *owner)
{//	printf("=======mozart_ingenicplayer_response_cmd=========== command = %s\n",command);
	struct cmd_info_c *cmd_info = NULL;

	cmd_info = (struct cmd_info_c *)calloc(sizeof(struct cmd_info_c), sizeof(char));

	cmd_info->owner = (struct appserver_cli_info *)calloc(
			sizeof(struct appserver_cli_info), sizeof(char));
	if (!cmd_info->owner) {
		pr_err("calloc %s\n", strerror(errno));
		free(cmd_info);
		return 0;
	}
	memcpy(cmd_info->owner, owner, sizeof(struct appserver_cli_info));
	cmd_info->command = strdup(command);
	cmd_info->data = strdup(data);

	pthread_mutex_lock(&cmd_mutex);
	list_insert_at_tail(&cmd_list, (struct cmd_info_c *)cmd_info);
	pthread_cond_signal(&cond_var);
	pthread_mutex_unlock(&cmd_mutex);

	return 0;
}

static void *ingenicplayer_func(void *arg)
{
	printf("*********************************ingenicplayer_func****************************************\n");
	char devicename[64] = {0};
	char macaddr[] = "255.255.255.255";
	struct stat devicename_stat;
	
	system("mkdir -p /usr/data/ingenicplayer");
	#if 0
	stat(DEVICENAME, &devicename_stat);
	if (devicename_stat.st_size == 0) {
		
		memset(macaddr, 0, sizeof(macaddr));
		get_mac_addr("wlan0", macaddr, "");
		sprintf(devicename, "NS-%s", macaddr + 6);
		pr_debug("devicename %s\n", devicename);
		sync_devicename(devicename);
			
	}
	#endif
	int fd = open("/usr/data/mac.txt",O_RDONLY);
	if(fd > 0){
		char id_buf[32] = {0};
		read(fd,id_buf,sizeof(id_buf));
		strncpy(devicename, id_buf,strlen(id_buf));
		devicename[strlen(id_buf)] = '\0';
		close(fd);
	}else{
		sprintf(devicename,"%s","tmp_ID");
	}
	//mozart_appserver_rename(devicename);
	pr_debug("devicename %s\n", devicename);
	sync_devicename(devicename);

	pthread_mutex_lock(&cmd_mutex);
	while (die_out) {
		if (!is_empty(&cmd_list)) {
			struct cmd_info_c *cmd_info;
			cmd_info = list_delete_at_index(&cmd_list, 0);

			if (cmd_info) {
				pthread_mutex_unlock(&cmd_mutex);
				ingenicplayer_handle_command(cmd_info);
				pthread_mutex_lock(&cmd_mutex);
			}
		} else {
			pthread_cond_wait(&cond_var, &cmd_mutex);
		}
	}
	list_destroy(&cmd_list, free_cmd_list);
	pthread_mutex_unlock(&cmd_mutex);
#ifdef SUPPORT_SONG_SUPPLYER
	if (snd_source == SND_SRC_CLOUD)
		mozart_musicplayer_stop(mozart_musicplayer_handler);
#endif
	
	return NULL;
}

int mozart_ingenicplayer_startup(void)
{
	pthread_mutex_lock(&cmd_mutex);
	if (die_out) {
		pthread_mutex_unlock(&cmd_mutex);
		return 0;
	}

	die_out = true;
	list_init(&cmd_list);

	if (pthread_create(&ingenicplayer_thread, NULL, ingenicplayer_func, NULL)) {
		pr_err("create ingenicplayer_thread failed\n");
		return -1;
	}
	pthread_detach(ingenicplayer_thread);

	pthread_mutex_unlock(&cmd_mutex);

	return 0;
}

int mozart_ingenicplayer_shutdown(void)
{
	pthread_mutex_lock(&cmd_mutex);
	if (die_out) {
		die_out = false;
		pthread_cond_signal(&cond_var);
	}
	pthread_mutex_unlock(&cmd_mutex);

	return 0;
}
