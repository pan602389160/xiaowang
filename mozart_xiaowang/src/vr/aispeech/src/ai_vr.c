#include <stdio.h>
#include <unistd.h>
#include "json-c/json.h"

#include "aispeech/aiengine.h"
#include "aispeech/aiengine_app.h"
#include "aispeech/ai_vr.h"
#include "aispeech/slot.h"
#include "aispeech/content.h"

const char *vr_domain_str[] = {
	[DOMAIN_NULL] = "DOMAIN_NULL",
	[DOMAIN_MOVIE] = "DOMAIN_MOVIE",
	[DOMAIN_MUSIC] = "DOMAIN_MUSIC",
	[DOMAIN_WEATHER] = "DOMAIN_WEATHER",
	[DOMAIN_COMMAND] = "DOMAIN_COMMAND",
	[DOMAIN_CALENDAR] = "DOMAIN_CALENDAR",
	[DOMAIN_REMINDER] = "DOMAIN_REMINDER",
	[DOMAIN_NETFM] = "DOMAIN_NETFM",
	[DOMAIN_CHAT] = "DOMAIN_CHAT",
	[DOMAIN_STORY] = "DOMAIN_STORY",
	[DOMAIN_JOKE] = "DOMAIN_JOKE",
	[DOMAIN_POETRY] = "DOMAIN_POETRY",
	[DOMAIN_COOKBOOK] = "DOMAIN_COOKBOOK",
	[DOMAIN_STOCK] = "DOMAIN_STOCK",
	[DOMAIN_TRANSLATE] = "DOMAIN_TRANSLATE",
	[DOMAIN_MAX] = "DOMAIN_UNSUPPORY"
};

#ifdef   SUPPORT_AEC
struct aiengine *wakeup_agn = NULL;
const char *wakeup_conf = "{"
	    "\"luaPath\":\"/usr/share/vr/res/luabin.lub\","
	    "\"appKey\":\"1491395693859642\","
	    "\"secretKey\":\"9c0306099673c0bbaf777861b715ccd9\","
	    "\"provision\":\"/usr/share/vr/res/aiengine-2.9.2-1491395693859642.provision\","
	    "\"serialNumber\":\"/usr/data/serialNumber\","
	    "\"prof\":{"
	        "\"enable\":0,"
	        "\"output\":\"/usr/data/aiengine.log\""
	    "},"
	    "\"echo\":{"
	        "\"enable\":1,"
	        "\"actCfgFile\":\"/usr/share/vr/res/aec.cfg\","
	        "\"isBin\":0,"
	        "\"frameLen\":512,"
	        "\"recChannel\":1"
	    "},"
	    "\"native\":{"
	        "\"cn.wakeup\":{"
	            "\"resBinPath\":\"/usr/share/vr/res/wakeup_common_nhxw.bin\","
	            "\"useOutputBoundary\": 0"
	        "}"
	    "}"
	"}";
#endif

#if 0//#ifdef VALGRIND_TEST
static const char *sds_conf =
"\
{\
    \"luaPath\": \"./bin/luabin.lub\",\
    \"appKey\": \"14327742440003c5\",\
    \"secretKey\": \"59db7351b3790ec75c776f6881b35d7e\",\
    \"provision\": \"./bin/aiengine-2.8.8-14327742440003c5.provision\",\
    \"serialNumber\": \"/usr/data/serialNumber\",\
    \"prof\": {\
        \"enable\": 0,\
        \"output\": \"./a.log\"\
    },\
    \"vad\":{\
        \"enable\": 1,\
        \"res\": \"./bin/vad.aihome.v0.5.20160324.bin\",\
        \"speechLowSeek\": 70,\
        \"sampleRate\": 16000,\
        \"pauseTime\":700,\
        \"strip\": 1\
    },\
    \"cloud\": {\
		\"server\": \"ws://s.api.aispeech.com:10000\"\
    }\
}";
#else
static const char *sds_conf ="{"
        "\"luaPath\":\"/usr/share/vr/res/luabin.lub\","
        "\"appKey\":\"1491395693859642\","
        "\"secretKey\":\"9c0306099673c0bbaf777861b715ccd9\","
        "\"provision\":\"/usr/share/vr/res/aiengine-2.9.2-1491395693859642.provision\","
        "\"serialNumber\":\"/usr/data/serialNumber\","
        "\"prof\":{"
            "\"enable\":0,"
            "\"output\":\"/mnt/sdcard/music/aaaa/1-9.log\""
        "},"
        "\"vad\":{"
            "\"enable\":1,"
            "\"res\":\"/usr/share/vr/res/vad.aihome.v0.5.20160324.bin\","
            "\"sampleRate\":16000,"
            "\"pauseTime\":800,"
            "\"strip\":1"
        "},"
        "\"cloud\":{"
            "\"server\":\"ws://s.api.aispeech.com:1028,ws://s.api.aispeech.com:80\""
        "}"
    "}";

