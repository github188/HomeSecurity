/*
 * json_opt.h
 *
 *  Created on: 2014-2-13
 *      Author: zhangs
 */

#ifndef JSON_OPT_H_
#define JSON_OPT_H_

#include "libjson/libjson.h"
#include "libjson/Source/JSONNode.h"
#include "../public/user_interface_defines.h"
#include "defines.h"

class CJsonOpt
{
public:
	CJsonOpt();
	virtual ~CJsonOpt();

	bool ParseMessageLogicType(const char* json_in_string, int& logic_type, int& message_type);
	bool ParseDeviceGuidMessage(const char* json_in_string, std::string &guid);

	std::string JsonJoinCommon(const int message_type, const int result);
	std::string JsonJoinHeartBeat(const int message_type, const int result, const std::string dc);

	void JsonJoinPublic(const int message_type, const int result);

	bool JsonParseSetOnlineStatus(int& online_status);
	bool VerifyC2SRelayMessage();
	bool JsonParseR2PMessageType(const std::string& message, int& im2relay_type);
	std::string RestructIm2RelayMessage(const int relay_type);
	bool JsonParseRelay2ImMessage(const std::string& message, std::string& peer_name);
	bool JsonParseRelay2ImDevMessage(const std::string& message, std::string& device_guid);

	bool JsonParseDeviceReportHumiture(std::string& device_tem, std::string& device_hum, std::string& timestamp);

	bool JsonParseDeviceModifyInfo(const std::string& message, std::string& device_guid, int& alarm_switch);
	std::string RestructDeviceModifyInfo2Client(const std::string& message, const int alarm_switch = 0);

	std::string RestructPush2DCommon(const std::string& message);

	std::string RestructDeviceUpdateCMD2Client(const std::string& message);

	std::string RestructGetDeviceUpdateStep2Client(const std::string& message);

	std::string RestructDeviceModifyPassword2Client(const std::string& message);

	bool JsonParsePushAlarmMessage(ALARM_INFO& alarm_info);

	bool JsonParsePushAlarmMessageFtp(ALARM_INFO& alarm_info);

	std::string JsonJoinAlarmMessage2Relay(const std::string& username, const ALARM_INFO& alarm_info);

	std::string JsonJoinAlarmFtpMessage2Relay(const std::string& username, const ALARM_INFO& alarm_info, int alarm_level);

	std::string JsonJoinPushAlarmMessage(const int message_type, const int result, const int amt, const std::string guid);

	std::string JsonJoinGetDeviceUsers(const std::string& device_guid);
	bool JsonParseGetDeviceUsers(const std::string& in_json, std::vector<std::string>& users);

	bool VerifyD2CRelayMessage();

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
