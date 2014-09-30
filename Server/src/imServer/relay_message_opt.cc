/*
 * relay_message_opt.cc
 *
 *  Created on: Mar 22, 2013
 *      Author: yaowei
 */

#include "relay_message_opt.h"
#include "redis_conn_pool.h"
#include "redis_opt.h"
#include "master_thread.h"
#include "worker_threads.h"
#include "global_settings.h"
#include "../public/message.h"
#include "../public/socket_wrapper.h"
#include "../public/utils.h"

extern CThreadSafeMap<int, NET_MSG> g_map_message;


CRelayMessageOpt::CRelayMessageOpt(const int sfd)
{
	sfd_ =sfd;
}

CRelayMessageOpt::~CRelayMessageOpt()
{

}

void CRelayMessageOpt::StartRelayMessageOpt(std::string& message)
{

	/* 判断消息类型*/
	int im2relay_type = 0;
	if(!jsonOpt_.JsonParseR2PMessageType(message, im2relay_type))
	{
		LOG4CXX_ERROR(g_logger, "CRelayMessageOpt::StartRelayMessageOpt:JsonParseMessageType json string invalid:" << message);
		return;
	}

	message_ = message;
	redisContext* con = CRedisConnPool::GetInstance()->GetRedisContext();
	redisOpt_.SetRedisContext(con);

	LOG4CXX_TRACE(g_logger, "CRelayMessageOpt::StartRelayMessageOpt: message = " << message);

	switch(im2relay_type)
	{
	case RELAY_NOTIFY_OFFLINE:
		OptReOnlineProblem();
		break;
	case SEND_MESSAGE_TO_USER:
		SendMessageToUser();
		break;
	case RELAY_DEVICE_MODIFY_RESULT:
		OptRelayDeviceModifyResult();
		break;
	case RELAY_DEVICE_UPDATE_CMD_RESULT:
		OptRelayDeviceUpdateCMDResult();
		break;
	case RELAY_DEVICE_CANCEL_CMD_RESULT:
		OptRelayDeviceCancelCMDResult();
		break;
	case RELAY_GET_DEVICE_UPDATE_STEP_RESULT:
		OptRelayGetDeviceUpdateStepResult();
		break;
	case RELAY_DEVICE_MODIFY_PASSWORD_RESULT:
		OptRelayDeviceModifyPasswordResult();
		break;
	case RELAY_ALARM_MESSAGE:
		OptRelayAlarmMessageToUser();
		break;
	default:
		break;
	}

	CRedisConnPool::GetInstance()->ReleaseRedisContext(con);
}

bool CRelayMessageOpt::VerifyUserSfd(const std::string& peer_name, int& sfd)
{
	if(!redisOpt_.SelectDB(STATUS))
	{
		LOG4CXX_ERROR(g_logger, "CRelayMessageOpt::VerifyUserSfd SelectDB.");
		return false;
	}
	/* 1.从缓存中取出此用户相应连接的sfd和其对应id */
	std::string str_sfd, str_sfd_id;
	int	sfd_id = 0;
	if (!redisOpt_.Hget(RedisKeyUserOnlineServerInfo(peer_name), KEY_ONLINE_SERVER_FD, str_sfd))
	{
		LOG4CXX_ERROR(g_logger, "CRelayMessageOpt::VerifyUserSfd KEY_ONLINE_SERVER_FD.");
		return false;
	}
	if (!redisOpt_.Hget(RedisKeyUserOnlineServerInfo(peer_name), KEY_ONLINE_SERVER_FD_ID, str_sfd_id))
	{
		LOG4CXX_ERROR(g_logger, "CRelayMessageOpt::VerifyUserSfd KEY_ONLINE_SERVER_FD_ID.");
		return false;
	}

	/* 2.在map中进行校验看客户端连接是否已经失效 */
	sfd = atoi(str_sfd.c_str());
	sfd_id = atoi(str_sfd_id.c_str());
	if (!CMasterThread::map_csfd_id_.findValueByKey(sfd, sfd_id))
	{
		LOG4CXX_TRACE(g_logger, "CRelayMessageOpt::OptCommon map_csfd_id_ has invalid. username = " << peer_name);
		return false;
	}

	return true;
}

