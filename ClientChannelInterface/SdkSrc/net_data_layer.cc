#include "net_data_layer.h"
#include "defines.h"
#include "message.h"

#define MESSAGE_ID			"message_id"
#define SOCKET_FD			"socket_fd"
#define TCP_CONNECT_TYPE    "tcp_connect_type"

CNetDataLayer::CNetDataLayer(void)
{
}

CNetDataLayer::~CNetDataLayer(void)
{
}

bool CNetDataLayer::VerifyCommonJson(JSONNode& in)
{
	return VerifyJsonField(in, JK_P2RELAY_MESSAGE_TYPE) && VerifyJsonField(in, JK_RESULT);
}

bool CNetDataLayer::VerifyJsonField(JSONNode& in, std::string field)
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

bool CNetDataLayer::JsonParseResult( const std::string in_json_string, int& result )
{
	JSONNode in;
	try
	{
		in = libjson::parse(in_json_string);
	}
	catch(...)
	{
		LOG4CXX_ERROR(g_logger_sdk, "CNetDataLayer::JsonParseResult std::invalid_argument");
		return false;
	}

	if (!VerifyCommonJson(in))
	{
		return false;
	}

	result = in[JK_RESULT].as_int();

	return true;
}

std::string CNetDataLayer::JsonJoinGetLiveStatus( const int message_id )
{
	JSONNode out;
	JsonJoinImDirectPublic(message_id, HEATBEAT_DETECT, out);
	return out.write();
}

bool CNetDataLayer::JsonParseMessageId( const std::string& in_json_string, int& message_id )
{
	JSONNode in;
	try
	{
		in = libjson::parse(in_json_string);
	}
	catch(...)
	{
		LOG4CXX_ERROR(g_logger_sdk, "CNetDataLayer::JsonParseMessageId std::invalid_argument");
		return false;
	}

	if (!VerifyJsonField(in, JK_MESSAGE_ID))
	{
		return false;
	}

	message_id = in[JK_MESSAGE_ID].as_int();

	return true;
}

void CNetDataLayer::JsonJoinImDirectPublic( const int message_id, const int message_type, JSONNode& out )
{
	out.push_back(JSONNode(JK_MESSAGE_ID, message_id));
	out.push_back(JSONNode(JK_P2RELAY_MESSAGE_TYPE, message_type));
	out.push_back(JSONNode(JK_PROTO_VERSION, PROTO_VERSION));
	out.push_back(JSONNode(JK_LOGIC_PROCESS_TYPE, ALARM_SERVER_RELAY));
}

bool CNetDataLayer::RestructJsonByMessageId(const std::string& in_json_string, const int message_id, std::string& out_json_string)
{
	JSONNode in;
	try
	{
		in = libjson::parse(in_json_string);
	}
	catch (...)
	{
		return false;
	}

	if(!VerifyJsonField(in, JK_MESSAGE_ID))
		in.push_back(JSONNode(JK_MESSAGE_ID, message_id));
	else
		in.at(JK_MESSAGE_ID) = message_id;

	in.push_back(JSONNode(JK_SESSION_ID, session_id_));

	out_json_string = in.write();

	return true;
}
