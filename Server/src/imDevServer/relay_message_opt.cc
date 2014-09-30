/*
 * relay_message_opt.cc
 *
 *  Created on: 2014-2-13
 *      Author: zhangs
 */

#include "relay_message_opt.h"
#include "redis_conn_pool.h"
#include "redis_opt.h"
#include "net_core.h"
#include "work_thread.h"
#include "global_settings.h"
#include "../public/message.h"
#include "../public/socket_wrapper.h"
#include "../public/utils.h"


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
	case RELAY_DEVICE_MODIFY_INFO:
		OptRelayDeviceModifyInfo();
		break;
	case RELAY_DEVICE_UPDATE_CMD:
		OptRelayDeviceUpdateCMD();
		break;
	case RELAY_DEVICE_CANCEL_CMD:
		OptRelayDeviceCancelCMD();
		break;
	case RELAY_GET_DEVICE_UPDATE_STEP:
		OptRelayGetDeviceUpdateStep();
		break;
	case RELAY_DEVICE_REBOOT_CMD:
		OptRelayDeviceRebootCMD();
		break;
	case RELAY_DEVICE_MODIFY_PASSWORD:
		OptRelayDeviceModifyPassword();
		break;
	default:
		break;
	}

	CRedisConnPool::GetInstance()->ReleaseRedisContext(con);
}

bool CRelayMessageOpt::VerifyDeviceSfd(const std::string& device_guid, int& sfd)
{
	redisOpt_.SelectDB(DEVICE);

	/* 1.从缓存中取出此设备相应连接的sfd和其对应id */
	std::string str_sfd, str_sfd_id;
	int	sfd_id = 0;
	if (!redisOpt_.Hget(RedisKeyDeviceOnlineServerInfo(device_guid), KEY_ONLINE_SERVER_FD, str_sfd))
	{
		LOG4CXX_ERROR(g_logger, "CRelayMessageOpt::VerifyDeviceSfd KEY_ONLINE_SERVER_FD.");
		return false;
	}
	if (!redisOpt_.Hget(RedisKeyDeviceOnlineServerInfo(device_guid), KEY_ONLINE_SERVER_FD_ID, str_sfd_id))
	{
		LOG4CXX_ERROR(g_logger, "CRelayMessageOpt::VerifyDeviceSfd KEY_ONLINE_SERVER_FD_ID.");
		return false;
	}

	/* 2.在map中进行校验看设备端连接是否已经失效 */
	sfd = atoi(str_sfd.c_str());
	sfd_id = atoi(str_sfd_id.c_str());
	if (!CNetCore::map_csfd_id_.findValueByKey(sfd, sfd_id))
	{
		LOG4CXX_TRACE(g_logger, "CRelayMessageOpt::VerifyDeviceSfd map_csfd_id_ has invalid.");
		return false;
	}

	return true;
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
		LOG4CXX_WARN(g_logger, "CRelayMessageOpt::SendToClient WriteSfd error." << strerror(errno));
		return false;
	}

	return true;
}

void CRelayMessageOpt::OptRelayDeviceModifyInfo()
{
	/* 1.解析出目的设备GUID */
	std::string device_guid;
	int alarm_switch = 0;

	if (!jsonOpt_.JsonParseDeviceModifyInfo(message_, device_guid, alarm_switch))
	{
		LOG4CXX_WARN(g_logger, "CRelayMessageOpt::OptRelayDeviceModifyInfo json string invalid:" << message_);
		return;
	}

	/* 2.验证此设备的长连接是否还有效*/
	int sfd = 0;
	if(!VerifyDeviceSfd(device_guid, sfd))
	{
		return;
	}

	std::string client_message;
	if (alarm_switch == 1)
	{
		int device_sub_type = DEV_TYPE_SUB_UNKNOWN;
		int ret = 0;
		if ((ret = sqlOpt_.GetDeviceSubTypeFromDB(device_guid, device_sub_type)) == SUCCESS)
		{
			// 清华同方的alarm_switch的打开为7
			if (device_sub_type == DEV_TYPE_IPC_E2 || device_sub_type == DEV_TYPE_IPC_E2_0130)
			{
				alarm_switch = 7;
			}
		}
		else
		{
			LOG4CXX_WARN(g_logger, "GetDeviceSubTypeFromDB error ret=" << ret);
		}
		client_message = jsonOpt_.RestructDeviceModifyInfo2Client(message_, alarm_switch);
	}
	else
	{
		client_message = jsonOpt_.RestructDeviceModifyInfo2Client(message_);
	}
	/* 3.将转发信息发送至设备端 */
	if(!SendToClient(sfd, client_message))
	{
		return;
	}
}

