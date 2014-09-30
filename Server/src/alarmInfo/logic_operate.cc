/*
 * logic_operate.cc
 *
 *  Created on: Sep 30, 2014
 *      Author: wangyc
 */

#include "logic_operate.h"
#include "json_opt.h"
#include "redis_conn_pool.h"
#include "redis_opt.h"
#include "../public/utils.h"
#include "../public/message.h"
#include "../public/redis_key.h"
#include "../public/socket_wrapper.h"
#include "../public/user_interface_defines.h"

extern struct ClientInfo g_clientInfo;

CLogicOperate::CLogicOperate()
{
	jsonOpt_ptr_ = NULL;
	jsonOpt_ptr_ = new CJsonOpt();
	assert(jsonOpt_ptr_ != NULL);
	redisOpt_ptr_ = NULL;
	redisOpt_ptr_ = new CRedisOpt;
	assert(redisOpt_ptr_ != NULL);
	result_ = SUCCESS;
}

CLogicOperate::~CLogicOperate()
{
	utils::SafeDelete(jsonOpt_ptr_);
	utils::SafeDelete(redisOpt_ptr_);
}

void CLogicOperate::StartLogicOpt(const std::string& message)
{
	/** 恢复客户端进行的转义处理 */
	std::string string = utils::ReplaceString(message, "\\\\r\\\\n", "\\r\\n");

	jsonOpt_ptr_->setJsonString(string);
	int message_type = 0;
	if(!jsonOpt_ptr_->JsonParseMessageType(message_type))
	{
		LOG4CXX_ERROR(g_logger, "CLogicOperate::StartLogicOpt:JsonParseMessageType invalid :" << string);
		return;
	}

	LOG4CXX_TRACE(g_logger, "CLogicOperate::StartLogicOpt:json = " << string);

	redisContext* con = CRedisConnPool::GetInstance()->GetRedisContext();
	redisOpt_ptr_->SetRedisContext(con);
	redisOpt_ptr_->SelectDB(SESSION);

	result_ = SUCCESS;

	if(!jsonOpt_ptr_->JsonParseUserSessionId(session_id_))
	{
		result_ = JSON_PARSE_ERROR;
	}
	else
	{
		if (!redisOpt_ptr_->Get(session_id_, username_))
			result_ = SESSION_NOT_EXSIT;
	}

	switch(message_type)
	{
	case GET_ALARM_INFO:
		GetAlarmInfo();
		break;
	case DEL_ALARM_INFO:
		DelAlarmInfo();
		break;
	case CLEAN_ALARM_INFO:
		CleanAlarmInfo();
		break;
	default:
		LOG4CXX_WARN(g_logger, "CLogicOperate::StartLogicOpt:message_type invadlid :" << message_type);
		break;
	}

	SendToDispatch();
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
		LOG4CXX_WARN(g_logger, "CLogicOperate::SendToDispatch:sendto failed.");
	}
	LOG4CXX_TRACE(g_logger, "CLogicOperate::SendToDispatch:sendto = " << responseToDispatch_);
}

