#ifndef _ASR_H_
#define _ASR_H_

/**
 * @brief ASR_SIZE ʶ������ÿ��¼��buff��С��byte����Ĭ�ϣ�3200��.����Ҫ�޸�.
 * 160000 * ��16bit / 8�� * (10 ms / 1s) = 3200��byte���� ��ÿ10ms����3200��byte�����ݡ�
 */
#define ASR_SIZE 3200

/**
 * @brief RATE ����ͨ����Ĭ�ϣ�1��.����Ҫ�޸�.
 */
#define ASR_CHANNEL	       1

/**
 * @brief RATE �����ʣ�Ĭ�ϣ�16000��.����Ҫ�޸�.
 */
#define ASR_RATE            16000

/**
 * @brief BIT �������ͣ�Ĭ�ϣ�16bit��.����Ҫ�޸�.
 */
#define ASR_BIT             16

/**
 * @brief VOLUME ��˷�¼��������Ĭ�ϣ�60�����ͻ����Ը��ݾ����Ʒ���е���. \n
 * ����ԭ��1�������������������ʱ�� ¼����Ƶ���ܽط�. \n
 * ����ԭ��2������1��ǰ���£� ¼������Խ��Խ����ʶ��Զ����˵��������. \n
 * ����ԭ��3������1��2��ǰ���£� ¼��Ƶ������ 10%��80% ��Χ�ڣ�Ч�����.
 */
#define ASR_VOLUME          60

/**
 * @brief asr_state_e ʶ��״̬ö��.
 */
typedef enum {
	/**
	 * @brief δ��ʶ��.
	 */
	ASR_IDLE,
	/**
	 * @brief ʶ��ʼ.
	 */
	ASR_START,
	/**
	 * @brief ˵������.
	 */
	ASR_SPEAK_END,
	/**
	 * @brief ʶ��ɹ�.
	 */
	ASR_SUCCESS,
	/**
	 * @brief ʶ��ʧ��.
	 */
	ASR_FAIL,
	/**
	 * @brief ʶ����ֹ.
	 */
	ASR_BREAK,
	/**
	 * @brief ״̬���ֵ��ֻ���ж���.
	 */
	ASR_MAX
} asr_state_e;

/**
 * @brief sds_state_e ���۶Ի�״̬ö��.
 */
typedef enum sds_state_e{
	/**
	 * @brief �Ƿ�״̬.
	 */
	SDS_STATE_NULL,
	/**
	 * @brief �û���ͼ������һ���������ս�������˳����۽���.
	 */
	SDS_STATE_DO,
	/**
	 * @brief ���û���ͼ�������ʣ�һ���������۽�����ֱ����ȡ��������ͼ.
	 */
	SDS_STATE_QUERY,
	/**
	 * @brief �û���ͼ��������һ���������۽�����ֱ����ȡ��������ͼ.
	 */
	SDS_STATE_OFFERNONE,
	/**
	 * @brief �û���ͼ������һ����˳����۽���.
	 */
	SDS_STATE_OFFER,
	/**
	 * @brief �������۶Ի���־.
	 */
	SDS_STATE_INTERACT,
	/**
	 * @brief �˳����۶Ի���־.
	 */
	SDS_STATE_EXIT,
	/**
	 * @brief ״̬���ֵ��ֻ���ж���.
	 */
	SDS_STATE_MAX
}sds_state_e;

/**
 * @brief domain_e ���û�����ͼö��.
 */
typedef enum domain_e{
	/*** @brief �Ƿ�״̬. */
	DOMAIN_NULL,
	/*** @brief ��Ӱ����.*/
	DOMAIN_MOVIE,
	/*** @brief ����.*/
	DOMAIN_MUSIC,
	/*** @brief ����.*/
	DOMAIN_WEATHER,
	/*** @brief �п�.*/
	DOMAIN_COMMAND,
	/*** @brief ����.*/
	DOMAIN_CALENDAR,
	/*** @brief ����.*/
	DOMAIN_REMINDER,
	/*** @brief ��̨.*/
	DOMAIN_NETFM,
	/*** @brief ����.*/
	DOMAIN_CHAT,
	/*** @brief ����.*/
	DOMAIN_STORY,
	/*** @brief Ц��.*/
	DOMAIN_JOKE,
	/*** @brief ��ʫ.*/
	DOMAIN_POETRY,
	/*** @brief ����.*/
	DOMAIN_COOKBOOK,
	/*** @brief ��Ʊ.*/
	DOMAIN_STOCK,
	/*** @brief ����.*/
	DOMAIN_TRANSLATE,
	/*** @brief ����. ---------------------�Ӵ˿�ʼΪ�ܲ�����*/
	DOMAIN_NEWS,
	/*** @brief ��������.*/
	DOMAIN_VOLUMECONTROL,
	/*** @brief �˳�.*/
	DOMAIN_QUIT,
	/*** @brief ״̬���ֵ��ֻ���ж���.*/

	DOMAIN_MAX
}domain_e;



