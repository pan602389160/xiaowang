#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <pthread.h>

#include "record_interface.h"
#include "utils_interface.h"
#include "vr_interface.h"

static bool aec_stop_flag = false;
static bool aec_running = false;
static aec_record *record = NULL;

/* vr init flag */
extern bool vr_init_flag;

/* vr working flag, change on vr_start() & vr_stop() */
extern bool vr_working_flag;

/* record current vr status */
extern vr_status_t vr_status;

#ifdef DEBUG_RECORD
static int file_count = 0;
char name1[32],name2[32];
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
}wav_header;

static void init_wave_head(wav_header *wave,int rate,int bits,int channels, int size)
{
	memcpy(wave->riff,"RIFF",4);
	wave->file_length = size + 36;
	memcpy(wave->wav_flag,"WAVE",4);
	memcpy(wave->fmt_flag,"fmt ",4);
	wave->transition = 0x10;
	wave->format_tag = 0x01;
	wave->channels = channels;
	wave->sample_rate = rate;
	wave->wave_rate = rate*channels*bits/8;
	wave->block_align = channels*bits/8;
	wave->sample_bits = bits;
	memcpy(wave->data_flag,"data",4);
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
	init_wave_head(&wave, AEC_RATE, AEC_BIT, AEC_CHANNEL, size);
	set_wave_head(fd, &wave);
}

static void finally_set_wav_data_size_dmic(char *filename, int fd)
{
	int size = 0;
	wav_header wave;
	size = get_file_size(filename)-44;
	init_wave_head(&wave, AEC_RATE, AEC_BIT, AEC_CHANNEL, size);
	set_wave_head(fd, &wave);
}

static void set_record_file_head()
{
	wav_header *wave;
	sprintf(name1,"/tmp/test_amic%d.wav",file_count);
	sprintf(name2,"/tmp/test_dmic%d.wav",file_count);
	wave = malloc(sizeof(wav_header));
	fd_amic = open(name1, O_RDWR | O_CREAT | O_TRUNC);
	fd_dmic = open(name2, O_RDWR | O_CREAT | O_TRUNC);
	init_wave_head(wave, AEC_RATE, AEC_BIT, AEC_CHANNEL, 0);
	set_wave_head(fd_amic, wave);
	init_wave_head(wave, AEC_RATE, AEC_BIT, AEC_CHANNEL, 0);
	set_wave_head(fd_dmic, wave);
	free(wave);
}
#endif


static int soundcard_init(void)
{
	int delay_time;
	int ret;

	record_param dmic_param = {
		.bits = AEC_BIT,
		.rates = AEC_RATE,
		.channels = AEC_CHANNEL,
		.volume = AEC_VOLUME,
	};

	record_param loopback_param = {
		.bits = AEC_BIT,
		.rates = AEC_RATE,
		.channels = AEC_CHANNEL,
		.volume = AEC_VOLUME,
	};

	if (get_audio_type() == AUDIO_ALSA)
		loopback_param.channels = 2;

	if (get_audio_codec_type() == EXTER_CODEC) {
		if (get_audio_type() == AUDIO_ALSA)
			loopback_param.rates = 48000;		//If sample rate == 48khz
		delay_time = 40;    //This time is related to the distance between mic and speaker.
	} else {
		delay_time = 47;
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
#define LIGHT_PURPLE "\033[1;35m"
#define NONE          "\033[m"
#define my_pr(fmt, args...)		printf(LIGHT_PURPLE fmt NONE,##args)

// extern static bool acoustic_netconfig_flag;
int mozart_aec_start(void)
{	
	char loopback_buf[AEC_SIZE] = {};
	char dmic_buf[AEC_SIZE] = {};
	aec_stop_flag = false;
	aec_running = true;

	soundcard_init();

	//system("ls -l /proc/self/fd/");

	/* real aec start */
	aec_start();

	my_pr(LIGHT_PURPLE"===> AEC: Please wakeup me. \n");
	while (!aec_stop_flag) {
		soundcard_aec_record(loopback_buf, dmic_buf, AEC_SIZE);//保存录像和参考音数据，dmic_buf 录音音频
#ifdef DEBUG_RECORD //no define
		write(fd_dmic, dmic_buf, AEC_SIZE);
		write(fd_amic, loopback_buf, AEC_SIZE);
#endif	
		aec_feed(dmic_buf, loopback_buf, AEC_SIZE);//做aec后向内核送音频
	}
	my_pr(LIGHT_PURPLE"RRRRRRRR  Listen Over \n");
#ifdef DEBUG_RECORD // no define

	finally_set_wav_data_size_amic(name1,fd_amic);
	finally_set_wav_data_size_dmic(name2,fd_dmic);
	close(fd_amic);
	close(fd_dmic);
	fd_amic = -1;
	fd_dmic = -1;
	file_count++;
#endif
	soundcard_uninit();
	/* real aec stop */
	aec_stop();
	aec_running = false;
	return 0;
}

int mozart_aec_stop(void)
{	printf("********mozart_aec_stop*********\n");
	aec_stop_flag = true;

	return 0;
}

int mozart_vr_aec_wakeup(void)
{
	if (!vr_init_flag || !vr_working_flag) {
		printf("warning: vr not init or not start, %s fail.\n", __func__);
		return -1;
	}

	aec_key_wakeup();

	return 0;
}
