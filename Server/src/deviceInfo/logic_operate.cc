#include "logic_operate.h"
#include "json_opt.h"
#include "sql_opt.h"
#include "redis_opt.h"
#include "redis_conn_pool.h"
#include "base64.h"
#include "cloudsee_transport.h"
#include "cloudsee_data_opt.h"
#include "../public/redis_key.h"
#include "../public/utils.h"
#include "../public/user_interface_defines.h"
#include "../public/socket_wrapper.h"
#include <math.h>

extern struct ClientInfo g_clientInfo;
CThreadSafeMap<int, NET_MSG> g_map_message;

CLogicOperate::CLogicOperate()
{
	jsonOpt_ptr_ = NULL;
	sqlOpt_ptr_	 = NULL;
	redisOpt_ptr_= NULL;
	dataOpt_ptr_ = NULL;
	jsonOpt_ptr_ = new CJsonOpt();
	assert(jsonOpt_ptr_ != NULL);
	sqlOpt_ptr_  = new CSqlOpt;
	assert(sqlOpt_ptr_ != NULL);
	redisOpt_ptr_ = new CRedisOpt;
	assert(redisOpt_ptr_ != NULL);
	dataOpt_ptr_ = new CCloudSeeDataOpt();
	assert(dataOpt_ptr_ != NULL);
	result_ = 0;
	pthread_mutex_init(&write_mutex, NULL);
}

CLogicOperate::~CLogicOperate()
{
	utils::SafeDelete(jsonOpt_ptr_);
	utils::SafeDelete(sqlOpt_ptr_);
	utils::SafeDelete(redisOpt_ptr_);
	utils::SafeDelete(dataOpt_ptr_);
	pthread_mutex_destroy(&write_mutex);
}

void CLogicOperate::StartLogicOpt(const std::string& message)
{
	unsigned long start_time = utils::GetTickCount();

	/** 恢复客户端进行的转义处理 */
	std::string string = utils::ReplaceString(message, "\\\\r\\\\n", "\\r\\n");
	LOG4CXX_TRACE(g_logger, "CLogicOperate::StartLogicOpt:message=" << string);

	jsonOpt_ptr_->setJsonString(string);
	int message_type = 0;
	std::string session_id;
	if((!jsonOpt_ptr_->JsonParseMessageType(message_type)))
	{
		LOG4CXX_ERROR(g_logger, "CLogicOperate::StartLogicOpt:JsonParseMessageType invalid :" << string);
		return;
	}

	result_ = SUCCESS;

	/* 进行session验证 */
	redisContext* con = CRedisConnPool::GetInstance()->GetRedisContext();
	redisOpt_ptr_->SetRedisContext(con);
	
	
	redisOpt_ptr_->SelectDB(SESSION);

	if(message_type != DEVICE_REGISTER && message_type != GET_DEVICE_USERNAMES)
	{
		if(!jsonOpt_ptr_->JsonParseSessionId(session_id_))
		{
			result_ = JSON_PARSE_ERROR;
		}
		else
		{
			if (!redisOpt_ptr_->Get(session_id_, username_))
				result_ = SESSION_NOT_EXSIT;
		}
	}
	

	switch(message_type)
	{
	case DEVICE_REGISTER:
		DeviceRegister();
		break;
	case GET_USER_DEVICES:
		GetUserDevices();
		break;
	case GET_DEVICE_INFO:
		GetDeviceInfo();
		break;
	case GET_USER_DEVICE_INFO:
		GetUserDeviceInfo();
		break;
	case MODIFY_DEVICE_INFO_VIDEO_LINK:
		ModifyDeviceInfoVideoLink();
		break;
	case USER_BIND_DEVICE:
		UserBindDevice();
		break;
	case USER_REMOVE_BIND_DEVICE:
		UserRemoveBindDevice();
		break;
	case GET_DEVICE_HUMITURE_STAT:
		GetDeviceHumiture();
		break;
	case GET_DEVICE_HUMITURE_ONTIME:
		GetDeviceHumitureOntime();
		break;
	case GET_USER_DEVICES_STATUS_INFO:
		GetUserDevicesStatusInfo();
		break;
	case GET_DEVICE_HUMITURE_SCORE:
		GetDeviceHumitureScore();
		break;
	case GET_DEVICE_USERNAMES:
		GetDeviceUsers();
		break;
	case MODIFY_DEVICE_CONF_INFO:
		ModifyDeviceConfInfo();
		break;
	case MODIFY_DEVICE_INFO_ADVANCED:
		ModifyDeviceInfoAdvanced();
		break;
	case GET_DEVICE_UPDATE_INFO:
		GetDeviceUpdateInfo();
		break;
	case MODIFY_DEVICE_PASSWORD:
		ModifyDevicePassword();
		break;
	case ADD_DEVICE_CHANNEL:
		AddDeviceChannel();
		break;
	case DELETE_DEVICE_CHANNEL:
		DeleteDeviceChannel();
		break;
	case GET_DEVICE_CHANNEL:
		GetDeviceChannel();
		break;
	case GET_USER_CHANNELS:
		GetUserChannels();
		break;
	case MODIFY_DEVICE_CHANNEL_NAME:
		ModifyDeviceChannelName();
		break;
	case MODIFY_DEVICE_WIFI_FLAG:
		ModifyDeviceWifiFlag();
		break;
	default:
		LOG4CXX_WARN(g_logger, "CLogicOperate::StartLogicOpt:message_type invalid.");
		break;
	}

	SendToDispatch();

	CRedisConnPool::GetInstance()->ReleaseRedisContext(con);

	unsigned long end_time = utils::GetTickCount();

	LOG4CXX_TRACE(g_logger, "CLogicOperate::StartLogicOpt: *******time_interval******* = " << end_time - start_time << "(ms), username = "
							<< username_ << ", message_type = " << message_type);
}

