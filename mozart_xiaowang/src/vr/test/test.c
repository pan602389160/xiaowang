#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <execinfo.h>
#include <signal.h>

#include "vr_interface.h"
#include "volume_interface.h"
#include "tips_interface.h"

#include "vrplayer.h"

static bool need_resume_after_asr = false;

static vr_result_t process_vr_aec_callback(aec_info_t *aec)
{
	vr_result_t result;
	memset(&result, 0, sizeof(result));

	if (aec->wakeup) {
		printf("aec wakeup.\n");
		vr_player_pause();
		while (1) {
			if (vr_player_getstatus() == PLAYER_PAUSED ||
				vr_player_getstatus() == PLAYER_STOPPED)
				break;
			usleep(1000);
		}
		mozart_play_key_sync("welcome");
		result.vr_next = VR_TO_ASR;
	} else {
		/* aec error, back to AEC again. */
		result.vr_next = VR_TO_AEC;
	}

	return result;
}

static void process_vr_asr_command_command(asr_info_t *asr)
{
	sem_command_t *command = &asr->sem.request.command;

	/* response volume first. */
	if (command->volume) {
		if (!strcmp(command->volume, "+")) {
			mozart_volume_set(mozart_volume_get() + 20, MUSIC_VOLUME);
		} else if (!strcmp(command->volume, "-")) {
			mozart_volume_set(mozart_volume_get() - 20, MUSIC_VOLUME);
		} else if (!strcmp(command->volume, "max")) {
			mozart_volume_set(100, MUSIC_VOLUME);
		} else if (!strcmp(command->volume, "min")) {
			mozart_volume_set(10, MUSIC_VOLUME);
		} else if (command->volume[0] >= '0' && command->volume[0] <= '9') {
			mozart_volume_set(atoi(command->volume), MUSIC_VOLUME);
		} else {
			printf("TODO: unsupport volume set format: %s.\n", command->volume);
		}
	}

	/* then response operation */
	if (command->operation) {
		if (strcmp(command->operation, "暂停") == 0) {
			printf("暂停\n");
			vr_player_pause();
			need_resume_after_asr = false;
		} else if (strcmp(command->operation, "播放") == 0) {
			/* FIXME: play new music on STOPPED status */
			if (PLAYER_PAUSED == vr_player_getstatus())
				vr_player_resume();
			need_resume_after_asr = false;
		} else if (strcmp(command->operation, "继续") == 0) {
			vr_player_resume();
			need_resume_after_asr = false;
		} else if (strcmp(command->operation, "停止") == 0) {
			vr_player_stop();
			need_resume_after_asr = false;
		} else if (strcmp(command->operation, "上一首") == 0) {
			need_resume_after_asr = false;
		} else if (strcmp(command->operation, "下一首") == 0) {
			need_resume_after_asr = false;
		} else if (strcmp(command->operation, "退出") == 0) {
		} else if (strcmp(command->operation, "结束") == 0) {
		} else {
		}
	}

	return;
}

static void process_vr_asr_command(asr_info_t *asr)
{
	printf("sem.request.domain: %s\n", vr_domain_str[asr->sem.request.domain]);

	switch (asr->sem.request.domain) {
	case DOMAIN_MUSIC:
		vr_player_playtts("音乐类命令未实现", true);
		break;
	case DOMAIN_MOVIE:
		vr_player_playtts("电影类命令未实现", true);
		break;
	case DOMAIN_NETFM:
		vr_player_playtts("电台类命令未实现", true);
		break;
	case DOMAIN_COMMAND:
		process_vr_asr_command_command(asr);
		break;
	default:
		vr_player_playtts("不支持的领域命令", true);
		break;
	}

	return;
}