/**
 * @brief music_info_t ������Ϣ�ṹ��.
 */
typedef struct music_info_t {
	/**
	 * @brief url ������Դ��ַ.
	 */
	char *url;

	/**
	 * @brief title ������.
	 */
	char *title;

	/**
	 * @brief title ������.
	 */
	char *artist;
}music_info_t;


/**
 * @brief netfm_info_t ��̨��Ϣ�ṹ��
 */
typedef struct netfm_info_t {
	/**
	 * @brief data ��̨����.
	 */
	char *track;
	/**
	 * @brief data ���ŵ�ַ.
	 */
	char *url;
}netfm_info_t;


/**
 * @brief sem_music_t (�û�˵������)ʶ��ġ����֡���Ϣ�ṹ��.
 */
typedef struct sem_music_t {
	/**
	 * @brief artist ������.
	 */
	char *artist;
	/**
	 * @brief title ������.
	 */
	char *title;
	/**
	 * @brief style ���.
	 */
	char *style;
	/**
	 * @brief type ��������.
	 */
	char *type;
	/**
	 * @brief album ר����.
	 */
	char *album;
	/**
	 * @brief number ���к�.
	 */
	char *number;
	/**
	 * @brief operation ����.
	 */
	char *operation;
	/**
	 * @brief volume ����.
	 */
	char *volume;
	/**
	 * @brief tgt request tgt.
	 */
	char *tgt;
}sem_music_t;


/**
 * @brief sem_movie_t (�û�˵������)ʶ��ġ���Ӱ����Ϣ�ṹ��.
 */
typedef struct sem_movie_t {
	/**
	 * @brief name Ƭ��.
	 */
	char *name;
	/**
	 * @brief player ����.
	 */
	char *player;
	/**
	 * @brief director ����.
	 */
	char *director;
	/**
	 * @brief type ����.
	 */
	char *type;
	/**
	 * @brief area ���ҵ���.
	 */
	char *area;
	/**
	 * @brief language ����.
	 */
	char *language;
	/**
	 * @brief crowd ������Ⱥ.
	 */
	char *crowd;
	/**
	 * @brief series ϵ��.
	 */
	char *series;
	/**
	 * @brief sequence ���к�.
	 */
	char *sequence;
	/**
	 * @brief channel Ƶ��.
	 */
	char *channel;
	/**
	 * @brief clarity ������.
	 */
	char *clarity;
}sem_movie_t;


/**
 * @brief sem_weather_t (�û�˵������)ʶ��ġ���������Ϣ�ṹ��.
 */
typedef struct sem_weather_t {
	/**
	 * @brief provinces ʡ��.
	 */
	char *provinces;
	/**
	 * @brief city ����.
	 */
	char *city;
	/**
	 * @brief district ����.
	 */
	char *district;
	/**
	 * @brief date ����.
	 */
	char *date;
	/**
	 * @brief time ʱ��.
	 */
	char *time;
	/**
	 * @brief lunar ��������.
	 */
	char *lunar;
	/**
	 * @brief meteorology ����.
	 */
	char *meteorology;
}sem_weather_t;


/**
 * @brief sem_command_t (�û�˵������)ʶ��ġ��пء���Ϣ�ṹ��.
 */
