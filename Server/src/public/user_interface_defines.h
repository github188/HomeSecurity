#ifndef user_interface_defines_h__
#define user_interface_defines_h__

#ifdef WIN32
#include <winsock2.h>  
#include <windows.h>
#endif

#include <string>
#include <vector>
#include <string.h>
#include <time.h>
#include <stdint.h>

/**
 * @brief   用户基本信息
 */
typedef struct userInfo_
{
	std::string username;
	std::string security_mail;
    //...
}USER_INFO;

enum OnlineStatus
{
	OFFLINE = 0,
	ONLINE,
};

enum alarmflag
{
	ALARM_ON  = 0,
	ALARM_OFF = 1,
};

enum CommonStatus
{
	UNKNOWN = -1,
	MOBILE_NONE = 0,		/*非手机系统*/
	MOBILE_ANDROID = 1,		/* android系统 */
	MOBILE_IPHONE = 2,		/* iphone系统 */
	MOBILE_IPAD	=3			/* ipad系统 */
};

enum PwdLevel
{
	PWD_LOW = 0,
	PWD_HIGH = 1,
};

typedef struct clientPlatformInfo_
{
	int terminal_type;		//登录平台类型
	int	language_type;		//语言类型
	std::string moblie_id;	//手机唯一识别符

	clientPlatformInfo_()
	{
		terminal_type = UNKNOWN;
		language_type  = UNKNOWN;
	}
}
CLIENT_PLATFORM_INFO;

/* 用户信息表 */
typedef struct AccountInfo
{
	std::string username;
	std::string mail;
	std::string phone;
	std::string nickname;
}AccountInfo;

/* 客户端版本信息 */
typedef struct SoftVersionInfo
{
	std::string soft_version;
	std::string soft_version_addr;
	std::string soft_version_desp;
}SoftVersionInfo;

#define STR_PTCP_HAS_CLOSED "TCP_CLOSED"
#define STR_PTCP_HAS_ERROR	"TCP_ERROR"

enum PushMessage
{
	NOTIFY_OFFLINE_ = 3102,
	PTCP_ERROR = 3103,
	PTCP_CLOSED = 3104,
};

/**
 * @brief   设备信息
 */
enum DeviceType
{
	DEV_TYPE_UNKNOWN = 0,
	DEV_TYPE_IPC = 1,
	DEV_TYPE_HOME_IPC = 2,
	DEV_TYPE_DVR = 3,
	DEV_TYPE_NVR = 4,
	DEV_TYPE_JNVR = 5,
	DEV_TYPE_CARD = 6,
};
enum DeviceSubType
{
	DEV_TYPE_SUB_UNKNOWN = 0,
	DEV_TYPE_IPC_H200 = 1,
	DEV_TYPE_IPC_H400 = 2,
	DEV_TYPE_IPC_E2 = 3,
	DEV_TYPE_IPC_E2_0130 = 31,
};
typedef struct deviceInfo_
{
	std::string device_guid;		// 设备guid(云视通号)
	int device_type;				// 设备类型 0-未知 1-DVR 2-IPC
	int device_sub_type;			// 设备型号id
	std::string device_sub_type_str;// 字符串型号
	std::string device_version;		// 设备软件版本
	std::string device_username;	// 设备用户名
	std::string device_password;	// 设备密码
	std::string device_name;		// 设备昵称
	std::string device_ip;			// 设备ip
	int device_netstat;				// IPC网络状态，0-有线连接，1-wifi连接
	int net_storage_switch;			// 云存储开关，0-关闭，1-打开
	int tf_storage_switch;			// TF存储开关
	int alarm_switch;				// 告警开关，0-关闭，1-打开
	std::string alarm_video_ftp_url;// 告警录像的ftp上传路径
	std::string alarm_snap_ftp_url;	// 告警抓拍图的ftp上传路径
	std::string alarm_ftp_acc;		// 告警FTP的用户名
	std::string alarm_ftp_pwd;		// 告警FTP的密码
	std::string alarm_time;			// 安全防护时间段 格式hh,hh
	std::string jpeg_ftp_url_big;	// 场景大图ftp路径
	std::string jpeg_ftp_url_small;	// 场景小图ftp路径
	std::string jpeg_ftp_acc;		// 场景图ftp用户名
	std::string jpeg_ftp_pwd;		// 场景图ftp密码
	int jpeg_upload_timing;			// 场景图上传时间间隔 单位sec
	int video_fluency;				// 视频流畅度，0-标清，1-高清，2-流畅
	int baby_mode;					// baby模式 0-关闭,1开启
	int full_alarm_mode;			// 全监控模式 0-关闭,1开启
	int online_status;				// (1在线 0离线)
	int humiture_flag;				// 是否支持温湿度 0-不支持 1-支持
	struct tm timestamp;			// 时间戳
	double temperature;				// 温度
	double humidity;				// 湿度
	int reset_flag;					// 出厂设置标识 1为出厂状态 0非出厂状态
	int device_verify;				// 设备用户名密码校验0成功 -1失败

	deviceInfo_()
	{
		device_type = 0;
		device_sub_type = 0;
		device_netstat = 0;
		net_storage_switch = 0;
		tf_storage_switch = 0;
		alarm_switch = 0;
		jpeg_upload_timing = 0;
		video_fluency = 0;
		baby_mode = 0;
		full_alarm_mode = 0;
		humiture_flag = 0;
		memset(&timestamp, 0, sizeof (timestamp));
		temperature = 0;
		humidity = 0;
		reset_flag = -1;
		device_verify = 0;
	}
}DEVICE_INFO;

