#include <stdio.h>
#include <string.h>
#include <CUnit/Basic.h>

#include "common.h"
#include "wifi_interface.h"

static __attribute__((unused)) void switch_invalid_ap_mode_test(void)
{

	wifi_ctl_msg_t switch_ap;

	memset(&switch_ap, 0, sizeof(wifi_ctl_msg_t));
	switch_ap.cmd = SW_AP;
	strncpy(switch_ap.param.switch_ap.ssid, "hostapd", 64);
	strncpy(switch_ap.param.switch_ap.psk, "123", 128);

	CU_ASSERT_FALSE_FATAL(request_wifi_mode(switch_ap));
}

static __attribute__((unused)) void switch_default_ap_mode_test(void)
{
	wifi_ctl_msg_t switch_ap;

	memset(&switch_ap, 0, sizeof(wifi_ctl_msg_t));
	switch_ap.cmd = SW_AP;
	CU_ASSERT_TRUE_FATAL(request_wifi_mode(switch_ap));

	message_buf_lock_timeout(30);
	CU_ASSERT_STRING_EQUAL_FATAL(message_buf, "{ \"name\": \"wifi\", \"type\": \"WIFI_MODE\","
				     " \"content\": \"AP\", \"last\": \"WIFI_NULL\" }");
	message_buf_unlock();

}

static __attribute__((unused)) void switch_ap_mode_test(void)
{
	wifi_info_t info;
	wifi_ctl_msg_t switch_ap;

	memset(&switch_ap, 0, sizeof(wifi_ctl_msg_t));
	switch_ap.cmd = SW_AP;
	strncpy(switch_ap.param.switch_ap.ssid, "", 64);
	strncpy(switch_ap.param.switch_ap.psk, "", 128);
	CU_ASSERT_TRUE_FATAL(request_wifi_mode(switch_ap));

	message_buf_lock_timeout(30);
	CU_ASSERT_STRING_EQUAL_FATAL(message_buf, "{ \"name\": \"wifi\", \"type\": \"WIFI_MODE\","
				     " \"content\": \"AP\", \"last\": \"AP\" }");
	message_buf_unlock();

	memset(&switch_ap, 0, sizeof(wifi_ctl_msg_t));
	switch_ap.cmd = SW_AP;
	strncpy(switch_ap.param.switch_ap.ssid, "hostapd", 64);
	strncpy(switch_ap.param.switch_ap.psk, "12345678", 128);
	CU_ASSERT_TRUE_FATAL(request_wifi_mode(switch_ap));

	message_buf_lock_timeout(30);
	CU_ASSERT_STRING_EQUAL_FATAL(message_buf, "{ \"name\": \"wifi\", \"type\": \"WIFI_MODE\","
				     " \"content\": \"AP\", \"last\": \"AP\" }");
	message_buf_unlock();

	info = get_wifi_mode();
	CU_ASSERT_EQUAL(info.wifi_mode, AP);
	CU_ASSERT_STRING_EQUAL_FATAL(info.ssid, "hostapd");

	printf("\n");
	printf("******************************************************\n");
	printf("connect ssid: 'hostapd' psk: '12345678'\n");
	printf("******************************************************\n");
	message_buf_lock_timeout(180);
	message_buf_unlock();

	printf("******************************************************\n");
	printf("disconnect ssid: 'hostapd'\n");
	printf("******************************************************\n");
	message_buf_lock_timeout(180);
	message_buf_unlock();

	stop_wifi_mode();
	info = get_wifi_mode();
	CU_ASSERT_EQUAL(info.wifi_mode, WIFI_NULL);

	strncpy(switch_ap.param.switch_ap.ssid, "hostapd", 64);
	strncpy(switch_ap.param.switch_ap.psk, "", 128);
	CU_ASSERT_TRUE_FATAL(request_wifi_mode(switch_ap));

	message_buf_lock_timeout(30);
	CU_ASSERT_STRING_EQUAL_FATAL(message_buf, "{ \"name\": \"wifi\", \"type\": \"WIFI_MODE\","
				     " \"content\": \"AP\", \"last\": \"WIFI_NULL\" }");
	message_buf_unlock();

	printf("\n");
	printf("******************************************************\n");
	printf("connect ssid: 'hostapd' psk: (null)\n");
	printf("******************************************************\n");
	message_buf_lock_timeout(180);
	message_buf_unlock();
#if 0
	printf("******************************************************\n");
	printf("disconnect ssid: 'hostapd'\n");
	printf("******************************************************\n");
	message_buf_lock_timeout(180);
	message_buf_unlock();
#endif
}

static __attribute__((unused)) void switch_chinese_ap_mode_test(void)
{
	wifi_info_t info;
	wifi_ctl_msg_t switch_ap;

	memset(&switch_ap, 0, sizeof(wifi_ctl_msg_t));
	switch_ap.cmd = SW_AP;
	strncpy(switch_ap.param.switch_ap.ssid, "!@#$%^&*()_+| \\\"\'[;你好", 64);
	CU_ASSERT_TRUE_FATAL(request_wifi_mode(switch_ap));

	message_buf_lock_timeout(30);
	CU_ASSERT_STRING_EQUAL_FATAL(message_buf, "{ \"name\": \"wifi\", \"type\": \"WIFI_MODE\","
				     " \"content\": \"AP\", \"last\": \"AP\" }");
	message_buf_unlock();

	info = get_wifi_mode();
	CU_ASSERT_EQUAL(info.wifi_mode, AP);
	CU_ASSERT_STRING_EQUAL_FATAL(info.ssid, "!@#$%^&*()_+| \\\"\'[;你好");
}

static __attribute__((unused)) CU_TestInfo net_ap_cases[] = {
	{"switch invalid ap mode", switch_invalid_ap_mode_test},
	{"switch default ap mode", switch_default_ap_mode_test},
	{"switch ap mode", switch_ap_mode_test},
	{"switch chinese ap mode", switch_chinese_ap_mode_test},
	CU_TEST_INFO_NULL
};

CU_SuiteInfo network_ap_suites[] = {
	{"network ap case:", stop_wifi_mode, NULL, NULL, NULL, net_ap_cases},
	CU_SUITE_INFO_NULL
};
