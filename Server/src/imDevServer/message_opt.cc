/*
 * message_opt.cc
 *
 *  Created on: 2014-2-13
 *      Author: zhangs
 */

#include "message_opt.h"
#include "local_transport.h"
#include "redis_conn_pool.h"
#include "json_opt.h"
#include "sql_opt.h"
#include "redis_opt.h"
#include "global_settings.h"
#include "../public/utils.h"
#include "../public/socket_wrapper.h"
#include "net_interface.h"

const std::string delimiter = ",";
extern INetChannelInterface *g_LogicDispatch_ptr;

CMessageOpt::CMessageOpt(conn* c)
{
	c_ = c;
	result_ = 0;
	jsonOpt_ptr_ = new CJsonOpt;
	assert(jsonOpt_ptr_ != NULL);
	redisOpt_ptr_ = new CRedisOpt;
	assert(redisOpt_ptr_ != NULL);
	sqlOpt_ptr_ = new CSqlOpt;
	assert(sqlOpt_ptr_ != NULL);
	isResponse_ = true;
}

CMessageOpt::~CMessageOpt()
{
	utils::SafeDelete(jsonOpt_ptr_);
	utils::SafeDelete(redisOpt_ptr_);
	utils::SafeDelete(sqlOpt_ptr_);
}

void CMessageOpt::StartLogicOpt(const std::string &message)
{
	//jsonOpt_ptr_ = new CJsonOpt;
	assert(jsonOpt_ptr_ != NULL);


	int logic_type = 0;
	int message_type = 0;
	if (!jsonOpt_ptr_->ParseMessageLogicType(message.c_str(), logic_type, message_type))
	{
		LOG4CXX_ERROR(g_logger, "CLogicOperate::StartLogicOpt:JsonParseMessageType invalid :" << message);
		return;
	}

	jsonOpt_ptr_->ParseDeviceGuidMessage(message.c_str(), deviceguid_);

	/* 从redis连接池获取一个链接 */
	redisContext* con = CRedisConnPool::GetInstance()->GetRedisContext();
	redisOpt_ptr_->SetRedisContext(con);

	result_ = SUCCESS;
	LOG4CXX_TRACE(g_logger, "CLogicOperate::StartLogicOpt:message = :" << message);

	switch(message_type)
	{
	case DEVICE_ONLINE:
		DeviceOnline();
		break;
	case DEVICE_OFFLINE:
		DeviceOffline();
		break;
	case DEVICE_HEARTBEAT:
		DeviceHeartBeat();
		break;
	case DEVICE_REPORT_HUMITURE:
		DeviceReportHumiture();
		break;
	case PUSH_DEVICE_MODIFY_RESULT:
		PushDeviceModifyInfoRes();
		break;
	case PUSH_DEVICE_UPDATE_CMD_RESULT:
		PushDeviceUpdateCMDRes();
		break;
	case PUSH_DEVICE_CANCEL_CMD_RESULT:
		PushDeviceCancelCMDRes();
		break;
	case GET_DEVICE_UPDATE_STEP_RESULT:
		GetDeviceUpdateStepRes();
		break;
	case PUSH_DEVICE_MODIFY_PASSWORD_RESULT:
		PushDeviceModifyPasswordRes();
		break;
	case PUSH_ALARM_MESSAGE:
		PushAlarmMessage();
		break;
	case PUSH_ALARM_MESSAGE_FTP:
		PushAlarmMessageFtp();
		break;
	default:
		break;
	}

	if (isResponse_)
	{
		SendToClient();
	}
	
	CRedisConnPool::GetInstance()->ReleaseRedisContext(con);
}

bool CMessageOpt::SendToRelayServer(const int relay_type)
{
	/* 在消息中加上源用户名 */
	std::string str_relay = jsonOpt_ptr_->RestructIm2RelayMessage(relay_type);

	LOG4CXX_TRACE(g_logger, "CMessageOpt::SendToRelayServer:message = " << str_relay);

	return CLocalTransport::GetInstance()->SendToRelay(str_relay);
}

