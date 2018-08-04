#include <stdio.h>
#include <CUnit/Basic.h>

#include "common.h"
#include "wifi_interface.h"

static __attribute__((unused)) void invalid_param_test(void)
{
	wifi_ctl_msg_t switch_netcfg;

	/* invalid vendor */
	memset(&switch_netcfg, 0, sizeof(wifi_ctl_msg_t));
	switch_netcfg.cmd = SW_NETCFG;
	switch_netcfg.param.network_config.timeout = -1;
	strcpy(switch_netcfg.param.network_config.wl_module, "invalid_vendor");
	switch_netcfg.param.network_config.method = AIRKISS_WE;

	CU_ASSERT_EQUAL_FATAL(request_wifi_mode(switch_netcfg), false);

	/* invalid method */
	memset(&switch_netcfg, 0, sizeof(wifi_ctl_msg_t));
	switch_netcfg.cmd = SW_NETCFG;
	switch_netcfg.param.network_config.timeout = -1;
	strcpy(switch_netcfg.param.network_config.wl_module, wifi_module_str[BROADCOM]);
	switch_netcfg.param.network_config.method = 0;

	CU_ASSERT_EQUAL_FATAL(request_wifi_mode(switch_netcfg), false);

	/* JOYLINK: invalid key */
	memset(&switch_netcfg, 0, sizeof(wifi_ctl_msg_t));
	switch_netcfg.cmd = SW_NETCFG;
	switch_netcfg.param.network_config.timeout = -1;
	strcpy(switch_netcfg.param.network_config.wl_module, wifi_module_str[BROADCOM]);
	switch_netcfg.param.network_config.method = JOYLINK;

	CU_ASSERT_EQUAL_FATAL(request_wifi_mode(switch_netcfg), false);

	/* ATALK: invalid product_model */
	memset(&switch_netcfg, 0, sizeof(wifi_ctl_msg_t));
	switch_netcfg.cmd = SW_NETCFG;
	switch_netcfg.param.network_config.timeout = -1;
	strcpy(switch_netcfg.param.network_config.wl_module, wifi_module_str[BROADCOM]);
	switch_netcfg.param.network_config.method = ATALK;

	CU_ASSERT_EQUAL_FATAL(request_wifi_mode(switch_netcfg), false);
}

static __attribute__((unused)) void broadcom_airkiss_timeout_test(void)
{
	wifi_ctl_msg_t switch_netcfg;

	memset(&switch_netcfg, 0, sizeof(wifi_ctl_msg_t));
	switch_netcfg.cmd = SW_NETCFG;
	switch_netcfg.param.network_config.timeout = 5;
	strcpy(switch_netcfg.param.network_config.wl_module, wifi_module_str[BROADCOM]);
	switch_netcfg.param.network_config.method = AIRKISS_WE;

	CU_ASSERT_EQUAL_FATAL(request_wifi_mode(switch_netcfg), true);

	message_buf_wait_status("{ \"name\": \"wifi\", \"type\": \"NETWORK_CONFIGURE\","
				" \"content\": \"AIRKISS_FAILED\","
				" \"reason\": \"CONFIGURE_TIMEOUT\"", 6);
}

static __attribute__((unused)) void broadcom_airkiss_test(void)
{
	wifi_ctl_msg_t switch_netcfg;

	memset(&switch_netcfg, 0, sizeof(wifi_ctl_msg_t));
	switch_netcfg.cmd = SW_NETCFG;
	switch_netcfg.param.network_config.timeout = -1;
	strcpy(switch_netcfg.param.network_config.wl_module, wifi_module_str[BROADCOM]);
	switch_netcfg.param.network_config.method = AIRKISS_WE;

	CU_ASSERT_EQUAL_FATAL(request_wifi_mode(switch_netcfg), true);

	message_buf_wait_status("{ \"name\": \"wifi\", \"type\": \"NETWORK_CONFIGURE\","
				" \"content\": \"AIRKISS_SUCCESS\"", 60);

	message_buf_wait_status("{ \"name\": \"wifi\","
				" \"type\": \"WIFI_MODE\", \"content\": \"STA\",", 60);
}

