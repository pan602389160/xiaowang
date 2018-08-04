#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <stdbool.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>
#include <time.h>
#include <sys/types.h>
#include <pwd.h>
#include <sys/stat.h>
#include <linux/input.h>
#include <fcntl.h>
#include <execinfo.h>
#include <sys/syscall.h>
#include <curl/curl.h>

#include "event_interface.h"
#include "wifi_interface.h"
#include "volume_interface.h"
#include "player_interface.h"
#include "tips_interface.h"
#include "json-c/json.h"
#include "utils_interface.h"
#include "power_interface.h"
#include "sharememory_interface.h"
#include "ini_interface.h"
#include "nvrw_interface.h"
#include "updater_interface.h"
#include "localplayer_interface.h"

#include "mozart_config.h"
#include "lapsule_control.h"
#include "battery_capacity.h"
#include "ingenicplayer.h"

#include "mozart_app.h"
#include "mozart_ui.h"
#include "mozart_musicplayer.h"
#include "mozart_key_function.h"
#include "mozart_face.h"
#include "client_shared.h"
#include "asr.h"
#ifdef SUPPORT_BT
#include "modules/mozart_module_bt.h"
#endif
#ifdef SUPPORT_VR
#include "modules/mozart_module_vr.h"
#include "vr_interface.h"
#endif

#if (SUPPORT_MULROOM == 1)
#include "mulroom_interface.h"
#include "modules/mozart_module_mulroom.h"
#endif /* SUPPORT_MULROOM == 1 */

#include "modules/mozart_module_linein.h"
#include "modules/mozart_module_local_music.h"

#if (SUPPORT_ALARM == 1)
#include "alarm_interface.h"
#endif
#include "inno_gpio_key.h"

event_handler *keyevent_handler = NULL;
event_handler *miscevent_handler = NULL;
char *app_name = NULL;
int tfcard_status = -1;
static int null_cnt = 0;

//#include <linux/ioctl.h> /* needed for the _IOW etc stuff used later */

/*
 * Ioctl definitions
 */

/* Use 'k' as magic number */
#define HT1628_IOC_MAGIC  	'H'
#define HT1628_IOC_MAXNR 	6

/* Please use a different 8-bit number in your code */

//#define INNO_IOCTL_HT1628_WRITE_DATA     _IOW(HT1628_IOC_MAGIC,  1, struct inno_mfi_msg *)
//#define INNO_IOCTL_HT1628_WRITE_ADDR_DATA     _IOW(HT1628_IOC_MAGIC, 2, struct inno_mfi_msg *)

int breath_battery_flag = 0;
pthread_t breath_battery_t;



int breath_wifi_config_flag = 0;
pthread_t breath_wifi_config_t;

int breath_pause_flag = 0;
pthread_t breath_pause_t;

static int low_power_flag = 0;
#if (SUPPORT_MULROOM == 1)
#define MULROOM_INFO_PATH	"/usr/data/mulroom.json"
static enum mul_state mulroom_state = MUL_IDLE;
#endif /* SUPPORT_MULROOM == 1 */

#if (SUPPORT_BT == BT_BCM)
#define APP_HS_CALL_VOLUME_MAX  15
#define APP_AVK_VOLUME_MAX      17
int bsa_ble_start_regular_enable = 0;
int bsa_start_regular_enable = 0;
int ble_service_create_enable = 0;
int ble_service_add_char_enable = 0;
int ble_service_start_enable = 0;
int ble_client_open_enable = 0;
int ble_client_write_enable = 0;
int ble_hh_add_dev_enable = 0;
int bt_link_state = BT_LINK_DISCONNECTED;
pthread_t bt_reconnect_pthread;
pthread_t bt_ring_pthread;
int bt_reconnect_num = 0;
bool bt_ring_flag;
int disc_num = -1;
pthread_mutex_t bt_reconnect_lock;
static UINT8 avk_volume_set_dsp[APP_AVK_VOLUME_MAX] = {0, 6, 12, 18, 25, 31, 37, 43, 50, 56, 62, 68, 75, 81, 87, 93, 100};
static UINT8 avk_volume_set_phone[APP_AVK_VOLUME_MAX] = {0, 7, 15, 23, 31, 39, 47, 55, 63, 71, 79, 87, 95, 103, 111, 119, 127};
#if (SUPPORT_BSA_BLE == 1)
extern ble_create_service_data ble_heart_rate_service;
extern ble_add_char_data ble_heart_rate_measure_char;
extern ble_add_char_data ble_notify_char;
#endif /* SUPPORT_BSA_BLE */

enum {
	DG_IDLE,
	DG_CONNECTED,
	DG_CONNECT_FAILED,
	DG_DISCONNECTED,
};
int bt_dg_flag = DG_IDLE;
#if (SUPPORT_BSA_A2DP_SOURCE == 1)
extern int avk_source_set_audio_output();
#endif /* SUPPORT_BSA_A2DP_SOURCE */

#if (SUPPORT_BSA_PBC == 1)
enum {
	TEL_ICH_PATH,		/* Path to local incoming calls history object */
	TEL_OCH_PATH,		/* Path to local outgoing calls history object */
	TEL_MCH_PATH,		/* Path to local missed calls history object */
	TEL_CCH_PATH,		/* Path to local combined calls history object */
	TEL_PB_PATH,		/* Path to local main phone book object */
	SIM1_TEL_ICH_PATH,	/* Path to SIM Card incoming calls history object */
	SIM1_TEL_OCH_PATH,	/* Path to SIM Card outgoing calls history object */
	SIM1_TEL_MCH_PATH,	/* Path to SIM Card missed calls history object */
	SIM1_TEL_CCH_PATH,	/* Path to SIM Card combined calls history object */
	SIM1_TEL_PB_PATH,	/* Path to SIM Card main phone book object */
};

static char *tel_str[] = {
	[TEL_ICH_PATH] 	     = "telecom/ich.vcf",       [TEL_OCH_PATH]       = "telecom/och.vcf",
	[TEL_MCH_PATH] 	     = "telecom/mch.vcf", 	[TEL_CCH_PATH]       = "telecom/cch.vcf",
	[TEL_PB_PATH] 	     = "telecom/pb.vcf",        [SIM1_TEL_ICH_PATH]  = "SIM1/telecom/ich.vcf",
	[SIM1_TEL_OCH_PATH]  = "SIM1/telecom/och.vcf",  [SIM1_TEL_MCH_PATH]  = "SIM1/telecom/mch.vcf",
	[SIM1_TEL_CCH_PATH]  = "SIM1/telecom/cch.vcf",  [SIM1_TEL_PB_PATH]   = "SIM1/telecom/pb.vcf",
};
#endif /* SUPPORT_BSA_PBC */
#endif /* SUPPORT_BT */

static char *signal_str[] = {
	[1] = "SIGHUP",       [2] = "SIGINT",       [3] = "SIGQUIT",      [4] = "SIGILL",      [5] = "SIGTRAP",
	[6] = "SIGABRT",      [7] = "SIGBUS",       [8] = "SIGFPE",       [9] = "SIGKILL",     [10] = "SIGUSR1",
	[11] = "SIGSEGV",     [12] = "SIGUSR2",     [13] = "SIGPIPE",     [14] = "SIGALRM",    [15] = "SIGTERM",
	[16] = "SIGSTKFLT",   [17] = "SIGCHLD",     [18] = "SIGCONT",     [19] = "SIGSTOP",    [20] = "SIGTSTP",
	[21] = "SIGTTIN",     [22] = "SIGTTOU",     [23] = "SIGURG",      [24] = "SIGXCPU",    [25] = "SIGXFSZ",
	[26] = "SIGVTALRM",   [27] = "SIGPROF",     [28] = "SIGWINCH",    [29] = "SIGIO",      [30] = "SIGPWR",
	[31] = "SIGSYS",      [34] = "SIGRTMIN",    [35] = "SIGRTMIN+1",  [36] = "SIGRTMIN+2", [37] = "SIGRTMIN+3",
	[38] = "SIGRTMIN+4",  [39] = "SIGRTMIN+5",  [40] = "SIGRTMIN+6",  [41] = "SIGRTMIN+7", [42] = "SIGRTMIN+8",
	[43] = "SIGRTMIN+9",  [44] = "SIGRTMIN+10", [45] = "SIGRTMIN+11", [46] = "SIGRTMIN+12", [47] = "SIGRTMIN+13",
	[48] = "SIGRTMIN+14", [49] = "SIGRTMIN+15", [50] = "SIGRTMAX-14", [51] = "SIGRTMAX-13", [52] = "SIGRTMAX-12",
	[53] = "SIGRTMAX-11", [54] = "SIGRTMAX-10", [55] = "SIGRTMAX-9",  [56] = "SIGRTMAX-8", [57] = "SIGRTMAX-7",
	[58] = "SIGRTMAX-6",  [59] = "SIGRTMAX-5",  [60] = "SIGRTMAX-4",  [61] = "SIGRTMAX-3", [62] = "SIGRTMAX-2",
	[63] = "SIGRTMAX-1",  [64] = "SIGRTMAX",
};
int g_prevolume = 0;
static void usage(const char *app_name)
{
	printf("%s [-bsh] \n"
		   " -h     help (show this usage text)\n"
		   " -s/-S  TODO\n"
		   " -b/-B  run a daemon in the background\n", app_name);

	return;
}

char buf[16] = {};

void sig_handler(int signo)
{	printf("############################################################qwer\n\n");
	char cmd[64] = {};
	void *array[10];
	int size = 0;
	char **strings = NULL;
	int i = 0;

	printf("\n\n[%s: %d] mozart crashed by signal %s.\n", __func__, __LINE__, signal_str[signo]);

	printf("Call Trace:\n");
	size = backtrace(array, 10);
	strings = backtrace_symbols(array, size);
	if (strings) {
		for (i = 0; i < size; i++)
			printf ("  %s\n", strings[i]);
		free (strings);
	} else {
		printf("Not Found\n\n");
	}

	if (signo == SIGSEGV || signo == SIGBUS ||
		signo == SIGTRAP || signo == SIGABRT) {
		sprintf(cmd, "cat /proc/%d/maps", getpid());
		printf("Process maps:\n");
		system(cmd);
	}

	printf("stop all services\n");
	stopall(-1);

	printf("unregister event manager\n");
	mozart_event_handler_put(keyevent_handler);
	mozart_event_handler_put(miscevent_handler);

	printf("unregister network manager\n");
	unregister_from_networkmanager();

#if (SUPPORT_ALARM == 1)
	printf("unregister alarm manager\n");
	unregister_to_alarm_manager();
#endif

	share_mem_clear();
	share_mem_destory();

	exit(-1);
}

#ifdef SUPPORT_VR
#if (SUPPORT_VR_WAKEUP == VR_WAKEUP_KEY_LONGPRESS)
int vr_flag = 0;
void *vr_longbutton_func(void *arg)
{	//printf("vr_longbutton_func===================================================>>>>>>>>>the tid = %d\n",syscall(SYS_gettid));
	vr_flag = 1;
	mozart_vr_wakeup();
	vr_flag = 0;

	return NULL; // make compile happy.
}

int create_vr_longbutton_pthread()
{
	int ret = 0;
	pthread_t vr_longbutton_thread = 0;

	if(vr_flag == 0) {
		ret = pthread_create(&vr_longbutton_thread, NULL, vr_longbutton_func, NULL);
		if(ret != 0) {
			printf ("Create iflytek pthread failed: %s!\n", strerror(errno));
			return ret;
		}
		pthread_detach(vr_longbutton_thread);
	}

	return ret;
}
#endif
#endif

bool wifi_configing = false;
pthread_t wifi_config_pthread;
pthread_mutex_t wifi_config_lock = PTHREAD_MUTEX_INITIALIZER;

pthread_t camera_pthread;
int fd;

extern bool acoustic_netconfig_flag;

pthread_t update_pthread;
pthread_mutex_t update_lock = PTHREAD_MUTEX_INITIALIZER;

//int play_volume = 0;
pthread_t capacity_alarm;

#define DARY_GRAY     "\033[1;30m"
#define LIGHT_RED     "\033[1;31m"
#define LIGHT_PURPLE "\033[1;35m"
#define NONE          "\033[m"
#define my_pr(fmt, args...)		printf(LIGHT_PURPLE fmt NONE,##args)

int update_flag_1 = 0;
int update_flag_2 = 0;
int g_not_post = 0;     //用于解除唤醒状态下发送语音的bug

int KEY_MENU_UP = 0;
int KEY_WAKE_UP = 0;
int KEY_ALL = 0;
int time_cnt1 = 0;
int time_cnt2 = 0;
//extern snd_source_t snd_source;
extern int g_playing_flag;