void CMessageOpt::SendToClient()
{
	responseToClient_.append(CRLF);

	if (!SocketOperate::WriteSfd(c_->sfd, responseToClient_.c_str(), responseToClient_.length()))
	{
		LOG4CXX_WARN(g_logger, "CLogicOperate::SendToClient:sendto failed.");
	}

	LOG4CXX_TRACE(g_logger, "CLogicOperate::SendToClient:sendto = " << responseToClient_);
}

void CMessageOpt::DeviceHeartBeat()
{
	redisOpt_ptr_->SelectDB(DEVICE);

	result_ = SUCCESS;
	std::string device_change;

	if (!redisOpt_ptr_->Set(RedisKeyDeviceOnlineFlag(deviceguid_), ONLINE))
	{
		LOG4CXX_WARN(g_logger, "CMessageOpt::DeviceHeartBeat:RedisKeyDeviceOnlineFlag error.");
	}

	/* 心跳，更新设备在线状态key缓存时间 */
	if (!redisOpt_ptr_->Expire(RedisKeyDeviceOnlineFlag(deviceguid_),utils::G<CGlobalSettings>().client_heartbeat_timeout_))
	{
		LOG4CXX_WARN(g_logger, "CMessageOpt::DeviceHeartBeat:Expire error.");
	}

	if(!redisOpt_ptr_->Get(RedisKeyDeviceChange(deviceguid_), device_change))
	{
		LOG4CXX_WARN(g_logger, "CMessageOpt::DeviceHeartBeat:RedisKeyDeviceChange error.");
	}

	if(!redisOpt_ptr_->Set(RedisKeyDeviceChange(deviceguid_), 0))
	{
		LOG4CXX_WARN(g_logger, "CMessageOpt::DeviceHeartBeat:RedisKeyDeviceChange error.");
	}

	responseToClient_ = jsonOpt_ptr_->JsonJoinHeartBeat(DEVICE_HEARTBEAT_RESPONSE, result_, device_change);
}

void CMessageOpt::DeviceOnline()
{
	redisOpt_ptr_->SelectDB(DEVICE);

	bool flag = false;
	assert(c_ != NULL);

	/* 将此设备的上线状态及上线服务器信息写入缓存 设置设备在线状态key的过期时间为客户端心跳超时时间，防止有错误在线情况*/
	LOG4CXX_TRACE(g_logger, "CMessageOpt::DeviceOnline>>>");
	flag = redisOpt_ptr_->Set(RedisKeyDeviceOnlineFlag(deviceguid_), ONLINE)
		&& redisOpt_ptr_->Hset(RedisKeyDeviceOnlineServerInfo(deviceguid_), KEY_ONLINE_SERVER_NO, utils::G<CGlobalSettings>().bind_port_)
		&& redisOpt_ptr_->Hset(RedisKeyDeviceOnlineServerInfo(deviceguid_), KEY_ONLINE_SERVER_FD, c_->sfd)
		&& redisOpt_ptr_->Hset(RedisKeyDeviceOnlineServerInfo(deviceguid_), KEY_ONLINE_SERVER_FD_ID, c_->id)
		&& redisOpt_ptr_->Expire(RedisKeyDeviceOnlineFlag(deviceguid_), utils::G<CGlobalSettings>().client_heartbeat_timeout_);
	if (!flag)
	{
		LOG4CXX_ERROR(g_logger, "CMessageOpt::DeviceOnline:redisOpt error.");
		result_ = REDIS_OPT_ERROR;
		goto GET_RESPONSE;
	}

GET_RESPONSE:
	responseToClient_ = jsonOpt_ptr_->JsonJoinCommon(DEVICE_ONLINE_RESPONSE, result_);
}
void CMessageOpt::DeviceOffline()
{
	redisOpt_ptr_->SelectDB(DEVICE);

	bool flag = false;

	/* 将此用户的上线状态及上线服务器号写入缓存 */
	LOG4CXX_TRACE(g_logger, "CMessageOpt::DeviceOffline>>>");
	flag = redisOpt_ptr_->Set(RedisKeyDeviceOnlineFlag(deviceguid_), OFFLINE);
	if (!flag)
	{
		result_ = REDIS_OPT_ERROR;
	}

	responseToClient_ = jsonOpt_ptr_->JsonJoinCommon(DEVICE_OFFLINE_RESPONSE, result_);
}

