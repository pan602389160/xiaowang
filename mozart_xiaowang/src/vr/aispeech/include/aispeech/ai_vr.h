#ifndef __AI_VR_H__
#define __AI_VR_H__
#include <stdbool.h>

/**
 * @brief VALGRIND_TEST 测试模式.
 */
#ifdef VALGRIND_TEST
//#define SUPPORT_AEC
#else
#define SUPPORT_AEC
#endif
#include "aispeech/aiengine.h"
#include "aispeech/aiengine_app.h"
#ifdef SUPPORT_AEC
#include "aispeech/aec.h"
extern struct aiengine *wakeup_agn;
#endif

/* asr,tts,content use sds_agn */
extern struct aiengine *sds_agn;
#include "aispeech/asr.h"
#include "aispeech/tts.h"
#include "aispeech/content.h"

/**
 * @brief vr_from_e 产生语音callback来源枚举.
 */
typedef enum vr_from_e{
	/**
	 * @brief 来自AEC的callback.
	 */
	VR_FROM_AEC,

	/**
	 * @brief 来自ASR的callback.
	 */
	VR_FROM_ASR,

	/**
	 * @brief 来自获取资源的callback.
	 */
	VR_FROM_CONTENT
} vr_from_e;

/**
 * @brief vr_info_t 语音处理信息结构体.
 */
typedef struct vr_info_t {
	/**
	 * @brief stop 停止语音处理标志位.
	 */
	bool stop;

	/**
	 * @brief from 产生callback时的语音状态： VR_FROM_AEC; VR_FROM_ASR.
	 */
	vr_from_e from;
	union{
		/**
		 * @brief aec 语音唤醒信息结构体.
		 */
		aec_info_t aec;

		/**
		 * @brief asr 语音识别信息结构体.
		 */
		asr_info_t asr;

		/**
		 * @brief content 获取资源信息结构体.
		 */
		content_info_t content;
	};

} vr_info_t;

/**
 * @brief ai_vr_info vr结构体.
 */
extern vr_info_t ai_vr_info;

/**
 * @brief vr_callback 语音服务的callback函数定义
 * @param  vr_info [in] 语言信息结构体.
 * @return 返回结果： 0 成功， -1：异常失败.
 */
typedef int (*vr_callback)(vr_info_t *vr_info);

/**
 * @brief vr_callback 语音服务的callback函数
 */
extern vr_callback ai_vr_callback;

/**
 * @brief ai_vr_init aispeech 语音初始化函数，在使用语音引擎前需先调用.
 * @param _callback [in] 语音服务的callback.
 * @return 返回结果： 0 成功， -1：异常失败.
 */
extern int ai_vr_init(vr_callback _callback);

/**
 * @brief ai_vr_uninit aispeech 语音反初始化函数，要销毁语音引擎时调用
 * @return 返回结果： 0 成功， -1：异常失败.
 */
extern int ai_vr_uninit(void);

/**
 * @brief vr_domain_str domain的类型
 */
extern const char *vr_domain_str[];

#endif
