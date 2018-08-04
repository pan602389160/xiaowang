#ifndef _ASR_H_
#define _ASR_H_

/**
 * @brief ASR_SIZE 识别引擎每次录音buff大小（byte），默认（3200）.不需要修改.
 * 160000 * （16bit / 8） * (10 ms / 1s) = 3200（byte）， 即每10ms采样3200（byte）数据。
 */
#define ASR_SIZE 3200

/**
 * @brief RATE 采样通道，默认（1）.不需要修改.
 */
#define ASR_CHANNEL	       1

/**
 * @brief RATE 采样率，默认（16000）.不需要修改.
 */
#define ASR_RATE            16000

/**
 * @brief BIT 数据类型，默认（16bit）.不需要修改.
 */
#define ASR_BIT             16

/**
 * @brief VOLUME 麦克风录音音量，默认（60），客户可以根据具体产品进行调整. \n
 * 调整原则1：播放音量调整到最大时， 录音音频不能截幅. \n
 * 调整原则2：满足1的前提下， 录音音量越大，越容易识别远距离说话的声音. \n
 * 调整原则3：满足1，2的前提下， 录音频幅度在 10%—80% 范围内，效果最佳.
 */
#define ASR_VOLUME          60

/**
 * @brief asr_state_e 识别状态枚举.
 */
typedef enum {
	/**
	 * @brief 未在识别.
	 */
	ASR_IDLE,
	/**
	 * @brief 识别开始.
	 */
	ASR_START,
	/**
	 * @brief 说话结束.
	 */
	ASR_SPEAK_END,
	/**
	 * @brief 识别成功.
	 */
	ASR_SUCCESS,
	/**
	 * @brief 识别失败.
	 */
	ASR_FAIL,
	/**
	 * @brief 识别被终止.
	 */
	ASR_BREAK,
	/**
	 * @brief 状态最大值，只做判断用.
	 */
	ASR_MAX
} asr_state_e;

/**
 * @brief sds_state_e 多论对话状态枚举.
 */
typedef enum sds_state_e{
	/**
	 * @brief 非法状态.
	 */
	SDS_STATE_NULL,
	/**
	 * @brief 用户意图清晰，一般会给出最终结果，并退出多论交互.
	 */
	SDS_STATE_DO,
	/**
	 * @brief 对用户意图产生疑问，一般会继续多论交互，直到获取到清晰意图.
	 */
	SDS_STATE_QUERY,
	/**
	 * @brief 用户意图不清晰，一般会继续多论交互，直到获取到清晰意图.
	 */
	SDS_STATE_OFFERNONE,
	/**
	 * @brief 用户意图清晰，一般会退出多论交互.
	 */
	SDS_STATE_OFFER,
	/**
	 * @brief 继续多论对话标志.
	 */
	SDS_STATE_INTERACT,
	/**
	 * @brief 退出多论对话标志.
	 */
	SDS_STATE_EXIT,
	/**
	 * @brief 状态最大值，只做判断用.
	 */
	SDS_STATE_MAX
}sds_state_e;

/**
 * @brief domain_e （用户）意图枚举.
 */
typedef enum domain_e{
	/*** @brief 非法状态. */
	DOMAIN_NULL,
	/*** @brief 电影搜索.*/
	DOMAIN_MOVIE,
	/*** @brief 音乐.*/
	DOMAIN_MUSIC,
	/*** @brief 天气.*/
	DOMAIN_WEATHER,
	/*** @brief 中控.*/
	DOMAIN_COMMAND,
	/*** @brief 日历.*/
	DOMAIN_CALENDAR,
	/*** @brief 提醒.*/
	DOMAIN_REMINDER,
	/*** @brief 电台.*/
	DOMAIN_NETFM,
	/*** @brief 闲聊.*/
	DOMAIN_CHAT,
	/*** @brief 故事.*/
	DOMAIN_STORY,
	/*** @brief 笑话.*/
	DOMAIN_JOKE,
	/*** @brief 古诗.*/
	DOMAIN_POETRY,
	/*** @brief 菜谱.*/
	DOMAIN_COOKBOOK,
	/*** @brief 股票.*/
	DOMAIN_STOCK,
	/*** @brief 翻译.*/
	DOMAIN_TRANSLATE,
	/*** @brief 新闻. ---------------------从此开始为萝卜新增*/
	DOMAIN_NEWS,
	/*** @brief 音量控制.*/
	DOMAIN_VOLUMECONTROL,
	/*** @brief 退出.*/
	DOMAIN_QUIT,
	/*** @brief 状态最大值，只做判断用.*/

	DOMAIN_MAX
}domain_e;



