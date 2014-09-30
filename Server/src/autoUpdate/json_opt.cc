/*
 * json_operate.cc
 *
 *  Created on: Mar 11, 2013
 *      Author: yaowei
 */

#include "json_opt.h"
#include "../public/message.h"

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

std::string CJsonOpt::JsonJoinCommonOpt(const int message_type, const int result)
{
	JsonJoinCommon(message_type, result);
	return out_.write();
}

bool CJsonOpt::JsonParseGetLastSoftVersion(std::string& current_version)
{
	if(!VerifyJsonField(in_,JK_SOFT_VERSION))
	{
		return false;
	}

	current_version = in_[JK_SOFT_VERSION].as_string();

	return true;
}

std::string CJsonOpt::JsonJoinGetLastSoftVersion(const int message_type, const int result, const SoftVersionInfo& last_versionInfo)
{
	JsonJoinCommon(message_type, result);

	out_.push_back(JSONNode(JK_SOFT_VERSION,last_versionInfo.soft_version));
	out_.push_back(JSONNode(JK_SOFT_VERSION_ADDR,last_versionInfo.soft_version_addr));
	out_.push_back(JSONNode(JK_SOFT_VERSION_DESP,last_versionInfo.soft_version_desp));

	return out_.write();
}
