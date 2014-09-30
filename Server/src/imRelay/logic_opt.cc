/*
 * message_opt.cc
 *
 *  Created on: Mar 22, 2013
 *      Author: yaowei
 */

#include "logic_opt.h"
#include "redis_conn_pool.h"
#include "local_transport.h"
#include "../public/message.h"
#include "../public/socket_wrapper.h"
#include "../public/utils.h"
#include "../public/user_interface_defines.h"
#include "../public/redis_key.h"

CLogicOpt::CLogicOpt()
{
	current_sfd_= 0;
	result_		= SUCCESS;
}

CLogicOpt::~CLogicOpt()
{
}

void CLogicOpt::StartMessageOpt(const std::string& message, const int sfd)
{
	current_sfd_ = sfd;
	message_ = message;
	LOG4CXX_TRACE(g_logger, "CMessageOpt::StartMessageOpt:message = " << message_);

	/* 判断消息代表的逻辑处理进程类型 */
	int logic_type = 0;
	if (!jsonOpt_.JsonParseLogicType(message, logic_type))
	{
		LOG4CXX_WARN(g_logger,
				"CMessageOpt::StartMessageOpt:message json invalid = " << message_);
		return;
	}

	int p2relay_messagetype = 0;
	if (!jsonOpt_.JsonParseP2RelayMessageType(p2relay_messagetype))
	{
		LOG4CXX_WARN(g_logger,
				"CMessageOpt::StartMessageOpt:JsonParseP2RelayMessageType json invalid = " << message_);
		return;
	}

	/** 每一个连接服务的公共处理逻辑 **/
	switch (p2relay_messagetype)
	{
	case HEATBEAT_DETECT:
		HeatbeatDetect();
		break;
	default:
		break;
	}

	redisContext* con = CRedisConnPool::GetInstance()->GetRedisContext();
	redisOpt_.SetRedisContext(con);

	/* 上线服务器到转发服务器消息逻辑 */
	if(IM_SERVER_RELAY == logic_type)
	{
		switch (p2relay_messagetype)
		{
		case RELAY_NOTIFY_OFFLINE:
			Im2RelayNotifyOffline();
			break;
		case RELAY_DEVICE_MODIFY_INFO:
			Im2RelayDeviceModifyInfo();
			break;
		case RELAY_DEVICE_MODIFY_RESULT:
			Im2RelayDeviceModifyResult();
			break;
		case RELAY_DEVICE_UPDATE_CMD:
			Im2RelayDeviceUpdateCMD();
			break;
		case RELAY_DEVICE_UPDATE_CMD_RESULT:
			Im2RelayDeviceUpdateCMDResult();
			break;
		case RELAY_DEVICE_CANCEL_CMD:
			Im2RelayDeviceCancelCMD();
			break;
		case RELAY_DEVICE_CANCEL_CMD_RESULT:
			Im2RelayDeviceCancelCMDResult();
			break;
		case RELAY_GET_DEVICE_UPDATE_STEP:
			Im2RelayGetDeviceUpdateStep();
			break;
		case RELAY_GET_DEVICE_UPDATE_STEP_RESULT:
			Im2RelayGetDeviceUpdateStepResult();
			break;
		case RELAY_DEVICE_REBOOT_CMD:
			Im2RelayDeviceRebootCMD();
			break;
		case RELAY_DEVICE_MODIFY_PASSWORD:
			Im2RelayDeviceModifyPassword();
			break;
		case RELAY_DEVICE_MODIFY_PASSWORD_RESULT:
			Im2RelayDeviceModifyPasswordResult();
			break;
		default:
			break;
		}
	}
	else if (IM_DEV_DIRECT == logic_type)
	{
		switch (p2relay_messagetype)
		{
		case RELAY_ALARM_MESSAGE:
			ImDev2RelayAlarmMessage();
			break;
		default:
			break;
		}
	}
	/* 告警服务器到转发服务器消息逻辑 */
	else if(ALARM_SERVER_RELAY == logic_type)
	{
		switch (p2relay_messagetype)
		{
		case GET_ACCOUNT_LIVE_INFO:
			GetAccountLiveInfo();
			break;
		case SEND_MESSAGE_TO_USER:
			SendMessageToUser();
			break;
		default:
			break;
		}
	}

	CRedisConnPool::GetInstance()->ReleaseRedisContext(con);
}