/**
 * @brief music_info_t 音乐信息结构体.
 */
typedef struct music_info_t {
	/**
	 * @brief url 播放资源网址.
	 */
	char *url;

	/**
	 * @brief title 歌曲名.
	 */
	char *title;

	/**
	 * @brief title 歌手名.
	 */
	char *artist;
}music_info_t;


/**
 * @brief netfm_info_t 电台信息结构体
 */
typedef struct netfm_info_t {
	/**
	 * @brief data 电台名称.
	 */
	char *track;
	/**
	 * @brief data 播放地址.
	 */
	char *url;
}netfm_info_t;


/**
 * @brief sem_music_t (用户说的声音)识别的“音乐”信息结构体.
 */
typedef struct sem_music_t {
	/**
	 * @brief artist 歌曲名.
	 */
	char *artist;
	/**
	 * @brief title 歌手名.
	 */
	char *title;
	/**
	 * @brief style 风格.
	 */
	char *style;
	/**
	 * @brief type 音乐类型.
	 */
	char *type;
	/**
	 * @brief album 专辑名.
	 */
	char *album;
	/**
	 * @brief number 序列号.
	 */
	char *number;
	/**
	 * @brief operation 操作.
	 */
	char *operation;
	/**
	 * @brief volume 音量.
	 */
	char *volume;
	/**
	 * @brief tgt request tgt.
	 */
	char *tgt;
}sem_music_t;


/**
 * @brief sem_movie_t (用户说的声音)识别的“电影”信息结构体.
 */
typedef struct sem_movie_t {
	/**
	 * @brief name 片名.
	 */
	char *name;
	/**
	 * @brief player 主演.
	 */
	char *player;
	/**
	 * @brief director 导演.
	 */
	char *director;
	/**
	 * @brief type 类型.
	 */
	char *type;
	/**
	 * @brief area 国家地区.
	 */
	char *area;
	/**
	 * @brief language 语言.
	 */
	char *language;
	/**
	 * @brief crowd 适用人群.
	 */
	char *crowd;
	/**
	 * @brief series 系列.
	 */
	char *series;
	/**
	 * @brief sequence 序列号.
	 */
	char *sequence;
	/**
	 * @brief channel 频道.
	 */
	char *channel;
	/**
	 * @brief clarity 清晰度.
	 */
	char *clarity;
}sem_movie_t;


/**
 * @brief sem_weather_t (用户说的声音)识别的“天气”信息结构体.
 */
typedef struct sem_weather_t {
	/**
	 * @brief provinces 省份.
	 */
	char *provinces;
	/**
	 * @brief city 城市.
	 */
	char *city;
	/**
	 * @brief district 区域.
	 */
	char *district;
	/**
	 * @brief date 日期.
	 */
	char *date;
	/**
	 * @brief time 时间.
	 */
	char *time;
	/**
	 * @brief lunar 阴历日期.
	 */
	char *lunar;
	/**
	 * @brief meteorology 气象.
	 */
	char *meteorology;
}sem_weather_t;


/**
 * @brief sem_command_t (用户说的声音)识别的“中控”信息结构体.
 */
