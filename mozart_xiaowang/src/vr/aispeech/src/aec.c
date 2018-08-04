#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <json-c/json.h>

#include "aispeech/ai_vr.h"


static const char *param = "{"
    "\"coreProvideType\":\"native\","
    "\"app\":{"
        "\"userId\":\"LeChange\""
    "},"
    "\"request\":{"
        "\"coreType\":\"cn.wakeup\","
        "\"env\":\"\""
    "}"
"}";

#define LIGHT_RED     "\033[1;31m"
#define LIGHT_PURPLE "\033[1;35m"
#define NONE          "\033[m"
#define my_pr(fmt, args...)		printf(LIGHT_RED fmt NONE,##args)
//static int cnt = 0;

int _aec_callback(void *usrdata, const char *id, int type, const void *message, int size){
//    my_pr("_aec_callback resp data: %.*s\n", size, (char *) message);
	json_object *out = NULL;
	json_object *result = NULL;
	json_object *status = NULL;
	out = json_tokener_parse((char*) message);
	if (!out)
	{
		return -1;
	}
//	printf("_aec_callback ->out = %s\n",json_object_get_string(out));
	if (json_object_object_get_ex(out, "result", &result)){
		if (json_object_object_get_ex(result, "status", &status)){
	        if (json_object_get_int(status) == 1){
				DEBUG(" =======>voice wake up SUCCESSFULLY<=======\n");
				// rgb  dd4001
				system("echo \"221\" > /sys/class/leds/led_red/brightness");
				system("echo \"64\" > /sys/class/leds/led_green/brightness");
				system("echo \"01\" > /sys/class/leds/led_blue/brightness");
				ai_mutex_lock();
				ai_vr_info.from    = VR_FROM_AEC;
				ai_vr_info.aec.wakeup = true;
				if (ai_vr_callback)
					ai_vr_callback(&ai_vr_info);
				ai_mutex_unlock();
	        }
		}
    }
	json_object_put(out);

    return 0;
}

int aec_start(void){
    char uuid[64] = {0};
	my_pr("\n aec_start param:%s\n", param);
	return aiengine_start(wakeup_agn, param, uuid, _aec_callback, NULL);
}

int aec_stop(void){
   return aiengine_stop(wakeup_agn);
}

int aec_feed(const void *rec, const void *play, int size)
{
   return aiengine_echo(wakeup_agn, rec, play, size, 0);
}

int aec_key_wakeup(void)
{
	DEBUG(" =======>key wake up SUCCESSFULLY<=======\n");
	// rgb  dd4001
	system("echo \"221\" > /sys/class/leds/led_red/brightness");
	system("echo \"64\" > /sys/class/leds/led_green/brightness");
	system("echo \"01\" > /sys/class/leds/led_blue/brightness");
	ai_mutex_lock();
	ai_vr_info.from    = VR_FROM_AEC;
	ai_vr_info.aec.wakeup = true;
	if (ai_vr_callback)
		ai_vr_callback(&ai_vr_info);
	ai_mutex_unlock();

	return 0;
}
