#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <pthread.h>
#include <curl/curl.h>
#include "sharememory_interface.h"
#include "wifi_interface.h"
#include "tips_interface.h"
#include "power_interface.h"
#include "lapsule_control.h"
#include "utils_interface.h"
#include "ini_interface.h"
#include "json-c/json.h"
#include "json-c/json_util.h"
#include "mozart_config.h"
#include "appserver.h"

#include "mozart_app.h"
#include "mozart_ui.h"
#include "mozart_tts.h"
#include "mozart_key_function.h"
#include "mozart_musicplayer.h"
#include "mozart_camera.h"
#include "mozart_face.h"
#include "modules/mozart_module_cloud_music.h"
#include "md5.h"


#ifdef SUPPORT_BT
#include "modules/mozart_module_bt.h"
#endif

#if (SUPPORT_DMR == 1)
#include "modules/mozart_module_dmr.h"
#endif

#include "modules/mozart_module_vr.h"
#include "modules/mozart_module_cloud_music.h"

#include "vr_interface.h"
#include "volume_interface.h"
#include "tips_interface.h"
#include "song_supplyer/speech.h"

#include <linux/ioctl.h>	/* needed for the _IOW etc stuff used later */
#include <sys/ioctl.h> 
#include "include/mozart_face.h"


extern pthread_t breath_answer_t;
extern int breath_answer_flag;

extern pthread_t music_playing_pthread;
//extern bool music_playing_configing = false;


int need_resume_after_asr = 0;

extern int g_translate_flag; 
extern char *g_translate_input; 
char translate_buf[1024] = {0};// 存放翻译结果
char luobo_buf[1024] = {0};
extern int g_alarm_cnt;  //防止特定时刻唤醒出问题
int no_voice_cnt = 0; //没有说话的次数，超过3次 退出ASR

 int wake_flage  = 0;
 int wakebefore_status = 0;


static vr_result_t process_vr_aec_callback(aec_info_t *aec)
{	printf("????  process_vr_aec_callback\n");
	vr_result_t result;
	memset(&result, 0, sizeof(result));
	int ret = 0;
	if (aec->wakeup) {
		wakebefore_status = mozart_musicplayer_get_status(mozart_musicplayer_handler);
		printf("aec wakeup.... wakebefore_status = %d\n",wakebefore_status);
		//FIXME: maybe not paused here.
		
		ret = mozart_module_pause();
		printf("mozart_module_pause ret = %d,g_alarm_cnt = %d\n",ret,g_alarm_cnt);
		if(ret == 0 && g_alarm_cnt == 0){
			wake_flage = 1;
			mozart_play_key_sync("welcome");
			result.vr_next = VR_TO_ASR;
			//result.vr_next = VR_TO_AEC;
		}else{
			result.vr_next = VR_TO_AEC;
		}
	} else {
		/* aec error, back to AEC again. */
		result.vr_next = VR_TO_AEC;
	}

	return result;
}
#define LIGHT_PURPLE "\033[1;35m"
#define NONE          "\033[m"
#define my_pr(fmt, args...)		printf(LIGHT_PURPLE fmt NONE,##args)
char pre_singer[128] = {0};

static void process_vr_asr_command_command(asr_info_t *asr)
{
	sem_command_t *command = &asr->sem.request.command;
	
	/* response volume first. */
	if (command->volume) {
		my_pr("process_vr_asr_command_command volume : [%s]####\n",command->volume);
		if (!strcmp(command->volume, "+")) {
			mozart_volume_up();
			mozart_play_key_sync("volume_up");
		} else if (!strcmp(command->volume, "-")) {
			mozart_volume_down();
			mozart_play_key_sync("volume_down");
		} else if (!strcmp(command->volume, "max")) {
			mozart_volume_set(100, MUSIC_VOLUME);
			mozart_play_key_sync("volume_max");
		} else if (!strcmp(command->volume, "min")) {
			mozart_volume_set(10, MUSIC_VOLUME);
			mozart_play_key_sync("volume_min");
		} else if (command->volume[0] >= '0' && command->volume[0] <= '9') {
			mozart_volume_set(atoi(command->volume), MUSIC_VOLUME);
		} else {
			printf("TODO: unsupport volume set format: %s.\n", command->volume);
		}
		mozart_play_pause();
		return;
	}
	/* then response operation */
	if (command->operation) {
		my_pr("process_vr_asr_command_command operation: [%s]####\n",command->operation);
		if (strcmp(command->operation, "暂停") == 0) {
			printf("暂停\n");
			//g_auto_pause = 1;
			mozart_module_pause();
			//FIXME: maybe not paused here.
			mozart_play_key_sync("paused");
			need_resume_after_asr = 0;
		} else if (strcmp(command->operation, "播放") == 0) {
			//g_auto_pause = 1;
			mozart_play_key_sync("resume");
			mozart_play_pause();
			need_resume_after_asr = 0;
		} else if (strcmp(command->operation, "继续") == 0) {
			//g_auto_pause = 1;
			mozart_play_key_sync("resume");
			mozart_play_pause();
			need_resume_after_asr = 0;
		} else if (strcmp(command->operation, "停止") == 0) {
			//mozart_module_stop();
			//FIXME: maybe not stop here.
			mozart_play_key_sync("paused");
			need_resume_after_asr = 0;
		} else if (strcmp(command->operation, "上一首") == 0) {
			mozart_play_key_sync("previous_song");
			mozart_previous_song();
			need_resume_after_asr = 0;
		} else if (strcmp(command->operation, "下一首") == 0) { 
			mozart_play_key_sync("next_song");
			mozart_next_song();
			need_resume_after_asr = 0;
		} else if (strcmp(command->operation, "退出") == 0) {
			mozart_play_key_sync("exit");
		} else if (strcmp(command->operation, "结束") == 0) {
			mozart_play_key_sync("exit");
		} else {
			printf("小乐暂时不支持该领域指令：%s\n", command->operation);
			mozart_play_key_sync("error_invalid_domain");
			mozart_play_pause();
			need_resume_after_asr = 0;
		}
	}else{	//说“没有火车票怎么办”  肯能会跳这
		mozart_play_key_sync("error_invalid_domain");
		mozart_play_pause();

	}
	return;
}

