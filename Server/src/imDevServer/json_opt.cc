/*
 * json_pt.cc
 *
 *  Created on: 2014-2-13
 *      Author: zhangs
 */

#include "json_opt.h"
#include "../public/message.h"
#include "../public/utils.h"

CJsonOpt::CJsonOpt()
{
	message_id_ = 0;
}

CJsonOpt::~CJsonOpt()
{
	// TODO Auto-generated destructor stub
}

bool CJsonOpt::ParseMessageLogicType(const char* json_in_string, int& logic_type, int& message_type)
{
	try
	{
		in_ = libjson::parse(json_in_string);
	}
	catch (...)
	{
		return false;
	}

	if (!VerifyPublicJson(in_))
	{
		return false;
	}

	logic_type 	= in_[JK_LOGIC_PROCESS_TYPE].as_int();
	message_type= in_[JK_MESSAGE_TYPE].as_int();
	message_id_ = in_[JK_MESSAGE_ID].as_int();
	return true;
}

bool CJsonOpt::ParseDeviceGuidMessage(const char* json_in_string, std::string &guid)
{
	try
	{
		in_ = libjson::parse(json_in_string);
	}
	catch (...)
	{
		return false;
	}

	if(!VerifyJsonField(in_, JK_DEVICE_GUID))
	{
		return false;
	}

	guid = in_[JK_DEVICE_GUID].as_string();
	return true;
}

bool CJsonOpt::VerifyC2SRelayMessage()
{
	if(!(VerifyJsonField(in_, JK_PEER_USERNAME) && VerifyJsonField(in_, JK_RELAY_MESSAGE)))
		return false;

	return true;
}

bool CJsonOpt::JsonParseR2PMessageType(const std::string& message, int& im2relay_type)
{
	try
	{
		in_ = libjson::parse(message);
	}
	catch (...)
	{
		return false;
	}

	if(!VerifyJsonField(in_, JK_P2RELAY_MESSAGE_TYPE))
	{
		return false;
	}

	im2relay_type = in_[JK_P2RELAY_MESSAGE_TYPE].as_int();

	return true;
}

std::string CJsonOpt::RestructIm2RelayMessage(const int relay_type)
{
	unsigned long timestamp = time(NULL);
	in_.push_back(JSONNode(JK_P2RELAY_MESSAGE_TYPE, relay_type));
	in_.push_back(JSONNode(JK_RELAY_MESSAGE_TIMESTAMP, utils::toString(timestamp)));
	return in_.write();
}

bool CJsonOpt::JsonParseRelay2ImMessage(const std::string& message, std::string& peer_name)
{
	try
	{
		in_ = libjson::parse(message);
	}
	catch (...)
	{
		return false;
	}

	if(!VerifyJsonField(in_, JK_PEER_USERNAME))
		return false;

	peer_name = in_[JK_PEER_USERNAME].as_string();

	return true;
}

bool CJsonOpt::JsonParseRelay2ImDevMessage(const std::string& message, std::string& device_guid)
{
	try
	{
		in_ = libjson::parse(message);
	}
	catch (...)
	{
		return false;
	}

	if(!VerifyJsonField(in_, JK_DEVICE_GUID))
		return false;

	device_guid = in_[JK_DEVICE_GUID].as_string();

	return true;
}

bool CJsonOpt::VerifyPublicJson(JSONNode & in)
{
	return VerifyJsonField(in, JK_LOGIC_PROCESS_TYPE)&&VerifyJsonField(in, JK_PROTO_VERSION) &&
		   VerifyJsonField(in, JK_MESSAGE_TYPE) && VerifyJsonField(in, JK_MESSAGE_ID);

}

bool CJsonOpt::VerifyJsonField(const JSONNode & in, const std::string & field)
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

std::string CJsonOpt::JsonJoinCommon(const int message_type, const int result)
{
	JsonJoinPublic(message_type, result);
	return out_.write();
}

std::string CJsonOpt::JsonJoinHeartBeat(const int message_type, const int result, const std::string dc)
{
	JsonJoinPublic(message_type, result);
	out_.push_back(JSONNode(JK_DEVICES_CHANGE, dc));
	return out_.write();
}

void CJsonOpt::JsonJoinPublic(const int message_type, const int result)
{
	out_.push_back(JSONNode(JK_MESSAGE_TYPE, message_type));
	out_.push_back(JSONNode(JK_RESULT, result));
	out_.push_back(JSONNode(JK_MESSAGE_ID, message_id_));
}


