#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdbool.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <execinfo.h>
#include <sys/ioctl.h>
#include <linux/soundcard.h>

#include "player_interface.h"
#include "localstream_interface.h"
#define DEFAULT_PATH "/var/run/mplayer0/mplayer.sock"
#define BUF_SIZE 128 * 1024
char sbuf[BUF_SIZE];
char *sock_path = DEFAULT_PATH;
char *file = NULL;
int quit_flag = 0;

struct play_param {
int audio_fd;
char * buf;
int buf_size;
};

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

	printf("\n\n[%s: %d] mozart crashed by signal %s.\n", __func__, __LINE__, signal_str[signo]);

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

static int lostream_init_callback(struct domain_init_hdr *header, void *data)
{
	struct play_param *lostream_priv = (struct play_param *)data;
	int format;
	int oss_rate;
#ifdef DEBUG
	printf("Audio channels: %d\n", header->info.channels);
	printf("Audio samplerate: %d\n", header->info.samplerate);
	printf("Audio samplesize: %d\n", header->info.samplesize);
#endif
	lostream_priv->audio_fd = -1;
	lostream_priv->buf = sbuf;
	lostream_priv->buf_size = BUF_SIZE;

	lostream_priv->audio_fd = open("/dev/dsp", O_WRONLY);
	if(lostream_priv->audio_fd < 0) {
		printf("open dsp fail!!\n");
		return -1;
	}
	if (header->info.channels > 2) {
		int channels = header->info.channels;
		if (ioctl(lostream_priv->audio_fd, SNDCTL_DSP_CHANNELS, &channels) < 0 ||
				channels != header->info.channels) {
			printf("Cannot set channels to %d", header->info.channels);
			goto err_out;
		}
	} else {
		int channels = header->info.channels - 1;
		if (ioctl(lostream_priv->audio_fd, SNDCTL_DSP_STEREO, &channels) < 0 ||
				channels + 1 != header->info.channels) {
			printf("Cannot set channels to %d", header->info.channels);
			goto err_out;
		}
	}

	if(header->info.samplesize == 1)
		format = AFMT_S8;
	else if(header->info.samplesize == 2)
		format = AFMT_S16_LE;
	if (ioctl(lostream_priv->audio_fd, SNDCTL_DSP_SETFMT, &format) < 0) {
		printf("Cannot set format to %d",  header->info.samplesize);
		goto err_out;
	}

	/* Some cards aren't too accurate with their clocks and set to the
	 * exact data rate, but something close.  Fail only if completely out
	 * of whack.
	 */
	oss_rate = header->info.samplerate;
	if (ioctl(lostream_priv->audio_fd, SNDCTL_DSP_SPEED, &oss_rate) < 0 ||
			oss_rate > 1.02 * header->info.samplerate ||
			oss_rate < 0.98 * header->info.samplerate) {
		printf("Cannot set rate to %d, get result %d", header->info.samplerate, oss_rate);
		goto err_out;
	}

	return 0;

err_out:
	close(lostream_priv->audio_fd);
	return -1;

}

static int lostream_stream_getbuffer(struct loStreamBuffer *s_buf, void *data)
{
	struct play_param *lostream_priv = (struct play_param *)data;

	s_buf->buf = lostream_priv->buf;
	s_buf->size = s_buf->wanted;

	return 0;
}

static int lostream_stream_callback(struct domain_stream_reply *header, int size, void *data)
{
	struct play_param *lostream_priv = (struct play_param *)data;
	int ret = 0;
	int len = 0;
	while(size) {
		ret = write(lostream_priv->audio_fd, lostream_priv->buf + len, size);
		size -= ret;
		len += ret;
	}

	return 0;
}

static void lostream_getspace_callback(struct domain_getspace_hdr *g_space, void *data)
{
	struct play_param *lostream_priv = (struct play_param *)data;
	static audio_buf_info zz;

	if (ioctl(lostream_priv->audio_fd, SNDCTL_DSP_GETOSPACE, &zz) < 0)
		printf("Cannot get oss buffer info");

	g_space->space = zz.fragstotal * zz.fragsize;
}

