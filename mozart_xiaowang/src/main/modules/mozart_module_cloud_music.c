#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include<errno.h>  
#include<string.h>  
#include<sys/types.h>  
#include<netinet/in.h>  
#include<sys/socket.h>  
#include<sys/wait.h>  


#include "vr-jushang_interface.h"
#include "tips_interface.h"
#include "mozart_config.h"
#include "lapsule_control.h"
#include "mozart_key_function.h"
#include "mozart_tts.h"
#include "mozart_ui.h"
#include "mozart_musicplayer.h"
#include "modules/mozart_module_cloud_music.h"
#include "sharememory_interface.h"
#include "mozart_face.h"
#include "nvrw_interface.h"
#include "airkiss.h"   //微信发现
#if(SUPPORT_SONG_SUPPLYER == SONG_SUPPLYER_XIMALAYA)
#include "song_supplyer/ximalaya.h"
#endif
#if(SUPPORT_SONG_SUPPLYER == SONG_SUPPLYER_JUSHANG)
#include "song_supplyer/jushang.h"
#endif
#if(SUPPORT_SONG_SUPPLYER == SONG_SUPPLYER_AISPEECH)
#include "song_supplyer/speech.h"
#endif

#ifdef SUPPORT_SONG_SUPPLYER

static char *artists[] = {"寓言故事","成语故事","十二生肖的故事","名人传","少儿百科","童话故事","中国历史","国学经典"};

extern char pre_singer[128];
int first_time = 0;

			
#define SERVER_ADDR	"119.23.207.229" 
#define SERVER_PORT	82 
#define POSTto "api.rayshine.cc" 
char check_deviceId_upload_head[] =
	"POST /devices/action/%s/%s?firmwareVersion=%s HTTP/1.1\r\n"
	"Host: %s\r\n"
	"User-Agent: Mozilla/5.0 (Windows NT 6.1; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/59.0.3071.115 Safari/537.36\r\n"
	"Content-Length: 0\r\n"
	"Accept: */*\n"
	"Accept-Encoding: gzip, deflate\r\n"
	"Accept-Language: zh-CN,zh;q=0.8\r\n"
	"Cache-Control: no-cache\r\n"
	"Proxy-Connection: keep-alive\r\n\r\n"
;

static int mozart_module_cloud_music_start(void)
{	printf("------------------mozart_module_cloud_music_start--------------\n");
	if (snd_source != SND_SRC_CLOUD) {
		if (mozart_module_stop())
			return -1;
		if (mozart_musicplayer_start(mozart_musicplayer_handler))
			return -1;
		mozart_ui_localplayer_plugin();
		snd_source = SND_SRC_CLOUD;
	} else if (!mozart_musicplayer_is_active(mozart_musicplayer_handler)) {
		if (mozart_musicplayer_start(mozart_musicplayer_handler))
			return -1;
	} else {
		mozart_musicplayer_musiclist_clean(mozart_musicplayer_handler);
	}

	return 0;
}

void *check_deviceId(void *args)
{	printf("==================check_deviceId=======\n");
	int sock = -1;
	int ret = -1;
	char send_date[4096] = {0};
	char deviceId[64] = {};
	nvinfo_t *nvinfo = NULL;
	
	sock = Connect_Server(SERVER_ADDR,SERVER_PORT);
	if(sock < 0 )
	{
		printf("connect server error!\n");
		return NULL;
	}

	char macaddr[] = "255.255.255.255";
	memset(macaddr, 0, sizeof(macaddr));
	get_mac_addr("wlan0", macaddr, "");
	memset(deviceId, 0, sizeof(deviceId));
	sprintf(deviceId, "NS-%s", &macaddr[6]);
	nvinfo = mozart_updater_chkver();
	ret = snprintf(send_date,4096,check_deviceId_upload_head,deviceId,macaddr,nvinfo->current_version,POSTto);
	printf("send_date : \n\n%s  \n\n",send_date);
	if(send(sock,send_date,ret,0) != ret)
	{
		printf("send head error!\n");
		close(sock);
		return NULL;
	}
	//printf("\r\n\r\n");
//接收返回值，用于判断是否上传成功
	//printf("#########################################recv:\n");
	memset(send_date,0,sizeof(send_date));
	if(recv(sock,send_date,4096,0) < 0)
		printf("recv error!\n");
		
	printf("%s\n",send_date);
	//printf("\n#########################################\n");
	close(sock);

	return NULL;
}

int creat_check_deviceId()
{
	pthread_t check_deviceId_t;

	if(pthread_create(&check_deviceId_t,NULL,check_deviceId,NULL) == -1)
	{
		printf("pthread_create check_deviceId_t failed\n");
		return -1;
	}
	return 0;
}