void CRelayMessageOpt::OptRelayDeviceUpdateCMD()
{
	/* 1.解析出目的设备GUID */
	std::string device_guid;
	if (!jsonOpt_.JsonParseRelay2ImDevMessage(message_, device_guid))
	{
		LOG4CXX_WARN(g_logger, "CRelayMessageOpt::OptRelayDeviceUpdateCMD json string invalid:" << message_);
		return;
	}

	/* 2.验证此设备的长连接是否还有效 */
	int sfd = 0;
	if(!VerifyDeviceSfd(device_guid, sfd))
	{
		return;
	}

	std::string client_message;
	client_message = jsonOpt_.RestructDeviceUpdateCMD2Client(message_);
	/* 3.将转发信息发送至设备端 */
	if(!SendToClient(sfd, client_message))
	{
		return;
	}
}

void CRelayMessageOpt::OptRelayDeviceCancelCMD()
{
	/* 1.解析出目的设备GUID */
	std::string device_guid;
	if (!jsonOpt_.JsonParseRelay2ImDevMessage(message_, device_guid))
	{
		LOG4CXX_WARN(g_logger, "CRelayMessageOpt::OptRelayDeviceUpdateCMD json string invalid:" << message_);
		return;
	}

	/* 2.验证此设备的长连接是否还有效 */
	int sfd = 0;
	if(!VerifyDeviceSfd(device_guid, sfd))
	{
		return;
	}

	std::string client_message;
	client_message = jsonOpt_.RestructPush2DCommon(message_);
	/* 3.将转发信息发送至设备端 */
	if(!SendToClient(sfd, client_message))
	{
		return;
	}
}

void CRelayMessageOpt::OptRelayGetDeviceUpdateStep()
{
	/* 1.解析出目的设备GUID */
	std::string device_guid;
	if (!jsonOpt_.JsonParseRelay2ImDevMessage(message_, device_guid))
	{
		LOG4CXX_WARN(g_logger, "CRelayMessageOpt::OptRelayGetDeviceUpdateStep json string invalid:" << message_);
		return;
	}

	/* 2.验证此设备的长连接是否还有效 */
	int sfd = 0;
	if(!VerifyDeviceSfd(device_guid, sfd))
	{
		return;
	}

	std::string client_message;
	client_message = jsonOpt_.RestructGetDeviceUpdateStep2Client(message_);
	/* 3.将转发信息发送至设备端 */
	if(!SendToClient(sfd, client_message))
	{
		return;
	}
}

void CRelayMessageOpt::OptRelayDeviceRebootCMD()
{
	/* 1.解析出目的设备GUID */
	std::string device_guid;
	if (!jsonOpt_.JsonParseRelay2ImDevMessage(message_, device_guid))
	{
		LOG4CXX_WARN(g_logger, "CRelayMessageOpt::OptRelayDeviceRebootCMD json string invalid:" << message_);
		return;
	}

	/* 2.验证此设备的长连接是否还有效 */
	int sfd = 0;
	if(!VerifyDeviceSfd(device_guid, sfd))
	{
		return;
	}

	std::string client_message;
	client_message = jsonOpt_.RestructPush2DCommon(message_);
	/* 3.将转发信息发送至设备端 */
	if(!SendToClient(sfd, client_message))
	{
		return;
	}
}

void CRelayMessageOpt::OptRelayDeviceModifyPassword()
{
	/* 1.解析出目的设备GUID */
	std::string device_guid;
	if (!jsonOpt_.JsonParseRelay2ImDevMessage(message_, device_guid))
	{
		LOG4CXX_WARN(g_logger, "CRelayMessageOpt::OptRelayDeviceModifyPassword json string invalid:" << message_);
		return;
	}

	/* 2.验证此设备的长连接是否还有效 */
	int sfd = 0;
	if(!VerifyDeviceSfd(device_guid, sfd))
	{
		return;
	}

	std::string client_message;
	client_message = jsonOpt_.RestructDeviceModifyPassword2Client(message_);
	/* 3.将转发信息发送至设备端 */
	if(!SendToClient(sfd, client_message))
	{
		return;
	}
}
