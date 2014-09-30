/*
 * json_pt.cc
 *
 *  Created on: Mar 6, 2013
 *      Author: yaowei
 */

#include "json_opt.h"
#include "../public/message.h"
#include "../public/utils.h"

CJsonOpt::CJsonOpt()
{
	message_id_ = 0;
}

CJsonOpt::~CJsonOpt()
{
}

bool CJsonOpt::ParseMessageLogicType(const char* json_in_string, int& logic_type, int& message_type, std::string& session_id)
{
	try
	{
		in_ = libjson::parse(json_in_string);
	}
	catch (...)
	{
		return false;
	}

	if (!VerifyPublicJson(in_))
	{
		return false;
	}

	logic_type 	= in_[JK_LOGIC_PROCESS_TYPE].as_int();
	message_type= in_[JK_MESSAGE_TYPE].as_int();
	session_id	= in_[JK_SESSION_ID].as_string();
	message_id_ = in_[JK_MESSAGE_ID].as_int();

	return true;
}


bool CJsonOpt::VerifyPublicJson(JSONNode & in)
{
	return VerifyJsonField(in, JK_LOGIC_PROCESS_TYPE)&&VerifyJsonField(in, JK_PROTO_VERSION) &&
		   VerifyJsonField(in, JK_MESSAGE_TYPE) && VerifyJsonField(in, JK_SESSION_ID) &&
		   VerifyJsonField(in, JK_MESSAGE_ID);
}


bool CJsonOpt::VerifyJsonField(const JSONNode & in, const std::string & field)
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

std::string CJsonOpt::JsonJoinCommon(const int message_type, const int result)
{
	JsonJoinPublic(message_type, result);
	return out_.write();
}

std::string CJsonOpt::JsonJoinNotifyOffline(const std::string& username, const int online_server_no, const int sfd, const int sfd_id)
{
	JSONNode out;
	out.push_back(JSONNode(JK_USERNAME, username));
	out.push_back(JSONNode(JK_MESSAGE_TYPE, NOTIFY_OFFLINE));
	out.push_back(JSONNode(JK_LOGIC_PROCESS_TYPE, IM_SERVER_RELAY));
	out.push_back(JSONNode(JK_P2RELAY_MESSAGE_TYPE, RELAY_NOTIFY_OFFLINE));
	out.push_back(JSONNode(JK_ONLINE_SERVER_NO, online_server_no));
	out.push_back(JSONNode(JK_ONLINE_SERVER_FD, sfd));
	out.push_back(JSONNode(JK_ONLINE_SERVER_FD_ID, sfd_id));
	return out.write();
}

void CJsonOpt::JsonJoinPublic(const int message_type, const int result)
{
	out_.push_back(JSONNode(JK_MESSAGE_TYPE, message_type));
	out_.push_back(JSONNode(JK_RESULT, result));
	out_.push_back(JSONNode(JK_MESSAGE_ID, message_id_));
}

bool CJsonOpt::JsonParseSetOnlineStatus(int& online_status)
{
	if(!VerifyJsonField(in_, JK_USER_ONLINE_STATUS))
		return false;

	online_status = in_[JK_USER_ONLINE_STATUS].as_int();

	return true;
}

bool CJsonOpt::JsonParseNotifyOffline(int& server_no, int& sfd, int& sfd_id)
{
	if(!(VerifyJsonField(in_, JK_ONLINE_SERVER_NO) && VerifyJsonField(in_, JK_ONLINE_SERVER_FD)
		&& VerifyJsonField(in_, JK_ONLINE_SERVER_FD_ID)))
	{
		return false;
	}

	server_no = in_[JK_ONLINE_SERVER_NO].as_int();
	sfd = in_[JK_ONLINE_SERVER_FD].as_int();
	sfd_id = in_[JK_ONLINE_SERVER_FD_ID].as_int();

	return true;
}

bool CJsonOpt::JsonParseR2PMessageType(const std::string& message, int& im2relay_type)
{
	try
	{
		in_ = libjson::parse(message);
	}
	catch (...)
	{
		return false;
	}

	if(!VerifyJsonField(in_, JK_P2RELAY_MESSAGE_TYPE))
	{
		return false;
	}

	im2relay_type = in_[JK_P2RELAY_MESSAGE_TYPE].as_int();

	return true;
}

