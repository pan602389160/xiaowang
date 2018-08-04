#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#include "player_interface.h"
#include "tips_interface.h"
#include "vr_interface.h"

static player_handler_t *vr_player_handler = NULL;
static player_status_t vr_player_status = PLAYER_UNKNOWN;

static int vr_player_callback(player_snapshot_t *snapshot, struct player_handler *handler, void *param)
{
	if (strcmp(handler->uuid, snapshot->uuid)) {
		printf("Not render control mode, exit...\n");
		vr_player_status = PLAYER_STOPPED;
		return 0;
	}

	printf("render recv player status: %d, %s.\n", snapshot->status, player_status_str[snapshot->status]);

	vr_player_status = snapshot->status;

	switch (snapshot->status) {
	case PLAYER_TRANSPORT:
		break;
	case PLAYER_PLAYING:
		break;
	case PLAYER_PAUSED:
		break;
	case PLAYER_STOPPED:
		break;
	case PLAYER_UNKNOWN:
		vr_player_status = PLAYER_STOPPED;
		break;
	default:
		vr_player_status = PLAYER_STOPPED;
		printf("Unsupported status: %d.\n", snapshot->status);
		break;
	}

	return 0;
}

int vr_player_init(void)
{
	char *cur_uuid = NULL;
	vr_player_status = PLAYER_UNKNOWN;

	vr_player_handler = mozart_player_handler_get("mozart-vr", vr_player_callback, NULL);
	if (!vr_player_handler) {
		printf("get player handler failed.\n");
		return -1;
	}
	cur_uuid = mozart_player_getuuid(vr_player_handler);

	if (!strcmp(cur_uuid, "mozart-vr"))
		vr_player_status = mozart_player_getstatus(vr_player_handler);

	return 0;
}

void vr_player_uninit(void)
{
	vr_player_status = PLAYER_STOPPED;

	mozart_player_handler_put(vr_player_handler);
	vr_player_handler = NULL;
}

int vr_player_playurl(char *file)
{
	return mozart_player_playurl(vr_player_handler, file);
}

int vr_player_cacheurl(char *file)
{
	return mozart_player_cacheurl(vr_player_handler, file);
}

int vr_player_pause(void)
{
	if (vr_player_status == PLAYER_PLAYING ||
		vr_player_status == PLAYER_TRANSPORT)
		return mozart_player_pause(vr_player_handler);
	else
		return 0;
}

int vr_player_resume(void)
{
	if (vr_player_status == PLAYER_PAUSED)
		return mozart_player_resume(vr_player_handler);
	else
		return 0;
}

int vr_player_stop(void)
{
	if (vr_player_status == PLAYER_STOPPED)
		return 0;
	else
		return mozart_player_stop(vr_player_handler);
}

int vr_player_volset(int volume)
{
	return mozart_player_volset(vr_player_handler, volume);
}

int vr_player_volget(void)
{
	return mozart_player_volget(vr_player_handler);
}

int vr_player_seek(int second)
{
	return mozart_player_seek(vr_player_handler, second);
}

int vr_player_getpos(void)
{
	return mozart_player_getpos(vr_player_handler);
}

int vr_player_getduration(void)
{
	return mozart_player_getduration(vr_player_handler);
}

player_status_t vr_player_getstatus(void)
{
	return vr_player_status;
}

void vr_player_playtts(char *word, bool sync)
{
	char *url = NULL;
	if (word && *word) {
		url = mozart_aispeech_tts(word);
		if (url) {
			printf("%s\n", url);
		} else {
			printf("tts error.\n");
			return;
		}

		if (sync)
			mozart_play_tone_sync(url);
		else
			mozart_play_tone(url);
	}

	return;
}
