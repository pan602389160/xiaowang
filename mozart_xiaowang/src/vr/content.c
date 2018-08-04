#include <stdio.h>
#include "vr_interface.h"

int mozart_vr_content_get(char *keyword)
{	
	char *real_keyword = keyword;
	if (!real_keyword)
		real_keyword = CONTENT_RANDOM_MUSIC;

	return content_get(real_keyword);
}

int mozart_vr_content_get_interrupt(void)
{
	content_interrupt();
	return 0;
}
