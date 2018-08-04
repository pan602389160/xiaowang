
/*
 * It is the demo of hardware I/O api.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <unistd.h>

#include <sys/ioctl.h>
#include <sys/types.h>
#include <linux/soundcard.h>
#include <fcntl.h>


#include "hardware_api.h"


//----------------------------------------------------------
// Video capture api.

static pthread_t s_threadVideoIn;
static unsigned int s_runningVideoIn = 0;
static video_capture_callback s_videoCaptureCallback = 0;

///
// In this demo, we use a thread to simulate video input.
static void* hardware_video_capture_thread_proc(void* lp)
{
	printf("hardware_video_capture_thread_proc: start\n");

	if (pthread_detach(pthread_self()) != 0) {
		printf("hardware_video_capture_thread_proc, err=%d", errno);
	}
 
	unsigned int uDataSize = 0;
	unsigned char ucData[16384] = {0};
	
	int ifrmSize = -1;
	
	FILE* pFile = fopen("one_h264_frm.frm", "rb");
	if (pFile != NULL) {
		fseek(pFile, 0, SEEK_END);
		ifrmSize = ftell(pFile);
	}

	while (s_runningVideoIn != 0) {

		// Get H.264 video frame data from file "one_h264_frm.frm".
		if (pFile != NULL) {
			fseek(pFile, 0, SEEK_SET);
			int iRead = fread(ucData, ifrmSize, 1, pFile);
			if (iRead > 0 || feof(pFile)) {
				uDataSize = ifrmSize; //
			}
			else {
				uDataSize = 1024; // assume the H.264 frame size.
			}
		}
		else {
			uDataSize = 1024; // assume the H.264 frame size.
		}

		unsigned int uKeyFrame = 0;
		if (ucData[4] == 0x67) {
			// H.264 Key frame (Must has a SPS frame head).
			uKeyFrame = 1;
		}

		// invoke callback function.
		if (s_videoCaptureCallback != 0) {
			s_videoCaptureCallback(0, ucData, uDataSize, uKeyFrame);
		}

    	// Sleep frame interval: 500ms, as 2fps.
		usleep(500 * 1000);
	}

	if (pFile != NULL) {
		fclose(pFile);
	}
	
	printf("hardware_video_capture_thread_proc: stop\n");
	pthread_exit(NULL);
}

int hardware_video_capture_open(void **context, int camera_no, int width, int height,
	int bit_rate, int frame_rate, int key_frame_rate, video_capture_callback callback_func)
{
	s_videoCaptureCallback = callback_func;

	// TODO: Open the hardware video capture device.

	// In this demo: we use a thread to simulate video capture.
	pthread_attr_t attr;
	pthread_attr_init(&attr);

	s_runningVideoIn = 1;
	int iRet = pthread_create(&s_threadVideoIn, &attr, hardware_video_capture_thread_proc, 0);
	if (iRet != 0) {
		s_runningVideoIn = 0;
		s_videoCaptureCallback = 0;
	}

	pthread_attr_destroy(&attr);

	return (s_runningVideoIn != 0);
}

int hardware_video_capture_force_key_frame(void *context)
{
	// TODO: Force the hardware encoder to output a key frame immediately.

	return 1;
}

void hardware_video_capture_close(void *context)
{
	// TODO: Close the hardware video capture device.

	// In this demo: break the simulate video capture thread.
	s_runningVideoIn = 0;
}



#define HW_AUDIO_BUF_DATA_SIZE  (12288 * 2 * 4)
#define HW_AUDIO_LOOP_BUF_NUM   4



//----------------------------------------------------------
// Audio record api.

static pthread_t s_threadAudioIn;	
static unsigned int	s_runningAudioIn = 0;
static audio_record_callback s_audioRecordCallback = 0;


#if 1

int set_param_In(int fd)  
{  
	int param; 
	int ret = -1; 
	printf("set params!\n");

	param = 0;
	ret = ioctl(fd,SNDCTL_DSP_STEREO,&param); //设置声道
	if(ret){
		return 2;  
	}
	printf("------------set_param_In--SNDCTL_DSP_CHANNELS param = %d\n",param);
	param = AFMT_S16_LE;    
	ret = ioctl(fd,SNDCTL_DSP_SETFMT,&param); 
	if(ret){
		return 3;  
	}
	printf("-------------set_param_In--SNDCTL_DSP_SETFMT param = %x\n",param);
	param = 16000;    
	ret = ioctl(fd,SNDCTL_DSP_SPEED,&param); 
	if(ret){
		return 4;  
	}
	printf("-------------set_param_In-SNDCTL_DSP_SPEED param = %d\n",param);
	return 0; 
}


#endif
int dsp3_fd = -1;
///
// In this demo, we use a thread to simulate Audio record.

static void* hardware_audio_record_thread_proc(void* lp)
{
  	printf("hardware_audio_record_thread_proc: start\n");
	int ret = 0;
	while(ret++ < 4){
		dsp3_fd = open("/dev/dsp3", O_RDONLY);
		if(dsp3_fd > 0)
			break;
		else{
			printf("############################dsp3_fd = %d,ret = %d\n",dsp3_fd,ret);
			sleep(1);
		}
	}
	if(ret > 3)
		return -1;
	printf("############################dsp3_fd = %d\n",dsp3_fd);

	if(dsp3_fd)
		set_param_In(dsp3_fd);

	unsigned char read_buf[1280];
	if (pthread_detach(pthread_self()) != 0) {
		printf("hardware_audio_record_thread_proc, err=%d", errno);
	}

	while (s_runningAudioIn != 0) {

		memset(read_buf,0,sizeof(read_buf));
		if(dsp3_fd)
			ret = read(dsp3_fd, read_buf, sizeof(read_buf));
		//printf("ret = %d,dsp3_fd = %d,set_ret = %d\n",ret,dsp3_fd,set_ret);
		if (s_audioRecordCallback != 0) {
				s_audioRecordCallback(0, read_buf, sizeof(read_buf));
		}
		usleep(40 * 1000);
	}
	if(dsp3_fd)
		close(dsp3_fd);
  	printf("hardware_audio_record_thread_proc: stop\n");

	pthread_exit(NULL);
}


int hardware_audio_record_open(void **context, int microphone_no,
	int sample_rate, int frame_len, audio_record_callback callback_func)
{
	s_audioRecordCallback = callback_func;

	// TODO: Open the hardware audio record device.
	
	// In this demo: we use a thread to simulate audio record.
	pthread_attr_t attr;
	pthread_attr_init(&attr);

	s_runningAudioIn = 1;
	int iRet = pthread_create(&s_threadAudioIn, &attr, hardware_audio_record_thread_proc, 0);
	if (iRet != 0) {
		s_runningAudioIn = 0;
		s_audioRecordCallback = 0;
	}

	pthread_attr_destroy(&attr);

	return (s_runningAudioIn != 0);
}

void hardware_audio_record_close(void *context)
{
	// TODO: Close the hardware audio record device.

	// In this demo: break the simulate record thread.
	s_runningAudioIn = 0;
}


//----------------------------------------------------------

int hardware_audio_play_open(void **context, int speaker_no, int sample_rate, int frame_len)
{

}

int hardware_audio_play_write_data(void *context, void *data, int len)
{

}

void hardware_audio_play_close(void *context)
{

}
