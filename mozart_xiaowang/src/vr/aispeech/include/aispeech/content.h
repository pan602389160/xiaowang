#ifndef _CONTENT_H_
#define _CONTENT_H_

/**
 * @brief CONTENT_TIMEOUT 获取资源的超时时间（单位秒）.
 * @details 这个时间不要设置太长或者太短，建议设置范围：（5-15）
 */
#define CONTENT_TIMEOUT 10

/**
 * @brief CONTENT_RANDOM_MUSIC 获取随机歌曲列表的传入参数.
 */
#define CONTENT_RANDOM_MUSIC "我要听歌"

/**
 * @brief content_state_e 获取资源状态枚举
 */
typedef enum {
	/**
	 * @brief 未在获取资源.
	 */
	CONTENT_IDLE,
	/**
	 * @brief 获取资源开始.
	 */
	CONTENT_START,
	/**
	 * @brief 获取资源成功.
	 */
	CONTENT_SUCCESS,
	/**
	 * @brief 获取资源失败.
	 */
	CONTENT_FAIL,
	/**
	 * @brief 获取资源被终止.
	 */
	CONTENT_INTERRUPT,
	/**
	 * @brief 状态最大值，只做判断用.
	 */
	CONTENT_MAX
} content_state_e;

/**
 * @brief asr_info_t 获取资源信息结构体
 */
typedef struct content_info_t {
	/**
	 * @brief state 获取资源状态
	 */
	content_state_e state;

	/**
	 * @brief errId 引擎返回的错误ID， 正常为0
	 */
	int errId;

	/**
	 * @brief error 引擎返回的错误描述， 正常为空
	 */
	char *error;

	/**
	 * @brief sds 获取资源内容结果
	 */
	sds_info_t sds;

} content_info_t;

/**
 * @brief content_stop 停止获取资源.
 * @details 启动获取资源后，需要手动停止时调用.

 */
extern void content_stop(void);
extern void content_interrupt(void);

/**
 * @brief content 获取AISpeech服务器资源的接口（只能用来获取指定的音乐列表）.
 * @param text [in] 需要获取的资源关键字.
 * @brief 如果需要获取某个歌手的歌曲列表，直接传歌手名字.
 * @brief 如果需要获取随机的歌曲列表，直接传入 CONTENT_RANDOM_MUSIC.
 * @retval 返回0 成功， 返回1 失败.
 * @brief 注意： 如果调用成功，会分配一定的内存空间存储内容.需要在后面手动调用 content_free() 接口释放分配的内存，否则会造成内存泄漏.
 */
extern int content_get(char *text);

/**
 * @brief content_free 释放获取资源分配的内存.
 * @param content_info [in] 获取的资源信息结构体.
 */
extern void content_free(content_info_t *content_info);

#endif


