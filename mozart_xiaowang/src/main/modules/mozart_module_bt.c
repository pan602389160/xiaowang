#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <linux/soundcard.h>
#include <string.h>
#include <inttypes.h>
#include <sys/mman.h>

#include "utils_interface.h"
#include "sharememory_interface.h"
#include "resample_interface.h"
#include "channels_interface.h"
#include "webrtc_aec.h"
#include "bluetooth_interface.h"
#include "mozart_config.h"
#include "localstream_interface.h"
#include "localstream_fmt.h"
#include "modules/mozart_module_bt.h"
#include "player_interface.h"
#include "ini_interface.h"
#include "volume_interface.h"
#include "json-c/json.h"
#if (SUPPORT_BSA_BLE_HH == 1)
int hh_client_num = -1;
#endif

#if (SUPPORT_BSA_BLE_HH_DIALOG_AUDIO == 1)
#define wavOutFile_path 		"/mnt/sdcard/bleaudio.wav"
#define WAV_ID_RIFF			0x46464952 /* "RIFF" */
#define WAV_ID_WAVE			0x45564157 /* "WAVE" */
#define WAV_ID_FMT			0x20746d66 /* "fmt " */
#define WAV_ID_DATA			0x61746164 /* "data" */
#define WAV_ID_PCM			0x0001
#define WAV_ID_FLOAT_PCM		0x0003
#define WAV_ID_FORMAT_EXTENSIBLE 	0xfffe

typedef signed long long    int64_t;
typedef unsigned char       uint8_t;
typedef unsigned short      uint16_t;
typedef unsigned int        uint32_t;

static int dia_FilterCoefs[] = {
	-215, -236, 326, 1236, 1451, 471, -610, -373, 727, 770,
	-599, -1189, 380, 1816, 150, -2856, -1557, 5926, 13522, 13522,
	5926, -1557, -2856, 150, 1816, 380, -1189, -599, 770, 727,
	-373, -610, 471, 1451, 1236, 326, -236, -215
};

static int dia_indexTable[] = {
	-1, -1, -1, -1, 2, 4, 6, 8,
	-1, -1, -1, -1, 2, 4, 6, 8
};

static int dia_stepSizeTable[] = {
	7, 8, 9, 10, 11, 12, 13, 14,
	16, 17, 19, 21, 23, 25, 28, 31, 34, 37, 41, 45, 50, 55, 60,
	66, 73, 80, 88, 97, 107, 118, 130, 143, 157, 173, 190, 209,
	230, 253, 279, 307, 337, 371, 408, 449, 494, 544, 598, 658,
	724, 796, 876, 963, 1060, 1166, 1282, 1411, 1552, 1707, 1878,
	2066, 2272, 2499, 2749, 3024, 3327, 3660, 4026, 4428, 4871,
	5358, 5894, 6484, 7132, 7845, 8630, 9493, 10442, 11487, 12635,
	13899, 15289, 16818, 18500, 20350, 22385, 24623, 27086, 29794,
	32767
};

extern BOOLEAN ble_client_connect_state;
extern int mozart_bluetooth_ble_client_connect_server(ble_client_connect_data *cl_connect_data);
/* Settings for Remote Control. Note instance is (report number - 1) */
static int  HID_STREAM_ENABLE_WRITE_INSTANCE		= 3;
static int  HID_STREAM_ENABLE_READ_INSTANCE		= 4;
static int  HID_STREAM_DATA_MIN_INSTANCE		= 5;
static int  HID_STREAM_DATA_MAX_INSTANCE		= 7;
static int  dia_imaIndex = 0;
static short dia_imaPredictedSample = 0;
static int dia_imaOr = 0;
static int DIALOG_FILT_LEN = 38;
static int dia_imaAnd = 0xF;
static int dia_imaSize = 4;
static int dia_blockLen = 40;
static int dia_prevInstance = 0;
static BOOLEAN dialog_doLiveAudio = true;
static BOOLEAN dia_doStoreAudio = false;
static BOOLEAN dia_doSpeechRecognition = false;
static BOOLEAN dia_doupSample = false;
static int dia_dsp_fd = -1;
static int dia_wav_fd = -1;
static int dia_nbytes = 0;

/*
 *  Set the Data Rate
 *  0: 64 Kbit/s = ima 4Bps, 16 Khz.
 *  1: 48 Kbit/s = ima 3Bps, 16 Khz.
 *  2: 32 Kbit/s = ima 2Bps, 16 Khz.
 *  3: 32 Kbit/s = ima 4Bps, 8 Khz (downsample).
 *  4: 24 Kbit/s = ima 3Bps, 8 Khz (downsample).
 */
static void dialog_setDecodeMode(int mode)
{
	dia_doupSample = false;

	/* 4 bits per pixel */
	if (mode == 0) 	{
		dia_imaOr = 0;
		dia_imaAnd = 0x0F;
		dia_imaSize = 4;
		dia_blockLen = 40;
	/* 3 bits per pixel */
	} else if (mode == 1) {
		dia_imaOr = 1;
		dia_imaAnd = 0x0E;
		dia_imaSize = 3;
		dia_blockLen = 53;
	/* 8Khz, 4 bits per pixel */
	} else if (mode == 2) {
		dia_imaOr = 0;
		dia_imaAnd = 0x0F;
		dia_imaSize = 4;
		dia_doupSample = true;
		dia_blockLen = 80;
	/* 8Khz, 3 bits per pixel */
	} else if (mode == 3) {
		dia_imaOr = 1;
		dia_imaAnd = 0x0E;
		dia_imaSize = 3;
		dia_doupSample = true;
		dia_blockLen = 106;
	}
	dia_imaIndex = 0;
	dia_imaPredictedSample = 0;
	printf("Codec new Mode: %d, dia_doupSample: %d, dia_imaSize: %d\n", mode, dia_doupSample, dia_imaSize);
}

static int config_device(int fd, int format, int channels, int speed)
{
	int ioctl_val;

	/* NOTE: config order is important: format, channel, speed */
	/* set bit format */
	ioctl_val = format;
	if (ioctl(fd, SNDCTL_DSP_SETFMT, &ioctl_val) == -1) {
		fprintf(stderr, "set format failed: %s\n",
				strerror(errno));
		return -1;
	}
	if (ioctl_val != format) {
		fprintf(stderr, "format not supported, changed by driver to 0x%08x\n",
				ioctl_val);
		return -1;
	}

	/*set channel */
	ioctl_val = channels;
	if ((ioctl(fd, SNDCTL_DSP_CHANNELS, &ioctl_val)) == -1) {
		fprintf(stderr, "set channels failed: %s\n",
				strerror(errno));
		return -1;
	}
	if (ioctl_val != channels) {
		fprintf(stderr, "%d channel is not supported, changed to %d by driver\n",
				channels, ioctl_val);
		return -1;
	}

	/*set speed */
	ioctl_val = speed;
	if (ioctl(fd, SNDCTL_DSP_SPEED, &ioctl_val) == -1) {
		fprintf(stderr, "set speed failed: %s\n",
				strerror(errno));
		return -1;
	}
	if (ioctl_val != speed) {
		fprintf(stderr, "speed %d is not supported, changed to %d by driver\n",
				speed, ioctl_val);
		return -1;
	}

	return 0;
}

static void hh_fput16le(uint16_t val, int fd)
{
	int ret = 0;
	uint8_t bytes[2] = {val, val >> 8};

	ret = write(fd, bytes, 2);
	if (ret != 2) {
		printf("Warinig: hh_fput16le write num %d not equa to return num %d!!\n", 2, ret);
	}
}

static void hh_fput32le(uint32_t val, int fd)
{
	int ret = 0;
	uint8_t bytes[4] = {val, val >> 8, val >> 16, val >> 24};

	ret = write(fd, bytes, 4);
	if (ret != 4) {
		printf("Warinig: hh_fput32le write num %d not equa to return num %d!!\n", 4, ret);
	}
}

static void hh_write_wave_header(int fd, uint32_t data_length)
{
	uint32_t fmt_chunk_size = 0x00000010;
	int samplerate = 16000;
	int channels = 0x0001;
	int format = 16;
	int bps = channels * samplerate * (format / 8);

	/* Master RIFF chunk */
	hh_fput32le(WAV_ID_RIFF, fd);
	/* RIFF chunk size: 'WAVE' + 'fmt ' + 4 + fmt_chunk_size + data chunk hdr (8) + data length */
	hh_fput32le((36 + data_length), fd);
	hh_fput32le(WAV_ID_WAVE, fd);	/* WAVE */

	/* Format chunk */
	hh_fput32le(WAV_ID_FMT, fd);	/* fmt */
	hh_fput32le(fmt_chunk_size, fd);   /* Subchunksize = 16a ->should be modified later.. */
	hh_fput16le(WAV_ID_PCM, fd);		/* PCM */
	hh_fput16le(channels, fd);		/* Mono */

	hh_fput32le(samplerate, fd);	/* Samplerate = 16000 HZ */
	hh_fput32le(bps, fd); 		/* ByteRate = channels * SampleRate * format/8 */
	hh_fput16le(channels * (format / 8), fd); /* BlockAlign */
	hh_fput16le(format, fd);			/* BitsPerSample */

	/* Data chunk */
	hh_fput32le(WAV_ID_DATA, fd);		/* DATA */
	hh_fput32le(data_length, fd);		/* SubChunk2size = dia_nbytes -> should be modified later.. */
}

