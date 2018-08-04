/*
Copyright (c) 2009-2014 Roger Light <roger@atchoo.org>

All rights reserved. This program and the accompanying materials
are made available under the terms of the Eclipse Public License v1.0
and Eclipse Distribution License v1.0 which accompany this distribution.
 
The Eclipse Public License is available at
   http://www.eclipse.org/legal/epl-v10.html
and the Eclipse Distribution License is available at
  http://www.eclipse.org/org/documents/edl-v10.php.
 
Contributors:
   Roger Light - initial implementation and documentation.
*/
#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include <fcntl.h>
#include <stdbool.h>

#ifndef MAX_PATH
#define MAX_PATH 256
#endif

#ifndef WIN32
#include <unistd.h>
#else
#include <process.h>
#include <winsock2.h>
#define snprintf sprintf_s
#endif

#include <mosquitto.h>
#include "client_shared.h"


#include "utils_interface.h"
#include "sharememory_interface.h"
#include "wifi_interface.h"
#include "localplayer_interface.h"

#include "mozart_config.h"
#include "lapsule_control.h"
#include "ingenicplayer.h"
#include "nvrw_interface.h"

#include "json-c/json.h"
#include "utils_interface.h"
#include "linklist_interface.h"
#include "localplayer_interface.h"

#include "mozart_musicplayer.h"
#include "mozart_key_function.h"
#include "mozart_face.h"

bool process_messages = true;
int msg_count = 0;
struct mosq_config cfg;

#define STATUS_CONNECTING 0
#define STATUS_CONNACK_RECVD 1
#define STATUS_WAITING 2


static char *topic = NULL;
static char *message = NULL;
static long msglen = 0;
static int qos = 0;
static int retain = 0;
static int mode = MSGMODE_NONE;
static int status = STATUS_CONNECTING;
static int mid_sent = 0;
static int last_mid = -1;
static int last_mid_sent = -1;
static bool connected = true;
static char *username = NULL;
static char *password = NULL;
static bool disconnect_sent = false;
static bool quiet = false;

#define VERSION "1.4.12"


const char * base64char = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

