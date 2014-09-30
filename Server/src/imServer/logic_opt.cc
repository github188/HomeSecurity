/*
 * message_opt.cc
 *
 *  Created on: Mar 18, 2013
 *      Author: yaowei
 */

#include "logic_opt.h"
#include "defines.h"
#include "json_opt.h"
#include "redis_conn_pool.h"
#include "global_settings.h"
#include "local_transport.h"
#include "../public/utils.h"
#include "../public/redis_key.h"
#include "../public/socket_wrapper.h"

CThreadSafeMap<int, NET_MSG> g_map_message;

CLogicOpt::CLogicOpt(conn* c)
{
	conn_ = c;
	result_ = 0;
	redis_con_ = NULL;
	jsonOpt_ptr_ = new CJsonOpt;
	assert(jsonOpt_ptr_ != NULL);
	redisOpt_ptr_ = new CRedisOpt;
	assert(redisOpt_ptr_ != NULL);
}

CLogicOpt::~CLogicOpt()
{
	utils::SafeDelete(jsonOpt_ptr_);
	utils::SafeDelete(redisOpt_ptr_);
}

void CLogicOpt::StartLogicOpt(const std::string& message)
{
	int logic_type = 0;
	int message_type = 0;
	std::string session_id;
	if (!jsonOpt_ptr_->ParseMessageLogicType(message.c_str(), logic_type, message_type, session_id))
	{
		LOG4CXX_ERROR(g_logger, "CLogicOpt::StartLogicOpt:ParseMessageLogicType invalid :" << message);
		return;
	}

	LOG4CXX_TRACE(g_logger, "CLogicOpt::StartLogicOpt:message = " << message);

	result_ = SUCCESS;

	/* 进行session验证 */
	redis_con_ = CRedisConnPool::GetInstance()->GetRedisContext();
	redisOpt_ptr_->SetRedisContext(redis_con_);
	redisOpt_ptr_->SelectDB(SESSION);
	if (!redisOpt_ptr_->Get(session_id, username_))
		result_ = SESSION_NOT_EXSIT;

	strcpy(conn_->chUsername,username_.c_str());

	redisOpt_ptr_->SelectDB(STATUS);
	switch (message_type)
	{
	case USER_ONLINE:
		UserOnline();
		break;
	case GET_LIVE_STATUS:
		GetLiveStatus();
		break;
	case SET_ONLINE_STATUS:
		SetOnlineStatus();
		break;
	case PUSH_DEVICE_MODIFY_INFO:
		PushDeviceModifyInfo();
		break;
	case PUSH_DEVICE_UPDATE_CMD:
		PushDeviceUpdateCMD();
		break;
	case PUSH_DEVICE_CANCEL_CMD:
		PushDeviceCancelCMD();
		break;
	case GET_UPDATE_DOWNLOAD_STEP:
		GetUpdateDownloadStep();
		break;
	case GET_UPDATE_WRITE_STEP:
		GetUpdateWriteStep();
		break;
	case PUSH_DEVICE_REBOOT_CMD:
		PushDeviceRebootCMD();
		break;
	case PUSH_DEVICE_MODIFY_PASSWORD:
		PushDeviceModifyPassword();
		break;
	default:
		LOG4CXX_WARN(g_logger, "CLogicOpt::StartLogicOpt:messagetype cannot be identified.");
		break;
	}

	SendToClient();

	CRedisConnPool::GetInstance()->ReleaseRedisContext(redis_con_);
}

void CLogicOpt::SendToClient()
{
	responseToClient_.append(CRLF);
	if (!SocketOperate::WriteSfd(conn_->sfd, responseToClient_.c_str(), responseToClient_.length()))
	{
		LOG4CXX_WARN(g_logger, "CLogicOpt::SendToClient:sendto failed :" << username_ << "sendto = " << responseToClient_ << ",errorCode = " << strerror(errno));
	}

	LOG4CXX_TRACE(g_logger, "CLogicOpt::SendToClient:message = " << responseToClient_);
}