static __attribute__((unused)) void broadcom_airkiss_null_psk_test(void)
{
	wifi_info_t info;
	wifi_ctl_msg_t switch_netcfg;

	memset(&switch_netcfg, 0, sizeof(wifi_ctl_msg_t));
	switch_netcfg.cmd = SW_NETCFG;
	switch_netcfg.param.network_config.timeout = -1;
	strcpy(switch_netcfg.param.network_config.wl_module, wifi_module_str[BROADCOM]);
	switch_netcfg.param.network_config.method = AIRKISS_WE;

	CU_ASSERT_EQUAL_FATAL(request_wifi_mode(switch_netcfg), true);

	info = get_wifi_mode();
	CU_ASSERT_EQUAL(info.wifi_mode, AIRKISS);

	message_buf_wait_status("{ \"name\": \"wifi\", \"type\": \"NETWORK_CONFIGURE\","
				" \"content\": \"AIRKISS_SUCCESS\"", 60);

	message_buf_wait_status("{ \"name\": \"wifi\", \"type\": \"NETWORK_CONFIGURE\","
				" \"content\": \"AIRKISS_FAILED\","
				" \"reason\": \"CONFIGURE_INVALID_PWD_LEN\"", 60);
}

static __attribute__((unused)) void broadcom_joylink_test(void)
{
	wifi_ctl_msg_t switch_netcfg;

	memset(&switch_netcfg, 0, sizeof(wifi_ctl_msg_t));
	switch_netcfg.cmd = SW_NETCFG;
	switch_netcfg.param.network_config.timeout = -1;
	strcpy(switch_netcfg.param.network_config.wl_module, wifi_module_str[BROADCOM]);
	switch_netcfg.param.network_config.method = JOYLINK;
	strcpy(switch_netcfg.param.network_config.key, "123456789");

	CU_ASSERT_EQUAL_FATAL(request_wifi_mode(switch_netcfg), true);

	message_buf_wait_status("{ \"name\": \"wifi\", \"type\": \"NETWORK_CONFIGURE\","
				" \"content\": \"AIRKISS_SUCCESS\"", 60);

	message_buf_wait_status("{ \"name\": \"wifi\","
				" \"type\": \"WIFI_MODE\", \"content\": \"STA\",", 60);
}

static __attribute__((unused)) void broadcom_joylink_invalid_len_psk_test(void)
{
	wifi_ctl_msg_t switch_netcfg;

	memset(&switch_netcfg, 0, sizeof(wifi_ctl_msg_t));
	switch_netcfg.cmd = SW_NETCFG;
	switch_netcfg.param.network_config.timeout = -1;
	strcpy(switch_netcfg.param.network_config.wl_module, wifi_module_str[BROADCOM]);
	switch_netcfg.param.network_config.method = JOYLINK;
	strcpy(switch_netcfg.param.network_config.key, "123456789");

	CU_ASSERT_EQUAL_FATAL(request_wifi_mode(switch_netcfg), true);

	message_buf_wait_status("{ \"name\": \"wifi\", \"type\": \"NETWORK_CONFIGURE\","
				" \"content\": \"AIRKISS_SUCCESS\"", 60);

	message_buf_wait_status("{ \"name\": \"wifi\", \"type\": \"NETWORK_CONFIGURE\","
				" \"content\": \"AIRKISS_FAILED\","
				" \"reason\": \"CONFIGURE_INVALID_PWD_LEN\"", 60);
}