void CMessageOpt::DeviceReportHumiture()
{
	redisOpt_ptr_->SelectDB(HUMITURE);

	result_ = SUCCESS;

	std::string tem;
	std::string hum;
	std::string timestamp;

	std::string hkey;
	if(!jsonOpt_ptr_->JsonParseDeviceReportHumiture(tem, hum, timestamp))
	{
		LOG4CXX_ERROR(g_logger, "CMessageOpt::DeviceReportHumiture:JsonParseDeviceReportHumiture invalid");
		result_ = JSON_PARSE_ERROR;
		goto GET_RESPONSE;
	}

	hkey = RedisKeyDeviceHumiture(deviceguid_, timestamp);
	if(!(redisOpt_ptr_->Hset(hkey, REDIS_HASH_FEILD_TEMPERATURE, tem) && redisOpt_ptr_->Hset(hkey, REDIS_HASH_FEILD_HUMIDNESS, hum)))
	{
		LOG4CXX_WARN(g_logger, "CMessageOpt::DeviceReportHumiture:Hset error.");
		result_ = REDIS_OPT_ERROR;
		goto GET_RESPONSE;
	}

	if (!(redisOpt_ptr_->Set(RedisKeyLatestDeviceHumitureKey(deviceguid_), hkey)))
	{
		LOG4CXX_WARN(g_logger, "CMessageOpt::DeviceReportHumiture:Set error.");
		result_ = REDIS_OPT_ERROR;
		goto GET_RESPONSE;
	}

	if (!redisOpt_ptr_->Expire(hkey, 86400))
	{
		LOG4CXX_WARN(g_logger, "CMessageOpt::DeviceReportHumiture:Expire error.");
		result_ = REDIS_OPT_ERROR;
		goto GET_RESPONSE;
	}

GET_RESPONSE:
	responseToClient_ = jsonOpt_ptr_->JsonJoinCommon(DEVICE_REPORT_HUMITURE_RESPONSE, result_);
}

void CMessageOpt::PushDeviceModifyInfoRes()
{
	bool flag = true;
	flag = jsonOpt_ptr_->VerifyD2CRelayMessage();
	if (!flag)
	{
		LOG4CXX_ERROR(g_logger, "CMessageOpt::PushDeviceModifyInfoRes. message is valid");
		result_ = JSON_PARSE_ERROR;
	}

	/* 将客户端消息进行重组发往转发服务器 */
	if (flag)
	{
		if (!SendToRelayServer(RELAY_DEVICE_MODIFY_RESULT))
		{
			LOG4CXX_WARN(g_logger, "CMessageOpt::PushDeviceModifyInfoRes:SendToRelayServer failed.");
		}
	}

	isResponse_ = false;
}

void CMessageOpt::PushDeviceUpdateCMDRes()
{
	bool flag = true;
	flag = jsonOpt_ptr_->VerifyD2CRelayMessage();
	if (!flag)
	{
		LOG4CXX_ERROR(g_logger, "CMessageOpt::PushDeviceUpdateCMDRes. message is valid");
		result_ = JSON_PARSE_ERROR;
	}

	/* 将客户端消息进行重组发往转发服务器 */
	if (flag)
	{
		if (!SendToRelayServer(RELAY_DEVICE_UPDATE_CMD_RESULT))
		{
			LOG4CXX_WARN(g_logger, "CMessageOpt::PushDeviceUpdateCMDRes:SendToRelayServer failed.");
		}
	}

	isResponse_ = false;
}

void CMessageOpt::PushDeviceCancelCMDRes()
{
	bool flag = true;
	flag = jsonOpt_ptr_->VerifyD2CRelayMessage();
	if (!flag)
	{
		LOG4CXX_ERROR(g_logger, "CMessageOpt::PushDeviceCancelCMDRes. message is valid");
		result_ = JSON_PARSE_ERROR;
	}

	/* 将客户端消息进行重组发往转发服务器 */
	if (flag)
	{
		if (!SendToRelayServer(RELAY_DEVICE_CANCEL_CMD_RESULT))
		{
			LOG4CXX_WARN(g_logger, "CMessageOpt::PushDeviceCancelCMDRes:SendToRelayServer failed.");
		}
	}

	isResponse_ = false;
}