/**
 * @brief   用户自身配置的设备信息
 */
typedef struct user_device_config_
{
	std::string user_name;			// 用户名
	std::string device_guid;		// 设备GUID
	int video_link_type;			// 视频连接模式 0-云视通，1-IP端口
	std::string video_username;		// 云视通连接模式 用户名
	std::string video_password;		// 云视通连接模式 密码
	std::string video_ip;			// 普通连接模式 IP
	int video_port;					// 普通连接模式 端口
	int device_verify;				// 设备用户名密码校验0成功 -1失败
	int device_permission;			// 用户的设备权限

	user_device_config_()
	{
		video_link_type = 0;
		video_port = 0;
		device_verify = 0;
		device_permission = 0;
	}
}USER_DEVICE_CONFIG;

/**
 * @brief   用户设备信息
 */
typedef struct user_device_info_
{
	DEVICE_INFO device_info;
	USER_DEVICE_CONFIG user_device_config;
}USER_DEVICE_INFO;

/**
 * @brief   设备信息列表
 */
typedef struct devicesList_
{
	std::vector<DEVICE_INFO> vec_device_info;
}DEVICES_LIST;

/**
 * @brief   用户-设备信息列表
 */
typedef struct userDevicesList_
{
	std::vector<USER_DEVICE_INFO> vec_user_device_info;
}USER_DEVICES_LIST;

/**
 * @brief 设备环境信息(温湿度)
*/
typedef struct device_environment_info_
{
	std::string device_guid;
	struct tm timestamp;
	double temperature;
	double humidity;
	int score;
	int assessment;
	
	device_environment_info_()
	{
		memset(&timestamp, 0, sizeof (struct tm));
		temperature = 0;
		humidity = 0;
		score = 0;
		assessment = 0;
	}

	bool operator>(const device_environment_info_ &denvinfo) const
	{
		return score > denvinfo.score;
	}
}DEVICE_ENV_INFO;

typedef struct humiture_statistics_info_
{
	std::string device_guid;
	std::string date;
	int hour;
	double statTemperature;
	double statHumidity;
	int score;
	int assessment;

	humiture_statistics_info_()
	{
		hour = 0;
		statTemperature = 0;
		statHumidity = 0;
		score = 0;
		assessment = 0;
	}
}HUMITURE_STAT_INFO;

/**
 * @brief 设备升级文件信息
*/
typedef struct updateFileInfo_
{
	std::string	update_version;
	std::string file_url;
	int file_size;
	std::string file_checksum;
	std::string file_description;
}UPDATE_FILE_INFO;

/**
 * @brief   设备通道信息
 */
typedef struct deviceChannelInfo_
{
	int channel_no;
	int channel_permission;
	std::string channel_name;
}DEVICE_CHANNEL_INFO;

/**
 * @brief 报警类型
 */
