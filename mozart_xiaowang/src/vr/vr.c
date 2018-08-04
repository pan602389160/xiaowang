#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>
#include <string.h>

#include "vr_interface.h"

#include "aec.h"
#include "asr.h"

#define CNT1000 1000
#define MS10 (10*1000)

/* vr logic thread */
static pthread_t vr_logic_thread;

/* callback to mozart */
static mozart_vr_callback callback_to_mozart = NULL;

/* vr init flag */
bool vr_init_flag = false;

/* vr working flag, change on vr_start() & vr_stop() */
bool vr_working_flag = false;

/* record current vr status */
vr_status_t vr_status = VR_IDLE;

/**
 * @brief next vr action: aec or asr.
 */
static vr_target_t vr_next = VR_TO_NULL;

/**
 * @brief storage vr result.
 */
static vr_info_t *info = NULL;

/**
 * @brief fast aec wakeup mode. It's possible that asr don't startup when user taking!!!
 */
static bool fast_aec_wakeup_mode;

/**
 * @brief state machine logic
 *
 * @param params NULL
 *
 * @return thread return
 */

#define LIGHT_GREEN   "\033[1;32m"
#define NONE          "\033[m"

static void *vr_logic_func(void *params)
{
	vr_result_t res;
	int cnt = 0;
	while (vr_init_flag) {
		while (vr_working_flag) {
			//if(!(tmp++%10))
			cnt = 0;
			switch (vr_next) {
			case VR_TO_AEC:
				printf("-------vr_logic_func----------VR_TO_AEC\n");
				vr_status = VR_AEC;
				mozart_aec_start();
				break;
			case VR_TO_ASR:
				printf("-------vr_logic_func----------VR_TO_ASR\n");
				vr_status = VR_ASR;
				mozart_asr_start(true);
				break;
			case VR_TO_ASR_SDS:
				printf("-------vr_logic_func----------VR_TO_ASR_SDS\n");
				vr_status = VR_ASR;
				mozart_asr_start(true);
				break;
			case VR_TO_WAIT:
				if (vr_working_flag) {
					usleep(5 * 1000);
					continue;
				} else {
					break;
				}
			default:
				printf("Unsupport vr_target: %d.\n", vr_next);
				break;
			}
			if (!vr_working_flag) {
				printf("-------vr_logic_func----------vr_working_flag == false\n");
				vr_status = VR_IDLE;
				printf("vr stopped!\n");
			} else if (info->from == VR_FROM_ASR) {
				/* waiting for asr result;
				 * asr callback #1 tells us speak end and begin recognition.
				 * asr callback #2 is asr result, then asr.state will become to ASR_SUCCESS */
				printf("-------vr_logic_func----------VR_FROM_ASR\n");
				while (info->asr.state != ASR_SUCCESS &&
					   info->asr.state != ASR_FAIL &&
					   info->asr.state != ASR_BREAK &&
					   cnt++ < CNT1000) {
					usleep(MS10);
				}

				if (info->asr.state == ASR_BREAK || cnt == CNT1000) {
					/* get result timeout or break, cancel current asr connect */
					printf("ASR_BREAK ASR_BREAK ASR_BREAK ASR_BREAK\n");
					asr_cancel();

					/* then, goto AEC*/
					vr_next = VR_TO_AEC;
				} else {
					/* got result */
					if (callback_to_mozart) {
						res = callback_to_mozart(info);
						vr_next = res.vr_next;
					} else {
						/* default next action: AEC */
						printf("vr_logic_func bb  default next action: AEC\n");
						vr_next = VR_TO_AEC;
					}
				}
			} else if (info->from == VR_FROM_AEC && !fast_aec_wakeup_mode) {
				/* TODO: error handle */

				if (callback_to_mozart) {
					res = callback_to_mozart(info);
					vr_next = res.vr_next;
				} else {
					/* default next action: AEC */
					printf("vr_logic_func aa  default next action: AEC\n");
					vr_next = VR_TO_AEC;
				}
			}
		}
		usleep(10 * 1000);

	}
	
	// make compile happy.
	return NULL;
}

