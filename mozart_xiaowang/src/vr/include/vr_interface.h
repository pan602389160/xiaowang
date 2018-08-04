#ifndef __ASR_INTERFACE_H__
#define __ASR_INTERFACE_H__

#include <stdlib.h>

#include "aispeech/ai_vr.h"

#ifdef __cplusplus
extern "C" {
#endif

	/**
	 * @brief vr status
	 */
	typedef enum {
		/**
		 * @brief not in vr mode.
		 */
		VR_IDLE,
		/**
		 * @brief on aec mode
		 */
		VR_AEC,
		/**
		 * @brief on asr mode
		 */
		VR_ASR,
		/**
		 * @brief requesting content
		 */
		VR_CONTENT_GET,
	} vr_status_t;

	/**
	 * @brief vr target
	 */
	typedef enum {
		/**
		 * @brief useless, for init
		 */
		VR_TO_NULL,
		/**
		 * @brief goto AEC mode
		 */
		VR_TO_AEC,
		/**
		 * @brief goto ASR mode
		 */
		VR_TO_ASR,
		/**
		 * @brief goto ASR dialogue mode
		 */
		VR_TO_ASR_SDS,
		/**
		 * @brief goto wait mode
		 */
		VR_TO_WAIT,
	} vr_target_t;

	/**
	 * @brief vr target
	 */
	typedef struct {
		/**
		 * @brief next vr mode
		 */
		vr_target_t vr_next;
	} vr_result_t;

	/**
	 * @brief callback on asr/aec result arrived.
	 *
	 * @param vr [in] aec/asr result
	 *
	 * @return return only next vr mode now.
	 */
	typedef vr_result_t (*mozart_vr_callback)(vr_info_t *info);
	/**
	 * @brief init vr(engine only, will not start aec/asr)
	 *
	 * @param callback [in] on asr/aec result arrived.
	 *
	 * @return return 0 on success, return -1 otherwise.
	 */
	extern int mozart_vr_init(mozart_vr_callback callback);
	/**
	 * @brief set fast aec wakeup mode
	 */
	extern void mozart_vr_set_fast_aec_wakeup_mode(bool fast);
	/**
	 * @brief enter vr(aec) mode
	 *
	 * @return return 0 on success, return -1 otherwise.
	 */
	extern int mozart_vr_start(void);
	/**
	 * @brief enter aec mode
	 *
	 * @return return 0 on success, return -1 otherwise.
	 */
	extern int mozart_vr_enter_aec(void);
	/**
	 * @brief enter asr mode
	 *
	 * @param sds [in] whether is dialogue.
	 *
	 * @return return 0 on success, return -1 otherwise.
	 */
	extern int mozart_vr_enter_asr(bool sds);
	/**
	 * @brief exit vr mode, will not destory engine.
	 *
	 * @return return 0 on success, return -1 otherwise.
	 */
	extern int mozart_vr_stop(void);
	/**
	 * @brief uninit vr, will destory engine.
	 *
	 * @return return 0 on success, return -1 otherwise.
	 */
	extern int mozart_vr_uninit(void);
	/**
	 * @brief key wakeup
	 *
	 * @return return 0 on success, return -1 otherwise.
	 */
	extern int mozart_vr_aec_wakeup(void);
	/**
	 * @brief interrupt asr(take effect only in asr mode)
	 *
	 * @return return 0 on success, return -1 otherwise.
	 */
	extern int mozart_vr_asr_break(void);
	/**
	 * @brief get current vr status
	 *
	 * @return return current vr mode.
	 */
	extern vr_status_t mozart_vr_get_status(void);
	/**
	 * @brief search resource from aispeech.
	 *
	 * @param keyword [in] search keyword, pass NULL if want to get random music.
	 *
	 * @return return 0 on success, return -1 otherwise. pass search result via mozart_vr_callback.
	 */
	extern int mozart_vr_content_get(char *keyword);
	/**
	 * @brief interrupt search resource
	 *
	 * @return return 0 on success, return -1 otherwise.
	 */
	extern int mozart_vr_content_get_interrupt(void);
	/**
	 * @brief text 2 audio file
	 *
	 * @param word [in] text
	 *
	 * @return return /tmp/ai_tts.mp3 if success, return NULL otherwise.
	 */
	extern char *mozart_aispeech_tts(char *word);
	/**
	 * @brief stop aispeech tts
	 *
	 */
	extern void mozart_aispeech_tts_stop(void);
#ifdef __cplusplus
}
#endif

#endif // __ASR_INTERFACE_H__