static void process_vr_asr_command(asr_info_t *asr)
{
	printf("sem.request.domain: %s\n", vr_domain_str[asr->sem.request.domain]);
	my_pr("asr->sem.request.domain: %d\n", asr->sem.request.domain);
	switch (asr->sem.request.domain) {
	case DOMAIN_MUSIC:
		//printf("小乐暂时不支持该领域指令：%s\n", vr_domain_str[asr->sem.request.domain]);
		//mozart_play_key_sync("error_invalid_domain");
		mozart_next_song();
		break;
	case DOMAIN_MOVIE:
		printf("小乐暂时不支持该领域指令：%s\n", vr_domain_str[asr->sem.request.domain]);
		mozart_play_key_sync("error_invalid_domain");
		break;
	case DOMAIN_NETFM:
		printf("小乐暂时不支持该领域指令：%s\n", vr_domain_str[asr->sem.request.domain]);
		mozart_play_key_sync("error_invalid_domain");
		break;
	case DOMAIN_COMMAND:
		process_vr_asr_command_command(asr);
		break;
	default:
		printf("小乐暂时不支持该领域指令：%d\n", asr->sem.request.domain);
		mozart_play_key_sync("error_invalid_domain");
		break;
	}

	return;
}

extern vr_info_t ai_vr_info;

#define SERVER_ADDR	"119.23.207.229" 
#define SERVER_PORT	82 
#define POSTto "api.rayshine.cc" 
char txt_upload_head[] =
	"POST /devlog/text?deviceId=NS-%s&&userSayText=%s&&ttsText=%s HTTP/1.1\r\n"
	"Host: %s\r\n"
	"User-Agent: Mozilla/5.0 (Windows NT 6.1; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/59.0.3071.115 Safari/537.36\r\n"
	"Content-Length: 0\r\n"
	"Accept: */*\n"
	"Accept-Encoding: gzip, deflate\r\n"
	"Accept-Language: zh-CN,zh;q=0.8\r\n"
	"Cache-Control: no-cache\r\n"
	"Proxy-Connection: keep-alive\r\n\r\n"
;
int URLEncode(const char* str, const int strSize, char* result, const int resultSize)  
{  
    int i;  
    int j = 0;//for result index  
    char ch;  
  
    if ((str==NULL) || (result==NULL) || (strSize<=0) || (resultSize<=0)) {  
        return 0;  
    }  
  
    for ( i=0; (i<strSize)&&(j<resultSize); ++i) {  
        ch = str[i];  
        if (((ch>='A') && (ch<'Z')) ||  
            ((ch>='a') && (ch<'z')) ||  
            ((ch>='0') && (ch<'9'))) {  
            result[j++] = ch;  
        } else if (ch == ' ') {  
            result[j++] = '+';  
        } else if (ch == '.' || ch == '-' || ch == '_' || ch == '*') {  
            result[j++] = ch;  
        } else {  
            if (j+3 < resultSize) {  
                sprintf(result+j, "%%%02X", (unsigned char)ch);  
                j += 3;  
            } else {  
                return 0;  
            }  
        }  
    }  
  
    result[j] = '\0';  
    return j;  
}  
  

int chat_txt_post(char *ask,char *answer)
{	printf("========chat_txt_post=======\n");
	int sock = -1;
	char send_date[4096] = {0};
	int ret = -1;

	char macaddr[] = "255.255.255.255";
	memset(macaddr, 0, sizeof(macaddr));
	get_mac_addr("wlan0", macaddr, "");
	sock = Connect_Server(SERVER_ADDR,SERVER_PORT);
	if(sock < 0 )
	{
		printf("connect server error!\n");
		return -1;
	}

	ret = snprintf(send_date,4096,txt_upload_head,&macaddr[6],ask,answer,POSTto);
	//printf("send_date = \n\n%s  \n\n",send_date);

	if(send(sock,send_date,ret,0) != ret)
	{
		printf("send head error!\n");
		close(sock);
		return -1;
	}
	printf("\r\n\r\n");

//接收返回值，用于判断是否上传成功
	//printf("#########################################\n");
	memset(send_date,0,sizeof(send_date));
	if(recv(sock,send_date,4096,0) < 0)
		printf("recv error!\n");
		
	//printf("recv:%s\n",send_date);
	close(sock);

	return 0;	
}
extern int translate_flag;

