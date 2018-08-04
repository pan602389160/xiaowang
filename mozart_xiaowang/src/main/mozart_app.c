#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <errno.h>
#include <time.h>



#include "json-c/json.h"
#include "json-c/json_util.h"
#include "utils_interface.h"
#include "sharememory_interface.h"
#include "wifi_interface.h"
#include "localplayer_interface.h"

#include "mozart_config.h"
#include "lapsule_control.h"
#include "ingenicplayer.h"
#include "nvrw_interface.h"
#if (SUPPORT_DMS == 1)
#include "dms.h"
#endif

#include "mozart_app.h"
#include "mozart_key_function.h"
#include "mozart_musicplayer.h"

#ifdef SUPPORT_BT
#include "modules/mozart_module_bt.h"
#endif
#ifdef SUPPORT_VR
#include "modules/mozart_module_vr.h"
#endif
#if (SUPPORT_DMR == 1)
#include "modules/mozart_module_dmr.h"
#endif
#include "modules/mozart_module_airplay.h"
#include "modules/mozart_module_cloud_music.h"

#if (SUPPORT_MULROOM == 1)
#include "mulroom_interface.h"
#endif

#if (SUPPORT_DMS == 1)
bool dms_started = false;
#endif

#if (SUPPORT_AIRPLAY == 1)
bool airplay_started = false;
#endif

#if (SUPPORT_LOCALPLAYER == 1)
bool localplayer_started = false;
#endif

#ifdef SUPPORT_BT
bool bt_started = false;
#endif

#ifdef SUPPORT_VR
bool vr_started = false;
bool vr_starting = false;
#endif

#if (SUPPORT_INGENICPLAYER == 1)
bool ingenicplayer_started = false;
#endif

__attribute__((unused)) static bool lapsule_started = false;

int start(app_t app)
{	printf("========start===================>>>>>>app = %d\n",app);
	switch(app){
	case MUSICPLAYER:
		mozart_musicplayer_startup();
		break;
#if (SUPPORT_DMR == 1)
	case DMR:
		if (dmr_is_started())
			mozart_render_refresh();
		else
			dmr_start();
		break;
#endif
#if (SUPPORT_DMS == 1)
	case DMS:
		if (dms_started) {
			stop(app);
			dms_started = false;
		}

		if (start_dms()) {
                        printf("ushare service start failed.\n");
			return -1;
		}

		dms_started = true;
		return 0;
#endif
	case DMC:
		break;
#if (SUPPORT_AIRPLAY == 1)
	case AIRPLAY:
		if (airplay_started) {
			stop(app);
			airplay_started = false;
		}

		mozart_airplay_start_service();

		airplay_started = true;
		return 0;
#endif
#if (SUPPORT_LOCALPLAYER == 1)
	case LOCALPLAYER:
		if (localplayer_started) {
			stop(app);
			localplayer_started = false;
		}

		if (mozart_localplayer_startup()) {
			return -1;
		}

		localplayer_started = true;
		return 0;
#endif
#ifdef SUPPORT_BT
	case BT:
		if (bt_started) {
			stop(app);
			bt_started = false;
		}

		start_bt();
		bt_started = true;
		return 0;
#endif

#ifdef SUPPORT_VR
	case VR:
		if(vr_starting){
			printf("vr starting .....\n");
			return -1;
		}
		if (vr_started) {
			stop(app);
			vr_started = false;
		}
		#if 1
		vr_starting = true;
		if (mozart_vr_init(process_vr_callback)) {
			printf("init error, vr disabled!!\n");
			mozart_play_key_sync("error_net_fail");
			vr_starting = false;
			return -1;
		}

		if (mozart_vr_start()) {
			printf("start error, vr disabled!!\n");
			mozart_vr_uninit();
			mozart_play_key_sync("error_net_fail");
			vr_starting = false;
			return -1;
		}

		vr_started = true;
		vr_starting = false;
		#endif
		break;
#endif
		
#if (SUPPORT_INGENICPLAYER == 1)
	case INGENICPLAYER:
		if (ingenicplayer_started) {
			stop(app);
			ingenicplayer_started = false;
		}
		if (mozart_ingenicplayer_startup()) {
			printf("start ingenicplayer failed in %s: %s.\n", __func__, strerror(errno));
			return -1;
		}
		ingenicplayer_started = true;
		break;
#endif

#if (SUPPORT_LAPSULE == 1)
	case LAPSULE:
		if (lapsule_started) {
			stop(app);
			lapsule_started = false;
		}

                if (start_lapsule_app()) {
                        printf("start lapsule services failed in %s: %s.\n", __func__, strerror(errno));
			return -1;
                }

		lapsule_started = true;
		return 0;
#endif

#if (SUPPORT_MULROOM == 1)
	case MULROOM:
		break;
#endif

	default:
		printf("WARNING: Not support this module(id = %d)\n", app);
		break;
	}

	return 0;
}