bool CLogicOpt::SendToRelayServer(const int relay_type)
{
	/* 在消息中加上源用户名 */
	std::string str_relay = jsonOpt_ptr_->RestructIm2RelayMessage(relay_type);

	LOG4CXX_TRACE(g_logger, "CMessageOpt::SendToRelayServer:message = " << str_relay);

	return CLocalTransport::GetInstance()->SendToRelay(str_relay);
}

void CLogicOpt::SendNotifyOfflineToRelayServer(std::string& message)
{
	CLocalTransport::GetInstance()->SendToRelay(message);
	LOG4CXX_TRACE(g_logger, "CLogicOpt::SendNotifyOfflineToRelayServer:message = " << message);
}

void CLogicOpt::UserOnline()
{
	bool flag = false;
	std::string s_online_status, s_online_server_no, str_sfd, str_sfd_id, im_relay;
	assert(conn_ != NULL);

	if (SessionNotExist())
	{
		goto GET_RESPONSE;
	}

	/* 如果此用户已经上线，需通知其上线端下线 */
	if(redisOpt_ptr_->Get(RedisKeyUserOnlineFlag(username_), s_online_status))
	{
		int online_status = atoi(s_online_status.c_str());
		if(online_status != OFFLINE)
		{
			/* 取出其上线服务器号 */
			if (!redisOpt_ptr_->Hget(RedisKeyUserOnlineServerInfo(username_), KEY_ONLINE_SERVER_NO, s_online_server_no))
			{
				LOG4CXX_ERROR(g_logger, "CLogicOpt::UserOnline redis KEY_ONLINE_SERVER_NO.");
				goto GET_RESPONSE;
			}

			/* 从缓存中取出相应连接的sfd和其对应id */
			if (!redisOpt_ptr_->Hget(RedisKeyUserOnlineServerInfo(username_), KEY_ONLINE_SERVER_FD, str_sfd))
			{
				LOG4CXX_ERROR(g_logger, "CLogicOpt::UserOnline redis KEY_ONLINE_SERVER_FD.");
				goto GET_RESPONSE;
			}
			if (!redisOpt_ptr_->Hget(RedisKeyUserOnlineServerInfo(username_), KEY_ONLINE_SERVER_FD_ID, str_sfd_id))
			{
				LOG4CXX_ERROR(g_logger, "CLogicOpt::UserOnline redis KEY_ONLINE_SERVER_FD_ID.");
				goto GET_RESPONSE;
			}

			/* 重组消息后发送给转发进程 */
			im_relay = jsonOpt_ptr_->JsonJoinNotifyOffline(username_, atoi(s_online_server_no.c_str()),
														   atoi(str_sfd.c_str()),
														   atoi(str_sfd_id.c_str()));

			SendNotifyOfflineToRelayServer(im_relay);
		}
	}

	/* 将此用户的上线状态及上线服务器信息写入缓存;
	 * 设置用户在线状态key的过期时间为客户端心跳超时时间，防止有错误在线情况 */
	flag = redisOpt_ptr_->Set(RedisKeyUserOnlineFlag(username_), ONLINE)
			&& redisOpt_ptr_->Hset(RedisKeyUserOnlineServerInfo(username_), KEY_ONLINE_SERVER_NO, utils::G<CGlobalSettings>().bind_port_)
			&& redisOpt_ptr_->Hset(RedisKeyUserOnlineServerInfo(username_), KEY_ONLINE_SERVER_FD, conn_->sfd)
			&& redisOpt_ptr_->Hset(RedisKeyUserOnlineServerInfo(username_), KEY_ONLINE_SERVER_FD_ID, conn_->id)
			&& redisOpt_ptr_->Expire(RedisKeyUserOnlineFlag(username_), utils::G<CGlobalSettings>().client_heartbeat_timeout_);

	if (!flag)
	{
		LOG4CXX_ERROR(g_logger, "CLogicOpt::UserOnline:redisOpt error.");
		result_ = REDIS_OPT_ERROR;
		goto GET_RESPONSE;
	}

GET_RESPONSE:
	responseToClient_ = jsonOpt_ptr_->JsonJoinCommon(USER_ONLINE_RESPONSE, result_);
}

