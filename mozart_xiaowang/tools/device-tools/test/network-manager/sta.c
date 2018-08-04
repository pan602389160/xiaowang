#include <stdio.h>
#include <CUnit/Basic.h>

#include "common.h"
#include "wifi_interface.h"

static __attribute__((unused)) void scan_over_1_test(void)
{
	wifi_ctl_msg_t switch_sta;

	memset(&switch_sta, 0, sizeof(wifi_ctl_msg_t));
	switch_sta.cmd = SW_STA;
	switch_sta.param.switch_sta.sta_timeout = 60;

	CU_ASSERT_EQUAL_FATAL(request_wifi_mode(switch_sta), true);

	message_buf_lock_timeout(10);
	CU_ASSERT_STRING_EQUAL_FATAL(message_buf, "{ \"name\": \"wifi\", \"type\": \"STA_STATUS\","
				     " \"content\": \"STA_SCAN_OVER\" }");
	message_buf_unlock();
}

static __attribute__((unused)) void wrong_key_test(void)
{
	wifi_ctl_msg_t switch_sta;

	memset(&switch_sta, 0, sizeof(wifi_ctl_msg_t));
	switch_sta.cmd = SW_STA;
	switch_sta.param.switch_sta.sta_timeout = 60;
	strncpy(switch_sta.param.switch_sta.ssid, "iad", 64);
	strncpy(switch_sta.param.switch_sta.psk, "123", 128);

	CU_ASSERT_EQUAL_FATAL(request_wifi_mode(switch_sta), false);

	memset(&switch_sta, 0, sizeof(wifi_ctl_msg_t));
	switch_sta.cmd = SW_STA;
	switch_sta.param.switch_sta.sta_timeout = 60;
	strncpy(switch_sta.param.switch_sta.ssid, "iad", 64);
	strncpy(switch_sta.param.switch_sta.psk, "12345678", 128);

	CU_ASSERT_EQUAL_FATAL(request_wifi_mode(switch_sta), true);

	message_buf_wait_status("{ \"name\": \"wifi\", \"type\": \"STA_STATUS\","
				     " \"content\": \"STA_CONNECT_FAILED\","
				     " \"reason\": \"WRONG_KEY\" }", 60);
}

static __attribute__((unused)) void switch_sta_test(void)
{
	wifi_info_t info;
	wifi_ctl_msg_t switch_sta;

	memset(&switch_sta, 0, sizeof(wifi_ctl_msg_t));
	switch_sta.cmd = SW_STA;
	switch_sta.param.switch_sta.sta_timeout = 60;
	strncpy(switch_sta.param.switch_sta.ssid, "iad", 64);
	strncpy(switch_sta.param.switch_sta.psk, "JZ_iaddep123", 128);

	CU_ASSERT_EQUAL_FATAL(request_wifi_mode(switch_sta), true);

	info = get_wifi_mode();
	CU_ASSERT_EQUAL(info.wifi_mode, STA_WAIT);

	message_buf_wait_status("{ \"name\": \"wifi\","
				" \"type\": \"WIFI_MODE\", \"content\": \"STA\",", 60);

	info = get_wifi_mode();
	CU_ASSERT_EQUAL(info.wifi_mode, STA);
	CU_ASSERT_STRING_EQUAL_FATAL(info.ssid, "iad");
}

static __attribute__((unused)) void switch_next_sta_1_test(void)
{
	wifi_ctl_msg_t switch_sta;

	memset(&switch_sta, 0, sizeof(wifi_ctl_msg_t));
	switch_sta.cmd = SW_STA;
	switch_sta.param.switch_sta.sta_timeout = 60;
	strncpy(switch_sta.param.switch_sta.ssid, "Guest", 64);
	strncpy(switch_sta.param.switch_sta.psk, "123", 128);

	CU_ASSERT_EQUAL_FATAL(request_wifi_mode(switch_sta), false);

	memset(&switch_sta, 0, sizeof(wifi_ctl_msg_t));
	switch_sta.cmd = SW_NEXT_NET;
	switch_sta.param.switch_sta.sta_timeout = 60;

	CU_ASSERT_EQUAL_FATAL(request_wifi_mode(switch_sta), true);

	message_buf_wait_status("{ \"name\": \"wifi\","
				" \"type\": \"WIFI_MODE\", \"content\": \"STA\",", 60);
}

