#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "aispeech/ai_vr.h"
#include "aispeech/slot.h"


const char *domain_type[DOMAIN_MAX]= {
				"null",
				"music",
				"netfm",
				"chat",
				"weather",
				"calendar",
				"reminder",
				"stock",
				"poetry",
				"movie"
				"command"
				};




void slot_free(asr_info_t *asr_info){

	free(asr_info->input);
	asr_info->input = NULL;

	asr_info->errId = 0;
	free(asr_info->error);
	asr_info->error = NULL;

	free_sem(&asr_info->sem);
	free_sds(&asr_info->sds);
}


int slot_resolve(json_object *sem_json, asr_info_t *asr_info){
	json_object *input = NULL;
	json_object *semantics = NULL;
	json_object *sds = NULL;

	/*  free all */
	slot_free(asr_info);

	if (json_object_object_get_ex(sem_json, "input", &input)){
		if (json_object_get_string(input)){
			asr_info->input = strdup(json_object_get_string(input));
		}
	}

//----------------------------------------------------------------- sds
	if (json_object_object_get_ex(sem_json, "sds", &sds)){
		get_sds(sds, &asr_info->sds);
 	}

	if (json_object_object_get_ex(sem_json, "semantics", &semantics)){
		get_sem(semantics, &asr_info->sem);
	}
	goto exit_slot;
//exit_error:
	slot_free(asr_info);
	return -1;
exit_slot:
	#if 1
		DEBUG("input : %s\n", asr_info->input);
	#endif
	return 0;
}



