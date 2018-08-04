#ifndef _AEC_H_
#define _AEC_H_

/**
 * @brief AEC_SIZE 唤醒引擎每次录音buff大小（1024），默认（1024）.不需要修改.
 */
#define AEC_SIZE 1024

/**
 * @brief RATE 采样通道，默认（1）.不需要修改.
 */
#define AEC_CHANNEL	       1

/**
 * @brief RATE 采样率，默认（16000）.不需要修改.
 */
#define AEC_RATE            16000

/**
 * @brief BIT 数据类型，默认（16bit）.不需要修改.
 */
#define AEC_BIT             16

/**
 * @brief VOLUME 麦克风录音音量，默认（60），客户可以根据具体产品进行调整. \n
 * 调整原则1：播放音量调整到最大时， 录音音频不能截幅. \n
 * 调整原则2：满足1的前提下， 录音音量越大，越容易识别远距离说话的声音. \n
 * 调整原则3：满足1，2的前提下， 录音频幅度在 10%—80% 范围内，效果最佳.
 */
#define AEC_VOLUME          60

/**
 * @brief asr_info_t 唤醒信息结构体
 */
typedef struct aec_info_t {
	/**
	 * @brief result 唤醒返回结果
	 */
	int result;

	/**
	 * @brief wakeup 唤醒标志， true : 已经唤醒; false： 未唤醒
	 */
	bool wakeup;
}aec_info_t;

/**
 * @brief aec_start 启动唤醒引擎.
 * @return 返回结果： 0 成功， -1：异常失败.
 */
extern int aec_start(void);


/**
 * @brief aec_stop 停止唤醒引擎.
 * @return 返回结果： 0 成功， -1：异常失败.
 */
extern int aec_stop(void);

/**
 * @brief aec_feed 往唤醒引擎送音频数据.
 * @param  rec [in] 麦克风录音 buff.
 * @param  play [in] 回录音频录音 buff.
 * @param  size [in] 数据长度.
 * @return 返回结果： 0 成功， -1：异常失败.
 */
extern int aec_feed(const void *rec, const void *play, int size);


/**
 * @brief aec_key_wakeup 上报唤醒事件做唤醒
 *
 * @return 返回结果：9 成功，-1：异常失败.
 */
extern int aec_key_wakeup(void);
#endif