int CLogicOpt::SendToImServer(const std::string& username, std::string& message)
{
	redisOpt_.SelectDB(STATUS);
	std::string s_online_status, s_online_server_no, str_sfd, str_sfd_id, to_im_message;
	int online_status = OFFLINE;
	int ret = SUCCESS;

	/* 先判断用户是否上线 */
	if (!redisOpt_.Get(RedisKeyUserOnlineFlag(username), s_online_status))
	{
		LOG4CXX_ERROR(g_logger, "CLogicOpt::SendToImServer Get RedisKeyUserOnlineFlag error.");
		ret = REDIS_OPT_ERROR;
		goto GET_RESPONSE;
	}

	online_status = atoi(s_online_status.c_str());
	if (OFFLINE == online_status)
	{
		LOG4CXX_WARN(g_logger, "CLogicOpt::SendToImServer user not online.");
		ret = OFFLINE;
		goto GET_RESPONSE;
	}

	/** 取出其上线服务器号 **/
	if (!redisOpt_.Hget(RedisKeyUserOnlineServerInfo(username), KEY_ONLINE_SERVER_NO, s_online_server_no))
	{
		LOG4CXX_ERROR(g_logger, "CLogicOpt::SendToImServer Hget RedisKeyUserOnlineServerInfo s_online_server_no error.");
		ret = REDIS_OPT_ERROR;
		goto GET_RESPONSE;
	}

	/** 从缓存中取出相应连接的sfd和其对应id **/
	if (!redisOpt_.Hget(RedisKeyUserOnlineServerInfo(username), KEY_ONLINE_SERVER_FD, str_sfd))
	{
		LOG4CXX_ERROR(g_logger, "CLogicOpt::SendToImServer Hget RedisKeyUserOnlineServerInfo str_sfd error.");
		ret = REDIS_OPT_ERROR;
		goto GET_RESPONSE;
	}
	if (!redisOpt_.Hget(RedisKeyUserOnlineServerInfo(username), KEY_ONLINE_SERVER_FD_ID, str_sfd_id))
	{
		LOG4CXX_ERROR(g_logger, "CLogicOpt::SendToImServer Hget RedisKeyUserOnlineServerInfo str_sfd_id error.");
		ret = REDIS_OPT_ERROR;
		goto GET_RESPONSE;
	}

	/* 重组消息后发送给制定的用户上线服务器 */
	to_im_message = jsonOpt_.RestructToImServerMessage(atoi(s_online_server_no.c_str()),
													   atoi(str_sfd.c_str()),
													   atoi(str_sfd_id.c_str()));

	if(!SendToSpecifyImServer(atoi(s_online_server_no.c_str()), to_im_message))
	{
		LOG4CXX_ERROR(g_logger, "CLogicOpt::SendToImServer SendToSpecifyImServer error.");
		ret = OTHER_ERROR;
	}

GET_RESPONSE:
	return ret;
}

