#include <stdio.h>
#include <CUnit/Basic.h>

#include "common.h"
#include "wifi_interface.h"

static __attribute__((unused)) void eth_plug_in_test(void)
{
	printf("\n");
	printf("******************************************************\n");
	printf("**** plug in cable (60s timeout)                  ****\n");
	printf("******************************************************\n");

	message_buf_wait_status("{ \"name\": \"ether\","
				" \"type\": \"ETHER_STATUS\", \"content\": \"ETHER_CONNECT\"", 60);
}

static __attribute__((unused)) void eth_stop_test(void)
{
	wifi_ctl_msg_t eth_cmd;

	memset(&eth_cmd, 0, sizeof(wifi_ctl_msg_t));
	eth_cmd.cmd = STOP_ETHERNET;

	CU_ASSERT_EQUAL_FATAL(request_wifi_mode(eth_cmd), true);

	message_buf_wait_status("{ \"name\": \"ether\", \"type\": \"ETHER_STATUS\","
				" \"content\": \"ETHER_DISCONNECT\"", 60);
}

static __attribute__((unused)) void eth_startup_test(void)
{
	wifi_ctl_msg_t eth_cmd;

	memset(&eth_cmd, 0, sizeof(wifi_ctl_msg_t));
	eth_cmd.cmd = STARTUP_ETHERNET;

	CU_ASSERT_EQUAL_FATAL(request_wifi_mode(eth_cmd), true);

	message_buf_wait_status("{ \"name\": \"ether\","
				" \"type\": \"ETHER_STATUS\", \"content\": \"ETHER_CONNECT\"", 60);
}

static __attribute__((unused)) void eth_ip_change_test(void)
{
	system("ifconfig eth0 192.168.1.123 up");

	message_buf_wait_status("{ \"name\": \"ether\","
				" \"type\": \"ETH_IP_CHANGE\", \"content\": \"192.168.1.123\"", 30);
}

static __attribute__((unused)) void eth_plug_out_test(void)
{
	wifi_ctl_msg_t eth_cmd;

	memset(&eth_cmd, 0, sizeof(wifi_ctl_msg_t));
	eth_cmd.cmd = STARTUP_ETHERNET;

	printf("\n");
	printf("******************************************************\n");
	printf("**** plug out cable (60s timeout)                 ****\n");
	printf("******************************************************\n");

	message_buf_wait_status("{ \"name\": \"ether\", \"type\": \"ETHER_STATUS\","
				" \"content\": \"ETHER_DISCONNECT\"", 60);

	CU_ASSERT_EQUAL_FATAL(request_wifi_mode(eth_cmd), false);
}

static __attribute__((unused)) CU_TestInfo net_eth_cases[] = {
	{"eth plug in test", eth_plug_in_test},
	{"eth stop test", eth_stop_test},
	{"eth startup test", eth_startup_test},
	{"eth ip change test", eth_ip_change_test},
	{"eth plug out test", eth_plug_out_test},
	CU_TEST_INFO_NULL
};

CU_SuiteInfo network_eth_suites[] = {
	{"network eth case:", NULL, NULL, NULL, NULL, net_eth_cases},
	CU_SUITE_INFO_NULL
};