void CLogicOpt::GetLiveStatus()
{
	if (SessionNotExist())
	{
		goto GET_RESPONSE;
	}

	if(!redisOpt_ptr_->Set(RedisKeyUserOnlineFlag(username_), ONLINE))
	{
		LOG4CXX_WARN(g_logger, "CLogicOpt::GetLiveStatus:Set ONLINE error. username = " << username_);
	}

	/* 心跳，更新用户在线状态key缓存时间 */
	if (!redisOpt_ptr_->Expire(RedisKeyUserOnlineFlag(username_), utils::G<CGlobalSettings>().client_heartbeat_timeout_))
	{
		LOG4CXX_WARN(g_logger, "CLogicOpt::GetLiveStatus:Expire error. username = " << username_);
	}

GET_RESPONSE:
	responseToClient_ = jsonOpt_ptr_->JsonJoinCommon(GET_LIVE_STATUS_RESPONSE, result_);
}

void CLogicOpt::SetOnlineStatus()
{
	int online_status = 0;

	if (SessionNotExist())
	{
		goto GET_RESPONSE;
	}

	if(!jsonOpt_ptr_->JsonParseSetOnlineStatus(online_status))
	{
		result_ = JSON_PARSE_ERROR;
		goto GET_RESPONSE;
	}

	/* 将此用户的状态写入缓存 */
	if(!redisOpt_ptr_->Set(RedisKeyUserOnlineFlag(username_), online_status))
	{
		result_ = REDIS_OPT_ERROR;
		goto GET_RESPONSE;
	}

GET_RESPONSE:
	responseToClient_ = jsonOpt_ptr_->JsonJoinCommon(SET_ONLINE_STATUS_RESPONSE, result_);
}

int CLogicOpt::GetDeviceOnlineStatus(const std::string& device_guid, int& online_flag)
{
	redisOpt_ptr_->SelectDB(DEVICE);

	online_flag = OFFLINE;
	std::string str_online_flag;

	if (!redisOpt_ptr_->Get(RedisKeyDeviceOnlineFlag(device_guid), str_online_flag))
	{
		LOG4CXX_ERROR(g_logger, "CLogicOpt::GetDeviceOnlineStatus:Get error.device_guid=" << device_guid);
		return REDIS_OPT_ERROR;
	}
	online_flag = atoi(str_online_flag.c_str());

	return SUCCESS;
}

void CLogicOpt::PushDeviceModifyInfo()
{
	int message_id;
	std::string device_guid;
	int device_online_flag = 0;
	std::string response;
	bool flag = true;

	if (SessionNotExist())
	{
		goto GET_RESPONSE;
	}

	flag = jsonOpt_ptr_->JsonParsePushC2DMessageId(username_, message_id, device_guid);
	if (!flag)
	{
		LOG4CXX_ERROR(g_logger, "CLogicOpt::PushDeviceModifyInfo. message is valid");
		result_ = JSON_PARSE_ERROR;
		goto GET_RESPONSE;
	}

	GetDeviceOnlineStatus(device_guid, device_online_flag);
	if (device_online_flag == 0)
	{
		LOG4CXX_ERROR(g_logger, "CLogicOpt::PushDeviceModifyInfo device is not online:" << device_guid);
		result_ = DEVICE_NOT_ONLINE;
		goto GET_RESPONSE;
	}

	// 向relay转发请求,进行处理 等待响应
	if ((result_ = GetResponseFromRelay(RELAY_DEVICE_MODIFY_INFO, message_id, response)) != SUCCESS)
	{
		LOG4CXX_ERROR(g_logger, "CLogicOpt::PushDeviceModifyInfo:GetResponseFromRelay failed.");
		goto GET_RESPONSE;
	}

	jsonOpt_ptr_->JsonParsePushDeviceModifyInfoResult(response, result_);


GET_RESPONSE:
	responseToClient_ = jsonOpt_ptr_->JsonJoinCommon(PUSH_DEVICE_MODIFY_INFO_RESPONSE, result_);
}