void *timeout_func(void *args)
{//printf("timeout_func=============================================>>>>>>>>>the tid = %d\n",syscall(SYS_gettid));
	void *flag = args;
	time_cnt1 = 2;
	*(int *)flag = 1;
	while(time_cnt1){
		time_cnt1--;
		printf("time_cnt1 = %d\n",time_cnt1);
		usleep(100*1000);
	}
	*(int *)flag = 0;
	return NULL;
}


int press_timeout(int *flag)
{	
	pthread_t timeout;
	if (pthread_create(&timeout, NULL, timeout_func,(void *)flag) == -1) {
		printf("Create press_timeout pthread failed: %s.\n", strerror(errno));
	}
	pthread_detach(timeout);

}


int my_printf()
{
	my_pr("%%%%%%%%%%%%%%%%%%%%OTA UPDATE%%%%%%%%%%%%%%%%%%%%%\n");
	return 0;
}


int led_music_playing(void)
{
	printf("=======led_music_playing====\n");

	if (breath_wifi_config_flag){
		pthread_cancel(breath_wifi_config_t);
		breath_wifi_config_flag = 0;
	}
	if (breath_pause_flag){
		pthread_cancel(breath_pause_t);
		breath_pause_flag = 0;
	}
        // rgb  dd4001

	system("echo \"221\" > /sys/class/leds/led_red/brightness");
	system("echo \"64\" > /sys/class/leds/led_green/brightness");
	system("echo \"01\" > /sys/class/leds/led_blue/brightness");
	


	return 0;
}

int led_bt_connected(void)
{
	printf("=======led_bt_connected====\n");

	if (breath_wifi_config_flag){
		pthread_cancel(breath_wifi_config_t);
		breath_wifi_config_flag = 0;
	}
	if (breath_pause_flag){
		pthread_cancel(breath_pause_t);
		breath_pause_flag = 0;
	}
	system("echo \"0\" > /sys/class/leds/led_red/brightness");
	system("echo \"1\" > /sys/class/leds/led_red/brightness");
	system("echo \"0\" > /sys/class/leds/led_green/brightness");
	system("echo \"1\" > /sys/class/leds/led_green/brightness");
	system("echo \"0\" > /sys/class/leds/led_blue/brightness");
	system("echo \"1\" > /sys/class/leds/led_blue/brightness");
	
	system("echo \"255\" > /sys/class/leds/led_blue/brightness");
	return 0;
}




void *breath_wificonfig_func(void *args)
{
	breath_wifi_config_flag = 1;
	int cnt = 4;
	char buf[128] = {0};
	system("echo \"0\" > /sys/class/leds/led_red/brightness");
	system("echo \"1\" > /sys/class/leds/led_red/brightness");
	system("echo \"0\" > /sys/class/leds/led_green/brightness");
	system("echo \"1\" > /sys/class/leds/led_green/brightness");
	system("echo \"0\" > /sys/class/leds/led_blue/brightness");
	system("echo \"1\" > /sys/class/leds/led_blue/brightness");
	while(breath_wifi_config_flag){
		while(1){
			cnt += 10;
			if(cnt > 260)
				break;
			memset(buf,0,sizeof(buf));
			sprintf(buf,"echo %d > /sys/class/leds/led_blue/brightness",cnt);//品红色呼吸
			system(buf);
			memset(buf,0,sizeof(buf));
			sprintf(buf,"echo %d > /sys/class/leds/led_red/brightness",cnt);//品红色呼吸
			system(buf);
			usleep(100*1000);
		}
		while(1){
			cnt -= 10;
			if(cnt < 0)
				break;
			memset(buf,0,sizeof(buf));
			sprintf(buf,"echo %d > /sys/class/leds/led_blue/brightness",cnt);//品红色呼吸
			system(buf);
			memset(buf,0,sizeof(buf));
			sprintf(buf,"echo %d > /sys/class/leds/led_red/brightness",cnt);//品红色呼吸
			system(buf);
			usleep(100*1000);
		}
	}	

	return NULL;


}

int create_breath_wificonfig_pthread(void)
{
	printf("----------create_breath_wificonfig_pthread-------------\n");
	#if 0


		if (breath_wifi_config_flag){
			pthread_cancel(breath_wifi_config_t);
			breath_wifi_config_flag = 0;
		}
		if (breath_pause_flag){
			pthread_cancel(breath_pause_t);
			breath_pause_flag = 0;
		}

		if (pthread_create(&breath_wifi_config_t, NULL, breath_wificonfig_func, NULL) == -1) {
			printf("Create  breath_answer_t pthread failed: %s.\n", strerror(errno));
			return -1;
		}
		pthread_detach(breath_wifi_config_t);
	#endif
	//r g b 9a 32 cd
	system("echo \"154\" > /sys/class/leds/led_red/brightness");

	system("echo \"50\" > /sys/class/leds/led_green/brightness");

	system("echo \"205\" > /sys/class/leds/led_blue/brightness");

	return 0;
}


void *breath_pause_func(void *args)
{
	printf("*******breath_pause_func*********\n");
	breath_pause_flag = 1;
	int cnt = 4;
	char buf[128] = {0};
	system("echo \"0\" > /sys/class/leds/led_red/brightness");
	system("echo \"1\" > /sys/class/leds/led_red/brightness");
	system("echo \"0\" > /sys/class/leds/led_green/brightness");
	system("echo \"1\" > /sys/class/leds/led_green/brightness");
	system("echo \"0\" > /sys/class/leds/led_blue/brightness");
	system("echo \"1\" > /sys/class/leds/led_blue/brightness");
	//改成红色闪烁
	while(breath_pause_flag){
		system("echo \"221\" > /sys/class/leds/led_red/brightness");
		usleep(500*1000);
		system("echo \"1\" > /sys/class/leds/led_red/brightness");
		usleep(500*1000);
	}

	return NULL;

}
int create_breath_pause_pthread(void)
{
	printf("----------create_breath_pause_pthread-------------\n");

	if (breath_pause_flag){
		pthread_cancel(breath_pause_t);
		breath_pause_flag = 0;
	}


	if (breath_wifi_config_flag){
		pthread_cancel(breath_wifi_config_t);
		breath_wifi_config_flag = 0;
	}
	
	if(low_power_flag){
		if (pthread_create(&breath_pause_t, NULL, breath_pause_func, NULL) == -1) {
			printf("Create  breath_pause_t pthread failed: %s.\n", strerror(errno));
			return -1;
		}
		pthread_detach(breath_pause_t);
	}

	return 0;
}

void *sendbroadcast_func(void *args)
{//printf("sendbroadcast_func=============================================>>>>>>>>>the tid = %d\n",syscall(SYS_gettid));
	printf("_____________________________________-sendbroadcast_func\n");
	struct sockaddr_in addr;
	
	int brdcFd;  
	if((brdcFd = socket(PF_INET, SOCK_DGRAM, 0)) == -1){  
		printf("socket fail\n");  
		return NULL;  
	} 
	int optval = 1;
	setsockopt(brdcFd, SOL_SOCKET, SO_BROADCAST, &optval, sizeof(int)); 

	memset(&addr, 0, sizeof(addr));  
	addr.sin_family=AF_INET;  
	addr.sin_port=htons(10000);  
	addr.sin_addr.s_addr=htonl(INADDR_BROADCAST);  

	int cnt = 0;

	int flag = *(int *)args;
	char msg[128] = {0};
	if(!flag)
		strcpy(msg,"ingenic:success");
	else
		strcpy(msg,"ingenic:fail");
	int sendBytes = 0;
	while(cnt++ < 20){
		if((sendBytes = sendto(brdcFd, msg, strlen(msg), 0,(struct sockaddr *)&addr, sizeof(addr))) == -1){  
			printf("sendto fail, errno=%d\n", errno);  
	    	}
	usleep(1000*1000);
	printf("msg=%s, msgLen=%d, sendBytes=%d\n", msg, strlen(msg), sendBytes); 		

	}
	return NULL;	

}

int  wifi_config_func_5G(void)
{
	printf("$$$$$$$$$$$$$$$$$wifi_config_func_5G$$$$$$$$$$$$$$$n\n");
	mozart_play_key("airkiss_config");
	system("killall wpa_supplicant");
	system("setipup -p 7");

	FILE *fp = NULL;
	wifi_ctl_msg_t new_mode;
	memset(&new_mode, 0, sizeof(new_mode));
	char read_buf[64] = {0};
	int ret_wificonfig = 0;

	if(!access("/tmp/wpa_supplicant_tmp.conf", R_OK)){
		printf("wifi_config_func_5G SUCCESSFULLY\n");
		system("wpa_supplicant -Dnl80211 -iwlan0 -c/usr/data/wpa_supplicant.conf -B &");
		ret_wificonfig = 0;
		pthread_t netconfig_t;
		if (pthread_create(&netconfig_t, NULL, sendbroadcast_func, (void *)&ret_wificonfig) == -1) {
			printf("Create netconfig_t pthread failed: %s.\n", strerror(errno));
			return -1;
		}
		pthread_detach(netconfig_t);
		mozart_play_key("airkiss_config_success");
		
		fp = fopen("/tmp/wpa_supplicant_tmp.conf","r");

		memset(read_buf,0,sizeof(read_buf));
		fgets(read_buf,64,fp);
		strncpy(new_mode.param.switch_sta.ssid,read_buf,strlen(read_buf) - 1);//少写\n

		memset(read_buf,0,sizeof(read_buf));
		fgets(read_buf,64,fp);
		strncpy(new_mode.param.switch_sta.psk,read_buf,strlen(read_buf));
		fclose(fp);

		new_mode.cmd = SW_STA;
		new_mode.force = true;

		strcpy(new_mode.name, app_name);
		if(request_wifi_mode(new_mode) != true)
			printf("ERROR: [%s] Request Network Failed, Please Register First!!!!\n", app_name);
		system("rm /tmp/wpa_supplicant_tmp.conf");
		system("sed -i \'73,$d\' /usr/data/wpa_supplicant.conf");
		return 0;
	}else{
		printf("wifi_config_func_5G FAILED\n");
		ret_wificonfig = 1;
		pthread_t netconfig_t;
		if (pthread_create(&netconfig_t, NULL, sendbroadcast_func, (void *)&ret_wificonfig) == -1) {
			printf("Create netconfig_t pthread failed: %s.\n", strerror(errno));
		}

		mozart_play_key("airkiss_config_fail");

		
		return -1;
	}
	
}
static void *sync_time_func(void *arg);
extern bool bt_started;
void *wifi_config_func(void *args)
{	//printf("wifi_config_func=============================================>>>>>>>>>the tid = %d\n",syscall(SYS_gettid));
	printf("======wifi_config_func======\n");
	
	pthread_t sync_time_pthread;
	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
	pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL);
	wifi_configing = true;
	mozart_module_stop();
	stopall(1);
	//mozart_config_wifi();
	create_breath_wificonfig_pthread();
	wifi_config_func_5G();
	return NULL; 
}

int create_wifi_config_pthread(void)
{	printf("0-------create_wifi_config_pthread---------\n");
	player_status_t status;
	status = mozart_player_getstatus(mozart_musicplayer_handler->player_handler);

	if (pthread_create(&wifi_config_pthread, NULL, wifi_config_func, NULL) == -1) {
		printf("Create wifi config pthread failed: %s.\n", strerror(errno));
		pthread_mutex_unlock(&wifi_config_lock);
		return -1;
	}
	pthread_detach(wifi_config_pthread);
	return 0;
}

void *camera_func(void *args)
{//printf("camera_func=============================================>>>>>>>>>the tid = %d\n",syscall(SYS_gettid));
	mozart_start_camera_rayshine();
	return NULL;
}
int create_camera_pthread(void)
{
	
	if (pthread_create(&camera_pthread, NULL, camera_func, NULL) == -1) {
		printf("Create camera_pthread failed: %s.\n", strerror(errno));
		
		return -1;
	}
	pthread_detach(camera_pthread);
	return 0;
}




#if (SUPPORT_LOCALPLAYER == 1)
void tfcard_scan_1music_callback(void)
{
#if 0
	tfcard_status = 1;
	mozart_module_local_music_startup();
#endif
}

void tfcard_scan_done_callback(char *musiclist)
{
	/* TODO; musicplayer add queue */
	printf("------tfcard_scan_done_callback-------------\n");
	//mozart_play_key("native_mode");
	//sleep(1);
	tfcard_status = 1;
//	mozart_module_local_music_startup();

}
#endif	/* SUPPORT_LOCALPLAYER */