/*
设置用户在线状态
 request:
 {
	JK_MESSAGE_TYPE : 	<int>, (SET_ONLINE_STATUS)
	JK_SESSION_ID :   	<char>
	JK_USER_ONLINE_STATUS: <int>
 }
 response:
 {
	JK_MESSAGE_TYPE : 	<int>, (SET_ONLINE_STATUS_RESPONSE)
	JK_RESULT :      	<int>
 }
 */
bool CJsonOpt::JsonParseSetOnlineStatus(int& online_status)
{
	if(!VerifyJsonField(in_, JK_USER_ONLINE_STATUS))
		return false;

	online_status = in_[JK_USER_ONLINE_STATUS].as_int();

	return true;
}

bool CJsonOpt::JsonParseDeviceReportHumiture(std::string& device_tem, std::string& device_hum, std::string& timestamp)
{
	if (!(VerifyJsonField(in_, JK_DEVICE_TEMPERATURE) && VerifyJsonField(in_, JK_DEVICE_HUMIDNESS) && VerifyJsonField(in_, JK_DEVICE_TIMESTAMP)))
	{
		return false;
	}

	device_tem = in_[JK_DEVICE_TEMPERATURE].as_string();
	device_hum = in_[JK_DEVICE_HUMIDNESS].as_string();
	timestamp = in_[JK_DEVICE_TIMESTAMP].as_string();

	return true;
}

bool CJsonOpt::JsonParseDeviceModifyInfo(const std::string& message, std::string& device_guid, int& alarm_switch)
{
	try
	{
		in_ = libjson::parse(message);
	}
	catch (...)
	{
		return false;
	}

	if(!VerifyJsonField(in_, JK_DEVICE_GUID))
		return false;

	device_guid = in_[JK_DEVICE_GUID].as_string();

	JSONNode dinfo = in_[JK_DEVICE_INFO].as_node();
	if (VerifyJsonField(dinfo, JK_ALARM_SWITCH))
	{
		alarm_switch = dinfo[JK_ALARM_SWITCH].as_int();
	}

	return true;
}

std::string CJsonOpt::RestructDeviceModifyInfo2Client(const std::string& message, const int alarm_switch)
{
	JSONNode in;
	JSONNode out;
	try
	{
		in = libjson::parse(message);
	}
	catch (...)
	{
		LOG4CXX_ERROR(g_logger, "CJsonOpt::RestructDeviceModifyInfo2Client:parse error.");
		return "";
	}
	
	if(!(VerifyJsonField(in, JK_MESSAGE_ID) && VerifyJsonField(in, JK_MESSAGE_TYPE) && VerifyJsonField(in, JK_DEVICE_GUID) && VerifyJsonField(in, JK_USERNAME) && VerifyJsonField(in, JK_DEVICE_INFO)))
	{
		LOG4CXX_ERROR(g_logger, "CJsonOpt::RestructDeviceModifyInfo2Client:VerifyJsonField error.");
		return "";
	}

	out.push_back(in[JK_MESSAGE_ID]);
	out.push_back(in[JK_MESSAGE_TYPE]);
	out.push_back(in[JK_DEVICE_GUID]);
	out.push_back(in[JK_USERNAME]);

	JSONNode dinfo = in[JK_DEVICE_INFO].as_node();
	if (VerifyJsonField(dinfo, JK_DEVICE_NAME))
	{
		out.push_back(dinfo[JK_DEVICE_NAME]);
	}
	if (VerifyJsonField(dinfo, JK_NET_STORAGE_SWITCH))
	{
		out.push_back(dinfo[JK_NET_STORAGE_SWITCH]);
	}
	if (VerifyJsonField(dinfo, JK_TF_STORAGE_SWITCH))
	{
		out.push_back(dinfo[JK_TF_STORAGE_SWITCH]);
	}
	if (VerifyJsonField(dinfo, JK_ALARM_SWITCH))
	{
		if (alarm_switch == 0)
			out.push_back(dinfo[JK_ALARM_SWITCH]);
		else
			out.push_back(JSONNode(JK_ALARM_SWITCH, alarm_switch));
	}
	if (VerifyJsonField(dinfo, JK_ALARM_TIME))
	{
		out.push_back(dinfo[JK_ALARM_TIME]);
	}
	if (VerifyJsonField(dinfo, JK_DEVICE_BABY_MODE))
	{
		out.push_back(dinfo[JK_DEVICE_BABY_MODE]);
	}
	if (VerifyJsonField(dinfo, JK_DEVICE_FULL_ALARM_MODE))
	{
		out.push_back(dinfo[JK_DEVICE_FULL_ALARM_MODE]);
	}

	return out.write();
}