static __attribute__((unused)) void broadcom_cooee_test(void)
{
	wifi_ctl_msg_t switch_netcfg;

	memset(&switch_netcfg, 0, sizeof(wifi_ctl_msg_t));
	switch_netcfg.cmd = SW_NETCFG;
	switch_netcfg.param.network_config.timeout = -1;
	strcpy(switch_netcfg.param.network_config.wl_module, wifi_module_str[BROADCOM]);
	switch_netcfg.param.network_config.method = COOEE;

	CU_ASSERT_EQUAL_FATAL(request_wifi_mode(switch_netcfg), true);

	message_buf_wait_status("{ \"name\": \"wifi\", \"type\": \"NETWORK_CONFIGURE\","
				" \"content\": \"AIRKISS_SUCCESS\"", 60);

	message_buf_wait_status("{ \"name\": \"wifi\","
				" \"type\": \"WIFI_MODE\", \"content\": \"STA\",", 60);
}

static __attribute__((unused)) void broadcom_cooee_airkiss_test(void)
{
	wifi_ctl_msg_t switch_netcfg;

	memset(&switch_netcfg, 0, sizeof(wifi_ctl_msg_t));
	switch_netcfg.cmd = SW_NETCFG;
	switch_netcfg.param.network_config.timeout = -1;
	strcpy(switch_netcfg.param.network_config.wl_module, wifi_module_str[BROADCOM]);
	switch_netcfg.param.network_config.method = COOEE | AIRKISS_WE;

	printf("\n");
	printf("******************************************************\n");
	printf("use airkiss and press enter key\n");
	getchar();
	printf("******************************************************\n");

	CU_ASSERT_EQUAL_FATAL(request_wifi_mode(switch_netcfg), true);

	message_buf_wait_status("{ \"name\": \"wifi\", \"type\": \"NETWORK_CONFIGURE\","
				" \"content\": \"AIRKISS_SUCCESS\"", 60);

	message_buf_wait_status("{ \"name\": \"wifi\","
				" \"type\": \"WIFI_MODE\", \"content\": \"STA\",", 60);
}

static __attribute__((unused)) void broadcom_airkiss_atalk_test(void)
{
	wifi_ctl_msg_t switch_netcfg;

	memset(&switch_netcfg, 0, sizeof(wifi_ctl_msg_t));
	switch_netcfg.cmd = SW_NETCFG;
	switch_netcfg.param.network_config.timeout = -1;
	strcpy(switch_netcfg.param.network_config.wl_module, wifi_module_str[BROADCOM]);
	strcpy(switch_netcfg.param.network_config.product_model, "ALINKTEST_LIVING_LIGHT_SMARTLED");
	switch_netcfg.param.network_config.method = AIRKISS_WE | ATALK;

	printf("\n");
	printf("******************************************************\n");
	printf("Airkiss + ATALK: use airkiss\n");
	printf("******************************************************\n");

	CU_ASSERT_EQUAL_FATAL(request_wifi_mode(switch_netcfg), true);

	message_buf_wait_status("{ \"name\": \"wifi\", \"type\": \"NETWORK_CONFIGURE\","
				" \"content\": \"AIRKISS_SUCCESS\"", 60);

	message_buf_wait_status("{ \"name\": \"wifi\","
				" \"type\": \"WIFI_MODE\", \"content\": \"STA\",", 60);

	printf("******************************************************\n");
	printf("Airkiss + ATALK: use atalk and press enter key\n");
	getchar();
	printf("******************************************************\n");

	CU_ASSERT_EQUAL_FATAL(request_wifi_mode(switch_netcfg), true);

	message_buf_wait_status("{ \"name\": \"wifi\", \"type\": \"NETWORK_CONFIGURE\","
				" \"content\": \"AIRKISS_SUCCESS\"", 60);

	message_buf_wait_status("{ \"name\": \"wifi\","
				" \"type\": \"WIFI_MODE\", \"content\": \"STA\",", 60);
}

