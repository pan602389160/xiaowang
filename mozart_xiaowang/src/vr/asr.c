#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <pthread.h>

//#include "../main/include/mozart_key_function.h"

#include "record_interface.h"
#include "utils_interface.h"
#include "vr_interface.h"
#include "interf_enc.h"
#include "tips_interface.h"
#include "enc_if.h"
#include "speex_resampler.h"
#include "aec.h"

bool acoustic_netconfig_flag = false;
static bool speak_start_flag = false;
//static bool speak_stop_flag = false;
static bool asr_stop_flag = false;
static bool asr_break_flag = false;
static mic_record *record = NULL;
bool asr_stopped = false;

/* vr init flag */
extern bool vr_init_flag;

/* vr working flag, change on vr_start() & vr_stop() */
extern bool vr_working_flag;

/* record current vr status */
extern vr_status_t vr_status;
//#define DEBUG_RECORD 1


//static int file_count = 0;
char name1[32],name2[32];
static int fd_dmic;
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

static void finally_set_wav_data_size_dmic(char *filename, int fd)
{
	int size = 0;
	wav_header wave;
	size = get_file_size(filename)-44;
	init_wave_head(&wave, ASR_RATE, ASR_BIT, ASR_CHANNEL, size);
	set_wave_head(fd, &wave);
}

static void finally_set_wav_data_size_dmic_8K(char *filename, int fd)
{
	int size = 0;
	wav_header wave;
	size = get_file_size(filename)-44;
	init_wave_head(&wave, 8000, ASR_BIT, ASR_CHANNEL, size);
	set_wave_head(fd, &wave);
}


static void set_record_file_head(int name)
{	
	wav_header *wave;
	if(name == 1){
		sprintf(name2,"/tmp/asr_dmic_16.wav");
		wave = malloc(sizeof(wav_header));
		fd_dmic = open(name2, O_RDWR | O_CREAT | O_TRUNC);
		init_wave_head(wave, ASR_RATE, ASR_BIT, ASR_CHANNEL, 0);
		set_wave_head(fd_dmic, wave);
	}
	if(name == 2){
		sprintf(name2,"/tmp/speak_16.wav");
		wave = malloc(sizeof(wav_header));
		fd_dmic = open(name2, O_RDWR | O_CREAT | O_TRUNC);
		init_wave_head(wave, ASR_RATE, ASR_BIT, ASR_CHANNEL, 0);
		set_wave_head(fd_dmic, wave);
	}
	free(wave);
}


int fd_asr;
//extern int fd;

#define HT1628_IOC_MAGIC  	'H'
#define HT1628_IOC_MAXNR 	6

#include <sys/ioctl.h> 


#define INNO_IOCTL_HT1628_DISPLAY_OFF     _IOW(HT1628_IOC_MAGIC, 3, int)
#define INNO_IOCTL_HT1628_SEND_BYTE_DATA     _IOW(HT1628_IOC_MAGIC, 4, int)
#define INNO_IOCTL_HI1628_STB_SET     _IOW(HT1628_IOC_MAGIC, 5, int)
#define INNO_IOCTL_KEYLED_BACKLIGHT_SET     _IOW(HT1628_IOC_MAGIC, 6, int)

#define FACE_LIKE 0x01
#define FACE_SMILE 0x02
#define FACE_SHY 0x03
#define FACE_THINK 0x04
#define FACE_SURPRISE 0x05
#define FACE_ANGRY 0x06
#define FACE_SAD 0x07
#define FACE_EMBARRASSED 0x08
#define FACE_YUN 0x09
#define FACE_ 0x0a
#define FACE_NEXT 0x0b
#define FACE_PREVIOUS 0x0c
#define FACE_MUSIC 0x0d
#define FACE_LOCAL 0x0e

#define FACE_THINK_1 0x11
#define FACE_THINK_2 0x12
#define FACE_THINK_3 0x13
#define FACE_THINK_4 0x14


