#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>

#include "alarm_interface.h"

static int alarm_callback(struct alarm_info *alarm)
{
	if(!strcmp(alarm->name, "alarm_test")) {
		printf("**************************************\n");
		printf("hour %d\n", alarm->hour);
		printf("minute %d\n", alarm->minute);
		printf("week_active %d\n", alarm->week_active);
		printf("weekly_repeat %d\n", alarm->weekly_repeat);
		printf("enable %d\n", alarm->enable);
		printf("alarm_id %d\n", alarm->alarm_id);
		printf("timestamp %ld\n", alarm->timestamp);
		printf("name %s\n", alarm->name);
		printf("len %d\n", alarm->prv_info.len);
		printf("info %s\n", alarm->prv_info.info);
		printf("*************************************\n");
	} else
		printf("receive alarm, but not my set!!\n");

	return 0;
}

/* get alarm list*/
void get_alarm_list(void)
{
	char *alarm_json = NULL;

	alarm_json = mozart_alarm_get_list();
	printf("\n%s\n", alarm_json);
	free(alarm_json);
}

int main(int argc, char *argv[])
{
	int len;
	char info[] = "{ \"volume\" : 50, \"url\" : \"/mnt/sdcard/1.mp3\" }";
	time_t timestamp;
	struct tm cur_tm;
	struct alarm_info *alarm = NULL;

	len = sizeof(info);
	alarm = calloc(sizeof(struct alarm_info) + len, 1);

	register_to_alarm_manager(alarm_callback);

	/* start alarm */
	mozart_alarm_start_alarm();

	timestamp = mozart_alarm_get_rtc_time();
	printf("rtc time : %ld\n", timestamp);

	/* add alarm */
	timestamp = time(NULL);
	localtime_r(&timestamp, &cur_tm);
	alarm->hour = cur_tm.tm_hour;
	alarm->minute = cur_tm.tm_min + 1;
	alarm->week_active = 0x7f;
	alarm->weekly_repeat = 0;
	alarm->enable = 1;
	strcpy(alarm->name, "alarm_test");
	alarm->prv_info.len = len;
	strncpy(alarm->prv_info.info, info, len);

	mozart_alarm_add(alarm);
	get_alarm_list();
	sleep(100);

	/* delete alarm */
	mozart_alarm_delete(alarm);
	get_alarm_list();
	sleep(1);

	/* update alarm */
	timestamp = time(NULL);
	localtime_r(&timestamp, &cur_tm);
	alarm->hour = cur_tm.tm_hour;
	alarm->minute = cur_tm.tm_min + 1;
	alarm->week_active = 0x1;
	alarm->weekly_repeat = 1;
	alarm->enable = 1;
	strcpy(alarm->name, "alarm_test");
	alarm->prv_info.len = len;
	strncpy(alarm->prv_info.info, info, len);

	mozart_alarm_update(alarm);
	get_alarm_list();

	free(alarm);
	sleep(300);

	/* stop alarm */
	mozart_alarm_stop_alarm();

	unregister_to_alarm_manager();

	return 0;
}