bool CRelayMessageOpt::SendToUser()
{
	std::string username;
	if(!jsonOpt_.JsonParseUsername(username))
	{
		LOG4CXX_WARN(g_logger, "CRelayMessageOpt::SendToUser:SendToUser error. message = " << message_);
		return false;
	}

	int sfd = 0;
	if(!VerifyUserSfd(username, sfd))
		return false;

	/* 去除message_id */
	std::string message_to_user = jsonOpt_.RestructMessageToUser(message_);
	message_to_user.append(CRLF);
	LOG4CXX_TRACE(g_logger, "CRelayMessageOpt::SendToUser:message = " << message_to_user);

	if(!SocketOperate::WriteSfd(sfd, message_to_user.c_str(), message_to_user.length()))
	{
		LOG4CXX_WARN(g_logger, "CRelayMessageOpt::SendToUser WriteSfd error." << strerror(errno)
								<< ", message = " << message_to_user);
		return false;
	}

	return true;
}

void CRelayMessageOpt::OptReOnlineProblem()
{
	int server_no, sfd, sfd_id;
	if (jsonOpt_.JsonParseNotifyOffline(server_no, sfd, sfd_id))
	{
		if (server_no == utils::G<CGlobalSettings>().bind_port_)
		{
			if (CMasterThread::map_csfd_id_.findValueByKey(sfd, sfd_id))
			{
				message_.append(CRLF);
				SocketOperate::WriteSfd(sfd, message_.c_str(), message_.length());
				LOG4CXX_TRACE(g_logger, "CRelayMessageOpt::OptReOnlineProblem:message = " << message_);
			}
		}
	}
}

void CRelayMessageOpt::SendMessageToUser()
{
	SendToUser();
}

void CRelayMessageOpt::OptRelayDeviceModifyResult()
{
	/* 1.解析出目的用户名 */
	std::string username;
	if (!jsonOpt_.JsonParseRelay2ImMessage(message_, username))
	{
		LOG4CXX_WARN(g_logger, "CRelayMessageOpt::OptRelayDeviceModifyResult json string invalid:" << message_)
			return;
	}

	/* 2.验证此设备的长连接是否还有效，如果无效将消息存入redis消息队列 */
	int sfd = 0;
	if(!VerifyUserSfd(username, sfd))
	{
		return;
	}

	
	std::string client_message;
	client_message = jsonOpt_.RestructDeviceModifyResult2Client(message_);

	LOG4CXX_TRACE(g_logger, "********** client_message=" << client_message);

	///* 3.将转发信息发送至客户端，如果发送失败将消息存入redis消息队列 */
	//if(!SendToClient(sfd, client_message))
	//{
	//	return;
	//}

	/** 解析消息ID */
	int message_id = 0;
	if (!jsonOpt_.JsonParseMessageId(client_message, message_id))
	{
		LOG4CXX_WARN(g_logger, "CRelayMessageOpt::OptRelayDeviceModifyResult invalid, message = " << client_message);
		return;
	}

	/** 根据传回的请求消息id找到相应的请求阻塞事件，激活事件读取response */
	sem_t* h_event = FindEventByMessageIdAndSetResponse(message_id, client_message);
	if (h_event)
	{
		sem_post(h_event);
	}
	else
	{
		LOG4CXX_WARN(g_logger, "CRelayMessageOpt::OptRelayDeviceModifyResult other message or timeout message. message_id = "  
			<< message_id << ", response = " << client_message);
	}
}

bool CRelayMessageOpt::SendToClient(const int sfd, std::string& message)
{
	if (message.empty())
	{
		LOG4CXX_ERROR(g_logger, "CRelayMessageOpt::SendToClient message empty");
		return false;
	}
	message.append(CRLF);
	LOG4CXX_TRACE(g_logger, "CRelayMessageOpt::SendToClient im2client. message = " << message);

	if(!SocketOperate::WriteSfd(sfd, message.c_str(), message.length()))
	{
		LOG4CXX_WARN(g_logger, "CRelayMessageOpt::SendToClient WriteSfd error." << strerror(errno))
			return false;
	}

	return true;
}

