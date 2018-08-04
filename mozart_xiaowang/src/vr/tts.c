#include "vr_interface.h"

char *mozart_aispeech_tts(char *word)
{	
	return ai_tts(word);
}

void mozart_aispeech_tts_stop(void)
{
	tts_stop();
}
