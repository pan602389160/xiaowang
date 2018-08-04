#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <pthread.h>
#include <ini_interface.h>
#include <record_interface.h>
#include <utils_interface.h>

#define BIT (16)
#define RATE (16000)
#define CHANEL (1)
#define VOLUME (60)
#define AEC_SIZE (1024)
#define DEBUG_RECORD

static aec_quit = 0;
static aec_record *record = NULL;
static char audio_typ[16] = {0};
static char loopback_buf[AEC_SIZE] = {};
static char dmic_buf[AEC_SIZE] = {};

#ifdef DEBUG_RECORD
static int file_count = 0;
char name1[32], name2[32];
static int fd_dmic;
static int fd_amic;
typedef struct{
	unsigned char riff[4];
	unsigned int file_length;
	unsigned char wav_flag[4];
	unsigned char fmt_flag[4];
	unsigned int transition;
	unsigned short format_tag;
	unsigned short channels;
	unsigned int sample_rate;
	unsigned int wave_rate;
	unsigned short block_align;
	unsigned short sample_bits;
	unsigned char data_flag[4];
	unsigned int data_length;
} wav_header;

static void init_wave_head(wav_header *wave, int rate,
		int bits, int channels, int size)
{
	memcpy(wave->riff, "RIFF", 4);
	wave->file_length = size + 36;
	memcpy(wave->wav_flag, "WAVE", 4);
	memcpy(wave->fmt_flag, "fmt ", 4);
	wave->transition = 0x10;
	wave->format_tag = 0x01;
	wave->channels = channels;
	wave->sample_rate = rate;
	wave->wave_rate = rate * channels * bits / 8;
	wave->block_align = channels * bits / 8;
	wave->sample_bits = bits;
	memcpy(wave->data_flag, "data", 4);
	wave->data_length = size;
}
static void set_wave_head(int file_fd, wav_header *wave)
{
	lseek(file_fd, 0, SEEK_SET);
	write(file_fd, wave, sizeof(wav_header));
}

static int get_file_size(char *file_name)
{
	struct stat statbuf;
	stat(file_name, &statbuf);
	int size = statbuf.st_size;
	return size;
}

static void finally_set_wav_data_size_amic(char *filename, int fd)
{
	int size = 0;
	wav_header wave;
	size = get_file_size(filename)-44;
	init_wave_head(&wave, 16000, 16, 1, size);
	set_wave_head(fd, &wave);
}

static void finally_set_wav_data_size_dmic(char *filename, int fd)
{
	int size = 0;
	wav_header wave;
	size = get_file_size(filename)-44;
	init_wave_head(&wave, RATE, BIT, 1, size);
	set_wave_head(fd, &wave);
}

static void set_record_file_head()
{
	wav_header *wave;
	sprintf(name1, "test_amic%d.wav", file_count);
	sprintf(name2, "test_dmic%d.wav", file_count);
	wave = malloc(sizeof(wav_header));
	fd_amic = open(name1, O_RDWR | O_CREAT | O_TRUNC, 0777);
	fd_dmic = open(name2, O_RDWR | O_CREAT | O_TRUNC, 0777);
	if (get_audio_type() == AUDIO_ALSA)
		init_wave_head(wave, RATE, BIT, 2, 0);
	else
		init_wave_head(wave, RATE, BIT, 1, 0);
	/* configuration overwrite in finally_set_wav_data_size() */
	set_wave_head(fd_amic, wave);
	init_wave_head(wave, RATE, BIT, 1, 0);
	/* configuration overwrite in finally_set_wav_data_size() */
	set_wave_head(fd_dmic, wave);
}
#endif

static int soundcard_init(void)
{
	int delay_time;
	int ret;

	record_param dmic_param = {
		.bits = BIT,
		.rates = RATE,
		.channels = 1,
		.volume = 60,
	};

	record_param loopback_param = {
		.bits = BIT,
		.rates = RATE,
		.channels = 1,
		.volume = 80,
	};

	if (get_audio_type() == AUDIO_ALSA)
		loopback_param.channels = 2;

	if (get_audio_codec_type() == EXTER_CODEC) {
		if (get_audio_type() == AUDIO_ALSA)
			loopback_param.rates = RATE;		/*If sample rate == 48khz*/
		/*This time is related to the distance between mic and speaker.*/
		delay_time = 40;
	} else {
		delay_time = 46;
	}

#ifdef DEBUG_RECORD
	set_record_file_head();
#endif

	record = mozart_soundcard_aec_init(loopback_param, dmic_param);
	if (record == NULL) {
		printf("error: soundcard aec init fail.\n");
		ret = -1;
		goto err_init;
	}

	ret = mozart_soundcard_aec_enable(record, delay_time);
	if (ret != 0) {
		printf("error: soundcard aec enable fail\n");
		goto err_aec_enable;
	}
	return 0;

err_init:
err_aec_enable:
	mozart_soundcard_aec_uninit(record);
	record = NULL;

	return ret;
}