typedef struct sem_command_t {
	/**
	 * @brief device �豸����.
	 */
	char *device;
	/**
	 * @brief number �豸���.
	 */
	char *number;
	/**
	 * @brief operation ����.
	 */
	char *operation;
	/**
	 * @brief position λ��.
	 */
	char *position;
	/**
	 * @brief floor ¥��.
	 */
	char *floor;
	/**
	 * @brief mode ģʽ.
	 */
	char *mode;
	/**
	 * @brief color ��ɫ.
	 */
	char *color;
	/**
	 * @brief timer ��ʱ.
	 */
	char *timer;
	/**
	 * @brief channel Ƶ��.
	 */
	char *channel;
	/**
	 * @brief tv_station ����̨.
	 */
	char *tv_station;
	/**
	 * @brief volume ����.
	 */
	char *volume;
	/**
	 * @brief luminance ����.
	 */
	char *luminance;
	/**
	 * @brief temperature �¶�.
	 */
	char *temperature;
	/**
	 * @brief humidity ʪ��.
	 */
	char *humidity;
	/**
	 * @brief wind_speed ����.
	 */
	char *wind_speed;
	/**
	 * @brief contrast �Աȶ�.
	 */
	char *contrast;
	/**
	 * @brief color_temperature ɫ��.
	 */
	char *color_temperature;
	/**
	 * @brief time ʱ��.
	 */
	char *time;
	/**
	 * @brief date ����.
	 */
	char *date;
	/**
	 * @brief group ����.
	 */
	char *group;
	/**
	 * @brief quantity ����.
	 */
	char *quantity;
}sem_command_t;


/**
 * @brief sem_reminder_t (�û�˵������)ʶ��ġ����ѡ���Ϣ�ṹ��.
 */
typedef struct sem_reminder_t {
	/**
	 * @brief time ʱ��.
	 */
	char *time;
	/**
	 * @brief date ����.
	 */
	char *date;
	/**
	 * @brief event �¼�.
	 */
	char *event;
	/**
	 * @brief object ����.
	 */
	char *object;
	/**
	 * @brief quantity ����.
	 */
	char *quantity;
	/**
	 * @brief operation ����.
	 */
	char *operation;
	/**
	 * @brief repeat �ظ�.
	 */
	char *repeat;
}sem_reminder_t;


/**
 * @brief sem_netfm_t (�û�˵������)ʶ��ġ���̨����Ϣ�ṹ��.
 */
typedef struct sem_netfm_t {
	/**
	 * @brief type ���.
	 */
	char *type;
	/**
	 * @brief column ��Ŀ.
	 */
	char *column;
	/**
	 * @brief item ��Ŀ.
	 */
	char *item;
	/**
	 * @brief artist ����.
	 */
	char *artist;
	/**
	 * @brief number ���к�.
	 */
	char *number;
	/**
	 * @brief operation ����.
	 */
	char *operation;
	/**
	 * @brief radio ��̨.
	 */
	char *radio;
	/**
	 * @brief channel Ƶ��.
	 */
	char *channel;
}sem_netfm_t;


/**
 * @brief sem_story_t (�û�˵������)ʶ��ġ����¡���Ϣ�ṹ��.
 */
typedef struct sem_story_t {
	/**
	 * @brief object ����.
	 */
	char *object;
	/**
	 * @brief name ������.
	 */
	char *name;
	/**
	 * @brief operation ����.
	 */
	char *operation;
	/**
	 * @brief type ��������.
	 */
	char *type;
}sem_story_t;


/**
 * @brief sem_story_t (�û�˵������)ʶ��ġ����¡���Ϣ�ṹ��.
 */
typedef struct sem_poetry_t {
	/**
	 * @brief object ����.
	 */
	char *object;
	/**
	 * @brief name ��ʫ��.
	 */
	char *name;
	/**
	 * @brief operation ����.
	 */
	char *operation;
	/**
	 * @brief verse ʫ��.
	 */
	char *verse;
	/**
	 * @brief place λ��.
	 */
	char *place;
	/**
	 * @brief writer ����.
	 */
	char *writer;
	/**
	 * @brief search ��ѯ����.
	 */
	char *search;
}sem_poetry_t;


/**
 * @brief sem_cookbook_t (�û�˵������)ʶ��ġ����ס���Ϣ�ṹ��.
 */
typedef struct sem_cookbook_t {
	/**
	 * @brief dish ����.
	 */
	char *dish;
}sem_cookbook_t;


/**
 * @brief sem_stock_t (�û�˵������)ʶ��ġ���Ʊ����Ϣ�ṹ��.
 */
