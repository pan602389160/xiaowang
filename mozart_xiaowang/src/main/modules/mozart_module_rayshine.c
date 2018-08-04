#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <unistd.h>
#include <linux/videodev2.h>
#if 1
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#endif


#include "modules/mozart_module_local_music.h"
#include "pgLibLiveMultiError.h"
#include "pgLibLiveMultiCapture.h"
#include "callbackVideo.h"
#include "callbackAudio.h"
#include "json-c/json.h"

#include "mozart_app.h"
#include "modules/mozart_module_vr.h"
#include "tips_interface.h"
#include "pgLibDevVideoIn.h"
#include "callbackVideo.h"
extern bool vr_started;
unsigned int uVideoMode = 3;	//0x2 is 320x240 [ 3 is 640*480, 10 is 1280*720 ]
unsigned int g_set_definetion = 0;
unsigned int g_get_definetion = 0;
char szVideoParam[256] = { 0 };
int g_video_start = 0;//标识录像是否开启，避免多重录像开启
int g_video_restart = 0;
int g_RenderLeave = 1;
void LogOutput(unsigned int uLevel, const char* lpszOut)
{
//	printf("lpszOut:{%s}\n",lpszOut);
}



#define LIGHT_PURPLE "\033[1;35m"
#define LIGHT_GREEN "\033[1;32m"
#define NONE          "\033[m"
#define my_pr_red(fmt, args...)		printf(LIGHT_PURPLE fmt NONE,##args)
#define my_pr_green(fmt, args...)		printf(LIGHT_GREEN fmt NONE,##args)

extern int dsp_fd;

extern int dsp3_fd;


extern int fd;