int stop(app_t app)
{
	switch(app){
	case MUSICPLAYER:
		mozart_musicplayer_shutdown();
		break;
#if (SUPPORT_DMR == 1)
	case DMR:
		mozart_render_terminate_fast();
		break;
#endif
#if (SUPPORT_DMS == 1)
	case DMS:
		if (dms_started) {
			stop_dms();
			dms_started = false;
		}
		break;
#endif
	case DMC:
		break;
#if (SUPPORT_AIRPLAY == 1)
	case AIRPLAY:
		if (airplay_started) {
			mozart_airplay_shutdown();
			airplay_started = false;
		}
		break;
#endif
#if (SUPPORT_LOCALPLAYER == 1)
	case LOCALPLAYER:
		if (localplayer_started) {
			mozart_localplayer_shutdown();
			localplayer_started = false;
		}
		break;
#endif
#ifdef SUPPORT_BT
	case BT:
		if (bt_started) {
			stop_bt();
			bt_started = false;
		}
		break;
#endif
#ifdef SUPPORT_VR
	case VR:
		mozart_vr_stop();
		mozart_vr_uninit();

		vr_started = false;
		break;
#endif

#if (SUPPORT_INGENICPLAYER == 1)
	case INGENICPLAYER:
		if (ingenicplayer_started) {
			mozart_ingenicplayer_shutdown();
			ingenicplayer_started = false;
		}
		break;
#endif

#if (SUPPORT_LAPSULE == 1)
	case LAPSULE:
		if (lapsule_started) {
			stop_lapsule_app();
			lapsule_started = false;
		}
		break;
#endif

#if (SUPPORT_MULROOM == 1)
	case MULROOM:
		mozart_mulroom_dismiss_group();
		break;
#endif

	default:
		printf("WARNING: Not support this module(id = %d)\n", app);
		break;
	}

	return 0;
}
int rayshine_appserver_rename()
{
	char devicename[64] = {0};
	char id_buf[32] = {0};
	int fd = open("/usr/data/mac.txt",O_RDONLY);
	if(fd > 0){
		read(fd,id_buf,sizeof(id_buf));
		strncpy(devicename, id_buf,strlen(id_buf));
		devicename[strlen(id_buf)] = '\0';
		close(fd);
	}else{
		sprintf(devicename,"%s","tmp_ID");
	}
	mozart_appserver_rename(devicename);

}
#if 0
#define SECRETKEY "f0d3955f36b112c2be25412ab3fd1accad1805b2"
#define LUOBO_UPLOAD_URL "http://test-service.luobotec.com/api/monitor/device_status/";
#define LUOBO_APPKEY     "AppKey:luobo_story_ruixin"

extern int set_battery;

char luobo_upload_buf[1024] = {0};
size_t upload_nn = 0;
size_t upload_write_data(void * ptr,size_t size,size_t nmemb,void *user_p)
{
	FILE *fp = (FILE *)user_p;
	size_t return_size = fwrite(ptr,size,nmemb,fp);
	upload_nn += return_size;
	return return_size;
}

int get_ssid(char *ssid)
{
	int ret = -1;
	FILE *fp = NULL;
	char ch_tmp[32] = {0};

	fp = popen("iwgetid | cut -f 2 -d \":\" |cut -f 2 -d \"\\\"\"", "r");
	if (fp != NULL) {
		ret = fread(ch_tmp, sizeof(char), sizeof(ch_tmp), fp);
		if (ret > 0)
			strncpy(ssid, ch_tmp, ret - 1);
		fclose(fp);
		fp = NULL;
	}
	return 0;
}
int get_device_id(char *device_id)
{

	int fd = open("/usr/data/mac.txt",O_RDONLY);
	if(fd > 0){
		char id_buf[32] = {0};
		read(fd,id_buf,sizeof(id_buf));
		strncpy(device_id, id_buf,strlen(id_buf));
		close(fd);
	}
	return 0;
}
int get_battery(char *battery)
{
	sprintf(battery,"%d",set_battery);
	return 0;
}
int get_firmware_version(char *firmware_version)
{
	int nvrw_lock = -1;
	struct nv_info *info = NULL;

	nvrw_lock = mozart_nv_lock();
	if ((info = mozart_nv_get_nvinfo()) != NULL) {
		strcpy(firmware_version, info->current_version);
		free(info);
	}
	mozart_nv_unlock(nvrw_lock);
	return 0;
}