static int audio_store_start()
{
	dia_wav_fd = open(wavOutFile_path, O_WRONLY|O_CREAT|O_TRUNC, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
	if (dia_wav_fd < 0) {
		printf("open wavfile (%s) error: %s.\n", wavOutFile_path, strerror(errno));
		return -1;
	}
	dia_nbytes = 0;  // Clear the buffer...
	hh_write_wave_header(dia_wav_fd, 0x800);

	return 0;
}

static void dialog_streamSetOnOff(BOOLEAN enable)
{
	int ret = 0, i = 0;
	ble_client_write_data cl_wdata;

	if (enable) {
		printf("Stream On !!\n");
		if (dia_doStoreAudio) {
			ret = audio_store_start();
			if (ret == -1) {
				printf("audio_store_start Failed !!\n");
			}
		}
		if (dia_doSpeechRecognition) {
			//speechRecGoogle.start();
		}
		dia_imaIndex = 0;
		dia_imaPredictedSample = 0;

		/* play audio pcm use dsp */
		if (dialog_doLiveAudio)
		{
			if (dia_dsp_fd == -1) {
				char buf[16] = {};
				int volume = 0;
				int hh_sample_rate = 16000;
				int hh_sample_bit = 16;
				int hh_sample_channel = 1;
				/* Parse playback dsp device. */
				if (mozart_ini_getkey("/usr/data/system.ini", "audio", "dev_playback", buf)) {
					printf("[app_avk - OSS] Can't get playback dsp device, force to /dev/dsp.\n");
					strcpy(buf, "/dev/dsp");
				}

				dia_dsp_fd = open(buf, O_WRONLY);
				if (dia_dsp_fd < 0) {
					printf("avk: open play dsp(%s) error: %s.\n", buf, strerror(errno));
					return;
				} else {
					if (mozart_ini_getkey("/usr/data/system.ini", "volume", "bt_music", buf)) {
						printf("failed to parse /usr/data/system.ini, set BT music volume to 60.\n");
						volume = 60;
					} else {
						volume = atoi(buf);
					}
					mozart_volume_set(volume, MUSIC_VOLUME);
					config_device(dia_dsp_fd, hh_sample_bit, hh_sample_channel, hh_sample_rate);
				}
			}
		}
	} else {
		printf("Stream Off !!\n");
		if (dia_doStoreAudio)
		{
			if (lseek(dia_wav_fd, 0, SEEK_SET) == 0) {
				hh_write_wave_header(dia_wav_fd, dia_nbytes);
				close(dia_wav_fd);
				dia_wav_fd = -1;
				printf("Close wav file, dia_nbytes is %d\n", dia_nbytes);
			} else {
				printf("Could not seek to start, WAV size headers not updated! \n");
				close(dia_wav_fd);
				close(dia_dsp_fd);
				dia_dsp_fd = -1;
				dia_wav_fd = -1;

				return;
			}
		}
		if (dia_doSpeechRecognition)
		{
			//speechRecGoogle.stop();
		}
		/* audioDecoder.stop(); */
		if (dialog_doLiveAudio)
		{
			close(dia_dsp_fd);
			dia_dsp_fd = -1;
		}
		printf("dia_nbytes = %d\n", dia_nbytes);
	}
	dia_prevInstance = 0;
	/* write enable to hid device */

	printf("enable = %d\n", enable);
	/* write connected ble device */
	cl_wdata.service_uuid = 0x1812;
	cl_wdata.char_uuid = 0x2a4d;
	cl_wdata.write_len = 20;
	cl_wdata.write_data[0] = enable;
	for (i = 1; i < cl_wdata.write_len; i++) {
		cl_wdata.write_data[i] = 0x00;
	}
	cl_wdata.client_num = hh_client_num;
	cl_wdata.is_primary = 1;
	cl_wdata.ser_inst_id = 0;
	cl_wdata.char_inst_id = HID_STREAM_ENABLE_WRITE_INSTANCE;
	cl_wdata.is_descript = 0;
	cl_wdata.descr_id = 0;
	cl_wdata.desc_value = 0;
	cl_wdata.write_type = 2;

	ret = mozart_bluetooth_ble_client_write(&cl_wdata);
	if (ret != 0) {
		printf("mozart_bluetooth_ble_client_write failed !\n");
		return;
	}
}

static void dialog_updatekeys(UINT8 *packet)
{
	int type = (int)packet[1];
	int i = 0;

	printf("Key Update type: %d\n", type);
	if (type == 0) {
		BOOLEAN enable = (packet[0] != 0);
		dialog_streamSetOnOff(enable);
	} else if (type == 2) {
		//keyReport.displayKeyPress(packet);
	} else if (type == 3) {
		int fifo_err = (int) packet[5];
		int spi_err  = (int) packet[6];
		int fifo_sz = ((int)packet[9]) & 0x000000FF;
		int wr = ((int)packet[7]) & 0x000000FF;
		int rd = ((int)packet[8]) & 0x000000FF;

		printf("Fifo error %d,  spi %d,  sz %d, wr %d, rd %d \n", fifo_err, spi_err, fifo_sz, rd, wr);
	} else if (type == 4) {
		int mode = (int) packet[5];
		int spi_sz = ((int)packet[6]) & 0x000000FF;
		int fifo_sz = ((int)packet[7]) & 0x000000FF;

		printf("Ima Mode Change %d, fsz %d, spi %d \n", mode, fifo_sz, spi_sz);
		for (i = 10; i < 20; i++) {
			int v = ((int)packet[i]) & 0x000000FF;
			printf("ImaMode = %d\n", v);
		}
	}
}

static void dialog_DecodeImaAdpcm(int len, UINT8 *inpBytes, UINT16 data_len, short *outSamples)
{
	int i;
	int predictedSample = (int)dia_imaPredictedSample;
	int index = dia_imaIndex;
	int stepSize;
	int inpBuf = 0;
	int inpMsb = 0;
	int inpIdx = 0;
	int shift = 4 - dia_imaSize;
	int bsize = data_len;

	for (i = 0; i < len; i++)
	{
		stepSize = dia_stepSizeTable[index];   /* Find new quantizer stepsize */
		int inp;

		/**
		 ** Unpack the bytes from 2 bytes (1 word)
		 */
		if (inpMsb < 8)
		{
			inpBuf = (int)(inpBuf << 8) & 0xFFFF;
			if (inpIdx < bsize)  inpBuf |= ((int)inpBytes[inpIdx++] & 0x00FF);
			inpMsb += 8;
		}
		inpMsb -= dia_imaSize;
		inp = ((inpBuf >> inpMsb) << shift ) | dia_imaOr;
		inp = inp & 0x0F;

		/* Compute predicted sample estimate                 */
		/* This part is different from regular IMA-ADPCM     */
		/* The prediction is calculated with full precision  */
		int diff = stepSize;  /* the 4 is for rounding */
		if ((inp & 4) != 0) {
			diff += (int)(stepSize << 3);
		}
		if ((inp & 2) != 0) {
			diff += (int)(stepSize << 2);
		}
		if ((inp & 1) != 0) {
			diff += (int)(stepSize << 1);
		}
		diff >>= 3;

		/* Adjust predictedSample based on calculated difference */
		//predictedSample += diff;
		if ((inp & 8) != 0)
			predictedSample -= diff;
		else
			predictedSample += diff;

		/* Saturate if there is overflow */
		if (predictedSample > 32767)
			predictedSample = 32767;
		if (predictedSample < -32768)
			predictedSample = -32768;

		/* 16-bit output sample can be stored at this point */
		outSamples[i] = (short)predictedSample;

		/* Compute new stepsize */
		/* Adjust index into dia_stepSizeTable using newIMA */
		index += dia_indexTable[inp];
		if (index < 0) index = 0;
		if (index > 88) index = 88;
	}
	dia_imaIndex = index;
	dia_imaPredictedSample = (short)predictedSample;
}

static short dialog_rndSat(int acc)
{
	if (acc > 32767)
		acc = 32767;
	else if (acc < -32768)
		acc = -32768;

	return (short)acc;
}

static void dialog_upSample(int len, short *inpSamples, short *outSamples)
{
	int i, j;
	short *filterTaps = malloc(38 * sizeof(short));

	for (i = 0; i < len; i++)
	{
		int acc1 = 0;
		int acc2 = 0;
		for (j = 0; j < (DIALOG_FILT_LEN / 2) - 1; j++) {
			acc1 += (int)dia_FilterCoefs[2*j] * (int)filterTaps[j];
			acc2 += (int)dia_FilterCoefs[2*j + 1] * (int)filterTaps[j];
			filterTaps[j] = filterTaps[j + 1];
		}
		filterTaps[(DIALOG_FILT_LEN/2) - 1] = inpSamples[i];
		acc1 += (int)dia_FilterCoefs[DIALOG_FILT_LEN - 2] * (int)filterTaps[(DIALOG_FILT_LEN/2) - 1];
		acc2 += (int)dia_FilterCoefs[DIALOG_FILT_LEN - 1] * (int)filterTaps[(DIALOG_FILT_LEN/2) - 1];
		outSamples[2*i] = (short)dialog_rndSat(acc2 >> 15);
		outSamples[2*i + 1] = (short)dialog_rndSat(acc1 >> 15);
	}
	free(filterTaps);
}

static void audio_decoder_process(UINT8 *inpBytes, UINT16 data_len, short *outSamples)
{
	if (dia_doupSample)
	{
		short *tmpSamples = malloc(dia_blockLen / 2 * (sizeof(short)));
		dialog_DecodeImaAdpcm(dia_blockLen / 2, inpBytes, data_len, tmpSamples);
		dialog_upSample(dia_blockLen / 2, tmpSamples, outSamples);
		free(tmpSamples);
	} else {
		dialog_DecodeImaAdpcm(dia_blockLen, inpBytes, data_len, outSamples);
	}
}

static void audio_addSampleData(short *samples)
{
	int ret = 0;
	int samples_length = dia_blockLen * sizeof(short);

	if (dialog_doLiveAudio)
	{
		if (dia_dsp_fd != -1) {
			ret = write(dia_dsp_fd, (char *)samples, samples_length);
			if (ret != samples_length) {
				printf("Warinig: audio_addSampleData write num %d not equa to return num %d!!\n", 4, ret);
			}
		}
	}

	if (dia_doStoreAudio)
	{
		dia_nbytes += samples_length;
		ret = write(dia_wav_fd, (char *)samples, samples_length);
	}
	if (dia_doSpeechRecognition)
	{
		//speechRecGoogle.addSampleData(samples);
	}
}

static void dialog_sendAudioData(UINT8 *data, UINT16 data_len)
{
	short *outSamples = malloc(dia_blockLen * sizeof(short));

	audio_decoder_process(data, data_len, outSamples);
	audio_addSampleData(outSamples);
	free(outSamples);
}

static int dialog_init()
{
	dia_imaIndex = 0;
	dia_imaPredictedSample = 0;
	dialog_setDecodeMode(0);  // Default mode

	return 0;
}
#endif /* SUPPORT_BSA_BLE_HH_DIALOG_AUDIO */

#if (SUPPORT_WEBRTC == 1)
bt_aec_callback bt_ac;
#endif

#if (SUPPORT_BT == BT_BCM)
extern int bsa_start_regular_enable;
extern int bt_link_state;
#endif

#if ((SUPPORT_BSA_HS_RESAMPLE == 1) || (SUPPORT_AEC_RESAMPLE == 1))
af_resample_t* hs_resample_s;
hs_sample_init_data hs_sample_data = {0};
hs_resample_init_data hs_resample_data = {
	.resample_rate = 0,
	.resample_bits = 0,
	.resample_channel = 0,
	.resample_enable = 0,
	.mozart_hs_resample_data_cback = NULL,
};
#endif

#if (SUPPORT_AEC_RESAMPLE == 1)
af_resample_t* aec_resample_s;

bt_aec_sample_init_data bt_aec_sdata = {0};
bt_aec_resample_init_data bt_aec_rdata = {
	.resample_rate = 0,
	.resample_bits = 0,
	.resample_channel = 0,
	.resample_enable = 0,
	.resample_time = 0,
	.aec_resample_data_cback = NULL,
};
#endif /* SUPPORT_AEC_RESAMPLE */

extern int bluetooth_cancel_auto_reconnect_pthread();

#if (SUPPORT_BSA_SPPS == 1)
static int mozart_bluetooth_dg_uipc_cback(UINT16 event, UINT8 *buf, UINT32 length)
{
	int i = 0;
	if (event == UIPC_RX_DATA_READY_EVT) {
		printf("length = %d\n", length);
		for (i = 0; i < length; i++) {
			printf("0x%02x ", buf[i]);
		}
		printf("\n");
	}

	return 0;
}
#endif /* SUPPORT_BSA_SPPS */

#if (SUPPORT_BSA_SPPC == 1)
enum {
	DG_IDLE,
	DG_CONNECTED,
	DG_CONNECT_FAILED,
	DG_DISCONNECTED,
};
extern int bt_dg_flag;
BOOLEAN sdpu_is_base_uuid(char *uuid)
{
	UINT16 i;
	static const char sdp_base_uuid[] = {0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x10, 0x00, 0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB};

	for (i = 4; i < MAX_UUID_SIZE; i++) {
		if (uuid[i] != sdp_base_uuid[i]) {
			return (false);
		}
	}
	/* If here, matched */
	return (true);
}
#endif

#if (SUPPORT_BSA_BLE == 1)
extern int bsa_ble_start_regular_enable;
extern int ble_service_create_enable;
extern int ble_service_add_char_enable;
extern int ble_service_start_enable;
extern int ble_client_open_enable;
int ble_client_read_enable;
extern int ble_client_write_enable;
extern int ble_hh_add_dev_enable;
ble_create_service_data ble_heart_rate_service;
ble_add_char_data ble_heart_rate_measure_char;
ble_add_char_data ble_notify_char;
ble_add_char_data ble_netconfig_char;

void *my_send()
{
	UINT8 value[BSA_BLE_MAX_ATTR_LEN];
	int server_num = 0;
	int char_attr_num = 0;
	while(1){
		sleep(1);
		printf("send ...............................................\n");
	

		ble_server_indication ble_indication;
		printf("???  ble_heart_rate_service.server_num = %d\n",ble_heart_rate_service.server_num);
		printf("???  ble_heart_rate_measure_char.char_attr_num= %d\n",ble_heart_rate_measure_char.char_attr_num);
		ble_indication.server_num = ble_heart_rate_service.server_num;
		ble_indication.attr_num = ble_heart_rate_measure_char.char_attr_num;
		ble_indication.need_confirm = 1;

		server_num = ble_heart_rate_service.server_num;
		char_attr_num = ble_heart_rate_measure_char.char_attr_num;

		value[0] = 0x12;
		value[1] = 0x13;
		value[2] = 0x14;

		mozart_bluetooth_server_set_char_value(server_num, char_attr_num, value, 3);
		/* send client indication */

		ble_indication.length_of_data = 3;
		ble_indication.value = value;
		int ret = mozart_bluetooth_ble_server_send_indication(&ble_indication);
		printf("ret = %d\n",ret);
	}
}

#if (SUPPORT_BSA_BLE_SERVER == 1)
#include "wifi_interface.h"
int packet_num = 0;
char packet_data[128];
UINT8 value[BSA_BLE_MAX_ATTR_LEN];
extern char *app_name;


int ble_net_config(char *ssid,char *passwd)
{
	if(!ssid||!passwd)
		return -1;
	mozart_module_stop();
	stopall(1);

	mozart_play_key("bt_netconfig");//开始蓝牙配网
	printf("ssid : %s,passwd:%s\n",ssid,passwd);
	FILE *fp = NULL;
	FILE *fp_w = NULL;
	  fp = fopen("/tmp/wpa_supplicant_inuse.conf","r");
	fp_w = fopen("/tmp/wpa_supplicant_inuse_tmp.conf","w");
	char read_buf[64] = {0};
	int cnt = 1;


	while (!feof(fp)&&fp&&fp_w) 
	{ 	
		memset(read_buf,0,sizeof(read_buf));
		fgets(read_buf,64,fp);

		if(strncmp(read_buf,"ssid=",5) == 0){
			memset(read_buf,0,sizeof(read_buf));
			sprintf(read_buf,"ssid=\"%s\"",ssid);
			read_buf[strlen(read_buf)] = '\n';
		}
		if(strncmp(read_buf,"psk=",4) == 0){
			memset(read_buf,0,sizeof(read_buf));
			sprintf(read_buf,"psk=\"%s\"",passwd);
			read_buf[strlen(read_buf)] = '\n';
		}
		#if 0
		if(strncmp(read_buf,"priority=",9) == 0){
			memset(read_buf,0,sizeof(read_buf));
			sprintf(read_buf,"priority=%d",3);
			read_buf[strlen(read_buf)] = '\n';
		}
		#endif
		printf("%s\n", read_buf); 
		fputs(read_buf,fp_w);
	} 
	if(fp)
		fclose(fp); 
	if(fp_w)
		fclose(fp_w); 
	
	printf("%%%%%%%%%%%%%%%%%%%%%%%%\n");
	system("rm /usr/data/wpa_supplicant.conf /tmp/wpa_supplicant_inuse.conf");
	system("mv /tmp/wpa_supplicant_inuse_tmp.conf /tmp/wpa_supplicant_inuse.conf");
	system("cp /tmp/wpa_supplicant_inuse.conf /usr/data/wpa_supplicant.conf");
	printf("%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%\n");
	system("cat /usr/data/wpa_supplicant.conf");
	//xiaole_sta();	
	wifi_ctl_msg_t new_mode;
	memset(&new_mode, 0, sizeof(new_mode));
	strcpy(new_mode.param.switch_sta.ssid,ssid);
	strcpy(new_mode.param.switch_sta.psk,passwd);
	new_mode.cmd = SW_STA;
	new_mode.force = true;
	strcpy(new_mode.name, app_name);
	if(request_wifi_mode(new_mode) != true)
		printf("ERROR: [%s] Request Network Failed, Please Register First!!!!\n", app_name);
	return 0;
}






static int bt_ble_server_write_cback(tBSA_BLE_SE_WRITE_MSG *ble_ser_write, UINT16 char_uuid)
{	printf("----------char_uuid = 0x%x\n",char_uuid);
				
	int server_num = 0;
	int char_attr_num = 0;
	unsigned int crc8_res = 0;
	crc8_res = crc8(ble_ser_write->value,(ble_ser_write->len -1));
	#if 1
	int i = 0;
	for (i = 0; i < ble_ser_write->len; i++) {
		printf("v[%d] = 0x%x ", i, ble_ser_write->value[i]);
	}
	printf("END\n");
	
	printf("############  crc8_res = 0x%x\n",crc8_res);
	printf("############  value[%d] = 0x%0x\n",i-1,ble_ser_write->value[i-1]);
	#endif
	#if 0 //蓝牙配网暂时关闭
	value[0] = 0x44;//0x11  0x44
	if(char_uuid == 0x2a04&&crc8_res == ble_ser_write->value[ble_ser_write->len-1])//ble_ser_write->value[0]
	{
		if(ble_ser_write->value[0] == 0){
			packet_num = ble_ser_write->value[1];
			memset(packet_data,0,sizeof(packet_data));
		}
		strncat(packet_data,&ble_ser_write->value[3],ble_ser_write->len - 4);

		if(ble_ser_write->value[0] == ble_ser_write->value[1] - 1){
			//printf("packet_data = %s\n",packet_data);
			
			json_object *json_msg= json_tokener_parse(packet_data);
			json_object *json_ssid = NULL;
			json_object *json_password = NULL;
			json_object_object_get_ex(json_msg,"passwd",&json_password);
			json_object_object_get_ex(json_msg,"ssid",&json_ssid);
			printf("ssid = %s\n",json_object_get_string(json_ssid));
			printf("passwd = %s\n",json_object_get_string(json_password));	
			printf("\n^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ SUCCESS\n");
			if(json_object_get_string(json_ssid)&&json_object_get_string(json_password)){
				ble_net_config(json_object_get_string(json_ssid),json_object_get_string(json_password));
				value[0] = 0x11;
			}

			server_num = ble_heart_rate_service.server_num;
			char_attr_num = ble_heart_rate_measure_char.char_attr_num;
			mozart_bluetooth_server_set_char_value(server_num, char_attr_num, value, 1);

			ble_server_indication ble_indication;
			ble_indication.server_num = ble_heart_rate_service.server_num;
			ble_indication.attr_num = ble_heart_rate_measure_char.char_attr_num;
			ble_indication.need_confirm = 1;
			ble_indication.length_of_data = 1;
			ble_indication.value = value;
			int ret = mozart_bluetooth_ble_server_send_indication(&ble_indication);
			printf("mozart_bluetooth_ble_server_send_indication ret = %d\n",ret);
						
		}
	}else if(char_uuid == 0x2a04&&crc8_res != ble_ser_write->value[ble_ser_write->len-1]){//配网失败
		printf("\n^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ FAILED\n");
		ble_server_indication ble_indication;
		ble_indication.server_num = ble_heart_rate_service.server_num;
		ble_indication.attr_num = ble_heart_rate_measure_char.char_attr_num;
		ble_indication.need_confirm = 1;

		server_num = ble_heart_rate_service.server_num;
		char_attr_num = ble_heart_rate_measure_char.char_attr_num;
		value[0] = 0x44;

		mozart_bluetooth_server_set_char_value(server_num, char_attr_num, value, 1);
		ble_indication.length_of_data = 1;
		ble_indication.value = value;
		int ret = mozart_bluetooth_ble_server_send_indication(&ble_indication);
		printf("mozart_bluetooth_ble_server_send_indication ret = %d\n",ret);
	}
	#endif
	return 0;
}

static int ble_netconfig_service_and_char_create()
{
	int i = 0;
	int state = 0;
	int server_num = 0;
	tBSA_DM_BLE_ADV_CONFIG ble_config_adv;
	tBSA_DM_BLE_ADV_PARAM ble_adv_param;
	ble_start_service_data ble_start_service;
	/* First 2 byte is Company Identifier Eg: 0x000f refers to Broadcom Corporation, followed by 6 bytes of data*/
	UINT8 app_ble_adv_value[APP_BLE_ADV_VALUE_LEN] = {0x0F, 0x01, 0x12, 0x23, 0x34, 0x45, 0x56, 0x67};

	memset(&ble_heart_rate_service, 0, sizeof(ble_create_service_data));
	memset(&ble_heart_rate_measure_char, 0, sizeof(ble_add_char_data));
	memset(&ble_config_adv, 0, sizeof(tBSA_DM_BLE_ADV_CONFIG));
	memset(&ble_start_service, 0, sizeof(ble_start_service_data));

	server_num = mozart_bluetooth_ble_server_register(0xabcd);//  注册服务 APP
	if (server_num < 0) {
		printf("?? mozart_bluetooth_ble_server_register failed, state = %d.\n", state);
		return -1;
	}
	ble_heart_rate_service.server_num = server_num;
	ble_heart_rate_service.service_uuid = UUID_SERVCLASS_TEST_SERVER;//0x9000
	ble_heart_rate_service.num_handle = 10;
	ble_heart_rate_service.is_primary = 1;
	state = mozart_bluetooth_ble_server_create_service(&ble_heart_rate_service);//创建GATT 服务
	if (state) {
		printf("?? mozart_bluetooth_ble_create_service failed, state = %d.\n", state);
		return -1;
	}
	for (i = 0; i < 30; i++) {
		if(ble_service_create_enable != 1)
			usleep(300*1000);
		else
			break;
	}
	if (ble_service_create_enable == 1)
		ble_service_create_enable = 0;
	else {
		printf("?? Error: not recived BSA_BLE_SE_CREATE_EVT, please checked!\n");
		return -1;
	}

	ble_heart_rate_measure_char.server_num = server_num;
	ble_heart_rate_measure_char.srvc_attr_num = ble_heart_rate_service.attr_num;
	ble_heart_rate_measure_char.char_uuid = 0x2a04;// 2A37
	ble_heart_rate_measure_char.is_descript = 0;
	/* Attribute Permissions[Eg: Read-0x1, Write-0x10, Read | Write-0x11] */
	ble_heart_rate_measure_char.attribute_permission =  BSA_GATT_PERM_READ |BSA_GATT_PERM_WRITE|BSA_GATT_CHAR_PROP_BIT_NOTIFY;
	/* Characterisic Properties Eg: WRITE-0x08, READ-0x02, Notify-0x10, Indicate-0x20
	 * Eg: For READ | WRITE|NOTIFY | INDICATE enter 0x3A */
	ble_heart_rate_measure_char.characteristic_property = BSA_GATT_CHAR_PROP_BIT_READ | BSA_GATT_CHAR_PROP_BIT_WRITE |
								BSA_GATT_CHAR_PROP_BIT_NOTIFY | BSA_GATT_CHAR_PROP_BIT_INDICATE;
	state = mozart_bluetooth_ble_server_add_character(&ble_heart_rate_measure_char);//添加特征
	if (state) {
		printf("?? mozart_bluetooth_ble_server_add_character failed, state = %d.\n", state);
		return -1;
	}
	for (i = 0; i < 30; i++) {
		if(ble_service_add_char_enable != 1)
			usleep(300*1000);
		else
			break;
	}
	if (ble_service_add_char_enable == 1) {
		ble_service_add_char_enable = 0;
	} else {
		printf("?? Error: not recived BSA_BLE_SE_ADDCHAR_EVT, please checked!\n");
		return -1;
	}
	/* descripter: service notify */
	ble_notify_char.server_num = server_num;
	ble_notify_char.srvc_attr_num = ble_heart_rate_service.attr_num;
	ble_notify_char.char_uuid = GATT_UUID_CHAR_CLIENT_CONFIG;// 2902
	ble_notify_char.is_descript = 1;
	ble_notify_char.attribute_permission = BSA_GATT_PERM_READ | BSA_GATT_PERM_WRITE;
	ble_notify_char.characteristic_property = BSA_GATT_CHAR_PROP_BIT_NOTIFY;//S0;
	state = mozart_bluetooth_ble_server_add_character(&ble_notify_char);//添加特征
	if (state) {
		printf("?? mozart_bluetooth_ble_server_add_character failed, state = %d.\n", state);
		return -1;
	}
	for (i = 0; i < 30; i++) {
		if(ble_service_add_char_enable != 1)
			usleep(300*1000);
		else
			break;
	}
	if (ble_service_add_char_enable == 1) {
		ble_service_add_char_enable = 0;
	} else {
		printf("?? Error: not recived BSA_BLE_SE_ADDCHAR_EVT, please checked!\n");
		return -1;
	}

	ble_start_service.server_num = server_num;
	ble_start_service.attr_num = ble_heart_rate_service.attr_num;
	state = mozart_bluetooth_ble_server_start_service(&ble_start_service);//开启ble 服务
	if (state) {
		printf("?? mozart_bluetooth_ble_server_tart_service failed, state = %d.\n", state);
		return -1;
	}
	for (i = 0; i < 30; i++) {
		if(ble_service_start_enable != 1)
			usleep(300*1000);
		else
			break;
	}
	if (ble_service_start_enable == 1) {
		ble_service_start_enable = 0;
	} else {
		printf("?? Error: not recived BSA_BLE_SE_START_EVT, please checked!\n");
		return -1;
	}

	/* Active broadcast */
	/* This is just sample code to show how BLE Adv data can be sent from application */
	/* Adv.Data should be < 31bytes, including Manufacturer data, Device Name, Appearance data, Services Info, etc.. */
	ble_config_adv.appearance_data = BLE_ADV_APPEARANCE_DATA;
	ble_config_adv.num_service = 1;
	ble_config_adv.uuid_val[0] = BSA_BLE_UUID_SERVCLASS_HEART_RATE;//180D
	ble_config_adv.len = APP_BLE_ADV_VALUE_LEN;
	ble_config_adv.flag = BSA_DM_BLE_ADV_FLAG_MASK;
	ble_config_adv.is_scan_rsp = 0; 	/* 0 is Active broadcast, 1 is Passive broadcast */
	ble_config_adv.is_part_service = 1; 	/* 1 is 0x02, 0 is 0x03 */
	memcpy(ble_config_adv.p_val, app_ble_adv_value, APP_BLE_ADV_VALUE_LEN);
	/* All the masks/fields that are set will be advertised */
	ble_config_adv.adv_data_mask = BSA_DM_BLE_AD_BIT_FLAGS|BSA_DM_BLE_AD_BIT_SERVICE|BSA_DM_BLE_AD_BIT_DEV_NAME;
	state = mozart_bluetooth_ble_configure_adv_data(&ble_config_adv); // 开始广播
	if (state) {
		printf("?? mozart_bluetooth_ble_configure_adv_data failed, state = %d.\n", state);
		return -1;
	}

	/* Passive broadcast */
	memset(&ble_config_adv, 0, sizeof(tBSA_DM_BLE_ADV_CONFIG));
	/* start advertising */
	ble_config_adv.adv_data_mask = BSA_DM_BLE_AD_BIT_DEV_NAME;
	ble_config_adv.is_scan_rsp = 1;  //是反应数据还是广播数据

	state = mozart_bluetooth_ble_configure_adv_data(&ble_config_adv);
	if (state) {
		printf("?? mozart_bluetooth_ble_configure_adv_data failed, state = %d.\n", state);
		return -1;
	}

	/* set Advertising_Interval 设置广播间隔*/
	/* Range: 0x0020 to 0x4000
	   Time = Range * 0.625 msec
	   Time Range: 20 ms to 10.24 sec */
	memset(&ble_adv_param, 0, sizeof(tBSA_DM_BLE_ADV_PARAM));
	ble_adv_param.adv_int_min = 0xa0;	/* 100 ms */
	ble_adv_param.adv_int_max = 0xa0;	/* 100 ms */
	mozart_bluetooth_ble_set_adv_param(&ble_adv_param);

	return 0;
}

static int ble_service_and_char_create()
{
	int i = 0;
	int state = 0;
	int server_num = 0;
	tBSA_DM_BLE_ADV_CONFIG ble_config_adv;
	tBSA_DM_BLE_ADV_PARAM ble_adv_param;
	ble_start_service_data ble_start_service;
	/* First 2 byte is Company Identifier Eg: 0x000f refers to Broadcom Corporation, followed by 6 bytes of data*/
	UINT8 app_ble_adv_value[APP_BLE_ADV_VALUE_LEN] = {0x0F, 0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66};

	memset(&ble_heart_rate_service, 0, sizeof(ble_create_service_data));
	memset(&ble_heart_rate_measure_char, 0, sizeof(ble_add_char_data));
	memset(&ble_config_adv, 0, sizeof(tBSA_DM_BLE_ADV_CONFIG));
	memset(&ble_start_service, 0, sizeof(ble_start_service_data));

	server_num = mozart_bluetooth_ble_server_register(APP_BLE_MAIN_DEFAULT_APPL_UUID);
	printf("???? mozart_bluetooth_ble_server_register server_num = %d\n",server_num);
	if (server_num < 0) {
		printf("mozart_bluetooth_ble_server_register failed, state = %d.\n", state);
		return -1;
	}
	ble_heart_rate_service.server_num = server_num;
	ble_heart_rate_service.service_uuid = BSA_BLE_UUID_SERVCLASS_HEART_RATE;
	ble_heart_rate_service.num_handle = 10;
	ble_heart_rate_service.is_primary = 1;
	state = mozart_bluetooth_ble_server_create_service(&ble_heart_rate_service);
	if (state) {
		printf("mozart_bluetooth_ble_create_service failed, state = %d.\n", state);
		return -1;
	}
	for (i = 0; i < 30; i++) {
		if(ble_service_create_enable != 1)
			usleep(300*1000);
		else
			break;
	}
	if (ble_service_create_enable == 1)
		ble_service_create_enable = 0;
	else {
		printf("Error: not recived BSA_BLE_SE_CREATE_EVT, please checked!\n");
		return -1;
	}

	ble_heart_rate_measure_char.server_num = server_num;
	ble_heart_rate_measure_char.srvc_attr_num = ble_heart_rate_service.attr_num;
	ble_heart_rate_measure_char.char_uuid = BSA_BLE_GATT_UUID_HEART_RATE_MEASUREMENT;
	ble_heart_rate_measure_char.is_descript = 0;
	/* Attribute Permissions[Eg: Read-0x1, Write-0x10, Read | Write-0x11] */
	ble_heart_rate_measure_char.attribute_permission = BSA_GATT_PERM_READ | BSA_GATT_PERM_WRITE;

	/* Characterisic Properties Eg: WRITE-0x08, READ-0x02, Notify-0x10, Indicate-0x20
	 * Eg: For READ | WRITE|NOTIFY | INDICATE enter 0x3A */
	ble_heart_rate_measure_char.characteristic_property = BSA_GATT_CHAR_PROP_BIT_READ | BSA_GATT_CHAR_PROP_BIT_WRITE |
								BSA_GATT_CHAR_PROP_BIT_NOTIFY | BSA_GATT_CHAR_PROP_BIT_INDICATE;

	state = mozart_bluetooth_ble_server_add_character(&ble_heart_rate_measure_char);
	if (state) {
		printf("mozart_bluetooth_ble_server_add_character failed, state = %d.\n", state);
		return -1;
	}
	for (i = 0; i < 30; i++) {
		if(ble_service_add_char_enable != 1)
			usleep(300*1000);
		else
			break;
	}
	if (ble_service_add_char_enable == 1) {
		ble_service_add_char_enable = 0;
	} else {
		printf("Error: not recived BSA_BLE_SE_ADDCHAR_EVT, please checked!\n");
		return -1;
	}
#if 0
	ble_netconfig_char.server_num = server_num;
	ble_netconfig_char.srvc_attr_num = ble_heart_rate_service.attr_num;
	ble_netconfig_char.char_uuid = 0x2A40;
	ble_netconfig_char.is_descript = 1;
	ble_netconfig_char.attribute_permission =  BSA_GATT_PERM_READ | BSA_GATT_PERM_WRITE;
	ble_netconfig_char.characteristic_property = 0 ;
	state = mozart_bluetooth_ble_server_add_character(&ble_netconfig_char);//添加特征
	if (state) {
		printf("mozart_bluetooth_ble_server_add_character failed, state = %d.\n", state);
		return -1;
	}
	for (i = 0; i < 30; i++) {
		if(ble_service_add_char_enable != 1)
			usleep(300*1000);
		else
			break;
	}
	if (ble_service_add_char_enable == 1) {
		ble_service_add_char_enable = 0;
	} else {
		printf("Error: not recived BSA_BLE_SE_ADDCHAR_EVT, please checked!\n");
		return -1;
	}
#endif 
	/* descripter: service notify */
	ble_notify_char.server_num = server_num;
	ble_notify_char.srvc_attr_num = ble_heart_rate_service.attr_num;
	ble_notify_char.char_uuid = GATT_UUID_CHAR_CLIENT_CONFIG;
	ble_notify_char.is_descript = 1;
	ble_notify_char.attribute_permission =  BSA_GATT_PERM_READ | BSA_GATT_PERM_WRITE;
	ble_notify_char.characteristic_property = 0;
	state = mozart_bluetooth_ble_server_add_character(&ble_notify_char);
	if (state) {
		printf("mozart_bluetooth_ble_server_add_character failed, state = %d.\n", state);
		return -1;
	}
	for (i = 0; i < 30; i++) {
		if(ble_service_add_char_enable != 1)
			usleep(300*1000);
		else
			break;
	}
	if (ble_service_add_char_enable == 1) {
		ble_service_add_char_enable = 0;
	} else {
		printf("Error: not recived BSA_BLE_SE_ADDCHAR_EVT, please checked!\n");
		return -1;
	}

	ble_start_service.server_num = server_num;
	ble_start_service.attr_num = ble_heart_rate_service.attr_num;
	state = mozart_bluetooth_ble_server_start_service(&ble_start_service);
	if (state) {
		printf("mozart_bluetooth_ble_server_tart_service failed, state = %d.\n", state);
		return -1;
	}
	for (i = 0; i < 30; i++) {
		if(ble_service_start_enable != 1)
			usleep(300*1000);
		else
			break;
	}
	if (ble_service_start_enable == 1) {
		ble_service_start_enable = 0;
	} else {
		printf("Error: not recived BSA_BLE_SE_START_EVT, please checked!\n");
		return -1;
	}

	/* Active broadcast */
	/* This is just sample code to show how BLE Adv data can be sent from application */
	/* Adv.Data should be < 31bytes, including Manufacturer data, Device Name, Appearance data, Services Info, etc.. */
	ble_config_adv.appearance_data = BLE_ADV_APPEARANCE_DATA;
	ble_config_adv.num_service = 1;
	ble_config_adv.uuid_val[0] = BSA_BLE_UUID_SERVCLASS_HEART_RATE;
	ble_config_adv.len = APP_BLE_ADV_VALUE_LEN;
	ble_config_adv.flag = BSA_DM_BLE_ADV_FLAG_MASK;
	ble_config_adv.is_scan_rsp = 0; 	/* 0 is Active broadcast, 1 is Passive broadcast */
	ble_config_adv.is_part_service = 1; 	/* 1 is 0x02, 0 is 0x03 */
	memcpy(ble_config_adv.p_val, app_ble_adv_value, APP_BLE_ADV_VALUE_LEN);
	/* All the masks/fields that are set will be advertised */
	ble_config_adv.adv_data_mask = BSA_DM_BLE_AD_BIT_FLAGS|BSA_DM_BLE_AD_BIT_SERVICE|BSA_DM_BLE_AD_BIT_DEV_NAME;
	state = mozart_bluetooth_ble_configure_adv_data(&ble_config_adv);
	if (state) {
		printf("mozart_bluetooth_ble_configure_adv_data failed, state = %d.\n", state);
		return -1;
	}

	/* Passive broadcast */
	memset(&ble_config_adv, 0, sizeof(tBSA_DM_BLE_ADV_CONFIG));
	/* start advertising */
	ble_config_adv.adv_data_mask = BSA_DM_BLE_AD_BIT_DEV_NAME;
	ble_config_adv.is_scan_rsp = 1;

	state = mozart_bluetooth_ble_configure_adv_data(&ble_config_adv);
	if (state) {
		printf("mozart_bluetooth_ble_configure_adv_data failed, state = %d.\n", state);
		return -1;
	}

	/* set Advertising_Interval */
	/* Range: 0x0020 to 0x4000
	   Time = Range * 0.625 msec
	   Time Range: 20 ms to 10.24 sec */
	memset(&ble_adv_param, 0, sizeof(tBSA_DM_BLE_ADV_PARAM));
	ble_adv_param.adv_int_min = 0xa0;	/* 100 ms */
	ble_adv_param.adv_int_max = 0xa0;	/* 100 ms */
	mozart_bluetooth_ble_set_adv_param(&ble_adv_param);

	return 0;
}
#endif /* SUPPORT_BSA_BLE_SERVER */

#if (SUPPORT_BSA_BLE_CLIENT == 1)
static int ble_common_profile_cback(tBSA_BLE_EVT event, tBSA_BLE_MSG *p_data)
{	printf("-----------------ble_common_profile_cback-------------------\n");
	int i = 0;
	switch (event) {
		case BSA_BLE_CL_READ_EVT:
			for (i = 0; i < p_data->cli_read.len; i++) {
				printf("Read: value[%d] = 0x%x\n", i, p_data->cli_read.value[i]);
			}
			ble_client_read_enable = 1;
			break;
		case BSA_BLE_CL_NOTIF_EVT:
			for (i = 0; i < p_data->cli_notif.len; i++) {
				printf("Notif: value[%d] = 0x%x\n", i, p_data->cli_notif.value[i]);
			}
			break;
		default:
			break;
	}
	return 0;
}

static int ble_client_create()
{
	int i = 0;
	int state = 0;
	int disc_index = -1;
	int client_num = 0;
	ble_client_connect_data cl_connect_data;
	ble_client_read_data client_rdata;
	ble_client_write_data cl_wdata;
	BLE_CL_NOTIFREG cl_notireg;
	char *connect_ble_name = "BM3508";
	UINT16 client_uuid = 0x4000;
	int num = 30;

	/* register ble client */
	client_num = state = mozart_bluetooth_ble_client_register(client_uuid);

	if (state < 0) {
		printf("ERROR: mozart_bluetooth_ble_client_register failed !\n");
		return -1;
	}
	for (i = 0; i < 5; i++) {
		/* discovery ble devices */
		state = mozart_bluetooth_ble_start_regular();
		if (state != 0) {
			printf("ERROR: mozart_bluetooth_ble_start_regular failed !\n");
			return -1;
		}

		while (bsa_ble_start_regular_enable != 1) {
			usleep(300*1000);
		}
		bsa_ble_start_regular_enable = 0;

		/* find connect_ble_name from discovered ble devices */
		disc_index = mozart_bluetooth_disc_find_device_name(connect_ble_name);
		if (disc_index != -1) {
			printf("mozart_bluetooth_disc_find_device_name %s success!\n", connect_ble_name);
			break;
		} else {
			printf("not find ble device: %s, rediscovery ble device!\n", connect_ble_name);
		}
	}
	if ((disc_index == -1) && (i >= 6)) {
		printf("ERROR: mozart_bluetooth_disc_find_device_name failed!\n");
		return -1;
	}

	/* connect ble server device */
	cl_connect_data.device_index = 1;	/*use Device found in last discovery */

	/* Priority judged ble_name device, if ble_name is NULL, then judged bd_addr device */
	cl_connect_data.ble_name = connect_ble_name;
	cl_connect_data.client_num = client_num;
	cl_connect_data.direct = 1;
	mozart_bluetooth_ble_client_connect_server(&cl_connect_data);
	for (i = 0; i < num; i++) {
		if(ble_client_open_enable != 1)
			usleep(300*1000);
		else {
			ble_client_open_enable = 0;
			break;
		}
	}
	if (i > num) {
		printf("ble_client_open_enable failed!\n");
		return -1;
	}

	/* read connected ble device */
	client_rdata.service_uuid = BSA_BLE_UUID_SERVCLASS_HEART_RATE;
	client_rdata.char_uuid = BSA_BLE_GATT_UUID_HEART_RATE_MEASUREMENT;
	client_rdata.client_num = client_num;
	client_rdata.is_primary = 1;
	client_rdata.descr_id = 0;
	client_rdata.is_descript = 0;
	client_rdata.ser_inst_id = 0;
	client_rdata.char_inst_id = 0;
	state = mozart_bluetooth_ble_client_read(&client_rdata);
	if (state != 0) {
		printf("mozart_bluetooth_ble_client_read failed !\n");
		return -1;
	}
	for (i = 0; i < num; i++) {
		if(ble_client_read_enable != 1)
			usleep(300*1000);
		else {
			ble_client_read_enable = 0;
			break;
		}
	}
	if (i >= num) {
		printf("ble_client_read_enable failed!\n");
		return -1;
	}

	/* write connected ble device */
	cl_wdata.write_len = 4;
	cl_wdata.service_uuid = BSA_BLE_UUID_SERVCLASS_HEART_RATE;
	cl_wdata.char_uuid = BSA_BLE_GATT_UUID_HEART_RATE_MEASUREMENT;
	cl_wdata.write_data[0] = 0x11;
	cl_wdata.write_data[1] = 0x22;
	cl_wdata.write_data[2] = 0x33;
	cl_wdata.write_data[3] = 0x44;

	cl_wdata.client_num = client_num;
	cl_wdata.is_primary = 1;
	cl_wdata.ser_inst_id = 0;
	cl_wdata.char_inst_id = 0;
	cl_wdata.is_descript = 0;
	cl_wdata.descr_id = 0;
	cl_wdata.desc_value = 0;
	cl_wdata.write_type = 1;
	state = mozart_bluetooth_ble_client_write(&cl_wdata);
	if (state != 0) {
		printf("mozart_bluetooth_ble_client_write failed !\n");
		return -1;
	}
	for (i = 0; i < num; i++) {
		if(ble_client_write_enable != 1)
			usleep(300*1000);
		else {
			ble_client_write_enable = 0;
			break;
		}
	}
	if (i >= num) {
		printf("ble_client_write_enable failed!\n");
		return -1;
	}

	/* register notification */
	cl_notireg.client_num = client_num;
	cl_notireg.service_id = BSA_BLE_UUID_SERVCLASS_HEART_RATE;
	cl_notireg.character_id = BSA_BLE_GATT_UUID_HEART_RATE_MEASUREMENT;
	cl_notireg.is_primary = 1;
	cl_notireg.ser_inst_id = 0;
	cl_notireg.char_inst_id = 0;
	state = mozart_bluetooth_ble_client_register_notification(&cl_notireg);
	printf("state = %d\n", state);

	/* open ble server notificaion */
	cl_wdata.service_uuid = BSA_BLE_UUID_SERVCLASS_HEART_RATE;
	cl_wdata.char_uuid = GATT_UUID_CHAR_CLIENT_CONFIG;
	cl_wdata.write_len = 2;
	cl_wdata.write_data[0] = 0x01;
	cl_wdata.write_data[1] = 0x00;
	cl_wdata.client_num = client_num;
	cl_wdata.is_primary = 1;
	cl_wdata.ser_inst_id = 0;
	cl_wdata.char_inst_id = 0;
	cl_wdata.is_descript = 0;
	cl_wdata.descr_id = 0;
	cl_wdata.desc_value = 0;
	cl_wdata.write_type = 1;
	state = mozart_bluetooth_ble_client_write(&cl_wdata);
	if (state != 0) {
		printf("mozart_bluetooth_ble_client_write failed !\n");
		return -1;
	}

	return 0;
}
#endif /* SUPPORT_BSA_BLE_CLIENT */

#if (SUPPORT_BSA_BLE_HH == 1)
static int mozart_bluetooth_hh_uipc_cback(UINT8 *data, UINT16 data_len)
{
#if (SUPPORT_BSA_BLE_HH_DIALOG_AUDIO == 1)
	/* compare instance, data[0] is report id */
	int instance = data[0] -1;

	data++;
	if ((instance >= HID_STREAM_DATA_MIN_INSTANCE) && (instance <= HID_STREAM_DATA_MAX_INSTANCE)) {
		dialog_sendAudioData(data, data_len - 1);
		if (dia_prevInstance == 0)
			dia_prevInstance = instance;
		dia_prevInstance++;
		if (dia_prevInstance > HID_STREAM_DATA_MAX_INSTANCE)
			dia_prevInstance = HID_STREAM_DATA_MIN_INSTANCE;
		if (dia_prevInstance != instance)
		{
			printf("Warning, packet sequence interruption, expected %d, received %d\n", dia_prevInstance, instance);
			dia_prevInstance = instance;
		}
	} else if (instance == HID_STREAM_ENABLE_READ_INSTANCE) {
		printf("-------->>> HID_STREAM_ENABLE_READ_INSTANCE!\n");
		dialog_updatekeys(data);
		if ((data[0] == 0) && (data[1] == 4)) {
			int mode = (int)data[2];
			dialog_setDecodeMode(mode);
		}
	}
#endif
	return 0;
}

static int ble_hh_create()
{
	int i = 0;
	int disc_index = -1;
	ble_hh_connect_data hh_connect_data;
	ble_client_connect_data cl_connect_data;
	char *connect_ble_name = "RemoteB008";
	int num = 120;
	int state = 0;

	UINT16 client_uuid = 0x4000;
	/* register ble client */
	hh_client_num = state = mozart_bluetooth_ble_client_register(client_uuid);
	if (state < 0) {
		printf("ERROR: mozart_bluetooth_ble_client_register failed !\n");
		return -1;
	}
	mozart_bluetooth_hh_start();

	for (i = 0; i < 5; i++) {
		/* discovery ble devices */
		state = mozart_bluetooth_ble_start_regular();
		if (state != 0) {
			printf("ERROR: mozart_bluetooth_ble_start_regular failed !\n");
			return -1;
		}

		while (bsa_ble_start_regular_enable != 1) {
			usleep(300*1000);
		}
		bsa_ble_start_regular_enable = 0;

		/* find connect_ble_name from discovered ble devices */
		disc_index = mozart_bluetooth_disc_find_device_name(connect_ble_name);
		if (disc_index != -1) {
			printf("mozart_bluetooth_disc_find_device_name %s success!\n", connect_ble_name);
			break;
		} else {
			printf("not find ble device: %s, rediscovery ble device!\n", connect_ble_name);
		}
	}
	if ((disc_index == -1) && (i >= 5)) {
		printf("ERROR: mozart_bluetooth_disc_find_device_name failed!\n");
		return -1;
	}

	/* connect ble server device */
	cl_connect_data.device_index = DEVICE_FROM_DISCOVERY;	/* use Device found in last discovery */
	/* Priority judged ble_name device, if ble_name is NULL, then judged bd_addr device */
	cl_connect_data.ble_name = connect_ble_name;
	cl_connect_data.client_num = hh_client_num;
	cl_connect_data.direct = 1;
	printf("mozart_bluetooth_ble_client_connect_server !!\n");
	state = mozart_bluetooth_ble_client_connect_server(&cl_connect_data);
	if (state != 0) {
		printf("mozart_bluetooth_ble_client_connect_server failed !\n");
		return -1;
	} else {
		printf("mozart_bluetooth_ble_client_connect_server succeed !\n");
	}
	for (i = 0; i < num; i++) {
		if(ble_client_open_enable != 1)
			usleep(300*1000);
		else {
			ble_client_open_enable = 0;
			break;
		}
	}
	if (i >= num) {
		printf("ble_client_open_enable failed!\n");
		return -1;
	}

	/* hh connect */
	hh_connect_data.device_index = DEVICE_FROM_DISCOVERY;	/* use Device found in last discovery */
	hh_connect_data.disc_index = disc_index;
	hh_connect_data.sec_type = SEC_NONE;
	hh_connect_data.mode = BSA_HH_PROTO_RPT_MODE;
	state = mozart_bluetooth_hh_connect(&hh_connect_data);
	if (state != 0) {
		printf("mozart_bluetooth_hh_connect failed !\n");
		return -1;
	}

	for (i = 0; i < num; i++) {
		if(ble_hh_add_dev_enable != 1)
			usleep(500*1000);
		else {
			ble_hh_add_dev_enable = 0;
			break;
		}
	}
	if (i >= num) {
		printf("ble_hh_add_dev_enable failed!\n");
		return -1;
	}

	return 0;
}
#endif /* SUPPORT_BSA_BLE_HH */
#endif /* SUPPORT_BSA_BLE */

static void bt_info_init(bt_init_info *bt_info, char *bt_name)
{
	char ble_new_name[32] = {0};
	char macaddr[] = "255.255.255.255";
	memset(macaddr, 0, sizeof(macaddr));
	get_mac_addr("wlan0", macaddr, "");
	sprintf(ble_new_name, "NS-%s", macaddr + 6);
	bt_info->bt_ble_name = ble_new_name;
	bt_info->bt_name = bt_name;
	//bt_info->bt_ble_name = "BSA_BLE_XiaoLe";
	bt_info->discoverable = 0;
	bt_info->connectable = 0;
	memset(bt_info->out_bd_addr, 0, sizeof(bt_info->out_bd_addr));
}
#if ((SUPPORT_AEC_RESAMPLE == 1) && (SUPPORT_AEC_RESAMPLE_48K_TO_8K == 1))
int bt_aec_resample_outlen_max = 0;
void mozart_bt_aec_resample_data_callback(bt_aec_resample_msg *bt_aec_rmsg)
{
	int resample_outlen = mozart_resample_get_outlen(bt_aec_rmsg->in_len, bt_aec_sdata.sample_rate, bt_aec_sdata.sample_channel, bt_aec_sdata.sample_bits, bt_aec_rdata.resample_rate);

	if (bt_aec_rmsg->out_buffer == NULL) {
		printf("bt_aec_rmsg->out_buffer == NULL !\n");
		bt_aec_resample_outlen_max = resample_outlen;
		bt_aec_rmsg->out_buffer = malloc(resample_outlen);
		bt_aec_rmsg->out_len = resample_outlen;
	} else {
		if (resample_outlen > bt_aec_resample_outlen_max) {
			printf("realloc !\n");
			bt_aec_resample_outlen_max = resample_outlen;
			bt_aec_rmsg->out_buffer = realloc(bt_aec_rmsg->out_buffer, resample_outlen);
			bt_aec_rmsg->out_len = resample_outlen;
		} else {
			bt_aec_rmsg->out_len = resample_outlen;
		}
	}
	bt_aec_rmsg->out_len = mozart_resample(aec_resample_s, bt_aec_sdata.sample_channel, bt_aec_sdata.sample_bits,
					    bt_aec_rmsg->in_buffer, bt_aec_rmsg->in_len, bt_aec_rmsg->out_buffer);
}
#endif

#if (SUPPORT_BSA_HS_RESAMPLE == 1)
int hs_resample_outlen_max = 0;
void mozart_hs_resample_data_callback(hs_resample_msg *hs_rmsg)
{
#if (SUPPORT_BSA_HS_RESAMPLE_8K_to_48K == 1)
	int resample_outlen = mozart_resample_get_outlen(hs_rmsg->in_len, hs_sample_data.sample_rate, hs_sample_data.sample_channel, hs_sample_data.sample_bits, hs_resample_data.resample_rate);

	if (hs_rmsg->out_buffer == NULL) {
		printf("hs_rmsg->out_buffer == NULL !\n");
		hs_resample_outlen_max = resample_outlen;
		hs_rmsg->out_buffer = malloc(resample_outlen);
		hs_rmsg->out_len = resample_outlen;
	} else {
		if (resample_outlen > hs_resample_outlen_max) {
			printf("realloc !\n");
			hs_resample_outlen_max = resample_outlen;
			hs_rmsg->out_buffer = realloc(hs_rmsg->out_buffer, resample_outlen);
			hs_rmsg->out_len = resample_outlen;
		} else {
			hs_rmsg->out_len = resample_outlen;
		}
	}
	hs_rmsg->out_len = mozart_resample(hs_resample_s, hs_sample_data.sample_channel, hs_sample_data.sample_bits,
					    hs_rmsg->in_buffer, hs_rmsg->in_len, hs_rmsg->out_buffer);
#else
	hs_rmsg->out_len = hs_rmsg->in_len;
	hs_rmsg->out_buffer = hs_rmsg->in_buffer;
#endif
}
#endif

#if (SUPPORT_BSA_A2DP_SOURCE == 1)
#define DEFAULT_SAMPLE_RATE	44100
#define DEFAULT_CHANNELS	2
#define DEFAULT_FORMAT		16
#define DEFAULT_SOCKET          "/var/run/bt-av-stream.socket"
#define BURSTSIZE		4096

static char *local_buf = NULL;
player_handler_t *av_handle = NULL;

typedef struct {
	int     channels;       /* number of audio channels */
	int     bytes;          /* bytes per sample */
	int     format;         /* oss format */
	int     rate;           /* samples per second (in a single channel) */
} ao_format_t;

static int lostream_init_callback(struct domain_init_hdr *hdr, void *data)
{
	tBSA_AV_MEDIA_FEED_CFG_PCM pcm_config;

	printf("Audio ori samplerate: %d\n", hdr->info.samplerate);
	printf("Audio ori channels: %d\n", hdr->info.channels);
	printf("Audio ori format: %d\n", hdr->info.format);

	/* Final sample rate */
	hdr->info.samplerate     = DEFAULT_SAMPLE_RATE;
	hdr->info.channels 	 = DEFAULT_CHANNELS;
	hdr->info.format 	 = AF_FORMAT_S16_LE;
	printf("Audio ori samplerate: %d\n", hdr->info.samplerate);
	printf("Audio ori channels: %d\n", hdr->info.channels);
	printf("Audio ori format: %d\n", hdr->info.format);

	local_buf = malloc(BURSTSIZE);
	if (local_buf == NULL) {
		perror("malloc failed\n");
		return -1;
	}
	pcm_config.sampling_freq = DEFAULT_SAMPLE_RATE;
	pcm_config.num_channel = DEFAULT_CHANNELS;
	pcm_config.bit_per_sample = DEFAULT_FORMAT;
	mozart_bluetooth_av_start_stream(&pcm_config);

	return 0;
}

static int lostream_get_buffer(struct loStreamBuffer *s_buf, void *data)
{
	if (local_buf != NULL) {
		memset(local_buf, 0, sizeof(local_buf));
		s_buf->buf = local_buf;
		s_buf->size = BURSTSIZE;
	} else {
		printf("ERROR: local_buf == NULL, please checked!!\n");
		return -1;
	}
	return 0;
}

static void lostream_getspace_callback(struct domain_getspace_hdr *hdr, void *data)
{
	hdr->space = BURSTSIZE;
	return;
}

static int lostream_stream_callback(struct domain_stream_reply *hdr, int size, void *data)
{
	mozart_bluetooth_av_send_stream(local_buf, size);

	return 0;
}

static int lostream_stop_callback(void *header, void *data)
{
	if (local_buf != NULL)
		free(local_buf);
	mozart_bluetooth_av_stop_stream();

	return 0;
}

static struct loStream_callback localstream_cback = {
	.initStream     = lostream_init_callback,
	.getBuffer      = lostream_get_buffer,
	.getSpace       = lostream_getspace_callback,
	.handleRecv     = lostream_stream_callback,
	.stopStream     = lostream_stop_callback,
};

int avk_source_set_audio_output()
{
	char *ao_iface = "stream:sockfile=/var/run/bt-av-stream.socket";
	av_handle = mozart_player_handler_get("bsa", NULL, NULL);
	if (!av_handle)
		return -1;

	mozart_player_aoswitch(av_handle, ao_iface);
	return 0;
}

struct localInStream  *loStream;
static int mozart_avk_source_stream_init()
{
	printf("avk source init create socket: %s\n", DEFAULT_SOCKET);
	loStream = mozart_local_stream_create(DEFAULT_SOCKET, BURSTSIZE);
	mozart_local_stream_init(loStream, &localstream_cback, NULL);

	return 0;
}
#endif /* SUPPORT_BSA_A2DP_SOURCE */

#if (SUPPORT_BSA_A2DP_SYNK == 1)
static af_resample_t *avk_resample_t;
static af_channels_t *avk_channel_t;
static char *avk_resample_outbuf;
static char *avk_channel_outbuf;
static int  avk_resample_max_outlen;
static int  avk_channel_max_outlen;
static int  avk_resample_outlen;
static int  avk_channel_outlen;

static void mozart_avk_event_cback(tBSA_AVK_EVT event, void *p_data)
{
	switch (event) {
		case BSA_AVK_STOP_EVT:
			/* uninit resample */
			if(avk_resample_t != NULL) {
				mozart_resample_uninit(avk_resample_t);
				avk_resample_t = NULL;
			}
			if(avk_channel_t != NULL) {
				mozart_channels_uninit(avk_channel_t);
				avk_channel_t = NULL;
			}

			/* free out buffer */
			if (avk_resample_outbuf != NULL) {
				free(avk_resample_outbuf);
				avk_resample_outbuf = NULL;
			}
			if (avk_channel_outbuf != NULL) {
				free(avk_channel_outbuf);
				avk_channel_outbuf = NULL;
			}
			avk_resample_outlen 	= 0;
			avk_channel_outlen 	= 0;
			avk_resample_max_outlen = 0;
			avk_channel_max_outlen 	= 0;
			break;

		default:
			printf("[AVK] %s Not handle this event: %d\n", __func__, event);
			break;
	}
}

static void mozart_avk_data_cback(avk_data_cback_msg *avk_msg)
{
	/* No Resample */
	if ((avk_msg->recv_rate == avk_msg->cfg_rate)
			&& (avk_msg->recv_channel == avk_msg->cfg_channel)
			&& (avk_msg->recv_bits == avk_msg->cfg_bits)) {
		avk_msg->out_len    = avk_msg->in_len;
		avk_msg->out_buffer = avk_msg->in_buffer;
		return;
	/* Resample */
	} else {
		/* resample rate */
		if (avk_msg->recv_rate != avk_msg->cfg_rate) {
			if (avk_resample_t == NULL) {
				avk_resample_t = mozart_resample_init(avk_msg->recv_rate, avk_msg->recv_channel, (avk_msg->recv_bits >> 3), avk_msg->cfg_rate);
				if (avk_resample_t == NULL) {
					printf("Error: mozart resample init rate failed\n");
					return;
				}
			}

			avk_resample_outlen = mozart_resample_get_outlen(avk_msg->in_len, avk_msg->recv_rate, avk_msg->recv_channel, avk_msg->recv_bits, avk_msg->cfg_rate);
			if (avk_resample_outbuf == NULL) {
				printf("malloc avk_resample_outbuf!\n");
				avk_resample_outbuf     = malloc(avk_resample_outlen);
				avk_resample_max_outlen = avk_resample_outlen;
			} else {
				if (avk_resample_outlen > avk_resample_max_outlen) {
					printf("realloc avk_resample_outbuf!\n");
					avk_resample_outbuf     = realloc(avk_resample_outbuf, avk_resample_outlen);
					avk_resample_max_outlen = avk_resample_outlen;
				}
			}
			avk_resample_outlen = mozart_resample(avk_resample_t, avk_msg->recv_channel, avk_msg->recv_bits, avk_msg->in_buffer, avk_msg->in_len, avk_resample_outbuf);

			/* resample channel */
			if (avk_msg->recv_channel != avk_msg->cfg_channel) {
				if (avk_channel_t == NULL) {
					avk_channel_t = mozart_channels_init(avk_msg->recv_channel, avk_msg->cfg_channel);
					if (avk_channel_t == NULL) {
						printf("Error: mozart resample init channel failed\n");
						return;
					}
				}
				avk_channel_outlen = mozart_channels_get_outlen(avk_channel_t, avk_resample_outlen);
				if (avk_channel_outbuf == NULL) {
					printf("malloc avk_channel_outbuf!\n");
					avk_channel_outbuf     = malloc(avk_channel_outlen);
					avk_channel_max_outlen = avk_channel_outlen;
				} else {
					if (avk_channel_outlen > avk_channel_max_outlen) {
						printf("realloc avk_channel_outbuf!\n");
						avk_channel_outbuf     = realloc(avk_channel_outbuf, avk_channel_outlen);
						avk_channel_max_outlen = avk_channel_outlen;
					}
				}
				mozart_channels(avk_channel_t, avk_msg->recv_bits>>3, 1, 0, avk_resample_outbuf, avk_resample_outlen, avk_channel_outbuf);
				avk_msg->out_len    = avk_channel_outlen;
				avk_msg->out_buffer = avk_channel_outbuf;
			} else {
				avk_msg->out_len    = avk_resample_outlen;
				avk_msg->out_buffer = avk_resample_outbuf;
			}
		} else {
			/* resample channel */
			if (avk_msg->recv_channel != avk_msg->cfg_channel) {
				if (avk_channel_t == NULL) {
					avk_channel_t = mozart_channels_init(avk_msg->recv_channel, avk_msg->cfg_channel);
					if (avk_channel_t == NULL) {
						printf("Error: mozart resample init channel failed\n");
						return;
					}
				}
				avk_channel_outlen = mozart_channels_get_outlen(avk_channel_t, avk_msg->in_len);
				if (avk_channel_outbuf == NULL) {
					printf("avk_channel_outbuf == NULL !\n");
					avk_channel_outbuf     = malloc(avk_channel_outlen);
					avk_channel_max_outlen = avk_channel_outlen;
				} else {
					if (avk_channel_outlen > avk_channel_max_outlen) {
						printf("realloc !\n");
						avk_channel_outbuf     = realloc(avk_channel_outbuf, avk_channel_outlen);
						avk_channel_max_outlen = avk_channel_outlen;
					}
				}
				mozart_channels(avk_channel_t, avk_msg->recv_bits>>3, 1, 0, avk_msg->in_buffer, avk_msg->in_len, avk_channel_outbuf);
				avk_msg->out_len    = avk_channel_outlen;
				avk_msg->out_buffer = avk_channel_outbuf;
			}
		}

		/* resample format */
		if (avk_msg->recv_bits != avk_msg->cfg_bits) {
			printf("Resample format: recv_format: %d, cfg_format: %d\n", avk_msg->recv_bits, avk_msg->cfg_bits);
			printf("Error: not handle format !!\n");
		}
	}
}
#endif /* SUPPORT_BSA_A2DP_SYNK */

static void *thr_fn(void *args)
{
	int i = 0;
	bt_init_info bt_info;
	char mac[] = "00:11:22:33:44:55";
	char bt_name[64] = {};
	char bt_avk_name[25] = "/var/run/bt-avk-fifo";
	char bt_socket_name[30] = "/var/run/bt-daemon-socket";
	BOOLEAN discoverable = true;
	BOOLEAN connectable = true;

	for(i = 0; i < 100; i++) {
		if(!access(bt_socket_name, 0) && !access(bt_avk_name, 0)) {
			break;
		} else {
			usleep(50000);
		}
	}

	if(access(bt_socket_name, 0) || access(bt_avk_name, 0)) {
		printf("%s or %s not exists, please check !!\n",
				bt_avk_name, bt_socket_name);
		goto err_exit;
	}

	int fd = open("/usr/data/mac.txt",O_RDONLY);
	if(fd > 0){
		char id_buf[32] = {0};
		read(fd,id_buf,sizeof(id_buf));
		strncpy(bt_name, id_buf,strlen(id_buf));
		bt_name[strlen(id_buf)] = '\0';
		close(fd);
	}else{
		memset(mac, 0, sizeof(mac));
		memset(bt_name, 0, sizeof(bt_name));
		get_mac_addr("wlan0", mac, "");
		strcat(bt_name, "NS-");
		strcat(bt_name, mac+4);
	}


	
	bt_info_init(&bt_info, bt_name);
#if (SUPPORT_BT == BT_RTK)
	system("bt_enable &");
#elif (SUPPORT_BT == BT_BCM)
	printf("Bluetooth name is: %s\n", bt_name);
	if (mozart_bluetooth_init(&bt_info)) {
		printf("bluetooth init failed.\n");
		goto err_exit;
	}
#if (SUPPORT_BSA_HFP_HF == 1)
	if (mozart_bluetooth_hs_start_service()) {
		printf("hs service start failed.\n");
		goto err_exit;
	}

#endif /* SUPPORT_BSA_HFP_HF */

#if (SUPPORT_BSA_PBC == 1)
	mozart_bluetooth_pbc_start();
#endif /* SUPPORT_BSA_PBC */

#if (SUPPORT_BSA_A2DP_SYNK == 1)
	avk_cback_init_data cback_data;
	cback_data.mozart_avk_evt_cback  = mozart_avk_event_cback;
	cback_data.mozart_avk_data_cback = mozart_avk_data_cback;
	mozart_bluetooth_avk_int_callback(&cback_data);
	if (mozart_bluetooth_avk_start_service()) {
		printf("avk service start failed.\n");
		goto err_exit;
	}
#endif /* SUPPORT_BSA_A2DP_SYNK */

	mozart_bluetooth_set_visibility(discoverable, connectable);
#if (SUPPORT_BSA_A2DP_SOURCE == 1)
	int j = 0;
	int num = 50;
	int disc_num = -1;
	char *playlist_patch = "/mnt/sdcard";
	char *disc_name = "SmartAudio-a23a13e1";
	av_open_param av_param;

	mozart_avk_source_stream_init();
	mozart_bluetooth_av_start_service(playlist_patch);
	for (j = 0; j < 5; j++) {
		mozart_bluetooth_disc_start_regular();
		for (i = 0; i < num; i++) {
			if(bsa_start_regular_enable != 1)
				usleep(200 * 1000);
			else {
				bsa_start_regular_enable = 0;
				break;
			}
		}
		if (i > num) {
			printf("mozart_bluetooth_disc_start_regular failed!\n");
			return NULL;
		}
		if (disc_num == -1) {
			disc_num = mozart_bluetooth_disc_find_device_name(disc_name);
			if (disc_num >= 0) {
				printf("find disc name: %s !!\n", disc_name);
				break;
			} else {
				printf("not find disc_name: %s\n", disc_name);
			}
		}
	}
	if (disc_num != -1) {
		av_param.select_num = 1;  /* Device found in last discovery */
		av_param.device_index = disc_num;
		mozart_bluetooth_av_open(&av_param);
		for (i = 0; i < num; i++) {
			if(bt_link_state != BT_LINK_CONNECTED)
				usleep(200*1000);
			else {
				break;
			}
		}
		if (i > num) {
			printf("mozart_bluetooth_av_open failed!\n");
			return NULL;
		}
	}
#endif /* SUPPORT_BSA_A2DP_SOURCE */

#if (SUPPORT_BSA_OPS == 1)
	ops_accept_param ops_param;
	ops_param.ops_push_dir = "/mnt/sdcard/";
	ops_param.ops_pull_file = "/mnt/sdcard/test.txt";
	mozart_bluetooth_ops_start();
	mozart_bluetooth_ops_auto_accept(&ops_param);
#endif /* SUPPORT_BSA_OPS */

#if (SUPPORT_BSA_OPC == 1)
	char *disc_name = "SmartAudio-a23a144d";
	int disc_num = -1;
	opc_push_param opc_param;
	int j = 0;
	int num = 50;
	mozart_bluetooth_opc_start();
	for (j = 0; j < 5; j++) {
		mozart_bluetooth_disc_start_regular();
		for (i = 0; i < num; i++) {
			if(bsa_start_regular_enable != 1)
				usleep(200 * 1000);
			else {
				bsa_start_regular_enable = 0;
				break;
			}
		}
		if (i >= num) {
			printf("mozart_bluetooth_disc_start_regular failed!\n");
			return NULL;
		}
		if (disc_num == -1) {
			disc_num = mozart_bluetooth_disc_find_device_name(disc_name);
			if (disc_num >= 0) {
				printf("find disc name: %s !!\n", disc_name);
				break;
			} else {
				printf("not find disc_name: %s\n", disc_name);
			}
		}
	}
	opc_param.select_num = 1; /* Device found in last discovery */
	opc_param.device_index = disc_num;
	opc_param.file_path = "/mnt/sdcard/test_file.txt";
	mozart_bluetooth_opc_push_file(&opc_param);
#endif /* SUPPORT_BSA_OPC */

#if (SUPPORT_BSA_SPPS == 1)
	char *service_name = "BluetoothChatSecure";
	dg_start_service_paramters dg_service;
	memset(&dg_service, 0, sizeof(dg_start_service_paramters));
	char *spps_uuid = "00001101-0000-1000-8000-00805F9B34FB"; /* standard 128bit SPP UUID (16bit: 0x1101) */
	//char *spps_uuid = "FA87C0D0-AFAC-11DE-8A39-0800200C9A66"; /* for Android App BluetoothChatSecure */
	sscanf (spps_uuid, "%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x",
			&dg_service.uuid[0], &dg_service.uuid[1], &dg_service.uuid[2], &dg_service.uuid[3],
			&dg_service.uuid[4], &dg_service.uuid[5], &dg_service.uuid[6], &dg_service.uuid[7],
			&dg_service.uuid[8], &dg_service.uuid[9], &dg_service.uuid[10], &dg_service.uuid[11],
			&dg_service.uuid[12], &dg_service.uuid[13], &dg_service.uuid[14], &dg_service.uuid[15]);
	dg_service.param.service = BSA_SPP_SERVICE_ID;
	memcpy(dg_service.param.service_name, service_name, strlen(service_name));
	mozart_bluetooth_dg_start();
	mozart_bluetooth_dg_start_server(&dg_service);
	mozart_bluetooth_dg_callback(mozart_bluetooth_dg_uipc_cback);
#else
	mozart_bluetooth_dg_callback(NULL);
#endif /* SUPPORT_BSA_SPPS */

#if (SUPPORT_BSA_SPPC == 1)
	int j = 0;
	int num = 50;
	int disc_num = -1;
	char *disc_name = "SmartAudio-a23a144d";

	mozart_bluetooth_dg_start();
	for (j = 0; j < 5; j++) {
		mozart_bluetooth_disc_start_services(BSA_SPP_SERVICE_MASK);
		while(bsa_start_regular_enable != 1) {
			usleep(200 * 1000);
		}
		bsa_start_regular_enable = 0;
		if (disc_num == -1) {
			disc_num = mozart_bluetooth_disc_find_device_name(disc_name);
			if (disc_num >= 0) {
				printf("find disc name: %s !!\n", disc_name);
				break;
			} else {
				printf("not find disc_name: %s\n", disc_name);
			}
		}
	}
	if (disc_num != -1) {
		int i = 0;
		BOOLEAN flag = 0;
		char connect_uuid[] = {0x00, 0x00, 0x11, 0x01, 0x00, 0x00, 0x10, 0x00,
			0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB}; /* standard 128bit SPP UUID (16bit: 0x1101) */
		/* char connect_uuid[] = {0xFA, 0x87, 0xC0, 0xD0, 0xAF, 0xAC, 0x11, 0xDE,
			0x8A, 0x39, 0x08, 0x00, 0x20, 0x0C, 0x9A, 0x66}; */ /* for Android App BluetoothChatSecure */

		dg_open_paramters dg_parm = {};
		dg_parm.connect_type = DEVICE_FROM_DISCOVERY;
		dg_parm.device_index = disc_num;
		dg_parm.param.service = BSA_SPP_SERVICE_ID;
		flag = sdpu_is_base_uuid(connect_uuid);
		if (flag) {
			printf("uuid is base uuid !!\n");
			dg_parm.is_128uuid = false;
		} else {
			char *connect_name = "BluetoothChatSecure";
			printf("uuid is not base uuid !!\n");
			for (i = 0; i < MAX_UUID_SIZE; i++) {
				dg_parm.uuid[i] = connect_uuid[i];
			}
			memcpy(dg_parm.param.service_name, connect_name, strlen(connect_name));
			dg_parm.is_128uuid = true;
		}
		dg_parm.param.sec_mask = BSA_SEC_NONE;
		for (j = 0; j < 5; j++) {
			bt_dg_flag = DG_IDLE;
			mozart_bluetooth_dg_open_connection(&dg_parm);
			for (i = 0; i < num; i++) {
				if(bt_dg_flag == DG_IDLE)
					usleep(200 * 1000);
				else if (bt_dg_flag == DG_CONNECTED) {
					goto connect_success;
				} else if (bt_dg_flag == DG_CONNECT_FAILED) {
					break;
				}
			}
		}
connect_success:
		bt_dg_flag = DG_IDLE;
	}
#endif /* SUPPORT_BSA_SPPC */

#if (SUPPORT_BSA_HS_RESAMPLE == 1)
	mozart_hs_get_default_sampledata(&hs_sample_data);
#if (SUPPORT_BSA_HS_RESAMPLE_8K_to_48K == 1)
	hs_resample_data.resample_rate = 48000;
	hs_resample_s = mozart_resample_init(hs_sample_data.sample_rate, hs_sample_data.sample_channel, hs_sample_data.sample_bits>>3, hs_resample_data.resample_rate);
	if(hs_resample_s == NULL){
		printf("ERROR: mozart resample inti failed\n");
		goto err_exit;
	}
#else
	hs_resample_data.resample_rate = hs_sample_data.sample_rate;
#endif /* SUPPORT_BSA_HS_RESAMPLE_8K_to_48K */

	hs_resample_data.resample_bits = hs_sample_data.sample_bits;
	hs_resample_data.resample_channel = hs_sample_data.sample_channel;
	hs_resample_data.resample_enable = 1;
	hs_resample_data.mozart_hs_resample_data_cback = mozart_hs_resample_data_callback;

	mozart_bluetooth_hs_set_resampledata_callback(&hs_resample_data);
#endif /* SUPPORT_BSA_HS_RESAMPLE */

#if ((SUPPORT_AEC_RESAMPLE == 1) && (SUPPORT_AEC_RESAMPLE_48K_TO_8K == 1))
	mozart_aec_get_bt_default_sampledata(&bt_aec_sdata);
	bt_aec_rdata.resample_rate = bt_aec_sdata.sample_rate;
	bt_aec_rdata.resample_bits = bt_aec_sdata.sample_bits;
	bt_aec_rdata.resample_channel = bt_aec_sdata.sample_channel;
	bt_aec_rdata.resample_time = 10; /* ms, resample_time is must equal to record time */

	bt_aec_sdata.sample_rate = hs_resample_data.resample_rate;
	bt_aec_sdata.sample_bits = hs_resample_data.resample_bits;
	bt_aec_sdata.sample_channel = hs_resample_data.resample_channel;
	aec_resample_s = mozart_resample_init(bt_aec_sdata.sample_rate, bt_aec_sdata.sample_channel, bt_aec_sdata.sample_bits>>3, bt_aec_rdata.resample_rate);
	if(aec_resample_s == NULL) {
		printf("ERROR: mozart resample inti failed\n");
		goto err_exit;
	}

	bt_aec_rdata.resample_enable = 1;
	bt_aec_rdata.aec_resample_data_cback = mozart_bt_aec_resample_data_callback;
	mozart_aec_set_bt_resampledata_callback(&bt_aec_rdata);
#endif /* SUPPORT_AEC_RESAMPLE */
#if (SUPPORT_BSA_BLE == 1)
	int state = 0;
	usleep(500*1000);

	state = mozart_bluetooth_ble_start();
	if (state) {
		printf("mozart_bluetooth_ble_start failed, state = %d.\n", state);
		goto err_exit;
	}
#if (SUPPORT_BSA_BLE_SERVER == 1)
	/* create ble service */
	state = ble_service_and_char_create();
	if (state != 0) {
		printf("ERROR: ble_service_and_char_create failed !\n");
		goto err_exit;
	}
	state = ble_netconfig_service_and_char_create();
	if (state != 0) {
		printf("ERROR: ble_netconfig_service_and_char_create failed !\n");
		goto err_exit;
	}
#endif	/* SUPPORT_BSA_BLE_SERVER */
	mozart_bluetooth_ble_set_visibility(discoverable, connectable);
#if (SUPPORT_BSA_BLE_CLIENT == 1)
#if (SUPPORT_BSA_BLE_HH == 1)
#if (SUPPORT_BSA_BLE_HH_DIALOG_AUDIO == 1)
	dialog_init();
#endif /* SUPPORT_BSA_BLE_HH_DIALOG_AUDIO */
	mozart_bluetooth_hh_callback(mozart_bluetooth_hh_uipc_cback);
	/* create ble_hid */
	state = ble_hh_create();
	if (state != 0) {
		printf("ERROR: ble_hh_create failed !\n");
	}
#else
	/* create ble client */
	//state = ble_client_create();
	//if (state != 0) {
	//	printf("ERROR: ble_client_create failed !\n");
	//}
#endif /* SUPPORT_BSA_BLE_HH */
#endif /* SUPPORT_BSA_BLE_CLIENT */
#endif /* SUPPORT_BSA_BLE */
#else  /* SUPPORT_BT */
	printf("Bt funcs are closed.\n");
	goto err_exit;
#endif /* SUPPORT_BT */

err_exit:

	return NULL;
}



int start_bt(void)
{
	printf("****************start_bt********************\n");
	int err;
	pthread_t p_tid;
	system("/usr/fs/etc/init.d/S04bsa.sh start");
#if (SUPPORT_WEBRTC == 1)
	bt_ac.aec_init = (aec_func_t)ingenic_apm_init;
	bt_ac.aec_destroy = (aec_func_t)ingenic_apm_destroy;
	bt_ac.aec_enable = (aec_func_t)webrtc_aec_enable;
	bt_ac.aec_get_buffer_length = (aec_func_t)webrtc_aec_get_buffer_length;
	bt_ac.aec_calculate = webrtc_aec_calculate;

	mozart_aec_callback(&bt_ac);
#else
	mozart_aec_callback(NULL);
#endif

#if (SUPPORT_BSA_BLE == 1)
	bt_ble_callback ble_cback = {};
#if (SUPPORT_BSA_BLE_CLIENT == 1)
	ble_cback.ble_common_profile_cback = ble_common_profile_cback;
#endif
#if (SUPPORT_BSA_BLE_SERVER == 1)
	ble_cback.ble_ser_write_cback = bt_ble_server_write_cback;
#endif
	mozart_bluetooth_ble_callback(&ble_cback);
#else
	mozart_bluetooth_ble_callback(NULL);
#endif


	err = pthread_create(&p_tid, NULL, thr_fn, NULL);
	if (err != 0)
		printf("can't create thread: %s\n", strerror(err));

	pthread_detach(p_tid);

	return 0;
}

int stop_bt(void)
{
	bluetooth_cancel_auto_reconnect_pthread();
	mozart_bluetooth_disconnect(USE_HS_AVK);
#if (SUPPORT_BSA_HFP_HF == 1)
	mozart_bluetooth_hs_stop_service();
#endif

#if (SUPPORT_BSA_A2DP_SYNK == 1)
	mozart_bluetooth_avk_stop_service();
#endif

#if (SUPPORT_BSA_PBC == 1)
	mozart_bluetooth_pbc_stop();
#endif
	mozart_bluetooth_uninit();
#if ((SUPPORT_BSA_HS_RESAMPLE == 1) && (SUPPORT_BSA_HS_RESAMPLE_8K_to_48K == 1))
	mozart_resample_uninit(hs_resample_s);
#endif
#if ((SUPPORT_AEC_RESAMPLE == 1) && (SUPPORT_AEC_RESAMPLE_48K_TO_8K == 1))
	mozart_resample_uninit(aec_resample_s);
#endif
	system("/usr/fs/etc/init.d/S04bsa.sh stop");

	return 0;
}