void CLogicOperate::SendToDispatch()
{
	responseToDispatch_.append(CRLF);
	if(responseToDispatch_.length() >= DATA_BUFFER_SIZE)
	{
		LOG4CXX_WARN(g_logger, "CLogicOperate::SendToDispatch data too large. data = " << responseToDispatch_);
		return;
	}
	
	if (!SocketOperate::WriteSfd(g_clientInfo.sfd, responseToDispatch_.c_str(), responseToDispatch_.length()))
	{
		LOG4CXX_WARN(g_logger, "CLogicOperate::SendToDispatch:sendto failed. error = " << strerror(errno));
	}
	
	LOG4CXX_TRACE(g_logger, "CLogicOperate::SendToDispatch:sendto = " << responseToDispatch_);
	LOG4CXX_TRACE(g_logger, "CLogicOperate::SendToDispatch return thread_id = " << pthread_self());
}

void CLogicOperate::DeviceRegister()
{
	redisOpt_ptr_->SelectDB(DEVICE);

	DEVICE_INFO deviceInfo;

	std::string device_password_base64;

	std::string hold_bind_username;

	if(!jsonOpt_ptr_->JsonParseDeviceRegister(deviceInfo))
	{
		result_ = JSON_PARSE_ERROR;
		goto GET_RESPONSE;
	}

	// device_password base64 encode
	device_password_base64 = base64_encode(reinterpret_cast<const unsigned char*>(deviceInfo.device_password.c_str()), deviceInfo.device_password.length());
	deviceInfo.device_password = device_password_base64;

	// device has been reseted where device_username=admin and device_password=null
	if (deviceInfo.device_username == "admin" && deviceInfo.device_password == "")
	{
		deviceInfo.reset_flag = 1;
	}
	else
	{
		deviceInfo.reset_flag = 0;
	}

	result_ = sqlOpt_ptr_->DeviceRegisterToDB(deviceInfo);
	if(SUCCESS == result_)
	{
		redisOpt_ptr_->Get(RedisKeyDeviceApConfFlag(deviceInfo.device_guid), hold_bind_username);
	}

	// 如果是出厂状态的设备,并且不是通过AP配置添加的,则删掉所有与用户关联
	if (deviceInfo.reset_flag == 1)
	{
		if (hold_bind_username.empty())
		{
			if ((result_ = RemoveBindByDevice_HomeCloud(deviceInfo.device_guid)) != SUCCESS)
			{
				goto GET_RESPONSE;
			}
		}
		else
		{
			if ((result_ = RemoveBindByDeviceHoldUser_HomeCloud(deviceInfo.device_guid, hold_bind_username)) != SUCCESS)
			{
				goto GET_RESPONSE;
			}

			redisOpt_ptr_->Del(RedisKeyDeviceApConfFlag(deviceInfo.device_guid));
		}

	}

GET_RESPONSE:
	responseToDispatch_ = jsonOpt_ptr_->JsonJoinDeviceRegister(result_);
}

int CLogicOperate::GetDeviceOnlineStatus(const std::string& device_guid, int& online_flag)
{
	redisOpt_ptr_->SelectDB(DEVICE);

	online_flag = OFFLINE;
	std::string str_online_flag;

	if (!redisOpt_ptr_->Get(RedisKeyDeviceOnlineFlag(device_guid), str_online_flag))
	{
		LOG4CXX_ERROR(g_logger, "CLogicOperate::GetDeviceOnlineStatus:Get error.device_guid=" << device_guid);
		return REDIS_OPT_ERROR;
	}
	online_flag = atoi(str_online_flag.c_str());

	return SUCCESS;
}

void CLogicOperate::GetUserDevices()
{
	redisOpt_ptr_->SelectDB(DEVICE);
	USER_DEVICES_LIST user_devices_list;
	std::vector<std::string> vec_device;
	std::string username;
	int message_id = 0;
	std::vector<DEVICE_INFO_CLOUDSEE> vec_dinfo_cloudsee;
	char cloudsee_request[DATA_BUFFER_SIZE] = {0};
	char cloudsee_response[DATA_BUFFER_SIZE] = {0};
	uint32_t request_len = 0;

	if (SessionNotExist())
	{
		goto GET_RESPONSE;
	}

	username = username_;

	message_id = jsonOpt_ptr_->JsonGetMessageId();

	result_ = sqlOpt_ptr_->GetUserDevicesInfoFromDB(username, vec_device, user_devices_list);
	if (result_ != SUCCESS)
	{
		goto GET_RESPONSE;
	}


	dataOpt_ptr_->CreateRequestDeviceInfo(message_id, vec_device, cloudsee_request, request_len);

	result_ = GetResponseFromCloudSee(message_id, cloudsee_request, request_len, cloudsee_response);
	if (result_ != SUCCESS)
	{
		LOG4CXX_ERROR(g_logger, "CLogicOperate::GetUserDevices:GetResponseFromCloudSee error.");
	}
	if (!dataOpt_ptr_->ParseResponseDeviceInfo(cloudsee_response, vec_dinfo_cloudsee))
	{
		LOG4CXX_ERROR(g_logger, "CLogicOperate::GetUserDevices:ParseResponseDeviceInfo error.");
		result_ = FAILED;
	}

	for (unsigned i = 0; i < user_devices_list.vec_user_device_info.size(); ++i)
	{
		GetDeviceOnlineStatusCloudSee(vec_dinfo_cloudsee, 
			user_devices_list.vec_user_device_info.at(i).device_info.device_guid, 
			user_devices_list.vec_user_device_info.at(i).device_info.online_status);
	}

GET_RESPONSE:

	responseToDispatch_ = jsonOpt_ptr_->JsonJoinGetUserDevices(result_, user_devices_list);
}