void CMessageOpt::GetDeviceUpdateStepRes()
{
	bool flag = true;
	flag = jsonOpt_ptr_->VerifyD2CRelayMessage();
	if (!flag)
	{
		LOG4CXX_ERROR(g_logger, "CMessageOpt::GetDeviceUpdateStepRes. message is valid");
		result_ = JSON_PARSE_ERROR;
	}

	/* 将客户端消息进行重组发往转发服务器 */
	if (flag)
	{
		if (!SendToRelayServer(RELAY_GET_DEVICE_UPDATE_STEP_RESULT))
		{
			LOG4CXX_WARN(g_logger, "CMessageOpt::GetDeviceUpdateStepRes:SendToRelayServer failed.");
		}
	}

	isResponse_ = false;
}

void CMessageOpt::PushDeviceModifyPasswordRes()
{
	bool flag = true;
	flag = jsonOpt_ptr_->VerifyD2CRelayMessage();
	if (!flag)
	{
		LOG4CXX_ERROR(g_logger, "CMessageOpt::PushDeviceModifyPasswordRes. message is valid");
		result_ = JSON_PARSE_ERROR;
	}

	/* 将客户端消息进行重组发往转发服务器 */
	if (flag)
	{
		if (!SendToRelayServer(RELAY_DEVICE_MODIFY_PASSWORD_RESULT))
		{
			LOG4CXX_WARN(g_logger, "CMessageOpt::PushDeviceModifyPasswordRes:SendToRelayServer failed.");
		}
	}

	isResponse_ = false;
}

int CMessageOpt::GetAccountLiveInfo(ACCOUNT_LIVE_INFO& accountLiveInfo)
{
	int ret = SUCCESS;

	std::string s_online_status = "0";
	std::string s_login_platform = "-1";
	std::string s_mobile_id = "";
	std::string s_language_type = "0";
	std::string s_alarm_flag = "1";

	redisOpt_ptr_->SelectDB(STATUS);

	if(!redisOpt_ptr_->Get(RedisKeyUserOnlineFlag(accountLiveInfo.username), s_online_status))
	{
		LOG4CXX_ERROR(g_logger, "CMessageOpt::GetAccountLiveInfo RedisKeyClientLoginInfo get onlineflag error " << accountLiveInfo.username);
	}
	else
	{
		if (s_online_status.empty())
		{
			s_online_status = "0";
		}
		accountLiveInfo.online_status = atoi(s_online_status.c_str());
	}

	if (!redisOpt_ptr_->Hget(RedisKeyClientLoginInfo(accountLiveInfo.username), KEY_LOGIN_PLATFORM, s_login_platform))
	{
		LOG4CXX_ERROR(g_logger, "CMessageOpt::GetAccountLiveInfo RedisKeyClientLoginInfo Hget KEY_LOGIN_PLATFORM error " << accountLiveInfo.username);
		ret = REDIS_OPT_ERROR;
	}
	else
	{
		if (s_login_platform.empty())
		{
			s_login_platform = "-1";
		}
		accountLiveInfo.login_platform = atoi(s_login_platform.c_str());
	}

	if (!redisOpt_ptr_->Hget(RedisKeyClientLoginInfo(accountLiveInfo.username), KEY_LANGUAGE_TYPE, s_language_type))
	{
		LOG4CXX_ERROR(g_logger, "CMessageOpt::GetAccountLiveInfo RedisKeyClientLoginInfo Hget KEY_LANGUAGE_TYPE error " << accountLiveInfo.username);
		ret = REDIS_OPT_ERROR;
	}
	else
	{
		if (s_language_type.empty())
		{
			s_language_type = "0";
		}
		accountLiveInfo.language_type = atoi(s_language_type.c_str());
	}
	

	if (!redisOpt_ptr_->Hget(RedisKeyClientLoginInfo(accountLiveInfo.username),KEY_ALARM_FLAG, s_alarm_flag))
	{
		LOG4CXX_ERROR(g_logger, "CMessageOpt::GetAccountLiveInfo RedisKeyClientLoginInfo Hget KEY_ALARM_FLAG error " << accountLiveInfo.username);
		ret = REDIS_OPT_ERROR;
	}
	else
	{
		if (s_alarm_flag.empty())
		{
			s_alarm_flag = "1";
		}
		accountLiveInfo.alarm_flag = atoi(s_alarm_flag.c_str());
	}

	if (!redisOpt_ptr_->Hget(RedisKeyClientLoginInfo(accountLiveInfo.username), KEY_MOBILE_ID, s_mobile_id))
	{
		s_mobile_id = "";
		LOG4CXX_ERROR(g_logger, "CMessageOpt::GetAccountLiveInfo RedisKeyClientLoginInfo Hget KEY_MOBILE_ID error " << accountLiveInfo.username);
		ret = REDIS_OPT_ERROR;
	}
	accountLiveInfo.moblie_id = s_mobile_id;

	return ret;
}