sem_t* CRelayMessageOpt::FindEventByMessageIdAndSetResponse(const int message_id, const std::string& response)
{
	NET_MSG newNetMsg;
	newNetMsg.h_event = NULL;
	newNetMsg.response = response;
	NET_MSG oldNetMsg;
	if (g_map_message.findAndSet(message_id, newNetMsg, oldNetMsg))
		return oldNetMsg.h_event;
	else
		return NULL;
}

void CRelayMessageOpt::OptRelayDeviceUpdateCMDResult()
{
	/* 1.解析出目的用户名 */
	std::string username;
	if (!jsonOpt_.JsonParseRelay2ImMessage(message_, username))
	{
		LOG4CXX_WARN(g_logger, "CRelayMessageOpt::OptRelayDeviceUpdateCMDResult json string invalid:" << message_)
			return;
	}

	/* 2.验证此设备的长连接是否还有效，如果无效将消息存入redis消息队列 */
	int sfd = 0;
	if(!VerifyUserSfd(username, sfd))
	{
		return;
	}

	std::string client_message;
	client_message = jsonOpt_.RestructPush2CCommon(message_);

	LOG4CXX_TRACE(g_logger, "********** client_message=" << client_message);

	///* 3.将转发信息发送至客户端，如果发送失败将消息存入redis消息队列 */
	//if(!SendToClient(sfd, client_message))
	//{
	//	return;
	//}

	/** 解析消息ID */
	int message_id = 0;
	if (!jsonOpt_.JsonParseMessageId(client_message, message_id))
	{
		LOG4CXX_WARN(g_logger, "CRelayMessageOpt::OptRelayDeviceUpdateCMDResult invalid, message = " << client_message);
		return;
	}

	/** 根据传回的请求消息id找到相应的请求阻塞事件，激活事件读取response */
	sem_t* h_event = FindEventByMessageIdAndSetResponse(message_id, client_message);
	if (h_event)
	{
		sem_post(h_event);
	}
	else
	{
		LOG4CXX_WARN(g_logger, "CRelayMessageOpt::OptRelayDeviceUpdateCMDResult other message or timeout message. message_id = "  
			<< message_id << ", response = " << client_message);
	}
}

void CRelayMessageOpt::OptRelayDeviceCancelCMDResult()
{
	/* 1.解析出目的用户名 */
	std::string username;
	if (!jsonOpt_.JsonParseRelay2ImMessage(message_, username))
	{
		LOG4CXX_WARN(g_logger, "CRelayMessageOpt::OptRelayDeviceCancelCMDResult json string invalid:" << message_);
		return;
	}

	/* 2.验证此设备的长连接是否还有效，如果无效将消息存入redis消息队列 */
	int sfd = 0;
	if(!VerifyUserSfd(username, sfd))
	{
		return;
	}

	std::string client_message;
	client_message = jsonOpt_.RestructPush2CCommon(message_);

	LOG4CXX_TRACE(g_logger, "********** client_message=" << client_message);

	///* 3.将转发信息发送至客户端，如果发送失败将消息存入redis消息队列 */
	//if(!SendToClient(sfd, client_message))
	//{
	//	return;
	//}

	/** 解析消息ID */
	int message_id = 0;
	if (!jsonOpt_.JsonParseMessageId(client_message, message_id))
	{
		LOG4CXX_WARN(g_logger, "CRelayMessageOpt::OptRelayDeviceCancelCMDResult invalid, message = " << client_message);
		return;
	}

	/** 根据传回的请求消息id找到相应的请求阻塞事件，激活事件读取response */
	sem_t* h_event = FindEventByMessageIdAndSetResponse(message_id, client_message);
	if (h_event)
	{
		sem_post(h_event);
	}
	else
	{
		LOG4CXX_WARN(g_logger, "CRelayMessageOpt::OptRelayDeviceCancelCMDResult other message or timeout message. message_id = "  
			<< message_id << ", response = " << client_message);
	}
}