static int soundcard_init(void)
{
	int ret;
	record_param dmic_param = {
		.bits = ASR_BIT,
		.rates = ASR_RATE,
		.channels = ASR_CHANNEL,
		.volume = ASR_VOLUME,
	};

#ifdef DEBUG_RECORD
	set_record_file_head();
#endif
	if(speak_start_flag)
		set_record_file_head(2);
	if(acoustic_netconfig_flag)
		set_record_file_head(1);

	record = mozart_soundcard_init(dmic_param);
	printf("22222222222222222 aa\n");
	if (record == NULL) {
		printf("error: soundcard asr init fail.\n");
		ret = -1;
		goto err_init;
	}

	return 0;

err_init:
	mozart_soundcard_uninit(record);
	record = NULL;

	return ret;
}

static int soundcard_uninit(void)
{
	mozart_soundcard_uninit(record);
	record = NULL;

	return 0;
}

static unsigned long soundcard_asr_record(char *dmic_buf, unsigned long len)
{
	if (!record) {
		printf("ASR  error: Please init soundcard firstly!\n");
		return 0;
	}

	return mozart_record(record, dmic_buf, ASR_SIZE);
}

pthread_t check_status_pthread;
#define AMR_MAGIC_NUMBER "#!AMR\n"

typedef signed long long    int64_t;
typedef unsigned char       uint8_t;
typedef unsigned short      uint16_t;
typedef unsigned int        uint32_t;

#define TAG(a, b, c, d) (((a) << 24) | ((b) << 16) | ((c) << 8) | (d))

struct wav_reader {
	FILE *wav;
	uint32_t data_length;

	int format;
	int sample_rate;
	int bits_per_sample;
	int channels;
	int byte_rate;
	int block_align;
};

static uint32_t read_tag(struct wav_reader* wr) {
	uint32_t tag = 0;
	tag = (tag << 8) | fgetc(wr->wav);
	tag = (tag << 8) | fgetc(wr->wav);
	tag = (tag << 8) | fgetc(wr->wav);
	tag = (tag << 8) | fgetc(wr->wav);
	return tag;
}

static uint32_t read_int32(struct wav_reader* wr) {
	uint32_t value = 0;
	value |= fgetc(wr->wav) <<  0;
	value |= fgetc(wr->wav) <<  8;
	value |= fgetc(wr->wav) << 16;
	value |= fgetc(wr->wav) << 24;
	return value;
}

static uint16_t read_int16(struct wav_reader* wr) {
	uint16_t value = 0;
	value |= fgetc(wr->wav) << 0;
	value |= fgetc(wr->wav) << 8;
	return value;
}


void* wav_read_open(const char *filename) {
	struct wav_reader* wr = (struct wav_reader*) malloc(sizeof(*wr));
	long data_pos = 0;
	memset(wr, 0, sizeof(*wr));

	wr->wav = fopen(filename, "rb");
	if (wr->wav == NULL) {
		free(wr);
		return NULL;
	}

	while (1) {
		uint32_t tag, tag2, length;
		tag = read_tag(wr);
		if (feof(wr->wav))
			break;
		length = read_int32(wr);
		if (tag != TAG('R', 'I', 'F', 'F') || length < 4) {
			fseek(wr->wav, length, SEEK_CUR);
			continue;
		}
		tag2 = read_tag(wr);
		length -= 4;
		if (tag2 != TAG('W', 'A', 'V', 'E')) {
			fseek(wr->wav, length, SEEK_CUR);
			continue;
		}
		// RIFF chunk found, iterate through it
		while (length >= 8) {
			uint32_t subtag, sublength;
			subtag = read_tag(wr);
			if (feof(wr->wav))
				break;
			sublength = read_int32(wr);
			length -= 8;
			if (length < sublength)
				break;
			if (subtag == TAG('f', 'm', 't', ' ')) {
				if (sublength < 16) {
					// Insufficient data for 'fmt '
					break;
				}
				wr->format          = read_int16(wr);
				wr->channels        = read_int16(wr);
				wr->sample_rate     = read_int32(wr);
				wr->byte_rate       = read_int32(wr);
				wr->block_align     = read_int16(wr);
				wr->bits_per_sample = read_int16(wr);
				fseek(wr->wav, sublength - 16, SEEK_CUR);
			} else if (subtag == TAG('d', 'a', 't', 'a')) {
				data_pos = ftell(wr->wav);
				wr->data_length = sublength;
				fseek(wr->wav, sublength, SEEK_CUR);
			} else {
				fseek(wr->wav, sublength, SEEK_CUR);
			}
			length -= sublength;
		}
		if (length > 0) {
			// Bad chunk?
			fseek(wr->wav, length, SEEK_CUR);
		}
	}
	fseek(wr->wav, data_pos, SEEK_SET);
	return wr;
}