#endif

static const char *version =
"X1000 Demo Code\n"\
"Ver : 1.0.3\n"\
"Date: 2016-09-10";

vr_info_t ai_vr_info;
vr_callback ai_vr_callback;
pthread_mutex_t ai_lock;
#define TMP_BUFFER_SZ 1024
struct aiengine *sds_agn = NULL;
int ai_vr_init(vr_callback _callback){

	char buf[TMP_BUFFER_SZ] = {0};
	int ret = -1;
	json_object *cjs = NULL;
	json_object *result = NULL;

	/* print version info */
	printf("%s\n", version);

	sds_agn = aiengine_new(sds_conf);
	if(NULL == sds_agn)
	{
	    printf("Create sds engine error.\n");
	    goto ERROR;
	}
	printf("Create sds engine successfully.\n");
	/* auth process */
	aiengine_opt(sds_agn, 10, buf, TMP_BUFFER_SZ);

	printf("Get opt: %s\n", buf);
	cjs = json_tokener_parse(buf);
	if (cjs)
	{
		if (json_object_object_get_ex(cjs, "success", &result)){
	        ret = 0;
	    }
	    json_object_put(cjs);
	}

	if (ret) {
		int cnt = 60;
		system("aispeech_authorization &");
		while (!system("pgrep -x aispeech_authorization >/dev/null") && cnt > 0) {
			cnt--;
			usleep(100 * 1000);
		}
		if (cnt <= 0) {
			printf("aispeech authorization failed\n");
			system("killall aispeech_authorization >/dev/null");
			aiengine_delete(sds_agn);
			goto ERROR;
		}
	}

#ifdef   SUPPORT_AEC
	/* create wakeup engine */
	wakeup_agn = (struct aiengine *)aiengine_new(wakeup_conf);
	if(NULL == wakeup_agn)
	{
	    printf("Create wakeup aiengine error. \n");
	    goto ERROR;
	}
#endif

	ai_mutex_lock();
	ai_vr_callback = _callback;
	ai_mutex_unlock();
	DEBUG("Success init vr\n");
	return 0;
ERROR:
	return -1;
}



int ai_vr_uninit(void){


	free(ai_vr_info.asr.sds.recordId);
	ai_vr_info.asr.sds.recordId = NULL;

	if (ai_vr_info.content.state != CONTENT_IDLE)
		content_free(&ai_vr_info.content);

	if (ai_vr_info.asr.state != ASR_IDLE)
		slot_free(&ai_vr_info.asr);

#ifdef   SUPPORT_AEC
	if (wakeup_agn){
		aiengine_delete(wakeup_agn);
		wakeup_agn = NULL;
	}
#endif
	if(sds_agn){
		aiengine_delete(sds_agn);
		sds_agn = NULL;
	}
	ai_mutex_lock();
	if (ai_vr_callback){
		ai_vr_callback    = NULL;
	}
	ai_mutex_unlock();
	return 0;
}

int ai_vr_authorization(void)
{
	struct aiengine *agn;
	char buf[TMP_BUFFER_SZ] = {0};

	agn = aiengine_new(sds_conf);
	if (NULL == agn) {
		printf("Create sds engine error.\n");
		return -1;
	}

	aiengine_opt(agn, 11, buf, TMP_BUFFER_SZ);
	printf("%s\n", buf);

	aiengine_delete(agn);

	return 0;
}