static __attribute__((unused)) void realtek_simple_config_test(void)
{
	wifi_ctl_msg_t switch_netcfg;

	printf("\n");
	printf("******************************************************\n");
	printf("Realtek: use simple config\n");
	printf("******************************************************\n");

	memset(&switch_netcfg, 0, sizeof(wifi_ctl_msg_t));
	switch_netcfg.cmd = SW_NETCFG;
	switch_netcfg.param.network_config.timeout = -1;
	strcpy(switch_netcfg.param.network_config.wl_module, wifi_module_str[REALTEK]);
	switch_netcfg.param.network_config.method = SIMPLE_CONFIG;

	CU_ASSERT_EQUAL_FATAL(request_wifi_mode(switch_netcfg), true);

	message_buf_wait_status("{ \"name\": \"wifi\", \"type\": \"NETWORK_CONFIGURE\","
				" \"content\": \"AIRKISS_SUCCESS\"", 60);

	message_buf_wait_status("{ \"name\": \"wifi\","
				" \"type\": \"WIFI_MODE\", \"content\": \"STA\",", 60);
}

static __attribute__((unused)) void realtek_airkiss_test(void)
{
	wifi_ctl_msg_t switch_netcfg;

	memset(&switch_netcfg, 0, sizeof(wifi_ctl_msg_t));
	switch_netcfg.cmd = SW_NETCFG;
	switch_netcfg.param.network_config.timeout = -1;
	strcpy(switch_netcfg.param.network_config.wl_module, wifi_module_str[REALTEK]);
	switch_netcfg.param.network_config.method = AIRKISS_WE;

	CU_ASSERT_EQUAL_FATAL(request_wifi_mode(switch_netcfg), true);

	message_buf_wait_status("{ \"name\": \"wifi\", \"type\": \"NETWORK_CONFIGURE\","
				" \"content\": \"AIRKISS_SUCCESS\"", 60);

	message_buf_wait_status("{ \"name\": \"wifi\","
				" \"type\": \"WIFI_MODE\", \"content\": \"STA\",", 60);
}

static __attribute__((unused)) void broadcom_cooee_with_chinese_ssid_test(void)
{
	wifi_ctl_msg_t switch_netcfg;

	printf("\n");
	printf("******************************************************\n");
	printf("please set router before and after chinese ssid with Space and press enter key!\n");
	printf("Note: Chinese must be utf8 encoding!\n");
	getchar();
	printf("******************************************************\n");

	memset(&switch_netcfg, 0, sizeof(wifi_ctl_msg_t));
	switch_netcfg.cmd = SW_NETCFG;
	switch_netcfg.param.network_config.timeout = -1;
	strcpy(switch_netcfg.param.network_config.wl_module, wifi_module_str[BROADCOM]);
	switch_netcfg.param.network_config.method = COOEE;

	CU_ASSERT_EQUAL_FATAL(request_wifi_mode(switch_netcfg), true);

	message_buf_wait_status("{ \"name\": \"wifi\", \"type\": \"NETWORK_CONFIGURE\","
				" \"content\": \"AIRKISS_SUCCESS\"", 60);

	message_buf_wait_status("{ \"name\": \"wifi\","
				" \"type\": \"WIFI_MODE\", \"content\": \"STA\",", 60);
}

static __attribute__((unused)) void broadcom_cooee_with_special_char_ssid_test(void)
{
	wifi_ctl_msg_t switch_netcfg;

	printf("\n");
	printf("******************************************************\n");
	printf("please set router ssid with special char and press enter key!\n");
	printf("Note: Special char must be utf8 encoding!\n");
	getchar();
	printf("******************************************************\n");

	memset(&switch_netcfg, 0, sizeof(wifi_ctl_msg_t));
	switch_netcfg.cmd = SW_NETCFG;
	switch_netcfg.param.network_config.timeout = -1;
	strcpy(switch_netcfg.param.network_config.wl_module, wifi_module_str[BROADCOM]);
	switch_netcfg.param.network_config.method = COOEE;

	CU_ASSERT_EQUAL_FATAL(request_wifi_mode(switch_netcfg), true);

	message_buf_wait_status("{ \"name\": \"wifi\", \"type\": \"NETWORK_CONFIGURE\","
				" \"content\": \"AIRKISS_SUCCESS\"", 60);

	message_buf_wait_status("{ \"name\": \"wifi\","
				" \"type\": \"WIFI_MODE\", \"content\": \"STA\",", 60);
}