void CLogicOperate::GetDeviceUsers()
{
	std::string device_guid;
	std::vector<std::string> vec_un;
	std::string device_name;
	std::string alarm_time;
	int full_alarm_mode = 0;
	int reset_flag = 1;
	std::string device_username, device_password;

	if (!jsonOpt_ptr_->JsonParseGetDeviceInfo(device_guid))
	{
		result_ = JSON_PARSE_ERROR;
		goto GET_RESPONSE;
	}

	sqlOpt_ptr_->GetDeviceResetFlagFromDB(device_guid, reset_flag);
	if(reset_flag == 1)
	{
		result_ = DEVICE_IS_RESET;
		goto GET_RESPONSE;
	}

	sqlOpt_ptr_->GetDevicePasswordFromDB(device_guid, device_username, device_password);

	result_ = sqlOpt_ptr_->GetDeviceUsersFromDB(device_guid, vec_un, device_name, alarm_time, full_alarm_mode, device_username, device_password);

GET_RESPONSE:
	responseToDispatch_ = jsonOpt_ptr_->JsonJoinGetDeviceUsers(result_, vec_un, device_name, alarm_time, full_alarm_mode);
}

void CLogicOperate::GetDeviceInfo()
{
	std::string device_guid;
	DEVICE_INFO device_info;

	if (SessionNotExist())
	{
		goto GET_RESPONSE;
	}

	if (!jsonOpt_ptr_->JsonParseGetDeviceInfo(device_guid))
	{
		result_ = JSON_PARSE_ERROR;
		goto GET_RESPONSE;
	}

	if ((result_ = VerifyDeviceExist(device_guid)) != SUCCESS)
	{
		goto GET_RESPONSE;
	}

	result_ = sqlOpt_ptr_->GetDeviceInfoFromDB(device_guid, device_info);

GET_RESPONSE:

	responseToDispatch_ = jsonOpt_ptr_->JsonJoinGetDeviceInfo(result_, device_info);
}

void CLogicOperate::GetUserDeviceInfo()
{
	std::string username;
	std::string device_guid;
	USER_DEVICE_INFO user_device_info;

	if (SessionNotExist())
	{
		goto GET_RESPONSE;
	}

	username = username_;

	if (!jsonOpt_ptr_->JsonParseGetUserDeviceInfo(device_guid))
	{
		result_ = JSON_PARSE_ERROR;
		goto GET_RESPONSE;
	}

	result_ = sqlOpt_ptr_->GetUserDeviceInfoFromDB(username, device_guid, user_device_info);

GET_RESPONSE:

	responseToDispatch_ = jsonOpt_ptr_->JsonJoinGetUserDeviceInfo(result_, user_device_info);
}

void CLogicOperate::ModifyDeviceInfoVideoLink()
{
	std::string username;
	std::string device_guid;
	int video_link_type = 0;
	std::string video_username;
	std::string video_password;
	std::string video_ip;
	int video_port = 0;
	int device_verify = 0;

	if (SessionNotExist())
	{
		goto GET_RESPONSE;
	}

	username = username_;

	if (!jsonOpt_ptr_->JsonParseModifyDeviceInfoVideoLink(device_guid, video_link_type, video_username, video_password, video_ip, video_port))
	{
		result_ = JSON_PARSE_ERROR;
		goto GET_RESPONSE;
	}

	//VerifyPasswordBeforeBind(device_guid, video_username, video_password, device_verify);
	if (device_verify == DEVICE_VERIFY_FAILED)
	{
		result_ = device_verify;
		goto GET_RESPONSE;
	}

	result_ = sqlOpt_ptr_->ModifyDeviceInfoVideoLinkToDB(username, device_guid, video_link_type, video_username, video_password, video_ip, video_port);

GET_RESPONSE:

	responseToDispatch_ = jsonOpt_ptr_->JsonJoinModifyDeviceInfoVideoLink(result_);
}

void CLogicOperate::UserBindDevice()
{
	std::string username;
	std::string device_guid;
	std::string video_username;
	std::string video_password;
	int reset_flag = 0;
	//int device_verify = 0;

	if (SessionNotExist())
	{
		goto GET_RESPONSE;
	}

	username = username_;

	if (!jsonOpt_ptr_->JsonParseUserBindDevice(device_guid, video_username, video_password))
	{
		result_ = JSON_PARSE_ERROR;
		goto GET_RESPONSE;
	}

	/*
	if ((result_ = VerifyDeviceExist(device_guid)) != SUCCESS)
	{
		goto GET_RESPONSE;
	}

	VerifyPasswordBeforeBind(device_guid, video_username, video_password, device_verify);
	if (device_verify == DEVICE_VERIFY_FAILED)
	{
		result_ = device_verify;
		goto GET_RESPONSE;
	}

	if ((reset_flag = GetDeviceResetFlag(device_guid)) == FAILED)
	{
		result_ = FAILED;
		goto GET_RESPONSE;
	}
	*/

	result_ = sqlOpt_ptr_->UserBindDeviceToDB(username, device_guid, video_username, video_password);

GET_RESPONSE:

	responseToDispatch_ = jsonOpt_ptr_->JsonJoinUserBindDevice(result_, reset_flag);
}

void CLogicOperate::UserRemoveBindDevice()
{
	std::string username;
	std::string device_guid;

	if (SessionNotExist())
	{
		goto GET_RESPONSE;
	}

	username = username_;

	if (!jsonOpt_ptr_->JsonParseUserRemoveBindDevice(device_guid))
	{
		result_ = JSON_PARSE_ERROR;
		goto GET_RESPONSE;
	}

	result_ = sqlOpt_ptr_->UserRemoveBindToDB(username, device_guid);

GET_RESPONSE:

	responseToDispatch_ = jsonOpt_ptr_->JsonJoinUserRemoveBindDevice(result_);
}