typedef struct sem_stock_t {
	/**
	 * @brief name ��Ʊ����.
	 */
	char *name;
	/**
	 * @brief tickers ��Ʊ����.
	 */
	char *tickers;
	/**
	 * @brief index ָ��.
	 */
	char *index;
	/**
	 * @brief qualification ����ָ��.
	 */
	char *qualification;
	/**
	 * @brief industries ��ҵ.
	 */
	char *industries;
}sem_stock_t;


/**
 * @brief sem_translate_t (�û�˵������)ʶ��ġ����롱��Ϣ�ṹ��.
 */
typedef struct sem_translate_t {
	/**
	 * @brief target_language Ŀ������.
	 */
	char *target_language;
	/**
	 * @brief source_language Դ����.
	 */
	char *source_language;
	/**
	 * @brief content ����.
	 */
	char *content;
}sem_translate_t;


/**
 * @brief sem_one_t ����ʶ����Ϣ�ṹ��.(һ��ʶ����ܻ᷵��1�����߶������ʶ����)
 */
typedef struct sem_one_t {
	/**
	 * @brief domain ����.
	 */
	domain_e domain;
	union{
		/**
		 * @brief movie ��Ӱ.
		 */
		sem_movie_t movie;
		/**
		 * @brief music ����.
		 */
		sem_music_t music;
		/**
		 * @brief weather ����.
		 */
		sem_weather_t weather;
		/**
		 * @brief command �п�.
		 */
		sem_command_t command;
		/**
		 * @brief reminder ����.
		 */
		sem_reminder_t reminder;
		/**
		 * @brief netfm ��̨.
		 */
		sem_netfm_t netfm;
		/**
		 * @brief story ����.
		 */
		sem_story_t story;
		/**
		 * @brief poetry ��ʫ.
		 */
		sem_poetry_t poetry;
		/**
		 * @brief cookbook ����.
		 */
		sem_cookbook_t cookbook;
		/**
		 * @brief stock ��Ʊ.
		 */
		sem_stock_t stock;
		/**
		 * @brief translate ����.
		 */
		sem_translate_t translate;
	};
}sem_one_t;


/**
 * @brief NBEST_MAX �����ķ����ʶ������������ ��3��
 */
#define NBEST_MAX 3


/**
 * @brief sem_info_t (�û�˵������)ʶ����Ϣ�ṹ��.
 */
typedef struct sem_info_t {
	/**
	 * @brief request ���Ž���ṹ��.
	 */
	sem_one_t request;
	/**
	 * @brief translate �����Ž������.
	 */
	int nbest_number;
	/**
	 * @brief translate �����Ž���ṹ��.
	 */
	sem_one_t nbest[NBEST_MAX];
}sem_info_t;




/**
 * @brief MUSIC_MAX �������������������� ��20��
 */
#define MUSIC_MAX 20


/**
 * @brief sds_music_t ��Դ���������صġ����֡����ݽṹ��
 */
typedef struct sds_music_t {
	/**
	 * @brief number ����.
	 */
	int number;
	/**
	 * @brief data �����������飬��˳�򻺴淵�ص������б�.
	 */
	music_info_t data[MUSIC_MAX];
}sds_music_t;



/**
 * @brief NETFM_MAX �����ĵ�̨���������� ��10��
 */
#define NETFM_MAX 10


/**
 * @brief sds_netfm_t ��Դ���������صġ���̨�����ݽṹ��
 */
typedef struct sds_netfm_t {
	/**
	 * @brief number ����.
	 */
	int number;
	/**
	 * @brief data ��̨�������飬��˳�򻺴淵�صĵ�̨�б�.
	 */
	netfm_info_t data[NETFM_MAX];
}sds_netfm_t;


/**
 * @brief sds_weather_t ��Դ���������صġ����������ݽṹ��
 */
typedef struct sds_weather_t {
	/**
	 * @brief meteorology ����.
	 */
	char *meteorology;
	/**
	 * @brief temperature �¶�.
	 */
	char *temperature;
	/**
	 * @brief wind ����.
	 */
	char *wind;
	/**
	 * @brief area λ��.
	 */
	char *area;
}sds_weather_t;



/**
 * @brief sds_reminder_t ��Դ���������صġ����ѡ����ݽṹ��
 */
