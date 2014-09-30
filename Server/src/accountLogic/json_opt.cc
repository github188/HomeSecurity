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

bool CJsonOpt::JsonParseUsername(std::string& username)
{
	if(!VerifyJsonField(in_, JK_USERNAME))
		return false;

	username = in_[JK_USERNAME].as_string();
	return true;
}

bool CJsonOpt::JsonParseUserRegister(std::string& username, std::string& password, std::string& custom_type)
{

	if (!(VerifyJsonField(in_, JK_USERNAME) && VerifyJsonField(in_, JK_PASSWORD)))
		return false;

	if(!VerifyJsonField(in_, JK_CUSTOM_TYPE))
	{
		custom_type = "";
	}
	else
	{
		custom_type = in_[JK_CUSTOM_TYPE].as_string();
	}

	username = in_[JK_USERNAME].as_string();
	password = in_[JK_PASSWORD].as_string();

	return true;
}

bool CJsonOpt::JsonParseSetAccountInfo(AccountInfo& account_info)
{
	if (!(VerifyJsonField(in_, JK_MAIL)&& VerifyJsonField(in_, JK_PHONE) && VerifyJsonField(in_, JK_NICKNAME)))
	{
		return false;
	}

	account_info.mail = in_[JK_MAIL].as_string();
	account_info.phone = in_[JK_PHONE].as_string();
	account_info.nickname = in_[JK_NICKNAME].as_string();

	return true;
}

std::string CJsonOpt::JsonJoinGetAccountMailWithNoSession(const int message_type, const int result, const std::string& mail)
{
	JsonJoinCommon(message_type, result);
	out_.push_back(JSONNode(JK_MAIL,mail));

	return out_.write();
}

bool CJsonOpt::JsonParseUserLogin(std::string& username, std::string& password)
{
	if (!(VerifyJsonField(in_, JK_USERNAME) && VerifyJsonField(in_, JK_PASSWORD)))
		return false;

	username = in_[JK_USERNAME].as_string();
	password = in_[JK_PASSWORD].as_string();

	return true;
}

bool CJsonOpt::JsonParseModifyUserName(std::string& username,std::string& password)
{
	if (!(VerifyJsonField(in_, JK_PASSWORD) && VerifyJsonField(in_, JK_NEW_USERNAME)))
		return false;

	username = in_[JK_NEW_USERNAME].as_string();
	password = in_[JK_PASSWORD].as_string();

	return true;
}

std::string CJsonOpt::JsonJoinUserLogin(const int result, const std::string& session_id)
{
	out_.push_back(JSONNode(JK_SESSION_ID, session_id));
	JsonJoinCommon(LOGIN_RESPONSE, result);
	return out_.write();
}

bool CJsonOpt::JsonParseMailOrPhone(std::string& mailOrPhone)
{
	if(!VerifyJsonField(in_, JK_MAIL_OR_PHONE))
		return false;

	mailOrPhone = in_[JK_MAIL_OR_PHONE].as_string();

	return true;
}

bool CJsonOpt::JsonParseReportClientPlatformInfo(CLIENT_PLATFORM_INFO& clientPlatformInfo)
{
	if(!(VerifyJsonField(in_, JK_TERMINAL_TYPE) && VerifyJsonField(in_, JK_LANGUAGE_TYPE)
	  && VerifyJsonField(in_, JK_LOGIN_MOBILE_ID)))
		return false;

	clientPlatformInfo.terminal_type = in_[JK_TERMINAL_TYPE].as_int();
	clientPlatformInfo.language_type = in_[JK_LANGUAGE_TYPE].as_int();
	clientPlatformInfo.moblie_id	 = in_[JK_LOGIN_MOBILE_ID].as_string();

	return true;
}

std::string CJsonOpt::JsonJoinGetCurrentAlarmFlag(const int result, const int alarm_flag)
{
	JsonJoinCommon(GET_ALARM_FLAG_RESPONSE, result);
	out_.push_back(JSONNode(JK_ALARM_FLAG, alarm_flag));
	return out_.write();
}

bool CJsonOpt::JsonParseSetCurrentAlarmFlag(int& alarm_flag, std::string& mobile_id)
{
	if(!(VerifyJsonField(in_, JK_ALARM_FLAG) && VerifyJsonField(in_, JK_LOGIN_MOBILE_ID)))
		return false;

	alarm_flag = in_[JK_ALARM_FLAG].as_int();
	mobile_id = in_[JK_LOGIN_MOBILE_ID].as_string();
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

void CJsonOpt::setJsonString(const std::string &msg)
{
	out_ = JSONNode();
	json_string_ = msg;
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

bool CJsonOpt::JsonParseUserPassword(std::string& password)
{
	if(!VerifyJsonField(in_, JK_PASSWORD))
		return false;

	password = in_[JK_PASSWORD].as_string();

	return true;
}

bool CJsonOpt::JsonParseModifyUserPassword(std::string& password, std::string& new_password)
{
	if (!(VerifyJsonField(in_, JK_PASSWORD) && VerifyJsonField(in_, JK_NEW_PASSWORD)))
			return false;

	password = in_[JK_PASSWORD].as_string();
	new_password = in_[JK_NEW_PASSWORD].as_string();

	return true;
}

bool CJsonOpt::JsonParseSendFeedbackToMail(std::string& feedback, std::string& mail_address)
{
	if(!(VerifyJsonField(in_, JK_FEEDBACK) && VerifyJsonField(in_, JK_SECURITY_MAIL)))
		return false;

	feedback = in_[JK_FEEDBACK].as_string();
	mail_address = in_[JK_SECURITY_MAIL].as_string();

	return true;
}

bool CJsonOpt::JsonParseSendToSecurityMail(std::string& username, std::string& security_mail)
{
	if (VerifyJsonField(in_, JK_USERNAME) && VerifyJsonField(in_, JK_SECURITY_MAIL))
	{
		username = in_[JK_USERNAME].as_string();
		security_mail = in_[JK_SECURITY_MAIL].as_string();
		return true;
	}

	return false;
}

bool CJsonOpt::JsonParseResetUserPassword(std::string&username, std::string& new_password)
{
	if (VerifyJsonField(in_, JK_USERNAME) && VerifyJsonField(in_, JK_NEW_PASSWORD))
	{
		username = in_[JK_USERNAME].as_string();
		new_password = in_[JK_NEW_PASSWORD].as_string();
		return true;
	}

	return false;
}

std::string CJsonOpt::JsonJoinCommonOpt(const int message_type, const int result)
{
	JsonJoinCommon(message_type, result);
	return out_.write();
}

std::string CJsonOpt::JsonJoinGetAccountInfo(const int message_type, const int result, const AccountInfo& account_info)
{
	JsonJoinCommon(message_type, result);
	out_.push_back(JSONNode(JK_USERNAME,account_info.username));
	out_.push_back(JSONNode(JK_MAIL,account_info.mail));
	out_.push_back(JSONNode(JK_PHONE,account_info.phone));
	out_.push_back(JSONNode(JK_NICKNAME,account_info.nickname));

	return out_.write();
}

std::string CJsonOpt::JsonJoinGetUserDetailInfo(const int result, const USER_INFO& userInfo)
{
	JsonJoinCommon(GET_USER_DETAIL_INFO_RESPONSE, result);

	JSONNode out_out;
	out_out.set_name(JK_USER_OTHER_INFO);
	out_out.push_back(JSONNode(JK_SECURITY_MAIL, userInfo.security_mail));
	out_.push_back(out_out);
	return out_.write();
}


