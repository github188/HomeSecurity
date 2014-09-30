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
	message_id_ = 0;
	sfd_ = 0;
	sfd_id_ = 0;
}
CJsonOpt::CJsonOpt()
{
	message_id_ = 0;
	sfd_ = 0;
	sfd_id_ = 0;
}
CJsonOpt::~CJsonOpt()
{
}

bool CJsonOpt::JsonParseCommonField(const std::string& in_json_string,
									std::string& session_id,
									std::string& custom_string)
{

	JSONNode in;
	try
	{
		in = libjson::parse(in_json_string);
	} catch (...)
	{
		return false;
	}

	if (!VerifyCommonJson(in))
		return false;

	message_id_ = in[JK_MESSAGE_ID].as_int();
	sfd_ = in[JK_CLINET_SFD].as_int();
	sfd_id_ = in[JK_CLINET_SFD_ID].as_int();

	in.pop_back(JK_MESSAGE_ID);
	in.pop_back(JK_CLINET_SFD);
	in.pop_back(JK_CLINET_SFD_ID);

	if(!VerifyJsonField(in, JK_SESSION_ID))
		return false;

	session_id = in[JK_SESSION_ID].as_string();

	if(!VerifyJsonField(in, JK_CUSTOM_STRING))
		return false;

	custom_string = in[JK_CUSTOM_STRING].as_string();

	return true;
}

std::string CJsonOpt::JsonJoinToUserJson(const int result, const std::string& custom_string)
{
	JSONNode out;

	out.push_back(JSONNode(JK_RESULT, result));
	out.push_back(JSONNode(JK_CUSTOM_STRING, custom_string));
	out.push_back(JSONNode(JK_MESSAGE_ID, message_id_));
	out.push_back(JSONNode(JK_CLINET_SFD, sfd_));
	out.push_back(JSONNode(JK_CLINET_SFD_ID, sfd_id_));

	return out.write();

}

std::string CJsonOpt::JsonJoinDirectToUserJson(const int result)
{
	JSONNode out;

	out.push_back(JSONNode(JK_RESULT, result));
	out.push_back(JSONNode(JK_MESSAGE_ID, message_id_));
	out.push_back(JSONNode(JK_CLINET_SFD, sfd_));
	out.push_back(JSONNode(JK_CLINET_SFD_ID, sfd_id_));

	return out.write();
}

bool CJsonOpt::VerifyCommonJson(JSONNode& in)
{
	return VerifyJsonField(in, JK_MESSAGE_ID) && VerifyJsonField(in, JK_CLINET_SFD)
		   && VerifyJsonField(in, JK_CLINET_SFD_ID);
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