bool CJsonOpt::VerifyC2DRelayMessage()
{
	if (!(VerifyJsonField(in_, JK_USERNAME) && VerifyJsonField(in_, JK_DEVICE_GUID)))
		return false;
	return true;
}

bool CJsonOpt::JsonParseUsername(std::string& username)
{
	if(!VerifyJsonField(in_, JK_USERNAME))
		return false;

	username = in_[JK_USERNAME].as_string();

	return true;
}

std::string CJsonOpt::RestructMessageToUser(const std::string& message)
{
	JSONNode in;
	try
	{
		in = libjson::parse(message);
	}
	catch (...)
	{
		return false;
	}

	if(VerifyJsonField(in, JK_MESSAGE_ID))
		in.pop_back(JK_MESSAGE_ID);

	return in.write();
}

std::string CJsonOpt::RestructIm2RelayMessage(const int relay_type)
{
	unsigned long timestamp = time(NULL);
	in_.push_back(JSONNode(JK_P2RELAY_MESSAGE_TYPE, relay_type));
	in_.push_back(JSONNode(JK_RELAY_MESSAGE_TIMESTAMP, utils::toString(timestamp)));
	return in_.write();
}

bool CJsonOpt::JsonParseRelay2ImMessage(const std::string& message, std::string& username)
{
	try
	{
		in_ = libjson::parse(message);
	}
	catch (...)
	{
		return false;
	}

	if(!VerifyJsonField(in_, JK_USERNAME))
		return false;

	username = in_[JK_USERNAME].as_string();

	return true;
}

std::string CJsonOpt::RestructDeviceModifyResult2Client(const std::string& message)
{
	JSONNode in;
	JSONNode out;
	try
	{
		in = libjson::parse(message);
	}
	catch (...)
	{
		LOG4CXX_ERROR(g_logger, "CJsonOpt::RestructDeviceModifyResult2Client:parse error.");
		return "";
	}

	if(!(VerifyJsonField(in, JK_MESSAGE_TYPE) && VerifyJsonField(in, JK_DEVICE_GUID)))
	{
		LOG4CXX_ERROR(g_logger, "CJsonOpt::RestructDeviceModifyResult2Client:VerifyJsonField error.");
		return "";
	}

	out.push_back(in[JK_MESSAGE_TYPE]);
	out.push_back(in[JK_DEVICE_GUID]);
	out.push_back(in[JK_MESSAGE_ID]);

	if (VerifyJsonField(in, JK_DEVICE_NAME_RESULT))
	{
		out.push_back(in[JK_DEVICE_NAME_RESULT]);
	}
	if (VerifyJsonField(in, JK_NET_STORAGE_SWITCH_RESULT))
	{
		out.push_back(in[JK_NET_STORAGE_SWITCH_RESULT]);
	}
	if (VerifyJsonField(in, JK_TF_STORAGE_SWITCH_RESULT))
	{
		out.push_back(in[JK_TF_STORAGE_SWITCH_RESULT]);
	}
	if (VerifyJsonField(in, JK_ALARM_SWITCH_RESULT))
	{
		out.push_back(in[JK_ALARM_SWITCH_RESULT]);
	}
	if (VerifyJsonField(in, JK_ALARM_TIME_RESULT))
	{
		out.push_back(in[JK_ALARM_TIME_RESULT]);
	}
	if (VerifyJsonField(in, JK_PIC_UPLOAD_TIMEING_RESULT))
	{
		out.push_back(in[JK_PIC_UPLOAD_TIMEING_RESULT]);
	}

	return out.write();
}

bool CJsonOpt::JsonParsePushC2DMessageId(const std::string& username, int& message_id, std::string& device_guid)
{
	if (!(VerifyJsonField(in_, JK_DEVICE_GUID) && VerifyJsonField(in_, JK_MESSAGE_ID)))
		return false;

	message_id = in_[JK_MESSAGE_ID].as_int();
	device_guid = in_[JK_DEVICE_GUID].as_string();

	if (VerifyJsonField(in_, JK_USERNAME))
	{
		in_.pop_back(JK_USERNAME);
	}
	in_.push_back(JSONNode(JK_USERNAME, username));

	return true;
}

