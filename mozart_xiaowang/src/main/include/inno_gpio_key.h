/**********************************************
*
* FileName: inno_ht1628.h
* Author : baodingsheng 
* Desc: to testting gpio of MARW0043 module board  
* Data: 2015-07-14
***********************************************/
	
#ifndef _INNO_GPIO_KEY_H
#define _INNO_GPIO_KEY_H
	
#include <linux/ioctl.h> /* needed for the _IOW etc stuff used later */
	
#define INNO_GPIO_KEY_NAME	"inno_gpio_key"	/* device name */

	/* Use 'k' as magic number */
#define INNO_GPIO_KEY_IOC_MAGIC  		'K'
#define INNO_GPIO_KEY_IOC_MAXNR 		5
	
	/* Please use a different 8-bit number in your code */	
#define INNO_IOCTL_GPIO_GET_LOW_POWER_WARING_STATE_VALUE    _IOWR(INNO_GPIO_KEY_IOC_MAGIC, 1 , int)
#define INNO_IOCTL_GPIO_GET_LOW_POWER_OFF_STATE_VALUE    _IOWR(INNO_GPIO_KEY_IOC_MAGIC, 2 , int)
//#define INNO_IOCTL_GPIO_GET_BATTERY_CHRG_STATE_VALUE    _IOWR(INNO_GPIO_KEY_IOC_MAGIC, 3 , int)
//#define INNO_IOCTL_GPIO_GET_LOW_POWER_STATE_VALUE    _IOWR(INNO_GPIO_KEY_IOC_MAGIC, 4 , int)


#define INNO_IOCTL_GPIO_SET_INIT     _IOWR(INNO_GPIO_KEY_IOC_MAGIC, 5, int)
	

#define GPIO_NR_INNO_PA00	0
#define GPIO_NR_INNO_PA01	1
#define GPIO_NR_INNO_PA02	2
#define GPIO_NR_INNO_PA03	3
#define GPIO_NR_INNO_PA04	4
#define GPIO_NR_INNO_PA05	5
#define GPIO_NR_INNO_PA06	6
#define GPIO_NR_INNO_PA07	7
#define GPIO_NR_INNO_PA08	8
#define GPIO_NR_INNO_PA09	9
#define GPIO_NR_INNO_PA10	10
#define GPIO_NR_INNO_PA11	11
#define GPIO_NR_INNO_PA12	12
#define GPIO_NR_INNO_PA13	13
#define GPIO_NR_INNO_PA14	14
#define GPIO_NR_INNO_PA15	15
#define GPIO_NR_INNO_PA16	16
#define GPIO_NR_INNO_PA17	17
#define GPIO_NR_INNO_PA18	18
#define GPIO_NR_INNO_PA19	19
#define GPIO_NR_INNO_PA20	20
#define GPIO_NR_INNO_PA21	21
#define GPIO_NR_INNO_PA22	22
#define GPIO_NR_INNO_PA23	23
#define GPIO_NR_INNO_PA24	24
#define GPIO_NR_INNO_PA25	25
#define GPIO_NR_INNO_PA26	26
#define GPIO_NR_INNO_PA27	27
#define GPIO_NR_INNO_PA28	28
#define GPIO_NR_INNO_PA29	29
#define GPIO_NR_INNO_PA30	30
#define GPIO_NR_INNO_PA31	31

#define GPIO_NR_INNO_PB00	32
#define GPIO_NR_INNO_PB01	33
#define GPIO_NR_INNO_PB02	34
#define GPIO_NR_INNO_PB03	35
#define GPIO_NR_INNO_PB04	36
#define GPIO_NR_INNO_PB05	37
#define GPIO_NR_INNO_PB06	38
#define GPIO_NR_INNO_PB07	39
#define GPIO_NR_INNO_PB08	40
#define GPIO_NR_INNO_PB09	41
#define GPIO_NR_INNO_PB10	42
#define GPIO_NR_INNO_PB11	43
#define GPIO_NR_INNO_PB12	44
#define GPIO_NR_INNO_PB13	45
#define GPIO_NR_INNO_PB14	46
#define GPIO_NR_INNO_PB15	47
#define GPIO_NR_INNO_PB16	48
#define GPIO_NR_INNO_PB17	49
#define GPIO_NR_INNO_PB18	50
#define GPIO_NR_INNO_PB19	51
#define GPIO_NR_INNO_PB20	52
#define GPIO_NR_INNO_PB21	53
#define GPIO_NR_INNO_PB22	54
#define GPIO_NR_INNO_PB23	55
#define GPIO_NR_INNO_PB24	56
#define GPIO_NR_INNO_PB25	57
#define GPIO_NR_INNO_PB26	58
#define GPIO_NR_INNO_PB27	59
#define GPIO_NR_INNO_PB28	60
#define GPIO_NR_INNO_PB29	61
#define GPIO_NR_INNO_PB30	62
#define GPIO_NR_INNO_PB31	63

