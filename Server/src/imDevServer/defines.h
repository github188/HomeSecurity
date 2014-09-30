/*
 * defines.h
 *
 *  Created on: 2014-2-13
 *      Author: zhangs
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
#include "../public/redis_key.h"

#define DATA_BUFFER_SIZE 10240
#define CRLF "\r\n"

typedef struct
{
	char buf[DATA_BUFFER_SIZE];
	unsigned int len;
	int  sfd;
	struct event_base 	*base;
}LOCAL_REV_DATA;

typedef struct conn_queue_item CQ_ITEM;
struct conn_queue_item {
    int               sfd;
    int				  id;
    int               event_flags;
    int               read_buffer_size;
    CQ_ITEM          *next;
};

/* A connection queue. */
typedef struct conn_queue CQ;
struct conn_queue {
    CQ_ITEM *head;
    CQ_ITEM *tail;
    pthread_mutex_t lock;
    pthread_cond_t  cond;
};

typedef struct {
    pthread_t 			thread_id;        	/* unique ID of this thread */
    struct event_base 	*base;    			/* libevent handle this thread uses */
    struct event 		notify_event;  		/* listen event for notify pipe */
    int 				notify_receive_fd;  /* receiving end of notify pipe */
    int 				notify_send_fd;     /* sending end of notify pipe */
    struct conn_queue 	*new_conn_queue; 	/* queue of new connections to handle */
} LIBEVENT_THREAD;

typedef struct conn conn;
struct conn {
    int    sfd;
    int    id;
    char*  rBuf;
    int    rlen;
    char*  wBuf;
    int    wlen;
	bool   isVerify;
    conn   *next;     		 /* Used for generating a list of conn structures */
    LIBEVENT_THREAD *thread; /* Pointer to the thread object serving this connection */
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
		online_status  =  OFFLINE;
		login_platform =  UNKNOWN;
		language_type  =  UNKNOWN;
		alarm_flag	   =  ALARM_OFF;
	}
}
ACCOUNT_LIVE_INFO;

#endif /* DEFINES_H_ */