static __attribute__((unused)) void broadcom_cooee_with_quoted_ssid_test(void)
{
	wifi_ctl_msg_t switch_netcfg;

	printf("\n");
	printf("******************************************************\n");
	printf("please set router ssid with single quotation marks and press enter key!\n");
	getchar();
	printf("******************************************************\n");

	memset(&switch_netcfg, 0, sizeof(wifi_ctl_msg_t));
	switch_netcfg.cmd = SW_NETCFG;
	switch_netcfg.param.network_config.timeout = -1;
	strcpy(switch_netcfg.param.network_config.wl_module, wifi_module_str[BROADCOM]);
	switch_netcfg.param.network_config.method = COOEE;

	CU_ASSERT_EQUAL_FATAL(request_wifi_mode(switch_netcfg), true);

	message_buf_wait_status("{ \"name\": \"wifi\", \"type\": \"NETWORK_CONFIGURE\","
				" \"content\": \"AIRKISS_SUCCESS\"", 60);

	message_buf_wait_status("{ \"name\": \"wifi\","
				" \"type\": \"WIFI_MODE\", \"content\": \"STA\",", 60);
}

static __attribute__((unused)) void broadcom_cooee_with_userinfo_test(void)
{
	wifi_ctl_msg_t switch_netcfg;

	printf("\n");
	printf("******************************************************\n");
	printf("please use dingdong app config network and press enter key!\n");
	getchar();
	printf("******************************************************\n");

	memset(&switch_netcfg, 0, sizeof(wifi_ctl_msg_t));
	switch_netcfg.cmd = SW_NETCFG;
	switch_netcfg.param.network_config.timeout = -1;
	strcpy(switch_netcfg.param.network_config.wl_module, wifi_module_str[BROADCOM]);
	switch_netcfg.param.network_config.method = COOEE;

	CU_ASSERT_EQUAL_FATAL(request_wifi_mode(switch_netcfg), true);

	message_buf_wait_status("{ \"name\": \"wifi\", \"type\": \"NETWORK_CONFIGURE\","
				" \"content\": \"AIRKISS_SUCCESS\"", 60);

	message_buf_wait_status("{ \"name\": \"wifi\","
				" \"type\": \"WIFI_MODE\", \"content\": \"STA\",", 60);
}

static __attribute__((unused)) CU_TestInfo net_cfg_cases[] = {
	{"invalid param test", invalid_param_test},
	{"broadcom airkiss timeout test", broadcom_airkiss_timeout_test},
	{"broadcom airkiss test", broadcom_airkiss_test},
	{"broadcom airkiss null psk test", broadcom_airkiss_null_psk_test},
	{"broadcom joylink test", broadcom_joylink_test},
	{"broadcom joylink invalid len psk test", broadcom_joylink_invalid_len_psk_test},
	{"broadcom cooee test", broadcom_cooee_test},
	{"broadcom cooee airkiss test", broadcom_cooee_airkiss_test},
	{"broadcom airkiss atalk test", broadcom_airkiss_atalk_test},
	{"realtek simple config test", realtek_simple_config_test},
	{"realtek airkiss test", realtek_airkiss_test},
	{"broadcom cooee with chinese ssid test", broadcom_cooee_with_chinese_ssid_test},
	{"broadcom cooee with special char ssid test", broadcom_cooee_with_special_char_ssid_test},
	{"broadcom cooee with single quotation marks test", broadcom_cooee_with_quoted_ssid_test},
	{"broadcom cooee with user info for psk test", broadcom_cooee_with_userinfo_test},
	CU_TEST_INFO_NULL
};

CU_SuiteInfo network_config_suites[] = {
	{"network config case:", stop_wifi_mode_and_clear, NULL,
	 NULL, (void (*)(void))stop_wifi_mode_and_clear, net_cfg_cases},
	CU_SUITE_INFO_NULL
};