void CRelayMessageOpt::OptRelayGetDeviceUpdateStepResult()
{
	/* 1.解析出目的用户名 */
	std::string username;
	if (!jsonOpt_.JsonParseRelay2ImMessage(message_, username))
	{
		LOG4CXX_WARN(g_logger, "CRelayMessageOpt::OptRelayGetDeviceUpdateStepResult json string invalid:" << message_);
		return;
	}

	/* 2.验证此设备的长连接是否还有效，如果无效将消息存入redis消息队列 */
	int sfd = 0;
	if(!VerifyUserSfd(username, sfd))
	{
		return;
	}

	std::string client_message;
	client_message = jsonOpt_.RestructDeviceUpdateStepResult2Client(message_);

	LOG4CXX_TRACE(g_logger, "********** client_message=" << client_message);

	///* 3.将转发信息发送至客户端，如果发送失败将消息存入redis消息队列 */
	//if(!SendToClient(sfd, client_message))
	//{
	//	return;
	//}

	/** 解析消息ID */
	int message_id = 0;
	if (!jsonOpt_.JsonParseMessageId(client_message, message_id))
	{
		LOG4CXX_WARN(g_logger, "CRelayMessageOpt::OptRelayGetDeviceUpdateStepResult invalid, message = " << client_message);
		return;
	}

	/** 根据传回的请求消息id找到相应的请求阻塞事件，激活事件读取response */
	sem_t* h_event = FindEventByMessageIdAndSetResponse(message_id, client_message);
	if (h_event)
	{
		sem_post(h_event);
	}
	else
	{
		LOG4CXX_WARN(g_logger, "CRelayMessageOpt::OptRelayGetDeviceUpdateStepResult other message or timeout message. message_id = "  
			<< message_id << ", response = " << client_message);
	}
}

void CRelayMessageOpt::OptRelayDeviceModifyPasswordResult()
{
	/* 1.解析出目的用户名 */
	std::string username;
	if (!jsonOpt_.JsonParseRelay2ImMessage(message_, username))
	{
		LOG4CXX_WARN(g_logger, "CRelayMessageOpt::OptRelayDeviceModifyPasswordResult json string invalid:" << message_);
		return;
	}

	/* 2.验证此设备的长连接是否还有效，如果无效将消息存入redis消息队列 */
	int sfd = 0;
	if(!VerifyUserSfd(username, sfd))
	{
		return;
	}

	std::string client_message;
	client_message = jsonOpt_.RestructPush2CCommon(message_);

	LOG4CXX_TRACE(g_logger, "********** client_message=" << client_message);

	///* 3.将转发信息发送至客户端，如果发送失败将消息存入redis消息队列 */
	//if(!SendToClient(sfd, client_message))
	//{
	//	return;
	//}

	/** 解析消息ID */
	int message_id = 0;
	if (!jsonOpt_.JsonParseMessageId(client_message, message_id))
	{
		LOG4CXX_WARN(g_logger, "CRelayMessageOpt::OptRelayDeviceModifyPasswordResult invalid, message = " << client_message);
		return;
	}

	/** 根据传回的请求消息id找到相应的请求阻塞事件，激活事件读取response */
	sem_t* h_event = FindEventByMessageIdAndSetResponse(message_id, client_message);
	if (h_event)
	{
		sem_post(h_event);
	}
	else
	{
		LOG4CXX_WARN(g_logger, "CRelayMessageOpt::OptRelayDeviceModifyPasswordResult other message or timeout message. message_id = "  
			<< message_id << ", response = " << client_message);
	}
}

void CRelayMessageOpt::OptRelayAlarmMessageToUser()
{
	message_ = jsonOpt_.RestructAlarmMessageToUser(message_);
	SendToUser();
}