bool CJsonOpt::JsonParsePushDeviceModifyInfoResult(const std::string& in_json_string, int& result)
{
	result = 0;

	JSONNode in;
	try
	{
		in = libjson::parse(in_json_string);
	}
	catch(...)
	{
		LOG4CXX_ERROR(g_logger, "CJsonOpt::JsonParsePushDeviceModifyInfoResult invalid argument.");
		return false;
	}

	if (VerifyJsonField(in, JK_DEVICE_NAME_RESULT))
	{
		result = in[JK_DEVICE_NAME_RESULT].as_int();
		if (result != 0)
		{
			return true;
		}
	}
	if (VerifyJsonField(in, JK_NET_STORAGE_SWITCH_RESULT))
	{
		result = in[JK_NET_STORAGE_SWITCH_RESULT].as_int();
		if (result != 0)
		{
			return true;
		}
	}
	if (VerifyJsonField(in, JK_TF_STORAGE_SWITCH_RESULT))
	{
		result = in[JK_TF_STORAGE_SWITCH_RESULT].as_int();
		if (result != 0)
		{
			return true;
		}
	}
	if (VerifyJsonField(in, JK_ALARM_SWITCH_RESULT))
	{
		result = in[JK_ALARM_SWITCH_RESULT].as_int();
		if (result != 0)
		{
			return true;
		}
	}
	if (VerifyJsonField(in, JK_ALARM_TIME_RESULT))
	{
		result = in[JK_ALARM_TIME_RESULT].as_int();
		if (result != 0)
		{
			return true;
		}
	}
	if (VerifyJsonField(in, JK_PIC_UPLOAD_TIMEING_RESULT))
	{
		result = in[JK_PIC_UPLOAD_TIMEING_RESULT].as_int();
		if (result != 0)
		{
			return true;
		}
	}

	return true;
}

bool CJsonOpt::JsonParseMessageId(const std::string& in_json_string, int& message_id)
{
	JSONNode in;
	try
	{
		in = libjson::parse(in_json_string);
	}
	catch(...)
	{
		LOG4CXX_ERROR(g_logger, "CJsonOpt::JsonParseMessageId invalid argument");
		return false;
	}

	if (!VerifyJsonField(in, JK_MESSAGE_ID))
	{
		return false;
	}

	message_id = in[JK_MESSAGE_ID].as_int();

	return true;
}

bool CJsonOpt::JsonParsePushResult(const std::string& in_json_string, int& result)
{
	result = 0;

	JSONNode in;
	try
	{
		in = libjson::parse(in_json_string);
	}
	catch(...)
	{
		LOG4CXX_ERROR(g_logger, "CJsonOpt::JsonParsePushDeviceModifyInfoResult invalid argument.");
		return false;
	}

	if (VerifyJsonField(in, JK_RESULT))
	{
		result = in[JK_RESULT].as_int();
		return true;
	}
	else
	{
		LOG4CXX_ERROR(g_logger, "CJsonOpt::JsonParsePushResult JK_RESULT Verify error.");
		return false;
	}
}

std::string CJsonOpt::RestructPush2CCommon(const std::string& message)
{
	JSONNode in;
	JSONNode out;
	try
	{
		in = libjson::parse(message);
	}
	catch (...)
	{
		LOG4CXX_ERROR(g_logger, "CJsonOpt::RestructPush2CCommon:parse error.");
		return "";
	}

	if(!(VerifyJsonField(in, JK_MESSAGE_TYPE) && VerifyJsonField(in, JK_DEVICE_GUID)))
	{
		LOG4CXX_ERROR(g_logger, "CJsonOpt::RestructPush2CCommon:VerifyJsonField error.");
		return "";
	}

	out.push_back(in[JK_MESSAGE_TYPE]);
	out.push_back(in[JK_DEVICE_GUID]);
	out.push_back(in[JK_MESSAGE_ID]);
	out.push_back(in[JK_RESULT]);

	return out.write();
}

