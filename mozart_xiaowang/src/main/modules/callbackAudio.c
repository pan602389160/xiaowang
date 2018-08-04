// callbackAudio.cpp 
//

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

#include "pgLibDevAudioIn.h"
#include "pgLibDevAudioOut.h"
#include "pgLibDevAudioConvert.h"
#include "callbackAudio.h"


#include "callbackAudioQueue.h"
#include "hardware_api.h"




// define the queue instance.
static PG_DEV_CALLBACK_AUDIO_QUEUE_S s_stQueue = PG_DEV_AUDIO_QUEUE_INIT();


//------------------------------------------------------------------------------
// Audio input callback

static int HardwareAudioRecordCallback(void *context, void *data, int len)
{
	if (pgDevAudioQueueRecordPush(&s_stQueue, PG_DEV_AUDIO_CVT_FMT_PCM16, data, len) <= 0) {
		return 0;
	}

	return 0;
}

static int AudioInOpen(unsigned int uDevNO, unsigned int uSampleBits,
	unsigned int uSampleRate, unsigned int uChannels, unsigned int uPackBytes)
{
	sleep(1);
	printf("AudioInOpen: uDevNO=%u, uSampleBits=%u, uSampleRate=%u, uChannels=%u,uPackBytes=%u\n",
		uDevNO, uSampleBits, uSampleRate, uChannels,uPackBytes);
	//                                                                 11025 441,16000 640
	int iDevIDIn = pgDevAudioQueueRecordOpen(&s_stQueue, PG_DEV_AUDIO_CVT_FMT_PCM16, 16000, 640, 0);
	if (iDevIDIn <= 0) {
		printf("AudioInOpen: Alloc audio convert failed.\n");
		return -1;
	}

	// Open the audio record device.
	if (!hardware_audio_record_open(0, uDevNO, uSampleRate,
		(uPackBytes / 2), HardwareAudioRecordCallback))
	{
		pgDevAudioQueueRecordClose(&s_stQueue);
		iDevIDIn = -1;
	}
	
	return iDevIDIn;
}

static void AudioInClose(int iDevID)
{
	printf("AudioInClose: iDevID=%d\n", iDevID);

	// Close the audio record device.
	hardware_audio_record_close(0);

	pgDevAudioQueueRecordClose(&s_stQueue);
}

static PG_DEV_AUDIO_IN_CALLBACK_S s_stCallback_AudioIn = {
	AudioInOpen,
	AudioInClose
};
 

//------------------------------------------------------------------------------
// Audio output callback  s_stQueue.iDevIDOut
// uDevNO=0, uSampleBits=16, uSampleRate=11025, uChannels=1,uPackBytes=882


#if 1
int set_param(int fd)  
{  
	int param; 
	int ret = -1; 
	printf("set params!\n");
 	//param = 0xnnnssss;
	//ret = ioctl(fd,SNDCTL_DSP_SETFRAGMENT,&param); //设置内核缓冲区大小
	//if(ret)
	//	return 1; 

	param = 0;
	ret = ioctl(fd,SNDCTL_DSP_STEREO,&param); //设置声道
	if(ret){
		return 2;  
	}
	printf("------------SNDCTL_DSP_CHANNELS param = %d\n",param);
	param = AFMT_S16_LE;    
	ret = ioctl(fd,SNDCTL_DSP_SETFMT,&param); 
	if(ret){
		
		return 3;  
	}
	printf("--------------SNDCTL_DSP_SETFMT param = %x\n",param);
	param = 16000;    
	ret = ioctl(fd,SNDCTL_DSP_SPEED,&param); 
	if(ret){
		
		return 4;  
	}
	printf("--------------SNDCTL_DSP_SPEED param = %d\n",param);
	return 0; 
}


