#include <stdio.h>
#include <stdlib.h>

#include "json-c/json.h"
#include "tips_interface.h"
#include "localplayer_interface.h"

#include "mozart_config.h"
#include "mozart_musicplayer.h"
#include "mozart_key_function.h"
#include "mozart_ui.h"
#include "modules/mozart_module_local_music.h"

#if (SUPPORT_LOCALPLAYER == 1)

static int mozart_module_local_music_start(void)
{
	if (snd_source != SND_SRC_LOCALPLAY) {
		if (mozart_module_stop())
			return -1;
		if (mozart_musicplayer_start(mozart_musicplayer_handler))
			return -1;

		mozart_ui_localplayer_plugin();
		mozart_play_key("native_mode");
		snd_source = SND_SRC_LOCALPLAY;
	} else if (!mozart_musicplayer_is_active(mozart_musicplayer_handler)) {
		if (mozart_musicplayer_start(mozart_musicplayer_handler))
			return -1;
	} else {
		mozart_musicplayer_musiclist_clean(mozart_musicplayer_handler);
	}

	return 0;
}

int mozart_module_local_music_startup(void)
{
	int ret, i, music_num, id = 0;
	bool scan = false;
	char *music = NULL;
	const char *music_name = NULL;
	const char *music_url = NULL;
	json_object *object, *song, *tmp;
	struct music_info *info = NULL;

	scan = mozart_localplayer_is_scanning();
	music = mozart_localplayer_get_musiclist();
	if (music == NULL) {
		printf("localplayer's music list is empty\n");
		return -1;
	}

	object = json_tokener_parse(music);
	if (object == NULL) {
		printf("%s: json parse failed\n", __func__);
		free(music);
		return -1;
	}

	if (mozart_module_local_music_start())
		return -1;

	if (scan)
		music_num = 1;
	else
		music_num = json_object_array_length(object);

	for (i = 0; i < music_num; i++) {
		song = json_object_array_get_idx(object, i);
		if (json_object_object_get_ex(song, "name", &tmp))
			music_name = json_object_get_string(tmp);
		if (json_object_object_get_ex(song, "url", &tmp))
			music_url = json_object_get_string(tmp);

		info = mozart_musiclist_get_info(id, (char *)music_name, (char *)music_url, NULL,
						 NULL, NULL, NULL);
		if (info)
			mozart_musicplayer_musiclist_insert(mozart_musicplayer_handler, info);
	}

	ret = mozart_musicplayer_play_index(mozart_musicplayer_handler, 0);

	json_object_put(object);
	free(music);

	return ret;
}

#endif	/* SUPPORT_LOCALPLAYER */
