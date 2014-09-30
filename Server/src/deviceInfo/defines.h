#ifndef DEFINES_H_
#define DEFINES_H_

#include <assert.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>
#include <errno.h>
#include <list>

#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>
#include <boost/thread/thread.hpp>
#include <boost/bind.hpp>

#include <log4cxx/logger.h>
#include <log4cxx/logstring.h>
#include <log4cxx/propertyconfigurator.h>

using namespace log4cxx;
extern LoggerPtr g_logger;

#include <event.h>
#include <semaphore.h>
#include "threadSafe_map.h"

#pragma pack(1)

#define DATA_BUFFER_SIZE 30*1024
#define CRLF "\r\n"
#define DEVICE_LIST_LIMIT 100

typedef struct
{
	char buf[DATA_BUFFER_SIZE];
	unsigned int len;
	evutil_socket_t  sfd;
	int  flag;
	struct event_base 	*base;
}LOCAL_REV_DATA;

struct ClientInfo
{
	int sfd;
	struct sockaddr_in clientAddr;
};

enum serviceType
{
	ST_DEVICESERVER = 0x0a,
};

enum messageType
{
	MT_HEARTBEAT = 0x01,
	MT_FULL = 0x02,
	MT_CHANGE = 0x03,
	MT_GETDEVICEINFO = 0x04,
};

typedef struct packet_head_
{
	uint32_t packet_len;
	uint8_t service_type;
	uint8_t message_type;
}PACKET_HEAD;

typedef struct device_info_cloudsee_
{
	std::string device_guid;
	uint8_t online_flag;
	uint8_t channel_num;
	uint8_t byte_stream;
	uint8_t device_net_stat;
	uint32_t device_type;

	device_info_cloudsee_()
	{
		online_flag = 0;
		channel_num = 0;
		byte_stream = 0;
		device_net_stat = 0;
		device_type = 0;
	}
}DEVICE_INFO_CLOUDSEE;

typedef struct net_msg
{
	sem_t* h_event;
	char response[DATA_BUFFER_SIZE];
}NET_MSG;

#pragma pack()


#endif /* DEFINES_H_ */