int dsp_fd = -1;
int ioctl_ret = -1;
#endif
static int AudioOutOpen(unsigned int uDevNO, unsigned int uSampleBits,
	unsigned int uSampleRate, unsigned int uChannels, unsigned int uPackBytes)
{
	sleep(1);
	printf("AudioOutOpen: uDevNO=%u, uSampleBits=%u, uSampleRate=%u, uChannels=%u,uPackBytes=%u\n",
		uDevNO, uSampleBits, uSampleRate, uChannels,uPackBytes);
	int ret = 0;
	while(ret++ < 4){
		dsp_fd = open("/dev/dsp", O_WRONLY);
		if(dsp_fd > 0)
			break;
		else{
			printf("###############################################dsp_fd = %d,ret = %d\n",dsp_fd,ret);
			sleep(1);
		}
	}
	if(ret > 3)
		return -1;
	printf("###############################################dsp_fd = %d\n",dsp_fd);
	if(dsp_fd)
		ioctl_ret = set_param(dsp_fd);

	int iDevIDOut = pgDevAudioQueuePlayOpen(&s_stQueue, PG_DEV_AUDIO_CVT_FMT_PCM16, 16000,640);
	if (iDevIDOut <= 0) {
		printf("AudioOutOpen: Alloc audio convert failed.\n");
		return -1;
	}

 	return iDevIDOut;
}

static void AudioOutClose(int iDevID)
{
 	printf("AudioOutClose: iDevID=%d\n", iDevID);
 
	pgDevAudioQueuePlayClose(&s_stQueue);

	// Close the audio playing device.
	//hardware_audio_play_close(0);
	if(dsp_fd)
		close(dsp_fd);
}
 
static int AudioOutPlay(int iDevID, const void* lpData, unsigned int uDataSize, unsigned int uFormat)
{	
	//fwrite(lpData,1,882,fp_11025K);
	int iRet = pgDevAudioQueuePlayPush(&s_stQueue, uFormat, lpData, uDataSize);// 把待转换的音频数据压入到队列中
	if (iRet <= 0) {
		printf("AudioOutPlay: Audio convert push failed.\n");
 	  	return -1;
	}
	iRet = -1;

	unsigned char ucDataPop[1280] = {0};
	unsigned char transfomer_data[3840] = {0};//6144 9216
	int index = 0;
	int cnt2 = 0;
	int loop_cnt = 0;
	int inside_cnt = 0;
	int loop_index = 0;
	int iPopSize = pgDevAudioQueuePlayPop(&s_stQueue, ucDataPop, sizeof(ucDataPop)); // 把转换后的数据从队列弹出
	if (iPopSize > 0) {
		// Output data to device.
		#if 0
		printf("转换后 [%d] [%d] [%d] [%d] [%d] [%d] [%d] [%d] [%d] [%d]----------------->[%d]\n",
				ucDataPop[0],ucDataPop[1],ucDataPop[2],ucDataPop[3],ucDataPop[4],
				ucDataPop[5],ucDataPop[6],ucDataPop[7],ucDataPop[8],ucDataPop[9],iPopSize);
		#endif
		//fwrite(ucDataPop,1,1280,fp_16K);
		for(index = 0;index < 3840;index++)//--------
		{
			loop_cnt = index/6;//----------
			inside_cnt = index%2;//-----------

			if(inside_cnt != 2)//-------
				transfomer_data[index] = ucDataPop[2*loop_cnt + inside_cnt];
			else
				transfomer_data[index] = transfomer_data[index - 1];
		}
		if(dsp_fd)
			iRet = write(dsp_fd, transfomer_data, 3840);//----------
		if(iRet < 0)
			printf("----->>>iRet = %d\n",iRet);
		
	}

 	return uDataSize;
}

static PG_DEV_AUDIO_OUT_CALLBACK_S s_stACallback_AudioOut = {
	AudioOutOpen,
	AudioOutClose,
	AudioOutPlay
};


void RegisterAudioCallback(void)
{
	pgDevAudioInSetCallback(&s_stCallback_AudioIn);
	pgDevAudioOutSetCallback(&s_stACallback_AudioOut);
}



