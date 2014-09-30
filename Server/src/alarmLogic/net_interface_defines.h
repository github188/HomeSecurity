#ifndef user_interface_defines_h__
#define user_interface_defines_h__

#ifdef WIN32
#include <winsock2.h>  
#include <windows.h>
#endif

#include <string>

#ifdef USE_LOG4CXX

#include <log4cxx/logger.h>
#include <log4cxx/logstring.h>
#include <log4cxx/propertyconfigurator.h>
using namespace log4cxx;
extern LoggerPtr g_logger;

#else

#define LOG4CXX_TRACE(a,b)
#define LOG4CXX_DEBUG(a,b)
#define LOG4CXX_INFO(a,b)
#define LOG4CXX_WARN(a,b)
#define LOG4CXX_ERROR(a,b)
#define LOG4CXX_FATAL(a,b)

#endif

/**
 * @brief   用户基本信息
 */
typedef struct userInfo_
{
	std::string username;
	std::string security_mail;
    //...
}USER_INFO;

/* 客户端类别 */
typedef enum apptype {
	APP_CLIENT, 		/* 客户端 */
	APP_MOBILE, 		/* 移动端 */
	APP_WEB 			/* web端 */
} apptype_t;

/* 手机系统类型 */
typedef enum mobiletype {
	MOBILE_NONE, 		/*非手机系统*/
	MOBILE_ANDROID, 	/* android系统 */
	MOBILE_IPHONE, 		/* iphone系统 */
	MOBILE_IPAD 		/* ipad系统 */
} mobiletype_t;

enum alarmflag
{
	ALARM_ON  = 0,
	ALARM_OFF = 1,
};

enum CommonStatus
{
	UNKNOWN = -1,
};

enum PushMessage
{
	NOTIFY_OFFLINE_ = 3102,
	PTCP_ERROR		= 3103,
	PTCP_CLOSED		= 3104,
};

enum OnlineStatus
{
	OFFLINE = 0,
	ONLINE,
};

typedef struct AccountLiveInfo
{
	std::string username;
	int	online_status;		//用户在线状态
	int login_platform;		//登录平台类型
	int	language_type;		//语言类型
	int alarm_flag;			//是否开启报警标志
	std::string moblie_id;	//手机唯一识别符

	AccountLiveInfo()
	{
		online_status  = OFFLINE;
		login_platform = UNKNOWN;
		language_type  = UNKNOWN;
		alarm_flag	   = ALARM_ON;
	}
}
ACCOUNT_LIVE_INFO;

typedef struct ServerPushInfo
{
	int message_type;
	std::string message;
	void* param;
}
SERVER_PUSH_INFO;

typedef void (*ServerPushCallBack_Info)(const SERVER_PUSH_INFO& serverPush);

#define STR_PTCP_HAS_CLOSED "TCP_CLOSED"
#define STR_PTCP_HAS_ERROR	"TCP_ERROR"

typedef void (*LiveStatusCB)(const int ret);

enum IReturnCode
{
	SUCCESS = 0,

	USER_HAS_EXIST		= 2,						//用户已经存在
	USER_NOT_EXIST		= 3,						//用户不存在
	PASSWORD_ERROR		= 4,						//密码错误
	SESSION_NOT_EXSIT	= 5,						//登录session不存在（登录已过期）
	SQL_NOT_FIND		= 6,						
	PTCP_HAS_CLOSED		= 7, 						

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

	OTHER_ERROR			= -1000,
};


#endif // user_interface_defines_h__