int	CLogicOpt::SendToImDevServer(const std::string& device_guid, std::string& message)
{
	redisOpt_.SelectDB(DEVICE);

	std::string s_online_status, s_online_server_no, str_sfd, str_sfd_id, to_im_message;
	int online_status = OFFLINE;
	int ret = SUCCESS;

	/* 先判断设备是否上线 */
	if (!redisOpt_.Get(RedisKeyDeviceOnlineFlag(device_guid), s_online_status))
	{
		ret = REDIS_OPT_ERROR;
		goto GET_RESPONSE;
	}

	online_status = atoi(s_online_status.c_str());
	if (OFFLINE == online_status)
	{
		ret = OFFLINE;
		goto GET_RESPONSE;
	}

	/** 取出其上线服务器号 **/
	if (!redisOpt_.Hget(RedisKeyDeviceOnlineServerInfo(device_guid), KEY_ONLINE_SERVER_NO, s_online_server_no))
	{
		ret = REDIS_OPT_ERROR;
		goto GET_RESPONSE;
	}

	/** 从缓存中取出相应连接的sfd和其对应id **/
	if (!redisOpt_.Hget(RedisKeyDeviceOnlineServerInfo(device_guid), KEY_ONLINE_SERVER_FD, str_sfd))
	{
		ret = REDIS_OPT_ERROR;
		goto GET_RESPONSE;
	}
	if (!redisOpt_.Hget(RedisKeyDeviceOnlineServerInfo(device_guid), KEY_ONLINE_SERVER_FD_ID, str_sfd_id))
	{
		ret = REDIS_OPT_ERROR;
		goto GET_RESPONSE;
	}

	/* 重组消息后发送给制定的用户上线服务器 */
	to_im_message = jsonOpt_.RestructToImServerMessage(atoi(s_online_server_no.c_str()),
		atoi(str_sfd.c_str()),
		atoi(str_sfd_id.c_str()));

	if(!SendToSpecifyImServer(atoi(s_online_server_no.c_str()), to_im_message))
	{
		ret = OTHER_ERROR;
	}

GET_RESPONSE:
	return ret;
}

bool CLogicOpt::SendToSpecifyImServer(const int im_server_no, std::string& message)
{
	std::map<int, int>::const_iterator iter;
	message.append(CRLF);
	if (message.length() >= DATA_BUFFER_SIZE)
	{
		LOG4CXX_WARN(g_logger, "CLogicOpt::SendToSpecifyImServer data too large. data = " << message);
		return false;
	}
	if((iter = CLocalTransport::map_no_sfd_.find(im_server_no)) != CLocalTransport::map_no_sfd_.end())
	{

		if(!SocketOperate::WriteSfd(iter->second, message.c_str(), message.length()))
		{
			LOG4CXX_WARN(g_logger, "CMessageOpt::SendToSpecifyImServer:error fd = " << (iter->second) << ", errormsg = " << strerror(errno));
			return false;
		}

		LOG4CXX_TRACE(g_logger, "CMessageOpt::SendToSpecifyImServer:message = " << message << ", sfd = " << (iter->second));

		return true;
	}
	
	LOG4CXX_ERROR(g_logger, "CLogicOpt::SendToSpecifyImServer map_no_sfd_ not find.message = " << message << ", im_server_no = " << im_server_no);
	return false;
}

void CLogicOpt::SendToPeer(std::string& message)
{
	message = message.append(CRLF);

	if (!SocketOperate::WriteSfd(current_sfd_, message.c_str(), message.length()))
	{
		LOG4CXX_WARN(g_logger,
				"CLogicOpt::SendToPeer:sendto failed. send = " << message << ",errorCode = " << strerror(errno));
	}

	LOG4CXX_TRACE(g_logger, "CLogicOpt::SendToPeer:send = " << message);
}

void CLogicOpt::HeatbeatDetect()
{
	responseToPeer_ = jsonOpt_.JsonJoinHeatbeatDetectResponse(SUCCESS);
	SendToPeer(responseToPeer_);
}

void CLogicOpt::Im2RelayNotifyOffline()
{
	int server_no = 0;
	if (!jsonOpt_.JsonParseUserServerNo(server_no))
	{
		LOG4CXX_WARN(g_logger, "CMessageOpt::Im2RelayNotifyOffline:json invalid = " << message_);
		return;
	}

	SendToSpecifyImServer(server_no, message_);
}