static vr_result_t process_vr_asr_callback(asr_info_t *asr)
{
	vr_result_t result;
	memset(&result, 0, sizeof(result));

	printf("asr result, domain: %s.\n", vr_domain_str[asr->sds.domain]);

	need_resume_after_asr = true;

	switch (asr->sds.domain) {
	case DOMAIN_MUSIC:
		if (asr->sds.music.number >= 1) {
			char tips[128] = {};
			int index = 0;
			if (asr->sds.music.number > 1) {
				srandom(time(NULL));
				index = random() % asr->sds.music.number;
			}
			/* cache music(keep paused state) */
			vr_player_cacheurl(asr->sds.music.data[index].url);

			/* play tts */
			sprintf(tips, "%s,%s", asr->sds.music.data[index].artist, asr->sds.music.data[index].title);
			vr_player_playtts(tips, true);

			/* resume paused cache music */
			vr_player_resume();
			need_resume_after_asr = false;
		} else {
			/* TODO: command, such as volume control. */
			/* TODO: speak 我想听, will recieve a tips, process as multi chat. */
			vr_player_playtts(asr->sds.output, true);
		}
		break;
	case DOMAIN_NETFM:
		if (asr->sds.netfm.number >= 1) {
			int index = 0;
			if (asr->sds.netfm.number > 1) {
				srandom(time(NULL));
				index = random() % asr->sds.netfm.number;
			}
			/* cache netfm(keep paused state) */
			vr_player_cacheurl(asr->sds.netfm.data[index].url);

			/* play tts */
			vr_player_playtts(asr->sds.netfm.data[index].track, true);

			/* resume paused cache netfm */
			vr_player_resume();
			need_resume_after_asr = false;
		} else {
			/* TODO: command, such as volume control. */
			vr_player_playtts(asr->sds.output, true);
		}
		break;
	case DOMAIN_CHAT:
		if (!strcmp(asr->input, "") ||
			!strcmp(asr->input, "^"))
			break;

		if (asr->sds.chat.url) {
			vr_player_playurl(asr->sds.chat.url);
			/* joke is offer */
			asr->sds.state = SDS_STATE_OFFER;
			need_resume_after_asr = false;
			printf("chat joke, do not need resume.\n");
			break;
		}
	case DOMAIN_WEATHER:
	case DOMAIN_CALENDAR:
	case DOMAIN_STOCK:
	case DOMAIN_POETRY:
	case DOMAIN_MOVIE:
		printf("output: %s.\n", asr->sds.output);
		vr_player_playtts(asr->sds.output, true);
		break;
	case DOMAIN_REMINDER:
		printf("TODO: 提醒\n");
		vr_player_playtts("提醒类功能未实现", true);
		break;
	case DOMAIN_COMMAND:
		process_vr_asr_command(asr);
		break;
	default:
		vr_player_playtts("不支持的领域", true);
		break;
	}

	if (asr->sds.state == SDS_STATE_OFFERNONE ||
		asr->sds.state == SDS_STATE_INTERACT) {
		result.vr_next = VR_TO_ASR;
	} else {
		if (need_resume_after_asr) {
			printf("do not need resume.\n");
			vr_player_resume();
		}
		result.vr_next = VR_TO_AEC;
	}

	return result;
}

static vr_result_t process_vr_callback(vr_info_t *vr_info)
{
	vr_result_t result;
	memset(&result, 0, sizeof(result));

	if (vr_info->from == VR_FROM_AEC) {
		return process_vr_aec_callback(&vr_info->aec);
	} else if (vr_info->from == VR_FROM_ASR) {
		switch (vr_info->asr.errId) {
		case 70604:
		case 70605:
			/*TODO: replace local tone */
			vr_player_playtts("网络异常，请检测网络", true);
			break;
		case 70603:
		case 70613:
			/*TODO: replace local tone */
			vr_player_playtts("网速有点慢，请稍等", true);
			break;
		case 0:
			return process_vr_asr_callback(&vr_info->asr);
		default:
			printf("errId: %d, error: %s\n", vr_info->asr.errId, vr_info->asr.error);
			/*TODO: replace local tone */
			vr_player_playtts("服务忙，请稍后再试", true);
			break;
		}
		result.vr_next = VR_TO_ASR;
		return result;
	} else {
		printf("Unsupport callback source: %d, back to AEC mode.\n", vr_info->from);
		result.vr_next = VR_TO_AEC;
		return result;
	}
}