void CLogicOperate::GetDeviceHumiture()
{
	std::string username, device_guid;
	int getNum = 0;

	std::vector<HUMITURE_STAT_INFO> vec_humiture_stat_info;

	if (SessionNotExist())
	{
		goto GET_RESPONSE;
	}

	if (!jsonOpt_ptr_->JsonParseGetDeviceHumiture(device_guid, getNum))
	{
		result_ = JSON_PARSE_ERROR;
		goto GET_RESPONSE;
	}

	result_ = sqlOpt_ptr_->GetDeviceHumitureFromDB(device_guid, getNum, vec_humiture_stat_info);


GET_RESPONSE:
	responseToDispatch_ = jsonOpt_ptr_->JsonJoinGetDeviceHumiture(result_, vec_humiture_stat_info);
}

std::string CLogicOperate::GetDeviceTimestampFromRedisKey(const std::string& redisKey)
{
	unsigned i = redisKey.find_first_of('&',0);
	if (i != std::string::npos)
		return redisKey.substr(i + 1);
	else
		return "";
}

int CLogicOperate::GetLatestDeviceHumitureStatus(const std::string& device_guid, struct tm& timestamp, double& temperature, double& humidity)
{
	redisOpt_ptr_->SelectDB(DEVICE);

	std::string strTimestamp, strTem, strHum;
	std::string latestKey;

	if (!redisOpt_ptr_->Get(RedisKeyLatestDeviceHumitureKey(device_guid), latestKey))
	{
		LOG4CXX_ERROR(g_logger, "CLogicOperate::GetLatestDeviceHumitureStatus:Get error.");
		return REDIS_OPT_ERROR;
	}

	if (!redisOpt_ptr_->Hget(latestKey, REDIS_HASH_FEILD_TEMPERATURE, strTem))
	{
		LOG4CXX_ERROR(g_logger, "CLogicOperate::GetLatestDeviceHumitureStatus:Hget error.");
		return REDIS_OPT_ERROR;
	}
	unsigned index = strTem.find('.', 0);
	if (index != std::string::npos)
	{
		strTem = strTem.substr(0, index+3);
	}

	if (!redisOpt_ptr_->Hget(latestKey, REDIS_HASH_FEILD_HUMIDNESS, strHum))
	{
		LOG4CXX_ERROR(g_logger, "CLogicOperate::GetLatestDeviceHumitureStatus:Hget error.");
		return REDIS_OPT_ERROR;
	}
	index = strHum.find('.', 0);
	if (index != std::string::npos)
	{
		strHum = strHum.substr(0, index+3);
	}

	strTimestamp = GetDeviceTimestampFromRedisKey(latestKey);
	strptime(strTimestamp.c_str(), "%Y-%m-%d|%H:%M:%S", &timestamp);
	temperature = atof(strTem.c_str());
	humidity = atof(strHum.c_str());

	return SUCCESS;
}

void CLogicOperate::GetDeviceHumitureOntime()
{
	redisOpt_ptr_->SelectDB(DEVICE);

	std::string device_guid;

	DEVICE_ENV_INFO device_env_info;

	if (SessionNotExist())
	{
		goto GET_RESPONSE;
	}

	if (!jsonOpt_ptr_->JsonParseGetDeviceHumitureOntime(device_guid))
	{
		result_ = JSON_PARSE_ERROR;
		goto GET_RESPONSE;
	}
	
	device_env_info.device_guid = device_guid;
	GetLatestDeviceHumitureStatus(device_guid, device_env_info.timestamp, device_env_info.temperature, device_env_info.humidity);

GET_RESPONSE:

	responseToDispatch_ = jsonOpt_ptr_->JsonJoinGetDeviceHumitureOntime(result_, device_env_info);
}

void CLogicOperate::GetUserDevicesStatusInfo()
{
	redisOpt_ptr_->SelectDB(DEVICE);

	//DEVICES_LIST deviceList;
	USER_DEVICES_LIST user_devices_list;
	std::vector<std::string> vec_device;
	std::string username;
	int message_id = 0;
	std::vector<DEVICE_INFO_CLOUDSEE> vec_dinfo_cloudsee;
	char cloudsee_request[DATA_BUFFER_SIZE] = {0};
	char cloudsee_response[DATA_BUFFER_SIZE] = {0};
	uint32_t request_len = 0;

	if (SessionNotExist())
	{
		goto GET_RESPONSE;
	}

	username = username_;

	result_ = sqlOpt_ptr_->GetUserDevicesStatusInfoFromDB(username, vec_device, user_devices_list);
	if (result_ != SUCCESS)
	{
		goto GET_RESPONSE;
	}

	dataOpt_ptr_->CreateRequestDeviceInfo(message_id, vec_device, cloudsee_request, request_len);

	result_ = GetResponseFromCloudSee(message_id, cloudsee_request, request_len, cloudsee_response);
	if (result_ != SUCCESS)
	{
		LOG4CXX_ERROR(g_logger, "CLogicOperate::GetUserDevices:GetResponseFromCloudSee error.");
	}
	if (!dataOpt_ptr_->ParseResponseDeviceInfo(cloudsee_response, vec_dinfo_cloudsee))
	{
		LOG4CXX_ERROR(g_logger, "CLogicOperate::GetUserDevices:ParseResponseDeviceInfo error.");
		result_ = FAILED;
	}

	for (unsigned i = 0; i < user_devices_list.vec_user_device_info.size(); ++i)
	{
		GetDeviceOnlineStatusCloudSee(vec_dinfo_cloudsee, 
			user_devices_list.vec_user_device_info.at(i).device_info.device_guid, 
			user_devices_list.vec_user_device_info.at(i).device_info.online_status);
	}

	/* 刷新此用户的设备列表 */
	if (!vec_device.empty())
	{
		if (!redisOpt_ptr_->SAdd(RedisKeyDevicelist(username), vec_device))
		{
			result_ = REDIS_OPT_ERROR;
			goto GET_RESPONSE;
		}
	}

GET_RESPONSE:

	responseToDispatch_ = jsonOpt_ptr_->JsonJoinGetUserDevicesStatusInfo(result_, user_devices_list);
}