static void key_bluetooth_handler(void)
{
#if (SUPPORT_BT == BT_BCM)
	int state = 0;

	state = mozart_bluetooth_hs_get_call_state();
	if (state == CALLSETUP_STATE_INCOMMING_CALL) {
		printf(">>>>>>>>>>>>>answer call>>>>>\n");
		mozart_bluetooth_hs_answer_call();
	} else if (state == CALL_STATE_LEAST_ONE_CALL_ONGOING ||
			   state == CALLSETUP_STATE_OUTGOING_CALL ||
			   state == CALLSETUP_STATE_REMOTE_BEING_ALERTED_IN_OUTGOING_CALL ||
			   state == CALLHELD_STATE_NO_CALL_HELD) {
		printf(">>>>>>>>>>>>>hang up>>>>>\n");
		mozart_bluetooth_hs_hangup_call();
	} else if (state == CALLSETUP_STATE_WAITING_CALL) {
		/* you can do other operation */
		mozart_bluetooth_hs_hold_call(BTHF_CHLD_TYPE_HOLDACTIVE_ACCEPTHELD);
	} else if (state == CALLHELD_STATE_PLACED_ON_HOLD_OR_SWAPPED) {
		/* you can do other operation */
		mozart_bluetooth_hs_hold_call(BTHF_CHLD_TYPE_RELEASEACTIVE_ACCEPTHELD);
	} else {
		printf("not support state : %d\n", state);
	}
#endif
}

#if (SUPPORT_BT == BT_BCM)
static void *bt_auto_reconnect_pthread(void *args)
{//printf("bt_auto_reconnect_pthread=============================================>>>>>>>>>the tid = %d\n",syscall(SYS_gettid));
	/* auto reconnect time:
	 * 10 minute */
	int ret = 0;

	pthread_mutex_lock(&bt_reconnect_lock);
	bt_reconnect_num = 10;
	while (bt_reconnect_num > 0) {
		if (BT_LINK_DISCONNECTING == mozart_bluetooth_get_link_status() ||
			BT_LINK_CONNECTING == mozart_bluetooth_get_link_status() ||
			BT_LINK_DISCONNECTED == mozart_bluetooth_get_link_status()) {
			printf("bt_auto_reconnect_pthread start!\n");
			ret = mozart_bluetooth_auto_reconnect(USE_HS_AVK, 0);
			printf("ret = %d\n", ret);
			if (ret == 0) {
				printf("reconnect successful!\n");
				usleep("1000*1000");
				bt_reconnect_num = 0;
			} else if (ret == -1) {
				printf("reconnect failed, retry!\n");
				player_status_t status;
				status = mozart_player_getstatus(mozart_musicplayer_handler->player_handler);
				usleep("1000*1000");
				bt_reconnect_num--;
			} else if (ret == -2) {
				printf("bt_reconnect_devices.xml not existed, no connected device\n");
				bt_reconnect_num = 0;
			} else {
				printf("Not supported this type: %d\n", ret);
			}
		} else if (BT_LINK_CONNECTED == mozart_bluetooth_get_link_status()) {
			printf("bt link status == BT_LINK_CONNECTED !\n");
			bt_reconnect_num = 0;
		}
	}

	bt_reconnect_num = 0;
	pthread_mutex_unlock(&bt_reconnect_lock);
	printf("return bt_auto_reconnect_pthread !\n");

	return NULL;
}

int bluetooth_create_auto_reconnect_pthread()
{
	if (pthread_create(&bt_reconnect_pthread, NULL, bt_auto_reconnect_pthread, NULL) != 0) {
		printf("create bt_auto_reconnect_pthread failed !\n");
		return -1;
	}
	pthread_detach(bt_reconnect_pthread);
	return 0;
}

int bluetooth_cancel_auto_reconnect_pthread()
{
	bt_reconnect_num = 0;
	return 0;
}

static void *bluetooth_ring_pthread(void *args)
{
	bt_ring_flag = true;
	while (bt_ring_flag) {
		printf("mozart_play_key: bluetooth_ring\n");
		mozart_play_key_sync("bluetooth_ring");
		sleep(1);
	}

	return NULL;
}
int bluetooth_create_ring_pthread()
{
	if (pthread_create(&bt_ring_pthread, NULL, bluetooth_ring_pthread, NULL) != 0) {
		printf("create bt_ring_pthread failed !\n");
		return -1;
	}
	pthread_detach(bt_ring_pthread);
	return 0;
}

int bluetooth_cancel_ring_pthread()
{
	printf("bluetooth_cancel_ring_pthread !\n");
	bt_ring_flag = false;

	return mozart_stop_tone();
}
#endif

/*******************************************************************************
 * long press
 *******************************************************************************/
enum key_long_press_state {
	KEY_LONG_PRESS_INVALID = 0,
	KEY_LONG_PRESS_WAIT,
	KEY_LONG_PRESS_CANCEL,
	KEY_LONG_PRESS_DONE,
};

struct input_event_key_info {
	char *name;
	int key_code;
	enum key_long_press_state state;
	pthread_t pthread;
	pthread_mutex_t lock;
	pthread_cond_t cond;
	int timeout_second;
	void (*handler)(bool long_press);
};
int record_func(void);
char wavconfig_ssid[16] = {0};
char wavconfig_passwd[16] = {0};

void mozart_wifi_config(void)
{	

	create_wifi_config_pthread();
		
}
static void wifi_switch_handler(bool long_press)//没有长短按分别
{	printf("==========wifi_switch_handler==========mozart_vr_get_status = %d\n",mozart_vr_get_status());
	//mozart_wifi_config();
	if(mozart_vr_get_status() != VR_ASR){
		mozart_play_pause();
		h5_speak_start();
	}else{
		g_not_post = 1;
	}
		
}

static void volume_down_handler(bool long_press)
{	printf("**************volume_down_handler********************\n");
	if (long_press)
		mozart_previous_song();
	else
		mozart_volume_down();
}

static void volume_up_handler(bool long_press)
{	printf("**************volume_up_handler********************\n");
	if (long_press)
		mozart_next_song();
	else
		mozart_volume_up();
}
static int initall(void);
static void play_pause_handler(bool long_press)
{
	if (long_press){
		printf("---------------Long PAUSE PRESS\n");
		key_bluetooth_handler();
	}
	else{
		printf("---------------Short PAUSE PRESS \n");
		mozart_play_pause();
		usleep(100*1000);
		player_status_t status;
		status = mozart_player_getstatus(mozart_musicplayer_handler->player_handler);
		if (mozart_vr_get_status() != VR_ASR&&status != 4) //蓝牙模式下 没有标识播放状态的变量，去掉蓝牙按键暂停的语音提示
			mozart_play_key_sync("paused");
	}
}

static struct input_event_key_info input_event_key_infos[] = {
	{
		.name = "wifi_switch_key",
		.key_code = KEY_MODE,
		.lock = PTHREAD_MUTEX_INITIALIZER,
		.cond = PTHREAD_COND_INITIALIZER,
		.timeout_second = 1,
		.handler = wifi_switch_handler,
	},
	{
		.name = "volume_up_key",
		.key_code = KEY_VOLUMEUP,
		.lock = PTHREAD_MUTEX_INITIALIZER,
		.cond = PTHREAD_COND_INITIALIZER,
		.timeout_second = 1,
		.handler = volume_up_handler,
	},
	{
		.name = "volume_down_key",
		.key_code = KEY_VOLUMEDOWN,
		.lock = PTHREAD_MUTEX_INITIALIZER,
		.cond = PTHREAD_COND_INITIALIZER,
		.timeout_second = 1,
		.handler = volume_down_handler,
	},
	{
		.name = "play_pause_key",
		.key_code = KEY_PLAYPAUSE,
		.lock = PTHREAD_MUTEX_INITIALIZER,
		.cond = PTHREAD_COND_INITIALIZER,
		.timeout_second = 1,
		.handler = play_pause_handler,
	},
};

static void *key_long_press_func(void *args)
{//printf("key_long_press_func=============================================>>>>>>>>>the tid = %d\n",syscall(SYS_gettid));
	printf("=============key_long_press_func==============\n");
	struct timeval now;
	struct timespec timeout;
	struct input_event_key_info *info = (struct input_event_key_info *)args;

	pthread_mutex_lock(&info->lock);
	if (info->state == KEY_LONG_PRESS_CANCEL) {
		info->state = KEY_LONG_PRESS_INVALID;
		printf("11-------->%s short press\n", info->name);
		if (info->handler)
			info->handler(false);
		pthread_mutex_unlock(&info->lock);
		return NULL;
	}

	gettimeofday(&now, NULL);
	timeout.tv_sec = now.tv_sec + info->timeout_second;
	timeout.tv_nsec = now.tv_usec * 1000;

	pthread_cond_timedwait(&info->cond,&info->lock, &timeout);

	if (info->state == KEY_LONG_PRESS_CANCEL) {
		info->state = KEY_LONG_PRESS_INVALID;
		printf("22-------->%s short press\n", info->name);
		if (info->handler)
			info->handler(false);
		pthread_mutex_unlock(&info->lock);
		return NULL;
	}

	info->state = KEY_LONG_PRESS_DONE;

	printf("-------------------%s long press\n", info->name);
	if (info->handler)
		info->handler(true);
	pthread_mutex_unlock(&info->lock);

	return NULL;
}

static void create_key_long_press_pthread(struct input_event_key_info *info)
{
	pthread_mutex_lock(&info->lock);
	if (info->state != KEY_LONG_PRESS_INVALID) {
		pthread_mutex_unlock(&info->lock);
		return ;
	}
	info->state = KEY_LONG_PRESS_WAIT;
	pthread_mutex_unlock(&info->lock);

	if (pthread_create(&info->pthread, NULL, key_long_press_func, (void *)info) == -1) {
		printf("Create key long press pthread failed: %s.\n", strerror(errno));
		return ;
	}
	pthread_detach(info->pthread);
}

static void key_long_press_cancel(struct input_event_key_info *info)
{
	pthread_mutex_lock(&info->lock);

	if (info->state == KEY_LONG_PRESS_DONE) {
		info->state = KEY_LONG_PRESS_INVALID;
	} else if (info->state == KEY_LONG_PRESS_WAIT) {
		info->state = KEY_LONG_PRESS_CANCEL;
		pthread_cond_signal(&info->cond);
	}

	pthread_mutex_unlock(&info->lock);
}

static int input_event_handler(struct input_event event)
{	printf("---------input_event_handler-------event.value = %d\n",event.value);
	int i;
	struct input_event_key_info *info;

	for (i = 0; i < sizeof(input_event_key_infos) / sizeof(struct input_event_key_info); i++) {
		info = &input_event_key_infos[i];

		if (event.code == info->key_code) {
			if (event.value == 1)
				create_key_long_press_pthread(info);
			else
				key_long_press_cancel(info);

			return 0;
		}
	}

	return -1;
}
void *flag_change_func(void *arg)
{
	sleep(2);
	update_flag_1 = 0;
	update_flag_2 = 0;
	
	
	printf("-----------------------flag_change_func : update_flag_1 = %d,update_flag_2 = %d\n",update_flag_1,update_flag_2);
	return NULL;
}

static source_index = 0;
static char *source[] = {"童话故事","成语故事","名人传"};
int xiaowang_next_source()
{
	printf("source_index = %d\n",source_index);
	speech_cloudmusic_play(source[source_index++]);
	if(source_index == 3)
		source_index = 0;
	return 0;
}

//extern int g_auto_pause;
pthread_t post_pthread;

void *post_func(void *args)
{//printf("post_func=============================================>>>>>>>>>the tid = %d\n",syscall(SYS_gettid));
	sleep(3);
	mozart_play_pause();
	int ret = h5_post_msg();
	printf("h5_post_msg(): ret = %d\n",ret);	
	return NULL;
}

