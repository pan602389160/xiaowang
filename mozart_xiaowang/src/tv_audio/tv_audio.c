/*
	tv_audio.c
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include <fcntl.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <linux/input.h>

#include "json-c/json.h"
#include "event_interface.h"
#include "wifi_interface.h"
#include "tips_interface.h"
#include "power_interface.h"
#include "utils_interface.h"
#include "localplayer_interface.h"
#include "sharememory_interface.h"

#include "mozart_config.h"
#include "mozart_key_function.h"
#include "mozart_app.h"
#include "mozart_musicplayer.h"
#include "mozart_ui.h"

#include "modules/mozart_module_local_music.h"

#if (SUPPORT_MULROOM == 1)
#include "mulroom_interface.h"
#include "modules/mozart_module_mulroom.h"
#endif /* SUPPORT_MULROOM == 1 */

#include "tv_audio.h"

static event_handler *keyevent_handler = NULL;
static event_handler *miscevent_handler = NULL;
static int null_cnt = 0;

char *global_app_name = NULL;
device_mode_t global_mode = M_EMPTY;
int tfcard_status = -1;

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
#if 1
	/* TODO; musicplayer add queue */
	tfcard_status = 1;
	mozart_module_local_music_startup();
#endif
}
#endif	/* SUPPORT_LOCALPLAYER */

static void volume_down_handler(bool long_press)
{
	if (long_press)
		mozart_previous_song();
	else
		mozart_volume_down();
}

static void volume_up_handler(bool long_press)
{
	if (long_press)
		mozart_next_song();
	else
		mozart_volume_up();
}

static void play_pause_handler(bool long_press)
{
	mozart_play_pause();
}

