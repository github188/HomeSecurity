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
#include <vector>
#include <map>

class CJsonOpt
{
public:
	CJsonOpt();
	explicit CJsonOpt(const std::string& json_string);
	virtual ~CJsonOpt();

public:

	void setJsonString(const std::string &);

	bool JsonParseMessageType(int& message_type);

	bool JsonParseUserSessionId(std::string& session_id);

	std::string JsonJoinCommonOpt(const int message_type, const int result);

	bool JsonParseGetAlarmInfo(int& idx_start, int& idx_stop);
	std::string JsonJoinGetAlarmInfo(const int result, const std::vector< std::map<std::string, std::string> >& alarms);

	bool JsonParseDelAlarmInfo(std::string& alarm_guid);

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