int get_mac(char *mac)
{
	char macaddr[] = "255.255.255.255";
	char buf[16] = {0};
	memset(macaddr, 0, sizeof(macaddr));
	get_mac_addr("wlan0", macaddr, "");
	sprintf(buf,"%c%c:%c%c:%c%c:%c%c:%c%c:%c%c",
		macaddr[0],macaddr[1],macaddr[2],macaddr[3],macaddr[4],macaddr[5],
		macaddr[6],macaddr[7],macaddr[8],macaddr[9],macaddr[10],macaddr[11]);
	strncpy(mac,buf,strlen(buf));
	return 0;
}

static void *sync_time_func()
{
#if (SUPPORT_MULROOM == 1)
	module_mulroom_run_ntpd();
#else
	mozart_system("ntpd -nq");
#endif

#if (SUPPORT_ALARM == 1)
	mozart_alarm_set_rtc_time(time(NULL));
#else
	mozart_system("hwclock -w -u -f /dev/rtc0");
#endif

	return NULL;
}


int luobo_upload_devinfo()
{
	if(access("/usr/data/mac.txt",F_OK)){
		printf("/usr/data/mac.txt not exist\n");
		return 0;
	}
	int ret = -1;
	CURL *curl;
	CURLcode curl_code;
	
	struct curl_httppost *post=NULL;
	struct curl_httppost *last=NULL;
	struct curl_slist * list = NULL;

	FILE *fp = NULL;
	time_t now;
	
	sync_time_func();
	int rand_num = rand();
	

	char cur_time[32] = {0};
	char nonce[16] = {0};
	char sha1sum_string[256] = {0};
	char checksum_tmp[64] = {0};
	char checksum[64] = {0};

	
	char ssid[32] = {0};
	char device_id[32] = {0};
	char firmware_version[32] = {0};
	char mac[32] = {0};
	char battery[32] = {0};

	get_ssid(ssid);
	get_device_id(device_id);
	get_firmware_version(firmware_version);
	get_mac(mac);
	get_battery(battery);
#if 0
	printf("ssid = %s\n",ssid);
	printf("device_id = %s\n",device_id);
	printf("firmware_version = %s\n",firmware_version);
	printf("mac = %s\n",mac);
	printf("battery = %s\n",battery);
#endif	
	curl_global_init(CURL_GLOBAL_ALL);

	curl_formadd(&post,
                 &last,
                 CURLFORM_COPYNAME, "wifi",
                 CURLFORM_COPYCONTENTS, ssid, CURLFORM_END);
	curl_formadd(&post,
                 &last,
                 CURLFORM_COPYNAME, "device_id",
                 CURLFORM_COPYCONTENTS, device_id, CURLFORM_END);
	curl_formadd(&post,
                 &last,
                 CURLFORM_COPYNAME, "battery",
                 CURLFORM_COPYCONTENTS, battery, CURLFORM_END);
	curl_formadd(&post,
                 &last,
                 CURLFORM_COPYNAME, "firmware_version",
                 CURLFORM_COPYCONTENTS, firmware_version, CURLFORM_END);
	curl_formadd(&post,
                 &last,
                 CURLFORM_COPYNAME, "mac",
                 CURLFORM_COPYCONTENTS, mac, CURLFORM_END);
	
	
	now = time(NULL);
	struct tm  *tp = localtime(&now);
	sprintf(cur_time,"CurTime:%d",now);
	sprintf(nonce,"Nonce:%d",rand_num);
	printf("%d-%d-%d-", tp->tm_mon+1, tp->tm_mday, tp->tm_year + 1900);  
	printf("%d:%d:%d\n", tp->tm_hour, tp->tm_min, tp->tm_sec);


	int fd = open("/tmp/sha1sum.txt",O_WRONLY|O_CREAT);
	if(fd > 0){
		sprintf(sha1sum_string,"%s%d%d",SECRETKEY,rand_num,now);
		ret = write(fd,sha1sum_string,strlen(sha1sum_string));
		printf("ret = %d\n",ret);
	}

	fp = popen("sha1sum /tmp/sha1sum.txt | cut -f 1 -d \" \"", "r");
	if (fp != NULL) {
		ret = fread(checksum_tmp, sizeof(char), sizeof(checksum_tmp), fp);
		if (ret > 0)
			snprintf(checksum,strlen(checksum_tmp) + 9,"CheckSum:%s",checksum_tmp);
		fclose(fp);
		fp = NULL;
	}

#if 1
	printf("cur_time = %s\n",cur_time);
	printf("nonce = %s\n",nonce);
	printf("checksum = %s\n",checksum);
#endif


	fp = NULL;	
	fp = fopen("/tmp/luobo_upload.txt","wb+");
	curl = curl_easy_init();
	if(curl) {
		char myurl[256] = LUOBO_UPLOAD_URL;

		list = curl_slist_append(list,LUOBO_APPKEY);
		list = curl_slist_append(list,cur_time);
		list = curl_slist_append(list,nonce);
		list = curl_slist_append(list,checksum);

		curl_easy_setopt(curl,CURLOPT_HTTPHEADER,list);
		curl_easy_setopt(curl,CURLOPT_URL,myurl);
		curl_easy_setopt(curl,CURLOPT_HTTPPOST, post);
		curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);	
		curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, 2000L);
	
		curl_easy_setopt(curl,CURLOPT_WRITEFUNCTION,upload_write_data);
		curl_easy_setopt(curl,CURLOPT_WRITEDATA,fp);
		curl_code = curl_easy_perform(curl);
		if(curl_code != CURLE_OK){
			curl_easy_cleanup(curl);
			fclose(fp);
			return -1;
		}
		curl_slist_free_all(list);
		curl_easy_cleanup(curl);
	}
	fclose(fp);

	fp = NULL;
	fp = fopen("/tmp/luobo_upload.txt","rb");
	memset(luobo_upload_buf,0,sizeof(luobo_upload_buf));
	fread(luobo_upload_buf,1,sizeof(luobo_upload_buf),fp);
	fclose(fp);

	printf(" luobo_upload_buf = %s\n",luobo_upload_buf);
	if(strlen(luobo_upload_buf) == 0)
		return -1;
	return 0;
}