void CLogicOpt::maketimeout(struct timespec *tsp, long milliseconds)
{
	struct timeval now;

	/* get current times */
	gettimeofday(&now, NULL);
	tsp->tv_sec = now.tv_sec;
	tsp->tv_nsec = now.tv_usec * 1000;
	tsp->tv_sec += (milliseconds/1000);
	tsp->tv_nsec += (milliseconds%1000) * 1000000;
}

std::string CLogicOpt::FindResponseByMessageId(const int message_id)
{
	NET_MSG netMsg;
	g_map_message.find(message_id, netMsg);
	return netMsg.response;
}

int CLogicOpt::GetResponseFromRelay(const int p2rmt, const int message_id, std::string& response)
{
	int ret = SUCCESS;

	NET_MSG net_msg;
	net_msg.response = "";
	sem_t cond;
	ret = sem_init(&cond, 0, 0);
	if(0 != ret)
	{
		LOG4CXX_ERROR(g_logger, "CLogicOpt::GetResponseFromRelay:sem_init failed. errorcode = " << result_);
		return ret;
	}
	net_msg.h_event = &cond;
	g_map_message.insert(message_id, net_msg);

	// 向转发服务器发送信息
	if (!SendToRelayServer(p2rmt))
	{
		LOG4CXX_WARN(g_logger, "CLogicOpt::GetResponseFromRelay:SendToRelayServer failed.");
	}

	struct timespec timestruct = {0, 0};
	maketimeout(&timestruct, 5000);
	int dw = sem_timedwait(&cond, &timestruct);
	sem_destroy(&cond);
	switch(dw)
	{
	case 0:
		/** 回应 */
		response = FindResponseByMessageId(message_id);
		if (response.empty())
		{
			LOG4CXX_WARN(g_logger,"CLogicOpt::GetResponseFromRelay:FindResponseByMessageId empty. message_id = " << message_id);
		}
		break;
	case ETIMEDOUT:
		LOG4CXX_WARN(g_logger, "CLogicOpt::GetResponseFromRelay TIMEOUT."<< ", message_id = " << message_id);
		ret = REQ_RES_TIMEOUT;
		break;
	default:
		LOG4CXX_ERROR(g_logger, "CLogicOpt::GetResponseFromRelay error. errorcode = " << strerror(errno) << ", message_id = " << message_id);
		ret = REQ_RES_OTHER_ERROR;
		break;
	}
	g_map_message.erase(message_id);

	LOG4CXX_TRACE(g_logger, "********** response=" << response);

	return ret;
}

void CLogicOpt::PushDeviceUpdateCMD()
{
	int message_id;
	std::string device_guid;
	int device_online_flag = 0;
	std::string response;
	bool flag = true;

	if (SessionNotExist())
	{
		goto GET_RESPONSE;
	}

	flag = jsonOpt_ptr_->JsonParsePushC2DMessageId(username_, message_id, device_guid);
	if (!flag)
	{
		LOG4CXX_ERROR(g_logger, "CLogicOpt::PushDeviceUpdateCMD. message is valid");
		result_ = JSON_PARSE_ERROR;
		goto GET_RESPONSE;
	}

	GetDeviceOnlineStatus(device_guid, device_online_flag);
	if (device_online_flag == 0)
	{
		LOG4CXX_ERROR(g_logger, "CLogicOpt::PushDeviceUpdateCMD device is not online:" << device_guid);
		result_ = DEVICE_NOT_ONLINE;
		goto GET_RESPONSE;
	}

	// 向relay转发请求,进行处理 等待响应
	if ((result_ = GetResponseFromRelay(RELAY_DEVICE_UPDATE_CMD, message_id, response)) != SUCCESS)
	{
		LOG4CXX_ERROR(g_logger, "CLogicOpt::PushDeviceUpdateCMD:GetResponseFromRelay failed.");
		goto GET_RESPONSE;
	}

	jsonOpt_ptr_->JsonParsePushResult(response, result_);


GET_RESPONSE:
	responseToClient_ = jsonOpt_ptr_->JsonJoinCommon(PUSH_DEVICE_UPDATE_CMD_RESPONSE, result_);
}