bool CJsonOpt::VerifyD2CRelayMessage()
{
	if (!(VerifyJsonField(in_, JK_USERNAME) && VerifyJsonField(in_, JK_DEVICE_GUID)))
		return false;
	return true;
}

std::string CJsonOpt::RestructPush2DCommon(const std::string& message)
{
	JSONNode in;
	JSONNode out;
	try
	{
		in = libjson::parse(message);
	}
	catch (...)
	{
		LOG4CXX_ERROR(g_logger, "CJsonOpt::RestructPush2DCommon:parse error.");
		return "";
	}

	if(!(VerifyJsonField(in, JK_MESSAGE_ID) && VerifyJsonField(in, JK_MESSAGE_TYPE) && VerifyJsonField(in, JK_DEVICE_GUID) && VerifyJsonField(in, JK_USERNAME)))
	{
		LOG4CXX_ERROR(g_logger, "CJsonOpt::RestructPush2DCommon:VerifyJsonField error.");
		return "";
	}

	out.push_back(in[JK_MESSAGE_ID]);
	out.push_back(in[JK_MESSAGE_TYPE]);
	out.push_back(in[JK_DEVICE_GUID]);
	out.push_back(in[JK_USERNAME]);

	return out.write();
}

std::string CJsonOpt::RestructDeviceUpdateCMD2Client(const std::string& message)
{
	JSONNode in;
	JSONNode out;
	try
	{
		in = libjson::parse(message);
	}
	catch (...)
	{
		LOG4CXX_ERROR(g_logger, "CJsonOpt::RestructPush2DCommon:parse error.");
		return "";
	}

	if(!(VerifyJsonField(in, JK_MESSAGE_ID) && VerifyJsonField(in, JK_MESSAGE_TYPE) && VerifyJsonField(in, JK_DEVICE_GUID) && VerifyJsonField(in, JK_USERNAME)))
	{
		LOG4CXX_ERROR(g_logger, "CJsonOpt::RestructPush2DCommon:VerifyJsonField error.");
		return "";
	}

	out.push_back(in[JK_MESSAGE_ID]);
	out.push_back(in[JK_MESSAGE_TYPE]);
	out.push_back(in[JK_DEVICE_GUID]);
	out.push_back(in[JK_USERNAME]);
	if (VerifyJsonField(in, JK_UPGRADE_FILE_VERSION))
	{
		out.push_back(in[JK_UPGRADE_FILE_VERSION]);
	}
	if (VerifyJsonField(in, JK_UPGRADE_FILE_URL))
	{
		out.push_back(in[JK_UPGRADE_FILE_URL]);
	}
	if (VerifyJsonField(in, JK_UPGRADE_FILE_SIZE))
	{
		out.push_back(in[JK_UPGRADE_FILE_SIZE]);
	}

	return out.write();
}

std::string CJsonOpt::RestructGetDeviceUpdateStep2Client(const std::string& message)
{
	JSONNode in;
	JSONNode out;
	try
	{
		in = libjson::parse(message);
	}
	catch (...)
	{
		LOG4CXX_ERROR(g_logger, "CJsonOpt::RestructGetDeviceUpdateStep2Client:parse error.");
		return "";
	}

	if(!(VerifyJsonField(in, JK_MESSAGE_ID) && VerifyJsonField(in, JK_MESSAGE_TYPE) && VerifyJsonField(in, JK_DEVICE_GUID) && VerifyJsonField(in, JK_USERNAME)))
	{
		LOG4CXX_ERROR(g_logger, "CJsonOpt::RestructDeviceUpdateCMD2Client:VerifyJsonField error.");
		return "";
	}

	out.push_back(in[JK_MESSAGE_ID]);
	out.push_back(in[JK_MESSAGE_TYPE]);
	out.push_back(in[JK_DEVICE_GUID]);
	out.push_back(in[JK_USERNAME]);

	if (VerifyJsonField(in, JK_UPGRADE_FILE_URL))
	{
		out.push_back(in[JK_UPGRADE_FILE_URL]);
	}
	if (VerifyJsonField(in, JK_UPGRADE_FILE_SIZE))
	{
		out.push_back(in[JK_UPGRADE_FILE_SIZE]);
	}

	return out.write();
}