char * base64_encode( const unsigned char * bindata, char * base64, int binlength )
{
    int i, j;
    unsigned char current;

    for ( i = 0, j = 0 ; i < binlength ; i += 3 )
    {
        current = (bindata[i] >> 2) ;
        current &= (unsigned char)0x3F;
        base64[j++] = base64char[(int)current];

        current = ( (unsigned char)(bindata[i] << 4 ) ) & ( (unsigned char)0x30 ) ;
        if ( i + 1 >= binlength )
        {
            base64[j++] = base64char[(int)current];
            base64[j++] = '=';
            base64[j++] = '=';
            break;
        }
        current |= ( (unsigned char)(bindata[i+1] >> 4) ) & ( (unsigned char) 0x0F );
        base64[j++] = base64char[(int)current];

        current = ( (unsigned char)(bindata[i+1] << 2) ) & ( (unsigned char)0x3C ) ;
        if ( i + 2 >= binlength )
        {
            base64[j++] = base64char[(int)current];
            base64[j++] = '=';
            break;
        }
        current |= ( (unsigned char)(bindata[i+2] >> 6) ) & ( (unsigned char) 0x03 );
        base64[j++] = base64char[(int)current];

        current = ( (unsigned char)bindata[i+2] ) & ( (unsigned char)0x3F ) ;
        base64[j++] = base64char[(int)current];
    }
    base64[j] = '\0';
    return base64;
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

struct cmd_info_c {
	struct appserver_cli_info *owner;
	char *command;
	char *data;
};

void pub_connect_callback(struct mosquitto *mosq, void *obj, int result)
{
	int rc = MOSQ_ERR_SUCCESS;

	if(!result){
		switch(mode){
			case MSGMODE_CMD:
			case MSGMODE_FILE:
			case MSGMODE_STDIN_FILE:
				rc = mosquitto_publish(mosq, &mid_sent, topic, msglen, message, qos, retain);
				break;
			case MSGMODE_NULL:
				rc = mosquitto_publish(mosq, &mid_sent, topic, 0, NULL, qos, retain);
				break;
			case MSGMODE_STDIN_LINE:
				status = STATUS_CONNACK_RECVD;
				break;
		}
		if(rc){
			if(!quiet){
				switch(rc){
					case MOSQ_ERR_INVAL:
						fprintf(stderr, "Error: Invalid input. Does your topic contain '+' or '#'?\n");
						break;
					case MOSQ_ERR_NOMEM:
						fprintf(stderr, "Error: Out of memory when trying to publish message.\n");
						break;
					case MOSQ_ERR_NO_CONN:
						fprintf(stderr, "Error: Client not connected when trying to publish.\n");
						break;
					case MOSQ_ERR_PROTOCOL:
						fprintf(stderr, "Error: Protocol error when communicating with broker.\n");
						break;
					case MOSQ_ERR_PAYLOAD_SIZE:
						fprintf(stderr, "Error: Message payload is too large.\n");
						break;
				}
			}
			mosquitto_disconnect(mosq);
		}
	}else{
		if(result && !quiet){
			fprintf(stderr, "%s\n", mosquitto_connack_string(result));
		}
	}
}

void pub_disconnect_callback(struct mosquitto *mosq, void *obj, int rc)
{
	connected = false;
}


void my_publish_callback(struct mosquitto *mosq, void *obj, int mid)
{
	last_mid_sent = mid;
	if(mode == MSGMODE_STDIN_LINE){
		if(mid == last_mid){
			mosquitto_disconnect(mosq);
			disconnect_sent = true;
		}
	}else if(disconnect_sent == false){
		mosquitto_disconnect(mosq);
		disconnect_sent = true;
	}
}

#if 0
int mqtt_pub(char *host,char *port,char *user,char *passwd,char *pub_topic,char *msg)
{
	printf("========mqtt_pub===========pub_topic = %s\n",pub_topic);
	printf("host = %s\n",host);
	printf("port = %d\n",atoi(port));
	printf("user = %s\n",user);
	printf("passwd = %s\n",passwd);
	printf("msg = %s\n",msg);

	struct mosq_config pub_cfg;
	struct mosquitto *mosq = NULL;
	int rc;
	int rc2;
	char *buf;
	int buf_len = 1024;
	int buf_len_actual;
	int read_len;
	int pos;

	buf = malloc(buf_len);
	if(!buf){
		fprintf(stderr, "Error: Out of memory.\n");
		return 1;
	}

	memset(&pub_cfg, 0, sizeof(struct mosq_config));
	rc = client_config_load(&pub_cfg, CLIENT_PUB, host, port, user, passwd, pub_topic,NULL, msg);
	if(rc){
		client_config_cleanup(&pub_cfg);
		if(rc == 2){
			/* --help */
			print_usage();
		}else{
			fprintf(stderr, "\nUse 'mosquitto_pub --help' to see usage.\n");
		}
		return 1;
	}

	topic = pub_cfg.topic;
	message = pub_cfg.message;
	msglen = pub_cfg.msglen;
	qos = pub_cfg.qos;
	retain = pub_cfg.retain;
	mode = pub_cfg.pub_mode;
	username = pub_cfg.username;
	password = pub_cfg.password;
	quiet = pub_cfg.quiet;

	mosquitto_lib_init();

	if(client_id_generate(&pub_cfg, "mosqpub")){
		return 1;
	}

	mosq = mosquitto_new(pub_cfg.id, true, NULL);
	if(!mosq){
		switch(errno){
			case ENOMEM:
				if(!quiet) fprintf(stderr, "Error: Out of memory.\n");
				break;
			case EINVAL:
				if(!quiet) fprintf(stderr, "Error: Invalid id.\n");
				break;
		}
		mosquitto_lib_cleanup();
		return 1;
	}

	mosquitto_connect_callback_set(mosq, pub_connect_callback);
	mosquitto_disconnect_callback_set(mosq, pub_disconnect_callback);
	mosquitto_publish_callback_set(mosq, my_publish_callback);

	if(client_opts_set(mosq, &cfg)){
		return 1;
	}
	rc = client_connect(mosq, &cfg);
	if(rc) return rc;

	if(mode == MSGMODE_STDIN_LINE){
		mosquitto_loop_start(mosq);
	}
	printf("aaaaa cfg->message = %s\n",cfg.message);
	do{
	if(mode == MSGMODE_STDIN_LINE){
		printf("ccc\n");
		if(status == STATUS_CONNACK_RECVD){
			printf("ddd\n");
			pos = 0;
			read_len = buf_len;
			while(fgets(&buf[pos], read_len, stdin)){
				buf_len_actual = strlen(buf);
				if(buf[buf_len_actual-1] == '\n'){
					buf[buf_len_actual-1] = '\0';
					rc2 = mosquitto_publish(mosq, &mid_sent, topic, buf_len_actual-1, buf, qos, retain);
					if(rc2){
						if(!quiet) fprintf(stderr, "Error: Publish returned %d, disconnecting.\n", rc2);
						mosquitto_disconnect(mosq);
					}
					break;
				}else{
					buf_len += 1024;
					pos += 1023;
					read_len = 1024;
					buf = realloc(buf, buf_len);
					if(!buf){
						fprintf(stderr, "Error: Out of memory.\n");
						return 1;
					}
				}
			}
			if(feof(stdin)){
				last_mid = mid_sent;
				status = STATUS_WAITING;
			}
		}else if(status == STATUS_WAITING){
			if(last_mid_sent == last_mid && disconnect_sent == false){
				mosquitto_disconnect(mosq);
				disconnect_sent = true;
			}
#ifdef WIN32
			Sleep(100);
#else
			usleep(100000);
#endif
		}
		rc = MOSQ_ERR_SUCCESS;
	}else{
		rc = mosquitto_loop(mosq, -1, 1);
		//printf("bbb rc = %d\n",rc);
	}

	}while(rc == MOSQ_ERR_SUCCESS && connected);

	if(mode == MSGMODE_STDIN_LINE){
		mosquitto_loop_stop(mosq, false);
	}
	if(message && mode == MSGMODE_FILE){
		free(message);
	}
	mosquitto_destroy(mosq);
	mosquitto_lib_cleanup();

	if(rc){
		fprintf(stderr, "Error: %s\n", mosquitto_strerror(rc));
	}
	
	return 0;
}
#endif
char *del_blank(char *ptr)
{
	char *buf = NULL;
	buf = (char *)calloc(strlen(ptr),sizeof(char));
	if(!buf){
		printf("EEROR : calloc \n");
		return NULL;
	}
	char *buf_p = buf;
	

	char *pTmp = ptr;  

	while (*pTmp != '\0')   
	{
		if (*pTmp != ' ')  
		{  
		    *buf_p++ = *pTmp;  
		}  
		pTmp++;
	}
	char *ret = NULL;
	ret = (char *)calloc(strlen(ptr),sizeof(char));
	if(!ret){
		printf("EEROR : calloc \n");
		return NULL;
	}
	strcpy(ret,buf);
	free(buf);
	return ret;
}

void my_message_callback(struct mosquitto *mosq, void *obj, const struct mosquitto_message *message)
{
	struct mosq_config *cfg;
	int i;
	bool res;

	char *command = NULL;
	if(process_messages == false) return;

	assert(obj);
	cfg = (struct mosq_config *)obj;
	if(message->retain && cfg->no_retain) return;
	if(cfg->filter_outs){
		for(i=0; i<cfg->filter_out_count; i++){
			mosquitto_topic_matches_sub(cfg->filter_outs[i], message->topic, &res);
			if(res) return;
		}
	}
	if(cfg->verbose){
		if(message->payloadlen){
			printf("%s ", message->topic);
			printf("receive : ");
			fwrite(message->payload, 1, message->payloadlen, stdout);
			if(cfg->eol){
				printf("\n");
			}
		}else{
			if(cfg->eol){
				printf("%s (null)\n", message->topic);
			}
		}
		fflush(stdout);
	}else{
		printf("receive : ");
		if(message->payloadlen){
			fwrite(message->payload, 1, message->payloadlen, stdout);
			if(cfg->eol){
				printf("\n");
			}
			fflush(stdout);
		}
	}

	if(cfg->msg_count>0){
		msg_count++;
		if(cfg->msg_count == msg_count){
			process_messages = false;
			mosquitto_disconnect(mosq);
		}
	}

	command = get_object_string("command",NULL,(char *)message->payload);
	if(command == NULL)
		return NULL;
	printf("============>get the command : [%s]\n",command);

	int send_flag = 0;
	char cur_time[8] = {0};
	time_t timep;  
//    struct tm *p;  
     long curtime_sec = time(&timep);  
//    p =localtime(&timep); 

//	sprintf(cur_time,"%02d%02d%02d",p->tm_hour, p->tm_min,p->tm_sec);
	sprintf(cur_time,"%d",curtime_sec);
	char *reply_json = NULL;
	json_object *reply_object = NULL;
	char *song = NULL;
	int index = 0;
	enum play_mode mode;
//	struct appserver_cli_info *sub_owner = NULL;
//	sub_owner = (struct cmd_info_c *)(message->payload);
	if(strcmp(command,"prefix") == 0){ //------------------
		mozart_play_pause();		
		mozart_tts_sync("上一首");
		mozart_previous_song();
		mozart_aispeech_tts("\C9\CFһ\CA\D7");
		sleep(5);
		reply_json = del_blank(ingenicplayer_get_song_info(command,cur_time));
		send_flag = 1;
	}else if(strcmp(command,"skip") == 0){//------------
		mozart_play_pause();		
		mozart_tts_sync("下一首");
		mozart_next_song();
		mozart_aispeech_tts("\CF\C2һ\CA\D7");
		sleep(5);
		reply_json = del_blank(ingenicplayer_get_song_info(command,cur_time));
		send_flag = 1;
	}else if(strcmp(command,"toggle") == 0){//---------------
		player_status_t status = mozart_player_getstatus(mozart_musicplayer_handler->player_handler);
		if(status == PLAYER_PAUSED){
			mozart_tts_sync("继续播发");
			//usleep(500*1000);
		}
		mozart_play_pause();
		if(status == PLAYER_PLAYING){
			mozart_tts_sync("已暂停");
		}
		sleep(5);
		reply_json = del_blank(ingenicplayer_get_song_info(command,cur_time));
		send_flag = 1;
	}else if(strcmp(command,"get_volume") == 0){//----------------
		reply_json = del_blank(ingenicplayer_get_volume(command,cur_time));
		send_flag = 1;
	}
	else if(strcmp(command,"set_volume") == 0){//----------------
		int value = -1;
		value = get_object_int("value", (char *)message->payload);
		mozart_musicplayer_set_volume(mozart_musicplayer_handler,value);
		
		reply_object = json_object_new_object();
		if(command)
			json_object_object_add(reply_object, "command", json_object_new_string(command));
		
		json_object_object_add(reply_object, "value", json_object_new_int(value));
		if(cur_time)
			json_object_object_add(reply_object, "timestamp", json_object_new_string(cur_time));
		reply_json = del_blank(strdup(json_object_get_string(reply_object)));
		send_flag = 1;
	}
	else if(strcmp(command,"increase_volume") == 0){//---------
		mozart_volume_up();
		send_flag = 1;
	}else if(strcmp(command,"decrease_volume") == 0){//------------
		mozart_volume_down();
		send_flag = 1;
	}else if(strcmp(command,"get_device_info") == 0){	
		//reply_json = ingenicplayer_get_device_info();
		reply_json = del_blank(ingenicplayer_get_device_info());
		send_flag = 1;
	}else if(strcmp(command,"upgrade_ota") == 0){//--------------------------
		nvinfo_t *nvinfo = NULL;
		nvinfo = mozart_updater_chkver();
		if (strcmp(nvinfo->update_version,nvinfo->current_version) != 0) {
			system("rm /usr/data/voice/*");
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
	}else if(strcmp(command,"get_status") == 0){ //---------------------
		reply_object = json_object_new_object();
		int stat = 1;
		json_object_object_add(reply_object, "status", json_object_new_int(stat));
		if(cur_time)
			json_object_object_add(reply_object, "timestamp", json_object_new_string(cur_time));
		reply_json = del_blank(strdup(json_object_get_string(reply_object)));
		//printf("reply_json = %s\n",reply_json);
		send_flag = 1;
	}else if(strcmp(command,"get_sd_music") == 0){//-------------------
		//reply_json = ingenicplayer_get_sd_music();
		reply_json = del_blank(ingenicplayer_get_sd_music());
		send_flag = 1;
	}else if(strcmp(command,"get_song_info") == 0){//----------------------------
		reply_json = del_blank(ingenicplayer_get_song_info(command,cur_time));
		send_flag = 1;
	}else if(strcmp(command,"get_position") == 0){//----------
		//reply_json = ingenicplayer_get_position();
		reply_json = del_blank(ingenicplayer_get_position(command,cur_time));
		send_flag = 1;
	}else if(strcmp(command,"get_song_queue") == 0){//------------------
		char *tmp = h5_get_queue();
		reply_json = del_blank(tmp);
		send_flag = 1;

	}else if(strcmp(command,"add_queue_to_tail") == 0){//\CC\ED\BCӵ\BD\B2\A5\B7\C5\C1б\EDβ\B2\BF\A3\AC\C1\A2\BC\B4\BEͲ\A5\B7\C5\C1\CB
		ingenicplayer_add_queue((char *)message->payload);
		
		char *tmp = h5_get_queue();
		reply_json = del_blank(tmp);
		send_flag = 1;
	}else if(strcmp(command,"delete_from_queue") == 0){//ɾ\B3\FD\CB\F7\D2\FDָ\B6\A8\B5ĸ\E8\C7\FA
		h5_musiclist_delete_index((char *)message->payload);
		char *tmp = h5_get_queue();
		reply_json = del_blank(tmp);
		send_flag = 1;
	}
	else if(strcmp(command,"add_queue_to_play") == 0){//\CC\ED\BCӶ\E0\CA׸\E8\C7\FA\B5\BD\C1б\EDβ\B2\BF
		char *tmp  = ingenicplayer_replace_queue((char *)message->payload, NULL);
		reply_json = del_blank(tmp);
		send_flag = 1;
	}else if(strcmp(command,"play_from_queue") == 0){//---------------
		char *tmp  = ingenicplayer_play_queue((char *)message->payload);
		reply_json = del_blank(tmp);
		send_flag = 1;
	}else if(strcmp(command,"set_queue_mode") == 0){//????????????????  \C9\E8\D6\C3\CE\DEЧ
		//ingenicplayer_play_mode((char *)message->payload);
		int value = h5_play_mode((char *)message->payload);
		reply_object = json_object_new_object();
		if(command)
			json_object_object_add(reply_object, "command", json_object_new_string(command));
		
		json_object_object_add(reply_object, "value", json_object_new_int(value));
		if(cur_time)	
			json_object_object_add(reply_object, "timestamp", json_object_new_string(cur_time));
		
		reply_json = del_blank(strdup(json_object_get_string(reply_object)));
		printf("reply_json = %s\n",reply_json);
		send_flag = 1;
	}else if(strcmp(command,"set_seek") == 0){//------------------------
		ingenicplayer_set_seek((char *)message->payload);
		int value = -1;
		value = get_object_int("value", (char *)message->payload);
		reply_object = json_object_new_object();
		if(command)
			json_object_object_add(reply_object, "command", json_object_new_string(command));
		
		json_object_object_add(reply_object, "value", json_object_new_int(value));
		if(cur_time)	
			json_object_object_add(reply_object, "timestamp", json_object_new_string(cur_time));
		reply_json = del_blank(strdup(json_object_get_string(reply_object)));
		send_flag = 1;
	}else if(strcmp(command,"wx_voice") == 0){
		char songUrl[256] = {0};
		//songUrl = get_object_string("songUrl",NULL,(char *)message->payload);
		strcpy(songUrl,get_object_string("songUrl",NULL,(char *)message->payload));
		if(songUrl == NULL)
			return NULL;
		printf("songUrl = %s\n",songUrl);
		h5_insert_voice(songUrl);
		
		#if 0
		printf("@@@@@@@@@@@@@ send back msg!!\n");
		char *id = NULL;
		char *deviceId = NULL;
		char *userName = NULL;
		char *nickName = NULL;

		id = get_object_string("id",NULL,(char *)message->payload);
		deviceId = get_object_string("deviceId",NULL,(char *)message->payload);
		userName = get_object_string("userName",NULL,(char *)message->payload);
		nickName = get_object_string("nickName",NULL,(char *)message->payload);
		
		printf("id = %s,deviveid = %s\n",id,deviceId);
		
		reply_object = json_object_new_object();
		if(command)
			json_object_object_add(reply_object, "command", json_object_new_string(command));
		if(id)
			json_object_object_add(reply_object, "id", json_object_new_string(id));
		if(deviceId)
			json_object_object_add(reply_object, "deviceId", json_object_new_string(deviceId));
		if(songUrl)
			json_object_object_add(reply_object, "songUrl", json_object_new_string(songUrl));
		if(cur_time)
			json_object_object_add(reply_object, "timestamp", json_object_new_string(cur_time));
		
		json_object_object_add(reply_object, "source", json_object_new_string("dev"));
		if(userName)
			json_object_object_add(reply_object, "userName", json_object_new_string(userName));
		if(nickName)
			json_object_object_add(reply_object, "nickName", json_object_new_string(nickName));

		reply_json = del_blank(strdup(json_object_get_string(reply_object)));
		printf("reply_json = %s\n",reply_json);
		send_flag = 1;
		#endif
		
	}

	char macaddr[] = "255.255.255.255";
	memset(macaddr, 0, sizeof(macaddr));
	get_mac_addr("wlan0", macaddr, "");
	char pub_topic[32] = {0};
	sprintf(pub_topic,"/app/%s",macaddr);
//	mqtt_pub("120.24.75.220","61613","admin","password",pub_topic,"123456789");
	if(send_flag){
		char *cmd = NULL;
		char send_msg[10*1024] = {0};
		base64_encode( reply_json, send_msg, strlen(reply_json));
		//printf("send_msg = %s\n",send_msg);

		cmd = (char *)calloc(strlen(send_msg) + 128,sizeof(char));
		if(!cmd){
			printf("EEROR :cmd calloc \n");
			return NULL;
		}
		sprintf(cmd,"mosquitto_pub -h 120.24.75.220 -p 61613 -u admin -P password -t /app/NS-%s -m %s",&macaddr[6],send_msg);
		//printf("SYSCMD = %s\n",cmd);
		system(cmd);
		free(cmd);
		//free(send_msg);
	}
	free(reply_json);

}

void my_connect_callback(struct mosquitto *mosq, void *obj, int result)
{
	int i;
	struct mosq_config *cfg;

	assert(obj);
	cfg = (struct mosq_config *)obj;

	if(!result){
		for(i=0; i<cfg->topic_count; i++){
			mosquitto_subscribe(mosq, NULL, cfg->topics[i], cfg->qos);
		}
	}else{
		if(result && !cfg->quiet){
			fprintf(stderr, "%s\n", mosquitto_connack_string(result));
		}
	}
}

void my_subscribe_callback(struct mosquitto *mosq, void *obj, int mid, int qos_count, const int *granted_qos)
{
	int i;
	struct mosq_config *cfg;

	assert(obj);
	cfg = (struct mosq_config *)obj;

	if(!cfg->quiet) printf("Subscribed (mid: %d): %d", mid, granted_qos[0]);
	for(i=1; i<qos_count; i++){
		if(!cfg->quiet) printf(", %d", granted_qos[i]);
	}
	if(!cfg->quiet) printf("\n");
}

void my_log_callback(struct mosquitto *mosq, void *obj, int level, const char *str)
{
	printf("%s\n", str);
}

void print_usage(void)
{
	int major, minor, revision;

	mosquitto_lib_version(&major, &minor, &revision);
	printf("mosquitto_sub is a simple mqtt client that will subscribe to a single topic and print all messages it receives.\n");
	printf("mosquitto_sub version %s running on libmosquitto %d.%d.%d.\n\n", VERSION, major, minor, revision);
	printf("Usage: mosquitto_sub [-c] [-h host] [-k keepalive] [-p port] [-q qos] [-R] -t topic ...\n");
	printf("                     [-C msg_count] [-T filter_out]\n");
#ifdef WITH_SRV
	printf("                     [-A bind_address] [-S]\n");
#else
	printf("                     [-A bind_address]\n");
#endif
	printf("                     [-i id] [-I id_prefix]\n");
	printf("                     [-d] [-N] [--quiet] [-v]\n");
	printf("                     [-u username [-P password]]\n");
	printf("                     [--will-topic [--will-payload payload] [--will-qos qos] [--will-retain]]\n");
#ifdef WITH_TLS
	printf("                     [{--cafile file | --capath dir} [--cert file] [--key file]\n");
	printf("                      [--ciphers ciphers] [--insecure]]\n");
#ifdef WITH_TLS_PSK
	printf("                     [--psk hex-key --psk-identity identity [--ciphers ciphers]]\n");
#endif
#endif
#ifdef WITH_SOCKS
	printf("                     [--proxy socks-url]\n");
#endif
	printf("       mosquitto_sub --help\n\n");
	printf(" -A : bind the outgoing socket to this host/ip address. Use to control which interface\n");
	printf("      the client communicates over.\n");
	printf(" -c : disable 'clean session' (store subscription and pending messages when client disconnects).\n");
	printf(" -C : disconnect and exit after receiving the 'msg_count' messages.\n");
	printf(" -d : enable debug messages.\n");
	printf(" -h : mqtt host to connect to. Defaults to localhost.\n");
	printf(" -i : id to use for this client. Defaults to mosquitto_sub_ appended with the process id.\n");
	printf(" -I : define the client id as id_prefix appended with the process id. Useful for when the\n");
	printf("      broker is using the clientid_prefixes option.\n");
	printf(" -k : keep alive in seconds for this client. Defaults to 60.\n");
	printf(" -N : do not add an end of line character when printing the payload.\n");
	printf(" -p : network port to connect to. Defaults to 1883.\n");
	printf(" -P : provide a password (requires MQTT 3.1 broker)\n");
	printf(" -q : quality of service level to use for the subscription. Defaults to 0.\n");
	printf(" -R : do not print stale messages (those with retain set).\n");
#ifdef WITH_SRV
	printf(" -S : use SRV lookups to determine which host to connect to.\n");
#endif
	printf(" -t : mqtt topic to subscribe to. May be repeated multiple times.\n");
	printf(" -T : topic string to filter out of results. May be repeated.\n");
	printf(" -u : provide a username (requires MQTT 3.1 broker)\n");
	printf(" -v : print published messages verbosely.\n");
	printf(" -V : specify the version of the MQTT protocol to use when connecting.\n");
	printf("      Can be mqttv31 or mqttv311. Defaults to mqttv31.\n");
	printf(" --help : display this message.\n");
	printf(" --quiet : don't print error messages.\n");
	printf(" --will-payload : payload for the client Will, which is sent by the broker in case of\n");
	printf("                  unexpected disconnection. If not given and will-topic is set, a zero\n");
	printf("                  length message will be sent.\n");
	printf(" --will-qos : QoS level for the client Will.\n");
	printf(" --will-retain : if given, make the client Will retained.\n");
	printf(" --will-topic : the topic on which to publish the client Will.\n");
#ifdef WITH_TLS
	printf(" --cafile : path to a file containing trusted CA certificates to enable encrypted\n");
	printf("            certificate based communication.\n");
	printf(" --capath : path to a directory containing trusted CA certificates to enable encrypted\n");
	printf("            communication.\n");
	printf(" --cert : client certificate for authentication, if required by server.\n");
	printf(" --key : client private key for authentication, if required by server.\n");
	printf(" --ciphers : openssl compatible list of TLS ciphers to support.\n");
	printf(" --tls-version : TLS protocol version, can be one of tlsv1.2 tlsv1.1 or tlsv1.\n");
	printf("                 Defaults to tlsv1.2 if available.\n");
	printf(" --insecure : do not check that the server certificate hostname matches the remote\n");
	printf("              hostname. Using this option means that you cannot be sure that the\n");
	printf("              remote host is the server you wish to connect to and so is insecure.\n");
	printf("              Do not use this option in a production environment.\n");
#ifdef WITH_TLS_PSK
	printf(" --psk : pre-shared-key in hexadecimal (no leading 0x) to enable TLS-PSK mode.\n");
	printf(" --psk-identity : client identity string for TLS-PSK mode.\n");
#endif
#endif
#ifdef WITH_SOCKS
	printf(" --proxy : SOCKS5 proxy URL of the form:\n");
	printf("           socks5h://[username[:password]@]hostname[:port]\n");
	printf("           Only \"none\" and \"username\" authentication is supported.\n");
#endif
	printf("\nSee http://mosquitto.org/ for more information.\n\n");
}
#if 0
int main(int argc, char *argv[])
{
	struct mosq_config cfg;
	struct mosquitto *mosq = NULL;
	int rc;
	
	rc = client_config_load(&cfg, CLIENT_SUB, argc, argv);
	if(rc){
		client_config_cleanup(&cfg);
		if(rc == 2){
			/* --help */
			print_usage();
		}else{
			fprintf(stderr, "\nUse 'mosquitto_sub --help' to see usage.\n");
		}
		return 1;
	}

	mosquitto_lib_init();

	if(client_id_generate(&cfg, "mosqsub")){
		return 1;
	}

	mosq = mosquitto_new(cfg.id, cfg.clean_session, &cfg);
	if(!mosq){
		switch(errno){
			case ENOMEM:
				if(!cfg.quiet) fprintf(stderr, "Error: Out of memory.\n");
				break;
			case EINVAL:
				if(!cfg.quiet) fprintf(stderr, "Error: Invalid id and/or clean_session.\n");
				break;
		}
		mosquitto_lib_cleanup();
		return 1;
	}
	if(client_opts_set(mosq, &cfg)){
		return 1;
	}
	if(cfg.debug){
		mosquitto_log_callback_set(mosq, my_log_callback);
		mosquitto_subscribe_callback_set(mosq, my_subscribe_callback);
	}
	mosquitto_connect_callback_set(mosq, my_connect_callback);
	mosquitto_message_callback_set(mosq, my_message_callback);

	rc = client_connect(mosq, &cfg);
	if(rc) return rc;


	rc = mosquitto_loop_forever(mosq, -1, 1);

	mosquitto_destroy(mosq);
	mosquitto_lib_cleanup();

	if(cfg.msg_count>0 && rc == MOSQ_ERR_NO_CONN){
		rc = 0;
	}
	if(rc){
		fprintf(stderr, "Error: %s\n", mosquitto_strerror(rc));
	}
	return rc;
}
#endif
int start_mqtt(char *host,char *port,char *user,char *passwd,char *topic1,char *topic2)
{	printf("======start_mqtt===== topic = %s,topic2 = %s\n",topic1,topic2);
	if(access("/usr/data/voice/",F_OK) != 0){
		system("mkdir /usr/data/voice");
	}
	check_msg();
	struct mosquitto *mosq = NULL;
	int rc;
	
	if(topic1)
		rc = client_config_load(&cfg, CLIENT_SUB,host,port,user,passwd,topic1,topic2,NULL);
	if(rc){
		client_config_cleanup(&cfg);
		if(rc == 2){
			/* --help */
			print_usage();
		}else{
			fprintf(stderr, "\nUse 'mosquitto_sub --help' to see usage.\n");
		}
		return 1;
	}

	mosquitto_lib_init();

	if(client_id_generate(&cfg, "mosqsub")){
		return 1;
	}

	mosq = mosquitto_new(cfg.id, cfg.clean_session, &cfg);
	if(!mosq){
		switch(errno){
			case ENOMEM:
				if(!cfg.quiet) fprintf(stderr, "Error: Out of memory.\n");
				break;
			case EINVAL:
				if(!cfg.quiet) fprintf(stderr, "Error: Invalid id and/or clean_session.\n");
				break;
		}
		mosquitto_lib_cleanup();
		return 1;
	}

	if(client_opts_set(mosq, &cfg)){
		return 1;
	}

	if(cfg.debug){
		mosquitto_log_callback_set(mosq, my_log_callback);
		mosquitto_subscribe_callback_set(mosq, my_subscribe_callback);
	}

	mosquitto_connect_callback_set(mosq, my_connect_callback);
	mosquitto_message_callback_set(mosq, my_message_callback);

	rc = client_connect(mosq, &cfg);

	if(rc) return rc;


	rc = mosquitto_loop_forever(mosq, -1, 1);

	mosquitto_destroy(mosq);
	mosquitto_lib_cleanup();

	if(cfg.msg_count>0 && rc == MOSQ_ERR_NO_CONN){
		rc = 0;
	}
	if(rc){
		fprintf(stderr, "Error: %s\n", mosquitto_strerror(rc));
	}
	return rc;	
	
}