void CLogicOpt::PushDeviceCancelCMD()
{
	int message_id;
	std::string device_guid;
	int device_online_flag = 0;
	std::string response;
	bool flag = true;

	if (SessionNotExist())
	{
		goto GET_RESPONSE;
	}

	flag = jsonOpt_ptr_->JsonParsePushC2DMessageId(username_, message_id, device_guid);
	if (!flag)
	{
		LOG4CXX_ERROR(g_logger, "CLogicOpt::PushDeviceCancelCMD. message is valid");
		result_ = JSON_PARSE_ERROR;
		goto GET_RESPONSE;
	}

	GetDeviceOnlineStatus(device_guid, device_online_flag);
	if (device_online_flag == 0)
	{
		LOG4CXX_ERROR(g_logger, "CLogicOpt::PushDeviceCancelCMD device is not online:" << device_guid);
		result_ = DEVICE_NOT_ONLINE;
		goto GET_RESPONSE;
	}

	// 向relay转发请求,进行处理 等待响应
	if ((result_ = GetResponseFromRelay(RELAY_DEVICE_CANCEL_CMD, message_id, response)) != SUCCESS)
	{
		LOG4CXX_ERROR(g_logger, "CLogicOpt::PushDeviceCancelCMD:GetResponseFromRelay failed.");
		goto GET_RESPONSE;
	}

	jsonOpt_ptr_->JsonParsePushResult(response, result_);


GET_RESPONSE:
	responseToClient_ = jsonOpt_ptr_->JsonJoinCommon(PUSH_DEVICE_CANCEL_CMD_RESPONSE, result_);
}

void CLogicOpt::GetUpdateDownloadStep()
{
	int message_id;
	std::string device_guid;
	int device_online_flag = 0;
	std::string response;
	int downloadstep = 0;
	int writestep = 0;
	bool flag = true;

	if (SessionNotExist())
	{
		goto GET_RESPONSE;
	}

	flag = jsonOpt_ptr_->JsonParsePushC2DMessageId(username_, message_id, device_guid);
	if (!flag)
	{
		LOG4CXX_ERROR(g_logger, "CLogicOpt::GetUpdateDownloadStep. message is valid");
		result_ = JSON_PARSE_ERROR;
		goto GET_RESPONSE;
	}

	GetDeviceOnlineStatus(device_guid, device_online_flag);
	if (device_online_flag == 0)
	{
		LOG4CXX_ERROR(g_logger, "CLogicOpt::GetUpdateDownloadStep device is not online:" << device_guid);
		result_ = DEVICE_NOT_ONLINE;
		goto GET_RESPONSE;
	}

	// 向relay转发请求,进行处理 等待响应
	if ((result_ = GetResponseFromRelay(RELAY_GET_DEVICE_UPDATE_STEP, message_id, response)) != SUCCESS)
	{
		LOG4CXX_ERROR(g_logger, "CLogicOpt::GetUpdateDownloadStep:GetResponseFromRelay failed.");
		goto GET_RESPONSE;
	}

	jsonOpt_ptr_->JsonParseGetDeviceUpdateStepResult(response, result_, downloadstep, writestep);


GET_RESPONSE:
	responseToClient_ = jsonOpt_ptr_->JsonJoinGetUpdateDownloadStep(result_, downloadstep);
}