std::string CLogicOperate::GetDeviceGuidByRedisKey(const std::string& redisKey)
{
	unsigned i = redisKey.find_first_of(':',0);
	if (i != std::string::npos)
		return redisKey.substr(0, i);
	else
		return "";
}

int CLogicOperate::GetDeviceOnlineStatusByRedisKey(const std::string& redisKey)
{
	redisOpt_ptr_->SelectDB(DEVICE);

	int online_flag = OFFLINE;
	std::string str_online_flag;

	if (!redisOpt_ptr_->Get(redisKey, str_online_flag))
	{
		LOG4CXX_ERROR(g_logger, "COntimeThread::GetDeviceOnlineStatus:Get error.");
		return REDIS_OPT_ERROR;
	}
	online_flag = atoi(str_online_flag.c_str());

	return online_flag;
}

int CLogicOperate::GetOnlineDevicesGuid(std::vector<std::string>& vec_online_guid)
{
	redisOpt_ptr_->SelectDB(DEVICE);

	int ret = SUCCESS;

	std::vector<std::string> vec_keys;
	std::string keys_pattern = std::string("*") + ONLINE_FLAG;
	if (!redisOpt_ptr_->Keys(keys_pattern, vec_keys))
	{
		LOG4CXX_ERROR(g_logger, "CLogicOperate::GetOnlineDevicesGuid:Keys error.");
		return REDIS_OPT_ERROR;
	}

	std::string online_guid;
	for (unsigned i = 0; i < vec_keys.size(); ++i)
	{
		if (GetDeviceOnlineStatusByRedisKey(vec_keys.at(i)) == ONLINE)
		{
			online_guid = GetDeviceGuidByRedisKey(vec_keys.at(i));
			if (!online_guid.empty())
			{
				vec_online_guid.push_back(online_guid);
			}
		}
	}

	return ret;
}

void CLogicOperate::GetDeviceHumitureScore()
{
	redisOpt_ptr_->SelectDB(DEVICE);

	std::string device_guid;
	int top = 0;
	int ratio = 0;
	char strRatio[16] = {0};
	std::vector<std::string> vec_online_guid;
	std::vector<DEVICE_ENV_INFO> vec_device_env_info;
	DEVICE_ENV_INFO device_env_info;
	char timestamp[32] = {0};
	int device_num = 0;
	int last_score = 0;
	int last_top = 0;

	if (SessionNotExist())
	{
		goto GET_RESPONSE;
	}

	if (!jsonOpt_ptr_->JsonParseGetDeviceHumitureScore(device_guid))
	{
		result_ = JSON_PARSE_ERROR;
		goto GET_RESPONSE;
	}

	if ((result_ = GetOnlineDevicesGuid(vec_online_guid)) != SUCCESS)
	{
		LOG4CXX_ERROR(g_logger, "CLogicOperate::GetDeviceHumitureScore:GetOnlineDevicesGuid error.");
		goto GET_RESPONSE;
	}

	for (unsigned i = 0; i < vec_online_guid.size(); ++i)
	{
		memset(&device_env_info, 0, sizeof (DEVICE_ENV_INFO));
		device_env_info.device_guid = vec_online_guid.at(i);
		if (GetLatestDeviceHumitureStatus(vec_online_guid.at(i), device_env_info.timestamp, device_env_info.temperature, device_env_info.humidity) != SUCCESS)
		{
			if (vec_online_guid.at(i) == device_guid)
			{
				result_ = DEVICE_HUMITURE_NOT_EXIST;
				goto GET_RESPONSE;
			}
			else
			{
				continue;
			}
		}
		device_env_info.score = GetHumitureScore(device_env_info.temperature, device_env_info.humidity);
		device_env_info.assessment = GetHumitureAssessment(device_env_info.temperature, device_env_info.humidity);
		vec_device_env_info.push_back(device_env_info);
	}

	std::sort(vec_device_env_info.begin(), vec_device_env_info.end(), greater<DEVICE_ENV_INFO>());

	device_num = vec_device_env_info.size();
	for (int i = 0; i < device_num; ++i)
	{
		if (vec_device_env_info.at(i).device_guid == device_guid)
		{
			top = i + 1;
			ratio = ((device_num-(top-1))/(double)device_num) * 100;
			snprintf(strRatio, sizeof(strRatio), "%d%%", ratio);
			device_env_info = vec_device_env_info.at(i);
			
			strftime(timestamp, sizeof (timestamp), "%Y-%m-%d %H:%M:%S", &(device_env_info.timestamp));
			
			// 获取上次打分
			result_ = sqlOpt_ptr_->GetLastHumitureScoreFromDB(device_guid, last_score, last_top);
			// 将打分记录入库,提供历史评比
			sqlOpt_ptr_->DeviceHumitureScoreToDB(device_guid, timestamp, device_env_info.score, top);
		}
	}

GET_RESPONSE:

	responseToDispatch_ = jsonOpt_ptr_->JsonJoinGetDeviceHumitureScore(result_, top, strRatio, last_score, last_top, device_env_info);
}

