/*
 * json_opt.h
 *
 *  Created on: Mar 6, 2013
 *      Author: yaowei
 */

#ifndef JSON_OPT_H_
#define JSON_OPT_H_

#include <libjson/libjson.h>
#include <libjson/Source/JSONNode.h>
#include "../public/user_interface_defines.h"
#include "defines.h"

class CJsonOpt
{
public:
	CJsonOpt();
	virtual ~CJsonOpt();

	bool ParseMessageLogicType(const char* json_in_string, int& logic_type, int& message_type, std::string& session_id);

	std::string JsonJoinCommon(const int message_type, const int result);

	std::string JsonJoinNotifyOffline(const std::string& username, const int online_server_no, const int sfd, const int sfd_id);

	void JsonJoinPublic(const int message_type, const int result);

	bool VerifyC2DRelayMessage();

	std::string RestructIm2RelayMessage(const int relay_type);

	bool JsonParseSetOnlineStatus(int& online_status);

	bool JsonParseNotifyOffline(int& server_no, int& sfd, int& sfd_id);

	bool JsonParseR2PMessageType(const std::string& message, int& im2relay_type);

	bool JsonParseUsername(std::string& username);

	std::string RestructMessageToUser(const std::string& message);

	bool JsonParseRelay2ImMessage(const std::string& message, std::string& device_guid);

	std::string RestructDeviceModifyResult2Client(const std::string& message);

	bool JsonParsePushC2DMessageId(const std::string& username, int& message_id, std::string& device_guid);
	bool JsonParseMessageId(const std::string& in_json_string, int& message_id);
	bool JsonParsePushDeviceModifyInfoResult(const std::string& in_json_string, int& result);
	bool JsonParsePushResult(const std::string& in_json_string, int& result);

	std::string RestructPush2CCommon(const std::string& message);

	bool JsonParseGetDeviceUpdateStepResult(const std::string& in_json_string, int& result, int& downloadstep, int& writestep);
	std::string JsonJoinGetUpdateDownloadStep(const int result, const int& downloadstep);
	std::string JsonJoinGetUpdateWriteStep(const int result, const int& writestep);
	std::string RestructDeviceUpdateStepResult2Client(const std::string& message);

	std::string RestructAlarmMessageToUser(const std::string& message);

private:

	bool VerifyPublicJson(JSONNode& in);

	bool VerifyJsonField(const JSONNode& in, const std::string& field);

private:

	JSONNode in_;
	JSONNode out_;

	int  message_id_;
	std::string version_;

};

#endif /* JSON_OPT_H_ */