void wav_read_close(void* obj) {
	struct wav_reader* wr = (struct wav_reader*) obj;
	fclose(wr->wav);
	free(wr);
}


int wav_get_header(void* obj, int* format, int* channels, int* sample_rate, int* bits_per_sample, unsigned int* data_length) {
	struct wav_reader* wr = (struct wav_reader*) obj;
	if (format)
		*format = wr->format;
	if (channels)
		*channels = wr->channels;
	if (sample_rate)
		*sample_rate = wr->sample_rate;
	if (bits_per_sample)
		*bits_per_sample = wr->bits_per_sample;
	if (data_length)
		*data_length = wr->data_length;
	return wr->format && wr->sample_rate;
}


int wav_read_data(void* obj, unsigned char* data, unsigned int length) {
	struct wav_reader* wr = (struct wav_reader*) obj;
	int n;
	if (wr->wav == NULL)
		return -1;
	if (length > wr->data_length)
		length = wr->data_length;
	n = fread(data, 1, length, wr->wav);
	wr->data_length -= length;
	return n;
}


void _x_audio_out_resample_mono(short* input_samples, int in_samples,short* output_samples, int out_samples)
{
  int osample;
  /* 16+16 fixed point math */
  uint32_t isample = 0;
  uint32_t istep = ((in_samples-2) << 16)/(out_samples-2);

#ifdef VERBOSE
  printf ("Audio : resample %d samples to %d\n",
          in_samples, out_samples);
#endif

  for (osample = 0; osample < out_samples - 1; osample++) {
    int s1;
    int s2;
    int16_t os;
    uint32_t t = isample&0xffff;
   
    /* don't "optimize" the (isample >> 16)*2 to (isample >> 15) */
    s1 = input_samples[(isample >> 16)];
    s2 = input_samples[(isample >> 16)+1];
   
    os = (s1 * (0x10000-t)+ s2 * t) >> 16;
    output_samples[osample] = os;

    isample += istep;
  }
  output_samples[out_samples-1] = input_samples[in_samples-1];
}


#if 0
int wav_to_amr()
{
	printf("----------wav_to_amr----------------\n");
	int mode = 8;
	int dtx = 0;
	const char *infile, *outfile;
	FILE* out;
	void *wav, *amr;
	int format, sampleRate, channels, bitsPerSample;
	int inputSize;
	uint8_t* inputBuf;

	infile = "/usr/data/asr_dmic.wav";
	outfile ="/usr/data/asr_dmic.amr";

	wav = wav_read_open(infile);
	if (!wav) {
		fprintf(stderr, "Unable to open wav file %s\n", infile);
		return 1;
	}
	if (!wav_get_header(wav, &format, &channels, &sampleRate, &bitsPerSample, NULL)) {
		fprintf(stderr, "Bad wav file %s\n", infile);
		return 1;
	}
	if (format != 1) {
		fprintf(stderr, "Unsupported WAV format %d\n", format);
		return 1;
	}
	if (bitsPerSample != 16) {
		fprintf(stderr, "Unsupported WAV sample depth %d\n", bitsPerSample);
		return 1;
	}
	if (channels != 1)
		fprintf(stderr, "Warning, only compressing one audio channel\n");
	if (sampleRate != 16000)
		fprintf(stderr, "Warning, AMR-WB uses 16000 Hz sample rate (WAV file has %d Hz)\n", sampleRate);
	inputSize = channels*2*320;
	inputBuf = (uint8_t*) malloc(inputSize);
	amr = E_IF_init();
	out = fopen(outfile, "wb");
	if (!out) {
		perror(outfile);
		return 1;
	}

	fwrite("#!AMR-WB\n", 1, 9, out);
	while (1) {
		int read, i, n;
		short buf[320];
		uint8_t outbuf[500];

		read = wav_read_data(wav, inputBuf, inputSize);
		read /= channels;
		read /= 2;
		if (read < 320)
			break;
		for (i = 0; i < 320; i++) {
			const uint8_t* in = &inputBuf[2*channels*i];
			buf[i] = in[0] | (in[1] << 8);
		}
		n = E_IF_encode(amr, mode, buf, outbuf, dtx);
		fwrite(outbuf, 1, n, out);
	}
	free(inputBuf);
	fclose(out);
	E_IF_exit(amr);
	wav_read_close(wav);
	return 0;
}
#else


