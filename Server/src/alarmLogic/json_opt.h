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

	bool JsonParseCommonField(const std::string& in_json_string,
							  std::string& session_id,
							  std::string& custom_string);

	std::string JsonJoinToUserJson(const int result, const std::string& custom_string);

	std::string JsonJoinDirectToUserJson(const int result);

private:

	bool VerifyCommonJson(JSONNode& in);

	bool VerifyJsonField(JSONNode& in, const std::string& field);

private:


	int message_id_;
	int sfd_;
	int sfd_id_;
};

#endif /* JSON_OPERATE_H_ */
