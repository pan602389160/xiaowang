// callbackVideo.cpp 
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <unistd.h>

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <getopt.h>      
#include <fcntl.h>
#include <malloc.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/ioctl.h>

#include <linux/videodev2.h>
//#include "videodev2.h"

#include "pgLibDevVideoIn.h"
#include "callbackVideo.h"


struct buffer {
        void *start;
        size_t	length;
};
struct buffer *buffers;

static int fd = -1;
static unsigned int n_buffers = 3;

FILE *file_fd;

extern unsigned int uVideoMode;

// Video size list.
struct VIDEO_SIZE_S {
	unsigned int uWidth;
	unsigned int uHeight;
};

static struct VIDEO_SIZE_S s_stVideoSizeList[] = {
	{80,   60},
	{160,  120},
	{320,  240},
	{640,  480},
	{800,  600},
	{1024, 768},
	{176,  144},
	{352,  288},
	{704,  576},
	{854,  480},
	{1280, 720},
	{1920, 1080}
};

static unsigned int s_uVideoMode = 0;


static int s_iDevID_VideoIn = -1;	
static pthread_t s_tThread_VideoIn;
static unsigned int s_uRunning_VideoIn = 0;


static int video_set_framerate(int dev)
{
	struct v4l2_streamparm parm;
	int ret;

	memset(&parm, 0, sizeof parm);
	parm.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

	ret = ioctl(dev, VIDIOC_G_PARM, &parm);
	if (ret < 0) {
		printf("Unable to get frame rate: %d.\n", errno);
		return ret;
	}

	printf("Current frame rate: %u/%u\n",
		parm.parm.capture.timeperframe.numerator,
		parm.parm.capture.timeperframe.denominator);

	parm.parm.capture.timeperframe.numerator = 1;
	parm.parm.capture.timeperframe.denominator = 20;

	ret = ioctl(dev, VIDIOC_S_PARM, &parm);
	if (ret < 0) {
		printf("Unable to set frame rate: %d.\n", errno);
		return ret;
	}

	ret = ioctl(dev, VIDIOC_G_PARM, &parm);
	if (ret < 0) {
		printf("Unable to get frame rate: %d.\n", errno);
		return ret;
	}

	printf("Frame rate set: %u/%u\n",
		parm.parm.capture.timeperframe.numerator,
		parm.parm.capture.timeperframe.denominator);
	return 0;
}


///
// In this demo, we use a thread to simulate video input.