static int callback_from_vr(vr_info_t *vr_info)
{
	if (vr_info->from == VR_FROM_AEC) {
		if (fast_aec_wakeup_mode) {
			vr_result_t res;

			if (callback_to_mozart) {
				res = callback_to_mozart(vr_info);
				vr_next = res.vr_next;
			} else {
				/* default next action: AEC */
				printf("callback_from_vr  default next action: AEC\n");
				vr_next = VR_TO_AEC;
			}
		}
		info = vr_info;
		mozart_aec_stop();
	} else if (vr_info->from == VR_FROM_ASR) {
		if (vr_info->asr.state == ASR_SPEAK_END)
			mozart_asr_stop(0);
		else if (vr_info->asr.state == ASR_BREAK)
			mozart_asr_stop(1);
		info = vr_info;
	} else if (vr_info->from == VR_FROM_CONTENT) {
		/* FIXME: waiting server solve err callback bug.*/
		//mozart_vr_content_get_stop();

		if (callback_to_mozart)
			callback_to_mozart(vr_info);
		return 0;
	}

	return 0;
}

void mozart_vr_set_fast_aec_wakeup_mode(bool fast)
{
	fast_aec_wakeup_mode = fast;
}

int mozart_vr_init(mozart_vr_callback callback)
{	
	int ret = 0;

	/* get callback to mozart */
	callback_to_mozart = callback;

	/* register inter callback */
	ret = ai_vr_init(callback_from_vr);
	if (ret) {
		printf("ai vr init error\n");
		goto err_vr_init_error;
	}

	vr_init_flag = true;

	ret = pthread_create(&vr_logic_thread, NULL, vr_logic_func, NULL);
	if (ret) {
		printf("create thread for vr logic error: %s\n", strerror(errno));
		goto err_create_thread_out;
	}

	return 0;

err_create_thread_out:
	callback_to_mozart = NULL;

err_vr_init_error:
	ai_vr_uninit();

	return ret;
}

int mozart_vr_start(void)
{	printf("------mozart_vr_start------------\n");
	if (!vr_init_flag) {
		printf("vr not init, %s fail.\n", __func__);
		return -1;
	}

	// wakeup vr machine logic
	vr_working_flag = true;

	// start aec deault.
	vr_next = VR_TO_AEC;
	printf("------vr_working_flag = %d\n",vr_working_flag);
	return 0;
}

int mozart_vr_enter_aec(void)
{	
	if (vr_next == VR_TO_WAIT) {
		vr_next = VR_TO_AEC;
		return 0;
	} else {
		return -1;
	}
}

int mozart_vr_enter_asr(bool sds)
{
	if (vr_next == VR_TO_WAIT) {
		if (sds)
			vr_next = VR_TO_ASR_SDS;
		else
			vr_next = VR_TO_ASR;
		return 0;
	} else {
		return -1;
	}
}

int mozart_vr_stop(void)
{
	if (!vr_init_flag || !vr_working_flag) {
		printf("111 warning: vr not init or not start,vr_init_flag = %d,vr_working_flag = %d %s fail.\n",vr_init_flag,vr_working_flag, __func__);
		return -1;
	}
	vr_working_flag = false;

	mozart_aec_stop();

	mozart_asr_stop(0);

	return 0;
}

int mozart_vr_uninit(void)
{
	if (!vr_init_flag) {
		printf("warning: vr not init, do not need %s.\n", __func__);
		return 0;
	}

	mozart_vr_stop();

	/* notify vr machine logic pthread stop */
	vr_init_flag = false;

	/* wait vr machine logic pthread stop */
	pthread_join(vr_logic_thread, NULL);

	/* uninit engine */
	ai_vr_uninit();

	return 0;
}

vr_status_t mozart_vr_get_status(void)
{
	return vr_status;
}
