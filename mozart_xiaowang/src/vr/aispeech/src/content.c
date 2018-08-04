#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include "ai_vr.h"
#include "slot.h"
#include "content.h"

static const char *_content_param =
"\
{\
    \"coreProvideType\": \"cloud\",\
    \"vadEnable\": 0,\
    \"app\": {\
        \"userId\": \"wifiBox\"\
    },\
    \"audio\": {\
        \"audioType\": \"wav\",\
        \"sampleBytes\": 2,\
        \"sampleRate\": 16000,\
        \"channel\": 1,\
		\"compress\": \"speex\"\
    },\
    \"request\": {\
        \"coreType\": \"cn.sds\",\
        \"res\": \"aihome\",\
 		\"sdsExpand\": {\
        	\"prevdomain\": \"\",\
			\"lastServiceType\": \"cloud\"\
		}\
    }\
}";

static bool is_end_flag = false;
static bool is_work_flag = false;


void content_free(content_info_t *content_info){
	content_info->errId = 0;
	free(content_info->error);
	content_info->error = NULL;
	free_sds(&content_info->sds);
	ai_vr_info.content.state = CONTENT_IDLE;
}


int _content_resolve(json_object *sem_json, content_info_t *content_info){
	json_object *sds = NULL;
	/*  free all */
//----------------------------------------------------------------- sds
	if (json_object_object_get_ex(sem_json, "sds", &sds)){
		get_sds(sds, &content_info->sds);
		return 0;
 	}
	content_free(content_info);
	return -1;
}

int _content_callback(void *usrdata, const char *id, int type, const void *message, int size)
{
    //printf("resp data: %.*s\n", size, (char *) message);
	json_object *out = NULL;
    json_object *result = NULL;
	json_object *errId = NULL;
	json_object *error = NULL;

    out = json_tokener_parse((char*) message);
    if (!out)
    {
        return -1;
    }

	if (json_object_object_get_ex(out, "result", &result)){
		content_free(&ai_vr_info.content);
#ifdef DEBUG_SHOW_ALL
		printf("%.*s\n", size, (char *) message);
#endif
		if(_content_resolve(result, &ai_vr_info.content) == -1){
			ai_vr_info.content.state = CONTENT_FAIL;
		} else {
			ai_vr_info.content.state = CONTENT_SUCCESS;
		}
		is_end_flag = true;
		ai_mutex_lock();
		ai_vr_info.from    = VR_FROM_CONTENT;
		ai_vr_info.content.state = CONTENT_SUCCESS;
		if (ai_vr_callback)
			ai_vr_callback(&ai_vr_info);
		ai_mutex_unlock();
    }

	if (json_object_object_get_ex(out, "error", &error)){
		content_free(&ai_vr_info.content);
		if (json_object_get_string(error)){
			ai_vr_info.content.error = strdup(json_object_get_string(error));
		}

		if (json_object_object_get_ex(out, "errId", &errId)){
			ai_vr_info.content.errId = json_object_get_int(errId);
		}

		is_end_flag = true;
		ai_mutex_lock();
		ai_vr_info.from    = VR_FROM_CONTENT;
		ai_vr_info.content.state = CONTENT_FAIL;
		if (ai_vr_callback)
			ai_vr_callback(&ai_vr_info);
		ai_mutex_unlock();
	}

//exit_error:
	if(out){
		json_object_put(out);
	}
    return 0;
}

void _content_stop(void){
	if (is_work_flag){
		if (sds_agn){
			aiengine_cancel(sds_agn);
		}
		is_work_flag = false;
	}
}

void *content_wait_func(void *arg)
{
	int timeout = CONTENT_TIMEOUT * 1000;
	int timeused = 0;

	pthread_detach(pthread_self());

    while (is_end_flag == false) {
        usleep(1000);
		timeused++;
		if (timeused > timeout) {
			PERROR("ERROR: get content timeout!\n");
			_content_stop();
			ai_mutex_lock();
			ai_vr_info.from    = VR_FROM_CONTENT;
			ai_vr_info.content.state = CONTENT_FAIL;
			ai_vr_info.content.errId = -1;
			ai_vr_info.content.error = strdup("get content timeout");
			if (ai_vr_callback)
				ai_vr_callback(&ai_vr_info);
			ai_mutex_unlock();
			return NULL;
		}
    }

    /* make compile happy. */
    return NULL;
}

int content_get(char *text){
	
	char uuid[64] = {0};
	const void *usrdata  ;//= NULL;
	char *_param = NULL;
	pthread_t content_wait_pthread;

	json_object *request = NULL;
	json_object *param = NULL;

	if (ai_vr_callback == NULL) {
		printf("ai_vr_callback is null\n");
		goto exit_error;
	}

	ai_vr_info.content.state = CONTENT_START;
	if (text == NULL){
		printf("text   = null !\n");
		goto exit_error;
	}

	param = json_tokener_parse(_content_param);
	if(!param) {
		printf("get param_js faild !\n");
		goto exit_error;
	}

	is_end_flag = false;
	is_work_flag = true;

	if (json_object_object_get_ex(param, "request", &request)){
		json_object_object_add(request, "refText", json_object_new_string(text));
		_param = (char *)json_object_to_json_string(param);
	}
	if (_param == NULL){
		PERROR("Error: get param faild !\n");
		goto exit_error;
	}
	aiengine_start(sds_agn,_param,
			uuid, _content_callback, &usrdata);
	aiengine_stop(sds_agn);

	if (pthread_create(&content_wait_pthread, NULL, content_wait_func, NULL)) {
		PERROR("create content_wait_pthread error: %s.\n", strerror(errno));
		return -1;
	}

	if(param){
		json_object_put(param);
	}
	return 0;

exit_error:
	if(param){
		json_object_put(param);
	}
	return -1;
}

void content_stop(void){
	return _content_stop();
}

void content_interrupt(void){
	ai_mutex_lock();
	_content_stop();
	ai_vr_info.from    = VR_FROM_CONTENT;
	ai_vr_info.content.state = CONTENT_INTERRUPT;
	if (ai_vr_callback)
		ai_vr_callback(&ai_vr_info);
	ai_mutex_unlock();
}

