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
#include "defines.h"

class CJsonOpt
{
public:
	CJsonOpt();
	virtual ~CJsonOpt();

	bool JsonParseLogicType(const std::string& message, int& logic_type);

	bool JsonParseP2RelayMessageType(int& p2relay_message_type);

	bool JsonParseUserServerNo(int& server_no);

	bool JsonParseUsername(std::string& username);

	std::string RestructToImServerMessage(const int online_server_no, const int sfd, const int sfd_id);

	std::string JsonJoinHeatbeatDetectResponse(const int result);

	std::string JsonJoinGetAccountLiveInfo(const int result, const ACCOUNT_LIVE_INFO& accountLiveInfo);

	bool JsonParseIm2RelayDeviceGUID(std::string& device_guid);

	std::string JsonJoinSendMessageToUser(const int result);

private:

	bool VerifyJsonField(const JSONNode& in, const std::string& field);

	void JsonJoinCommon(const int p2rmessage_type, const int result);

private:

	JSONNode in_;
	JSONNode out_;
	int message_id_;
	std::string protocol_version_;

};

#endif /* JSON_OPT_H_ */
