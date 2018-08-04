#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "aispeech/ai_vr.h"
#include "aispeech/slot.h"

int g_translate_flag = 0;
char *g_translate_input = NULL;
static char *_asr_param =
"\
{\
    \"coreProvideType\": \"cloud\",\
    \"vadEnable\": 1,\
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
static char *_asr_mult_sds_param = NULL;

char *_new_asr_param(sds_info_t *sds_info){
	json_object *request = NULL;
	json_object *root = NULL;
	json_object *sdsExpand = NULL;
	if (sds_info->is_mult_sds == false){
	    goto exit_normal;
	}

	root = json_tokener_parse(_asr_param);
	if(!root) {
	    printf("get root faild !\n");
	    goto exit_normal;
	}

	if (json_object_object_get_ex(root, "request", &request)){
		if (json_object_object_get_ex(request, "sdsExpand", &sdsExpand)){
			if (sds_info->contextId){
				json_object_object_add(sdsExpand, "contextId", json_object_new_string(sds_info->contextId));
			}
			if (sds_info->env){
				json_object_object_add(request, "env", json_object_new_string(sds_info->env));
			}
		}
		free(_asr_mult_sds_param);
		_asr_mult_sds_param = NULL;
		_asr_mult_sds_param = strdup((char *)json_object_to_json_string(root));
		if(root){
		    json_object_put(root);
	    }
		return _asr_mult_sds_param;
	}
exit_normal:
	return _asr_param;
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

static int asr_time = 0;
int _asr_callback(void *usrdata, const char *id, int type, const void *message, int size)
{
    	//printf("_asr_callback -->resp data: %.*s\n", size, (char *) message);
	json_object *out = NULL;
	json_object *result = NULL;
	json_object *vad_status = NULL;
	json_object *recordId = NULL;
	json_object *errId = NULL;
	json_object *error = NULL;

	char *xxxx = NULL;
	xxxx = get_object_string("result",NULL,(char *)message);
	if(xxxx){
		//printf("xxxx  result = %s\n",xxxx);

		xxxx = get_object_string("semantics",NULL,xxxx);
		if(xxxx){
			//printf("xxxx  semantics = %s\n",xxxx);
			xxxx = get_object_string("request",NULL,xxxx);
			if(xxxx){
				//printf("xxxx  request = %s\n",xxxx);
				xxxx = get_object_string("param",NULL,xxxx);
				if(xxxx){
					//printf("xxxx  param = %s\n",xxxx);
					char *translate_to_language = NULL;
					char *caozuo = NULL;
					translate_to_language = get_object_string("目标语言",NULL,xxxx);
					caozuo = get_object_string("操作",NULL,xxxx);
					//printf("caozuo = %s,translate_to_language = %s\n",caozuo,translate_to_language);
					if(caozuo&&translate_to_language){
						if(!strcmp(caozuo,"翻译")&&!strcmp(translate_to_language,"英文")){
							g_translate_input = get_object_string("内容",NULL,xxxx);
							g_translate_flag = 666;
						}
					}
				}

				
			}
		}
		
	}

	out = json_tokener_parse((char*) message);
	if (!out)
	{
		return -1;
	}
	if (json_object_object_get_ex(out, "vad_status", &vad_status)){
		// vad_status  0 - start , 1 -speaking, 2 - end
	if(json_object_get_int(vad_status) == 0){
		asr_time++;
		//printf("asr_time = %d\n",asr_time);
	}
        if(json_object_get_int(vad_status)== 2 || asr_time >= 100)  {
            printf("*****************vad end**********************\n");
			asr_time = 0;
			slot_free(&ai_vr_info.asr);
			ai_mutex_lock();
			ai_vr_info.from    = VR_FROM_ASR;
			ai_vr_info.asr.state = ASR_SPEAK_END;
			if (ai_vr_callback)
				ai_vr_callback(&ai_vr_info);
			ai_mutex_unlock();
        }
        goto exit_error;
    }

	if (json_object_object_get_ex(out, "recordId", &recordId)){
		free(ai_vr_info.asr.sds.recordId);
		ai_vr_info.asr.sds.recordId = NULL;
		if (json_object_get_string(recordId)){
			ai_vr_info.asr.sds.recordId = strdup(json_object_get_string(recordId));
		}
	}		//*/


	if (json_object_object_get_ex(out, "result", &result)){
		slot_free(&ai_vr_info.asr);
#ifdef VALGRIND_TEST    	/* used pc test*/
		char *slot = slot_test();
		json_object *result_test = NULL;
  	 	result_test = json_tokener_parse(slot);
		if (result_test){
			if(slot_resolve(result_test, &ai_vr_info.asr) == -1){
				ai_vr_info.asr.state = ASR_FAIL;
			} else {
				ai_vr_info.asr.state = ASR_SUCCESS;
			}
			json_object_put(result_test);
		}	//*/
#else
#ifdef DEBUG_SHOW_ALL
		printf("%.*s\n", size, (char *) message);
#endif
		if(slot_resolve(result, &ai_vr_info.asr) == -1){
			ai_vr_info.asr.state = ASR_FAIL;
		} else {
			ai_vr_info.asr.state = ASR_SUCCESS;
		}
#endif
		ai_mutex_lock();
		ai_vr_info.from    = VR_FROM_ASR;
		ai_vr_info.asr.state = ASR_SUCCESS;
		if (ai_vr_callback)
			ai_vr_callback(&ai_vr_info);
		ai_mutex_unlock();
    }

	if (json_object_object_get_ex(out, "error", &error)){
#ifdef DEBUG_SHOW_ALL
		printf("%.*s\n", size, (char *) message);
#endif

		slot_free(&ai_vr_info.asr);
		if (json_object_get_string(error)){
			ai_vr_info.asr.error = strdup(json_object_get_string(error));
		}

		if (json_object_object_get_ex(out, "errId", &errId)){
			ai_vr_info.asr.errId = json_object_get_int(errId);
		}

		ai_mutex_lock();
		ai_vr_info.from    = VR_FROM_ASR;
		ai_vr_info.asr.state = ASR_FAIL;
		if (ai_vr_callback)
			ai_vr_callback(&ai_vr_info);
		ai_mutex_unlock();
	}


exit_error:
	if(out){
		json_object_put(out);
	}
    return 0;
}


int asr_start(bool sds)
{	printf("============asr_start=========================\n");

	char uuid[64] = {0};
	const void *usrdata  ;//= NULL;
	ai_vr_info.asr.state = ASR_START;
	if (sds)
		ai_vr_info.asr.sds.is_mult_sds = true;
	return aiengine_start(sds_agn,
			_new_asr_param(&ai_vr_info.asr.sds),
			uuid, _asr_callback, &usrdata);
}

int asr_stop(void){
	ai_vr_info.asr.state = ASR_IDLE;
   return aiengine_stop(sds_agn);
}

int asr_feed(const void *rec, int size){
	return aiengine_feed(sds_agn, rec, size);
}


int asr_cancel(void){
	return aiengine_cancel(sds_agn);
}

void asr_break(void){
	ai_mutex_lock();
	ai_vr_info.from    = VR_FROM_ASR;
	ai_vr_info.asr.state = ASR_BREAK;
	if (ai_vr_callback)
		ai_vr_callback(&ai_vr_info);
	ai_mutex_unlock();
}

