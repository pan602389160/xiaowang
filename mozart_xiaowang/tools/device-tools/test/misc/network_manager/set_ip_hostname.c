#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "wifi_interface.h"

int main(int argc,char *argv[])
{
	bool status;
	wifi_ctl_msg_t set_ip_hostname_test;
	struct wifi_client_register register_info;

	register_info.pid = getpid();
	register_info.reset = 1;
	register_info.priority = 3;
	strcpy(register_info.name,argv[0]);
	register_to_networkmanager(register_info, NULL);

	wifi_info_t infor = get_wifi_mode();
	printf("wifi mode is %s, SSID is %s\n", wifi_mode_str[infor.wifi_mode],infor.ssid);

	set_ip_hostname_test.force = true;
	set_ip_hostname_test.cmd = SET_IP_HOSTNAME;
	strcpy(set_ip_hostname_test.name,argv[0]);
	strcpy(set_ip_hostname_test.param.ip_hostname, "123456789bbox");
	status = request_wifi_mode(set_ip_hostname_test);

	memset(&infor, 0, sizeof(infor));
	infor = get_wifi_mode();
	printf("wifi mode is %s, SSID is %s\n", wifi_mode_str[infor.wifi_mode], infor.ssid);

	return 0;
}