static struct input_event_key_info input_event_key_infos[] = {
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
{
	struct timeval now;
	struct timespec timeout;
	struct input_event_key_info *info = (struct input_event_key_info *)args;

	pthread_mutex_lock(&info->lock);
	if (info->state == KEY_LONG_PRESS_CANCEL) {
		info->state = KEY_LONG_PRESS_INVALID;
		printf("%s short press\n", info->name);
		if (info->handler)
			info->handler(false);
		pthread_mutex_unlock(&info->lock);
		return NULL;
	}

	gettimeofday(&now, NULL);
	timeout.tv_sec = now.tv_sec + info->timeout_second;
	timeout.tv_nsec = now.tv_usec * 1000;

	pthread_cond_timedwait(&info->cond, &info->lock, &timeout);

	if (info->state == KEY_LONG_PRESS_CANCEL) {
		info->state = KEY_LONG_PRESS_INVALID;
		printf("%s short press\n", info->name);
		if (info->handler)
			info->handler(false);
		pthread_mutex_unlock(&info->lock);
		return NULL;
	}

	info->state = KEY_LONG_PRESS_DONE;

	printf("%s long press\n", info->name);
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
{
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

static void keyevent_callback(mozart_event event, void *param)
{
	switch (event.type) {
	case EVENT_KEY:
		if (event.event.key.key.type != EV_KEY) {
			printf("Only support keyboard now.\n");
			break;
		}
		printf("[DEBUG] key %s %s!!!\n",
				keycode_str[event.event.key.key.code],
				keyvalue_str[event.event.key.key.value]);

		if (input_event_handler(event.event.key.key) == 0)
			break;

		if (event.event.key.key.value == 1) {
			switch (event.event.key.key.code) {
			case KEY_RECORD:
				break;
			case KEY_F1:
				break;
			case KEY_PREVIOUSSONG:
				mozart_previous_song();
				break;
			case KEY_NEXTSONG:
				mozart_next_song();
				break;
			case KEY_MENU:
				mozart_snd_source_switch();
				break;
			case KEY_F3:            /* music music Shortcut key 1 */
				mozart_music_list(0);
				break;
			case KEY_F4:            /* music music Shortcut key 2 */
				mozart_music_list(1);
				break;
			case KEY_F5:            /* music music Shortcut key 3 */
				mozart_music_list(2);
				break;
			case KEY_POWER:
				mozart_power_off();
				break;
			default:
				//printf("UnSupport key down in %s:%s:%d.\n", __FILE__, __func__, __LINE__);
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

static void miscevent_callback(mozart_event event, void *param)
{
	switch (event.type) {
	case EVENT_MISC:
		printf("[misc event] %s : %s.\n", event.event.misc.name, event.event.misc.type);
		if (!strcasecmp(event.event.misc.name, "linein")
#if (SUPPORT_MULROOM == 1)
			&& global_mode != M_MOON
#endif /* SUPPORT_MULROOM == 1 */
			) {
			if (!strcasecmp(event.event.misc.type, "plugin")) { // linein plugin
				mozart_ui_linein_plugin();
				mozart_module_stop();
				mozart_linein_on();
			} else if (!strcasecmp(event.event.misc.type, "plugout")) { // linein plugout
				mozart_ui_linein_plugout();
				mozart_linein_off();
			}
		} else if (!strcasecmp(event.event.misc.name, "tfcard")) {
			if (!strcasecmp(event.event.misc.type, "plugin")) { // tfcard plugin
				if (tfcard_status != 1) {
					mozart_ui_localplayer_plugin();
					mozart_play_key("tf_add");
					tfcard_status = 1;
				} else {
					// do nothing.
				}
			} else if (!strcasecmp(event.event.misc.type, "plugout")) { // tfcard plugout
				if (tfcard_status != 0) {
					mozart_ui_localplayer_plugout();
#if (SUPPORT_LOCALPLAYER == 1)
					if (snd_source == SND_SRC_LOCALPLAY)
						mozart_musicplayer_stop(mozart_musicplayer_handler);
#endif
					mozart_play_key("tf_remove");
					tfcard_status = 0;
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
			if (!strcasecmp(event.event.misc.type, "update music")) {
				printf("%s volume to %d.\n", event.event.misc.type, event.event.misc.value[0]);
			} else if (!strcasecmp(event.event.misc.type, "update unknown")) {
				printf("%s volume to %d.\n", event.event.misc.type, event.event.misc.value[0]);
			} else {
				printf("unhandle volume event: %s.\n", event.event.misc.type);
			}
		} else if (!strcasecmp(event.event.misc.name, "musicplayer")) {
			if (!strcasecmp(event.event.misc.type, "play")) {
				if (mozart_module_stop())
					share_mem_set(MUSICPLAYER_DOMAIN, RESPONSE_CANCEL);
				else
					share_mem_set(MUSICPLAYER_DOMAIN, RESPONSE_DONE);
			} else if (!strcasecmp(event.event.misc.type, "playing")) {
				printf("music player playing.\n");
			} else if (!strcasecmp(event.event.misc.type, "pause")) {
				printf("music player paused.\n");
			} else if (!strcasecmp(event.event.misc.type, "resume")) {
				printf("music player resume.\n");
			} else {
				printf("unhandle music event: %s.\n", event.event.misc.type);
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
				stopall(1);
			} else if (!strcasecmp(event.event.misc.type, "dismiss_recv")) {
				printf("dismiss recv event\n");
				startall(1);
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

static int network_callback(const char *p)
{
	struct json_object *wifi_event = NULL;
	char test[64] = {0};
	wifi_ctl_msg_t new_mode;
	event_info_t network_event;
	int err;

	printf("[%s] network event: %s\n", __func__, p);
	wifi_event = json_tokener_parse(p);
	//printf("event.to_string()=%s\n", json_object_to_json_string(event));

	memset(&network_event, 0, sizeof(event_info_t));
	struct json_object *tmp = NULL;
	json_object_object_get_ex(wifi_event, "name", &tmp);
	if (tmp != NULL) {
		strncpy(network_event.name, json_object_get_string(tmp), strlen(json_object_get_string(tmp)));
		//printf("name:%s\n", json_object_get_string(tmp));
	}

	json_object_object_get_ex(wifi_event, "type", &tmp);
	if (tmp != NULL) {
		strncpy(network_event.type, json_object_get_string(tmp), strlen(json_object_get_string(tmp)));
		//printf("type:%s\n", json_object_get_string(tmp));
	}

	json_object_object_get_ex(wifi_event, "content", &tmp);
	if (tmp != NULL) {
		strncpy(network_event.content, json_object_get_string(tmp), strlen(json_object_get_string(tmp)));
		//printf("content:%s\n", json_object_get_string(tmp));
	}

	memset(&new_mode, 0, sizeof(wifi_ctl_msg_t));
	if (!strncmp(network_event.type, event_type_str[STA_STATUS], strlen(event_type_str[STA_STATUS]))) {
		//printf("[%s]: %s\n", network_event.type, network_event.content);
		null_cnt = 0;
		if (!strncmp(network_event.content, "STA_CONNECT_STARTING", strlen("STA_CONNECT_STARTING"))) {
			mozart_ui_net_connecting();
			mozart_play_key("wifi_linking");
		}

		if (!strncmp(network_event.content, "STA_CONNECT_FAILED", strlen("STA_CONNECT_FAILED"))) {
			json_object_object_get_ex(wifi_event, "reason", &tmp);
			if(tmp != NULL){
				strncpy(test, json_object_get_string(tmp), strlen(json_object_get_string(tmp)));
				printf("STA_CONNECT_FAILED REASON:%s\n", json_object_get_string(tmp));
			}
			{
				mozart_ui_net_connect_failed();
				mozart_play_key("wifi_link_failed");
				new_mode.cmd = SW_NEXT_NET;
				new_mode.force = true;
				strcpy(new_mode.name, global_app_name);
				new_mode.param.network_config.timeout = -1;
				memset(new_mode.param.network_config.key, 0, sizeof(new_mode.param.network_config.key));
				if(request_wifi_mode(new_mode) != true)
					printf("ERROR: [%s] Request Network Failed, Please Register First!!!!\n", global_app_name);
			}
		}
		if (!strncmp(network_event.content, "STA_SCAN_OVER", strlen("STA_SCAN_OVER"))) {
			new_mode.cmd = SW_NETCFG;
			new_mode.force = true;
			strcpy(new_mode.name, global_app_name);
			new_mode.param.network_config.timeout = -1;
			memset(new_mode.param.network_config.key, 0, sizeof(new_mode.param.network_config.key));
			new_mode.param.network_config.method |= COOEE;
			strcpy(new_mode.param.network_config.wl_module, wifi_module_str[BROADCOM]);
			if(request_wifi_mode(new_mode) != true)
				printf("ERROR: [%s] Request Network Failed, Please Register First!!!!\n", global_app_name);
		}
	} else if (!strncmp(network_event.type, event_type_str[NETWORK_CONFIGURE], strlen(event_type_str[NETWORK_CONFIGURE]))) {
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
			null_cnt = 0;
			mozart_ui_net_connect_failed();
			mozart_led_turn_off(LED_RECORD);
			mozart_led_turn_slow_flash(LED_WIFI);
			mozart_play_key("wifi_ap_mode");

#if (SUPPORT_MULROOM == 1)
			err = module_mulroom_group_master(1);
			if (err < 0) {
				printf("Error: [%s] set mulroom master mode failed\n", global_app_name);
				return -1;
			}
#endif /* SUPPORT_MULROOM == 1 */

			startall(1);
		} else if (infor.wifi_mode == STA) {
			char gate_ip[16] = {0};
			char *host_name = NULL;

			mozart_ui_net_connected();
			null_cnt = 0;
			mozart_led_turn_off(LED_RECORD);
			mozart_led_turn_on(LED_WIFI);
			mozart_play_key("wifi_sta_mode");

			mozart_system("dnsmasq &");

#if (SUPPORT_MULROOM == 1)
			module_mulroom_get_gateip(gate_ip);
			host_name = module_mulroom_get_hostname();
			if (!host_name)
				return -1;

			printf("@@ host_name: %s, gate_ip: %s\n", host_name, gate_ip);
			err = module_mulroom_group_slave(1, host_name, gate_ip);
			free(host_name);
			if (err < 0) {
				printf("Error: [%s] set mulroom slave mode failed\n", global_app_name);
				return -1;
			}
#endif /* SUPPORT_MULROOM == 1 */
		} else if (infor.wifi_mode == WIFI_NULL) {
			null_cnt++;
			if(null_cnt >= 10){
				null_cnt = 0;
				printf("[Error]: WIFI NULL\n");
			}
		} else {
			printf("[ERROR]: Unknown event type!!!!!!\n");
		}
	} else if (!strncmp(network_event.type, event_type_str[AP_STATUS], strlen(event_type_str[AP_STATUS]))) {
		//printf("[%s]: %s\n", network_event.type, network_event.content);
		if(!strncmp(network_event.content, "AP-STA-CONNECTED", strlen("AP-STA-CONNECTED"))) {
			printf("\nThe client has the connection is successful.\n");
		} else if (!strncmp(network_event.content, "AP-STA-DISCONNECTED", strlen("AP-STA-DISCONNECTED"))) {
			printf("\nThe client has been disconnected.\n");
		}
	} else if (!strncmp(network_event.type, event_type_str[STA_IP_CHANGE], strlen(event_type_str[STA_IP_CHANGE]))) {
		printf("[WARNING] STA IP ADDR HAS CHANGED!!\n");
	} else {
		printf("Unknown Network Events-[%s]: %s\n", network_event.type, network_event.content);
	}

	json_object_put(wifi_event);
	return 0;
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

	return 0;
}

int tv_audio_start(char *app_name, device_mode_t mode)
{
	struct wifi_client_register wifi_info;
	wifi_ctl_msg_t w_mode;
	int err;

	global_app_name	= app_name;
	global_mode	= mode;

#if (SUPPORT_LOCALPLAYER == 1)
	mozart_localplayer_scan_callback(tfcard_scan_1music_callback, tfcard_scan_done_callback);
#endif

	initall();

	if (mode != M_MOON)
		startall(0);

	/* register key event */
	keyevent_handler = mozart_event_handler_get(keyevent_callback, app_name);
	miscevent_handler = mozart_event_handler_get(miscevent_callback, app_name);

	/* register network manager */
	memset(&wifi_info, 0, sizeof(wifi_info));
	if (register_to_networkmanager(wifi_info, network_callback) != 0) {
		printf("ERROR: [%s] register to Network Server Failed!!!!\n", app_name);
		return -1;
	}

	memset(&w_mode, 0, sizeof(wifi_ctl_msg_t));

	switch (mode) {
	case M_PLANET:
		/* Config wifi to AP mode */
		/* TODO wifi config need use pthread */
		w_mode.force	= true;
		w_mode.cmd	= SW_AP;

		strcpy(w_mode.name, app_name);

		err = request_wifi_mode(w_mode);
		if (!err) {
			printf("ERROR: [%s] Request Network AP mode Failed\n", app_name);
			return -1;
		}
		break;
	case M_MOON:
		/* Config wifi to STA mode */
		w_mode.force	= true;
		w_mode.cmd	= SW_STA;

		w_mode.param.switch_sta.sta_timeout = -1;
		strcpy(w_mode.name, app_name);

		err = request_wifi_mode(w_mode);
		if (!err) {
			printf("ERROR: [%s] Request Network STA mode Failed\n", app_name);
			return -1;
		}
		break;
	default:
		return -1;
	}

	return 0;
}

void tv_audio_terminate(void)
{
	printf("stop all services\n");
	stopall(-1);

	printf("unregister event manager\n");
	mozart_event_handler_put(keyevent_handler);
	mozart_event_handler_put(miscevent_handler);

	printf("unregister network manager\n");
	unregister_from_networkmanager();

	share_mem_clear();
	share_mem_destory();
}