int wav_to_amr()
{	printf("==============wav_to_amr=============\n");
	enum Mode mode = MR122;

	const char *infile, *outfile;
	FILE *out;
	void *wav, *amr;
	int format, sampleRate, channels, bitsPerSample;
	int inputSize;
	uint8_t* inputBuf;
	infile ="/tmp/speak_8.wav";
	outfile ="/tmp/speak_8.amr";
	wav = wav_read_open(infile);
	if (!wav) {
		fprintf(stderr, "Unable to open wav file %s\n", infile);
		return 1;
	}
	if (!wav_get_header(wav, &format, &channels, &sampleRate, &bitsPerSample, NULL)) {
		fprintf(stderr, "Bad wav file %s\n", infile);
		return 1;
	}
	printf("format = %d,channels = %d,sampleRate = %d,bitsPerSample = %d\n",format,channels,sampleRate,bitsPerSample);

	
	if (format != 1) {
		fprintf(stderr, "Unsupported WAV format %d\n", format);
		return 1;
	}
	if (bitsPerSample != 16) {
		fprintf(stderr, "Unsupported WAV sample depth %d\n", bitsPerSample);
		return 1;
	}
	if (channels != 1)
		fprintf(stderr, "Warning, only compressing one audio channel\n");
	if (sampleRate != 8000)
		fprintf(stderr, "Warning, AMR-NB uses 8000 Hz sample rate (WAV file has %d Hz)\n", sampleRate);
	inputSize = channels*2*160;
	inputBuf = (uint8_t*) malloc(inputSize);
	amr = Encoder_Interface_init(1);

	out = fopen(outfile, "wb");
	if (!out) {
		perror(outfile);
		return 1;
	}

	fwrite("#!AMR\n", 1, 6, out);
	short buf[160];
	uint8_t outbuf[500];
	while (1) {
		memset(buf,0,sizeof(buf));
		memset(outbuf,0,sizeof(outbuf));
		int read, i, n;
		read = wav_read_data(wav, inputBuf, inputSize);
		read /= channels;
		read /= 2;
		if (read < 160)
			break;
		for (i = 0; i < 160; i++) {
			const uint8_t* in = &inputBuf[2*channels*i];
			buf[i] = in[0] | (in[1] << 8);
		}
		n = Encoder_Interface_Encode(amr, mode, buf, outbuf, 0);
		fwrite(outbuf, 1, n, out);
	}
	free(inputBuf);
	fclose(out);
	Encoder_Interface_exit(amr);
	wav_read_close(wav);

	return 0;
}

#endif




