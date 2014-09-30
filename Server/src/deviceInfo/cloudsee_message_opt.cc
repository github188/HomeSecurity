/*
 * cloudsee_message_opt.cc
 *
 *  Created on: Mar 30, 2014
 *      Author: zhangs
 */

#include "cloudsee_message_opt.h"
#include "redis_conn_pool.h"
#include "redis_opt.h"
#include "global_settings.h"
#include "../public/message.h"
#include "../public/socket_wrapper.h"
#include "../public/utils.h"

extern CThreadSafeMap<int, NET_MSG> g_map_message;


CCloudSeeMessageOpt::CCloudSeeMessageOpt(const int sfd)
{
	sfd_ = sfd;
}

CCloudSeeMessageOpt::~CCloudSeeMessageOpt()
{

}

void CCloudSeeMessageOpt::StartCloudSeeMessageOpt(char* message)
{
	message_ = message;
	dataOpt_.SetMessageData(message);

	int message_type = 0;
	if (!dataOpt_.ParseMessageType(message_type))
	{
		LOG4CXX_ERROR(g_logger, "CLogicOperate::StartLogicOpt:ParseMessageType error.");
		return;
	}

	redisContext* con = CRedisConnPool::GetInstance()->GetRedisContext();
	redisOpt_.SetRedisContext(con);

	switch (message_type)
	{
	case MT_HEARTBEAT:
		HeartBeat();
		break;
	case MT_GETDEVICEINFO:
		GetCloudSeeDeviceInfo();
		break;
	default:
		LOG4CXX_WARN(g_logger, "CLogicOperate::StartLogicOpt:message_type invalid.");
	}

	CRedisConnPool::GetInstance()->ReleaseRedisContext(con);
}


void CCloudSeeMessageOpt::HeartBeat()
{
	LOG4CXX_TRACE(g_logger, "start HeartBeat...");

	PACKET_HEAD packet_head;
	dataOpt_.PackHeartBeat(packet_head);

	if (!SocketOperate::WriteSfd(sfd_, (const char*)&packet_head, sizeof (packet_head)))
	{
		LOG4CXX_WARN(g_logger, "CCloudSeeMessageOpt::HeartBeat:sendto failed. error = " << strerror(errno));
	}
}

void CCloudSeeMessageOpt::GetCloudSeeDeviceInfo()
{
	LOG4CXX_TRACE(g_logger, "start GetCloudSeeDeviceInfo...");

	int message_id = 0;
	if (!dataOpt_.ParseMessageId(message_id))
	{
		LOG4CXX_ERROR(g_logger, "CCloudSeeMessageOpt::GetCloudSeeDeviceInfo:ParseMessageId error.");
		return;
	}

	sem_t* h_event = FindEventByMessageIdAndSetResponse(message_id, message_);
	if (h_event)
	{
		sem_post(h_event);
	}
	else
	{
		LOG4CXX_WARN(g_logger, "CCloudSeeMessageOpt::GetCloudSeeDeviceInfo other message or timeout message. message_id = "  
			<< message_id);
	}
}

sem_t* CCloudSeeMessageOpt::FindEventByMessageIdAndSetResponse(const int message_id, const char* response)
{
	NET_MSG newNetMsg;
	newNetMsg.h_event = NULL;

	memcpy(newNetMsg.response, response, DATA_BUFFER_SIZE);

	NET_MSG oldNetMsg;
	if (g_map_message.findAndSet(message_id, newNetMsg, oldNetMsg))
		return oldNetMsg.h_event;
	else
		return NULL;
}