static char *signal_str[] = {
	[1] = "SIGHUP",       [2] = "SIGINT",       [3] = "SIGQUIT",      [4] = "SIGILL",      [5] = "SIGTRAP",
	[6] = "SIGABRT",      [7] = "SIGBUS",       [8] = "SIGFPE",       [9] = "SIGKILL",     [10] = "SIGUSR1",
	[11] = "SIGSEGV",     [12] = "SIGUSR2",     [13] = "SIGPIPE",     [14] = "SIGALRM",    [15] = "SIGTERM",
	[16] = "SIGSTKFLT",   [17] = "SIGCHLD",     [18] = "SIGCONT",     [19] = "SIGSTOP",    [20] = "SIGTSTP",
	[21] = "SIGTTIN",     [22] = "SIGTTOU",     [23] = "SIGURG",      [24] = "SIGXCPU",    [25] = "SIGXFSZ",
	[26] = "SIGVTALRM",   [27] = "SIGPROF",     [28] = "SIGWINCH",    [29] = "SIGIO",      [30] = "SIGPWR",
	[31] = "SIGSYS",      [34] = "SIGRTMIN",    [35] = "SIGRTMIN+1",  [36] = "SIGRTMIN+2", [37] = "SIGRTMIN+3",
	[38] = "SIGRTMIN+4",  [39] = "SIGRTMIN+5",  [40] = "SIGRTMIN+6",  [41] = "SIGRTMIN+7", [42] = "SIGRTMIN+8",
	[43] = "SIGRTMIN+9",  [44] = "SIGRTMIN+10", [45] = "SIGRTMIN+11", [46] = "SIGRTMIN+12", [47] = "SIGRTMIN+13",
	[48] = "SIGRTMIN+14", [49] = "SIGRTMIN+15", [50] = "SIGRTMAX-14", [51] = "SIGRTMAX-13", [52] = "SIGRTMAX-12",
	[53] = "SIGRTMAX-11", [54] = "SIGRTMAX-10", [55] = "SIGRTMAX-9",  [56] = "SIGRTMAX-8", [57] = "SIGRTMAX-7",
	[58] = "SIGRTMAX-6",  [59] = "SIGRTMAX-5",  [60] = "SIGRTMAX-4",  [61] = "SIGRTMAX-3", [62] = "SIGRTMAX-2",
	[63] = "SIGRTMAX-1",  [64] = "SIGRTMAX",
};

void sig_handler(int signo)
{
	char cmd[64] = {};
	void *array[10];
	int size = 0;
	char **strings = NULL;
	int i = 0;

	printf("\n\n[%s: %d] asr_test crashed by signal %s.\n", __func__, __LINE__, signal_str[signo]);

	printf("Call Trace:\n");
	size = backtrace(array, 10);
	strings = backtrace_symbols(array, size);
	if (strings) {
		for (i = 0; i < size; i++)
			printf ("  %s\n", strings[i]);
		free (strings);
	} else {
		printf("Not Found\n\n");
	}

	if (signo == SIGSEGV || signo == SIGBUS ||
		signo == SIGTRAP || signo == SIGABRT) {
		sprintf(cmd, "cat /proc/%d/maps", getpid());
		printf("Process maps:\n");
		system(cmd);
	}

	exit(-1);
}

int main(int argc, char **argv)
{	
	printf("ee==================main=============\n");
	// signal hander,
	signal(SIGINT, sig_handler);
	signal(SIGUSR1, sig_handler);
	signal(SIGUSR2, sig_handler);
	signal(SIGTERM, sig_handler);
	signal(SIGBUS, sig_handler);
	signal(SIGSEGV, sig_handler);
	signal(SIGABRT, sig_handler);
	signal(SIGPIPE, SIG_IGN);

	vr_player_init();

	mozart_vr_init(process_vr_callback);

	mozart_vr_start();

	while (1) {
		sleep(1);
	}

	return 0;
}