void EventProc(unsigned int uInstID, const char* lpszAction, const char* lpszData, const char* lpszRenID)
{
	if (strcmp(lpszAction, "Message") == 0) {
		printf("MESSAGE: szData=%s, szRender=%s\n", lpszData, lpszRenID);
		
		int iRet = -1;
		json_object *json_msg= json_tokener_parse(lpszData);
		json_object *json_command = NULL;
		json_object *json_value = NULL;
		json_object_object_get_ex(json_msg,"command",&json_command);
		if(strcmp("get_video_definition",json_object_get_string(json_command)) == 0){//获取分辨率
			int value = -1;
			if(uVideoMode == 3)value = 0;
			if(uVideoMode == 10)value = 1;
			g_get_definetion = 1;
			json_object_object_add(json_msg, "value", json_object_new_int(value));
			//printf("???send = %s\n",json_object_get_string(json_msg));

			//iRet = pgLiveMultiCaptureMessageSend(uInstID, lpszRenID, json_object_get_string(json_msg));
			iRet = pgLiveMultiCaptureNotifySend(uInstID,json_object_get_string(json_msg));
			if (iRet != PG_LIVE_MULTI_ERR_Normal) {
				printf("1111111  pgLiveMultiCaptureMessageSend: iErr=%d\n", iRet);
			}

		}else if(strcmp("set_video_definition",json_object_get_string(json_command)) == 0){ //设置分辨率
			unsigned int set_value = 0;
			json_object_object_get_ex(json_msg,"value",&json_value);
			printf("value = %s\n",json_object_get_string(json_value));
			if(strcmp("0",json_object_get_string(json_value)) == 0){
				set_value = 0;
				if(uVideoMode != 3)
					g_set_definetion = 1;
			}
			if(strcmp("1",json_object_get_string(json_value)) == 0){
				set_value = 1;
				if(uVideoMode != 10)
					g_set_definetion = 1;
			}
			sleep(1);

			iRet = pgLiveMultiCaptureNotifySend(uInstID,lpszData);
	
			if (iRet != PG_LIVE_MULTI_ERR_Normal) {
				printf("222222 pgLiveMultiCaptureMessageSend: iErr=%d\n", iRet);
			}
		}else if(strcmp("call_phone",json_object_get_string(json_command)) == 0){
			iRet = pgLiveMultiCaptureNotifySend(uInstID,json_object_get_string(json_msg));
			if (iRet != PG_LIVE_MULTI_ERR_Normal) {
				printf("333333  pgLiveMultiCaptureMessageSend: iErr=%d\n", iRet);
			}

		}

	}
	else if (strcmp(lpszAction, "RenderJoin") == 0) {
		my_pr_red("RENDER_JOIN: szRender=%s\n", lpszRenID);
		stopall(0);
		stopall(1);	
		g_RenderLeave = 0;
	}
	else if (strcmp(lpszAction, "RenderLeave") == 0) {
		my_pr_red("RENDER_LEAVE: szRender=%s\n", lpszRenID);
		g_RenderLeave = 1;
		if(dsp_fd){
			close(dsp_fd);
			my_pr_red("<<<<<<CLOSE dsp_fd>>>>>>>\n");
		}
		if(dsp3_fd){
			close(dsp3_fd);
			my_pr_red("<<<<<<CLOSE dsp3_fd>>>>>>>\n");
		}
		startall(0);
		startall(1);
	}
	else if (strcmp(lpszAction, "VideoStatus") == 0) {
	//	printf("VIDEO_STATUS: szData=%s\n", lpszData);
	}
	else if (strcmp(lpszAction, "VideoCamera") == 0) {
		printf("CAMERA_FILE: szData=%s\n", lpszData);
	}
	else if (strcmp(lpszAction, "VideoFrameStat") == 0) {
		printf("FRAME_STAT: szData=%s, szRender=%s\n", lpszData, lpszRenID);
	}
	else if (strcmp(lpszAction, "Login") == 0) {
		printf("SVR_LOGIN: szData=%s\n", lpszData);
	}
	else if (strcmp(lpszAction, "Logout") == 0) {
		printf("SVR_LOGOUT: szData=%s\n", lpszData);
	}
	else if (strcmp(lpszAction, "SvrReply") == 0) {
		printf("SVR_REPLY: szData=%s\n", lpszData);
	}
	else if (strcmp(lpszAction, "SvrReplyError") == 0) {
		printf("SVR_ERROR: szData=%s\n", lpszData);
	}
	else if (strcmp(lpszAction, "SvrNotify") == 0) {
		printf("SVR_NOTIFY: szData=%s\n", lpszData);
	}
	else if (strcmp(lpszAction, "KickOut") == 0) {
		printf("SVR_KICK_OUT: szData=%s\n", lpszData);
	}
	else if (strcmp(lpszAction, "ForwardAllocReply") == 0) {
		printf("FORWARD_ALLOC_REPLY: szData=%s, szRender=%s\n", lpszData, lpszRenID);
	}
	else if (strcmp(lpszAction, "ForwardFreeReply") == 0) {
		printf("FORWARD_FREE_REPLY: szData=%s, szRender=%s\n", lpszData, lpszRenID);
	}
	else if (strcmp(lpszAction, "FilePutRequest") == 0) {
		printf("FILE_PUT_REQUEST: szData=%s, szRender=%s\n", lpszData, lpszRenID);
		int iRet = pgLiveMultiCaptureFileAccept(uInstID, lpszRenID, "testput.txt");
		if (iRet != PG_LIVE_MULTI_ERR_Normal) {
			printf("pgLiveFileAccept: iRet=%d\n", iRet);
		}
	}
	else if (strcmp(lpszAction, "FileGetRequest") == 0) {
		printf("FILE_GET_REQUEST: szData=%s, szRender=%s\n", lpszData, lpszRenID);
		int iRet = pgLiveMultiCaptureFileAccept(uInstID, lpszRenID, "testget.txt");
		if (iRet != PG_LIVE_MULTI_ERR_Normal) {
			printf("pgLiveFileAccept: iRet=%d\n", iRet);
		}
	}
	else if (strcmp(lpszAction, "FileAccept") == 0) {
		printf("FILE_ACCEPT: szData=%s, szRender=%s\n", lpszData, lpszRenID);
	}
	else if (strcmp(lpszAction, "FileReject") == 0) {
		printf("FILE_REJECT: szData=%s, szRender=%s\n", lpszData, lpszRenID);
	}
	else if (strcmp(lpszAction, "FileProgress") == 0) {
		printf("FILE_PROGRESS: szData=%s, szRender=%s\n", lpszData, lpszRenID);
	}
	else if (strcmp(lpszAction, "FileFinish") == 0) {
		printf("FILE_FINISH: szData=%s, szRender=%s\n", lpszData, lpszRenID);
	}
	else if (strcmp(lpszAction, "FileAbort") == 0) {
		printf("FILE_ABORT: szData=%s, szRender=%s\n", lpszData, lpszRenID);
	}
	else if (strcmp(lpszAction, "PeerInfo") == 0) {
		printf("PeerInfo: szData=%s\n", lpszData);
	}
}