static int lostream_stop_callback(void *header, void *data)
{
	struct play_param *lostream_priv = (struct play_param *)data;

	if(lostream_priv->audio_fd > 0)
		close(lostream_priv->audio_fd);

	return 0;
}

static struct loStream_callback cbs = {
	.initStream	= lostream_init_callback,
	.getBuffer	= lostream_stream_getbuffer,
	.getSpace	= lostream_getspace_callback,
	.handleRecv	= lostream_stream_callback,
	.stopStream	= lostream_stop_callback,
};

int callback(player_snapshot_t *snapshot, struct player_handler *handler, void *param)
{
	struct localInStream *lostream = (struct localInStream *)param;
	printf("player controller uuid: %s, status: %s, pos is %d, duration is %d.\n",
			snapshot->uuid, player_status_str[snapshot->status], snapshot->position, snapshot->duration);

	switch (snapshot->status) {
		case PLAYER_TRANSPORT:
		case PLAYER_PLAYING:
		case PLAYER_PAUSED:
		case PLAYER_UNKNOWN:
			break;
		case AOSWITCH_FAIL:
			printf("ao switch fail!!\n");
			break;
		case PLAYER_STOPPED:
			if (!strcmp(handler->uuid, snapshot->uuid)) {
				mozart_local_stream_uninit(lostream);
				quit_flag = 1;
			}
			break;
		default:
			break;
	}

	return 0;
}

static void usage(const char *app_name)
{
	printf("%s [-f file]\n"
			" -h help (show this usage text)\n"
			" -s socket path\n"
			" -f file\n", app_name);

	return;
}

int main(int argc, char **argv)
{
	int ret = 0;
	int c = -1;
	player_handler_t *handler = NULL;
	struct localInStream *lostream = NULL;
	struct play_param *lostream_priv = malloc(sizeof(struct play_param));
	memset(lostream_priv, 0, sizeof(struct play_param));

	signal(SIGPIPE, SIG_IGN);
	signal(SIGINT, sig_handler);
	signal(SIGTERM, sig_handler);
	signal(SIGBUS, sig_handler);
	signal(SIGSEGV, sig_handler);
	signal(SIGABRT, sig_handler);

	while (1) {
		c = getopt(argc, argv, "s:f:u");
		if (c < 0)
			break;
		switch (c) {
			case 's':
				sock_path = optarg;
				break;
			case 'f':
				file = optarg;
				break;
			case 'h':
				usage(argv[0]);
				return 0;
			default:
				usage(argv[0]);
				return -1;
		}
	}
	lostream = mozart_local_stream_create(sock_path, 128 * 1024);
	if(!lostream) {
		printf("local_stream_create fail!!\n");
		goto err;
	}
	/*  Init localstream */
	ret = mozart_local_stream_init(lostream, &cbs, (void *)lostream_priv);
	if(ret < 0) {
		printf("local_stream_init fail!!\n");
		goto err;
	}

	handler = mozart_player_ch_handler_get("localstream_test", 0, callback, (void *)lostream);
	if (!handler) {
		printf("get player handler failed, do nothing, exit...\n");
		goto err;
	}
	char *ao = malloc(strlen("stream:sockfile=") + strlen(sock_path) + 1);
	sprintf(ao,"stream:sockfile=%s",sock_path);
	printf("ao:%s\n",ao);

	ret = mozart_player_aoswitch(handler, ao);
	if(ret < 0) {
		printf("mozart_player_aoswitch(%s) fail!!\n",ao);
	}
	free(ao);

	if (file)
		mozart_player_playurl(handler, file);

	while(1) {
		if(!quit_flag)
			sleep(1);
		else{
			ret = mozart_player_aoswitch(handler, "oss:/dev/dsp");
			if(ret < 0) {
				printf("mozart_player_aoswitch(oss) fail!!\n");
			}
			mozart_local_stream_release(lostream);
			if(lostream_priv) {
				free(lostream_priv);
				lostream_priv = NULL;
			}
			break;
		}
	}
	return 0;
err:
	if(lostream)
		mozart_local_stream_release(lostream);
	if(lostream_priv) {
		free(lostream_priv);
		lostream_priv = NULL;
	}
	return -1;
}