bool CJsonOpt::JsonParseGetDeviceUpdateStepResult(const std::string& in_json_string, int& result, int& downloadstep, int& writestep)
{
	result = 0;

	JSONNode in;
	try
	{
		in = libjson::parse(in_json_string);
	}
	catch(...)
	{
		LOG4CXX_ERROR(g_logger, "CJsonOpt::JsonParseGetDeviceUpdateStepResult invalid argument.");
		return false;
	}

	if (VerifyJsonField(in, JK_RESULT))
	{
		result = in[JK_RESULT].as_int();
		if (VerifyJsonField(in, JK_UPGRADE_DOWNLOAD_STEP))
		{
			downloadstep = in[JK_UPGRADE_DOWNLOAD_STEP].as_int();
		}
		if (VerifyJsonField(in, JK_UPGRADE_WRITE_STEP))
		{
			writestep = in[JK_UPGRADE_WRITE_STEP].as_int();
		}
		
		return true;
	}
	else
	{
		LOG4CXX_ERROR(g_logger, "CJsonOpt::JsonParseGetDeviceUpdateStepResult JK_RESULT Verify error.");
		return false;
	}
}

std::string CJsonOpt::JsonJoinGetUpdateDownloadStep(const int result, const int& downloadstep)
{
	JsonJoinPublic(GET_UPDATE_DOWNLOAD_STEP_RESPONSE, result);

	out_.push_back(JSONNode(JK_UPGRADE_DOWNLOAD_STEP, downloadstep));

	return out_.write();
}

std::string CJsonOpt::JsonJoinGetUpdateWriteStep(const int result, const int& writestep)
{
	JsonJoinPublic(GET_UPDATE_WRITE_STEP_RESPONSE, result);

	out_.push_back(JSONNode(JK_UPGRADE_WRITE_STEP, writestep));

	return out_.write();
}

std::string CJsonOpt::RestructDeviceUpdateStepResult2Client(const std::string& message)
{
	JSONNode in;
	JSONNode out;
	try
	{
		in = libjson::parse(message);
	}
	catch (...)
	{
		LOG4CXX_ERROR(g_logger, "CJsonOpt::RestructGetDeviceUpdateStepResult2Client:parse error.");
		return "";
	}

	if(!(VerifyJsonField(in, JK_RESULT) && VerifyJsonField(in, JK_MESSAGE_TYPE) && VerifyJsonField(in, JK_DEVICE_GUID)))
	{
		LOG4CXX_ERROR(g_logger, "CJsonOpt::RestructGetDeviceUpdateStepResult2Client:VerifyJsonField error.");
		return "";
	}

	out.push_back(in[JK_RESULT]);
	out.push_back(in[JK_MESSAGE_TYPE]);
	out.push_back(in[JK_DEVICE_GUID]);
	out.push_back(in[JK_MESSAGE_ID]);

	if (VerifyJsonField(in, JK_UPGRADE_DOWNLOAD_STEP))
	{
		out.push_back(in[JK_UPGRADE_DOWNLOAD_STEP]);
	}
	if (VerifyJsonField(in, JK_UPGRADE_WRITE_STEP))
	{
		out.push_back(in[JK_UPGRADE_WRITE_STEP]);
	}

	return out.write();
}

std::string CJsonOpt::RestructAlarmMessageToUser(const std::string& message)
{
	JSONNode json_message;
	try
	{
		json_message = libjson::parse(message);
	}
	catch (...)
	{
		LOG4CXX_ERROR(g_logger, "CJsonOpt::RestructAlarmMessageToUser:parse error.");
		return "";
	}

	if (VerifyJsonField(json_message, JK_LOGIC_PROCESS_TYPE))
		json_message.pop_back(JK_LOGIC_PROCESS_TYPE);
	if (VerifyJsonField(json_message, JK_ONLINE_SERVER_NO))
		json_message.pop_back(JK_ONLINE_SERVER_NO);
	if (VerifyJsonField(json_message, JK_ONLINE_SERVER_FD))
		json_message.pop_back(JK_ONLINE_SERVER_FD);
	if (VerifyJsonField(json_message, JK_ONLINE_SERVER_FD_ID))
		json_message.pop_back(JK_ONLINE_SERVER_FD_ID);

	return json_message.write();
}
