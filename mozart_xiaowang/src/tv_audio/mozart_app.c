#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <errno.h>
#include <time.h>

#include "utils_interface.h"
#include "sharememory_interface.h"
#include "wifi_interface.h"
#include "localplayer_interface.h"

#include "mozart_config.h"

#include "mozart_key_function.h"
#include "mozart_musicplayer.h"

#include "mozart_app.h"

#if (SUPPORT_MULROOM == 1)
#include "mulroom_interface.h"
#endif

#if (SUPPORT_LOCALPLAYER == 1)
bool localplayer_started = false;
#endif

__attribute__((unused)) static bool lapsule_started = false;

int start(app_t app)
{
	switch(app){
	case MUSICPLAYER:
		mozart_musicplayer_startup();
		break;
#if (SUPPORT_LOCALPLAYER == 1)
	case LOCALPLAYER:
		if (localplayer_started) {
			stop(app);
			localplayer_started = false;
		}

		if (mozart_localplayer_startup()) {
			return -1;
		}

		localplayer_started = true;
		break;
#endif
#if (SUPPORT_MULROOM == 1)
	case MULROOM:
		break;
#endif

	default:
		printf("WARNING: Not support this module(id = %d)\n", app);
		break;
	}

	return 0;
}

int stop(app_t app)
{
	switch(app){
	case MUSICPLAYER:
		mozart_musicplayer_shutdown();
		break;
#if (SUPPORT_LOCALPLAYER == 1)
	case LOCALPLAYER:
		if (localplayer_started) {
			mozart_localplayer_shutdown();
			localplayer_started = false;
		}
		break;
#endif
#if (SUPPORT_MULROOM == 1)
	case MULROOM:
		mozart_mulroom_dismiss_group();
		break;
#endif

	default:
		printf("WARNING: Not support this module(id = %d)\n", app);
		break;
	}

	return 0;
}


int startall(int depend_network)
{
	if (depend_network == -1 || depend_network == 1) {
#if (SUPPORT_LOCALPLAYER == 1)
		start(LOCALPLAYER);
#endif
	}

	if (depend_network == -1 || depend_network == 0) {
		start(MUSICPLAYER);
	}

	return 0;
}

int stopall(int depend_network)
{
	if (depend_network == -1 || depend_network == 1) {
#if (SUPPORT_LOCALPLAYER == 1)
		stop(LOCALPLAYER);
#endif
	}

	if (depend_network == -1 || depend_network == 0) {
		stop(MUSICPLAYER);
#if (SUPPORT_MULROOM == 1)
		stop(MULROOM);
#endif
	}

	return 0;
}

int restartall(int depend_network)
{
	stopall(depend_network);
	startall(depend_network);

	return 0;
}
