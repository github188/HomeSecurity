/*
 * json_operate.cc
 *
 *  Created on: Mar 11, 2013
 *      Author: yaowei
 */

#include "json_opt.h"
#include "../public/message.h"
#include <vector>
#include <map>
#include "../public/utils.h"

CJsonOpt::CJsonOpt(const std::string& json_string)
{
	json_string_ = json_string;
	message_id_ = 0;
	sfd_ = 0;
	sfd_id_ = 0;
}
CJsonOpt::CJsonOpt()
{
	json_string_ = std::string();
	message_id_ = 0;
	sfd_ = 0;
	sfd_id_ = 0;
}
CJsonOpt::~CJsonOpt()
{
}

void CJsonOpt::setJsonString(const std::string &msg)
{
	out_ = JSONNode();
	json_string_ = msg;
}

bool CJsonOpt::JsonParseMessageType(int & message_type)
{

	try
	{
		in_ = libjson::parse(json_string_);
	}
	catch (...)
	{
		return false;
	}

	if(!VerifyCommonJson(in_))
		return false;

	message_type = in_[JK_MESSAGE_TYPE].as_int();
	message_id_	 = in_[JK_MESSAGE_ID].as_int();
	sfd_ 		 = in_[JK_CLINET_SFD].as_int();
	sfd_id_ 	 = in_[JK_CLINET_SFD_ID].as_int();

	return true;
}

bool CJsonOpt::JsonParseUserSessionId(std::string& session_id)
{
	if(VerifyJsonField(in_, JK_SESSION_ID))
	{
		session_id = in_[JK_SESSION_ID].as_string();
		return true;
	}

	return false;
}

void CJsonOpt::JsonJoinCommon(const int message_type, const int result)
{
	out_.push_back(JSONNode(JK_MESSAGE_TYPE, message_type));
	out_.push_back(JSONNode(JK_RESULT, result));
	out_.push_back(JSONNode(JK_MESSAGE_ID, message_id_));
	out_.push_back(JSONNode(JK_CLINET_SFD, sfd_));
	out_.push_back(JSONNode(JK_CLINET_SFD_ID, sfd_id_));
}


bool CJsonOpt::VerifyCommonJson(JSONNode& in)
{
	return VerifyJsonField(in, JK_MESSAGE_ID) && VerifyJsonField(in, JK_MESSAGE_TYPE)
		   && VerifyJsonField(in, JK_CLINET_SFD) && VerifyJsonField(in, JK_CLINET_SFD_ID);
}

bool CJsonOpt::VerifyJsonField(JSONNode& in, const std::string& field)
{
	try
	{
		return (in.end() != in.find(field));
	}
	catch (...)
	{
		return false;
	}
}

bool CJsonOpt::JsonParseGetAlarmInfo(int& idx_start, int& idx_stop)
{
	if(VerifyJsonField(in_, JK_ALARM_INDEX_START) && VerifyJsonField(in_, JK_ALARM_INDEX_STOP))
	{
		idx_start = in_[JK_ALARM_INDEX_START].as_int();
		idx_stop = in_[JK_ALARM_INDEX_STOP].as_int();
		return true;
	}

	return false;
}

std::string CJsonOpt::JsonJoinCommonOpt(const int message_type, const int result)
{
	JsonJoinCommon(message_type, result);
	return out_.write();
}

std::string CJsonOpt::JsonJoinGetAlarmInfo(const int result, std::vector< std::map<std::string, std::string> >& alarms)
{
	JsonJoinCommon(GET_ALARM_INFO_RESPONSE, result);

	std::vector< std::map<std::string, std::string> >::iterator iter;
	JSONNode alarmInfoArray(JSON_ARRAY);
	alarmInfoArray.set_name(JK_ALARM_INFO);
	for(iter=alarms.begin(); iter!=alarms.end(); iter++)
	{
		JSONNode alarmInfo;
		if(iter->count(JK_ALARM_GUID)){
			alarmInfo.push_back(JSONNode(JK_ALARM_GUID, iter->find(JK_ALARM_GUID)->second));
		}
		if(iter->count(JK_ALARM_SOLUTION)){
			alarmInfo.push_back(JSONNode(JK_ALARM_SOLUTION, atoi(iter->find(JK_ALARM_SOLUTION)->second.c_str())));
		}
		if(iter->count(JK_ALARM_STATUS)){
			alarmInfo.push_back(JSONNode(JK_ALARM_STATUS, atoi(iter->find(JK_ALARM_STATUS)->second.c_str())));
		}
		if(iter->count(JK_DEVICE_GUID)){
			alarmInfo.push_back(JSONNode(JK_DEVICE_GUID, iter->find(JK_DEVICE_GUID)->second));
		}
		if(iter->count(JK_DEVICE_NAME)){
			alarmInfo.push_back(JSONNode(JK_DEVICE_NAME, iter->find(JK_DEVICE_NAME)->second));
		}
		if(iter->count(JK_DEVICE_CHANNEL_NO)){
			alarmInfo.push_back(JSONNode(JK_DEVICE_CHANNEL_NO, atoi(iter->find(JK_DEVICE_CHANNEL_NO)->second.c_str())));
		}
		if(iter->count(JK_ALARM_TYPE)){
			alarmInfo.push_back(JSONNode(JK_ALARM_TYPE, atoi(iter->find(JK_ALARM_TYPE)->second.c_str())));
		}
		if(iter->count(JK_ALARM_PIC)){
			alarmInfo.push_back(JSONNode(JK_ALARM_PIC, iter->find(JK_ALARM_PIC)->second));
		}
		if(iter->count(JK_ALARM_VIDEO)){
			alarmInfo.push_back(JSONNode(JK_ALARM_VIDEO, iter->find(JK_ALARM_VIDEO)->second));
		}
		if(iter->count(JK_ALARM_TIMESTAMP)){
			alarmInfo.push_back(JSONNode(JK_ALARM_TIMESTAMP, atoi(iter->find(JK_ALARM_TIMESTAMP)->second.c_str())));
		}

		alarmInfoArray.push_back(alarmInfo);
	}
	out_.push_back(alarmInfoArray);
	std::string out_utf = out_.write();
	utils::Unicode2UTF8(out_utf);
	return out_utf;
}

bool CJsonOpt::JsonParseDelAlarmInfo(std::string& alarm_guid)
{
	if(!VerifyJsonField(in_,JK_ALARM_GUID))
	{
		return false;
	}

	guid = in_[JK_ALARM_GUID].as_string();
	return true;
}

