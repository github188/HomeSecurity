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
extern LoggerPtr g_logger_sdk;

#else

#define LOG4CXX_TRACE(a,b)
#define LOG4CXX_DEBUG(a,b)
#define LOG4CXX_INFO(a,b)
#define LOG4CXX_WARN(a,b)
#define LOG4CXX_ERROR(a,b)
#define LOG4CXX_FATAL(a,b)

#endif


#define STR_PTCP_HAS_CLOSED "TCP_CLOSED"
#define STR_PTCP_HAS_ERROR	"TCP_ERROR"


enum PushMessage
{
	PTCP_ERROR		= 3103,
	PTCP_CLOSED		= 3104,
};

enum IReturnCode
{
	SUCCESS = 0,

	PTCP_HAS_CLOSED		= 7, 						

	REQ_RES_TIMEOUT		= -5,
	CONN_OTHER_ERROR	= -6,
	CANT_CONNECT_SERVER = -7,
	JSON_INVALID		= -8,
	REQ_RES_OTHER_ERROR = -9,
	JSON_PARSE_ERROR	= -10,

	OTHER_ERROR			= -1000,
};


#endif // user_interface_defines_h__