std::string CJsonOpt::RestructDeviceModifyPassword2Client(const std::string& message)
{
	JSONNode in;
	JSONNode out;
	try
	{
		in = libjson::parse(message);
	}
	catch (...)
	{
		LOG4CXX_ERROR(g_logger, "CJsonOpt::RestructDeviceModifyPassword2Client:parse error.");
		return "";
	}

	if(!(VerifyJsonField(in, JK_MESSAGE_ID) && VerifyJsonField(in, JK_MESSAGE_TYPE) && VerifyJsonField(in, JK_DEVICE_GUID) && VerifyJsonField(in, JK_USERNAME)))
	{
		LOG4CXX_ERROR(g_logger, "CJsonOpt::RestructDeviceModifyPassword2Client:VerifyJsonField error.");
		return "";
	}

	out.push_back(in[JK_MESSAGE_ID]);
	out.push_back(in[JK_MESSAGE_TYPE]);
	out.push_back(in[JK_DEVICE_GUID]);
	out.push_back(in[JK_USERNAME]);

	if (VerifyJsonField(in, JK_DEVICE_USERNAME))
	{
		// 兼容老设备 修改admin密码
		if (in[JK_DEVICE_USERNAME].as_string() == "jwifiApuser")
		{
			out.push_back(JSONNode(JK_DEVICE_USERNAME, "admin"));
		}
		else
		{
			out.push_back(in[JK_DEVICE_USERNAME]);
		}
	}
	if (VerifyJsonField(in, JK_DEVICE_PASSWORD))
	{
		out.push_back(in[JK_DEVICE_PASSWORD]);
	}

	return out.write();
}

bool CJsonOpt::JsonParsePushAlarmMessage(ALARM_INFO& alarm_info)
{
	if (!(VerifyJsonField(in_, JK_ALARM_GUID) && VerifyJsonField(in_, JK_DEVICE_GUID) && 
		VerifyJsonField(in_, JK_DEVICE_NAME) && VerifyJsonField(in_, JK_DEVICE_CHANNEL_NO) && 
		VerifyJsonField(in_, JK_ALARM_TYPE) && VerifyJsonField(in_, JK_ALARM_PIC) && 
		VerifyJsonField(in_, JK_ALARM_VIDEO) && VerifyJsonField(in_, JK_ALARM_TIMESTAMP)))
	{
		return false;
	}

	alarm_info.alarm_guid = in_[JK_ALARM_GUID].as_string();
	alarm_info.device_guid = in_[JK_DEVICE_GUID].as_string();
	alarm_info.device_name = in_[JK_DEVICE_NAME].as_string();
	alarm_info.alarm_channel_no = in_[JK_DEVICE_CHANNEL_NO].as_int();
	alarm_info.alarm_type = in_[JK_ALARM_TYPE].as_int();
	alarm_info.alarm_pic = in_[JK_ALARM_PIC].as_string();
	alarm_info.alarm_video = in_[JK_ALARM_VIDEO].as_string();
	alarm_info.alarm_timestamp = in_[JK_ALARM_TIMESTAMP].as_int();

	return true;
}