int mozart_asr_start(bool sds)
{
	printf("---------mozart_asr_start----------sds: %d,speak_start_flag : %d\n",sds,speak_start_flag);
	int ret = -1;
	char dmic_buf[ASR_SIZE] = {};
	asr_stop_flag = false;
	asr_break_flag = false;
	asr_stopped = false;
	system("rm /usr/data/*.wav");
	system("rm /usr/data/*.amr");
	
	ret = soundcard_init();
	printf("11 record = %p\n",record);
	if(ret != 0){
		printf("soundcard_init error :ret = %d!!\n",ret);
		return -1;
	}
	/* real asr start */
	if(speak_start_flag)
		asr_start(false);
	else
		asr_start(true);
	printf("===============================> ASR: What can I do for you?\n");


	int wav_fd = open("/tmp/speak_8.wav", O_RDWR | O_CREAT | O_TRUNC);
	SpeexResamplerState *resampler;
	int sr = 16000;
	int target_sr = 8000;
	unsigned int  out_len = 1024*4;
	unsigned int in_len = 0;
	unsigned int in_len_x = 0;
	int err = 0;
	resampler = speex_resampler_init(1, sr, target_sr, 5, &err);//\B3\F5ʼ\BB\AF  
	speex_resampler_skip_zeros(resampler); 
	short out_buf[ASR_SIZE];
	

	printf("@@@ mozart_vr_get_status = %d\n",mozart_vr_get_status());
	while (!asr_stop_flag && !asr_break_flag) {
		memset(out_buf,0,sizeof(out_buf));
		soundcard_asr_record(dmic_buf, ASR_SIZE);

		#ifdef DEBUG_RECORD
			write(fd_dmic, dmic_buf, ASR_SIZE);
		#endif
		
		if(speak_start_flag){
			in_len = write(fd_dmic, dmic_buf, ASR_SIZE);
			in_len_x = in_len/2;
			speex_resampler_process_int(resampler, 0, (spx_int16_t *)dmic_buf, &in_len_x,out_buf, &out_len); 
			write(wav_fd, out_buf,2*out_len );
		}
		asr_feed(dmic_buf, ASR_SIZE);
	

	}
	printf("===============================RECORD OVER> \n");

	if(speak_start_flag){
		finally_set_wav_data_size_dmic(name2,fd_dmic);
		close(fd_dmic);
		fd_dmic = -1;	
	}
	speex_resampler_destroy(resampler);
	finally_set_wav_data_size_dmic_8K("speak_8.wav",wav_fd);
	wav_fd = -1;
	
#ifdef DEBUG_RECORD
	finally_set_wav_data_size_dmic(name2,fd_dmic);
	close(fd_dmic);
	fd_dmic = -1;
	file_count++;
#endif
	close(wav_fd);
	if(speak_start_flag)
		wav_to_amr();
	soundcard_uninit();
	
	if (asr_stop_flag) {
		/* real asr stop */
		asr_stop();
	}
	asr_stopped = true;
	
	if(speak_start_flag){
			printf("?????????????stop?????????????????????\n");
			usleep(500*1000);	
			mozart_vr_asr_break();
			mozart_stop_tone_sync();
			//mozart_play_pause();
			speak_start_flag = false;
			printf("@@@@@@@@@@@@@@stop@@@@@@@@@@@@@@@@@\n");
	}
	

	return 0;
}
void *acoustic_netconfig_start()
{
	if(access("/tmp/speak_16.wav",F_OK) == 0)	
		system("rm /tmp/speak_16.wav");
	acoustic_netconfig_flag = true;
	int ret = -1;
	long long int size_in = 0;
	char dmic_buf[ASR_SIZE] = {};
	printf("1111111111111111111111  aa\n");
	ret = soundcard_init();

	if(ret != 0){
		printf("soundcard_init error :ret = %d!!\n",ret);
		return NULL;
	}
	
	while (1) {
		printf("size_in:%lld\n",size_in);
		if(size_in >= 700*1000||acoustic_netconfig_flag == false)
			break;
		soundcard_asr_record(dmic_buf, ASR_SIZE);

		if(acoustic_netconfig_flag){
			ret = write(fd_dmic, dmic_buf, ASR_SIZE);
			if(ret < 0)
				break;
			size_in += ret;
		}

		asr_feed(dmic_buf, ASR_SIZE);
	}



	finally_set_wav_data_size_dmic(name2,fd_dmic);
	close(fd_dmic);
	fd_dmic = -1;

	printf("acoustic_netconfig_start=====================================================================RECORD OVER> \n");
	acoustic_netconfig_flag = false;
	soundcard_uninit();

	return NULL;
}
void *h5_speak_start()
{
	printf("====================h5_speak_start=========================\n");

	speak_start_flag = true;

	aec_key_wakeup();

	return NULL;
}

int mozart_asr_stop(int reason)
{
	if (reason == 1)
		asr_break_flag = true;
	else
		asr_stop_flag = true;

	return 0;
}

int mozart_vr_asr_break(void)
{
	if (!vr_init_flag || !vr_working_flag) {
		printf("warning: vr not init or not start, %s fail.\n", __func__);
		return -1;
	}
	vr_status = VR_IDLE;
	/* break asr process */
	asr_break();
	/* waiting for asr stopped */
	while (!asr_stopped)
		usleep(10 * 1000);
	/* close asr connection */
	asr_cancel();
	return 0;
}