typedef enum alarmtype
{
	ALARM_DISKERROR,			/*存储错误告警 Level 2*/
	ALARM_DISKFULL,				/*存储满告警 Level 1*/
	ALARM_DISCONN,				/*连接断开告警 Level 1*/
	ALARM_UPGRADE,				/*系统升级告警 Level 2*/	
	ALARM_GPIN,					/*外接告警 Level 2*/
	ALARM_VIDEOLOSE,			/*视频丢失告警 Level 2*/
	ALARM_HIDEDETECT,			/*视频遮挡告警 Level 3*/
	ALARM_MOTIONDETECT,			/*移动侦测 Level 3*/
	ALARM_POWER_OFF,			/*断电告警 Level 1*/
	ALARM_MANUALALARM,			/*手动告警 Level 1*/
	ALARM_GPS,					/*GPS告警 Level 3*/
	ALARM_NONE,					/*无报警*/
	ALARM_NOTIFY,				/*有报警，报警通知*/
	ALARM_TEMP_HIGH = 16,		/*温度过高 Level 2*/
	ALARM_TEMP_LOW	= 32,		/*温度过低 Level 2*/
	ALARM_HUM_HIGH	= 64,		/*湿度过高 Level 2*/
	ALARM_HUM_LOW	= 128,		/*湿度过低 Level 2*/
	ALARM_TOTAL					
} alarmtype_t;

/**
 * @brief 报警消息
 */
typedef struct alarmInfo_
{
	std::string alarm_guid;
	int			alarm_msg_type;		// (报警消息类型：0文本消息，1图片上传完成，2视频上传完成)
	std::string device_guid;		// (设备云视通号)
	std::string device_name;		// (设备名)
	int			alarm_channel_no;	// (发生报警的通道号)
	int			alarm_type;			// (报警类型)
	std::string alarm_pic;			// (报警图片名称 可选)
	int			alarm_pic_size;		// (报警图片大小 可选)
	std::string alarm_video;		// (报警视频名称 可选)
	int			alarm_video_size;	// (报警视频大小 可选)
	time_t		alarm_timestamp;	// (时间戳 time的返回值。云视通号与时间戳定位一次报警)

	alarmInfo_()
	{
		alarm_msg_type = 0;
		alarm_channel_no = 0;
		alarm_type = 0;
		alarm_pic_size = 0;
		alarm_video_size = 0;
		alarm_timestamp = 0;
	}
}ALARM_INFO;

enum AlarmSolution
{
	AS_LOCAL = 0,
	AS_CLOUD = 1,
	AS_FTP = 2,
};

#define ALARM_MSG_LIST_LEN 50


enum IReturnCode
{
	SUCCESS = 0,
	FAILED  = -1,

	USER_HAS_EXIST		= 2,
	USER_NOT_EXIST		= 3,
	PASSWORD_ERROR		= 4,
	SESSION_NOT_EXSIT	= 5,
	SQL_NOT_FIND		= 6,
	PTCP_HAS_CLOSED		= 7,

	LOW_STRENGTH_PASSWORD	= 118,						//低强度密码用户
	HIGH_STRENGTH_PASSWORD	= 119,						//高强度密码用户

	/** DeviceRegisterResponse */
	DEVICE_HAS_EXIST = 8,
	DEVICE_NOT_EXIST = 9,
	DEVICE_NOT_BIND = 10,
	DEVICE_HIS_NOT_EXIST = 11,
	DEVICE_HUMITURE_NOT_EXIST = 12,
	DEVICE_CONF_NOT_SET = 13,
	DEVICE_HAS_NO_UPDATE = 14,
	DEVICE_IS_RESET = 15,
	DEVICE_HUM_NOT_SUPPORT = 16,
	DEVICE_HAS_BIND = 17,

	GENERATE_PASS_ERROR = -2,
	REDIS_OPT_ERROR		= -3,
	MY_SQL_ERROR		= -4,
	REQ_RES_TIMEOUT		= -5,
	CONN_OTHER_ERROR	= -6,
	CANT_CONNECT_SERVER = -7,
	JSON_INVALID		= -8,
	REQ_RES_OTHER_ERROR = -9,
	JSON_PARSE_ERROR	= -10,
	SEND_MAIL_FAILED	= -11,

	DEVICE_UPDATE_ERROR = -12,
	DEVICE_VERIFY_FAILED = -13,
	DEVICE_NOT_ONLINE = -14,

	ACCOUNTNAME_INVALID = -15,
	ACCOUNTNAME_OTHER 	= -16,
	PASSWORD_DANGER 	= -17,
	NEED_UPDATE 		= -18,
	
	RELAY_ERROR			= -19,
	DEVICE_RESULT_ERROR = -20,

	OTHER_ERROR			= -1000,
};

enum HumitureAssessment
{
	HUMITURE_OK				= 0,

	TEMPERATURE_TOO_HIGH	= 1,
	TEMPERATURE_TOO_LOW		= 2,

	HUMIDITY_TOO_HIGH		= 4,
	HUMIDITY_TOO_LOW		= 8,
};

#endif // user_interface_defines_h__