#define DEVICE_TYPE   "gh_27098xxxxx"
#define DEVICE_ID     "BD5D7899xxxxx"
#define DEFAULT_LAN_PORT    12476
#define AIRKISS_BIND_PORT 12476

//定义AirKiss库需要用到的一些标准函数，由对应的硬件平台提供，前三个为必要函数
const airkiss_config_t akconf =
{
	(airkiss_memset_fn)&memset,
	(airkiss_memcpy_fn)&memcpy,
	(airkiss_memcmp_fn)&memcmp,
	0
};

void *wx_deviceId_find(void *args)
{	
	printf("==============%s==============\n", __FUNCTION__);
	int server_sock_fd,len;  
	struct sockaddr_in addr;
	struct sockaddr_in reply_addr;
	int sock_timeout_val = 10000;
	int addr_len = sizeof(struct sockaddr_in);  
	char buffer[256] = {0};
  	int reply_fd;
	int cnt = 0;
	int send_cnt = 0;
	unsigned int lan_buf[256]; 
	int lan_buf_len = sizeof(lan_buf);
	airkiss_lan_ret_t ret = -1;
	airkiss_lan_ret_t packret;
	if((server_sock_fd = socket(AF_INET,SOCK_DGRAM,0)) < 0){  
		printf( "create sock error!\n"); 
		return -1;
	}
	memset(&addr, 0, sizeof(addr));  
	addr.sin_family=AF_INET;  
	addr.sin_port=htons(AIRKISS_BIND_PORT);  
	addr.sin_addr.s_addr=htonl(INADDR_ANY);  

	if (bind(server_sock_fd, (struct sockaddr *)&addr, sizeof(addr))<0) {
		printf( "bind sock error!\n"); 
		return -1;
	}

	setsockopt(server_sock_fd, SOL_SOCKET, SO_RCVTIMEO, &sock_timeout_val, sizeof(sock_timeout_val));

	while(1) {  
	  	memset(&buffer, 0, sizeof(buffer));  
		len = recvfrom(server_sock_fd, buffer, sizeof(buffer), 0 , (struct sockaddr *)&addr ,&addr_len); 
                if(len != -1){
			printf( "receive size=%d,",len);
			printf("source_IP = %s,source_port = %d\n",inet_ntoa( addr.sin_addr),ntohs(addr.sin_port));
			ret = airkiss_lan_recv(buffer, len, &akconf);
			switch (ret){
			case AIRKISS_LAN_SSDP_REQ:
				lan_buf_len = sizeof(lan_buf);
				packret = airkiss_lan_pack(AIRKISS_LAN_SSDP_RESP_CMD, "gh_37ee6f67283e", "NS-Test01", 0,0, lan_buf, &lan_buf_len, &akconf);
				if (packret != AIRKISS_LAN_PAKE_READY) {
					printf("Pack lan packet error!");
					continue;
				}

				if((reply_fd = socket(AF_INET,SOCK_DGRAM,0)) < 0){  
					printf("create sock error!\n"); 
					continue;
				}
				printf("response msg to wei xin\n");
				memset(&reply_addr, 0, sizeof(reply_addr));
				reply_addr.sin_family=AF_INET;  
				reply_addr.sin_port = htons(ntohs(addr.sin_port)); 
				reply_addr.sin_addr.s_addr=inet_addr(inet_ntoa( addr.sin_addr));

				send_cnt = sendto(reply_fd,lan_buf,sizeof(lan_buf),0,(struct sockaddr *)&reply_addr,sizeof(reply_addr));
				close(reply_fd);
				break;
			default:
				printf("Pack is not ssdq req!\n");

			break;
			}

		}
        
	}
	return NULL;
}

int creat_wx_deviceId_find()
{
	pthread_t wx_deviceId_find_t;

	if(pthread_create(&wx_deviceId_find_t,NULL,wx_deviceId_find,NULL) == -1)
	{
		printf("pthread_create check_deviceId_t failed\n");
		return -1;
	}
	pthread_detach(wx_deviceId_find_t);
	int cnt = 0;
	//while(cnt++ < 10){
		//sleep(5);
		//printf("creat_wx_deviceId_find recvfrom####\n");
	//}
	printf("creat_wx_deviceId_find recvfrom  OVER ####################\n");
	return 0;

}