extern void pgPrintfEnable(unsigned int uFlag);

int mozart_start_camera_rayshine()
{
	sleep(1);
	printf("####################### mozart_start_camera_rayshine\n");
	if(g_video_start == 1){
		//g_video_restart = 1;
		return 0;
	}
	if(access("/dev/video0", R_OK))//如果没插摄像头，就退出
		return 0;

	char szDevID[32] = {0};
	char szSvrAddr[128] = {0};
	char szCameraNo[128] = {0};
	char szMaxStream[128] = {0};

	int fd = open("/usr/data/mac.txt",O_RDONLY);
	if(fd > 0){
		printf("###############\n");
		char id_buf[32] = {0};
		read(fd,id_buf,sizeof(id_buf));
		printf("id_buf = %s\n",id_buf);
		strncpy(szDevID, id_buf,strlen(id_buf));
		szDevID[strlen(id_buf)] = '\0';
		close(fd);
	}else{
		strcpy(szDevID, "lskj121201");
	}

	//strcpy(szSvrAddr, "connect.peergine.com:7781");
	strcpy(szSvrAddr, "120.55.42.83:7781");
	strcpy(szCameraNo, "0");
	strcpy(szMaxStream, "2");
	printf("###########szDevID = [%s]\n",szDevID);
	

	//pgPrintfEnable(1);
	RegisterVideoCallback();
	RegisterAudioCallback();

	pgLiveMultiCaptureSetCallback(LogOutput, EventProc);
//	pgLiveMultiRenderSetCallback(LogOutput,NULL);
	
	
	unsigned int uInstID = 0;

	if (pgLiveMultiCaptureInitialize(&uInstID, szDevID, "", szSvrAddr, "", 2, "") != PG_LIVE_MULTI_ERR_Normal) {
		printf("Init peergine module failed.\n");
		return 0;
	}	
	printf("Init peergine module success.\n");
	// Set video callback parameters.
	int iErr = -1;
	while(1){
		//while(g_RenderLeave){
		//	printf("-------------------------------------------------------------->not start\n");
		//	usleep(2000*1000);
		//}
		//sleep(1);
		while (g_RenderLeave) {
			usleep(2000*1000);
		}
		#if 1
		printf("############################start Video############################## \n");
		SetVideoCallbackParam(uVideoMode);
		// Open video
		#if 0
		if(g_set_definetion){
			if(uVideoMode == 3)
				uVideoMode = 10;
			else if(uVideoMode == 10)
				uVideoMode = 3;
			g_set_definetion = 0;
		}
		#endif
		sprintf(szVideoParam, "(Code){1}(Mode){%u}(Rate){80}(BitRate){500}(CameraNo){1}", uVideoMode);
		iErr = pgLiveMultiCaptureVideoStart(uInstID, 0, szVideoParam, 0);
		if (iErr != PG_LIVE_MULTI_ERR_Normal) {
			printf("pgLiveVideoStart. iErr=%d\n", iErr);
			return 0;
		}
		#endif
		#if 1
		// Open audio.
		printf("############################start Audio############################## \n");
		char szAudioParam[256] = { 0 };
		strcpy(szAudioParam,"(Reliable){0}(MuteInput){0}(MuteOutput){0}(EchoCancel){1}");
		printf("szAudioParam = %s\n",szAudioParam);
		iErr = pgLiveMultiCaptureAudioStart(uInstID, 0, szAudioParam);
		if (iErr != PG_LIVE_MULTI_ERR_Normal) {
			printf("pgLiveAudioStart. iErr=%d\n", iErr);
			return 0;
		}
		
		#endif
		if(!g_video_start)
			g_video_start = 1;
		while (!g_RenderLeave) {
			usleep(2000*1000);
		}
		
		printf("############################stop audio##############################\n");
		pgLiveMultiCaptureAudioStop(uInstID, 0);
		printf("############################stop video##############################\n");
		pgLiveMultiCaptureVideoStop(uInstID, 0);
		
	}
	pgLiveMultiCaptureCleanup(uInstID);

	return 0;
}