void CLogicOpt::GetUpdateWriteStep()
{
	int message_id;
	std::string device_guid;
	int device_online_flag = 0;
	std::string response;
	int downloadstep = 0;
	int writestep = 0;
	bool flag = true;

	if (SessionNotExist())
	{
		goto GET_RESPONSE;
	}

	flag = jsonOpt_ptr_->JsonParsePushC2DMessageId(username_, message_id, device_guid);
	if (!flag)
	{
		LOG4CXX_ERROR(g_logger, "CLogicOpt::GetUpdateWriteStep. message is valid");
		result_ = JSON_PARSE_ERROR;
		goto GET_RESPONSE;
	}

	GetDeviceOnlineStatus(device_guid, device_online_flag);
	if (device_online_flag == 0)
	{
		LOG4CXX_ERROR(g_logger, "CLogicOpt::GetUpdateWriteStep device is not online:" << device_guid);
		result_ = DEVICE_NOT_ONLINE;
		goto GET_RESPONSE;
	}

	// 向relay转发请求,进行处理 等待响应
	if ((result_ = GetResponseFromRelay(RELAY_GET_DEVICE_UPDATE_STEP, message_id, response)) != SUCCESS)
	{
		LOG4CXX_ERROR(g_logger, "CLogicOpt::GetUpdateWriteStep:GetResponseFromRelay failed.");
		goto GET_RESPONSE;
	}

	jsonOpt_ptr_->JsonParseGetDeviceUpdateStepResult(response, result_, downloadstep, writestep);


GET_RESPONSE:
	responseToClient_ = jsonOpt_ptr_->JsonJoinGetUpdateWriteStep(result_, writestep);
}

void CLogicOpt::PushDeviceRebootCMD()
{
	int message_id;
	std::string device_guid;
	int device_online_flag = 0;
	std::string response;
	bool flag = true;

	if (SessionNotExist())
	{
		goto GET_RESPONSE;
	}

	flag = jsonOpt_ptr_->JsonParsePushC2DMessageId(username_, message_id, device_guid);
	if (!flag)
	{
		LOG4CXX_ERROR(g_logger, "CLogicOpt::PushDeviceRebootCMD. message is valid");
		result_ = JSON_PARSE_ERROR;
		goto GET_RESPONSE;
	}

	GetDeviceOnlineStatus(device_guid, device_online_flag);
	if (device_online_flag == 0)
	{
		LOG4CXX_ERROR(g_logger, "CLogicOpt::PushDeviceRebootCMD device is not online:" << device_guid);
		result_ = DEVICE_NOT_ONLINE;
		goto GET_RESPONSE;
	}

	// 向转发服务器发送信息
	if (!SendToRelayServer(RELAY_DEVICE_REBOOT_CMD))
	{
		LOG4CXX_WARN(g_logger, "CLogicOpt::PushDeviceRebootCMD:SendToRelayServer failed.");
		result_ = DEVICE_UPDATE_ERROR;
		goto GET_RESPONSE;
	}

GET_RESPONSE:
	responseToClient_ = jsonOpt_ptr_->JsonJoinCommon(PUSH_DEVICE_REBOOT_CMD_RESPONSE, result_);
}

void CLogicOpt::PushDeviceModifyPassword()
{
	int message_id;
	std::string device_guid;
	int device_online_flag = 0;
	std::string response;
	bool flag = true;

	if (SessionNotExist())
	{
		goto GET_RESPONSE;
	}

	flag = jsonOpt_ptr_->JsonParsePushC2DMessageId(username_, message_id, device_guid);
	if (!flag)
	{
		LOG4CXX_ERROR(g_logger, "CLogicOpt::PushDeviceModifyPassword. message is valid");
		result_ = JSON_PARSE_ERROR;
		goto GET_RESPONSE;
	}

	GetDeviceOnlineStatus(device_guid, device_online_flag);
	if (device_online_flag == 0)
	{
		LOG4CXX_ERROR(g_logger, "CLogicOpt::PushDeviceModifyPassword device is not online:" << device_guid);
		result_ = DEVICE_NOT_ONLINE;
		goto GET_RESPONSE;
	}

	// 向relay转发请求,进行处理 等待响应
	if ((result_ = GetResponseFromRelay(RELAY_DEVICE_MODIFY_PASSWORD, message_id, response)) != SUCCESS)
	{
		LOG4CXX_ERROR(g_logger, "CLogicOpt::PushDeviceModifyPassword:GetResponseFromRelay failed.");
		goto GET_RESPONSE;
	}

	jsonOpt_ptr_->JsonParsePushResult(response, result_);


GET_RESPONSE:
	responseToClient_ = jsonOpt_ptr_->JsonJoinCommon(PUSH_DEVICE_MODIFY_PASSWORD_RESPONSE, result_);
}
