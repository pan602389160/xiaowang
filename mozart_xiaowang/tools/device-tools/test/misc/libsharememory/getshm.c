#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include "sharememory_interface.h"

int main(int argc, char *argv[])
{
	int i = 0;
	module_status status = STATUS_ERROR;

	share_mem_init();

	for (i = 0; i < get_domain_cnt(); i++) {
		if (i != UNKNOWN_DOMAIN) {
			if (i == TONE_DOMAIN)
				printf("%20s:\t%d tones to play\n",
					   memory_domain_str[i],
					   share_mem_statusget(i));
			else
				printf("%20s:\t%s(%d)\n",
					   memory_domain_str[i],
					   module_status_str[share_mem_statusget(i)],
					   share_mem_statusget(i));
		}
	}

	return 0;
}