static void* VideoInputThreadProc(void* lp)
{
	printf("VideoInputThreadProc\n");

	if (pthread_detach(pthread_self()) != 0) {
		printf("VideoInputThreadProc, err=%d", errno);
	}
 
#if 1

//	file_fd = fopen("test-mmap2.jpg", "w");

	fd = open("/dev/video0", O_RDWR, 0);	
	if (fd<0)
	{
		printf("open error\n");
		return  NULL;
	}
	struct v4l2_format fmt;							
	memset( &fmt, 0, sizeof(fmt));
	fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	if(uVideoMode == 10){	
		fmt.fmt.pix.width = 1280;							
		fmt.fmt.pix.height =720;
	}
	if(uVideoMode == 3){	
		fmt.fmt.pix.width = 640;							
		fmt.fmt.pix.height =480;
	}
								
	fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_MJPEG;	
	fmt.fmt.pix.field = V4L2_FIELD_ANY;
	if (ioctl(fd, VIDIOC_S_FMT, &fmt) < 0) 	
	{
		printf("set format failed\n");
	//	return -1;
	}

	/* Set the frame rate. */
//	if (video_set_framerate(fd) < 0) {
//		close(fd);
	//	return 1;
//	}
	
	struct v4l2_requestbuffers req;				 //申请帧缓冲
	memset(&req, 0, sizeof (req));
	req.count = 1; 						 //缓存数量，即可保存的图片数量
	req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;			 //数据流类型，永远都是V4L2_BUF_TYPE_VIDEO_CAPTURE
	req.memory = V4L2_MEMORY_MMAP; 				 //存储类型：V4L2_MEMORY_MMAP或V4L2_MEMORY_USERPTR
	if (ioctl(fd, VIDIOC_REQBUFS, &req) == -1) 		 //使配置生效
	{
		perror("request buffer error \n");
	//	return -1;
	}
	buffers = (struct buffer *)calloc(req.count, sizeof(*buffers));//内存中建立对应空间	

	struct v4l2_buffer buf;
	 
	memset( &buf, 0, sizeof(buf));
	buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;		 //数据流类型，永远都是V4L2_BUF_TYPE_VIDEO_CAPTURE
	buf.memory = V4L2_MEMORY_MMAP; 			 //存储类型：V4L2_MEMORY_MMAP（内存映射）或V4L2_MEMORY_USERPTR（用户指针）
	buf.index = 0;
	if (ioctl(fd, VIDIOC_QUERYBUF, &buf) < 0)		 //使配置生效
	{
		printf("VIDIOC_QUERYBUF error\n");
	//	return -1;
	}
	buffers[0].length = buf.length;
	buffers[0].start = mmap(NULL, buf.length, PROT_READ | PROT_WRITE, MAP_SHARED, fd, buf.m.offset); 		 
	if (buffers[0].start == MAP_FAILED)
	{
		perror("buffers error\n");
	//	return -1;
	}
	if (ioctl(fd, VIDIOC_QBUF, &buf) < 0)			 //放入缓存队列
	{
		printf("VIDIOC_QBUF error\n");
	//	return -1;
	}
	enum v4l2_buf_type type;					 //开始视频显示
	type = V4L2_BUF_TYPE_VIDEO_CAPTURE;			 //数据流类型，永远都是V4L2_BUF_TYPE_VIDEO_CAPTURE
	if (ioctl(fd, VIDIOC_STREAMON, &type) < 0)
	{
		printf("VIDIOC_STREAMON error\n");
	//	return -1;
	}

//	unsigned int cap_flag = 0;
	unsigned int sps_frm=0;
	unsigned char ucData[10] = {0};
//	unsigned char *ucData = (unsigned char *)malloc(614400*sizeof(unsigned char));

	while (s_uRunning_VideoIn != 0) {
		
//		CLEAR (buf);
		buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf.memory = V4L2_MEMORY_MMAP;
		
		ioctl (fd, VIDIOC_DQBUF, &buf); //出列采集的帧缓冲

	//	if(cap_flag == 0){
	//		fwrite(buffers[buf.index].start, buffers[buf.index].length, 1, file_fd); //将其写入文件中
	//		cap_flag = 1;
	//	}
		//printf("buffers[0].length = %d, buf.bytesused = %dKbits\n",buffers[0].length,buf.bytesused*8/1024);

		unsigned int uFlag = 1;
		
	//	memcpy(ucData,buffers[buf.index].start,10);

	//	printf("ucData[4] = 0x%x\n", ucData[4]);
		
	//	if (ucData[4] == 0x65) { // H.264 Key frame (Has a SPS frame head).
	//		uFlag |= PG_DEV_VIDEO_IN_FLAG_KEY_FRAME;
	//	}
		
		pgDevVideoInCaptureProc(s_iDevID_VideoIn,  buffers[0].start, buf.bytesused, PG_DEV_VIDEO_IN_FMT_MJPEG, uFlag);

		ioctl (fd, VIDIOC_QBUF, &buf); //再将其入列
		
		//Sleep frame interval: 500ms, as 2fps.
		usleep(80 * 1000);
	}

	
#endif
#if 0
	unsigned int uDataSize = 0;
//	unsigned char ucData[615424] = {0};
	unsigned char *ucData = (unsigned char *)malloc(614400*sizeof(unsigned char));
	
	int ifrmSize = -1;

	FILE* pFile = fopen("RecordH264.h264", "rb");
	if (pFile != NULL) {
		fseek(pFile, 0, SEEK_END);
		ifrmSize = ftell(pFile);
	}


	while (s_uRunning_VideoIn != 0) {

		// Get H.264 video input frame data from codec.
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

		unsigned uFlag = 1;
		if (ucData[4] == 0x67) { // H.264 Key frame (Has a SPS frame head).
			uFlag |= PG_DEV_VIDEO_IN_FLAG_KEY_FRAME;
		}

		// Call peergine video input API.
		printf("*******Call peergine video input API******uDataSize = %d ******* \n",uDataSize);
		pgDevVideoInCaptureProc(s_iDevID_VideoIn, ucData, uDataSize, PG_DEV_VIDEO_IN_FMT_H264, uFlag);
		
    	// Sleep frame interval: 500ms, as 2fps.
		usleep(500 * 1000);
	}

	if (pFile != NULL) {
		fclose(pFile);
	}
#endif

	printf("VideoInputThreadProc\n");
	pthread_exit(NULL);
}