static __attribute__((unused)) void switch_next_sta_2_test(void)
{
	wifi_ctl_msg_t switch_sta;

	memset(&switch_sta, 0, sizeof(wifi_ctl_msg_t));
	switch_sta.cmd = SW_STA;
	switch_sta.param.switch_sta.sta_timeout = 60;
	strncpy(switch_sta.param.switch_sta.ssid, "Guest", 64);
	strncpy(switch_sta.param.switch_sta.psk, "12345678", 128);

	CU_ASSERT_EQUAL_FATAL(request_wifi_mode(switch_sta), true);

	message_buf_wait_status("{ \"name\": \"wifi\", \"type\": \"STA_STATUS\","
				     " \"content\": \"STA_CONNECT_FAILED\","
				     " \"reason\": \"WRONG_KEY\" }", 60);

	memset(&switch_sta, 0, sizeof(wifi_ctl_msg_t));
	switch_sta.cmd = SW_NEXT_NET;
	switch_sta.param.switch_sta.sta_timeout = 60;

	CU_ASSERT_EQUAL_FATAL(request_wifi_mode(switch_sta), true);

	message_buf_wait_status("{ \"name\": \"wifi\","
				" \"type\": \"WIFI_MODE\", \"content\": \"STA\",", 60);
}

static __attribute__((unused)) void scan_over_2_test(void)
{
	wifi_ctl_msg_t switch_sta;

	memset(&switch_sta, 0, sizeof(wifi_ctl_msg_t));
	switch_sta.cmd = SW_STA;
	switch_sta.param.switch_sta.sta_timeout = 60;
	strncpy(switch_sta.param.switch_sta.ssid, "iad", 64);
	strncpy(switch_sta.param.switch_sta.psk, "12345678", 128);

	CU_ASSERT_EQUAL_FATAL(request_wifi_mode(switch_sta), true);

	message_buf_wait_status("{ \"name\": \"wifi\", \"type\": \"STA_STATUS\","
				     " \"content\": \"STA_CONNECT_FAILED\","
				     " \"reason\": \"WRONG_KEY\" }", 60);

	memset(&switch_sta, 0, sizeof(wifi_ctl_msg_t));
	switch_sta.cmd = SW_NEXT_NET;
	switch_sta.param.switch_sta.sta_timeout = 60;

	CU_ASSERT_EQUAL_FATAL(request_wifi_mode(switch_sta), true);

	message_buf_wait_status("{ \"name\": \"wifi\", \"type\": \"STA_STATUS\","
				     " \"content\": \"STA_CONNECT_FAILED\","
				     " \"reason\": \"WRONG_KEY\" }", 60);

	memset(&switch_sta, 0, sizeof(wifi_ctl_msg_t));
	switch_sta.cmd = SW_NEXT_NET;
	switch_sta.param.switch_sta.sta_timeout = 60;

	CU_ASSERT_EQUAL_FATAL(request_wifi_mode(switch_sta), true);

	message_buf_lock_timeout(10);
	CU_ASSERT_STRING_EQUAL_FATAL(message_buf, "{ \"name\": \"wifi\", \"type\": \"STA_STATUS\","
				     " \"content\": \"STA_SCAN_OVER\" }");
	message_buf_unlock();
}

static __attribute__((unused)) void scan_over_invalid_ssid_test(void)
{
	wifi_ctl_msg_t switch_sta;

	stop_wifi_mode_and_clear();

	memset(&switch_sta, 0, sizeof(wifi_ctl_msg_t));
	switch_sta.cmd = SW_STA;
	switch_sta.param.switch_sta.sta_timeout = 60;
	strncpy(switch_sta.param.switch_sta.ssid, "invalid_ssid0", 64);
	strncpy(switch_sta.param.switch_sta.psk, "12345678", 128);

	CU_ASSERT_EQUAL_FATAL(request_wifi_mode(switch_sta), false);

	memset(&switch_sta, 0, sizeof(wifi_ctl_msg_t));
	switch_sta.cmd = SW_STA;
	switch_sta.param.switch_sta.sta_timeout = 60;
	strncpy(switch_sta.param.switch_sta.ssid, "invalid_ssid1", 64);
	strncpy(switch_sta.param.switch_sta.psk, "12345678", 128);

	CU_ASSERT_EQUAL_FATAL(request_wifi_mode(switch_sta), false);

	memset(&switch_sta, 0, sizeof(wifi_ctl_msg_t));
	switch_sta.cmd = SW_NEXT_NET;
	switch_sta.param.switch_sta.sta_timeout = 60;

	CU_ASSERT_EQUAL_FATAL(request_wifi_mode(switch_sta), true);

	message_buf_wait_status("{ \"name\": \"wifi\", \"type\": \"STA_STATUS\","
				" \"content\": \"STA_SCAN_OVER\" }", 30);
}

