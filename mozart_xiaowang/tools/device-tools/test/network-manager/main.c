#include <stdio.h>
#include <stdlib.h>
#include <libgen.h>
#include <CUnit/Basic.h>

#include "wifi_interface.h"

char *app_name;
char *ctrl_ifname = "wlan0";

extern CU_SuiteInfo network_cli_suites[];
extern CU_SuiteInfo network_ap_suites[];
extern CU_SuiteInfo network_sta_suites[];
extern CU_SuiteInfo network_config_suites[];
extern CU_SuiteInfo network_eth_suites[];

int stop_wifi_mode(void)
{
	wifi_ctl_msg_t stop_wifi;

	memset(&stop_wifi, 0, sizeof(wifi_ctl_msg_t));
	stop_wifi.cmd = STOP_WIFI;
	request_wifi_mode(stop_wifi);

	return 0;
}

int stop_wifi_mode_and_clear(void)
{
	stop_wifi_mode();

	system("echo \"ctrl_interface=/var/run/wpa_supplicant\" > /usr/data/wpa_supplicant.conf");
	system("echo \"ap_scan=1\" >> /usr/data/wpa_supplicant.conf");

	return 0;
}

int main(int argc, char *argv[])
{
	app_name = basename(argv[0]);

	if (system("ps | grep -v grep | grep \"network_manager -c 10 -B\" > /dev/null")) {
		printf("Please run 'netowrk_manager -c 10 -B'\n");
		return -1;
	}

	if (CU_initialize_registry() != CUE_SUCCESS) {
		fprintf(stderr, "Initialization of Test Registry failed.\n");
		return -1;
	}
	if (CU_register_suites(network_cli_suites) != CUE_SUCCESS) {
		fprintf(stderr, "Register cli suites failed - %s ", CU_get_error_msg());
		return -1;
	}
	if (CU_register_suites(network_ap_suites) != CUE_SUCCESS) {
		fprintf(stderr, "Register ap suites failed - %s ", CU_get_error_msg());
		return -1;
	}
	if (CU_register_suites(network_sta_suites) != CUE_SUCCESS) {
		fprintf(stderr, "Register sta suites failed - %s ", CU_get_error_msg());
		return -1;
	}
	if (CU_register_suites(network_config_suites) != CUE_SUCCESS) {
		fprintf(stderr, "Register config suites failed - %s ", CU_get_error_msg());
		return -1;
	}
#if 0
	if (CU_register_suites(network_eth_suites) != CUE_SUCCESS) {
		fprintf(stderr, "Register eth suites failed - %s ", CU_get_error_msg());
		return -1;
	}
#endif
	CU_basic_set_mode(CU_BRM_VERBOSE);
	CU_basic_run_tests();

	CU_cleanup_registry();

	return 0;
}
