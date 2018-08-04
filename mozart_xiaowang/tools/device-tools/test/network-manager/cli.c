#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/types.h>
#include <CUnit/Basic.h>

#include "wifi_interface.h"

extern char *ctrl_ifname;

char *message_buf;
pthread_mutex_t message_pthread_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t message_pthread_cond = PTHREAD_COND_INITIALIZER;

static int network_callback(const char *p)
{
	pthread_mutex_lock(&message_pthread_lock);

	if (!strstr(p, "\"type\": \"STA_STATUS\", \"content\": \"SCANNING\"") &&
	    !strstr(p, "\"type\": \"STA_STATUS\", \"content\": \"STA_CONNECT_STARTING\"") &&
	    !strstr(p, "\"type\": \"STA_STATUS\", \"content\": \"ASSOCIATING\"") &&
	    !strstr(p, "\"type\": \"STA_STATUS\", \"content\": \"ASSOCIATED\"")) {
		if (message_buf == NULL)
			message_buf = strdup(p);

		pthread_cond_signal(&message_pthread_cond);
	}
	printf("====> callback message: %s\n", p);

	pthread_mutex_unlock(&message_pthread_lock);

	return 0;
}

static void cli_register_test(void)
{
	struct wifi_client_register cli_register_info;

	memset(&cli_register_info, 0, sizeof(struct wifi_client_register));
	CU_ASSERT_EQUAL(register_to_networkmanager(cli_register_info, network_callback), 0);
}

static __attribute__((unused)) CU_TestInfo cli_register_cases[] = {
	{"cli register test", cli_register_test},
	CU_TEST_INFO_NULL
};

CU_SuiteInfo network_cli_suites[] = {
	{"network cli register case:", NULL, NULL, NULL, NULL, cli_register_cases},
	CU_SUITE_INFO_NULL
};