void keyevent_callback(mozart_event event, void *param)
{	my_pr("---------------keyevent_callback:%s---------------------\n",keyvalue_str[event.event.key.key.value]);
	if(strcmp(keyvalue_str[event.event.key.key.value],"Pressed") == 0){
		if(KEY_ALL){	
			time_cnt1 = 10;
			return;
		}
		press_timeout(&KEY_ALL);
	}

	switch (event.type) {
	case EVENT_KEY:
		if (event.event.key.key.type != EV_KEY) {
			printf("Only support keyboard now.\n");
			break;
		}

		if((strcmp(keycode_str[event.event.key.key.code],"KEY_F10") == 0)&&(strcmp(keyvalue_str[event.event.key.key.value],"Released") == 0))
		{	
			if(!g_not_post){	
				pthread_create(&post_pthread, NULL, post_func, NULL);
				pthread_detach(post_pthread);

			}else{
				g_not_post = 0;
			}
			
		}
		int ret = input_event_handler(event.event.key.key);
		printf("------------------------------>>>>> input_event_handler ret = %d\n",ret);
		if (ret == 0)
			break;
		
		if(mozart_vr_get_status() == VR_ASR){ //其他按键屏蔽，除了暂停
			my_pr("VR status = %d\n",mozart_vr_get_status());
			my_pr("Not int VR_AEC mode,press invalid\n");
			return;
		}
		if (event.event.key.key.value == 1) {

			switch (event.event.key.key.code)
			{
			case KEY_RECORD:
				printf("=============KEY_RECORD --->");
				if (mozart_vr_get_status() == VR_AEC) {
					printf("AEC mode, wakeup\n");
					//mozart_stop_tone_sync();
					mozart_vr_aec_wakeup();
				} else if (mozart_vr_get_status() == VR_ASR) {
					printf("ASR mode, interrupt......\n");
					mozart_vr_asr_break();
					mozart_stop_tone_sync();
					//mozart_play_pause();
					aec_key_wakeup();
				}

				break;
			case KEY_F1:
				printf("====KEY_F1=====\n");
				//create_wifi_config_pthread();
				break;
				
			case KEY_BLUETOOTH:
				printf("====KEY_BLUETOOTH===\n");
				key_bluetooth_handler();
				break;
			case KEY_F9:
				printf("====KEY_F9====\n");	
				//h5_play_msg();
				if(snd_source == SND_SRC_CLOUD)
					xiaowang_next_source();
				break;
			case KEY_PREVIOUSSONG:	
				printf("====KEY_NEXTSONG===\n");
				mozart_next_song();
				break;
			case KEY_NEXTSONG:
				printf("=====KEY_PREVIOUSSONG====\n");
				mozart_previous_song();			
				break;			
			case KEY_MENU:
				printf("====KEY_MENU====\n");//切换资源
				//if(snd_source == SND_SRC_CLOUD)
				//	xiaowang_next_source();
				h5_play_msg();
				break;
				
			case KEY_F3:            /* music music Shortcut key 1 */
				printf("=====KEY_F3====\n");
				mozart_music_list(0);
				break;
				
			case KEY_F4:            /* music music Shortcut key 2 */
				printf("====KEY_F4====\n");
				mozart_music_list(1);
				break;
				
			case KEY_F5:            /* music music Shortcut key 3 */
				printf("=====KEY_F5=====\n");
				mozart_music_list(2);
				break;
			case KEY_F10:
				printf("====KEY_F10=====\n");	
				mozart_wifi_config();
				break;
			case KEY_POWER:
				mozart_power_off();
				break;
			default:
				//printf("UnSupport key down in %s:%s:%d.\n", __FILE__, __func__, __LINE__);
				break;
			}
		} else {
			switch (event.event.key.key.code) {
			case KEY_RECORD:
				/*TODO: add VR_WAKEUP_KEY_LONGPRESS support*/
				break;
			default:
				break;
			}
		}
		break;
	case EVENT_MISC:
	default:
		break;
	}

	return;
}
#if 1
int rayshine_save_volume(int volume)
{
	FILE *fp = NULL;
	char value[4] = {0};
	sprintf(value,"%d",volume);
	fp = fopen("/usr/data/volume.ini","w");
	if(fp){
		fwrite(value,1,sizeof(value),fp);
		fclose(fp);
	}
	return 0;
}

int rayshine_get_volume(void)
{
	FILE *fp = NULL;
	char volume[4] = {0};
	fp = fopen("/usr/data/volume.ini","r");
	if(fp){
		if(fread(volume,1,sizeof(volume),fp)){
			printf("last shutdown volume = %d\n",atoi(volume));
			fclose(fp);
			return atoi(volume);
		}else{
			printf("fread error set default : 30\n");
			return 30;
		}
	}else{
		printf("rayshine_get_volume  default : 30\n");
		return 30;
	}	
	
}

