#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include <sys/time.h>
#include <netinet/in.h>
#include <json-c/json.h>

#include "ini_interface.h"
#include "player_interface.h"
#include "utils_interface.h"
#include "mulroom_interface.h"
#include "mulroom_jsonkey.h"

#include "modules/mozart_module_mulroom.h"

#define MODULE_MR_DEBUG

#ifdef MODULE_MR_DEBUG
#define pr_debug(fmt, args...)                  \
	printf("[Module.MultiRoom][Debug] -%d- %s. "fmt, __LINE__, __func__, ##args)
#define fpr_debug(fmt, args...)                 \
	fprintf(stderr, "[Module.MultiRoom][Debug] -%d- %s. "fmt, __LINE__, __func__, ##args)
#else
#define pr_debug(fmt, args...)
#define fpr_debug(fmt, args...)
#endif /* MODULE_MR_DEBUG */

#define pr_err(fmt, args...)                    \
	fprintf(stderr, "[Module.MultiRoom][Error] -%d- %s. "fmt, __LINE__, __func__, ##args)

#define pr_warn(fmt, args...)                   \
	printf("[Module.MultiRoom][Warn] -%d- %s. "fmt, __LINE__, __func__, ##args)

#define pr_info(fmt, args...)                   \
	printf("[Module.MultiRoom][Info] -%d- %s. "fmt, __LINE__, __func__, ##args)

static char *ao_mulroom_distributor = "stream:sockfile=/var/run/mulroom/stream.sock";
static char *ini_path = "/usr/data/player.ini";
static char ao_mulroom_normal[128];

static int normal_play_interface_get(char *ini_path, char *ao_iface)
{
	return mozart_ini_getkey(ini_path, "mplayer", "ao", ao_iface);
}

static int mulroom_change_audio_output(char *ao_iface)
{
	player_handler_t *handle;
	int err;

	handle = mozart_player_handler_get("multiroom", NULL, NULL);
	if (!handle)
		return -1;

	err = mozart_player_aoswitch(handle, ao_iface);
	mozart_player_handler_put(handle);

	return err;
}

int module_mulroom_audio_change(mulroom_ao_mode_t mode)
{
	int err;

	switch (mode) {
	case MR_AO_NORMAL:
		err = normal_play_interface_get(ini_path, ao_mulroom_normal);
		if (err) {
			pr_err("get ao normal interface failed\n");
			err = -1;
			break;
		}

		err = mulroom_change_audio_output(ao_mulroom_normal);
		if (err < 0)
			pr_err("change player ao normal failed");
		break;
	case MR_AO_DISTRIBUTOR:
		err = mulroom_change_audio_output(ao_mulroom_distributor);
		if (err < 0)
			pr_err("change player ao stream failed\n");
		break;
	default:
		pr_err("error mode code: %d\n", mode);
		err = -1;
	}

	return err;
}

int module_mulroom_get_gateip(char *gateip_buf)
{
	char res_str[INET_ADDRSTRLEN] = {0};
	FILE *res_ptr;
	ssize_t res_len;

	char p_cmd[] = {
		"ip route | awk '/default/{print $3}'"
	};

	res_ptr = popen(p_cmd, "re");
	if (!res_ptr) {
		pr_err("Get gate ip popen: %s\n", strerror(errno));
		return -1;
	}

	res_len = fread(res_str, 1, INET_ADDRSTRLEN, res_ptr);
	pclose(res_ptr);

	if (res_len) {
		strcpy(gateip_buf, res_str);
	} else {
		pr_err("Not get gateway ip addr\n");
		return -1;
	}

	return 0;
}

char *module_mulroom_get_hostname(void)
{
	char *host_name;
	char hw_addr[6 * 2 + 1] = {0};

	host_name = malloc(64);
	if (!host_name) {
		pr_err("alloc name buffer: %s\n", strerror(errno));
		return NULL;
	}

	/* Host name + '-' + MAC address */
	gethostname(host_name, sizeof(host_name) - (sizeof(hw_addr) - 4) - 1);
	get_mac_addr("wlan0", hw_addr, "");

	strcat(host_name, "-");
	strcat(host_name, hw_addr + 4);

	return host_name;
}

int module_mulroom_group_master(int group_id)
{
	struct json_object *master_obj;
	struct json_object *groupid_obj;
	struct json_object *list_obj;
	char *master_str;
	int err = 0;

	master_obj = json_object_new_object();
	if (!master_obj) {
		pr_err("new master group json failed\n");
		return -1;
	}

	groupid_obj = json_object_new_int(group_id);
	if (!groupid_obj) {
		pr_err("new group id json failed\n");
		err = -1;
		goto err_groupid_obj;
	}
	json_object_object_add(master_obj, JSON_KEY_GROUP_ID, groupid_obj);

	list_obj = json_object_new_array();
	if (!list_obj) {
		pr_err("new empty recv array failed\n");
		err = -1;
		goto err_list_obj;
	}

	master_str = (char *)json_object_to_json_string(master_obj);
	mozart_mulroom_group_distributor(master_str, NULL);

err_list_obj:
err_groupid_obj:
	json_object_put(master_obj);

	return err;
}

int module_mulroom_group_slave(
	int group_id,
	char *slave_name,
	char *gate_ip)
{
	struct json_object *slave_obj;
	struct json_object *groupid_obj;
	struct json_object *masterip_obj;
	struct json_object *name_obj;
	char *slave_str;
	int err = 0;

	slave_obj = json_object_new_object();
	if (!slave_obj) {
		pr_err("new slave group json failed\n");
		return -1;
	}

	groupid_obj = json_object_new_int(group_id);
	if (!groupid_obj) {
		pr_err("new groupid json failed\n");
		err = -1;
		goto err_groupid_obj;
	}
	json_object_object_add(slave_obj, JSON_KEY_GROUP_ID, groupid_obj);

	masterip_obj = json_object_new_string(gate_ip);
	if (!masterip_obj) {
		pr_err("new master ip json failed\n");
		err = -1;
		goto err_masterip_obj;
	}
	json_object_object_add(slave_obj, JSON_KEY_MASTER_IP, masterip_obj);

	name_obj = json_object_new_string(slave_name);
	if (!name_obj) {
		pr_err("new slave name json failed\n");
		err = -1;
		goto err_name_obj;
	}
	json_object_object_add(slave_obj, JSON_KEY_SLAVE_NAMEID, name_obj);

	slave_str = (char *)json_object_to_json_string(slave_obj);
	mozart_mulroom_group_receiver(slave_str, NULL);

err_name_obj:
err_masterip_obj:
err_groupid_obj:
	json_object_put(slave_obj);

	return err;
}

int module_mulroom_group_dismiss(void)
{
	return mozart_mulroom_dismiss_group();
}