#endif
int startall(int depend_network)
{	printf("========startall=================>>>>>>depend_network = %d\n",depend_network);
	if (depend_network == -1 || depend_network == 1) {
		rayshine_appserver_rename();
		//luobo_upload_devinfo();
#if (SUPPORT_DMR == 1)
		start(DMR);
#endif
#if (SUPPORT_DMS == 1)
		start(DMS);
#endif
		start(DMC);
#if (SUPPORT_AIRPLAY == 1)
		start(AIRPLAY);
#endif
#ifdef SUPPORT_VR
	wifi_info_t infor = get_wifi_mode();
	int cnt = 0;
	while(1){
		sleep(1);
		infor = get_wifi_mode();
		printf("------wifi_mode = %d\n",infor.wifi_mode);	
		if (infor.wifi_mode == STA){
			start(VR);
			mozart_module_cloud_music_startup();
			break;
		}else{
			if(cnt++ > 10)
				break;
			else
				continue;
		}
	}
#endif
#if (SUPPORT_LAPSULE == 1)
		start(LAPSULE);
#endif

#if (SUPPORT_INGENICPLAYER == 1)
		start(INGENICPLAYER);
		mozart_appserver_register_callback(1 * 1024 * 1024, appserver_cmd_callback);
#endif

	}

	if (depend_network == -1 || depend_network == 0) {
		start(MUSICPLAYER);
#ifdef SUPPORT_BT
		start(BT);
#endif
#if (SUPPORT_LOCALPLAYER == 1)
		start(LOCALPLAYER);
#endif
	}

	return 0;
}

int stopall(int depend_network)
{
	if (depend_network == -1 || depend_network == 1) {
#if (SUPPORT_DMR == 1)
		stop(DMR);
#endif
#if (SUPPORT_DMS == 1)
		stop(DMS);
#endif
#if (SUPPORT_AIRPLAY == 1)
		stop(AIRPLAY);
#endif
#ifdef SUPPORT_VR
		stop(VR);
#endif
#if (SUPPORT_INGENICPLAYER == 1)
		stop(INGENICPLAYER);
		mozart_appserver_shutdown();
#endif
	}
	if (depend_network == -1 || depend_network == 0) {
#ifdef SUPPORT_BT
		stop(BT);
#endif
#if (SUPPORT_LOCALPLAYER == 1)
		stop(LOCALPLAYER);
#endif
		stop(MUSICPLAYER);
	}

#if (SUPPORT_MULROOM == 1)
	if (depend_network == 1)
		stop(MULROOM);
#endif

	return 0;
}

int restartall(int depend_network)
{
	stopall(depend_network);
	startall(depend_network);

	return 0;
}