#endif
void miscevent_callback(mozart_event event, void *param)
{	//printf("================miscevent_callback  plugin  tfcard_status = %d\n",tfcard_status);
	switch (event.type) {
	case EVENT_MISC:
		printf("[misc event] %s ==>: %s.\n", event.event.misc.name, event.event.misc.type);
		if (!strcasecmp(event.event.misc.name, "linein")
#if (SUPPORT_MULROOM == 1)
			&& mulroom_state != MUL_GROUP_RECEIVER
#endif /* SUPPORT_MULROOM == 1 */
			) {
			if (!strcasecmp(event.event.misc.type, "plugin")) { // linein plugin
				mozart_ui_linein_plugin();
				mozart_module_stop();
				mozart_linein_on();
				lapsule_do_linein_on();
			} else if (!strcasecmp(event.event.misc.type, "plugout")) { // linein plugout
				mozart_ui_linein_plugout();
				mozart_linein_off();
				lapsule_do_linein_off();
			}
		} else if(!strcasecmp(event.event.misc.name, "usb")) {
#if 0
			if (!strcasecmp(event.event.misc.type, "connected")) { // usb plugin
				system("umount /mnt/sdcard/");
				system("echo /dev/mmcblk0p1 > /sys/devices/platform/jz-dwc2/dwc2/gadget/lun0/file");
			} else if (!strcasecmp(event.event.misc.type, "disconnected")) { // usb plugout
				system("echo  0 > /sys/class/android_usb/android0/enable");
				system("mount /dev/mmcblk0p1 /mnt/sdcard/");
			}
#endif
		} else if (!strcasecmp(event.event.misc.name, "tfcard")) {
			if (!strcasecmp(event.event.misc.type, "plugin")) { // tfcard plugin
				;
				if (tfcard_status != 1) {
					mozart_ui_localplayer_plugin();
					player_status_t status;
					status = mozart_player_getstatus(mozart_musicplayer_handler->player_handler);
					
					if(status == PLAYER_PLAYING)
						mozart_play_pause();
					sleep(1);
					mozart_play_key("tf_add");
					tfcard_status = 1;
					if(snd_source == SND_SRC_LOCALPLAY){
						if(-1 == mozart_module_local_music_startup())
							mozart_tts_sync("内存卡没有音频文件");
					}
				} else {
					// do nothing.
				}
			} else if (!strcasecmp(event.event.misc.type, "plugout")) { // tfcard plugout
				
				if (tfcard_status != 0) {
					mozart_ui_localplayer_plugout();
					if (snd_source == SND_SRC_LOCALPLAY)
						mozart_musicplayer_stop(mozart_musicplayer_handler);
					player_status_t status;
					status = mozart_player_getstatus(mozart_musicplayer_handler->player_handler);
					
					if(status == PLAYER_PAUSED){
						mozart_play_key("tf_remove");
					}
					if(status == PLAYER_PLAYING){
						mozart_play_pause();
						mozart_play_key("tf_remove");
					}
					if(snd_source == SND_SRC_BT_AVK || snd_source == SND_SRC_LOCALPLAY)
						mozart_play_key("tf_remove");
					//sleep(2);
					tfcard_status = 0;

					//mozart_snd_source_switch();
				}
			}
		} else if (!strcasecmp(event.event.misc.name, "headset")) {
			if (!strcasecmp(event.event.misc.type, "plugin")) { // headset plugin
				printf("headset plugin.\n");
				// do nothing.
			} else if (!strcasecmp(event.event.misc.type, "plugout")) { // headset plugout
				printf("headset plugout.\n");
				// do nothing.
			}
		} else if (!strcasecmp(event.event.misc.name, "spdif")) {
			if (!strcasecmp(event.event.misc.type, "plugin")) { // spdif-out plugin
				printf("spdif plugin.\n");
				// do nothing.
			} else if (!strcasecmp(event.event.misc.type, "plugout")) { // spdif-out plugout
				printf("spdif plugout.\n");
				// do nothing.
			}
		} else if (!strcasecmp(event.event.misc.name, "volume")) {
			rayshine_save_volume(event.event.misc.value[0]);
			if (!strcasecmp(event.event.misc.type, "update music")) {
				printf("%s volume to %d.\n", event.event.misc.type, event.event.misc.value[0]);
			} else if (!strcasecmp(event.event.misc.type, "update bt_music")) {
				printf("%s volume to %d.\n", event.event.misc.type, event.event.misc.value[0]);
			} else if (!strcasecmp(event.event.misc.type, "update bt_call")) {
				printf("%s volume to %d.\n", event.event.misc.type, event.event.misc.value[0]);
			} else if (!strcasecmp(event.event.misc.type, "update unknown")) {
				printf("%s volume to %d.\n", event.event.misc.type, event.event.misc.value[0]);
			} else {
				printf("unhandle volume event: %s.\n", event.event.misc.type);
			}
#if (SUPPORT_BT == BT_BCM)
		} else if (!strcasecmp(event.event.misc.name, "bluetooth")) {
			if (!strcasecmp(event.event.misc.type, "connecting")) {
				; // do nothing if bluetooth device is connecting.
			} else if (!strcasecmp(event.event.misc.type, "disc_new")) {
			} else if (!strcasecmp(event.event.misc.type, "disc_complete")) {
				bsa_start_regular_enable = 1;
			} else if (!strcasecmp(event.event.misc.type, "sec_link_down")) {
				/*
				 * Reason code 0x13: Mobile terminate bluetooth connection
				 * Reason code 0x8: connection timeout(beyond distance)
				 */
				UINT8 link_down_data = 0;
				link_down_data = mozart_bluetooth_get_link_down_status();
				if (link_down_data == 0x13) {
					printf("Mobile terminate bluetooth connection.\n");
				} else if (link_down_data == 0x8) {
					printf("BT connection timeout(beyond distance maybe)!\n");
					int state = 0;
					state = bluetooth_create_auto_reconnect_pthread();
					if (state == 0) {
						printf("bluetooth_create_auto_reconnect_pthread success !\n");
					} else if (state == -1) {
						printf("bluetooth_create_auto_reconnect_pthread failed !\n");
					}
				} else if (link_down_data == 0x16) {
					printf("connection terminated by local host!\n");
				} else {
					printf("not handle link_down_data %d, Ignore.\n", link_down_data);
				}
			} else {
				printf("unhandle bluetooth event: %s.\n", event.event.misc.type);
			}
		} else if (!strcasecmp(event.event.misc.name, "bt_hs")) {
			if (!strcasecmp(event.event.misc.type, "connected")) {
				if (mozart_bluetooth_get_link_status() == BT_LINK_CONNECTED) {
					if (bt_link_state == BT_LINK_DISCONNECTED) {
						bt_link_state = BT_LINK_CONNECTED;
						mozart_bluetooth_set_visibility(0, 0);
						mozart_play_pause();
						stop(VR);
						mozart_appserver_shutdown();
						usleep(100*1000);
						//led_bt_connected();
						mozart_play_key_sync("bluetooth_connect");
						mozart_tts_wait();
					}
#if (SUPPORT_BSA_PBC == 1)
					mozart_bluetooth_pbc_open_connection();
#endif
				}
			} else if (!strcasecmp(event.event.misc.type, "disconnected")) {
				bluetooth_cancel_ring_pthread();
				if (mozart_bluetooth_get_link_status() == BT_LINK_DISCONNECTED) {
					if (bt_link_state == BT_LINK_CONNECTED) {
						bt_link_state = BT_LINK_DISCONNECTED;
						mozart_play_key("bluetooth_disconnect");
						mozart_tts_wait();
						mozart_bluetooth_set_visibility(1, 1);
						//stopall(1);
					}
					system("rm /usr/data/pre_url.txt");//防止后面播放跟列表不一样
					restartall(1);
					//sleep(5);
					//startall(1);
					//xiaowang_next_source();
#if (SUPPORT_BSA_PBC == 1)
					mozart_bluetooth_pbc_close_connection();
#endif
				}
			} else if (!strcasecmp(event.event.misc.type, "ring")) {

#ifdef SUPPORT_VR
				if (mozart_vr_get_status()) {
					printf("invoking mozart_vr_stop()\n");
					mozart_vr_stop();
				}
#endif
				//TODO: should pause here, resume music after hangup.
				if (mozart_module_stop()) {
					printf("can not ring.\n");
				} else {
					bluetooth_create_ring_pthread();
					snd_source = SND_SRC_BT_AVK;
				}
			} else if (!strcasecmp(event.event.misc.type, "hangup")) {
				bluetooth_cancel_ring_pthread();
#ifdef SUPPORT_VR
				if(!mozart_vr_get_status()) {
					printf("invoking mozart_vr_start()\n");
					mozart_vr_start();
				}
#endif
			} else if (!strcasecmp(event.event.misc.type, "call")) {
				bluetooth_cancel_ring_pthread();
#ifdef SUPPORT_VR
				if (mozart_vr_get_status()) {
					printf("invoking mozart_vr_stop()\n");
					mozart_vr_stop();
				}
#endif
				//TODO: should pause here, resume music after hangup.
				if (mozart_module_stop()) {
					share_mem_set(BT_HS_DOMAIN, RESPONSE_CANCEL);
				} else {
					share_mem_set(BT_HS_DOMAIN, RESPONSE_DONE);
					snd_source = SND_SRC_BT_AVK;
				}
			} else if (!strcasecmp(event.event.misc.type, "close")) {
			} else if (!strcasecmp(event.event.misc.type, "vgs")) {
				int volume = event.event.misc.value[0];

				mozart_volume_set(volume * 100 / APP_HS_CALL_VOLUME_MAX, BT_CALL_VOLUME);
				printf("phone volume: %d, mozart set hs volume: %d\n",
					   volume,
					   volume * 100 / APP_HS_CALL_VOLUME_MAX);
			} else {
				printf("unhandle bt phone event: %s.\n", event.event.misc.type);
			}
		} else if (!strcasecmp(event.event.misc.name, "bt_avk")) {
		printf("bt_avk-------------------event.event.misc.type = %s-----------------\n",event.event.misc.type);
			if (!strcasecmp(event.event.misc.type, "connected")) {

				if (mozart_bluetooth_get_link_status() == BT_LINK_CONNECTED) {
					if (bt_link_state == BT_LINK_DISCONNECTED) {
						bt_link_state = BT_LINK_CONNECTED;
						mozart_ui_bt_connected();
						mozart_bluetooth_set_visibility(0, 0);
						//mozart_module_stop();
						/* FIXME: do not stop if music playing. will skip tone. */
						mozart_play_key_sync("bluetooth_connect");
					}
				}
			} else if (!strcasecmp(event.event.misc.type, "disconnected")) {
				if (mozart_bluetooth_get_link_status() == BT_LINK_DISCONNECTED) {
					if (bt_link_state == BT_LINK_CONNECTED) {
						bt_link_state = BT_LINK_DISCONNECTED;
						mozart_play_key("bluetooth_disconnect");
						mozart_bluetooth_set_visibility(1, 1);
						mozart_ui_bt_disconnected();
					}
				}
			} else if (!strcasecmp(event.event.misc.type, "play")) {
				if (mozart_module_stop()) {
					share_mem_set(BT_AVK_DOMAIN, RESPONSE_CANCEL);
				} else {
					share_mem_set(BT_AVK_DOMAIN, RESPONSE_DONE);
					snd_source = SND_SRC_BT_AVK;
				}

			} else if (!strcasecmp(event.event.misc.type, "pause")) {
				create_breath_pause_pthread();
				printf("bt music player paused(or stopped).\n");
			} else if (!strcasecmp(event.event.misc.type, "set_abs_vol")) {
				int index = 0;
				int volume = 0;
				int fd = 0;
				
				volume = event.event.misc.value[0];
				fd = event.event.misc.value[1];
				for (index = (APP_AVK_VOLUME_MAX -1); index >= 0; index--) {
					if (avk_volume_set_phone[index] <= volume)
						break;
				}
				if (index < 0) {
					printf("failed to get music volume %d from avk_volume_set_dsp\n", volume);
					break;
				}
				if (AUDIO_OSS == get_audio_type()) {
					/* Note: add app_avk_cb.fd judged, because when play tts,
					 * we reviced ABS_VOL_CMD_EVT, and set volume to dsp,
					 * it will generate a case of sound mutation */
					if (fd != -1) {
						mozart_volume_set(avk_volume_set_dsp[index], BT_MUSIC_VOLUME);
					} else {
						char vol[8] = {};
						sprintf(vol, "%d", avk_volume_set_dsp[index]);
						if (mozart_ini_setkey("/usr/data/system.ini", "volume", "bt_music", vol)) {
							printf("save volume to /usr/data/system.ini error.\n");
							break;
						}
					}
				} else if (AUDIO_ALSA == get_audio_type()) {
					mozart_volume_set(avk_volume_set_dsp[index], BT_MUSIC_VOLUME);
				} else {
					printf("Not support audio type: %d!!\n", get_audio_type());
				}
				printf("phone volume: 0x%x, mozart set avk volume: %d\n",
						volume,
						avk_volume_set_dsp[index]);
			} else if (!strcasecmp(event.event.misc.type, "get_elem_attr")) {
				printf("###########  33 get_elem_attr\n");
				int i = 0;
				int attr_id = 0;
				tBSA_AVK_GET_ELEMENT_ATTR_MSG *p_data = NULL;
				p_data = mozart_bluetooth_avk_get_element_att();
				printf("p_data->num_attr = %d\n", p_data->num_attr);
				printf("p_data->status = %d\n", p_data->status);
				for (i = 0; i < p_data->num_attr; i++) {
					attr_id = p_data->attr_entry[i].attr_id;
					if (attr_id == AVRC_MEDIA_ATTR_ID_TITLE) {
						printf("music Title: %s\n", p_data->attr_entry[i].name.data);
					} else if (attr_id == AVRC_MEDIA_ATTR_ID_ARTIST) {
						printf("music Artist Name: %s\n", p_data->attr_entry[i].name.data);
					} else if (attr_id == AVRC_MEDIA_ATTR_ID_ALBUM) {
						printf("music Album Name: %s\n", p_data->attr_entry[i].name.data);
					} else if (attr_id == AVRC_MEDIA_ATTR_ID_TRACK_NUM) {
						printf("music Track Number: %s\n", p_data->attr_entry[i].name.data);
					} else if (attr_id == AVRC_MEDIA_ATTR_ID_NUM_TRACKS) {
						printf("music Total Number of Tracks: %s\n", p_data->attr_entry[i].name.data);
					} else if (attr_id == AVRC_MEDIA_ATTR_ID_GENRE) {
						printf("music Genre: %s\n", p_data->attr_entry[i].name.data);
					} else if (attr_id == AVRC_MEDIA_ATTR_ID_PLAYING_TIME) {
						printf("music Playing Time: %s\n", p_data->attr_entry[i].name.data);
					}
				}
			} else if (!strcasecmp(event.event.misc.type, "play_status")) {
				printf("###########  44 play_status\n");
				tBSA_AVK_GET_PLAY_STATUS_MSG *play_status_msg;
				play_status_msg = mozart_bluetooth_avk_get_play_status();
				printf("play status : %d\n", play_status_msg->play_status);
				printf("play song len : %d\n", play_status_msg->song_len);
				printf("play song pos : %d\n", play_status_msg->song_pos);
			} else {
				printf("unhandle bt music event: %s.\n", event.event.misc.type);
			}
		} else if (!strcasecmp(event.event.misc.name, "bt_av")) {
			if (!strcasecmp(event.event.misc.type, "connected")) {
#if (SUPPORT_BSA_A2DP_SOURCE == 1)
				avk_source_set_audio_output();
#endif
			} else if (!strcasecmp(event.event.misc.type, "disconnected")) {
			} else if (!strcasecmp(event.event.misc.type, "play")) {
			} else if (!strcasecmp(event.event.misc.type, "pause")) {
			} else {
				printf("unhandle bt music event: %s.\n", event.event.misc.type);
			}
		} else if (!strcasecmp(event.event.misc.name, "bt_ble")) {
			if (!strcasecmp(event.event.misc.type, "disc_new")) {
			} else if (!strcasecmp(event.event.misc.type, "disc_complete")) {
				bsa_ble_start_regular_enable = 1;
#if (SUPPORT_BSA_BLE_HH == 0)
				int index;
				tAPP_DISCOVERY_CB *app_discovery_cb = NULL;
				app_discovery_cb = mozart_bluetooth_disc_get_device_info();
				for (index = 0; index < APP_DISC_NB_DEVICES; index++) {
					if (app_discovery_cb->devs[index].in_use != FALSE) {
						printf("Dev: %d\n", index);
						printf("\tBdaddr:%02x:%02x:%02x:%02x:%02x:%02x \n",
								app_discovery_cb->devs[index].device.bd_addr[0],
								app_discovery_cb->devs[index].device.bd_addr[1],
								app_discovery_cb->devs[index].device.bd_addr[2],
								app_discovery_cb->devs[index].device.bd_addr[3],
								app_discovery_cb->devs[index].device.bd_addr[4],
								app_discovery_cb->devs[index].device.bd_addr[5]);
						printf("\tName: %s\n", app_discovery_cb->devs[index].device.name);
						char *ble_name = (char *)app_discovery_cb->devs[index].device.name;
						if (!strcmp(ble_name, "BSA_BLE"))
						{
							bsa_manu_data manu_data;
							int index = 0;
							UINT8 * eir_data = app_discovery_cb->devs[index].device.eir_data;
							mozart_bluetooth_parse_eir_manuf_specific(eir_data, &manu_data);
							printf("manu_data.data_length = %d\n", manu_data.data_length);
							printf("manu_data.company_id = 0x%02x\n", manu_data.company_id);
							printf("manu_data: \n");
							for (index = 0; index < manu_data.data_length; index++)
							{
								printf("%x ", manu_data.p_manu[index]);
							}
							printf("\n");
						}
						printf("\tClassOfDevice: %02x:%02x:%02x => %s\n",
							   app_discovery_cb->devs[index].device.class_of_device[0],
							   app_discovery_cb->devs[index].device.class_of_device[1],
							   app_discovery_cb->devs[index].device.class_of_device[2],
							   mozart_bluetooth_get_cod_string(app_discovery_cb->devs[index].device.class_of_device));

						printf("\tRssi: %d\n", app_discovery_cb->devs[index].device.rssi);
						if (app_discovery_cb->devs[index].device.eir_vid_pid[0].valid) {
							printf("\tVidSrc: %d Vid: 0x%04X Pid: 0x%04X Version: 0x%04X",
								   app_discovery_cb->devs[index].device.eir_vid_pid[0].vendor_id_source,
								   app_discovery_cb->devs[index].device.eir_vid_pid[0].vendor,
								   app_discovery_cb->devs[index].device.eir_vid_pid[0].product,
								   app_discovery_cb->devs[index].device.eir_vid_pid[0].version);
						}
					}
				}
#endif
			} else if (!strcasecmp(event.event.misc.type, "se_open")) {
				printf("se_open!\n");
			} else if (!strcasecmp(event.event.misc.type, "se_close")) {
				printf("se_close!\n");
			} else if (!strcasecmp(event.event.misc.type, "se_create")) {
				ble_service_create_enable = 1;
			} else if (!strcasecmp(event.event.misc.type, "se_addchar")) {
				ble_service_add_char_enable = 1;
			} else if (!strcasecmp(event.event.misc.type, "se_start")) {
				ble_service_start_enable = 1;
			} else if (!strcasecmp(event.event.misc.type, "se_r_complete")) {
				int state = 0;
				ble_client_write_data cl_wdata;
				cl_wdata.write_len = 4;
				cl_wdata.service_uuid = BSA_BLE_UUID_SERVCLASS_HEART_RATE;
				cl_wdata.char_uuid = BSA_BLE_GATT_UUID_HEART_RATE_MEASUREMENT;
				cl_wdata.write_data[0] = 0x11;
				cl_wdata.write_data[1] = 0x22;
				cl_wdata.write_data[2] = 0x33;
				cl_wdata.write_data[3] = 0x44;

				cl_wdata.client_num = 1;
				cl_wdata.is_primary = 1;
				cl_wdata.ser_inst_id = 0;
				cl_wdata.char_inst_id = 0;
				cl_wdata.is_descript = 0;
				cl_wdata.descr_id = 0;
				cl_wdata.desc_value = 0;
				cl_wdata.write_type = 1;
				state = mozart_bluetooth_ble_client_write(&cl_wdata);

			} else if (!strcasecmp(event.event.misc.type, "se_w_complete")) {
				
#if (SUPPORT_BSA_BLE == 1)	
				int i = 0;
				int value_num = 0;
				int server_num = 0;
				int char_attr_num = 0;
				int length_of_data = 3;
				UINT8 value[BSA_BLE_MAX_ATTR_LEN];
				ble_server_indication ble_indication;

				server_num = ble_heart_rate_service.server_num;
				char_attr_num = ble_notify_char.char_attr_num;
				value_num = mozart_bluetooth_server_get_char_value(server_num, char_attr_num, value);
				printf("value_num = %d\n", value_num);
				for (i = 0; i < value_num; i++)
					printf("value[%d] = 0x%x\n", i, value[i]);
				/* Listen for notify */
				if (value[0] == 1) {
					ble_indication.server_num = server_num;
					ble_indication.attr_num = ble_heart_rate_measure_char.char_attr_num;
					ble_indication.length_of_data = length_of_data;
					server_num = ble_heart_rate_service.server_num;
					char_attr_num = ble_heart_rate_measure_char.char_attr_num;
					/* get and set client vaule */
					value_num = mozart_bluetooth_server_get_char_value(server_num, char_attr_num, value);
					printf("value_num1 = %d\n", value_num);
					for (i = 0; i < value_num; i++)
						printf("value1 = 0x%x\n", value[i]);
					value[0] = 0x12;
					value[1] = 0x13;
					value[2] = 0x14;
					mozart_bluetooth_server_set_char_value(server_num, char_attr_num, value, 3);
					/* send client indication */
					mozart_bluetooth_ble_server_send_indication(&ble_indication);
				}
#endif /* SUPPORT_BSA_BLE */
			} else if (!strcasecmp(event.event.misc.type, "cl_open")) {
				ble_client_open_enable = 1;
			} else if (!strcasecmp(event.event.misc.type, "cl_write")) {
				ble_client_write_enable = 1;
			} else if (!strcasecmp(event.event.misc.type, "cl_close")) {
				printf("cl_close!\n");
			} else {
				printf("unhandle bt ble event: %s.\n", event.event.misc.type);
			}
		} else if (!strcasecmp(event.event.misc.name, "bt_hh")) {
			if (!strcasecmp(event.event.misc.type, "add_dev")) {
				ble_hh_add_dev_enable = 1;
			} else if (!strcasecmp(event.event.misc.type, "connected")) {

			} else if (!strcasecmp(event.event.misc.type, "close")) {
			} else {
				printf("unhandle bt hh event: %s.\n", event.event.misc.type);
			}
		} else if (!strcasecmp(event.event.misc.name, "bt_avrcp")) {
			if (!strcasecmp(event.event.misc.type, "playing")) {
				printf("-----------playing!\n");
			} else if (!strcasecmp(event.event.misc.type, "paused")) {
				printf("pause!\n");
			} else if (!strcasecmp(event.event.misc.type, "stopped")) {
				printf("stopped!\n");
			} else if (!strcasecmp(event.event.misc.type, "track_change")) {//\C7и\E8\C1\CB
			printf("------> track_change -----<-----------\n");
				mozart_bluetooth_avk_send_get_element_att_cmd();
			} else if (!strcasecmp(event.event.misc.type, "play_pos")) {
				printf("play_pos: %d\n", event.event.misc.value[0]);
			}
		} else if (!strcasecmp(event.event.misc.name, "bt_ops")) {
			if (!strcasecmp(event.event.misc.type, "connected")) {

			} else if (!strcasecmp(event.event.misc.type, "disconnected")) {

			} else if (!strcasecmp(event.event.misc.type, "send_request")) {
				char *ops_file_name = NULL;
				ops_file_name = mozart_bluetooth_ops_get_object_name();
				printf("ops_file_name = %s\n", ops_file_name);
				printf("len = %d\n", strlen(ops_file_name));
			} else if (!strcasecmp(event.event.misc.type, "send_start")) {

			} else if (!strcasecmp(event.event.misc.type, "send_end")) {

			}
		} else if (!strcasecmp(event.event.misc.name, "bt_dg")) {
			if (!strcasecmp(event.event.misc.type, "open")) {
				printf("state: %d\n", event.event.misc.value[0]);
				if (event.event.misc.value[0] == 0)
					bt_dg_flag = DG_CONNECTED;
				else if (event.event.misc.value[0] == 1)
					bt_dg_flag = DG_CONNECT_FAILED;
			} else if (!strcasecmp(event.event.misc.type, "disconnected")) {
				bt_dg_flag = DG_DISCONNECTED;
			} else if (!strcasecmp(event.event.misc.type, "find_service")) {
			}
		/* phone book client */
		} else if (!strcasecmp(event.event.misc.name, "bt_pbc")) {
			if (!strcasecmp(event.event.misc.type, "connected")) {
#if (SUPPORT_BSA_PBC == 1)
				mozart_bluetooth_pbc_get_phonebook(tel_str[TEL_PB_PATH]);
#endif /* SUPPORT_BSA_PBC */
			} else if (!strcasecmp(event.event.misc.type, "disconnected")) {

			}
#endif /* SUPPORT_BT */
		} else if (!strcasecmp(event.event.misc.name, "dlna")) {
			printf("mozart_vr_get_status = %d\n",mozart_vr_get_status());
			if(VR_ASR != mozart_vr_get_status()){
				if (!strcasecmp(event.event.misc.type, "connected")) {
					; // do nothing on dlna device connected event
				} else if (!strcasecmp(event.event.misc.type, "disconnected")) {
					; // do nothing on dlna device disconnected event
				} else if (!strcasecmp(event.event.misc.type, "play")){
					if (mozart_module_stop())
						share_mem_set(RENDER_DOMAIN, RESPONSE_CANCEL);
					else
						share_mem_set(RENDER_DOMAIN, RESPONSE_DONE);
				} else if (!strcasecmp(event.event.misc.type, "pause")){
					printf("dlna player paused.\n");
				} else if (!strcasecmp(event.event.misc.type, "resume")){
					printf("dlna player resume.\n");
				} else {
					printf("unhandle dlna event: %s.\n", event.event.misc.type);
				}
			}
		} else if (!strcasecmp(event.event.misc.name, "musicplayer")) {
			if (!strcasecmp(event.event.misc.type, "play")) {
				if (mozart_module_stop())
					share_mem_set(MUSICPLAYER_DOMAIN, RESPONSE_CANCEL);
				else
					share_mem_set(MUSICPLAYER_DOMAIN, RESPONSE_DONE);
			} else if (!strcasecmp(event.event.misc.type, "playing")) {
				printf("8888 music player playing. g_playing_flag = %d\n",g_playing_flag);
				if(!g_playing_flag)
					g_playing_flag = 1;
				//create_music_playing_pthread();
				
			} else if (!strcasecmp(event.event.misc.type, "pause")) {
				g_prevolume = mozart_musicplayer_get_volume(mozart_musicplayer_handler);
				printf("8888 music player paused.g_prevolume = %d \n",g_prevolume);
				create_breath_pause_pthread();
				
			} else if (!strcasecmp(event.event.misc.type, "resume")) {
				usleep(100*1000);//命令恢复后表情未切回播放
				printf("8888 music player resume.\n");
				led_music_playing();
				//mozart_musicplayer_set_volume(mozart_musicplayer_handler,g_prevolume);
			} else {
				printf("unhandle music event: %s.\n", event.event.misc.type);
			}
		} else if (!strcasecmp(event.event.misc.name, "airplay")) {
			if (!strcasecmp(event.event.misc.type, "connected")) {
				if (mozart_module_stop())
					share_mem_set(AIRPLAY_DOMAIN, RESPONSE_CANCEL);
				else
					share_mem_set(AIRPLAY_DOMAIN, RESPONSE_DONE);
			} else if (!strcasecmp(event.event.misc.type, "disconnected")) {
				printf("phone disconnected, airplay play stop.\n");
			} else if (!strcasecmp(event.event.misc.type, "paused")) {
				printf("airplay play paused.\n");
			} else if (!strcasecmp(event.event.misc.type, "resumed")) {
				printf("airplay play resumed.\n");
			}
		} else if (!strcasecmp(event.event.misc.name, "localplayer")) {
			if (!strcasecmp(event.event.misc.type, "play")) {
				if (mozart_module_stop())
					share_mem_set(LOCALPLAYER_DOMAIN, RESPONSE_CANCEL);
				else
					share_mem_set(LOCALPLAYER_DOMAIN, RESPONSE_DONE);
			} else if (!strcasecmp(event.event.misc.type, "playing")) {
				printf("localplayer playing.\n");
			} else if (!strcasecmp(event.event.misc.type,"pause")) {
				printf("localplayer paused.\n");
			} else if (!strcasecmp(event.event.misc.type,"resume")) {
				printf("localplayer resumed.\n");
			}
		} else if (!strcasecmp(event.event.misc.name,"vr")) {
			if (!strcasecmp(event.event.misc.type,"vr wake up"))
				mozart_ui_asr_wakeup();
			else if (!strcasecmp(event.event.misc.type,"vr failed"))
				mozart_ui_asr_failed();
			else if (!strcasecmp(event.event.misc.type,"vr unclear"))
				mozart_ui_asr_unclear();
#if (SUPPORT_MULROOM == 1)
		} else if (!strcasecmp(event.event.misc.name, "multiroom")) {
			if (!strcasecmp(event.event.misc.type, "group_dist")) {
				printf("group dist event\n");
				module_mulroom_audio_change(MR_AO_DISTRIBUTOR);
			} else if (!strcasecmp(event.event.misc.type, "dismiss_dist")) {
				printf("dismiss dist event\n");
				module_mulroom_audio_change(MR_AO_NORMAL);
			} else if (!strcasecmp(event.event.misc.type, "group_recv")) {
				printf("group recv event\n");
				mozart_module_stop();
				stopall(-1);
			} else if (!strcasecmp(event.event.misc.type, "dismiss_recv")) {
				printf("dismiss recv event\n");
				startall(-1);
			}
#endif /* SUPPORT_MULROOM == 1 */
		} else {
			printf("Unhandle event: %s-%s.\n", event.event.misc.name, event.event.misc.type);
		}
		break;
	case EVENT_KEY:
	default:
		break;
	}

	return;
}
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
	
	


	int fd = open("/tmp/sha1sum.txt",O_WRONLY|O_CREAT);
	if(fd > 0){
		now = time(NULL);
		sprintf(sha1sum_string,"%s%d%d",SECRETKEY,rand_num,now);
		sprintf(cur_time,"CurTime:%d",now);
		sprintf(nonce,"Nonce:%d",rand_num);	
		
		write(fd,sha1sum_string,strlen(sha1sum_string));
		
		printf("sha1sum_string = %s\n",sha1sum_string);
		printf("cur_time = %s\n",cur_time);
		printf("nonce = %s\n",nonce);
		struct tm  *tp = localtime(&now);
		printf("%d-%d-%d-", tp->tm_mon+1, tp->tm_mday, tp->tm_year + 1900);  
		printf("%d:%d:%d\n", tp->tm_hour, tp->tm_min, tp->tm_sec);
		close(fd);	
	}

	fp = popen("sha1sum /tmp/sha1sum.txt | cut -f 1 -d \" \"", "r");
	if (fp != NULL) {
		ret = fread(checksum_tmp, sizeof(char), sizeof(checksum_tmp), fp);
		if (ret > 0)
			snprintf(checksum,strlen(checksum_tmp) + 9,"CheckSum:%s",checksum_tmp);
		fclose(fp);
		fp = NULL;
	}

	printf("checksum = %s\n",checksum);



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

