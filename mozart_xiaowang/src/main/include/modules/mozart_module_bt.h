#ifndef __MOZART_MODULE_BT_H__
#define __MOZART_MODULE_BT_H__

#include "bluetooth_interface.h"

/* A2DP Source */
#define SUPPORT_BSA_A2DP_SOURCE			0
/* A2DP Synk */
#define SUPPORT_BSA_A2DP_SYNK			1

/* HFP AG */
#define SUPPORT_BSA_HFP_AG			0
/* HFP HF */
#define SUPPORT_BSA_HFP_HF			1

/* phone book server */
#define SUPPORT_BSA_PBS				0
/* phone book client */
#define SUPPORT_BSA_PBC				0

/* OPP Server */
#define SUPPORT_BSA_OPS				0
/* OPP Client */
#define SUPPORT_BSA_OPC				0

/* SPP Server */
#define SUPPORT_BSA_SPPS			0
/* SPP Client */
#define SUPPORT_BSA_SPPC			0

/* BSA BLE SUPPORT */
#define SUPPORT_BSA_BLE				1
#define SUPPORT_BSA_BLE_SERVER			1
#define SUPPORT_BSA_BLE_CLIENT			1

/* BSA BLE HID SUPPORT */
#define SUPPORT_BSA_BLE_HH			0

/* BLE HID DIALOG DEVICE */
#define SUPPORT_BSA_BLE_HH_DIALOG_AUDIO		0

/* BSA HFP HF RESAMPLE SUPPORT */
#define SUPPORT_BSA_HS_RESAMPLE			0
#define SUPPORT_BSA_HS_RESAMPLE_8K_to_48K	0

/* Automatic Echo Cancellation RESAMPLE SUPPORT */
#define SUPPORT_AEC_RESAMPLE			0
#define SUPPORT_AEC_RESAMPLE_48K_TO_8K		0

extern int start_bt(void);
extern int stop_bt(void);

#endif /* __MOZART_MODULE_BT_H__ */