typedef struct sem_command_t {
	/**
	 * @brief device 设备名称.
	 */
	char *device;
	/**
	 * @brief number 设备编号.
	 */
	char *number;
	/**
	 * @brief operation 操作.
	 */
	char *operation;
	/**
	 * @brief position 位置.
	 */
	char *position;
	/**
	 * @brief floor 楼层.
	 */
	char *floor;
	/**
	 * @brief mode 模式.
	 */
	char *mode;
	/**
	 * @brief color 颜色.
	 */
	char *color;
	/**
	 * @brief timer 定时.
	 */
	char *timer;
	/**
	 * @brief channel 频道.
	 */
	char *channel;
	/**
	 * @brief tv_station 电视台.
	 */
	char *tv_station;
	/**
	 * @brief volume 音量.
	 */
	char *volume;
	/**
	 * @brief luminance 亮度.
	 */
	char *luminance;
	/**
	 * @brief temperature 温度.
	 */
	char *temperature;
	/**
	 * @brief humidity 湿度.
	 */
	char *humidity;
	/**
	 * @brief wind_speed 风速.
	 */
	char *wind_speed;
	/**
	 * @brief contrast 对比度.
	 */
	char *contrast;
	/**
	 * @brief color_temperature 色温.
	 */
	char *color_temperature;
	/**
	 * @brief time 时间.
	 */
	char *time;
	/**
	 * @brief date 日期.
	 */
	char *date;
	/**
	 * @brief group 分组.
	 */
	char *group;
	/**
	 * @brief quantity 数量.
	 */
	char *quantity;
}sem_command_t;


/**
 * @brief sem_reminder_t (用户说的声音)识别的“提醒”信息结构体.
 */
typedef struct sem_reminder_t {
	/**
	 * @brief time 时间.
	 */
	char *time;
	/**
	 * @brief date 日期.
	 */
	char *date;
	/**
	 * @brief event 事件.
	 */
	char *event;
	/**
	 * @brief object 对象.
	 */
	char *object;
	/**
	 * @brief quantity 数量.
	 */
	char *quantity;
	/**
	 * @brief operation 操作.
	 */
	char *operation;
	/**
	 * @brief repeat 重复.
	 */
	char *repeat;
}sem_reminder_t;


/**
 * @brief sem_netfm_t (用户说的声音)识别的“电台”信息结构体.
 */
typedef struct sem_netfm_t {
	/**
	 * @brief type 类别.
	 */
	char *type;
	/**
	 * @brief column 栏目.
	 */
	char *column;
	/**
	 * @brief item 节目.
	 */
	char *item;
	/**
	 * @brief artist 艺人.
	 */
	char *artist;
	/**
	 * @brief number 序列号.
	 */
	char *number;
	/**
	 * @brief operation 操作.
	 */
	char *operation;
	/**
	 * @brief radio 电台.
	 */
	char *radio;
	/**
	 * @brief channel 频道.
	 */
	char *channel;
}sem_netfm_t;


/**
 * @brief sem_story_t (用户说的声音)识别的“故事”信息结构体.
 */
typedef struct sem_story_t {
	/**
	 * @brief object 对象.
	 */
	char *object;
	/**
	 * @brief name 故事名.
	 */
	char *name;
	/**
	 * @brief operation 操作.
	 */
	char *operation;
	/**
	 * @brief type 故事类型.
	 */
	char *type;
}sem_story_t;


/**
 * @brief sem_story_t (用户说的声音)识别的“故事”信息结构体.
 */
typedef struct sem_poetry_t {
	/**
	 * @brief object 对象.
	 */
	char *object;
	/**
	 * @brief name 古诗名.
	 */
	char *name;
	/**
	 * @brief operation 操作.
	 */
	char *operation;
	/**
	 * @brief verse 诗句.
	 */
	char *verse;
	/**
	 * @brief place 位置.
	 */
	char *place;
	/**
	 * @brief writer 作者.
	 */
	char *writer;
	/**
	 * @brief search 查询对象.
	 */
	char *search;
}sem_poetry_t;


/**
 * @brief sem_cookbook_t (用户说的声音)识别的“菜谱”信息结构体.
 */
typedef struct sem_cookbook_t {
	/**
	 * @brief dish 菜名.
	 */
	char *dish;
}sem_cookbook_t;