void CLogicOpt::GetAccountLiveInfo()
{
	ACCOUNT_LIVE_INFO accountLiveInfo;
	std::string s_online_status, s_login_platform, s_mobile_id, s_language_type, s_alarm_flag;
	if(!jsonOpt_.JsonParseUsername(accountLiveInfo.username))
	{
		result_ = JSON_PARSE_ERROR;
		goto GET_RESPONSE;
	}

	redisOpt_.SelectDB(STATUS);

	if(!redisOpt_.Get(RedisKeyUserOnlineFlag(accountLiveInfo.username), s_online_status))
	{
		s_online_status = "0";
	}

	accountLiveInfo.online_status = atoi(s_online_status.c_str());

	if (!redisOpt_.Hget(RedisKeyClientLoginInfo(accountLiveInfo.username), KEY_LOGIN_PLATFORM, s_login_platform))
	{

		result_ = REDIS_OPT_ERROR;
		goto GET_RESPONSE;
	}
	accountLiveInfo.login_platform = atoi(s_login_platform.c_str());

	if (!redisOpt_.Hget(RedisKeyClientLoginInfo(accountLiveInfo.username), KEY_LANGUAGE_TYPE, s_language_type))
	{

		result_ = REDIS_OPT_ERROR;
		goto GET_RESPONSE;
	}
	accountLiveInfo.language_type = atoi(s_language_type.c_str());

	if (!redisOpt_.Hget(RedisKeyClientLoginInfo(accountLiveInfo.username),KEY_ALARM_FLAG, s_alarm_flag))
	{

		result_ = REDIS_OPT_ERROR;
		goto GET_RESPONSE;
	}
	accountLiveInfo.alarm_flag = atoi(s_alarm_flag.c_str());

	if (!redisOpt_.Hget(RedisKeyClientLoginInfo(accountLiveInfo.username), KEY_MOBILE_ID, s_mobile_id))
	{

		result_ = REDIS_OPT_ERROR;
		goto GET_RESPONSE;
	}
	accountLiveInfo.moblie_id = s_mobile_id;

GET_RESPONSE:
	responseToPeer_ = jsonOpt_.JsonJoinGetAccountLiveInfo(result_, accountLiveInfo);
	SendToPeer(responseToPeer_);
}

void CLogicOpt::Im2RelayDeviceModifyInfo()
{
	std::string device_guid;
	if (!jsonOpt_.JsonParseIm2RelayDeviceGUID(device_guid))
	{
		LOG4CXX_WARN(g_logger, "CMessageOpt::Im2RelayDeviceModifyInfo:json invalid = " << message_);
		return;
	}

	if (!SendToImDevServer(device_guid, message_))
	{
		return;
	}
}

void CLogicOpt::SendMessageToUser()
{
	std::string username;
	if(!jsonOpt_.JsonParseUsername(username))
	{
		result_ = JSON_PARSE_ERROR;
		goto GET_RESPONSE;
	}

	/* 将消息转发给上线服务器 */
	result_ = SendToImServer(username, message_);
	if(SUCCESS != result_)
	{
		LOG4CXX_WARN(g_logger, "CLogicOpt::SendMessageToUser failed. ret = " << result_);
	}

GET_RESPONSE:
	responseToPeer_ = jsonOpt_.JsonJoinSendMessageToUser(result_);
	SendToPeer(responseToPeer_);
}

void CLogicOpt::Im2RelayDeviceModifyResult()
{
	LOG4CXX_TRACE(g_logger, "CLogicOpt::Im2RelayDeviceModifyResult:json = " << message_);

	std::string username;
	if (!jsonOpt_.JsonParseUsername(username))
	{
		LOG4CXX_WARN(g_logger, "CLogicOpt::Im2RelayDeviceModifyResult:json invalid = " << message_);
		return;
	}

	if (!SendToImServer(username, message_))
	{
		return;
	}
}

void CLogicOpt::Im2RelayDeviceUpdateCMD()
{
	std::string device_guid;
	if (!jsonOpt_.JsonParseIm2RelayDeviceGUID(device_guid))
	{
		LOG4CXX_WARN(g_logger, "CLogicOpt::Im2RelayDeviceUpdateCMD:json invalid = " << message_);
		return;
	}

	if (!SendToImDevServer(device_guid, message_))
	{
		return;
	}
}

void CLogicOpt::Im2RelayDeviceUpdateCMDResult()
{
	LOG4CXX_TRACE(g_logger, "CLogicOpt::Im2RelayDeviceUpdateCMDResult:json = " << message_);

	std::string username;
	if (!jsonOpt_.JsonParseUsername(username))
	{
		LOG4CXX_WARN(g_logger, "CLogicOpt::Im2RelayDeviceModifyResult:json invalid = " << message_);
		return;
	}

	if (!SendToImServer(username, message_))
	{
		return;
	}
}