static void *sync_time_func(void *arg)
{//printf("sync_time_func=============================================>>>>>>>>>the tid = %d\n",syscall(SYS_gettid));
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
	luobo_upload_devinfo();
	return NULL;
}
int mqtt_start_flag = 0;
void *mqttsub_func(void *arg)
{//printf("mqttsub_func=============================================>>>>>>>>>the tid = %d\n",syscall(SYS_gettid));
	char macaddr[] = "255.255.255.255";
	char topic1[32] = {0}; 
	char topic2[32] = {0};	
	memset(macaddr, 0, sizeof(macaddr));
	get_mac_addr("wlan0", macaddr, "");
	sprintf(topic1,"/dev/NS-%s",&macaddr[6]);
	sprintf(topic2,"/dev/voice/NS-%s",&macaddr[6]);
	printf("-----macaddr = %s\n",macaddr);
	printf("-----topic1 = %s\n",topic1);
	printf("-----topic2 = %s\n",topic2);
	mqtt_start_flag = 1;
	//start_mqtt("120.24.75.220","61613","admin","password",topic1,topic2);	
	start_mqtt("119.23.207.229","1883","leden","ray123456",topic1,topic2);
	
	return NULL;
}

int create_mqttsub_pthread(void)
{
	printf("----------create_mqttsub_pthread-------------\n");
	if(mqtt_start_flag)
			return 0;
	pthread_t mqttsub_pthread;
	if (pthread_create(&mqttsub_pthread, NULL, mqttsub_func, NULL) == -1) {
		printf("Create wifi blink pthread failed: %s.\n", strerror(errno));
		return -1;
	}
	pthread_detach(mqttsub_pthread);

	return 0;
}

#include "my_fft.h"
char out[256];
typedef struct{
	unsigned char riff[4];
	unsigned char file_length[4];
	unsigned char wav_flag[4];
	unsigned char fmt_flag[4];
	unsigned char transition[4];
	unsigned char format_tag[2];
	unsigned char channels[2];
	unsigned char sample_rate[4];
	unsigned char wave_rate[4];
	unsigned char block_align[2];
	unsigned char sample_bits[2];
	unsigned char data_flag[4];
	unsigned char data_length[4];
}_mywav_header;
complex x[DECORDE_SIZE]; 

int decode_num(char *data,int len,char *out);
void clear_out(void);
FILE *audioP = NULL;//FIL audioP;
int audio_decorder(void)
{	
	int i=0;
	int temp;
	int frequency=0;
	unsigned int readsize;
	unsigned char *tmp;
	tmp=(unsigned char *)&temp;
	for(i=0;i<DECORDE_SIZE;i++){
		fread(tmp,2,1,audioP);//f_read(Fp,(void *)tmp,2,&readsize);
		x[i].real = temp;
		x[i].img = 0;
	}
	fft();
	frequency=output();
	temp=changefrequencytonum(frequency);
	return temp;
}