/**
 * @brief sem_stock_t (用户说的声音)识别的“股票”信息结构体.
 */
typedef struct sem_stock_t {
	/**
	 * @brief name 股票名称.
	 */
	char *name;
	/**
	 * @brief tickers 股票代码.
	 */
	char *tickers;
	/**
	 * @brief index 指数.
	 */
	char *index;
	/**
	 * @brief qualification 技术指标.
	 */
	char *qualification;
	/**
	 * @brief industries 行业.
	 */
	char *industries;
}sem_stock_t;


/**
 * @brief sem_translate_t (用户说的声音)识别的“翻译”信息结构体.
 */
typedef struct sem_translate_t {
	/**
	 * @brief target_language 目标语言.
	 */
	char *target_language;
	/**
	 * @brief source_language 源语言.
	 */
	char *source_language;
	/**
	 * @brief content 内容.
	 */
	char *content;
}sem_translate_t;


/**
 * @brief sem_one_t 单个识别信息结构体.(一次识别可能会返回1个或者多个此类识别结果)
 */
typedef struct sem_one_t {
	/**
	 * @brief domain 领域.
	 */
	domain_e domain;
	union{
		/**
		 * @brief movie 电影.
		 */
		sem_movie_t movie;
		/**
		 * @brief music 音乐.
		 */
		sem_music_t music;
		/**
		 * @brief weather 天气.
		 */
		sem_weather_t weather;
		/**
		 * @brief command 中控.
		 */
		sem_command_t command;
		/**
		 * @brief reminder 提醒.
		 */
		sem_reminder_t reminder;
		/**
		 * @brief netfm 电台.
		 */
		sem_netfm_t netfm;
		/**
		 * @brief story 故事.
		 */
		sem_story_t story;
		/**
		 * @brief poetry 古诗.
		 */
		sem_poetry_t poetry;
		/**
		 * @brief cookbook 菜谱.
		 */
		sem_cookbook_t cookbook;
		/**
		 * @brief stock 股票.
		 */
		sem_stock_t stock;
		/**
		 * @brief translate 翻译.
		 */
		sem_translate_t translate;
	};
}sem_one_t;


/**
 * @brief NBEST_MAX 解析的非最佳识别结果的最多个数 （3）
 */
#define NBEST_MAX 3


/**
 * @brief sem_info_t (用户说的声音)识别信息结构体.
 */
typedef struct sem_info_t {
	/**
	 * @brief request 最优结果结构体.
	 */
	sem_one_t request;
	/**
	 * @brief translate 非最优结果个数.
	 */
	int nbest_number;
	/**
	 * @brief translate 非最优结果结构体.
	 */
	sem_one_t nbest[NBEST_MAX];
}sem_info_t;




/**
 * @brief MUSIC_MAX 解析的音乐内容最多个数 （20）
 */
#define MUSIC_MAX 20


/**
 * @brief sds_music_t 资源服务器返回的“音乐”内容结构体
 */
typedef struct sds_music_t {
	/**
	 * @brief number 数量.
	 */
	int number;
	/**
	 * @brief data 音乐内容数组，按顺序缓存返回的音乐列表.
	 */
	music_info_t data[MUSIC_MAX];
}sds_music_t;



/**
 * @brief NETFM_MAX 解析的电台内容最多个数 （10）
 */
#define NETFM_MAX 10


/**
 * @brief sds_netfm_t 资源服务器返回的“电台”内容结构体
 */
typedef struct sds_netfm_t {
	/**
	 * @brief number 数量.
	 */
	int number;
	/**
	 * @brief data 电台内容数组，按顺序缓存返回的电台列表.
	 */
	netfm_info_t data[NETFM_MAX];
}sds_netfm_t;


/**
 * @brief sds_weather_t 资源服务器返回的“天气”内容结构体
 */
typedef struct sds_weather_t {
	/**
	 * @brief meteorology 气象.
	 */
	char *meteorology;
	/**
	 * @brief temperature 温度.
	 */
	char *temperature;
	/**
	 * @brief wind 风向.
	 */
	char *wind;
	/**
	 * @brief area 位置.
	 */
	char *area;
}sds_weather_t;



