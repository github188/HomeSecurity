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

	std::string JsonJoinCommonOpt(const int message_type, const int result);

	bool JsonParseGetLastSoftVersion(std::string& current_version);
	std::string JsonJoinGetLastSoftVersion(const int message_type, const int result, const SoftVersionInfo& last_versionInfo);

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