static __attribute__((unused)) void timeout_wrong_key_test(void)
{
	wifi_ctl_msg_t switch_sta;

	memset(&switch_sta, 0, sizeof(wifi_ctl_msg_t));
	switch_sta.cmd = SW_STA;
	switch_sta.param.switch_sta.sta_timeout = 30;
	strncpy(switch_sta.param.switch_sta.ssid, "iad", 64);
	strncpy(switch_sta.param.switch_sta.psk, "12345678", 128);

	CU_ASSERT_EQUAL_FATAL(request_wifi_mode(switch_sta), true);

	message_buf_wait_status("{ \"name\": \"wifi\", \"type\": \"STA_STATUS\","
				     " \"content\": \"STA_CONNECT_FAILED\","
				     " \"reason\": \"WRONG_KEY\" }", 30);

	message_buf_wait_status("{ \"name\": \"wifi\", \"type\": \"STA_STATUS\","
				" \"content\": \"STA_CONNECT_FAILED\","
				" \"reason\": \"CONNECT_TIMEOUT\" }", 40);
}

static __attribute__((unused)) void disconnect_test(void)
{
	wifi_ctl_msg_t switch_sta;

	memset(&switch_sta, 0, sizeof(wifi_ctl_msg_t));
	switch_sta.cmd = SW_STA;
	switch_sta.param.switch_sta.sta_timeout = 180;
	strncpy(switch_sta.param.switch_sta.ssid, "Tenda_5C2750", 64);

	CU_ASSERT_EQUAL_FATAL(request_wifi_mode(switch_sta), true);

	message_buf_wait_status("{ \"name\": \"wifi\","
				" \"type\": \"WIFI_MODE\", \"content\": \"STA\",", 180);

	printf("******************************************************\n");
	printf("**** Please turn off Tenda_5C2750 (180s timeout)  ****\n");
	printf("******************************************************\n");

	message_buf_lock_timeout(180);
	CU_ASSERT_STRING_EQUAL_FATAL(message_buf, "{ \"name\": \"wifi\", \"type\": \"STA_STATUS\","
				     " \"content\": \"STA_CONNECT_FAILED\","
				     " \"reason\": \"DISCONNECTED\" }");
	message_buf_unlock();

	message_buf_lock_timeout(180);
	CU_ASSERT_STRING_EQUAL_FATAL(message_buf, "{ \"name\": \"wifi\", \"type\": \"STA_STATUS\","
				     " \"content\": \"STA_CONNECT_FAILED\","
				     " \"reason\": \"SCAN_SSID_FAILED\" }");
	message_buf_unlock();

	memset(&switch_sta, 0, sizeof(wifi_ctl_msg_t));
	switch_sta.cmd = SW_STA;
	switch_sta.param.switch_sta.sta_timeout = 60;
	strncpy(switch_sta.param.switch_sta.ssid, "iad", 64);
	strncpy(switch_sta.param.switch_sta.psk, "JZ_iaddep123", 128);

	CU_ASSERT_EQUAL_FATAL(request_wifi_mode(switch_sta), true);

	message_buf_wait_status("{ \"name\": \"wifi\","
				" \"type\": \"WIFI_MODE\", \"content\": \"STA\",", 60);

	memset(&switch_sta, 0, sizeof(wifi_ctl_msg_t));
	switch_sta.cmd = SW_NEXT_NET;
	switch_sta.param.switch_sta.sta_timeout = 60;

	CU_ASSERT_EQUAL_FATAL(request_wifi_mode(switch_sta), true);

	message_buf_lock_timeout(180);
	CU_ASSERT_STRING_EQUAL_FATAL(message_buf, "{ \"name\": \"wifi\", \"type\": \"STA_STATUS\","
				     " \"content\": \"STA_CONNECT_FAILED\","
				     " \"reason\": \"SCAN_SSID_FAILED\" }");
	message_buf_unlock();
}