/**
 * @brief sds_reminder_t 资源服务器返回的“提醒”内容结构体
 */
typedef struct sds_reminder_t {
	/**
	 * @brief date 日期.
	 */
	char *date;
	/**
	 * @brief time 时间.
	 */
	char *time;
	/**
	 * @brief event 事件.
	 */
	char *event;
}sds_reminder_t;


/**
 * @brief sds_chat_t 资源服务器返回的“对话”内容结构体. 对话中笑话等内容通过此返回.
 */
typedef struct sds_chat_t {
	/**
	 * @brief url 播放地址.
	 */
	char *url;
}sds_chat_t;



/**
 * @brief sds_info_t 资源服务器返回信息结构体.
 */
typedef struct sds_info_t {
	/**
	 * @brief state 多论对话的状态.
	 */
	sds_state_e state;
	/**
	 * @brief domain 意图.
	 */
	domain_e domain;
	/**
	 * @brief is_mult_sds 是否进行多论对话.
	 */
	bool is_mult_sds;
	/**
	 * @brief contextId 上下文ID. 多论对话需传递此信息.
	 */
	char *contextId;

	/**
	 * @brief env 当前意图. 多论对话需传递此信息.
	 */
	char *env;
	/**
	 * @brief output "推荐的”返回结果. 如果用户有特殊操作，可以忽略.
	 */
	char *output;
	/**
	 * @brief recordId 当前用户说话的录音ID. 调试用.
	 */
	char *recordId;
	union{
		/**
		 * @brief music “音乐”内容结构体.
		 */
		sds_music_t music;
		/**
		 * @brief netfm “电台”内容结构体.
		 */
		sds_netfm_t netfm;
		/**
		 * @brief weather “天气”内容结构体.
		 */
		sds_weather_t weather;
		/**
		 * @brief reminder “提醒”内容结构体.
		 */
		sds_reminder_t reminder;
		/**
		 * @brief chat “对话”内容结构体.
		 */
		sds_chat_t chat;
	};
}sds_info_t;

/**
 * @brief asr_info_t 识别信息结构体
 */
typedef struct asr_info_t {
	/**
	 * @brief state 识别状态
	 */
	asr_state_e state;

	/**
	 * @brief errId 引擎返回的错误ID， 正常为0
	 */
	int errId;

	/**
	 * @brief error 引擎返回的错误描述， 正常为空
	 */
	char *error;

	/**
	 * @brief input 用户的语言输入，引擎返回原始的文本
	 */
	char *input;

	/**
	 * @brief sem 语义解析结果
	 */
	sem_info_t sem;

	/**
	 * @brief sds 多论对话及内容结果
	 */
	sds_info_t sds;

}asr_info_t;

/**
 * @brief asr_start 启动识别引擎.
 * @param sds [in] 是否是一个多轮对话
 * @return 返回结果： 0 成功， -1：异常失败.
 */
//extern bool speak_start_flag ;

extern int asr_start(bool sds);

/**
 * @brief asr_stop 停止识别引擎.
 * @return 返回结果： 0 成功， -1：异常失败.
 */
extern int asr_stop(void);

/**
 * @brief asr_feed 往识别引擎送音频数据.
 * @param rec [in] 麦克风录音 buff.
 * @param size [in] 数据长度.
 * @return 返回结果： 0 成功， -1：异常失败.
 */
extern int asr_feed(const void *rec, int size);


/**
 * @brief asr_cancel 取消识别引擎.
 * @return 返回结果： 0 成功， -1：异常失败.
 *
 * @details 建议仅在如下情况下调用asr_cancel接口.\n
 * 在发生错误时, 即回调的json里有errId项.\n
 * 在取消当前录音操作时.
 */
extern int asr_cancel(void);



/**
 * @brief asr_break 终止识别引擎.
 * @details 启动识别后，需要手动停止识别引擎时调用.
 */
extern void asr_break(void);

#endif

