#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>
#include <errno.h>

#include "json-c/json.h"
#include "tips_interface.h"
#include "nvrw_interface.h"
#include "utils_interface.h"
#include "appserver.h"
#include "mozart_key_function.h"

#define DEVICE_TIMER_FILENAME "/tmp/timer"
#define DEVICE_NAME_FILENAME "/usr/data/ingenicplayer/devicename"

enum {
	SET_TIMER = 0,
	GET_TIMER,
	RESUME_DEFAULT_SETTING,
	GET_DETAIL_DEVICE_INFO,
	SET_DEVICE_NAME,
	GET_DEVICE_NAME,
};

static char *device_cmd[] = {
	[SET_TIMER] = "set_timer",
	[GET_TIMER] = "get_timer",
	[RESUME_DEFAULT_SETTING] = "resume_default_setting",
	[GET_DETAIL_DEVICE_INFO] = "get_detail_device_info",
	[SET_DEVICE_NAME] = "set_device_name",
	[GET_DEVICE_NAME] = "get_device_name",
};

static void signal_power_off(int sig)
{
	system("poweroff");
}

static void signal_cancel_timer(int sig)
{
	alarm(0);
	unlink(DEVICE_TIMER_FILENAME);
	exit(0);
}

static char *get_device_info(char *path)
{
	int length = 0;
	char *timer_json = NULL;
	FILE *fp = NULL;

	if (access(path, F_OK) != 0)
		return NULL;

	if ((fp = fopen(path, "r")) == NULL) {
		printf("fopen %s : %s\n", path, strerror(errno));
		return NULL;
	}

	fseek(fp, 0, SEEK_END);
	if ((length = ftell(fp)) == -1) {
		printf("ftell %s\n", strerror(errno));
		fclose(fp);
		return NULL;
	}
	fseek(fp, 0, SEEK_SET);

	timer_json = (char *)calloc(length, sizeof(char) + 1);
	if (timer_json != NULL)
		fread(timer_json, sizeof(char), length, fp);

	fclose(fp);

	return timer_json;
}

static int get_device_timer_element(char *element)
{
	int ret = -1;
	char *timer_json = NULL;
	json_object *object = NULL;
	json_object *tmp = NULL;

	timer_json = get_device_info(DEVICE_TIMER_FILENAME);
	if (timer_json != NULL) {
		if ((object = json_tokener_parse(timer_json)) == NULL) {
			free(timer_json);
			return -1;
		}

		if (json_object_object_get_ex(object, element, &tmp))
			ret = json_object_get_int(tmp);

		json_object_put(object);
		free(timer_json);
	}

	return ret;
}

static int cancel_timer(void)
{
	int pid = 0;

	pid = get_device_timer_element("pid");
	if (pid > 0) {
		if (kill(pid, SIGUSR1) != 0)
			printf("kill %d : %s\n", pid, strerror(errno));
	}

	return 0;
}

static int set_timer(char *command, char *data, struct appserver_cli_info *owner)
{
	int fpid = -1;
	time_t timer = -1;
	FILE *fp = NULL;
	json_object *object = NULL;
	json_object *tmp = NULL;

	cancel_timer();

	if ((object = json_tokener_parse(data)) == NULL)
		return -1;

	if (json_object_object_get_ex(object, "timer", &tmp))
		timer = (time_t)json_object_get_int(tmp);

	json_object_put(object);

	if (timer > 0) {
		fpid = fork();
		if (fpid < 0) {
			printf("fork %s\n", strerror(errno));
		} else if (fpid == 0) {
			signal(SIGALRM, signal_power_off);
			signal(SIGUSR1, signal_cancel_timer);

			alarm(timer);
			timer += time(NULL);

			if ((object = json_object_new_object()) == NULL)
				return -1;

			json_object_object_add(object, "pid", json_object_new_int(getpid()));
			json_object_object_add(object, "timestamp", json_object_new_int(timer));

			if ((fp = fopen(DEVICE_TIMER_FILENAME, "w+")) == NULL) {
				json_object_put(object);
				return -1;
			}

			fwrite(json_object_get_string(object), sizeof(char),
					strlen(json_object_get_string(object)), fp);

			fclose(fp);

			json_object_put(object);

			while (1)
				sleep(1);
		} else {
			mozart_appserver_response(command, data, owner);
		}
	}
	return 0;
}

static int get_timer(char *command, char *data, struct appserver_cli_info *owner)
{
	time_t timer = 0;
	json_object *object = NULL;

	timer = get_device_timer_element("timestamp");
	if (timer > 0) {
		timer -= time(NULL);

		if ((object = json_object_new_object()) != NULL) {
			json_object_object_add(object, "timer",
					json_object_new_int(timer));
			mozart_appserver_response(command,
					(char *)json_object_get_string(object), owner);
			json_object_put(object);
		}
	}

	return 0;
}

static int resume_default_setting(char *command, char *data, struct appserver_cli_info *owner)
{
	system("rm -rf /usr/data/*");
	system("cp -r /usr/share/data/* /usr/data");

	mozart_appserver_response(command, data, owner);

	mozart_module_stop();
	mozart_play_key_sync("factory_reset_success");
	system("reboot");

	return 0;
}

