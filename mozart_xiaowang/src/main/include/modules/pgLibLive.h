/****************************************************************
  copyright   : Copyright (C) 2014-2016, chenbichao,
              : All rights reserved.
              : www.pptun.com, www.peergine.com
              : 
  filename    : pgLibLiveCap.h
  discription : 
  modify      : create, chenbichao, 2014/07/22
              :
              : Modify, chenbichao, 2014/08/02
              : 1. pgLiveEvent()��������lpszRender������
              : 2. ����pgLiveVideoStart()������������Ƶ�ɼ�
              : 3. ����pgLiveVideoStop()������ֹͣ��Ƶ�ɼ�
              : 4. ����pgLiveCamera()����������Ƶ����
			  : 5. ���ӡ�PG_LIVE_EVENT_CAMERA_FILE���¼����ϱ����ճɹ��¼���
              : 
              : Modify, chenbichao, 2014/11/10
              : 1. ����pgLiveAudioStart()������������Ƶ�ɼ�
              : 2. ����pgLiveAudioStop()������ֹͣ��Ƶ�ɼ�
			  : 
              : Modify, chenbichao, 2015/03/04
              : 1. ����pgLiveVideoSource()������ѡ����ƵԴ
              :
              : Modify, chenbichao, 2015/10/06
              : 1. ����pgLiveForward* ϵ�к�����������ͷ�ֱ����Ƶת����������Դ
              : 2. ����pgLiveFile* ϵ�к������ļ��������������
              : 3. ����PG_LIVE_EVENT_FORWARD_* ϵ���¼���������ͷ�ת����Դ���¼���
              : 4. ����PG_LIVE_EVENT_FILE* ϵ���¼����ļ�����������¼���
              :
              : Modify, chenbichao, 2015/10/17
              : 1. PG_LIVE_VIDEO_S�ṹ������uMaxVideoStream��Ա��
              : 2. PG_LIVE_VIDEO_S�ṹ������uForward��Ա��
              :
              : Modify, chenbichao, 2015/11/03
              : 1. ����pgLiveVideoParam�������޸���Ƶ������
              :
			  : modify, chenbichao, 2015/12/12
			  : 1. ����PG_LIVE_INIT_CFG_S�ṹ��uForwardUse���ԣ����������Ƿ�ʹ�õ�3��P2P�ڵ����ת����
			  : 2. �޸�PG_LIVE_INIT_CFG_S�ṹ��uParam����Ϊvoidָ�����ͣ����Լ���64bit��ʹ�ó�����
			  :
			  : modify, chenbichao, 2015/12/21
			  : 1. ����pgLiveAccess����������ָ�����Ŷ˵���Ƶ����Ƶ����Ȩ�ޡ�
			  :
			  : modify, chenbichao, 2016/01/05
			  : 1. ����PG_LIVE_EVENT_FRAME_STAT�¼����ϱ���Ƶ֡����ʧ��ͳ����Ϣ��
			  :
			  : modify, chenbichao, 2016/01/21
			  : 1. ����PG_LIVE_INIT_CFG_S�ṹ��uVideoInExternal���ԡ�
			  : 2. ����PG_LIVE_INIT_CFG_S�ṹ��uAudioInExternal���ԡ�
			  : 3. ����PG_LIVE_INIT_CFG_S�ṹ��uAudioOutExternal���ԡ�
			  :
			  : modify, chenbichao, 2016/04/06
			  : 1. ����pgLiveVideoModeSize()���������¶���ָ����Ƶģʽ�ĳߴ硣
			  
			  : modify, chenbichao, 2016/04/06
			  : 1. ����pgLiveRenderEnum()������ö�ٲ�ѯ��ǰ���ӵ�Render��P2P ID��
			  : 
*****************************************************************/
#ifndef _PG_LIB_LIVE_CAP_H
#define _PG_LIB_LIVE_CAP_H


#ifdef _PG_DLL_EXPORT
#define PG_DLL_API __declspec(dllexport)
#else
#define PG_DLL_API
#endif