bool CJsonOpt::JsonParsePushAlarmMessageFtp(ALARM_INFO& alarm_info)
{
	if (!VerifyJsonField(in_, JK_ALARM_FTP_GUID)) {
		LOG4CXX_ERROR(g_logger, "CJsonOpt::JsonParsePushAlarmMessageFtp error JK_ALARM_FTP_GUID");
		return false;
	}

	if (!VerifyJsonField(in_, JK_ALARM_FTP_MESSAGE_TYPE)) {
		LOG4CXX_ERROR(g_logger, "CJsonOpt::JsonParsePushAlarmMessageFtp error JK_ALARM_FTP_MESSAGE_TYPE");
		return false;
	}

	if (!VerifyJsonField(in_, JK_ALARM_FTP_DEVICE_GUID)) {
		LOG4CXX_ERROR(g_logger, "CJsonOpt::JsonParsePushAlarmMessageFtp error JK_ALARM_FTP_DEVICE_GUID");
		return false;
	}

	if (!VerifyJsonField(in_, JK_ALARM_FTP_TIMESTAMP)) {
		LOG4CXX_ERROR(g_logger, "CJsonOpt::JsonParsePushAlarmMessageFtp error JK_ALARM_FTP_TIMESTAMP");
		return false;
	}

	if (!VerifyJsonField(in_, JK_ALARM_FTP_DEVICE_NAME)) {
		LOG4CXX_ERROR(g_logger, "CJsonOpt::JsonParsePushAlarmMessageFtp error JK_ALARM_FTP_DEVICE_NAME");
		return false;
	}

	if (!VerifyJsonField(in_, JK_ALARM_FTP_CHANNEL_NO)) {
		LOG4CXX_ERROR(g_logger, "CJsonOpt::JsonParsePushAlarmMessageFtp error JK_ALARM_FTP_CHANNEL_NO");
		return false;
	}

	if (!VerifyJsonField(in_, JK_ALARM_FTP_TYPE)) {
		LOG4CXX_ERROR(g_logger, "CJsonOpt::JsonParsePushAlarmMessageFtp error JK_ALARM_FTP_TYPE");
		return false;
	}

	if (!VerifyJsonField(in_, JK_ALARM_FTP_PIC)) {
		LOG4CXX_ERROR(g_logger, "CJsonOpt::JsonParsePushAlarmMessageFtp error JK_ALARM_FTP_PIC");
		return false;
	}

// 	if (!VerifyJsonField(in_, JK_ALARM_FTP_PIC_SIZE)) {
// 		LOG4CXX_ERROR(g_logger, "CJsonOpt::JsonParsePushAlarmMessageFtp error JK_ALARM_FTP_PIC_SIZE");
// 		return false;
// 	}

	if (!VerifyJsonField(in_, JK_ALARM_FTP_VIDEO)) {
		LOG4CXX_ERROR(g_logger, "CJsonOpt::JsonParsePushAlarmMessageFtp error JK_ALARM_FTP_VIDEO");
		return false;
	}

// 	if (!VerifyJsonField(in_, JK_ALARM_FTP_VIDEO_SZIE)) {
// 		LOG4CXX_ERROR(g_logger, "CJsonOpt::JsonParsePushAlarmMessageFtp error JK_ALARM_FTP_VIDEO_SZIE");
// 		return false;
// 	}

	alarm_info.alarm_guid			= in_[JK_ALARM_FTP_GUID].as_string();
	alarm_info.alarm_msg_type		= in_[JK_ALARM_FTP_MESSAGE_TYPE].as_int();
	alarm_info.device_guid			= in_[JK_ALARM_FTP_DEVICE_GUID].as_string();
	alarm_info.alarm_timestamp		= in_[JK_ALARM_FTP_TIMESTAMP].as_int();
	alarm_info.device_name			= in_[JK_ALARM_FTP_DEVICE_NAME].as_string();
	alarm_info.alarm_channel_no		= in_[JK_ALARM_FTP_CHANNEL_NO].as_int();
	alarm_info.alarm_type			= in_[JK_ALARM_FTP_TYPE].as_int();
	alarm_info.alarm_pic			= in_[JK_ALARM_FTP_PIC].as_string();

	if (VerifyJsonField(in_, JK_ALARM_FTP_PIC_SIZE)) {
		alarm_info.alarm_pic_size		= in_[JK_ALARM_FTP_PIC_SIZE].as_int();
	}

	alarm_info.alarm_video			= in_[JK_ALARM_FTP_VIDEO].as_string();

	if (VerifyJsonField(in_, JK_ALARM_FTP_VIDEO_SZIE)) {
		alarm_info.alarm_video_size		= in_[JK_ALARM_FTP_VIDEO_SZIE].as_int();
	}

	return true;
}

std::string CJsonOpt::JsonJoinAlarmMessage2Relay(const std::string& username, const ALARM_INFO& alarm_info)
{
	JSONNode out;
	out.push_back(JSONNode(JK_P2RELAY_MESSAGE_TYPE, RELAY_ALARM_MESSAGE));
	out.push_back(in_[JK_MESSAGE_ID]);
	out.push_back(in_[JK_MESSAGE_TYPE]);
	out.push_back(JSONNode(JK_LOGIC_PROCESS_TYPE, 9));
	out.push_back(JSONNode(JK_USERNAME, username));
	out.push_back(in_[JK_ALARM_GUID]);
	out.push_back(in_[JK_DEVICE_GUID]);
	out.push_back(in_[JK_DEVICE_NAME]);
	out.push_back(in_[JK_DEVICE_CHANNEL_NO]);
	out.push_back(in_[JK_ALARM_TYPE]);
	out.push_back(in_[JK_ALARM_TIMESTAMP]);
	if (VerifyJsonField(in_, JK_ALARM_MESSAGE_TYPE))
	{
		out.push_back(in_[JK_ALARM_MESSAGE_TYPE]);
	}
	if (VerifyJsonField(in_, JK_ALARM_PIC))
	{
		out.push_back(in_[JK_ALARM_PIC]);
	}
	if (VerifyJsonField(in_, JK_ALARM_PIC_SIZE))
	{
		out.push_back(in_[JK_ALARM_PIC_SIZE]);
	}
	if (VerifyJsonField(in_, JK_ALARM_VIDEO))
	{
		out.push_back(in_[JK_ALARM_VIDEO]);
	}
	if (VerifyJsonField(in_, JK_ALARM_VIDEO_SZIE))
	{
		out.push_back(in_[JK_ALARM_VIDEO_SZIE]);
	}

	return out.write();
}