int CLogicOperate::GetHumitureScore(const double temperature, const double humidity)
{
	double res = 0;
	double len_of_point2best_point = sqrt(pow(temperature-25,2)+pow(humidity-50,2));

	if( (humidity < 227.85 - 6.57 * temperature) && (humidity < 1019 - 37 * temperature) && (humidity > 1317.5 -55 * temperature) && (humidity > 58.57 - 1.43 * temperature))
	{
		res = 100 - (len_of_point2best_point/30)*10;
	}
	else
	{
		double temScore = 0;
		double humScore = 0;

		if(temperature < 15) 
			temScore = 40 - (15 - temperature) / 2 * 5;
		else if(temperature < 18) 
			temScore = 42.5 - (18 - temperature) / 3 * 2.5;
		else if(temperature < 24) 
			temScore = 45 - (24 - temperature) / 6 * 2.5;
		else if(temperature < 27) 
			temScore = 45 - (temperature - 24) / 3 * 2.5;
		else
			temScore = 42.5 - (temperature - 27) /8 * 42.5;

		if(humidity < 40) 
			humScore = humidity;
		else if(humidity < 45) 
			humScore = 42.5 - (45 - humidity) / 5 * 2.5;
		else if(humidity < 55) 
			humScore = 45 - (55 - humidity) / 10 * 2.5;
		else if(humidity < 65) 
			humScore = 45 - (humidity - 55) / 10 * 2.5;
		else if(humidity < 70) 
			humScore = 42.5 - (humidity - 65) / 5 * 2.5;
		else
			humScore = 40 - (humidity - 70) / 30 * 40;

		res = temScore + humScore;
	}

	return (int)res;
}

int CLogicOperate::GetHumitureAssessment(const double temperature, const double humidity)
{
	int res = 0;
	if(temperature < 22)
		res |= TEMPERATURE_TOO_LOW;
	if(temperature > 27)
		res |= TEMPERATURE_TOO_HIGH;
	if(humidity < 20)
		res |= HUMIDITY_TOO_LOW;
	if(humidity > 80)
		res |= HUMIDITY_TOO_HIGH;

	return res;
}

void CLogicOperate::ModifyDeviceConfInfo()
{
	std::string username;
	std::string device_guid;
	std::string device_name;	// 设备昵称
	std::string video_username;	// 云视通连接模式 用户名
	std::string video_password;	// 云视通连接模式 密码
	//int device_verify = 0;

	if (SessionNotExist())
	{
		goto GET_RESPONSE;
	}

	username = username_;

	if (!jsonOpt_ptr_->JsonParseModifyDeviceConfInfo(device_guid, device_name, video_username, video_password))
	{
		result_ = JSON_PARSE_ERROR;
		goto GET_RESPONSE;
	}

	/*
	VerifyPasswordBeforeBind(device_guid, video_username, video_password, device_verify);
	if (device_verify == DEVICE_VERIFY_FAILED)
	{
		result_ = device_verify;
		goto GET_RESPONSE;
	}
	*/

	result_ = sqlOpt_ptr_->ModifyDeviceConfInfoToDB(username, device_guid, device_name, video_username, video_password);

GET_RESPONSE:

	responseToDispatch_ = jsonOpt_ptr_->JsonJoinModifyDeviceConfInfo(result_);
}

void CLogicOperate::ModifyDeviceInfoAdvanced()
{
	std::string device_guid;
	int net_storage_switch = DEVICE_CONF_NOT_SET;
	int tf_storage_switch = DEVICE_CONF_NOT_SET;
	int alarm_switch = DEVICE_CONF_NOT_SET;
	std::string alarm_time;
	int baby_mode = DEVICE_CONF_NOT_SET;
	int full_alarm_mode = DEVICE_CONF_NOT_SET;

	if (SessionNotExist())
	{
		goto GET_RESPONSE;
	}

	if (!jsonOpt_ptr_->JsonParseModifyDeviceInfoAdvanced(device_guid, net_storage_switch, tf_storage_switch, 
		alarm_switch, alarm_time, baby_mode, full_alarm_mode))
	{
		result_ = JSON_PARSE_ERROR;
		goto GET_RESPONSE;
	}

	result_ = sqlOpt_ptr_->ModifyDeviceInfoAdvancedToDB(device_guid, net_storage_switch, tf_storage_switch, 
		alarm_switch, alarm_time, baby_mode, full_alarm_mode);

GET_RESPONSE:

	responseToDispatch_ = jsonOpt_ptr_->JsonJoinModifyDeviceInfoAdvanced(result_);
}

void CLogicOperate::GetDeviceUpdateInfo()
{
	std::string client_version;
	int device_type;
	int device_sub_type;
	UPDATE_FILE_INFO updateFileInfo;

	if (SessionNotExist())
	{
		goto GET_RESPONSE;
	}

	if(!jsonOpt_ptr_->JsonParseGetDeviceUpdateInfo(client_version, device_type, device_sub_type))
	{
		result_ = JSON_PARSE_ERROR;
		goto GET_RESPONSE;
	}

	result_ = sqlOpt_ptr_->GetUpdateInfo(device_type, device_sub_type, client_version, updateFileInfo);

GET_RESPONSE:
	responseToDispatch_ = jsonOpt_ptr_->JsonJoinGetDeviceUpdateInfo(result_, updateFileInfo);
}

void CLogicOperate::ModifyDevicePassword()
{
	std::string device_guid;
	std::string device_username, device_password;

	if (SessionNotExist())
	{
		goto GET_RESPONSE;
	}

	if (!jsonOpt_ptr_->JsonParseModifyDevicePassword(device_guid, device_username, device_password))
	{
		result_ = JSON_PARSE_ERROR;
		goto GET_RESPONSE;
	}

	// reset_flag=0 from here
	result_ = sqlOpt_ptr_->ModifyDevicePasswordToDB(username_, device_guid, device_username, device_password);

GET_RESPONSE:
	responseToDispatch_ = jsonOpt_ptr_->JsonJoinModifyDevicePassword(result_);
}