unsigned int crc8( char *buf,int len )
{
	if(len == 0)
		return -1;
	unsigned int crc = 0;
	unsigned int i;
	unsigned char *ptr = buf;

	    while(len--)
	    {
	       crc ^= *ptr++;
	       for(i = 0;i < 8;i++)
	       {
		   if(crc & 0x01)
		   {
		       crc = (crc >> 1) ^ 0x8C;
		   }else crc >>= 1;
	       }
	    }
	   

	//printf(" crc == %d \n",crc);
	return crc;
}

int readWave(char *audioName)
{	
	if(!audioName)
		return 0;
	unsigned int crc8_res;
	char decord[102400];
	int dindx=0;
	int Res;
	_mywav_header buff;
	int res=0;
	audioP = fopen(audioName,"rb");
	if(audioP != NULL){
		fread(&buff,1,sizeof(_mywav_header),audioP);
		dindx=0;
		memset(out,0,sizeof(out));//clear_out();
		while(!feof(audioP)){
			decord[dindx++] = audio_decorder();
			
		}
		res=decode_num(decord,dindx,out);
	}
	fclose(audioP);
	
	printf("####out = %s\n",out);
	printf("####res = %d\n",res);
	if(res > 1){
		crc8_res = crc8(out,(res - 1));
		printf("crc8_res = %d,out[res - 1] = %d\n",crc8_res,(unsigned char)out[res - 1]);
		if(crc8_res == (unsigned char)out[res - 1])
			return res;
		else
			return 0;
	}
	return 0;
}

int record_func(void)
{
	int res = 0;
	//usleep(4000*1000); //等待语音 “进入网络配置模式”   播放完
	acoustic_netconfig_start();
	printf("-------------------------------------------------------------->FFT\n");
	
	res = readWave("/tmp/asr_dmic_16.wav");
	system("rm /tmp/asr_dmic_16.wav");
	if(res){
		mozart_play_key("wav_config_succ");
		char result_decode[64] = {0};
		strncpy(result_decode,out,res - 1);
		printf("result_decode = %s\n",result_decode);

		char *tmp = NULL;
		char *result = NULL;
		tmp = result_decode;
		memset(wavconfig_ssid,0,sizeof(wavconfig_ssid));
		memset(wavconfig_passwd,0,sizeof(wavconfig_passwd));
		result = strsep(&tmp,"##");
		if(result){
			strsep(&tmp,"#");
			strcpy(wavconfig_ssid,result);
			strcpy(wavconfig_passwd,tmp);
			printf("~~~~wavconfig_ssid = %s\n",wavconfig_ssid);
			printf("~~~~wavconfig_passwd = %s\n",wavconfig_passwd);
		}
		return 0;
	}else{
		printf("~~~~ wav_config_failed\n");
		mozart_play_key("wav_config_failed");
		return -1;
	}
	
}
static int initall(void)
{
	// recover volume in S01system_init.sh

	//share memory init.
	if(0 != share_mem_init()){
		printf("share_mem_init failure.\n");
	}
	if(0 != share_mem_clear()){
		printf("share_mem_clear failure.\n");
	}

	if (mozart_path_is_mount("/mnt/sdcard")) {
		tfcard_status = 1;
		share_mem_set(SDCARD_DOMAIN, STATUS_INSERT);
	} else {
		share_mem_set(SDCARD_DOMAIN, STATUS_EXTRACT);
	}

	if (mozart_path_is_mount("/mnt/usb"))
		share_mem_set(UDISK_DOMAIN, STATUS_INSERT);
	else
		share_mem_set(UDISK_DOMAIN, STATUS_EXTRACT);

	// shairport ini_interface.hinit(do not depend network part)

	// TODO: render init if needed.

	// TODO: localplayer init if needed.

	// TODO: bt init if needed.

	// TODO: vr init if needed.

	// TODO: other init if needed.

	return 0;
}

int check_version()
{
	nvinfo_t *nvinfo = NULL;
	nvinfo = mozart_updater_chkver();
	if (strcmp(nvinfo->update_version,nvinfo->current_version) != 0) {
		system("rm /usr/data/voice/*");
		system("echo \"154\" > /sys/class/leds/led_red/brightness");
		system("echo \"50\" > /sys/class/leds/led_green/brightness");
		system("echo \"205\" > /sys/class/leds/led_blue/brightness");
		mozart_system("updater -p");
	}
	free(nvinfo);
	return 0;
}

pthread_t auto_reconnect_t;
int auto_reconnect_cnt = 0;
int auto_reconnect_flag = 0;
void *wifi_auto_reconnect(void *args)
{
	printf("=======wifi_auto_reconnect=======auto_reconnect_cnt = %d\n",auto_reconnect_cnt);
	wifi_ctl_msg_t new_mode;
	auto_reconnect_flag = 1;
	if(auto_reconnect_cnt++ < 3&&(mozart_bluetooth_get_link_status() != BT_LINK_CONNECTED)){
		
		mozart_play_key_sync("error_net_fail");
		mozart_module_stop();
		stopall(1);
		sleep(3);
		
		memset(&new_mode, 0, sizeof(new_mode));
		new_mode.cmd = SW_STA;
		new_mode.force = true;
		strcpy(new_mode.name, app_name);
		request_wifi_mode(new_mode);
	}else{
		sleep(300);
		if(auto_reconnect_cnt&&(mozart_bluetooth_get_link_status() != BT_LINK_CONNECTED)){
			auto_reconnect_cnt = 1;
			mozart_play_key_sync("error_net_fail");
			mozart_module_stop();
			stopall(1);
			sleep(3);
		
			memset(&new_mode, 0, sizeof(new_mode));
			new_mode.cmd = SW_STA;
			new_mode.force = true;
			strcpy(new_mode.name, app_name);
			request_wifi_mode(new_mode);
		}
	}
	auto_reconnect_flag = 0;
	return NULL;
}

int rayshine_auto_reconnect()
{
	if(auto_reconnect_flag)
		return 0;
	if (pthread_create(&auto_reconnect_t, NULL, wifi_auto_reconnect, NULL) == -1) {
		printf("Create wifi config pthread failed: %s.\n", strerror(errno));
		return -1;
	}
	pthread_detach(auto_reconnect_t);

	return 0;
}

int network_callback(const char *p)
{
	pthread_t sync_time_pthread;
	struct json_object *wifi_event = NULL;
	char test[64] = {0};
	wifi_ctl_msg_t new_mode;
	event_info_t network_event;

	printf("[%s] network event: %s\n", __func__, p);
	wifi_event = json_tokener_parse(p);
	//printf("event.to_string()=%s\n", json_object_to_json_string(event));

	memset(&network_event, 0, sizeof(event_info_t));
	struct json_object *tmp = NULL;
	json_object_object_get_ex(wifi_event, "name", &tmp);
	if(tmp != NULL){
		strncpy(network_event.name, json_object_get_string(tmp), strlen(json_object_get_string(tmp)));
		//printf(" name:%s\n", json_object_get_string(tmp));
	}

	json_object_object_get_ex(wifi_event, "type", &tmp);
	if(tmp != NULL){
		strncpy(network_event.type, json_object_get_string(tmp), strlen(json_object_get_string(tmp)));
		//printf("type:%s\n", json_object_get_string(tmp));
	}

	json_object_object_get_ex(wifi_event, "content", &tmp);
	if(tmp != NULL){
		strncpy(network_event.content, json_object_get_string(tmp), strlen(json_object_get_string(tmp)));
		//printf("content:%s\n", json_object_get_string(tmp));
	}

	memset(&new_mode, 0, sizeof(wifi_ctl_msg_t));
	if (!strncmp(network_event.type, event_type_str[STA_STATUS], strlen(event_type_str[STA_STATUS]))) 
	{
		//printf("[%s]: %s\n", network_event.type, network_event.content);
		null_cnt = 0;
		if(!strncmp(network_event.content, "STA_CONNECT_STARTING", strlen("STA_CONNECT_STARTING"))){
			printf("----------------------------mozart_ui_net_connecting \n");
			if(mozart_vr_get_status() == VR_ASR){   //如果是在唤醒状态下，退出唤醒模式
				mozart_stop_tone_sync();
				mozart_vr_asr_break();	
			}
			if (mozart_bluetooth_get_link_status() != BT_LINK_CONNECTED) 
				mozart_play_key("wifi_linking");
		}

		if(!strncmp(network_event.content, "STA_CONNECT_FAILED", strlen("STA_CONNECT_FAILED"))){
			json_object_object_get_ex(wifi_event, "reason", &tmp);
			if(tmp != NULL){
				strncpy(test, json_object_get_string(tmp), strlen(json_object_get_string(tmp)));
				printf("STA_CONNECT_FAILED REASON:%s\n", json_object_get_string(tmp));
			}
			{	printf("----------------------mozart_ui_net_connect_failed------------------------------------\n");
				if(mozart_vr_get_status() == VR_ASR){   //如果是在唤醒状态下，退出唤醒模式
					mozart_stop_tone_sync();
					mozart_vr_asr_break();	
				}

				if (mozart_bluetooth_get_link_status() != BT_LINK_CONNECTED) 
					mozart_play_key("wifi_link_failed");
				new_mode.cmd = SW_NEXT_NET;
				new_mode.force = true;
				strcpy(new_mode.name, app_name);
				new_mode.param.network_config.timeout = -1;
				memset(new_mode.param.network_config.key, 0, sizeof(new_mode.param.network_config.key));
				if(request_wifi_mode(new_mode) != true)
					printf("ERROR: [%s] Request Network Failed, Please Register First!!!!\n", app_name);
			}
		}
		if(!strncmp(network_event.content, "STA_SCAN_OVER", strlen("STA_SCAN_OVER"))){
			player_status_t status;
			wifi_ctl_msg_t new_mode;
			status = mozart_player_getstatus(mozart_musicplayer_handler->player_handler);
			if (status == PLAYER_PLAYING)
				mozart_play_pause();
			if (mozart_bluetooth_get_link_status() != BT_LINK_CONNECTED) 
				rayshine_auto_reconnect();

		}
	} else if (!strncmp(network_event.type, event_type_str[NETWORK_CONFIGURE], strlen(event_type_str[NETWORK_CONFIGURE]))) {
		//printf("[%s]: %s\n", network_event.type, network_event.content);
		null_cnt = 0;
		if(!strncmp(network_event.content, network_configure_status_str[AIRKISS_STARTING], strlen(network_configure_status_str[AIRKISS_STARTING]))) {
			printf("----------------NETWORK_CONFIGURE\n");
			mozart_play_key("airkiss_config");
			mozart_ui_net_config();
			
		}

		if(!strncmp(network_event.content, network_configure_status_str[AIRKISS_FAILED], strlen(network_configure_status_str[AIRKISS_FAILED]))) {
			mozart_ui_net_config_failed();
			printf("----------------------mozart_ui_net_config_failed------------------------------------\n");
			memory_domain domain;
			share_mem_get_active_domain(&domain);
			switch (domain)
			{	
			case MUSICPLAYER_DOMAIN:
				break;
			default:
					printf("[%s, %d]: %s domain is active\n", __func__, __LINE__,memory_domain_str[domain]);
				break;

			}
			snd_source = 0;

			mozart_play_key("airkiss_config_fail");

		

		} else if (!strncmp(network_event.content, network_configure_status_str[AIRKISS_SUCCESS], strlen(network_configure_status_str[AIRKISS_SUCCESS]))) {
			printf("----------------------mozart_ui_net_config_success------------------------------------\n");
			acoustic_netconfig_flag = false;
			memory_domain domain;
			share_mem_get_active_domain(&domain);
			switch (domain)
			{	
			case MUSICPLAYER_DOMAIN:
				break;
			default:
				printf("[%s, %d]: %s domain is active\n", __func__, __LINE__,memory_domain_str[domain]);
				break;

			}
			mozart_ui_net_config_success();
			mozart_play_key("airkiss_config_success");
			{
#if 1
				json_object_object_get_ex(wifi_event, "ssid", &tmp);
				if(tmp != NULL){
					strncpy(test, json_object_get_string(tmp), strlen(json_object_get_string(tmp)));
					printf("####ssid:%s\n", json_object_get_string(tmp));
				}
				json_object_object_get_ex(wifi_event, "passwd", &tmp);
				if(tmp != NULL){
					strncpy(test, json_object_get_string(tmp), strlen(json_object_get_string(tmp)));
					printf("####passwd:%s\n", json_object_get_string(tmp));
				}
				json_object_object_get_ex(wifi_event, "ip", &tmp);
				if(tmp != NULL){
					strncpy(test, json_object_get_string(tmp), strlen(json_object_get_string(tmp)));
					printf("####ip:%s\n", json_object_get_string(tmp));
				}
#endif
			}


		} else if(!strncmp(network_event.content, network_configure_status_str[AIRKISS_CANCEL], strlen(network_configure_status_str[AIRKISS_CANCEL]))) {
			printf("==================AIRKISS_CANCEL====================\n");

		}
	} else if (!strncmp(network_event.type, event_type_str[WIFI_MODE], strlen(event_type_str[WIFI_MODE]))) {
		//printf("[%s]: %s\n", network_event.type, network_event.content);
		json_object_object_get_ex(wifi_event, "last", &tmp);
		if(tmp != NULL){
			strncpy(test, json_object_get_string(tmp), strlen(json_object_get_string(tmp)));
			//printf("last:%s\n", json_object_get_string(tmp));
		}
		wifi_info_t infor = get_wifi_mode();
		mozart_system("killall ntpd > /dev/null 2>&1");
		mozart_system("killall dnsmasq > /dev/null 2>&1");
		if (infor.wifi_mode == AP) {
			printf("======AP======\n");
			null_cnt = 0;
			mozart_ui_net_connect_failed();
			mozart_led_turn_off(LED_RECORD);
			mozart_led_turn_slow_flash(LED_WIFI);
			//mozart_play_key("wifi_ap_mode");
			startall(1);
		} else if (infor.wifi_mode == STA) {
		printf("===STA=======\n");
			auto_reconnect_cnt = 0;
			null_cnt = 0;
			check_version();
			if (pthread_create(&sync_time_pthread, NULL, sync_time_func, NULL) == -1)
				printf("Create sync time pthread failed: %s.\n", strerror(errno));
			pthread_detach(sync_time_pthread);
			mozart_system("dnsmasq &");

			startall(1);
			//create_mqttsub_pthread();
			if(set_battery > 20)
				create_camera_pthread();
			else
				printf("battery is low,stop camera\n");
			
		} else if (infor.wifi_mode == WIFI_NULL) {
		printf("====WIFI NULL====\n");
			null_cnt++;
			if(null_cnt >= 10){
				null_cnt = 0;
				new_mode.cmd = SW_NETCFG;
				new_mode.force = true;
				strcpy(new_mode.name, app_name);
				new_mode.param.network_config.timeout = -1;
				memset(new_mode.param.network_config.key, 0, sizeof(new_mode.param.network_config.key));
				new_mode.param.network_config.method |= COOEE;
				strcpy(new_mode.param.network_config.wl_module, wifi_module_str[BROADCOM]);
				if(request_wifi_mode(new_mode) != true)
					printf("ERROR: [%s] Request Network Failed, Please Register First!!!!\n", app_name);
			}
		} else {
			printf("[ERROR]: Unknown event type!!!!!!\n");
		}
	} else if (!strncmp(network_event.type, event_type_str[AP_STATUS], strlen(event_type_str[AP_STATUS]))) {
		//printf("[%s]: %s\n", network_event.type, network_event.content);
		if(!strncmp(network_event.content, "AP-STA-CONNECTED", strlen("AP-STA-CONNECTED"))) {
			printf("\n-----------------------The client has the connection is successful.\n");
		} else if (!strncmp(network_event.content, "AP-STA-DISCONNECTED", strlen("AP-STA-DISCONNECTED"))) {
			printf("\n-------------------The client has been disconnected.\n");
		}
	} else if (!strncmp(network_event.type, event_type_str[STA_IP_CHANGE], strlen(event_type_str[STA_IP_CHANGE]))) {
		printf("[WARNING] STA IP ADDR HAS CHANGED!!\n");
	} else {
		printf("Unknown Network Events-[%s]: %s\n", network_event.type, network_event.content);
	}

	json_object_put(wifi_event);
	return 0;
}