bool CMessageOpt::SetAlarmMsgToRedis(const std::string& username, const int alarm_solution, const ALARM_INFO& alarm_info)
{
	redisOpt_ptr_->SelectDB(ALARM_MSG);
	
	std::string alarm_msg_val;

	// 视频图片保存在设备
	if (AS_LOCAL == alarm_solution)
	{
		// alarm_guid|alarm_solution|device_guid|device_name|channel_no|alarm_type|pic_name|vedio_name|alarm_timestamp
		alarm_msg_val = std::string(alarm_info.alarm_guid).append("|")
			.append(utils::int2str(alarm_solution)).append("|")
			.append(alarm_info.device_guid).append("|")
			.append(alarm_info.device_name).append("|")
			.append(utils::int2str(alarm_info.alarm_channel_no)).append("|")
			.append(utils::int2str(alarm_info.alarm_type)).append("|")
			.append(alarm_info.alarm_pic).append("|")
			.append(alarm_info.alarm_video).append("|")
			.append(utils::int2str(alarm_info.alarm_timestamp));
		if (!redisOpt_ptr_->LPush(username, alarm_msg_val))
		{
			LOG4CXX_ERROR(g_logger, "CMessageOpt::SetAlarmMsgToRedis LPush error.");
			return false;
		}
		int list_len = 0;
		redisOpt_ptr_->LLen(username, list_len);
		list_len -= ALARM_MSG_LIST_LEN;
		if (list_len > 0)
		{
			for (int i = 0; i < list_len; ++i)
			{
				redisOpt_ptr_->LPop(username);
			}
		}
	}
	if (AS_FTP == alarm_solution)
	{
		// alarm_guid|alarm_solution|message_type|device_guid|device_name|channel_no|alarm_type|pic_name|vedio_name|alarm_timestamp

	}

	return true;
}

int CMessageOpt::GetAlarmLevel(int alarm_type) {
	switch (alarm_type)
	{            
	case ALARM_DISKERROR:
		return 2;
		break;
	case ALARM_DISKFULL:
		return 1;
		break;
	case ALARM_DISCONN:
		return 1;
		break;
	case ALARM_UPGRADE:
		return 2;
		break;
	case ALARM_GPIN:
		return 2;
		break;
	case ALARM_VIDEOLOSE:
		return 2;
		break;
	case ALARM_HIDEDETECT:
		return 3;
		break;
	case ALARM_MOTIONDETECT:
		return 3;
		break;
	case ALARM_POWER_OFF:
		return 1;
		break;
	case ALARM_MANUALALARM:
		return 1;
		break;
	case ALARM_GPS:
		return 3;
		break;
		//case ALARM_PIR:
		//	return 3;
		//	break;
	case ALARM_NONE:
		return 1;
		break;
	case ALARM_NOTIFY:
		return 2;
		break;
	case ALARM_TEMP_HIGH:
		return 3;
		break;
	case ALARM_TEMP_LOW:
		return 3;
		break;
	case ALARM_HUM_HIGH:
		return 3;
		break;
	case ALARM_HUM_LOW:
		return 3;
		break;
	}

	return 3;
}