int CLogicOperate::GetDeviceResetFlag(const std::string& device_guid)
{
	int b_ok = FAILED;
	int reset_flag = 0;

	b_ok = sqlOpt_ptr_->GetDeviceResetFlagFromDB(device_guid, reset_flag);
	if (b_ok != SUCCESS)
	{
		LOG4CXX_ERROR(g_logger, "CLogicOperate::GetDeviceResetFlag:GetDeviceResetFlagFromDB error:" << b_ok);
		return b_ok;
	}

	return reset_flag;
}

int CLogicOperate::RemoveBindByDevice_HomeCloud(const std::string& device_guid)
{
	int b_ok = FAILED;

	b_ok = sqlOpt_ptr_->RemoveBindByDeviceToDB_HomeCloud(device_guid);

	return b_ok;
}

int CLogicOperate::RemoveBindByDeviceHoldUser_HomeCloud(const std::string& device_guid, const std::string& hold_user)
{
	int b_ok = FAILED;

	b_ok = sqlOpt_ptr_->RemoveBindByDeviceHoldUserToDB_HomeCloud(device_guid, hold_user);

	return b_ok;
}

int CLogicOperate::VerifyVedioAndDevicePassword(const std::string& username, const std::string& device_guid, int& verify_flag)
{
	int b_ok = FAILED;

	std::string device_username, device_password, video_username, video_password;

	b_ok = sqlOpt_ptr_->GetDevicePasswordFromDB(device_guid, device_username, device_password);
	if (b_ok != SUCCESS)
	{
		LOG4CXX_ERROR(g_logger, "CLogicOperate::VerifyVedioAndDevicePassword:GetDevicePasswordFromDB error:" << b_ok);
	}
	b_ok = sqlOpt_ptr_->GetUserDevicePasswordFromDB(username, device_guid, video_username, video_password);
	if (b_ok != SUCCESS)
	{
		LOG4CXX_ERROR(g_logger, "CLogicOperate::VerifyVedioAndDevicePassword:GetUserDevicePasswordFromDB error:" << b_ok);
	}

	LOG4CXX_TRACE(g_logger, "VerifyVedioAndDevicePassword device_username=" << device_username << ",device_password=" << device_password << ",video_username=" << video_username << ",video_password=" << video_password);
	if (device_username.compare(video_username) == 0 && device_password.compare(video_password) == 0)
	{
		verify_flag = SUCCESS;
	}
	else
	{
		verify_flag = DEVICE_VERIFY_FAILED;
	}

	return b_ok;
}

int CLogicOperate::VerifyPasswordBeforeBind(const std::string& device_guid, std::string& video_username, std::string& video_password, int& verify_flag)
{
	int b_ok = FAILED;

	std::string device_username, device_password;

	b_ok = sqlOpt_ptr_->GetDevicePasswordFromDB(device_guid, device_username, device_password);
	if (b_ok != SUCCESS)
	{
		LOG4CXX_ERROR(g_logger, "CLogicOperate::VerifyPasswordBeforeBind:GetDevicePasswordFromDB error:" << b_ok);
	}

	LOG4CXX_TRACE(g_logger, "VerifyPasswordBeforeBind device_username=" << device_username << ",device_password=" << device_password << ",video_username=" << video_username << ",video_password=" << video_password);
	if (device_username.compare(video_username) == 0 && device_password.compare(video_password) == 0)
	{
		verify_flag = SUCCESS;
	}
	else
	{
		verify_flag = DEVICE_VERIFY_FAILED;
	}

	return b_ok;
}

int CLogicOperate::VerifyDeviceExist(const std::string& device_guid)
{
	int ret = SUCCESS;

	ret = sqlOpt_ptr_->VerifyDeviceExistFromDB(device_guid);
	if (ret != SUCCESS)
	{
		LOG4CXX_ERROR(g_logger, "CLogicOperate::VerifyDeviceExist ret=" << ret);
	}
	return ret;
}

void CLogicOperate::AddDeviceChannel()
{
	std::string username;
	std::string device_guid;
	int add_sum = 0;
	std::string channel_name;

	if (SessionNotExist())
	{
		goto GET_RESPONSE;
	}

	username = username_;

	if (!jsonOpt_ptr_->JsonParseAddDeviceChannel(device_guid, add_sum, channel_name))
	{
		result_ = JSON_PARSE_ERROR;
		goto GET_RESPONSE;
	}

	if (add_sum > 64)
	{
		result_ = DEVICE_CHANNEL_LIMIT;
		goto GET_RESPONSE;
	}

	result_ = sqlOpt_ptr_->AddDeviceChannelToDB(username, device_guid, add_sum, channel_name);

GET_RESPONSE:
	responseToDispatch_ = jsonOpt_ptr_->JsonJoinAddDeviceChannel(result_);
}

void CLogicOperate::DeleteDeviceChannel()
{
	std::string username;
	std::string device_guid;
	int channel_no = 0;

	if (SessionNotExist())
	{
		goto GET_RESPONSE;
	}

	username = username_;

	if (!jsonOpt_ptr_->JsonParseDeleteDeviceChannel(device_guid, channel_no))
	{
		result_ = JSON_PARSE_ERROR;
		goto GET_RESPONSE;
	}

	result_ = sqlOpt_ptr_->DeleteDeviceChannelToDB(username, device_guid, channel_no);

GET_RESPONSE:
	responseToDispatch_ = jsonOpt_ptr_->JsonJoinDeleteDeviceChannel(result_);
}