#if (SUPPORT_ALARM == 1)
int alarm_callback(struct alarm_info *a)
{
	printf("alarm callback id: %d, hour: %d, minute: %d, timestamp: %ld info: %s\n",
		   a->alarm_id, a->hour, a->minute, a->timestamp, a->prv_info.info);
	return 0;
}
#endif




static void dump_compile_info(void)
{
	time_t timep;
	struct passwd *pwd;
	char hostname[16] = "Unknown";

	time(&timep);
	pwd = getpwuid(getuid());
	gethostname(hostname, 16);
	printf("mozart compiled at %s on %s@%s\n", asctime(gmtime(&timep)), pwd->pw_name, hostname);
}


static void sig_usr(int signo)
{
        if(signo == SIGUSR1)
                printf("received SIGUSR1\n");
        else if(signo == SIGUSR2)
                printf("received SIGUSR2\n");
        else
                printf("received signal %d\n", signo);
}
#define ANSI_COLOR_RESET   "\x1b[0m"

#define my_pr(fmt, args...)		printf(ANSI_COLOR_RED fmt ANSI_COLOR_RESET,##args)


#define TM7711_IOC_MAGIC  	'H'
#define TM7711_IOC_MAXNR 	2

#define INNO_IOCTL_TM7711_READ_CH1_10MH     _IOW(TM7711_IOC_MAGIC, 1, int)

pthread_t tmptest_pthread;


void *getcapacity_pthread_func(void *args)
{	sleep(10);
	//printf("getcapacity_pthread_func=============================================>>>>>>>>>the tid = %d\n",syscall(SYS_gettid));
	printf("-------------getcapacity_pthread_func-----------------set_battery = %d\n",set_battery);
	int fd,ret;
	int val_1[1];
	int set_volume = 0;
	player_status_t status;

	while(1){
		fd=open("/dev/tm7711_adc" ,O_RDWR);		
		if (-1 == fd){
			printf("\n\n tm7711_adc open failure!\n\n");
			sleep(60);
			continue;
		}
		else
			printf("\n\n tm7711_adc open successful! fd = %d\n\n",fd);


		ret=ioctl(fd, INNO_IOCTL_TM7711_READ_CH1_10MH , &val_1);
		usleep(500*1000);
		ret = close(fd);
		if(!ret)
			printf("------------------------------close tm7711_adc successful fd = %d\n",fd);
		else
			printf("------------------------------close tm7711_adc failure fd = %d\n",fd);


		set_volume = val_1[0]&0x00ff;
		set_battery = (val_1[0]&0xff00)>>8;
		


		printf("getcapacity_pthread_func  =============>>>>>>>>>>>>>>>,set_battery = %d,\n",set_battery);

		if(set_battery > 50){
			if(set_battery > 100)
				set_battery = 100;
			sleep(10*60);
			if(low_power_flag)
				low_power_flag = 0;
		}
		else if(set_battery > 20 && set_battery <= 50){
			sleep(60);
			if(low_power_flag)
				low_power_flag = 0;
		}
		else if(set_battery <=20){
			status = mozart_player_getstatus(mozart_musicplayer_handler->player_handler);
			low_power_flag = 1;
			if (status == PLAYER_PLAYING){
				mozart_play_pause();
				usleep(100*1000);
				mozart_play_key("charge_prompt");
				mozart_play_pause();
			}else{
				mozart_play_key("charge_prompt");
			}	
			sleep(15);
		}
 
		
	}

	return NULL;
}


int create_getcapacity_pthread(void)
{
	printf("----------create_getcapacity_pthread-------------\n");

	
	if (pthread_create(&tmptest_pthread, NULL, getcapacity_pthread_func, NULL) == -1) {
		printf("Create create_tmptest_pthread pthread failed\n");
		return -1;
	}
	return 0;
}


int main(int argc, char **argv)
{	
	#define ANSI_COLOR_RED     "\x1b[31m"
	system("echo \"255\" > /sys/class/leds/led_red/brightness");//开机白色
	system("echo \"255\" > /sys/class/leds/led_green/brightness");
	system("echo \"255\" > /sys/class/leds/led_blue/brightness");

	my_pr("     #######             ########\n");
	my_pr("     #######             ########\n");
	my_pr("     #######             ########\n");
	my_pr("     #######             ########\n");
	my_pr("     #######             ########\n");
	my_pr("     #######             ########\n");
	my_pr("     #######             ########\n");
	my_pr("     #######             ########\n");
	my_pr("     #######             ########\n");
	my_pr("     #######             ########\n");
	my_pr("     #######             ########\n");
	my_pr("     ##########################\n");
	my_pr("     #######################\n");
	my_pr("     #####################                                 @ Rayshine \n");
	printf("make time:%s-%s\n",__DATE__,__TIME__);
	mozart_volume_set(rayshine_get_volume(), MUSIC_VOLUME);


	if(signal(SIGUSR1, sig_usr) == SIG_ERR)
                printf("can't catch SIGUSR1\n");
	
	int daemonize = 0;
	app_name = argv[0];
	wifi_ctl_msg_t new_mode;
	struct wifi_client_register wifi_info;
	
	
	/* Get command line parameters */
	int c;
	while (1) {
		c = getopt(argc, argv, "bBsSh");
		if (c < 0)
			break;
		switch (c) {
		case 'b':
		case 'B':
			daemonize = 1;
			break;
		case 's':
		case 'S':
			break;
		case 'h':
			usage(app_name);
			return 0;
		default:
			usage(app_name);
			return 1;
		}
	}

	/* run in the background */
	if (daemonize) {
		if (daemon(0, 1)) {
			perror("daemon");
			return -1;
		}
	}
	dump_compile_info();

#if (SUPPORT_MULROOM == 1)
	mulroom_state = mozart_mulroom_check_saved_mode(MULROOM_INFO_PATH);
#endif /* SUPPORT_MULROOM == 1 */


#if (SUPPORT_MULROOM == 1)
	if (mulroom_state != MUL_GROUP_RECEIVER) {
#if (SUPPORT_LOCALPLAYER == 1)
		mozart_localplayer_scan_callback(tfcard_scan_1music_callback, tfcard_scan_done_callback);
#endif
	}
#else
#if (SUPPORT_LOCALPLAYER == 1)
	mozart_localplayer_scan_callback(tfcard_scan_1music_callback, tfcard_scan_done_callback);
#endif
#endif /* SUPPORT_MULROOM == 1 */

	initall();

#if (SUPPORT_MULROOM == 1)
	if (mulroom_state != MUL_GROUP_RECEIVER)
		startall(0);
#else
	// start modules do not depend network.
	startall(0);
#endif /* SUPPORT_MULROOM == 1 */

	//register_battery_capacity();
	//create_capacity_alarm_pthread();
	create_getcapacity_pthread();
	// register key event
	keyevent_handler = mozart_event_handler_get(keyevent_callback, app_name);
	miscevent_handler = mozart_event_handler_get(miscevent_callback, app_name);

	// register network manager
	memset(&wifi_info, 0, sizeof(wifi_info));
	if(register_to_networkmanager(wifi_info, network_callback) != 0) {
		printf("ERROR: [%s] register to Network Server Failed!!!!\n", app_name);
	} else if(!access("/usr/data/wpa_supplicant.conf", R_OK)) {
		memset(&new_mode, 0, sizeof(new_mode));
		new_mode.cmd = SW_STA;
		new_mode.force = true;
		strcpy(new_mode.name, app_name);
		if(request_wifi_mode(new_mode) != true)
			printf("ERROR: [%s] Request Network Failed, Please Register First!!!!\n", app_name);
	}

#if (SUPPORT_ALARM == 1)
	struct alarm_info *alarm = NULL;
	//register alarm manager
	register_to_alarm_manager(alarm_callback);

	if ((alarm = mozart_alarm_check_wakeup_status()) != NULL) {
		printf("[%s] hour %d minute %d name %s info %s\n",
				app_name, alarm->hour, alarm->minute, alarm->name, alarm->prv_info.info);
		free(alarm);
	}

	mozart_alarm_start_alarm();
#endif

#if (SUPPORT_MULROOM == 1)
	if (mulroom_state != MUL_GROUP_RECEIVER) {
		// process linein insert event on startup.
		if (mozart_linein_is_in()) {
			printf("linein detect, switch to linein mode...\n");
			mozart_linein_on();
		}
	}
#else
	// process linein insert event on startup.
	if (mozart_linein_is_in()) {
		printf("linein detect, switch to linein mode...\n");
		mozart_linein_on();
	}
#endif /* SUPPORT_MULROOM == 1 */


	// signal hander,
	signal(SIGINT, sig_handler);
	signal(SIGUSR1, sig_handler);
	signal(SIGUSR2, sig_handler);
	signal(SIGTERM, sig_handler);
	signal(SIGBUS, sig_handler);
	signal(SIGSEGV, sig_handler);
	signal(SIGABRT, sig_handler);
	signal(SIGPIPE, SIG_IGN);

	system("echo 1 > /proc/sys/vm/overcommit_memory");

	while(1) {
		sleep(20);
		system("echo 3 > /proc/sys/vm/drop_caches");
	}

	return 0;
}
