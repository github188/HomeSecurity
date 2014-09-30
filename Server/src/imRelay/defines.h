/*
 * defines.h
 *
 *  Created on: 2012-9-12
 *      Author: yaowei
 */

#ifndef DEFINES_H_
#define DEFINES_H_

#include <assert.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>
#include <errno.h>
#include <arpa/inet.h>

#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>
#include <boost/thread/thread.hpp>

#include <log4cxx/logger.h>
#include <log4cxx/logstring.h>
#include <log4cxx/propertyconfigurator.h>
using namespace log4cxx;
extern LoggerPtr g_logger;

#include <event.h>

#include "../public/user_interface_defines.h"

#define DATA_BUFFER_SIZE 30*1024
#define CRLF "\r\n"

typedef struct
{
	char buf[DATA_BUFFER_SIZE];
	unsigned int len;
	int  sfd;
}LOCAL_REV_DATA;

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
		online_status  =  OFFLINE;
		login_platform =  UNKNOWN;
		language_type  =  UNKNOWN;
		alarm_flag	   =  ALARM_OFF;
	}
}
ACCOUNT_LIVE_INFO;

#endif /* DEFINES_H_ */