void CLogicOperate::GetDeviceChannel()
{
	std::string username;
	std::string device_guid;
	std::vector<DEVICE_CHANNEL_INFO> vec_device_channel;

	if (SessionNotExist())
	{
		goto GET_RESPONSE;
	}

	username = username_;

	if (!jsonOpt_ptr_->JsonParseGetDeviceChannel(device_guid))
	{
		result_ = JSON_PARSE_ERROR;
		goto GET_RESPONSE;
	}

	result_ = sqlOpt_ptr_->GetDeviceChannelFromDB(username, device_guid, vec_device_channel);

GET_RESPONSE:
	responseToDispatch_ = jsonOpt_ptr_->JsonJoinGetDeviceChannel(result_, vec_device_channel);
}

void CLogicOperate::GetUserChannels()
{
	std::string username;
	std::vector<DEVICE_CHANNEL_INFO> vec_user_channels;

	if (SessionNotExist())
	{
		goto GET_RESPONSE;
	}

	username = username_;

	result_ = sqlOpt_ptr_->GetUserChannelsFromDB(username, vec_user_channels);

GET_RESPONSE:
	responseToDispatch_ = jsonOpt_ptr_->JsonJoinGetUserChannels(result_, vec_user_channels);
}

void CLogicOperate::ModifyDeviceChannelName()
{
	std::string username;
	std::string device_guid;
	int channel_no = 0;
	std::string channel_name;

	if (SessionNotExist())
	{
		goto GET_RESPONSE;
	}

	username = username_;

	if (!jsonOpt_ptr_->JsonParseModifyDeviceChannelName(device_guid, channel_no, channel_name))
	{
		result_ = JSON_PARSE_ERROR;
		goto GET_RESPONSE;
	}

	result_ = sqlOpt_ptr_->ModifyDeviceChannelNameToDB(username, device_guid, channel_no, channel_name);

GET_RESPONSE:
	responseToDispatch_ = jsonOpt_ptr_->JsonJoinModifyDeviceChannelName(result_);
}

void CLogicOperate::ModifyDeviceWifiFlag()
{
	std::string device_guid;
	int wifi_flag = 0;

	if (SessionNotExist())
	{
		goto GET_RESPONSE;
	}

	if (!jsonOpt_ptr_->JsonParseModifyDeviceWifiFlag(device_guid, wifi_flag))
	{
		result_ = JSON_PARSE_ERROR;
		goto GET_RESPONSE;
	}

	result_ = sqlOpt_ptr_->ModifyDeviceWifiFlagToDB(device_guid, wifi_flag);

GET_RESPONSE:
	responseToDispatch_ = jsonOpt_ptr_->JsonJoinModifyDeviceWifiFlag(result_);
}


int CLogicOperate::GetResponseFromCloudSee(const int message_id, const char* request, const uint32_t request_len, char* response)
{
	int ret = SUCCESS;

	NET_MSG net_msg;
	bzero(net_msg.response, DATA_BUFFER_SIZE);
	sem_t cond;
	ret = sem_init(&cond, 0, 0);
	if(0 != ret)
	{
		LOG4CXX_ERROR(g_logger, "CLogicOperate::GetResponseFromCloudSee:sem_init failed. errorcode = " << result_);
		return ret;
	}
	net_msg.h_event = &cond;
	g_map_message.insert(message_id, net_msg);

	if (!SendToCloudSeeServer(request, request_len))
	{
		LOG4CXX_WARN(g_logger, "CLogicOperate::GetResponseFromCloudSee:SendToCloudSeeServer failed.");
	}

	struct timespec timestruct = {0, 0};
	maketimeout(&timestruct, 3000);
	int dw = sem_timedwait(&cond, &timestruct);
	sem_destroy(&cond);
	switch(dw)
	{
	case 0:
		/** 回应 */
		FindResponseByMessageId(message_id, response);
		break;
	case ETIMEDOUT:
		LOG4CXX_WARN(g_logger, "CLogicOperate::GetResponseFromCloudSee TIMEOUT."<< ", message_id = " << message_id);
		ret = REQ_RES_TIMEOUT;
		break;
	default:
		LOG4CXX_ERROR(g_logger, "CLogicOperate::GetResponseFromCloudSee error. errorcode = " << strerror(errno) << ", message_id = " << message_id);
		ret = REQ_RES_OTHER_ERROR;
		break;
	}
	g_map_message.erase(message_id);

	return ret;
}

bool CLogicOperate::SendToCloudSeeServer(const char* request, const uint32_t request_len)
{
	LOG4CXX_TRACE(g_logger, "CLogicOperate::SendToCloudSeeServer:request len = " << request_len);

	return CCloudSeeTransport::GetInstance()->SendToCloudSeeServer(request, request_len);
}

void CLogicOperate::maketimeout(struct timespec *tsp, long milliseconds)
{
	struct timeval now;

	/* get current times */
	gettimeofday(&now, NULL);
	tsp->tv_sec = now.tv_sec;
	tsp->tv_nsec = now.tv_usec * 1000;
	tsp->tv_sec += (milliseconds/1000);
	tsp->tv_nsec += (milliseconds%1000) * 1000000;
}

void CLogicOperate::FindResponseByMessageId(const int message_id, char* response)
{
	NET_MSG netMsg;
	g_map_message.find(message_id, netMsg);
	memcpy(response, netMsg.response, DATA_BUFFER_SIZE);
}

void CLogicOperate::GetDeviceOnlineStatusCloudSee(const std::vector<DEVICE_INFO_CLOUDSEE>& vec_dinfo_cloudsee, const std::string& device_guid, int& online_flag)
{
	for (unsigned i = 0; i < vec_dinfo_cloudsee.size(); ++i)
	{
		if (vec_dinfo_cloudsee.at(i).device_guid == device_guid)
		{
			online_flag = vec_dinfo_cloudsee.at(i).online_flag;
			if (!(online_flag == 1 || online_flag == 0))
			{
				online_flag = 0;
			}
		}
	}
}