size_t nn = 0;
size_t write_data(void * ptr,size_t size,size_t nmemb,void *user_p)
{
	FILE *fp = (FILE *)user_p;
	size_t return_size = fwrite(ptr,size,nmemb,fp);
	nn += return_size;
	return return_size;
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
char *translate_zh_en(char *input)
{
	CURL *curl;
	CURLcode curl_code;
	curl = curl_easy_init();
	FILE *fp = NULL;
	fp = fopen("/tmp/translate_en.txt","ab+");// 
	if(curl) {
		char myurl[1000] = "http://api.fanyi.baidu.com/api/trans/vip/translate?";
		char *appid = "20171026000090956";    
		char *q = input;
		char *from = "zh";        
		char *to = "en";           
		char salt[60];
		int a = rand();
		sprintf(salt,"%d",a);
		char *secret_key = "RNQMz02CgPmXY95hmg63";  
		char sign[120] = "";
		strcat(sign,appid);
		strcat(sign,q);
		strcat(sign,salt);
		strcat(sign,secret_key);
		printf("????sign = %s\n",sign);
		unsigned char md[16];

		int i;
		char tmp[3]={'\0'},buf[33]={'\0'};
		MD5_CTX md5; 
		MD5Init(&md5); 
		MD5Update(&md5,sign,strlen((char *)sign));
		MD5Final(&md5,md);  
		for (i = 0; i < 16; i++){
			sprintf(tmp,"%2.2x",md[i]);
			strcat(buf,tmp);
		}

		printf("????buf = %s\n",buf);
		strcat(myurl,"appid=");
		strcat(myurl,appid);
		strcat(myurl,"&q=");
		strcat(myurl,q);
		strcat(myurl,"&from=");
		strcat(myurl,from);
		strcat(myurl,"&to=");
		strcat(myurl,to);
		strcat(myurl,"&salt=");
		strcat(myurl,salt);
		strcat(myurl,"&sign=");
		strcat(myurl,buf);
		printf("myurl------->:%s\n",myurl);	

		MC_SET_DEBUG(1);
		
		curl_code = curl_easy_setopt(curl,CURLOPT_URL,myurl);
		if(curl_code != CURLE_OK){
			goto err;
		}
		curl_easy_setopt(curl,CURLOPT_WRITEFUNCTION,write_data);
		curl_easy_setopt(curl,CURLOPT_WRITEDATA,fp);
		curl_code = curl_easy_perform(curl);

	err:
		curl_easy_cleanup(curl);
	}
	fclose(fp);
	fp = NULL;
	fp = fopen("/tmp/translate_en.txt","rb");
	memset(translate_buf,0,sizeof(translate_buf));
	fread(translate_buf,1,4096,fp);
	
	printf("translate_buf = %s\n",translate_buf);
	fclose(fp);
	system("rm /tmp/translate_en.txt");
	g_translate_flag = 0;
	g_translate_input = NULL;
	int cnt = strlen(translate_buf);
	int i = 0 ;
	for(i=0;i < cnt;i++){
		//printf("i = %d,translate_buf[i] = %c\n",i,translate_buf[i]);
		if(translate_buf[i] == '[')
			translate_buf[i] = ' ';
		if(translate_buf[i] == ']'){
			translate_buf[i] = ' ';
			break;
		}	
	}
	//printf("$$$ translate_buf = %s\n",translate_buf);
	return translate_buf;
}
int luobo_tech_callback(char *input)
{
	printf("====luobo_tech_callback=======input = %s\n",input);
	if(!input||(strcmp(input,"") == 0)){
		printf("NULL input\n");
		return 1;
	}
	CURL *curl;
	CURLcode curl_code;
	curl = curl_easy_init();
	struct curl_slist * list = NULL;
	FILE *fp = NULL;
	fp = fopen("/tmp/luobo.txt","wb+");
	if(curl) {
		char myurl[1000] = "http://test-service.luobotec.com/nlu/rest/api?";
		//char *appid = "20171026000090956";	  
		char *content = input;
		char *imei = "123";		  		   

		strcat(myurl,"content=");
		strcat(myurl,content);
		strcat(myurl,"&imei=");
		strcat(myurl,imei);
		printf("myurl------->:%s\n",myurl); 
		MC_SET_DEBUG(1);
		list = curl_slist_append(list,"AppKey:luobo_story_ruixin");
		list = curl_slist_append(list,"Nonce:3254435");
		list = curl_slist_append(list,"CurTime:12345678");
		list = curl_slist_append(list,"CheckSum:4ad14d03017224600ff37090304264051f057820");
		curl_easy_setopt(curl,CURLOPT_HTTPHEADER,list);
		curl_code = curl_easy_setopt(curl,CURLOPT_URL,myurl);
		//curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);	
		//curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, 2000L);
		if(curl_code != CURLE_OK){
			curl_easy_cleanup(curl);
			fclose(fp);
			return -1;
		}
		curl_easy_setopt(curl,CURLOPT_WRITEFUNCTION,write_data);
		curl_easy_setopt(curl,CURLOPT_WRITEDATA,fp);
		curl_code = curl_easy_perform(curl);
			
	}
	fclose(fp);

	fp = NULL;
	fp = fopen("/tmp/luobo.txt","rb");
	memset(luobo_buf,0,sizeof(luobo_buf));
	fread(luobo_buf,1,sizeof(luobo_buf),fp);
	fclose(fp);
	printf("******************************luobo_buf**************************************\n");
	printf("%s\n",luobo_buf);
	printf("*****************************************************************************\n");
	return 0;
}

	typedef struct remind_datas{
		int seconds;
		char *content;
	}REMIND_DATAS;

	pthread_t remind_pthread[10] = {0};
	int remind_pthread_cnt = 0;

void *remind_func(void *args)
{
	REMIND_DATAS *remind_data = (REMIND_DATAS *)args;
	int ret = 3;
	int cnt = remind_data->seconds;
	printf("remind_func cnt = %d\n",cnt);
	printf("remind_func content = %s\n",remind_data->content);
	while(cnt--){
		sleep(1);
	}
	player_status_t status;
	status = mozart_player_getstatus(mozart_musicplayer_handler->player_handler);
	if (status == PLAYER_PLAYING) {
		mozart_play_pause();
	}
	while(ret--){
		mozart_tts("提醒时间到了");
		mozart_tts_wait();
		usleep(200*1000);
		mozart_tts(remind_data->content);
		mozart_tts_wait();
	}
	mozart_play_pause();
	remind_pthread_cnt--;
	free(remind_data);
	return NULL;
}

int get_remind_seconds(char *time_string)
{
	if(!time_string)
		return -1;
	time_t now;
	now = time(NULL);
	
	int seconds_now = 0;
	int seconds_set = 0;	
	int seconds = 0;
	struct tm *tp = localtime(&now); 
	
	printf("%d-%d-%d \n", tp->tm_mon+1, tp->tm_mday, tp->tm_year + 1900);  
	printf("%d:%d:%d\n", tp->tm_hour, tp->tm_min, tp->tm_sec);
        seconds_now = (tp->tm_mday)*24*3600 + tp->tm_hour*3600 + tp->tm_min*60 + tp->tm_sec;

	char time_buf[64] = {0};
	strcpy(time_buf,time_string);
	//printf("time_buf = %s\n",time_buf);
	char *tmp = NULL;
	char *result = NULL;

	tmp = time_buf;
	result = strsep(&tmp,"-");
	result = strsep(&tmp,"-");
	result = strsep(&tmp," ");
	seconds_set += atoi(result)*3600*24;

	result = strsep(&tmp,":");
	seconds_set += atoi(result)*3600;
	result = strsep(&tmp,":");
	seconds_set += atoi(result)*60;
	seconds_set += atoi(tmp);
	seconds = seconds_set - seconds_now ; 
	//printf("seconds = %d\n",seconds);
	return seconds;
}

static vr_result_t process_vr_asr_callback(asr_info_t *asr)
{	
	my_pr("asr.input = %s\n",asr->input);
	my_pr("asr->sds.output = %s\n",asr->sds.output);
	my_pr("g_translate_flag = %d\n",g_translate_flag);
	my_pr("g_translate_input = %s\n",g_translate_input);
	my_pr("asr->sds.state = %d\n",asr->sds.state);

	json_object *json_msg = NULL;
	json_object *json_code = NULL;
	json_object *json_result = NULL;
	json_object *json_domain = NULL;
	json_object *json_answer = NULL;
	json_object *json_params = NULL;
	json_object *json_speechText = NULL;
	json_object *json_speechUrl = NULL;
	json_object *json_confidence = NULL;
	json_object *json_intent = NULL;
	json_object *json_singer = NULL;
	json_object *json_name = NULL;
	json_object *json_playUrl = NULL;
	json_object *json_ablumName = NULL;
	json_object *json_news = NULL;
	json_object *json_news_array = NULL;
	json_object *json_volume = NULL;
	json_object *json_time = NULL;
	json_object *json_content = NULL;

	int remind_seconds = 0;	
	REMIND_DATAS *remind_data = NULL;
	int luobo_ret = -1;
	luobo_ret = luobo_tech_callback(ai_vr_info.asr.input);
	if(strncmp(luobo_buf,"{",1)&&strncmp(luobo_buf,"[",1))
		luobo_ret = 1;
	if(!luobo_ret){
		json_msg= json_tokener_parse(luobo_buf);
		json_object_object_get_ex(json_msg,"ret_code",&json_code);
		if(json_code&&strcmp("0000",json_object_get_string(json_code)) != 0)
			luobo_ret = -1;
		json_object_object_get_ex(json_msg,"result",&json_result);
		json_object_object_get_ex(json_result,"confidence",&json_confidence);
	}
	//文本上传部分
	char input[4096] = {0};	
	char output[4096] = {0};
	if(!luobo_ret){
		if(json_confidence){
			if(strcmp("1",json_object_get_string(json_confidence)) == 0){
				json_object_object_get_ex(json_result,"params",&json_params);
				json_object_object_get_ex(json_params,"speechText",&json_speechText);
				json_object_object_get_ex(json_params,"speechUrl",&json_speechUrl);
				json_object_object_get_ex(json_params,"answer",&json_answer);
				if(json_object_get_string(json_speechText))
					URLEncode(json_object_get_string(json_speechText),strlen(json_object_get_string(json_speechText)),output,4096);
				else if(json_object_get_string(json_answer))
					URLEncode(json_object_get_string(json_answer),strlen(json_object_get_string(json_answer)),output,4096);
				URLEncode(ai_vr_info.asr.input,strlen(ai_vr_info.asr.input),input,4096);
				chat_txt_post(input,output);
			}
		}
	}else if(asr->sds.output){
		if(ai_vr_info.asr.input)
			URLEncode(ai_vr_info.asr.input,strlen(ai_vr_info.asr.input),input,4096);
		if(asr->sds.output)
			URLEncode(asr->sds.output,strlen(asr->sds.output),output,4096);
		chat_txt_post(input,output);
	}

	vr_result_t result;
	memset(&result, 0, sizeof(result));
	printf("asr result, domain: %s\n", vr_domain_str[asr->sds.domain]);

	need_resume_after_asr = 1;
	my_pr("--------------SWITCH-----------------START :asr->sds.domain = %d\n",asr->sds.domain);
	
	domain_e domain_x = asr->sds.domain;
	if(json_confidence){
		if(strcmp("1",json_object_get_string(json_confidence)) == 0){
			json_object_object_get_ex(json_result,"intent",&json_intent);
			json_object_object_get_ex(json_result,"params",&json_params);

			json_object_object_get_ex(json_params,"ablumName",&json_ablumName);
			json_object_object_get_ex(json_params,"singer",&json_singer);
			json_object_object_get_ex(json_params,"name",&json_name);
			json_object_object_get_ex(json_params,"playUrl",&json_playUrl);

			if (strncmp(json_object_get_string(json_intent),"dialog",6) == 0)
				domain_x = DOMAIN_CHAT;
			else if(strncmp(json_object_get_string(json_intent),"media",5) == 0)
				domain_x = DOMAIN_MUSIC;
			else if(strncmp(json_object_get_string(json_intent),"news",4) == 0)
				domain_x = DOMAIN_NEWS;
			else if(strncmp(json_object_get_string(json_intent),"playControl",11) == 0)
				domain_x = DOMAIN_COMMAND;
			else if(strncmp(json_object_get_string(json_intent),"volumeControl",13) == 0)
				domain_x = DOMAIN_VOLUMECONTROL;
			else if(strncmp(json_object_get_string(json_intent),"AddReminder",11) == 0)
				domain_x = DOMAIN_REMINDER;
			else
				domain_x = DOMAIN_CHAT;
		}
	}
	my_pr("--------------SWITCH-----------------luobo START :domain_x = %d\n",domain_x);
	
	//create_answer_pthread();
	switch (domain_x)
	{
	case DOMAIN_MUSIC:
		if(!luobo_ret&&json_confidence&&(strcmp("1",json_object_get_string(json_confidence)) == 0)){
			my_pr("------------luobo DOMAIN_MUSIC 111 url = %s\n",json_object_get_string(json_playUrl));
			my_pr("------------luobo DOMAIN_MUSIC 111 singer = %s\n",json_object_get_string(json_singer));
			my_pr("------------luobo DOMAIN_MUSIC 111 song = %s\n",json_object_get_string(json_name));
			memset(pre_singer,0,sizeof(pre_singer));
			strcpy(pre_singer,json_object_get_string(json_singer));
			FILE *fp = NULL;
			fp = fopen("/usr/data/pre.txt","wb");
			if(fp)
				fwrite(pre_singer,1,sizeof(pre_singer),fp);
			fclose(fp);
			mozart_tts_sync(json_object_get_string(json_speechText));
			luobo_cloudmusic_playmusic( json_object_get_string(json_singer),
						json_object_get_string(json_ablumName),
						json_object_get_string(json_name),
						json_object_get_string(json_playUrl));//播放资源歌曲
			//speech_cloudmusic_playmusic(json_object_get_string());
			need_resume_after_asr = 0;
			asr->sds.state = SDS_STATE_DO;
			break;
		}
		if (asr->sds.music.number >= 1) {
			char tips[128] = {};
			int index = 0;
			if (asr->sds.music.number > 1) {
				srandom(time(NULL));
				index = random() % asr->sds.music.number;
			}
			/* play tts */
			my_pr("-------------------------------------DOMAIN_MUSIC 111 url = %s\n",asr->sds.music.data[index].url);
			memset(pre_singer,0,sizeof(pre_singer));
			strcpy(pre_singer,asr->sds.music.data[index].artist);
			FILE *fp = NULL;
			fp = fopen("/usr/data/pre.txt","wb");
			if(fp)
				fwrite(pre_singer,1,sizeof(pre_singer),fp);
			fclose(fp);

			sprintf(tips, "%s,%s", asr->sds.music.data[index].artist, asr->sds.music.data[index].title);
			mozart_tts_sync(tips);
			speech_cloudmusic_playmusic(&asr->sds.music, index);
			need_resume_after_asr = 0;
		} else {
			mozart_tts_sync(asr->sds.output);
		}
		
		break;
	case DOMAIN_NETFM:
		my_pr("------------------------------------DOMAIN_NETFM ");
		if (asr->sds.netfm.number >= 1) {
			int index = 0;
			if (asr->sds.netfm.number > 1) {
				srandom(time(NULL));
				index = random() % asr->sds.netfm.number;
			}
			/* play tts */
			mozart_tts_sync(asr->sds.netfm.data[index].track);

			speech_cloudmusic_playfm(&asr->sds.netfm, index);

			need_resume_after_asr = 0;
		} else {
			/* TODO: command, such as volume control. */
			mozart_tts_sync(asr->sds.output);
		}
		break;
	case DOMAIN_CHAT:
		my_pr("------------------------------------ DOMAIN_CHAT \n");
		if(!luobo_ret&&json_confidence&&(strcmp("1",json_object_get_string(json_confidence)) == 0)){
			if(json_speechText||json_speechUrl){
				if(json_speechText&&(strcmp(json_object_get_string(json_speechText),"null"))){
					my_pr("speechText: %s\n",json_object_get_string(json_speechText));
					mozart_tts_sync(json_object_get_string(json_speechText));
				}

				if(json_speechUrl&&(strcmp(json_object_get_string(json_speechUrl),"null"))){
					my_pr("speechUrl: %s\n",json_object_get_string(json_speechUrl));
					luobo_playurl("fanyi","fanyi","fanyi",json_object_get_string(json_speechUrl));
				}
			}
			else if(json_object_get_string(json_answer)){
				my_pr("answer: %s.\n",json_object_get_string(json_answer));
				mozart_tts_sync(json_object_get_string(json_answer));
			}else{
				asr->sds.state = SDS_STATE_OFFERNONE;	
			}	
			break;
		}
		if (!strcmp(asr->input, "")) {
			no_voice_cnt++;
			if(no_voice_cnt == 3){
				no_voice_cnt = 0;
				mozart_play_key_sync("error_no_voice_quit");
				asr->sds.state = SDS_STATE_OFFER;
				need_resume_after_asr = 0;
				break;
			}else{
				mozart_play_key_sync("error_no_voice");
				asr->sds.state = SDS_STATE_OFFERNONE;
				break;
			}
		}
		if(asr->sds.output){
			my_pr("DOMAIN_CHAT output: %s.\n", asr->sds.output);
			if(strcmp(asr->sds.output,"我还没有学会呢")){
				mozart_tts_sync(asr->sds.output);
				
			}else{
				mozart_play_key_sync("error_invalid_domain");
			}
			asr->sds.state = SDS_STATE_OFFERNONE;
			break;
		}
		if (asr->sds.chat.url) {
			printf("asr->sds.chat.url = %s\n",asr->sds.chat.url);
			speech_cloudmusic_playjoke(asr->sds.chat.url);
			asr->sds.state = SDS_STATE_OFFER;
			need_resume_after_asr = 0;
			break;
		}
		
	case DOMAIN_WEATHER:
	case DOMAIN_CALENDAR:
	case DOMAIN_STOCK:
	case DOMAIN_POETRY:
	case DOMAIN_MOVIE:
		printf("--------------output: %s.\n", asr->sds.output);
		mozart_tts_sync(asr->sds.output);
		break;
	case DOMAIN_REMINDER:
	{
		my_pr("------------------------------------ DOMAIN_REMINDER \n");
		if(!luobo_ret&&json_confidence&&(strcmp("1",json_object_get_string(json_confidence)) == 0)){
			json_object_object_get_ex(json_params,"time",&json_time);	
			json_object_object_get_ex(json_params,"content",&json_content);
			if(json_time&&json_content){
				printf("reminder time = %s\n",json_object_get_string(json_time));
				printf("reminder content = %s\n",json_object_get_string(json_content));
				remind_data = (REMIND_DATAS *)calloc(1, sizeof(REMIND_DATAS));
				if(remind_data){
					remind_data->seconds = get_remind_seconds(json_object_get_string(json_time));
					remind_data->content = json_object_get_string(json_content);
					if (pthread_create(&remind_pthread[remind_pthread_cnt], NULL, remind_func, remind_data) != 0) {
						printf("create remind_pthread failed !\n");
					}
					pthread_detach(remind_pthread[remind_pthread_cnt]);
					remind_pthread_cnt++;
				}
				mozart_tts_sync("好的，我已经设置了");
				mozart_tts_wait();
				
			}else{
				printf("Reminder time content Err!\n");
				mozart_play_key_sync("error_invalid_domain");
			}

		}
	}
		break;
	
	case DOMAIN_COMMAND:   // 命令直接返回
		my_pr("------------------------------------ DOMAIN_COMMAND \n");
		process_vr_asr_command(asr);
		result.vr_next = VR_TO_AEC;
		wake_flage = 0;
		wakebefore_status = 0;
		return result;
		break;
	case DOMAIN_NEWS:
		my_pr("------------------------------------ DOMAIN_NEWS \n");
		if(!luobo_ret&&json_confidence&&(strcmp("1",json_object_get_string(json_confidence)) == 0)){
			json_object_object_get_ex(json_params,"news",&json_news_array);
			json_news = json_object_array_get_idx(json_news_array, 0);
			json_object_object_get_ex(json_news,"name",&json_name);
			json_object_object_get_ex(json_news,"playUrl",&json_playUrl);

			my_pr("luobo DOMAIN_MUSIC  url = %s\n",json_object_get_string(json_playUrl));
			my_pr("luobo DOMAIN_MUSIC  song = %s\n",json_object_get_string(json_name));

			mozart_tts_sync(json_object_get_string(json_name));
			luobo_cloudmusic_playmusic( "news","news",
						json_object_get_string(json_name),
						json_object_get_string(json_playUrl));//播放资源歌曲
			need_resume_after_asr = 0;
			asr->sds.state = SDS_STATE_DO;
			break;
		}
		asr->sds.state == SDS_STATE_INTERACT;
		break;
	case DOMAIN_VOLUMECONTROL:
		my_pr("------------------------------------ DOMAIN_VOLUMECONTROL \n");
		if(!luobo_ret&&json_confidence&&(strcmp("1",json_object_get_string(json_confidence)) == 0)){
			json_object_object_get_ex(json_params,"volume",&json_volume);

			my_pr("------------luobo DOMAIN_VOLUMECONTROL 111 url = %s\n",json_object_get_string(json_volume));
			if(strcmp("max",json_object_get_string(json_volume)) == 0){
				mozart_volume_set(100, MUSIC_VOLUME);
				mozart_play_key_sync("volume_max");
			}
			if(strcmp("min",json_object_get_string(json_volume)) == 0){
				mozart_volume_set(5, MUSIC_VOLUME);
				mozart_play_key_sync("volume_min");
			}
			if(strcmp("up",json_object_get_string(json_volume)) == 0){
				mozart_volume_up();
				mozart_play_key_sync("volume_up");
			}
			if(strcmp("down",json_object_get_string(json_volume)) == 0){
				mozart_volume_down();
				mozart_play_key_sync("volume_down");
			}
			need_resume_after_asr = 0;
			asr->sds.state = SDS_STATE_DO;
			break;
		}
		asr->sds.state == SDS_STATE_INTERACT;
		break;
	case DOMAIN_QUIT:
		
		break;
	default:
		
		if(g_translate_flag){
			g_translate_flag = 0;
			char *output = NULL;
			translate_zh_en(g_translate_input);
			output = get_object_string("trans_result",NULL,translate_buf);
			if(output){
				//printf("###output = %s\n",output);
				output = get_object_string("dst",NULL,output);
				printf("=====>output = %s\n",output);
			}
			mozart_tts(output);
		}else if(asr->sds.output){
			mozart_tts_sync(asr->sds.output);
		}else{
			mozart_play_key_sync("error_invalid_domain");
			asr->sds.state = SDS_STATE_OFFER;
			need_resume_after_asr = 1;
		}
		break;
	}
	//pthread_cancel(breath_answer_t);
	//breath_answer_flag = 0;

	my_pr("--------------SWITCH-----------------END\n");

	if ((asr->sds.state == SDS_STATE_OFFERNONE ||asr->sds.state == SDS_STATE_INTERACT)
		&&need_resume_after_asr != -1) {//多次
		printf("[NEXT ASR]\n");
		result.vr_next = VR_TO_ASR_SDS;
		mozart_play_key_sync("asr_continue");
	} else {
		if (need_resume_after_asr == 1) {
			#if 1
			result.vr_next = VR_TO_ASR_SDS;
			mozart_play_key_sync("asr_continue");
			return result;
			#endif
		}else if(need_resume_after_asr == 0){
			printf(" [QUIT ASR] 3333\n");
			wake_flage = 0;
			wakebefore_status = 0;
			mozart_play_pause();
		}else if(need_resume_after_asr == -1){
			usleep(100*1000);   // 特殊ASR打断，状态切换不能过快，要等mozart_vr_asr_break（）执行完
			printf("555 [QUIT ASR] 555\n");
			wake_flage = 0;
			wakebefore_status = 0;
		}
		result.vr_next = VR_TO_AEC;
	}
	return result;
}

#define LIGHT_PURPLE "\033[1;35m"
#define NONE          "\033[m"
#define my_pr(fmt, args...)		printf(LIGHT_PURPLE fmt NONE,##args)


static int process_vr_content_callback(content_info_t *content)
{
	memory_domain domain;
	int ret = share_mem_get_active_domain(&domain);


	if (ret) {
		printf("get active domain error in %s:%s:%d, do nothing.\n",
		       __FILE__, __func__, __LINE__);
		
	}
	printf("????   domain = %d\n",domain);
	printf("---->content result, domain: %s.\n", vr_domain_str[content->sds.domain]);

	switch (content->sds.domain) {
	case DOMAIN_MUSIC:
		{	printf("------DOMAIN_MUSIC--------\n");
			if (content->sds.music.number >= 1) {
				char tips[128] = {};
				int index = 0;
				if (content->sds.music.number > 1) {
					srandom(time(NULL));
					index = random() % content->sds.music.number;
				}
				/* play tts */
				sprintf(tips, "%s,%s", content->sds.music.data[index].artist, content->sds.music.data[index].title);
				
				my_pr("tips = %s,pre_singer = %s,index = %d\n",tips,pre_singer,index);
				mozart_tts_sync(tips);
				speech_cloudmusic_playmusic(&content->sds.music, index);
			}
		}
		break;
	case DOMAIN_NETFM:
			printf("------DOMAIN_NETFM--------\n");
		if (content->sds.netfm.number >= 1) {
			int index = 0;
			if (content->sds.netfm.number > 1) {
				srandom(time(NULL));
				index = random() % content->sds.netfm.number;
			}
			/* play tts */
			//mozart_tts_sync(content->sds.netfm.data[index].track);

			speech_cloudmusic_playfm(&content->sds.netfm, index);
			
		} else {
			/* TODO: command, such as volume control. 网络不好搜索不到资源*/
			mozart_tts_sync(content->sds.output);
			mozart_module_cloud_music_startup();
		}

		break;
	case DOMAIN_CHAT:
		printf("------DOMAIN_CHAT--------@ content->sds.chat.url = %s\n",content->sds.chat.url);

		if (content->sds.chat.url) {
			/* joke, is offer */	
			speech_cloudmusic_playjoke(content->sds.chat.url);
	
		}else{
			mozart_play_key_sync("badnet");
			//mozart_snd_source_switch();
			mozart_module_cloud_music_startup();
		}
		break;
	default:
//		mozart_play_key_sync("error_net_fail");
//		mozart_snd_source_switch();
		mozart_play_key_sync("badnet");
		system("rm /usr/data/pre.txt");//删除不佳资源
		mozart_module_cloud_music_startup();
		printf("Unhandled domain: %s.\n", vr_domain_str[content->sds.domain]);
		break;
	}

	return 0;
}

static void *process_vr_content_func(void *arg)
{
	pthread_detach(pthread_self());

	vr_info_t *vr_info = (vr_info_t *)arg;

	if (vr_info->content.state == CONTENT_SUCCESS) {
		process_vr_content_callback(&vr_info->content);
	} else if (vr_info->content.state == CONTENT_FAIL) {
		printf("content errId: %d, error: %s\n",
			   vr_info->content.errId, vr_info->content.error);
		switch (vr_info->content.errId) {
		case 70604:
		case 70605:
			mozart_play_key_sync("error_net_fail");
			break;
		case 70603:
		case 70613:
			mozart_play_key_sync("error_net_slow_wait");
			break;

		default:
			printf("process_vr_content_func errId: %d, error: %s\n", vr_info->asr.errId, vr_info->asr.error);
			//mozart_play_key_sync("error_server_busy");
			break;
		}

	} else if (vr_info->content.state == CONTENT_INTERRUPT) {
		printf("get content request has been break.\n");
	}
	content_free(&vr_info->content);

	return NULL;
}

vr_result_t process_vr_callback(vr_info_t *vr_info)
{	
	vr_result_t result;
	printf("--------------process_vr_callback---------\n");
	
	memset(&result, 0, sizeof(result));

	if (vr_info->from == VR_FROM_AEC) {
		printf("---1111---VR_FROM_AEC---\n");
		return process_vr_aec_callback(&vr_info->aec);
	} else if (vr_info->from == VR_FROM_ASR) {
		printf("--22222- -- VR_FROM_ASR---errId = %d\n",vr_info->asr.errId);
		switch (vr_info->asr.errId) {
		case -1:
		case 70604:
		case 70605:
			mozart_play_key_sync("error_net_fail");
			break;
		case 70610:
			//换成定制版本后 开机会跳这里一次 errid = 70610 ,暂时屏蔽
			break;
		case 70603:
		case 70613:
			mozart_play_key_sync("error_net_slow_wait");
			break;
		case 0:
			if (vr_info->asr.state == ASR_BREAK) {
				printf("asr interrupt.\n");
				break;
			} else {
				return process_vr_asr_callback(&vr_info->asr);
			}
		default:
			printf("process_vr_callback errId: %d, error: %s\n", vr_info->asr.errId, vr_info->asr.error);
			//mozart_play_key_sync("error_server_busy");
			break;
		}
		result.vr_next = VR_TO_AEC;
		return result;
	} else if (vr_info->from == VR_FROM_CONTENT) {
		printf("-333333---VR_FROM_CONTENT--\n");
		result.vr_next = VR_TO_NULL;
		pthread_t process_vr_content_thread;

		if (pthread_create(&process_vr_content_thread, NULL, process_vr_content_func, vr_info) == -1)
			printf("Create process vr content pthread failed: %s.\n", strerror(errno));

		/* return-ed result will be ignored */
		return result;
	} else {
		printf("Unsupport callback source: %d, back to AEC mode.\n", vr_info->from);
		result.vr_next = VR_TO_AEC;
		return result;
	}
}