static __attribute__((unused)) void auto_reconnect_test(void)
{

	wifi_ctl_msg_t switch_sta;

	stop_wifi_mode();
	message_buf_clear();

	printf("\n");
	printf("******************************************************\n");
	printf("***Please turn on Tenda_5C2750 and press enter key!***\n");
	getchar();
	printf("******************************************************\n");
	sleep(10);

	memset(&switch_sta, 0, sizeof(wifi_ctl_msg_t));
	switch_sta.cmd = SW_STA;
	switch_sta.param.switch_sta.sta_timeout = 180;
	strncpy(switch_sta.param.switch_sta.ssid, "Tenda_5C2750", 64);

	CU_ASSERT_TRUE_FATAL(request_wifi_mode(switch_sta));

	message_buf_wait_status("{ \"name\": \"wifi\","
				" \"type\": \"WIFI_MODE\", \"content\": \"STA\",", 180);

	printf("******************************************************\n");
	printf("**** Please turn off Tenda_5C2750 (180s timeout)  ****\n");
	printf("******************************************************\n");

	message_buf_lock_timeout(180);
	CU_ASSERT_STRING_EQUAL_FATAL(message_buf, "{ \"name\": \"wifi\", \"type\": \"STA_STATUS\","
				     " \"content\": \"STA_CONNECT_FAILED\","
				     " \"reason\": \"DISCONNECTED\" }");
	message_buf_unlock();

	message_buf_lock_timeout(180);
	CU_ASSERT_STRING_EQUAL_FATAL(message_buf, "{ \"name\": \"wifi\", \"type\": \"STA_STATUS\","
				     " \"content\": \"STA_CONNECT_FAILED\","
				     " \"reason\": \"SCAN_SSID_FAILED\" }");
	message_buf_unlock();

	printf("******************************************************\n");
	printf("**** Please turn on Tenda_5C2750 (180s timeout)   ****\n");
	printf("******************************************************\n");

	message_buf_wait_status("{ \"name\": \"wifi\","
				" \"type\": \"WIFI_MODE\", \"content\": \"STA\",", 180);
}

static __attribute__((unused)) void create_wifi_info(char *ssid, char *psk, int priority)
{
	char cmd[128];

	system("echo \"network={\" >> /usr/data/wpa_supplicant.conf");
	system("echo \"scan_ssid=1\" >> /usr/data/wpa_supplicant.conf");
	sprintf(cmd, "echo \"ssid=\\\"%s\\\"\" >> /usr/data/wpa_supplicant.conf", ssid);
	system(cmd);
	snprintf(cmd, 128, "echo \"psk=\"%s\"\" >> /usr/data/wpa_supplicant.conf", psk);
	system(cmd);
	system("echo \"bssid=\" >> /usr/data/wpa_supplicant.conf");
	snprintf(cmd, 128, "echo \"priority=%d\" >> /usr/data/wpa_supplicant.conf", priority);
	system(cmd);
	system("echo \"}\" >> /usr/data/wpa_supplicant.conf");
}

static __attribute__((unused)) void update_wifi_priority(void)
{
	wifi_ctl_msg_t switch_sta;

	stop_wifi_mode_and_clear();

	create_wifi_info("ssid3", "12345678", 3);
	create_wifi_info("Guest", "12347678", 100);
	create_wifi_info("ssid2", "12345678", 2);
	create_wifi_info("ssid6", "12345678", 25);
	create_wifi_info("iad", "JZ_iaddep123", 50);
	create_wifi_info("ssid5", "12345678", 10);
	create_wifi_info("ssid4", "12345678", 7);
	create_wifi_info("ssid1", "12345678", 1);

	memset(&switch_sta, 0, sizeof(wifi_ctl_msg_t));
	switch_sta.cmd = SW_STA;
	switch_sta.param.switch_sta.sta_timeout = 60;

	CU_ASSERT_EQUAL_FATAL(request_wifi_mode(switch_sta), true);

	message_buf_wait_status("{ \"name\": \"wifi\", \"type\": \"STA_STATUS\","
				     " \"content\": \"STA_CONNECT_FAILED\","
				     " \"reason\": \"WRONG_KEY\" }", 60);

	memset(&switch_sta, 0, sizeof(wifi_ctl_msg_t));
	switch_sta.cmd = SW_NEXT_NET;
	switch_sta.param.switch_sta.sta_timeout = 60;

	CU_ASSERT_EQUAL_FATAL(request_wifi_mode(switch_sta), true);

	message_buf_wait_status("{ \"name\": \"wifi\","
				" \"type\": \"WIFI_MODE\", \"content\": \"STA\",", 60);

	stop_wifi_mode_and_clear();
}