#ifdef __cplusplus
extern "C" {
#endif



/**
 *  �����붨��
 */
typedef enum tagPG_LIVE_ERROR_E {
	PG_LIVE_ERROR_OK = 0,             // �ɹ�
	PG_LIVE_ERROR_INIT = -1,          // û�е���pgLiveInitialize()�����Ѿ�����pgLiveCleanup()����ģ�顣
	PG_LIVE_ERROR_CLOSE = -2,         // �Ự�Ѿ��رգ��Ự�Ѿ����ɻָ�����
	PG_LIVE_ERROR_BADPARAM = -3,      // ���ݵĲ�������
	PG_LIVE_ERROR_BADRENDER = -4,     // ָ����Render�����ڡ�
	PG_LIVE_ERROR_NOBUF = -5,         // �Ự���ͻ�����������
	PG_LIVE_ERROR_NOSPACE = -6,       // ���ݵĽ��ջ�����̫С��
	PG_LIVE_ERROR_TIMEOUT = -7,       // ������ʱ��
	PG_LIVE_ERROR_BUSY = -8,          // ϵͳ��æ��
	PG_LIVE_ERROR_NOLOGIN = -9,       // ��û�е�¼��P2P��������
	PG_LIVE_ERROR_MAXINST = -11,      // ���ʵ������
	PG_LIVE_ERROR_NOCONNECT = -12,    // Render��δ����P2P����
	PG_LIVE_ERROR_BADSTATUS = -13,    // ״̬����ȷ
	PG_LIVE_ERROR_NOIMP = -14,        // �ýӿ�û��ʵ��
	PG_LIVE_ERROR_SYSTEM = -127,      // ϵͳ����
} PG_LIVE_ERROR_E;


/**
 *  ���ݷ���/�������ȼ�
 */
typedef enum tagPG_PRIORITY_E {
	PG_PRIORITY_0,         // ���ȼ�0, ������ȼ���
	PG_PRIORITY_1,         // ���ȼ�1
	PG_PRIORITY_2,         // ���ȼ�2
	PG_PRIORITY_3,         // ���ȼ�3, ������ȼ�
	PG_PRIORITY_BUTT,
} PG_PRIORITY_E;


/**
 *  pgLiveEvent()�����ȴ����¼�����
 */
typedef enum tagPG_LIVE_EVENT_E {
	PG_LIVE_EVENT_NULL,                 // NULL
	PG_LIVE_EVENT_MESSAGE,              // ���յ�Render���͵���Ϣ��
	PG_LIVE_EVENT_RENDER_JOIN,          // Render������Ƶֱ���顣
	PG_LIVE_EVENT_RENDER_LEAVE,         // Render�뿪��Ƶֱ���顣
	PG_LIVE_EVENT_VIDEO_STATUS,         // �ϱ���Ƶ�ɼ���״̬��
	PG_LIVE_EVENT_CAMERA_FILE,          // ץ����Ƶ��Ƭ���浽�ļ��ɹ���
	PG_LIVE_EVENT_FRAME_STAT,           // �ϱ���Ƶ֡����ʧ��ͳ����Ϣ��
	PG_LIVE_EVENT_INFO,                 /// �ϱ�Render������ͨ����Ϣ

	PG_LIVE_EVENT_SVR_LOGIN = 16,       // ��¼��P2P�������ɹ������ߣ�
	PG_LIVE_EVENT_SVR_LOGOUT,           // ��P2P������ע������ߣ����ߣ�
	PG_LIVE_EVENT_SVR_REPLY,            // P2P������Ӧ���¼���
	PG_LIVE_EVENT_SVR_NOTIFY,           // P2P�����������¼���
	PG_LIVE_EVENT_SVR_ERROR,            // pgLiveSvrRequest���ش���
	PG_LIVE_EVENT_SVR_KICK_OUT,         // ���������߳�����Ϊ������һ����ͬID�Ľڵ��¼�ˡ�
	
	PG_LIVE_EVENT_FORWARD_ALLOC_REPLY = 32,   // ������Ƶת����Դ���ؽ��
	PG_LIVE_EVENT_FORWARD_FREE_REPLY,         // �ͷ���Ƶת����Դ���ؽ��

	PG_LIVE_EVENT_FILE_PUT_REQUEST = 48,      // ���Ŷ������ϴ��ļ�
	PG_LIVE_EVENT_FILE_GET_REQUEST,           // ���Ŷ����������ļ�
	PG_LIVE_EVENT_FILE_ACCEPT,                // ���Ŷ˽����˱��ɼ��˵��ļ���������
	PG_LIVE_EVENT_FILE_REJECT,                // ���Ŷ˾����˱��ɼ��˵��ļ���������
	PG_LIVE_EVENT_FILE_PROGRESS,              // �ļ���������ϱ�
	PG_LIVE_EVENT_FILE_FINISH,                // �ļ��������
	PG_LIVE_EVENT_FILE_ABORT,                 // �ļ������ж�

} PG_LIVE_EVENT_E;


/**
 *  ͨ������������
 */
typedef enum tagPG_LIVE_CNNT_E {
	PG_LIVE_CNNT_Unknown = 0,            // δ֪����û�м�⵽��������

	PG_LIVE_CNNT_IPV4_Pub = 4,           // ����IPv4��ַ
	PG_LIVE_CNNT_IPV4_NATConeFull,       // ��ȫ׶��NAT
	PG_LIVE_CNNT_IPV4_NATConeHost,       // ��������׶��NAT
	PG_LIVE_CNNT_IPV4_NATConePort,       // �˿�����׶��NAT
	PG_LIVE_CNNT_IPV4_NATSymmet,         // �Գ�NAT

	PG_LIVE_CNNT_IPV4_Private = 12,      // ˽��ֱ��
	PG_LIVE_CNNT_IPV4_NATLoop,           // ˽��NAT����

	PG_LIVE_CNNT_IPV4_TunnelTCP = 16,    // TCPv4ת��
	PG_LIVE_CNNT_IPV4_TunnelHTTP,        // HTTPv4ת��

	PG_LIVE_CNNT_IPV4_PeerFwd = 24,      // ����P2P�ڵ�ת��

	PG_LIVE_CNNT_IPV6_Pub = 32,          // ����IPv6��ַ

	PG_LIVE_CNNT_IPV6_TunnelTCP = 40,    // TCPv6ת��
	PG_LIVE_CNNT_IPV6_TunnelHTTP,        // HTTPv6ת��

	PG_LIVE_CNNT_Offline = 0xffff,       // �Զ˲�����

} PG_LIVE_CNNT_E;


/**
 *  ��ʼ��������
 */
typedef struct tagPG_LIVE_INIT_CFG_S {

	// 4�����ȼ��ķ��ͻ��������ȣ���λΪ��K�ֽڡ�
	// uBufSize[0] Ϊ���ȼ�0�ķ��ͻ��������ȣ���0��ʹ��ȱʡֵ��ȱʡֵΪ128(K)
	// uBufSize[1] Ϊ���ȼ�1�ķ��ͻ��������ȣ���0��ʹ��ȱʡֵ��ȱʡֵΪ128(K)
	// uBufSize[2] Ϊ���ȼ�2�ķ��ͻ��������ȣ���0��ʹ��ȱʡֵ��ȱʡֵΪ256(K)
	// uBufSize[3] Ϊ���ȼ�3�ķ��ͻ��������ȣ���0��ʹ��ȱʡֵ��ȱʡֵΪ256(K)
	// ��ʾ�����������ڴ治�ǳ�ʼ��ʱ�ͷ���ã�Ҫ�õ�ʱ��ŷ��䡣
	//       ���磬������256(K)������ǰֻʹ����16(K)����ֻ����16(K)���ڴ档
	//       ����������󣬷��͵����ݲ��ڻ��������������򻺳���ʵ��ʹ�õĳ��Ȳ���������
	// ע�⣺�������ĳ���ֵ���ܳ���32768��
	unsigned int uBufSize[PG_PRIORITY_BUTT];

	// ����P2P��͸��ʱ�䡣���ʱ�䵽���û�д�͸�����л���ת��ͨ�š�
	// (uTryP2PTimeout == 0)��ʹ��ȱʡֵ��ȱʡֵΪ6�롣
	// (uTryP2PTimeout > 0 && uTryP2PTimeout <= 3600)����ʱֵΪ������uTryP2PTimeout
	// (uTryP2PTimeout > 3600)������P2P��͸��ֱ����ת����
	unsigned int uTryP2PTimeout;

	// ��������P2P�ڵ�������ڵ�ת����������0��ת�����ʣ��ֽ�/�룩��0��������ת����
	// ��������: 32K (�ֽ�/��) ���ϡ�
	unsigned int uForwardSpeed;

	// �Ƿ������3��P2P�ڵ�ת����������0���ǣ�0����
	unsigned int uForwardUse;

	// ���ݳ�ʼ��������Ŀǰ��Androidϵͳ����Java VM��ָ�롣��
	// ��JNIģ����ʵ��JNI_Onload�ӿڣ���ȡ��Java VM��ָ�룬����pgLiveInitialize()�����P2Pģ�顣
	void* pvParam;

	// ��Ƶ�ⲿ�ɼ�/ѹ���ӿ�ѡ���0���ⲿ�ɼ�/ѹ����0���ڲ��Դ��Ĳɼ�/ѹ����
	// �ӿڶ���ο�pgLibDevVideoIn.h
	unsigned int uVideoInExternal;
	
	// ��Ƶ�ⲿ�ɼ�/ѹ���ӿ�ѡ���0���ⲿ�ɼ�/ѹ����0���ڲ��Դ��Ĳɼ�/ѹ����
	// �ӿڶ���ο�pgLibDevAudioIn.h
	unsigned int uAudioInExternal;

	// ��Ƶ�ⲿ��ѹ/���Žӿ�ѡ���0���ⲿ��ѹ/���ţ�0���ڲ��Դ��Ľ�ѹ/���š�
	// �ӿڶ���ο�pgLibDevAudioOut.h
	unsigned int uAudioOutExternal;
								
} PG_LIVE_INIT_CFG_S;


/**
 *  ��Ƶ�ɼ�������
 */
typedef struct tagPG_LIVE_VIDEO_S {
	unsigned int uCode;             // ��Ƶ��������ͣ�1ΪMJPEG��2ΪVP8��3ΪH264, 4ΪH265
	unsigned int uMode;             // ��Ƶ�ߴ�ģʽ��0: 80x60, 1: 160x120, 2: 320x240, 3: 640x480, 4: 800x600, 5: 1024x768, 6: 176x144, 7: 352x288, 8: 704x576, 9: 854x480, 10: 1280x720, 11: 1920x1080
	unsigned int uRate;             // ��Ƶ��֡�٣�����ָ����֡��������룩
	unsigned int uCameraNo;         // ָ������ͷ(��ƵԴ)���(0, 1, 2, ...)
	unsigned int uBitRate;          // ��Ƶ����������С(���H264��VP8)����λΪKbps
	unsigned int uMaxVideoStream;   // ����ͬʱ������Ƶ�������������Ĭ��Ϊ2����
	unsigned int uForward;          // ת��ֱ����Ƶ����1Ϊ����ת����0Ϊ����ת����
} PG_LIVE_VIDEO_S;


/**
 *  Render�ĵ�ַ��Ϣ��
 */
typedef struct tagPG_LIVE_INFO_S {
	char szPeerID[128];         // Render��P2P ID
	char szAddrPub[64];         // Render�Ĺ���IP��ַ
	char szAddrPriv[64];        // Render��˽��IP��ַ
	unsigned int uCnntType;     // Render����ͨ�����������ͣ�NAT���ͣ�����ö�١�PG_LIVE_CNNT_E��
} PG_LIVE_INFO_S;


///
// Node Event hook
typedef int (*TpfnPGLiveEventHookOnExtRequest)(unsigned int uInstID,
	const char* sObj, int uMeth, const char* sData, unsigned int uHandle, const char* sPeer);

typedef int (*TpfnPGLiveEventHookOnReply)(unsigned int uInstID,
	const char* sObj, int uErr, const char* sData, const char* sParam);


PG_DLL_API
void pgLiveSetEventHook(TpfnPGLiveEventHookOnExtRequest pfnOnExtRequest,
	TpfnPGLiveEventHookOnReply pfnOnReply);


/**
 *  ��־����ص�����
 *
 *  uLevel��[IN] ��־����
 *
 *  lpszOut��[IN] ��־�������
 */
typedef void (*TfnLogOut)(unsigned int uLevel, const char* lpszOut);

/**
 *  ������P2Pֱ��ģ���ʼ������
 *
 *  ������ʽ�����������������ء�
 *
 *  lpuInstID��[OUT] ʵ��ID��P2Pģ��֧�ֶ�ʵ������ʼ��ʱ����ʵ��ID��
 *
 *  lpszUser��[IN] �ͻ���ʱΪ�ʺ��û�����������ʱͨ��Ϊ�豸ID��
 *
 *  lpszPass��[IN] �ͻ���ʱΪ�ʺ�����
 *
 *  lpszSvrAddr��[IN] P2P�������ĵ�ַ�˿ڣ����磺��127.0.0.1:3333��
 * 
 *  lpszRelayList��[IN] �м̷�������ַ�б�P2P�޷���͸�������ͨ���м̷�����ת����
 *      ��ʽʾ����"type=0&load=0&addr=127.0.0.1:8000;type=1&load=100&addr=192.168.0.1:443"
 *      ÿ���м̷�������type��load��addr��������������м̷�����֮���÷ֺš�;��������
 * 
 *  lpstInitCfg��[IN] ��ʼ�����������ṹ��PG_LIVE_INIT_CFG_S���Ķ��塣
 *
 *  lpstVideo��[IN] ��Ƶ�ɼ����������ṹ��PG_lIVE_VIDEO_S���Ķ���
 *
 *  pfnLogOut��[IN] ��־����ص�������ָ�롣�ص�����ԭ�ͼ���TfnLogOut�����塣
 *
 *  ����ֵ����ö�١�PG_LIVE_ERROR_E���Ķ���
 */
PG_DLL_API
int pgLiveInitialize(unsigned int* lpuInstID, const char* lpszUser,
	const char* lpszPass, const char* lpszSvrAddr, const char* lpszRelayList,
	PG_LIVE_INIT_CFG_S *lpstInitCfg, PG_LIVE_VIDEO_S* lpstVideo, TfnLogOut pfnLogOut);


PG_DLL_API
int pgLiveInitializeEx(unsigned int* lpuInstID, const char* lpszUser,
	const char* lpszPass, const char* lpszSvrAddr, const char* lpszRelayAddr,
	int iP2PTryTime, const char* lpszInstParam, const char* lpszVideoParam,
	const char* lpszAudioParam, TfnLogOut pfnLogOut);


/**
 *  ������P2Pֱ��ģ�������ͷ�������Դ��
 * 
 *  ������ʽ�����������������ء�
 *
 *  uInstID��[IN] ʵ��ID������pgLiveInitialize()ʱ�����
 */
PG_DLL_API
void pgLiveCleanup(unsigned int uInstID);


/**
 *  ������ѡ����ƵԴ��
 *
 *  ������ʽ�����������������ء�
 *
 *  uInstID��[IN] ʵ��ID������pgLiveInitialize()ʱ�����
 *
 *  uCameraNo��[IN] ����ͷ���(0, 1, 2, ...)��
 *
 *  ����ֵ������0Ϊ�ɹ���С��0Ϊ������
 */
PG_DLL_API
int pgLiveVideoSource(unsigned int uInstID, unsigned int uCameraNo);


/**
 *  ���������¶���ָ����Ƶģʽ�ĳߴ硣
 *        �˺�������ʱ������pgLiveInitialize()����֮��pgLiveVideoStart()����֮ǰ��
 *
 *  ������ʽ�����������������ء�
 *
 *  uInstID��[IN] ʵ��ID������pgLiveInitialize()ʱ�����
 *
 *  uVideoMode [IN] ��Ҫ���¶���ߴ����Ƶģʽ��
 *
 *  uWidth [IN] ��Ƶ�Ŀ����أ���
 *
 *  uHeight [IN] ��Ƶ�ĸߣ����أ���
 *
 *  ����ֵ������0Ϊ�ɹ���С��0Ϊ������
 */
PG_DLL_API
int pgLiveVideoModeSize(unsigned int uInstID, unsigned int uVideoMode,
	unsigned int uWidth, unsigned int uHeight);


/**
 *  �������޸���Ƶ������
 *
 *  ������ʽ�����������������ء�
 *
 *  uInstID��[IN] ʵ��ID������pgLiveInitialize()ʱ�����
 *
 *  lpstVideo��[IN] ��Ƶ������
 *
 *  ����ֵ������0Ϊ�ɹ���С��0Ϊ������
 */
PG_DLL_API
int pgLiveVideoParam(unsigned int uInstID, const PG_LIVE_VIDEO_S* lpstVideo);


/**
 *  ��������ʼ��Ƶ�ɼ���
 *
 *  ������ʽ�����������������ء�
 *
 *  uInstID��[IN] ʵ��ID������pgLiveInitialize()ʱ�����
 *
 *  ����ֵ������0Ϊ�ɹ���С��0Ϊ������
 */
PG_DLL_API
int pgLiveVideoStart(unsigned int uInstID);


/**
 *  ������ֹͣ��Ƶ�ɼ���
 *
 *  ������ʽ�����������������ء�
 *
 *  uInstID��[IN] ʵ��ID������pgLiveInitialize()ʱ�����
 */
PG_DLL_API
void pgLiveVideoStop(unsigned int uInstID);


/**
 *  ������ץ����Ƭ��ͨ����PG_LIVE_EVENT_CAMERA_FILE���¼��ϱ����ս����
 *
 *  ������ʽ�����������������ء�
 *
 *  uInstID��[IN] ʵ��ID������pgLiveInitialize()ʱ�����
 *
 *  lpszPath��[IN] ������Ƭ�ļ���·�����ļ�����׺����Ϊ*.jpg��
 *
 *  ����ֵ������0Ϊ�ɹ���С��0Ϊ������
 */
PG_DLL_API
int pgLiveCamera(unsigned int uInstID, const char* lpszPath);


/**
 *  ��������ʼ��Ƶ�ɼ���
 *
 *  ������ʽ�����������������ء�
 *
 *  uInstID��[IN] ʵ��ID������pgLiveInitialize()ʱ�����
 *
 *  ����ֵ������0Ϊ�ɹ���С��0Ϊ������
 */
PG_DLL_API
int pgLiveAudioStart(unsigned int uInstID);


/**
 *  ������ֹͣ��Ƶ�ɼ��� 
 *
 *  ������ʽ�����������������ء�
 *
 *  uInstID��[IN] ʵ��ID������pgLiveInitialize()ʱ�����
 */
PG_DLL_API
void pgLiveAudioStop(unsigned int uInstID);


/**
*  ������������Ƶ������
*
*  ������ʽ�����������������ء�
*
*  sParam��[IN] ��Ƶ������
*
*  uInstID��[IN] ʵ��ID������pgLiveInitialize()ʱ�����
*
*  ����ֵ������0Ϊ�ɹ���С��0Ϊ������
*/
PG_DLL_API
int pgLiveAudioParam(unsigned int uInstID, const char* sParam);


/**
 *  �������ȴ��ײ��¼��ĺ���������ͬʱ�ȴ�����Ự�ϵĶ����¼����¼������������ء�
 *
 *  ������ʽ�����������¼��ﵽ��ȴ���ʱ�󷵻ء�
 *
 *  uInstID��[IN] ʵ��ID������pgLiveInitialize()ʱ�����
 *
 *  lpuEvent��[OUT] �����ǰ�������¼����ͣ���ö�١�PG_LIVE_EVENT_E������
 *
 *  lpszData��[OUT] ���ܵ�ǰ�����¼������ݻ�������
 *
 *  uDataSize��[IN] ���������ȡ���������������ʱ�����������������ݽ���������
 *
 *  lpuParam��[OUT] �����ǰ�����¼��Ĳ�����
 *      �ڡ�PG_LIVE_EVENT_SVR_REPLY���¼�ʱʹ�ô˲�����
 *
 *  lpszRender��[OUT] ���մ�����ǰ�¼���Render��P2P�ڵ�ID�����ȱ�����ڵ���128�ַ���
 *
 *  uTimeout��[IN] �ȴ���ʱʱ��(����)����0Ϊ���ȴ����������ء�
 *
 *  ����ֵ����ö�١�PG_LIVE_ERROR_E���Ķ��塣
 */
PG_DLL_API
int pgLiveEvent(unsigned int uInstID, unsigned int* lpuEvent, char* lpszData,
	unsigned int uDataSize, unsigned int* lpuParam, char* lpszRender, unsigned int uTimeout);


/**
 *  �������鲥����֪ͨ������Render
 *
 *  ������ʽ�����������������ء�
 *
 *  uInstID��[IN] ʵ��ID������pgLiveInitialize()ʱ�����
 *
 *  lpszData��[IN] �鲥֪ͨ������
 *
 *  ����ֵ������0Ϊ���͵����ݳ��ȡ�С��0Ϊ�����루��ö�١�PG_LIVE_ERROR_E���Ķ��壩
 */
PG_DLL_API
int pgLiveNotify(unsigned int uInstID, const char* lpszData);


/**
 *  ������������Ϣ��һ��ָ����Render.
 *
 *  ������ʽ�����������������ء�
 *
 *  uInstID��[IN] ʵ��ID������pgLiveInitialize()ʱ�����
 *
 *  lpszRender��[IN] ����Ŀ��Render��P2P�ڵ�ID
 *
 *  lpszData��[IN] ������Ϣ������
 *
 *  ����ֵ������0Ϊ���յ����ݳ��ȡ�С��0Ϊ�����루��ö�١�PG_LIVE_ERROR_E���Ķ��壩
 */
PG_DLL_API
int pgLiveMessage(unsigned int uInstID, const char* lpszRender, const char* lpszData);


/**
 *  ��������ȡһ��ָ��Render�ĵ�ַ��Ϣ��
 *
 *  ������ʽ�����������������ء�
 *
 *  uInstID��[IN] ʵ��ID������pgLiveInitialize()ʱ�����
 *
 *  lpszRender��[IN] ָ��Render��P2P�ڵ�ID��
 *
 *  lpstInfo��[OUT] ��ȡ���ĵ�ַ��Ϣ������PG_LIVE_INFO_S���ṹ����
 *
 *  ����ֵ��0Ϊ�ɹ���С��0Ϊ�����루��ö�١�PG_LIVE_ERROR_E���Ķ��壩
 */
PG_DLL_API
int pgLiveInfo(unsigned int uInstID, const char* lpszRender, PG_LIVE_INFO_S* lpstInfo);


/**
 *  �������ܾ�һ��ָ����Render��
 *
 *  ������ʽ�����������������ء�
 *
 *  uInstID��[IN] ʵ��ID������pgLiveInitialize()ʱ�����
 *
 *  lpszRender��[IN] ָ��Render��P2P�ڵ�ID
 *
 *  ����ֵ��0Ϊ�ɹ���С��0Ϊ�����루��ö�١�PG_LIVE_ERROR_E���Ķ��壩
 */
PG_DLL_API
int pgLiveReject(unsigned int uInstID, const char* lpszRender);


/**
 *  �����������͹ر�ָ����Render����Ƶ����Ƶ����Ȩ�ޡ�
 *
 *  ������ʽ�����������������ء�
 *
 *  uInstID��[IN] ʵ��ID������pgLiveInitialize()ʱ�����
 *
 *  lpszRender��[IN] ָ��Render��P2P�ڵ�ID
 *
 *  uVideoEnable��[IN] ��0��������Ƶ���ʣ�0����ֹ��Ƶ���ʡ�Ĭ��Ϊ������ʡ�
 *
 *  uAudioEnable��[IN] ��0��������Ƶ���ʣ�0����ֹ��Ƶ���ʡ�Ĭ��Ϊ������ʡ�
 *
 *  ����ֵ��0Ϊ�ɹ���С��0Ϊ�����루��ö�١�PG_LIVE_ERROR_E���Ķ��壩
 */
PG_DLL_API
int pgLiveAccess(unsigned int uInstID, const char* lpszRender,
	unsigned int uVideoEnable, unsigned int uAudioEnable);


/**
 *  ������ö�ٲ�ѯ��ǰ���ӵ�Render��P2P ID��
 *
 *  ������ʽ�����������������ء�
 *
 *  uInstID��[IN] ʵ��ID������pgLiveInitialize()ʱ�����
 *
 *  iIndex: [IN] ö�ٲ�ѯ������ֵ��0, 1, 2, 3, ...
 *
 *  sRenID: [OUT] ����Render��P2P ID�Ļ�������
 *
 *  uSize: [IN] ���������ȡ�
 *
 *  ����ֵ��0Ϊ�ɹ���С��0Ϊ�����루��ö�١�PG_LIVE_ERROR_E���Ķ��壩
 */
PG_DLL_API
int pgLiveRenderEnum(unsigned int uInstID, int iIndex, char* sRenID, unsigned int uSize);


/**
 *  �������ж�һ��ָ����Render�Ƿ��Ѿ����ӵ����ɼ��ˡ�
 *
 *  ������ʽ�����������������ء�
 *
 *  uInstID��[IN] ʵ��ID������pgLiveInitialize()ʱ�����
 *
 *  lpszRender��[IN] ָ��Render��P2P�ڵ�ID
 *
 *  ����ֵ��0Ϊ�ɹ���С��0Ϊ�����루��ö�١�PG_LIVE_ERROR_E���Ķ��壩
 */
PG_DLL_API
int pgLiveRenderConnected(unsigned int uInstID, const char* lpszRender);


/**
 *  ��������ʼ¼�Ʊ�����Ƶ����Ƶ��
 *
 *  ������ʽ�����������������ء�
 *
 *  uInstID��[IN] ʵ��ID������pgLiveInitialize()ʱ�����
 *
 *  sAviPath��[IN] ¼�Ƶ�ý���ļ�·�����ļ�������չ��Ϊ ".avi", ".mp4", "mov"��
 *
 *  bVideo��[IN] �Ƿ�¼����Ƶ��1Ϊ¼�ƣ�0Ϊ��¼��
 *
 *  bAudio��[IN] �Ƿ�¼����Ƶ��1Ϊ¼�ƣ�0Ϊ��¼��
 *
 *  ����ֵ��0Ϊ�ɹ���С��0Ϊ�����루��ö�١�PG_LIVE_ERROR_E���Ķ��壩
 */
PG_DLL_API
int pgLiveRecordStart(unsigned int uInstID, const char* sAviPath, unsigned int bVideo, unsigned int bAudio);


/**
 *  ������ֹͣ¼�Ʊ�����Ƶ����Ƶ��
 *
 *  ������ʽ�����������������ء�
 *
 *  uInstID��[IN] ʵ��ID������pgLiveInitialize()ʱ�����
 *
 *  ����ֵ����
 */
PG_DLL_API
void pgLiveRecordStop(unsigned int uInstID);


/**
 *  ��������P2P����������һ������
 *
 *  ������ʽ�����������������ء�
 *
 *  uInstID��[IN] ʵ��ID������pgLiveInitialize()ʱ�����
 *
 *  lpszData��[IN] �����͵����ݣ������ַ�����
 *
 *  uParam��[IN] �Զ����������Ŀ����ʹ�����Ӧ���ܹ���ƥ�䣩��
 *
 *  ����ֵ��0Ϊ�ɹ���С��0Ϊ�����루��ö�١�PG_LIVE_ERROR_E���Ķ��壩
 */
PG_DLL_API
int pgLiveSvrRequest(unsigned int uInstID, const char* lpszData, unsigned int uParam);


/**
 *  ���������������Ƶת����Դ��
 *
 *  ������ʽ�����������������ء�
 *
 *  uInstID��[IN] ʵ��ID������pgLiveInitialize()ʱ�����
 *
 *  ����ֵ��0Ϊ�ɹ���С��0Ϊ�����루��ö�١�PG_LIVE_ERROR_E���Ķ��壩
 */
PG_DLL_API
int pgLiveForwardAlloc(unsigned int uInstID);


/**
 *  �����������ͷ���Ƶת����Դ��
 *
 *  ������ʽ�����������������ء�
 *
 *  uInstID��[IN] ʵ��ID������pgLiveInitialize()ʱ�����
 *
 *  ����ֵ��0Ϊ�ɹ���С��0Ϊ�����루��ö�١�PG_LIVE_ERROR_E���Ķ��壩
 */
PG_DLL_API
int pgLiveForwardFree(unsigned int uInstID);


/**
 *  �����������ϴ��ļ������Ŷˡ�
 *
 *  ������ʽ�����������������ء�
 *
 *  uInstID��[IN] ʵ��ID������pgLiveInitialize()ʱ�����
 * 
 *  lpszRender��[IN] ���Ŷ˵�P2P ID
 *
 *  lpszPath��[IN] ָ���ϴ����ļ�·����ȫ·����
 *  
 *  lpszPeerPath��[IN] �ļ��ڲ��Ŷ˴洢�����·����
 *                ����˲������գ���SDK�Զ���lpszPath�����н�ȡ�ļ�����Ϊ��������
 *
 *  ����ֵ��0Ϊ�ɹ���С��0Ϊ�����루��ö�١�PG_LIVE_ERROR_E���Ķ��壩
 */
PG_DLL_API
int pgLiveFilePutRequest(unsigned int uInstID, const char* lpszRender,
	const char* lpszPath, const char* lpszPeerPath);


/**
 *  ����������Ӳ��Ŷ������ļ���
 *
 *  ������ʽ�����������������ء�
 *
 *  uInstID��[IN] ʵ��ID������pgLiveInitialize()ʱ�����
 * 
 *  lpszRender��[IN] ���Ŷ˵�P2P ID
 *
 *  lpszPath��[IN] ָ������������ļ���·����ȫ·����
 *  
 *  lpszPeerPath��[IN] �ļ��ڲ��Ŷ˴洢�����·����
 *                ����˲������գ���SDK�Զ���lpszPath�����н�ȡ�ļ�����Ϊ��������
 *
 *  ����ֵ��0Ϊ�ɹ���С��0Ϊ�����루��ö�١�PG_LIVE_ERROR_E���Ķ��壩
 */
PG_DLL_API
int pgLiveFileGetRequest(unsigned int uInstID, const char* lpszRender,
	const char* lpszPath, const char* lpszPeerPath);


/**
 *  ���������ܲ��Ŷ˵��ļ��ϴ����ļ���������
 *
 *  ������ʽ�����������������ء�
 *
 *  uInstID��[IN] ʵ��ID������pgLiveInitialize()ʱ�����
 * 
 *  lpszRender��[IN] ���Ŷ˵�P2P ID
 *
 *  lpszPath��[IN] ָ��������ϴ��ļ���·������ָ�������ļ���·����ȫ·����
 *  
 *  ����ֵ��0Ϊ�ɹ���С��0Ϊ�����루��ö�١�PG_LIVE_ERROR_E���Ķ��壩
 */
PG_DLL_API
int pgLiveFileAccept(unsigned int uInstID,
	const char* lpszRender, const char* lpszPath);


/**
 *  �������ܾ����Ŷ˵��ļ��ϴ����ļ���������
 *
 *  ������ʽ�����������������ء�
 *
 *  uInstID��[IN] ʵ��ID������pgLiveInitialize()ʱ�����
 * 
 *  lpszRender��[IN] ���Ŷ˵�P2P ID
 *  
 *  ����ֵ��0Ϊ�ɹ���С��0Ϊ�����루��ö�١�PG_LIVE_ERROR_E���Ķ��壩
 */
PG_DLL_API
int pgLiveFileReject(unsigned int uInstID, const char* lpszRender);


/**
 *  ������ȡ�����жϣ����ڽ��е��ļ����䡣
 *
 *  ������ʽ�����������������ء�
 *
 *  uInstID��[IN] ʵ��ID������pgLiveInitialize()ʱ�����
 * 
 *  lpszRender��[IN] ���Ŷ˵�P2P ID
 *  
 *  ����ֵ��0Ϊ�ɹ���С��0Ϊ�����루��ö�١�PG_LIVE_ERROR_E���Ķ��壩
 */
PG_DLL_API
int pgLiveFileCancel(unsigned int uInstID, const char* lpszRender);


/**
 *  ������������־����ļ���
 *
 *  ������ʽ�����������������ء�
 *
 *  uLevel��[IN] ��־�������
 *          0����Ҫ����־��Ϣ��Ĭ�Ͽ�������
 *          1�������϶൫��Ҫ����־��Ϣ��Ĭ�Ϲرգ���
 *
 *  uEnable��[IN] 0���رգ���0��������
 *
 *  ����ֵ������0Ϊ�ɹ���С��0Ϊ������
 */
PG_DLL_API
int pgLiveLevel(unsigned int uLevel, unsigned int uEnable);


/**
 *  ��������ȡ��ģ��İ汾��
 *
 *  ������ʽ�����������������ء�
 *
 *  lpszVersion��[OUT] ���ܰ汾��Ϣ�Ļ�������
 *
 *  uSize��[IN] �������ĳ��ȣ�������ڵ���16�ֽڣ�
 */
PG_DLL_API
void pgLiveVersion(char* lpszVersion, unsigned int uSize);



#ifdef __cplusplus
}
#endif


#endif //_PG_LIB_LIVE_CAP_H
