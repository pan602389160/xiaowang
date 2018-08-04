#ifndef __MOZART_MODULE_MULROOM_H__
#define __MOZART_MODULE_MULROOM_H__

#ifdef  __cplusplus
extern "C" {
#endif

typedef enum {
	MR_AO_NORMAL,
	MR_AO_DISTRIBUTOR,
} mulroom_ao_mode_t;

/**
 * module_mulroom_audio_change -
 * @mode:
 */
extern int module_mulroom_audio_change(mulroom_ao_mode_t mode);

/**
 * module_mulroom_get_gateip -
 * @gateip_buf:
 */
extern int module_mulroom_get_gateip(char *gateip_buf);

/**
 * module_mulroom_get_hostname -
 */
extern char *module_mulroom_get_hostname(void);

/**
 * module_mulroom_group_master -
 * @group_id:
 */
extern int module_mulroom_group_master(int group_id);

/**
 * module_mulroom_group_slave -
 * @group_id:
 * @slave_name:
 * @gate_ip:
 */
extern int module_mulroom_group_slave(
	int group_id,
	char *slave_name,
	char *gate_ip);

/**
 * module_mulroom_group_dismiss -
 */
extern int module_mulroom_group_dismiss(void);

#ifdef  __cplusplus
}
#endif

#endif /* __MOZART_MODULE_MULROOM_H__ */
