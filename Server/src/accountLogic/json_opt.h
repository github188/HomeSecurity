/*
 * json_operate.h
 *
 *  Created on: Mar 11, 2013
 *      Author: yaowei
 */

#ifndef JSON_OPERATE_H_
#define JSON_OPERATE_H_

#include <libjson/libjson.h>
#include <libjson/Source/JSONNode.h>
#include "../public/user_interface_defines.h"

class CJsonOpt
{
public:
	CJsonOpt();
	explicit CJsonOpt(const std::string& json_string);
	virtual ~CJsonOpt();

public:

	void setJsonString(const std::string &);

	bool JsonParseMessageType(int& message_type);

	bool JsonParseUsername(std::string& username);

	bool JsonParseUserRegister(std::string& username, std::string& password, std::string& custom_type);

	bool JsonParseSetAccountInfo(AccountInfo& account_info);
	std::string JsonJoinGetAccountInfo(const int message_type, const int result, const AccountInfo& account_info);
	std::string JsonJoinGetAccountMailWithNoSession(const int message_type, const int result, const std::string& mail);

	bool JsonParseUserLogin(std::string& username, std::string& password);
	std::string JsonJoinUserLogin(const int result, const std::string& session_id);

	bool JsonParseModifyUserName(std::string& username, std::string& old_username);

	bool JsonParseMailOrPhone(std::string& mailOrPhone);

	bool JsonParseReportClientPlatformInfo(CLIENT_PLATFORM_INFO& clientPlatformInfo);

	std::string JsonJoinGetCurrentAlarmFlag(const int result, const int alarm_flag);

	bool JsonParseSetCurrentAlarmFlag(int& alarm_flag, std::string& mobile_id);

	bool JsonParseUserSessionId(std::string& session_id);

	bool JsonParseUserPassword(std::string& password);

	bool JsonParseModifyUserPassword(std::string& password, std::string& new_password);

	bool JsonParseSendFeedbackToMail(std::string& feedback, std::string& mail_address);

	bool JsonParseSendToSecurityMail(std::string& username, std::string& security_mail);
	bool JsonParseResetUserPassword(std::string&username, std::string& new_password);

	std::string JsonJoinCommonOpt(const int message_type, const int result);

	std::string JsonJoinGetUserDetailInfo(const int result, const USER_INFO& userInfo);

private:

	void JsonJoinCommon(const int message_type, const int result);

	bool VerifyCommonJson(JSONNode& in);

	bool VerifyJsonField(JSONNode& in, const std::string& field);

private:

	std::string json_string_;
	JSONNode in_;
	JSONNode out_;
	int  message_id_;
	int sfd_;
	int sfd_id_;
};

#endif /* JSON_OPERATE_H_ */
