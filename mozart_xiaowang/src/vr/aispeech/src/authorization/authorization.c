#include <stdio.h>
#include "aispeech/aiengine.h"
#include "aispeech/aiengine_app.h"
#include "aispeech/ai_vr.h"
#include "aispeech/slot.h"

int ai_vr_authorization(void);

void content_free(content_info_t *content_info)
{
	;
}

void slot_free(asr_info_t *asr_info)
{
	;
}

int main(void)
{
	printf("aispeech authorization...\n");
	return ai_vr_authorization();
}