static __attribute__((unused)) void wifi_info_max_count_test(void)
{
	wifi_ctl_msg_t switch_sta;

	stop_wifi_mode_and_clear();

	create_wifi_info("ssid1", "12345678", 1);
	create_wifi_info("ssid2", "12345678", 2);
	create_wifi_info("ssid3", "12345678", 3);
	create_wifi_info("ssid4", "12345678", 4);
	create_wifi_info("ssid5", "12345678", 5);
	create_wifi_info("ssid6", "12345678", 6);
	create_wifi_info("ssid7", "12345678", 7);
	create_wifi_info("ssid8", "12345678", 8);
	create_wifi_info("ssid9", "12345678", 9);
	create_wifi_info("ssid0", "12345678", 10);

	memset(&switch_sta, 0, sizeof(wifi_ctl_msg_t));
	switch_sta.cmd = SW_STA;
	switch_sta.param.switch_sta.sta_timeout = 60;
	strncpy(switch_sta.param.switch_sta.ssid, "iad", 64);
	strncpy(switch_sta.param.switch_sta.psk, "12345678", 128);

	CU_ASSERT_EQUAL_FATAL(request_wifi_mode(switch_sta), true);

	message_buf_wait_status("{ \"name\": \"wifi\", \"type\": \"STA_STATUS\","
				     " \"content\": \"STA_CONNECT_FAILED\","
				     " \"reason\": \"WRONG_KEY\" }", 60);

	CU_ASSERT_NOT_EQUAL_FATAL(system("grep \"ssid1\" /usr/data/wpa_supplicant.conf"), 0);

	stop_wifi_mode_and_clear();
}

static __attribute__((unused)) void ip_change_test(void)
{
	wifi_ctl_msg_t switch_sta;

	memset(&switch_sta, 0, sizeof(wifi_ctl_msg_t));
	switch_sta.cmd = SW_STA;
	switch_sta.param.switch_sta.sta_timeout = 60;
	strncpy(switch_sta.param.switch_sta.ssid, "iad", 64);
	strncpy(switch_sta.param.switch_sta.psk, "JZ_iaddep123", 128);

	CU_ASSERT_EQUAL_FATAL(request_wifi_mode(switch_sta), true);

	message_buf_wait_status("{ \"name\": \"wifi\","
				" \"type\": \"WIFI_MODE\", \"content\": \"STA\",", 60);

	printf("\n");
	printf("******************************************************\n");
	printf("**** Manual set ip 10.4.50.123.                   ****\n");
	printf("******************************************************\n");
	system("ifconfig wlan0 10.4.50.123 up");

	message_buf_wait_status("{ \"name\": \"wifi\","
				" \"type\": \"STA_IP_CHANGE\", \"content\": \"10.4.50.123\"", 30);
}

static __attribute__((unused)) CU_TestInfo net_sta_cases[] = {
	{"wrong key test", wrong_key_test},
	{"switch sta test", switch_sta_test},
	{"switch next sta 1 test", switch_next_sta_1_test},
	{"switch next sta 2 test", switch_next_sta_2_test},
	{"scan over 2 test", scan_over_2_test},
	{"scan over invalid ssid test", scan_over_invalid_ssid_test},
	{"timeout wrong key test", timeout_wrong_key_test},
	{"disconnect test", disconnect_test},
	{"auto-reconnect test", auto_reconnect_test},
	{"update wifi priority test", update_wifi_priority},
	{"wifi info max count test", wifi_info_max_count_test},
	{"sta ip change test", ip_change_test},
	CU_TEST_INFO_NULL
};

CU_SuiteInfo network_sta_suites[] = {
	{"network sta case:", stop_wifi_mode_and_clear, NULL, NULL, NULL, net_sta_cases},
	CU_SUITE_INFO_NULL
};