///
// Video input callback functions.
static int VideoInOpen(unsigned int uDevNO, unsigned int uPixBytes,
	unsigned int uWidth, unsigned int uHeight, unsigned int uBitRate,
	unsigned int uFrmRate, unsigned int uKeyFrmRate)
{
	printf("VideoInOpen: uDevNO=%u,uPixBytes = %u, uWidth=%u, uHeight=%u, uBitRate=%u, uFrmRate=%u, uKeyFrmRate=%u\n",
		uDevNO,uPixBytes,uWidth, uHeight, uBitRate, uFrmRate, uKeyFrmRate);
	
	// Check video size.
	if (uWidth != s_stVideoSizeList[s_uVideoMode].uWidth
		|| uHeight != s_stVideoSizeList[s_uVideoMode].uHeight)
	{
		printf("VideoInOpen: uDevNO=%u, invalid video size.\n", uDevNO);	
		return -1;
	}
	
	// Start to capture video ...

	// In this demo, we use a thread to simulate video input.
	pthread_attr_t attr;
	pthread_attr_init(&attr);

	s_uRunning_VideoIn = 1;

	int iRet = pthread_create(&s_tThread_VideoIn, &attr, VideoInputThreadProc, 0);
	if (iRet != 0) {
		s_uRunning_VideoIn = 0;
	}
	else {
		s_iDevID_VideoIn = 1234;
	}

	pthread_attr_destroy(&attr);

	return s_iDevID_VideoIn;
}

static void VideoInClose(int iDevID)
{
	printf("VideoInClose: iDevID=%d\n", iDevID);

	enum v4l2_buf_type type;					
	type = V4L2_BUF_TYPE_VIDEO_CAPTURE;			
	if (ioctl(fd, VIDIOC_STREAMOFF, &type) < 0)
	{
		printf("VIDIOC_STREAMON error\n");
	}

	if (-1 == munmap(buffers[0].start, buffers[0].length))
		printf("munmap error!\n");

	free(buffers);
	int ret = close(fd);
	printf("VideoInClose ret:%d\n",ret);
	// Stop capture video ...
	
	if (iDevID == s_iDevID_VideoIn) {
		s_uRunning_VideoIn = 0;
		s_iDevID_VideoIn = -1;
	}
}


static void VideoInCtrl(int iDevID, unsigned int uCtrl, unsigned int uParam)
{
	printf("VideoInCtrl: iDevID=%d, uCtrl=%u\n", iDevID, uCtrl);

	if (uCtrl == PG_DEV_VIDEO_IN_CTRL_PULL_KEY_FRAME) {
		// Force the encoder to output a key frame immediately.
		// ...
	}
}

static PG_DEV_VIDEO_IN_CALLBACK_S s_stCallback_VideoIn = {
	VideoInOpen,
	VideoInClose,
	VideoInCtrl
};

void RegisterVideoCallback(void)
{
	pgDevVideoInSetCallback(&s_stCallback_VideoIn);  
}

void SetVideoCallbackParam(unsigned int uMode)
{
	if (uMode < (sizeof(s_stVideoSizeList) / sizeof(s_stVideoSizeList[0]))) {
		s_uVideoMode = uMode;
	}
}