void CMessageOpt::PushAlarmMessage()
{
	ALARM_INFO alarm_info;
	std::vector<std::string> users;
	std::string get_device_users_request;
	std::string get_device_users_response;
	std::string str_relay;
	ACCOUNT_LIVE_INFO acc_live_info;

	if (!jsonOpt_ptr_->JsonParsePushAlarmMessage(alarm_info))
	{
		result_ = JSON_PARSE_ERROR;
		goto GET_RESPONSE;
	}

	// 向业务进程通信，根据云视通号获取用户列表
	get_device_users_request = jsonOpt_ptr_->JsonJoinGetDeviceUsers(alarm_info.device_guid);
	g_LogicDispatch_ptr->GetResponseByRequestShortConnection(get_device_users_request, get_device_users_response);
	if (!jsonOpt_ptr_->JsonParseGetDeviceUsers(get_device_users_response, users))
	{
		result_ = JSON_PARSE_ERROR;
		goto GET_RESPONSE;
	}

	for (unsigned i = 0; i < users.size(); ++i)
	{
		// 获取账号状态
		acc_live_info.username = users.at(i);
		if ((result_ = GetAccountLiveInfo(acc_live_info)) != SUCCESS)
		{
			goto GET_RESPONSE;
		}

		if (acc_live_info.alarm_flag == ALARM_ON)
		{
			if (acc_live_info.online_status == ONLINE)
			{
				// 向客户端发送
				str_relay = jsonOpt_ptr_->JsonJoinAlarmMessage2Relay(users.at(i), alarm_info);
				LOG4CXX_TRACE(g_logger, "CMessageOpt::PushAlarmMessage SendToRelay message=" << str_relay);
				CLocalTransport::GetInstance()->SendToRelay(str_relay);
			}
			else if (acc_live_info.login_platform == MOBILE_IPHONE || acc_live_info.login_platform == MOBILE_IPAD)
			{
				// todo 苹果离线推送服务器
			}

			// 记录报警消息缓存
			if (!SetAlarmMsgToRedis(acc_live_info.username, AS_LOCAL, alarm_info))
			{
				LOG4CXX_ERROR(g_logger, "CMessageOpt::PushAlarmMessage SetAlarmMsgToRedis error.");
				result_ = REDIS_OPT_ERROR;
				goto GET_RESPONSE;
			}
		}
	}

GET_RESPONSE:
	responseToClient_ = jsonOpt_ptr_->JsonJoinPushAlarmMessage(PUSH_ALARM_MESSAGE_RESPONSE, result_, alarm_info.alarm_msg_type, alarm_info.alarm_guid);
}