std::string CJsonOpt::JsonJoinAlarmFtpMessage2Relay(const std::string& username, const ALARM_INFO& alarm_info, int alarm_level) {

	char						strtime[100];
	struct tm					*p;

	memset(strtime, 0, sizeof(strtime));

	p = localtime(&alarm_info.alarm_timestamp);
	p->tm_year = p->tm_year + 1900;
	p->tm_mon = p->tm_mon + 1;

	snprintf(strtime, sizeof(strtime), "%04d-%02d-%02d %02d:%02d:%02d", p->tm_year, p->tm_mon, p->tm_mday,p->tm_hour,p->tm_min,p->tm_sec);


	JSONNode out;
	out.push_back(JSONNode(JK_P2RELAY_MESSAGE_TYPE, SEND_MESSAGE_TO_USER));
	out.push_back(JSONNode(JK_LOGIC_PROCESS_TYPE, 10));
	out.push_back(JSONNode(JK_USERNAME, username));

	JSONNode item;
	item.push_back(JSONNode(JK_MESSAGE_ID, MID_RESPONSE_PUSHALARM));
	item.push_back(JSONNode(JK_ALARM_SEND_GUID, alarm_info.alarm_guid));
	item.push_back(JSONNode(JK_ALARM_SEND_CLOUDNUM, alarm_info.device_guid));
	item.push_back(JSONNode(JK_ALARM_SEND_CLOUDNAME, alarm_info.device_name));
	item.push_back(JSONNode(JK_ALARM_SEND_CLOUDCHN, alarm_info.alarm_channel_no));
	item.push_back(JSONNode(JK_ALARM_SEND_ALARMTYPE, alarm_info.alarm_type));
	item.push_back(JSONNode(JK_ALARM_SEND_ALARMLEVEL, alarm_level));
	item.push_back(JSONNode(JK_ALARM_SEND_ALARMTIME, strtime));
	item.push_back(JSONNode(JK_ALARM_SEND_PICURL, alarm_info.alarm_pic));

	out.push_back(JSONNode("rm", item.write()));

	return out.write();
}

std::string CJsonOpt::JsonJoinPushAlarmMessage(const int message_type, const int result, const int amt, const std::string guid)
{
	JSONNode out;
	out.push_back(JSONNode(JK_MESSAGE_TYPE, message_type));
	out.push_back(JSONNode(JK_RESULT, result));
	out.push_back(JSONNode(JK_MESSAGE_ID, message_id_));
	out.push_back(JSONNode(JK_ALARM_FTP_MESSAGE_TYPE, amt));
	out.push_back(JSONNode(JK_ALARM_FTP_GUID, guid));

	return out.write();
}

std::string CJsonOpt::JsonJoinGetDeviceUsers(const std::string& device_guid)
{
	JSONNode out;
	out.push_back(JSONNode(JK_PROTO_VERSION, PROTO_VERSION));
	out.push_back(JSONNode(JK_LOGIC_PROCESS_TYPE, DEV_INFO_HOMECLOUD));
	out.push_back(JSONNode(JK_MESSAGE_TYPE, GET_DEVICE_USERNAMES));
	out.push_back(JSONNode(JK_DEVICE_GUID, device_guid));

	return out.write();
}

bool CJsonOpt::JsonParseGetDeviceUsers(const std::string& in_json, std::vector<std::string>& users)
{
	JSONNode in;
	try
	{
		in = libjson::parse(in_json);
	}
	catch(...)
	{
		LOG4CXX_ERROR(g_logger, "CJsonOpt::JsonParseGetDeviceUsers invalid_argument");
		return false;
	}

	if (!VerifyJsonField(in, JK_USERNAME))
	{
		return false;
	}

	JSONNode jn_users(JSON_ARRAY);
	jn_users = in[JK_USERNAME].as_array();
	for (unsigned i = 0; i < jn_users.size(); ++i)
	{
		users.push_back(jn_users.at(i).as_string());
	}

	return true;
}