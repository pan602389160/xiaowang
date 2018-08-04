#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <json-c/json.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


#include "tips_interface.h"

#include "aispeech/ai_vr.h"
#include "aispeech/tts.h"
#define HT1628_IOC_MAGIC  	'H'
#define HT1628_IOC_MAXNR 	6


static const char *_tts_param =
"\
{\
    \"coreProvideType\": \"cloud\",\
    \"audio\": {\
        \"audioType\": \"mp3\",\
        \"channel\": 1,\
        \"sampleBytes\": 2,\
        \"sampleRate\": 16000,\
        \"compress\":\"raw\" \
    },\
    \"request\": {\
        \"coreType\": \"cn.sent.syn\",\
        \"res\": \"syn_chnsnt_qianranf\",\
        \"speechRate\": 0.85,\
        \"speechVolume\": 80\
    }\
}";

static char *fn = NULL;
static bool is_end_flag = false;
static bool is_work_flag = false;




int _tts_callback(void *usrdata, const char *id, int type, const void *message, int size){
	if (type == 1){
		PERROR("Error: type = 1\n");
		is_end_flag = true;
		return 0;
	} else if   (type == 2){
		if (size == 0){
			is_end_flag = true;
		} else {
			FILE *fout = NULL;
        	fout = fopen(fn, "a+");
        	if(fout)
        	{
            	fwrite(message, size, 1, fout);
            	fclose(fout);
        	}
		}
	}
    return 0;
}

int  _tts_wait(int t_ms){
	int timeout = 0;
    while(is_end_flag == false) {
        usleep(1000);
		timeout ++;
		if (timeout > t_ms){	//	timeout    10 s
			PERROR("ERROR: tts time out!\n");
			return -1;
		}
    }
	return 0;
}

void tts_stop(void){
	if (is_work_flag){
		is_end_flag = true;
		if (sds_agn){
			aiengine_cancel(sds_agn);
		}
		is_work_flag = false;

		if (fn){
		printf("%s:%d, fn: %s\n", __func__, __LINE__, fn);
			unlink(fn);
			fn = NULL;
		}
	}
}


char *ai_tts(char *tts_text){

	char uuid[64] = {0};
	const void *usrdata  ;//= NULL;
    char *_param = NULL;
    json_object *param = NULL;
    json_object *request = NULL;

#ifdef TTS_NAME
	fn = TTS_NAME;
#else
	int fd = -1;
 	char tempfile[] = "/tmp/tts_XXXXXX.mp3";
	fd = mkstemps(tempfile, 4);
	if (fd == -1){
		return NULL;
	}
	close(fd);
	fn = strdup(tempfile);
#endif

	DEBUG("=======tts syn = %s : %s =========\n", fn, tts_text);
    FILE *fout =NULL;
	fout = fopen(fn, "w+");
    if (fout){
   	 	fclose(fout);
    }
	else{
		 goto exit_error;
	}

	if ((tts_text == NULL) || (!strcmp(tts_text, ""))){
		PERROR("Error: tts_text empty!\n");
		 goto exit_error;
	}

	param = json_tokener_parse(_tts_param);
	if (!param){
	   	 PERROR("Error: get param faild !\n");
		 goto exit_error;
	}

	if (json_object_object_get_ex(param, "request", &request)){
		json_object_object_add(request, "refText", json_object_new_string(tts_text));
		_param = (char *)json_object_to_json_string(param);
	}
	if (_param == NULL){
	   	 PERROR("Error: get param faild !\n");
		 goto exit_error;
	}

	is_end_flag = false;
	is_work_flag = true;
    aiengine_start(sds_agn,(const char *)_param, uuid, _tts_callback, &usrdata);
    aiengine_stop(sds_agn);
	if (_tts_wait(TTS_TIMEOUT *1000) == -1){
		printf("????long_txt?????\n");
		tts_stop();
		mozart_play_key_sync("long_txt");
	}
	is_work_flag = false;

//	free(_param);
//	_param = NULL;
	if(param){
		json_object_put(param);
	}
	
	return fn;

exit_error:
//	free(_param);
//	_param = NULL;
	if(param){
		json_object_put(param);
	}

#ifndef TTS_NAME
	if (fn){
		printf("%s:%d, fn: %s\n", __func__, __LINE__, fn);
		unlink(fn);
		fn = NULL;
	}
#endif

	return NULL;
}


