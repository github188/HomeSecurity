/*
 * json_pt.cc
 *
 *  Created on: Mar 6, 2013
 *      Author: yaowei
 */

#include "json_opt.h"
#include "../public/message.h"

CJsonOpt::CJsonOpt()
{
	message_id_ = 0;
}

CJsonOpt::~CJsonOpt()
{
}

bool CJsonOpt::VerifyJsonField(const JSONNode & in, const std::string & field)
{
	try
		{
			return (in.end()!= in.find(field));
		}
		catch(...)
		{
			// LOG4CXX_ERROR(g_logger, "json异常");
			return false;
		}
}

bool CJsonOpt::JsonParseLogicType(const std::string& message, int& logic_type)
{
	try
	{
		in_ = libjson::parse(message.c_str());
	}
	catch (...)
	{
		return false;
	}

	if (!VerifyJsonField(in_, JK_LOGIC_PROCESS_TYPE))
		return false;

	logic_type = in_[JK_LOGIC_PROCESS_TYPE].as_int();

	if(VerifyJsonField(in_, JK_MESSAGE_ID) && VerifyJsonField(in_, JK_PROTO_VERSION))
	{
		message_id_ = in_[JK_MESSAGE_ID].as_int();
		protocol_version_ = in_[JK_PROTO_VERSION].as_string();
	}

	return true;
}

bool CJsonOpt::JsonParseP2RelayMessageType(int& p2relay_message_type)
{

	if(!VerifyJsonField(in_, JK_P2RELAY_MESSAGE_TYPE))
		return false;

	p2relay_message_type = in_[JK_P2RELAY_MESSAGE_TYPE].as_int();

	return true;
}

void CJsonOpt::JsonJoinCommon(const int p2rmessage_type, const int result)
{
	out_.push_back(JSONNode(JK_MESSAGE_ID, message_id_));
	out_.push_back(JSONNode(JK_PROTO_VERSION, protocol_version_));
	out_.push_back(JSONNode(JK_P2RELAY_MESSAGE_TYPE, p2rmessage_type));
	out_.push_back(JSONNode(JK_RESULT, result));
}

bool CJsonOpt::JsonParseUserServerNo(int& server_no)
{
	if (!VerifyJsonField(in_, JK_ONLINE_SERVER_NO))
		return false;

	server_no = in_[JK_ONLINE_SERVER_NO].as_int();
	return true;
}

std::string CJsonOpt::RestructToImServerMessage(const int online_server_no, const int sfd, const int sfd_id)
{
	in_.push_back(JSONNode(JK_ONLINE_SERVER_NO, online_server_no));
	in_.push_back(JSONNode(JK_ONLINE_SERVER_FD, sfd));
	in_.push_back(JSONNode(JK_ONLINE_SERVER_FD_ID, sfd_id));
//	if(VerifyJsonField(in_, JK_MESSAGE_ID))
//		in_.pop_back(JK_MESSAGE_ID);

	return in_.write();
}

bool CJsonOpt::JsonParseUsername(std::string& username)
{
	if (!VerifyJsonField(in_, JK_USERNAME))
		return false;

	username = in_[JK_USERNAME].as_string();
	return true;
}

std::string CJsonOpt::JsonJoinHeatbeatDetectResponse(const int result)
{
	JsonJoinCommon(HEATBEAT_DETECT_RESPONSE, result);
	return out_.write();
}

std::string CJsonOpt::JsonJoinGetAccountLiveInfo(const int result, const ACCOUNT_LIVE_INFO& accountLiveInfo)
{
	JsonJoinCommon(GET_ACCOUNT_LIVE_INFO_RESPONSE, result);

	JSONNode out_out;
	out_out.set_name(JK_ACCOUNT_LIVE_INFO);
	out_out.push_back(JSONNode(JK_USERNAME, accountLiveInfo.username));
	out_out.push_back(JSONNode(JK_USER_ONLINE_STATUS, accountLiveInfo.online_status));
	out_out.push_back(JSONNode(JK_CLIENT_LOGIN_PLATFORM, accountLiveInfo.login_platform));
	out_out.push_back(JSONNode(JK_LOGIN_MOBILE_ID, accountLiveInfo.moblie_id));
	out_out.push_back(JSONNode(JK_LANGUAGE_TYPE, accountLiveInfo.language_type));
	out_out.push_back(JSONNode(JK_ALARM_FLAG, accountLiveInfo.alarm_flag));
	out_.push_back(out_out);

	return out_.write();
}

bool CJsonOpt::JsonParseIm2RelayDeviceGUID(std::string& device_guid)
{
	if(!VerifyJsonField(in_, JK_DEVICE_GUID))
	{
		return false;
	}

	device_guid 	= in_[JK_DEVICE_GUID].as_string();
	return true;
}

std::string CJsonOpt::JsonJoinSendMessageToUser(const int result)
{
	JsonJoinCommon(SEND_MESSAGE_TO_USER_RESPONSE, result);
	return out_.write();
}