static int soundcard_uninit(void)
{
	mozart_soundcard_aec_disable(record);
	mozart_soundcard_aec_uninit(record);
	record = NULL;
	return 0;
}

static unsigned long soundcard_aec_record(char *loopback_buf,
		char *dmic_buf, unsigned long len)
{
	if (!record) {
		printf("error: Please init soundcard firstly!\n");
		return 0;
	}

	return mozart_soundcard_aec_record(record, loopback_buf, dmic_buf, AEC_SIZE);
}

float compute_delay_time()
{
	char buf[64] = {0};
	short value = 0;
	float amic_offset = 0, dmic_offset = 0;
	int flag = 0, ret = 0;

	printf("start compute_delay_time\n");

	fd_amic = open("test_amic0.wav", O_RDONLY);
	fd_dmic = open("test_dmic0.wav", O_RDONLY);
	read(fd_amic, buf, 44);
	read(fd_dmic, buf, 44);

	while (1) {
		ret = read(fd_amic, &value, sizeof(short));
		if (ret == 0)
			break;
		if (value > 3000) {
			flag = 1;
			break;
		}
		amic_offset++;
	}
	if (flag == 0) {
		printf("play data is error\n");
		return -999;
	}
	flag = 0;
	while (1) {
		ret = read(fd_dmic, &value, sizeof(short));
		if (ret == 0)
			break;
		if (value > 1500) {
			flag = 1;
			break;
		}
		dmic_offset++;
	}
	if (flag == 0) {
		printf("dmic data is error\n");
		return -999;
	}

	printf("amic_offset :%.0f dmic_offset :%.0f\n",
			amic_offset, dmic_offset);
	close(fd_dmic);
	close(fd_amic);
	return (dmic_offset - amic_offset) / 16;
}

void *aec_read()
{
	int status = 0;
	char loopback_buf[AEC_SIZE] = {};
	char dmic_buf[AEC_SIZE] = {};
	while (!aec_quit)
		soundcard_aec_record(loopback_buf, dmic_buf, AEC_SIZE);
	pthread_exit(&status);
}

int main(int argc, char **argv)
{
	char cmd[128] = {0}, input[16] = {0};
	int ret = 0, cnt = 200, mode = 0;
	float time = 0;
	void *tret = NULL;
	pthread_t tid1;
	enum {
		SHORT,
		LONG
	};
	if (argc != 3) {
		printf("usage : ./a.out mode audio_type[0: short time test 1: long time test]\n");
		return -1;
	}
	mode = atoi(argv[1]);
	if (mode != SHORT && mode != LONG) {
		printf("usage : ./a.out mode[0: short time test 1: long time test]\n");
		return -1;
	}
	memcpy(audio_typ, argv[2], strlen(argv[2]));
	printf("===========test start==============\n");
	soundcard_init();
	if (mode == 1) {
		pthread_create(&tid1, NULL, aec_read, NULL);
		printf("start long time test, input aecstop quit\n");
		do {
			scanf("%s", input);
		} while (strcmp(input, "aecstop"));
		aec_quit = 1;
		pthread_join(tid1, &tret);
	}
	sprintf(cmd, "mplayer -slave -quiet -ao %s align.wav > /dev/null &", audio_typ);
	ret = system(cmd);
	if (ret) {
		printf("system mplayer error\n");
		perror("system");
		return -1;
	} else {
		printf("system mplayer successful\n");
	}
	while (cnt--) {
		soundcard_aec_record(loopback_buf, dmic_buf, AEC_SIZE);
#ifdef DEBUG_RECORD
		write(fd_dmic, dmic_buf, AEC_SIZE);
		write(fd_amic, loopback_buf, AEC_SIZE);
#endif
	}
#ifdef DEBUG_RECORD

	printf("===========test finish=============\n");
	finally_set_wav_data_size_amic(name1, fd_amic);
	finally_set_wav_data_size_dmic(name2, fd_dmic);
	close(fd_amic);
	close(fd_dmic);
	file_count++;
#endif
	soundcard_uninit();

	time = compute_delay_time();
	if (time == -999) {
		printf("test error=====================\n");
		return -1;
	}

	printf("---------------------------------------\n");
	printf("-----time play-dmic is :%.3f (ms)--\n", time);
	printf("---------------------------------------\n");
	return 0;
}