void CLogicOpt::Im2RelayDeviceCancelCMD()
{
	std::string device_guid;
	if (!jsonOpt_.JsonParseIm2RelayDeviceGUID(device_guid))
	{
		LOG4CXX_WARN(g_logger, "CLogicOpt::Im2RelayDeviceCancelCMD:json invalid = " << message_);
		return;
	}

	if (!SendToImDevServer(device_guid, message_))
	{
		return;
	}
}

void CLogicOpt::Im2RelayDeviceCancelCMDResult()
{
	LOG4CXX_TRACE(g_logger, "CLogicOpt::Im2RelayDeviceCancelCMDResult:json = " << message_);

	std::string username;
	if (!jsonOpt_.JsonParseUsername(username))
	{
		LOG4CXX_WARN(g_logger, "CLogicOpt::Im2RelayDeviceCancelCMDResult:json invalid = " << message_);
		return;
	}

	if (!SendToImServer(username, message_))
	{
		return;
	}
}

void CLogicOpt::Im2RelayGetDeviceUpdateStep()
{
	std::string device_guid;
	if (!jsonOpt_.JsonParseIm2RelayDeviceGUID(device_guid))
	{
		LOG4CXX_WARN(g_logger, "CLogicOpt::Im2RelayGetDeviceUpdateStep:json invalid = " << message_);
		return;
	}

	if (!SendToImDevServer(device_guid, message_))
	{
		return;
	}
}

void CLogicOpt::Im2RelayGetDeviceUpdateStepResult()
{
	LOG4CXX_TRACE(g_logger, "CLogicOpt::Im2RelayGetDeviceUpdateStepResult:json = " << message_);

	std::string username;
	if (!jsonOpt_.JsonParseUsername(username))
	{
		LOG4CXX_WARN(g_logger, "CLogicOpt::Im2RelayGetDeviceUpdateStepResult:json invalid = " << message_);
		return;
	}

	if (!SendToImServer(username, message_))
	{
		return;
	}
}

void CLogicOpt::Im2RelayDeviceRebootCMD()
{
	std::string device_guid;
	if (!jsonOpt_.JsonParseIm2RelayDeviceGUID(device_guid))
	{
		LOG4CXX_WARN(g_logger, "CLogicOpt::Im2RelayDeviceRebootCMD:json invalid = " << message_);
		return;
	}

	if (!SendToImDevServer(device_guid, message_))
	{
		return;
	}
}

void CLogicOpt::Im2RelayDeviceModifyPassword()
{
	std::string device_guid;
	if (!jsonOpt_.JsonParseIm2RelayDeviceGUID(device_guid))
	{
		LOG4CXX_WARN(g_logger, "CMessageOpt::Im2RelayDeviceModifyPassword:json invalid = " << message_);
		return;
	}

	if (!SendToImDevServer(device_guid, message_))
	{
		return;
	}
}

void CLogicOpt::Im2RelayDeviceModifyPasswordResult()
{
	LOG4CXX_TRACE(g_logger, "CLogicOpt::Im2RelayDeviceModifyPasswordResult:json = " << message_);

	std::string username;
	if (!jsonOpt_.JsonParseUsername(username))
	{
		LOG4CXX_WARN(g_logger, "CLogicOpt::Im2RelayDeviceModifyPasswordResult:json invalid = " << message_);
		return;
	}

	if (!SendToImServer(username, message_))
	{
		return;
	}
}

void CLogicOpt::ImDev2RelayAlarmMessage()
{
	LOG4CXX_TRACE(g_logger, "CLogicOpt::ImDev2RelayAlarmMessage:json = " << message_);

	std::string username;
	if (!jsonOpt_.JsonParseUsername(username))
	{
		LOG4CXX_WARN(g_logger, "CLogicOpt::ImDev2RelayAlarmMessage:json invalid = " << message_);
		return;
	}

	if (!SendToImServer(username, message_))
	{
		return;
	}
}