#define GPIO_NR_INNO_PC00	64
#define GPIO_NR_INNO_PC01	65
#define GPIO_NR_INNO_PC02	66
#define GPIO_NR_INNO_PC03	67
#define GPIO_NR_INNO_PC04	68
#define GPIO_NR_INNO_PC05	69
#define GPIO_NR_INNO_PC06	70
#define GPIO_NR_INNO_PC07	71
#define GPIO_NR_INNO_PC08	72
#define GPIO_NR_INNO_PC09	73
#define GPIO_NR_INNO_PC10	74
#define GPIO_NR_INNO_PC11	75
#define GPIO_NR_INNO_PC12	76
#define GPIO_NR_INNO_PC13	77
#define GPIO_NR_INNO_PC14	78
#define GPIO_NR_INNO_PC15	79
#define GPIO_NR_INNO_PC16	80
#define GPIO_NR_INNO_PC17	81
#define GPIO_NR_INNO_PC18	82
#define GPIO_NR_INNO_PC19	83
#define GPIO_NR_INNO_PC20	84
#define GPIO_NR_INNO_PC21	85
#define GPIO_NR_INNO_PC22	86
#define GPIO_NR_INNO_PC23	87
#define GPIO_NR_INNO_PC24	88
#define GPIO_NR_INNO_PC25	89
#define GPIO_NR_INNO_PC26	90
#define GPIO_NR_INNO_PC27	91
#define GPIO_NR_INNO_PC28	92
#define GPIO_NR_INNO_PC29	93
#define GPIO_NR_INNO_PC30	94
#define GPIO_NR_INNO_PC31	95


#define GPIO_NR_INNO_PD00	96
#define GPIO_NR_INNO_PD01	97
#define GPIO_NR_INNO_PD02	98	//UART1_TX
#define GPIO_NR_INNO_PD03	99	//UART1_RX
#define GPIO_NR_INNO_PD04	100
#define GPIO_NR_INNO_PD05	101
#define GPIO_NR_INNO_PD06	102
#define GPIO_NR_INNO_PD07	103
#define GPIO_NR_INNO_PD08	104
#define GPIO_NR_INNO_PD09	105
#define GPIO_NR_INNO_PD10	106
#define GPIO_NR_INNO_PD11	107
#define GPIO_NR_INNO_PD12	108
#define GPIO_NR_INNO_PD13	109
#define GPIO_NR_INNO_PD14	110
#define GPIO_NR_INNO_PD15	111
#define GPIO_NR_INNO_PD16	112
#define GPIO_NR_INNO_PD17	113
#define GPIO_NR_INNO_PD18	114
#define GPIO_NR_INNO_PD19	115
#define GPIO_NR_INNO_PD20	116
#define GPIO_NR_INNO_PD21	117
#define GPIO_NR_INNO_PD22	118
#define GPIO_NR_INNO_PD23	119
#define GPIO_NR_INNO_PD24	120
#define GPIO_NR_INNO_PD25	121
#define GPIO_NR_INNO_PD26	122
#define GPIO_NR_INNO_PD27	123
#define GPIO_NR_INNO_PD28	124
#define GPIO_NR_INNO_PD29	125
#define GPIO_NR_INNO_PD30	126
#define GPIO_NR_INNO_PD31	127

//#define GPIO_NR_INNO_PD02	98	//UART1_TX
//#define GPIO_NR_INNO_PD03	99	//UART1_RX

//#define GPIO_NR_INNO_PD04	100	//UART1_CTS, key mode
//#define GPIO_NR_INNO_PD05	101	//UART1_RTS, key play

#endif // _INNO_GPIO_KEY_H