void CMessageOpt::PushAlarmMessageFtp()
{
	ALARM_INFO					alarm_info;
	std::vector<std::string>	users;
	std::string					get_device_users_request;
	std::string					get_device_users_response;
	std::string					str_relay;
	int							alarm_level;
	ACCOUNT_LIVE_INFO			acc_live_info;
	char						strtime[100];
	struct tm					*p;
	char						url_pic[200];
	char						url_video[200];
	const char url_pic_format[]		= "http://%s:%d/GetAlarmImage.php/%s-%04d%02d%02d-%02d%02d%02d";
	const char url_video_format[]	= "http://%s:%d/GetAlarmVideo.php/%s-%04d%02d%02d-%02d%02d%02d";

	if (!jsonOpt_ptr_->JsonParsePushAlarmMessageFtp(alarm_info))
	{
		LOG4CXX_ERROR(g_logger, "CMessageOpt::PushAlarmMessageFtp JsonParsePushAlarmMessageFtp error");
		result_ = JSON_PARSE_ERROR;
		goto GET_RESPONSE;
	}

	// 向业务进程通信，根据云视通号获取用户列表
	get_device_users_request = jsonOpt_ptr_->JsonJoinGetDeviceUsers(alarm_info.device_guid);
	g_LogicDispatch_ptr->GetResponseByRequestShortConnection(get_device_users_request, get_device_users_response);
	if (!jsonOpt_ptr_->JsonParseGetDeviceUsers(get_device_users_response, users))
	{
		LOG4CXX_ERROR(g_logger, "CMessageOpt::PushAlarmMessageFtp JsonParseGetDeviceUsers error device=" << alarm_info.device_guid);
		result_ = JSON_PARSE_ERROR;
		goto GET_RESPONSE;
	}

	LOG4CXX_ERROR(g_logger, "CMessageOpt::PushAlarmMessageFtp JsonParseGetDeviceUsers users count" << users.size());

	memset(strtime, 0, sizeof(strtime));

	p = localtime(&alarm_info.alarm_timestamp);
	p->tm_year = p->tm_year + 1900;
	p->tm_mon = p->tm_mon + 1;

	snprintf(strtime, sizeof(strtime), "%04d-%02d-%02d %02d:%02d:%02d", p->tm_year, p->tm_mon, p->tm_mday,p->tm_hour,p->tm_min,p->tm_sec);

	snprintf(url_pic, sizeof(url_pic) - 1, url_pic_format, utils::G<CGlobalSettings>().ftp_server_domain_.c_str(), utils::G<CGlobalSettings>().ftp_server_port_, alarm_info.device_guid.c_str(), p->tm_year, p->tm_mon, p->tm_mday,p->tm_hour,p->tm_min,p->tm_sec);
	snprintf(url_video, sizeof(url_video) - 1, url_video_format, utils::G<CGlobalSettings>().ftp_server_domain_.c_str(), utils::G<CGlobalSettings>().ftp_server_port_, alarm_info.device_guid.c_str(), p->tm_year, p->tm_mon, p->tm_mday,p->tm_hour,p->tm_min,p->tm_sec);

 	alarm_level = GetAlarmLevel(alarm_info.alarm_type);

	alarm_info.alarm_pic = url_pic;
	alarm_info.alarm_video = url_video;

	for (unsigned i = 0; i < users.size(); ++i)
	{
		// 获取账号状态
		acc_live_info.username = users.at(i);
		if ((result_ = GetAccountLiveInfo(acc_live_info)) != SUCCESS)
		{
			LOG4CXX_ERROR(g_logger, "CMessageOpt::PushAlarmMessageFtp GetAccountLiveInfo error user=" << acc_live_info.username);
			goto GET_RESPONSE;
		}

		LOG4CXX_TRACE(g_logger, "CMessageOpt::PushAlarmMessageFtp GetAccountLiveInfo OK user=" << acc_live_info.username << 
			" Online=" << acc_live_info.online_status <<
			" AlarmFlag=" << acc_live_info.alarm_flag);

		if (acc_live_info.alarm_flag == ALARM_ON)
		{
			if (acc_live_info.online_status == ONLINE)
			{
				// 向客户端发送
				str_relay = jsonOpt_ptr_->JsonJoinAlarmFtpMessage2Relay(users.at(i), alarm_info, alarm_level);
				LOG4CXX_TRACE(g_logger, "CMessageOpt::PushAlarmMessageFtp SendToRelay message=" << str_relay);
				CLocalTransport::GetInstance()->SendToRelay(str_relay);
			}
			else if (acc_live_info.login_platform == MOBILE_IPHONE || acc_live_info.login_platform == MOBILE_IPAD)
			{
				// todo 苹果离线推送服务器
			}

			// 记录报警消息缓存
			if (!SetAlarmMsgToRedis(acc_live_info.username, AS_LOCAL, alarm_info))
			{
				LOG4CXX_ERROR(g_logger, "CMessageOpt::PushAlarmMessageFtp SetAlarmMsgToRedis error.");
				result_ = REDIS_OPT_ERROR;
				goto GET_RESPONSE;
			}
		}
	}

GET_RESPONSE:
	responseToClient_ = jsonOpt_ptr_->JsonJoinPushAlarmMessage(PUSH_ALARM_MESSAGE_FTP_RESPONSE, result_, alarm_info.alarm_msg_type, alarm_info.alarm_guid);
}