int mozart_module_cloud_music_startup(void)
{	printf("===========mozart_module_cloud_music_startup==========@ mozart_vr_get_status = %d\n",mozart_vr_get_status());
	memory_domain domain;
	int ret;
	char buf[1024] = {};
	char last_singer[1024] = {};
	int idx = random() % (sizeof(artists) / sizeof(artists[0]));

	FILE *fp = NULL;
	if(first_time == 1){
		ret = share_mem_get_active_domain(&domain);
		if(ret != 0 || domain != 0)
			return 0;
		if (mozart_module_cloud_music_start())
			return -1;
		ret = share_mem_get_active_domain(&domain);
		//mozart_play_key("cloud_mode");
		mozart_tts_wait();
		#if 0
		mozart_tts("连上网络了，让我们一起愉快的玩耍吧");
		usleep(3500*1000);
		if(strlen(pre_singer)){
			printf("pre_singer**%s*****************\n",pre_singer);
			sprintf(buf, "欢迎回来，继续为您播放");
		}
		else{
			printf("pre_singer***%s****************\n",artists[idx]);
			sprintf(buf, "欢迎回来，继续为您播放");

		}
		mozart_tts(buf);
		mozart_tts_wait();
		#endif
		ret = share_mem_get_active_domain(&domain);
		if(ret != 0 || domain != 0)
			return 0;
		mozart_tts(buf);
		mozart_tts_wait();
		if(strlen(pre_singer))
			return speech_cloudmusic_play(pre_singer);
		else
			return speech_cloudmusic_play(artists[idx]);
	}else{
		first_time = 1;
		char buf_r[128] = {};
		ret = share_mem_get_active_domain(&domain);
		if(ret != 0 || domain != 0)
			return 0;
		if (mozart_module_cloud_music_start())
			return -1;
		
		creat_check_deviceId();//检查设备ID，提交版本号
		//creat_wx_deviceId_find();
		ret = share_mem_get_active_domain(&domain);
		mozart_tts("连上网络了，让我们一起学习吧");
		mozart_tts_wait();
		time_t timep;  
	    	struct tm *p;  
	    	time(&timep);  
	    	p =localtime(&timep);
		system("wget -O /tmp/beijing http://api.rayshine.cc/time/beijing");
		if(access("/tmp/beijing",F_OK) != 0){
		if(p->tm_wday)	
			sprintf(buf, "今天是%d年%d月%d日，星期%d ",(1900+p->tm_year),(1+p->tm_mon),(p->tm_mday),p->tm_wday);
		else
			sprintf(buf, "今天是%d年%d月%d日，星期天 ",(1900+p->tm_year),(1+p->tm_mon),(p->tm_mday),p->tm_wday);
		}else{	
			fp = fopen("/tmp/beijing","r");
			fread(buf_r,1,100,fp);
			fclose(fp);
			sprintf(buf,"今天是%s ",buf_r);
			//sprintf(buf,"welcome back,today is good day,please put your hands up,knee down,fire in the hole,go,go,go,%s,",buf_r);
		}
		ret = share_mem_get_active_domain(&domain);
		if(ret != 0 || domain != 0)
			return 0;
		mozart_tts(buf);
		mozart_tts_wait();
		return speech_cloudmusic_play(artists[idx]);
	}

}




#if 0

int mozart_module_cloud_news_startup(void)
{	
	printf("===========mozart_module_cloud_news_startup==========\n");
	memory_domain domain;
	int ret;
	char buf[1024] = {};

	ret = share_mem_get_active_domain(&domain);
	if(ret != 0 || domain != 0)
		return 0;

	if (mozart_module_cloud_music_start())
		return -1;
	ret = share_mem_get_active_domain(&domain);
	
	mozart_play_key("cloud_mode");

	time_t timep;  
	struct tm *p;  
	time(&timep);  
	p =localtime(&timep);
	if(p->tm_wday)	
		sprintf(buf, "主人您好,今天是%d年%d月%d日，星期%d,现在为您播放%s",(1900+p->tm_year),(1+p->tm_mon),(p->tm_mday),p->tm_wday,news_name);
	else
		sprintf(buf, "主人您好,今天是%d年%d月%d日，星期天,现在为您播放%s",(1900+p->tm_year),(1+p->tm_mon),(p->tm_mday),p->tm_wday,news_name);


	ret = share_mem_get_active_domain(&domain);	
	if(ret != 0 || domain != 0)
		return 0;
	mozart_tts(buf);
	mozart_tts_wait();

#if (SUPPORT_SONG_SUPPLYER == SONG_SUPPLYER_XIMALAYA)
	return ximalaya_cloudmusic_play("我要听新闻");
#elif (SUPPORT_SONG_SUPPLYER == SONG_SUPPLYER_LAPSULE)
	return 0;
#elif (SUPPORT_SONG_SUPPLYER == SONG_SUPPLYER_JUSHANG)
	return jushang_cloudmusic_play("我要听新闻");
#elif (SUPPORT_SONG_SUPPLYER == SONG_SUPPLYER_AISPEECH)
	return speech_cloudmusic_play(artists[0]);
#else
	return 0;
#endif
	
}
#endif

#endif	/* SUPPORT_SONG_SUPPLYER */