typedef struct sds_reminder_t {
	/**
	 * @brief date ����.
	 */
	char *date;
	/**
	 * @brief time ʱ��.
	 */
	char *time;
	/**
	 * @brief event �¼�.
	 */
	char *event;
}sds_reminder_t;


/**
 * @brief sds_chat_t ��Դ���������صġ��Ի������ݽṹ��. �Ի���Ц��������ͨ���˷���.
 */
typedef struct sds_chat_t {
	/**
	 * @brief url ���ŵ�ַ.
	 */
	char *url;
}sds_chat_t;



/**
 * @brief sds_info_t ��Դ������������Ϣ�ṹ��.
 */
typedef struct sds_info_t {
	/**
	 * @brief state ���۶Ի���״̬.
	 */
	sds_state_e state;
	/**
	 * @brief domain ��ͼ.
	 */
	domain_e domain;
	/**
	 * @brief is_mult_sds �Ƿ���ж��۶Ի�.
	 */
	bool is_mult_sds;
	/**
	 * @brief contextId ������ID. ���۶Ի��贫�ݴ���Ϣ.
	 */
	char *contextId;

	/**
	 * @brief env ��ǰ��ͼ. ���۶Ի��贫�ݴ���Ϣ.
	 */
	char *env;
	/**
	 * @brief output "�Ƽ��ġ����ؽ��. ����û���������������Ժ���.
	 */
	char *output;
	/**
	 * @brief recordId ��ǰ�û�˵����¼��ID. ������.
	 */
	char *recordId;
	union{
		/**
		 * @brief music �����֡����ݽṹ��.
		 */
		sds_music_t music;
		/**
		 * @brief netfm ����̨�����ݽṹ��.
		 */
		sds_netfm_t netfm;
		/**
		 * @brief weather �����������ݽṹ��.
		 */
		sds_weather_t weather;
		/**
		 * @brief reminder �����ѡ����ݽṹ��.
		 */
		sds_reminder_t reminder;
		/**
		 * @brief chat ���Ի������ݽṹ��.
		 */
		sds_chat_t chat;
	};
}sds_info_t;

/**
 * @brief asr_info_t ʶ����Ϣ�ṹ��
 */
typedef struct asr_info_t {
	/**
	 * @brief state ʶ��״̬
	 */
	asr_state_e state;

	/**
	 * @brief errId ���淵�صĴ���ID�� ����Ϊ0
	 */
	int errId;

	/**
	 * @brief error ���淵�صĴ��������� ����Ϊ��
	 */
	char *error;

	/**
	 * @brief input �û����������룬���淵��ԭʼ���ı�
	 */
	char *input;

	/**
	 * @brief sem ����������
	 */
	sem_info_t sem;

	/**
	 * @brief sds ���۶Ի������ݽ��
	 */
	sds_info_t sds;

}asr_info_t;

/**
 * @brief asr_start ����ʶ������.
 * @param sds [in] �Ƿ���һ�����ֶԻ�
 * @return ���ؽ���� 0 �ɹ��� -1���쳣ʧ��.
 */
//extern bool speak_start_flag ;

extern int asr_start(bool sds);

/**
 * @brief asr_stop ֹͣʶ������.
 * @return ���ؽ���� 0 �ɹ��� -1���쳣ʧ��.
 */
extern int asr_stop(void);

/**
 * @brief asr_feed ��ʶ����������Ƶ����.
 * @param rec [in] ��˷�¼�� buff.
 * @param size [in] ���ݳ���.
 * @return ���ؽ���� 0 �ɹ��� -1���쳣ʧ��.
 */
extern int asr_feed(const void *rec, int size);


/**
 * @brief asr_cancel ȡ��ʶ������.
 * @return ���ؽ���� 0 �ɹ��� -1���쳣ʧ��.
 *
 * @details ���������������µ���asr_cancel�ӿ�.\n
 * �ڷ�������ʱ, ���ص���json����errId��.\n
 * ��ȡ����ǰ¼������ʱ.
 */
extern int asr_cancel(void);



/**
 * @brief asr_break ��ֹʶ������.
 * @details ����ʶ�����Ҫ�ֶ�ֹͣʶ������ʱ����.
 */
extern void asr_break(void);

#endif

