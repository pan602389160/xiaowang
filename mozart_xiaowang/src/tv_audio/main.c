/*
	main.c
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <getopt.h>
#include <libgen.h>
#include <signal.h>
#include <execinfo.h>

#include "tv_audio.h"

struct tv_audio_args{
	device_mode_t	mode;
	int		daemon;
};

static const char *usage_str = {
	"Usage: tvaudio [-d,--daemon] <-p,--planet|-m,--moon>\n"
	"    or tvaudio [-h,--help]\n\n"
	"  -p, --planet    tv audio run as a planet module\n"
	"  -m, --moon      tv audio run as a moon module\n"
	"  -d, --daemon    process daemon\n"
	"  -h, --help      print help usage\n"
};

static char *app_name;

static void usage_print(void)
{
	printf(usage_str);
}

static int parse_argument(struct tv_audio_args *args, int argc, char **argv)
{
	struct option longopts[] = {
		{"planet", no_argument, NULL, 'p'},
		{"moon", no_argument, NULL, 'm'},
		{"daemon", no_argument, NULL, 'd'},
		{"help", no_argument, NULL, 'h'},
		{NULL, 0, NULL, 0},
	};
	int op;

	while ((op = getopt_long(argc, argv, "pmdh", longopts, NULL)) != -1) {
		switch (op) {
		case 'p':
			if (args->mode != M_EMPTY) {
				printf("%s: mode has been set\n", app_name);
				usage_print();
				exit(0);
			}
			args->mode = M_PLANET;
			break;
		case 'm':
			if (args->mode != M_EMPTY) {
				printf("%s: mode has been set\n", app_name);
				usage_print();
				exit(0);
			}
			args->mode = M_MOON;
			break;
		case 'd':
			args->daemon = 1;
			break;
		case 'h':
		default:
			usage_print();
			exit(1);
		}
	}

	/* Export no detect args */
	if (optind < argc) {
		int i;
		for (i = optind; i < argc; i++)
			fprintf(stderr, "%s: ignoring extra argument -- %s\n",
					app_name, argv[i]);
	}

	return 0;
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

static void sig_handler(int signo)
{
	char cmd[64] = {};
	void *array[10];
	int size = 0;
	char **strings = NULL;
	int i = 0;

	printf("%s: service %s by %s.\n",
			app_name, strsignal(signo), signal_str[signo]);

	printf("Call Trace:\n");
	size = backtrace(array, 10);
	strings = backtrace_symbols(array, size);
	if (strings) {
		for (i = 0; i < size; i++)
			printf("  %s\n", strings[i]);
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

	/* TODO tv_audio_exit */
	tv_audio_terminate();

	fflush(stdout);
	exit(-1);
}

int main(int argc, char **argv)
{
	struct tv_audio_args tva;
	int err;

	app_name = basename(argv[0]);

	/* perse arguments */
	memset(&tva, 0, sizeof(tva));
	if (parse_argument(&tva, argc, argv) < 0) {
		usage_print();
		return -1;
	}

	if (!tva.mode) {
		printf("%s: run mode NOT set\n", app_name);
		usage_print();
		exit(1);
	}

	/* run in the background */
	if (tva.daemon) {
		err = daemon(0, 1);
	       	if (err < 0){
			printf("%s: daemon %s\n", app_name, strerror(errno));
			return -1;
		}
	}

	/* Set signal hander */
	signal(SIGINT, sig_handler);
	signal(SIGUSR1, sig_handler);
	signal(SIGUSR2, sig_handler);
	signal(SIGTERM, sig_handler);
	signal(SIGBUS, sig_handler);
	signal(SIGSEGV, sig_handler);
	signal(SIGABRT, sig_handler);
	signal(SIGPIPE, SIG_IGN);

	err = tv_audio_start(app_name, tva.mode);
	if (err < 0) {
		printf("%s: tv_audio_start failed\n", app_name);
		return -1;
	}

	for (;;) {
		sleep(20);
		system("echo 3 > /proc/sys/vm/drop_caches");
	}

	return 0;
}
