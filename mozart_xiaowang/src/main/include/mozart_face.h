

#ifndef _MOZART_FACE_H
#define _MOZART_FACE_H

#include <linux/ioctl.h> /* needed for the _IOW etc stuff used later */


#define HT1628_IOC_MAGIC  	'H'
#define HT1628_IOC_MAXNR 	6



#define INNO_IOCTL_HT1628_DISPLAY_OFF     _IOW(HT1628_IOC_MAGIC, 3, int)
#define INNO_IOCTL_HT1628_SEND_BYTE_DATA     _IOW(HT1628_IOC_MAGIC, 4, int)
#define INNO_IOCTL_HI1628_STB_SET     _IOW(HT1628_IOC_MAGIC, 5, int)
#define INNO_IOCTL_KEYLED_BACKLIGHT_SET     _IOW(HT1628_IOC_MAGIC, 6, int)

#define FACE_LIKE 0x01
#define FACE_SMILE 0x02
#define FACE_SHY 0x03
#define FACE_THINK 0x04
#define FACE_SURPRISE 0x05
#define FACE_ANGRY 0x06
#define FACE_SAD 0x07
#define FACE_EMBARRASSED 0x08
#define FACE_YUN 0x09
#define FACE_CRY 0x0a
#define FACE_NEXT 0x0b
#define FACE_PREVIOUS 0x0c
#define FACE_MUSIC 0x0d
#define FACE_LOCAL 0x0e
#define FACE_BT 0x0f


#define FACE_WIFI_1 0x11
#define FACE_WIFI_2 0x12
#define FACE_WIFI_3 0x13
#define FACE_WIFI_4 0x14
#define FACE_WIFI_5 0x15

#define FACE_WIFI_6 0x16
#define FACE_WIFI_7 0x17
#define FACE_WIFI_8 0x18
#define FACE_WIFI_9 0x19

#define FACE_PAUSE  0x1a
#define FACE_PLAY  0x1b




int ht1628_display_face(unsigned char data);

int ht1628_display(unsigned char data);

int led_music_playing();

int create_answer_pthread(void);

int create_alarm_pthread(int cnt);

#endif 
