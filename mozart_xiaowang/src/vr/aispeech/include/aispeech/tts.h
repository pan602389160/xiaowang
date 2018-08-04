#ifndef _TTS_H_
#define _TTS_H_

/**
 * @brief TTS_NAME tts的音频缓存文件名称.
 */
#define TTS_NAME "/tmp/ai_tts.mp3"

/**
 * @brief TTS_TIMEOUT tts的超时时间（单位秒）.
 * @details 这个时间不要设置太长或者太短，建议设置范围：（5-15）
 */
#define TTS_TIMEOUT 15


/**
 * @brief tts_stop 停止TTS.
 * @details 启动TTS后，需要手动停止时调用.
 */
extern void tts_stop(void);

/**
 * @brief ai_tts aispeech的tts（TextToSpeech）功能.
 * @param tts_text [in] 需要合成的文本信息.
 * @retval 返回的播放地址.
 *
 * @details 调用此功能会一直阻塞，等到引擎返回完整的音频，或者超时（TTS_TIMEOUT）再退出阻塞.\n
 * 由于tts和识别，是调用同一个引擎，需在应用层保证不会同时调用2个引擎.\n
 * tts结束后，会将音频缓存的路径名称返回， 需要上层调用播放器播放.
 */
extern char *ai_tts(char *tts_text);

#endif