static int get_detail_device_info(char *command, char *data, struct appserver_cli_info *owner)
{
	int ret = 0;
	int nvrw_lock = -1;
	struct nv_info *info = NULL;
	char *name = NULL;
	char ch_tmp[18] = "0";
	char sd_size[10] = "0";
	char sd_avail[10] = "0";
	char macaddr[] = "255.255.255.255";
	FILE *fp = NULL;
	json_object *object = NULL;
	json_object *tmp = NULL;

	if ((object = json_object_new_object()) == NULL)
		return -1;

	/* device name*/
	if ((name = get_device_info(DEVICE_NAME_FILENAME)) != NULL)
		json_object_object_add(object, "device_name", json_object_new_string(name));

	/* device id */
	if ((tmp = json_object_new_string("123456")) == NULL)
		goto device_info_err;

	json_object_object_add(object, "device_id", tmp);

	/* device electricify */
	if ((tmp = json_object_new_int(100)) == NULL)
		goto device_info_err;

	json_object_object_add(object, "device_electricify", tmp);

	/* device version */
	nvrw_lock = mozart_nv_lock();
	if ((info = mozart_nv_get_nvinfo()) != NULL) {
		strcpy(ch_tmp, info->current_version);
		free(info);
	}
	mozart_nv_unlock(nvrw_lock);

	if ((tmp = json_object_new_string(ch_tmp)) == NULL)
		goto device_info_err;

	json_object_object_add(object, "device_version", tmp);

	/* sd storage */
	memset(ch_tmp, 0, sizeof(ch_tmp));
	fp = popen("df -h | awk -F: '/mnt/sdcard' | awk '{print $2}'", "r");
	if (fp != NULL) {
		ret = fread(ch_tmp, sizeof(char), sizeof(ch_tmp), fp);
		if (ret > 0)
			strncpy(sd_size, ch_tmp, ret - 1);
		fclose(fp);
	}

	if ((tmp = json_object_new_string(sd_size)) == NULL)
		goto device_info_err;

	json_object_object_add(object, "sd_storage", tmp);

	/* sd available */
	memset(ch_tmp, 0, sizeof(ch_tmp));
	fp = popen("df -h | awk -F: '/mnt/sdcard' | awk '{print $4}'", "r");
	if (fp != NULL) {
		ret = fread(ch_tmp, sizeof(char), sizeof(ch_tmp), fp);
		if (ret > 0)
			strncpy(sd_avail, ch_tmp, ret - 1);
		fclose(fp);
	}

	if ((tmp = json_object_new_string(sd_avail)) == NULL)
		goto device_info_err;

	json_object_object_add(object, "sd_available", tmp);

	/* mac address */
	memset(macaddr, 0, sizeof(macaddr));
	get_mac_addr("wlan0", macaddr, "");

	if ((tmp = json_object_new_string(macaddr)) == NULL)
		goto device_info_err;

	json_object_object_add(object, "mac_address", tmp);

	mozart_appserver_response(command, (char *)json_object_get_string(object), owner);

device_info_err:

	json_object_put(object);

	if (name)
		free(name);

	return 0;
}

static int set_device_name(char *command, char *data, struct appserver_cli_info *owner)
{
	int length = 0;
	FILE *fp = NULL;
	json_object *object = NULL;
	json_object *tmp = NULL;

	if ((fp = fopen(DEVICE_NAME_FILENAME, "w+")) == NULL) {
		printf("fopen %s\n", strerror(errno));
		return -1;
	}

	if ((object = json_tokener_parse(data)) != NULL) {
		if (json_object_object_get_ex(object, "name", &tmp)) {
			length = strlen(json_object_get_string(tmp));
			fwrite(json_object_get_string(tmp),
					length, sizeof(char), fp);
		}
		json_object_put(object);
	}
	fclose(fp);

	mozart_appserver_notify(command, data);

	return 0;
}

static int get_device_name(char *command, char *data, struct appserver_cli_info *owner)
{
	int tmp = 0;
	int length = 0;
	char *name = NULL;
	FILE *fp = NULL;
	json_object *object = NULL;

	if ((fp = fopen(DEVICE_NAME_FILENAME, "r")) == NULL) {
		printf("fopen %s\n", strerror(errno));
		return -1;
	}

	fseek(fp, 0, SEEK_END);
	if ((length = ftell(fp)) == -1) {
		printf("ftell %s\n", strerror(errno));
		fclose(fp);
		return -1;
	}
	fseek(fp, 0, SEEK_SET);

	if ((name = calloc(length + 1, sizeof(char))) != NULL) {
		tmp = fread(name, sizeof(char), length, fp);
		if (tmp == length) {
			object = json_object_new_object();
			if (object) {
				json_object_object_add(object, "name",
						json_object_new_string(name));
				mozart_appserver_notify(command,
						(char *)json_object_get_string(object));
				json_object_put(object);
			}
		}
		free(name);
	}
	fclose(fp);

	return 0;
}

int mozart_device_response_cmd(char *command, char *data, struct appserver_cli_info *owner)
{
	if (strcmp(device_cmd[SET_TIMER], command) == 0)
		return set_timer(command, data, owner);
	else if (strcmp(device_cmd[GET_TIMER], command) == 0)
		return get_timer(command, data, owner);
	else if (strcmp(device_cmd[RESUME_DEFAULT_SETTING], command) == 0)
		return resume_default_setting(command, data, owner);
	else if (strcmp(device_cmd[GET_DETAIL_DEVICE_INFO], command) == 0)
		return get_detail_device_info(command, data, owner);
	else if (strcmp(device_cmd[SET_DEVICE_NAME], command) == 0)
		return set_device_name(command, data, owner);
	else if (strcmp(device_cmd[GET_DEVICE_NAME], command) == 0)
		return get_device_name(command, data, owner);

	return 0;
}