void CLogicOperate::GetAlarmInfo()
{
	std::string username;
	int idx_start = 0;
	int idx_stop = -1;
	std::vector<std::string> res;
	std::vector<std::string>::iterator iter;
	std::vector<std::string>::iterator i;
	std::vector< std::map<std::string, std::string> >alarms;

	if (SessionNotExist())
	{
		goto GET_RESPONSE;
	}

	username = username_;

	if (!jsonOpt_ptr_->JsonParseGetAlarmInfo(idx_start, idx_stop))
	{
		result_ = JSON_PARSE_ERROR;
		goto GET_RESPONSE;
	}

	if(!redisOpt_ptr_->SelectDB(ALARM_MSG))
	{
		LOG4CXX_ERROR(g_logger, "CLogicOperate::GetAlarmInfo:selectDB Error ");
		result_ = REDIS_OPT_ERROR;
		goto GET_RESPONSE;
	}

	if(redisOpt_ptr_->LRange(username, idx_start, idx_stop, res))
	{
		for(iter=res.begin(); iter!=res.end(); iter++)
		{
			std::vector<std::string> r = utils::SplitString(*iter, "|");
			LOG4CXX_DEBUG(g_logger, "al type " << r[1]);
			std::map<std::string, std::string> item;
			if(std::atoi(r[1].c_str()) == AS_LOCAL)
			{
				// alarm_guid|alarm_solution|device_guid|device_name|channel_no|alarm_type|pic_name|vedio_name|alarm_timestamp
				item[JK_ALARM_GUID] = r[0];
				item[JK_ALARM_SOLUTION] = r[1];
				item[JK_DEVICE_GUID] = r[2];
				item[JK_DEVICE_NAME] = r[3];
				item[JK_DEVICE_CHANNEL_NO] = r[4];
				item[JK_ALARM_TYPE] = r[5];
				item[JK_ALARM_PIC] = r[6];
				item[JK_ALARM_VIDEO] = r[7];
				item[JK_ALARM_TIMESTAMP] = r[8];
			}
			else if(std::atoi(r[1].c_str()) == AS_CLOUD)
			{
				// todo
				item[JK_ALARM_GUID] = r[0];
				item[JK_ALARM_SOLUTION] = r[1];
			}
			else if(std::atoi(r[1].c_str()) == AS_FTP)
			{
				// alarm_guid|alarm_solution|message_status|device_guid|device_name|channel_no|alarm_type|pic_name|vedio_name|alarm_timestamp
				item[JK_ALARM_GUID] = r[0];
				item[JK_ALARM_SOLUTION] = r[1];
				item[JK_ALARM_STATUS] = r[2];
				item[JK_DEVICE_GUID] = r[3];
				item[JK_DEVICE_NAME] = r[4];
				item[JK_DEVICE_CHANNEL_NO] = r[5];
				item[JK_ALARM_TYPE] = r[6];
				item[JK_ALARM_PIC] = r[7];
				item[JK_ALARM_VIDEO] = r[8];
				item[JK_ALARM_TIMESTAMP] = r[9];
			}
			else
			{
				LOG4CXX_ERROR(g_logger, "CLogicOperate::GetAlarmInfo:AlarmSolution Error ");
				continue;
			}
			alarms.push_back(item);
		}
	}
	else
	{
		result_ = REDIS_OPT_ERROR;
	}

GET_RESPONSE:
	responseToDispatch_ = jsonOpt_ptr_->JsonJoinGetAlarmInfo(result_, alarms);
}

void CLogicOperate::DelAlarmInfo()
{
	std::string username;
	std::string alarm_guid;
	std::vector<std::string> res;
	std::vector<std::string>::iterator iter;
	std::vector<std::string>::iterator i;

	username = username_;

	if(!jsonOpt_ptr_->JsonParseDelAlarmInfo(alarm_guid))
	{
		result_ = JSON_PARSE_ERROR;
		LOG4CXX_ERROR(g_logger, "CLogicOperate::DelAlarmInfo:JsonParseDelAlarmInfo invalid ");
		goto GET_RESPONSE;
	}
	
	redisOpt_ptr_->SelectDB(ALARM_MSG);

	if(redisOpt_ptr_->LRange(username, res))
	{
		for(iter=res.begin(); iter!=res.end(); iter++)
		{
			std::vector<string> r = utils::SplitString(*iter, "|");
			if(r[0] == alarm_guid)
			{
				redisOpt_ptr_->LRem(username, *iter);

				if(std::atoi(r[1].c_str()) == AS_LOCAL){

				}else if(std::atoi(r[1].c_str()) == AS_CLOUD){

				}else if(std::atoi(r[1].c_str()) == AS_FTP){

				}else{
					LOG4CXX_ERROR(g_logger, "CLogicOperate::DelAlarmInfo:AlarmSolution Error ");
				}
			}
			LOG4CXX_DEBUG(g_logger, "CLogicOperate::DelAlarmInfo:LRange " << *iter);
		}

		goto GET_RESPONSE;
	}


GET_RESPONSE:
	responseToDispatch_ = jsonOpt_ptr_->JsonJoinCommonOpt(DEL_ALARM_INFO_RESPONSE, result_);
}

void CLogicOperate::CleanAlarmInfo()
{
	std::string username;
	username = username_;

	redisOpt_ptr_->SelectDB(ALARM_MSG);

}
