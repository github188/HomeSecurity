/*
 * json_operate.cc
 *
 *  Created on: Feb 12, 2014
 *      Author: zhangs
 */

#include "json_opt.h"
#include "../public/message.h"
#include "defines.h"
#include "../public/utils.h"

CJsonOpt::CJsonOpt(const std::string& json_string)
{
	json_string_= json_string;
	message_id_	= 0;
	sfd_		= 0;
	sfd_id_		= 0;
}
CJsonOpt::CJsonOpt()
{
	json_string_ = std::string();
	message_id_ = 0;
	sfd_ = 0;
	sfd_id_ = 0;
}
CJsonOpt::~CJsonOpt()
{
}

bool CJsonOpt::JsonParseMessageType(int & message_type)
{

	try
	{
		in_ = libjson::parse(json_string_);
	}
	catch (...)
	{
		return false;
	}

	if(!VerifyPublicJson(in_))
		return false;

	message_type = in_[JK_MESSAGE_TYPE].as_int();
	message_id_	 = in_[JK_MESSAGE_ID].as_int();
	//version_	 = in_[JK_PROTO_VERSION].as_string();
	sfd_ 		 = in_[JK_CLINET_SFD].as_int();
	sfd_id_ 	 = in_[JK_CLINET_SFD_ID].as_int();

	return true;
}

bool CJsonOpt::JsonParseSessionId(std::string& session_id)
{
	if(!VerifyJsonField(in_, JK_SESSION_ID))
		return false;

	session_id = in_[JK_SESSION_ID].as_string();
	return true;
}

int CJsonOpt::JsonGetMessageId()
{
	return message_id_;
}

std::string CJsonOpt::JsonJoinCommon(const int message_type, const int result)
{
	JsonJoinPublic(message_type, result);
	return out_.write();
}

void CJsonOpt::setJsonString(const std::string &msg)
{
	out_ = JSONNode();
	json_string_ = msg;
}

void CJsonOpt::JsonJoinPublic(const int message_type, const int result)
{
	out_.push_back(JSONNode(JK_MESSAGE_TYPE, message_type));
	out_.push_back(JSONNode(JK_RESULT, result));
	out_.push_back(JSONNode(JK_MESSAGE_ID, message_id_));
	out_.push_back(JSONNode(JK_CLINET_SFD, sfd_));
	out_.push_back(JSONNode(JK_CLINET_SFD_ID, sfd_id_));
}

bool CJsonOpt::VerifyPublicJson(JSONNode& in)
{
	return VerifyJsonField(in, JK_MESSAGE_ID) && VerifyJsonField(in, JK_MESSAGE_TYPE)
		   && VerifyJsonField(in, JK_CLINET_SFD) && VerifyJsonField(in, JK_CLINET_SFD_ID)
		   && VerifyJsonField(in, JK_PROTO_VERSION);
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

bool CJsonOpt::JsonParseDeviceRegister(DEVICE_INFO &device_info)
{
	if(VerifyJsonField(in_, JK_DEVICE_GUID) && VerifyJsonField(in_, JK_DEVICE_NAME) && VerifyJsonField(in_, JK_DEVICE_TYPE))
	{
		device_info.device_guid = in_[JK_DEVICE_GUID].as_string();
		device_info.device_name = in_[JK_DEVICE_NAME].as_string();
		device_info.device_type = in_[JK_DEVICE_TYPE].as_int();

		if (VerifyJsonField(in_, JK_DEVICE_SUB_TYPE))
		{
			device_info.device_sub_type = in_[JK_DEVICE_SUB_TYPE].as_int();
		}
		if (VerifyJsonField(in_, JK_DEVICE_SOFT_VERSION))
		{
			device_info.device_version = in_[JK_DEVICE_SOFT_VERSION].as_string();
		}
		if (VerifyJsonField(in_, JK_DEVICE_USERNAME))
		{
			device_info.device_username = in_[JK_DEVICE_USERNAME].as_string();
		}
		if (VerifyJsonField(in_, JK_DEVICE_PASSWORD))
		{
			device_info.device_password = in_[JK_DEVICE_PASSWORD].as_string();
		}
		if (VerifyJsonField(in_, JK_DEVICE_IP))
		{
			device_info.device_ip = in_[JK_DEVICE_IP].as_string();
		}
		if (VerifyJsonField(in_, JK_DEVICE_NET_STATE))
		{
			device_info.device_netstat = in_[JK_DEVICE_NET_STATE].as_int();
		}
		if (VerifyJsonField(in_, JK_NET_STORAGE_SWITCH))
		{
			device_info.net_storage_switch = in_[JK_NET_STORAGE_SWITCH].as_int();
		}
		if (VerifyJsonField(in_, JK_TF_STORAGE_SWITCH))
		{
			device_info.tf_storage_switch = in_[JK_TF_STORAGE_SWITCH].as_int();
		}
		if (VerifyJsonField(in_, JK_ALARM_SWITCH))
		{
			device_info.alarm_switch = in_[JK_ALARM_SWITCH].as_int();
		}
		if (VerifyJsonField(in_, JK_ALARM_VIDEO_FTP))
		{
			device_info.alarm_video_ftp_url = in_[JK_ALARM_VIDEO_FTP].as_string();
		}
		if (VerifyJsonField(in_, JK_ALARM_SNAP_FTP))
		{
			device_info.alarm_snap_ftp_url = in_[JK_ALARM_SNAP_FTP].as_string();
		}
		if (VerifyJsonField(in_, JK_ALARM_FTP_ACC))
		{
			device_info.alarm_ftp_acc = in_[JK_ALARM_FTP_ACC].as_string();
		}
		if (VerifyJsonField(in_, JK_ALARM_FTP_PWD))
		{
			device_info.alarm_ftp_pwd = in_[JK_ALARM_FTP_PWD].as_string();
		}
		if (VerifyJsonField(in_, JK_ALARM_TIME))
		{
			device_info.alarm_time = in_[JK_ALARM_TIME].as_string();
		}
		if (VerifyJsonField(in_, JK_PIC_FTP_BIG))
		{
			device_info.jpeg_ftp_url_big = in_[JK_PIC_FTP_BIG].as_string();
		}
		if (VerifyJsonField(in_, JK_PIC_FTP_SMALL))
		{
			device_info.jpeg_ftp_url_small = in_[JK_PIC_FTP_SMALL].as_string();
		}
		if (VerifyJsonField(in_, JK_PIC_FTP_ACC))
		{
			device_info.jpeg_ftp_acc = in_[JK_PIC_FTP_ACC].as_string();
		}
		if (VerifyJsonField(in_, JK_PIC_FTP_PWD))
		{
			device_info.jpeg_ftp_pwd = in_[JK_PIC_FTP_PWD].as_string();
		}
		if (VerifyJsonField(in_, JK_PIC_UPLOAD_TIMEING))
		{
			device_info.jpeg_upload_timing = in_[JK_PIC_UPLOAD_TIMEING].as_int();
		}
		if (VerifyJsonField(in_, JK_VIDEO_FLUENCY))
		{
			device_info.video_fluency = in_[JK_VIDEO_FLUENCY].as_int();
		}
		if (VerifyJsonField(in_, JK_DEVICE_BABY_MODE))
		{
			device_info.baby_mode = in_[JK_DEVICE_BABY_MODE].as_int();
		}
		if (VerifyJsonField(in_, JK_DEVICE_FULL_ALARM_MODE))
		{
			device_info.full_alarm_mode = in_[JK_DEVICE_FULL_ALARM_MODE].as_int();
		}

		return true;
	}

	return false;
}

std::string CJsonOpt::JsonJoinDeviceRegister(const int result)
{
	JsonJoinCommon(DEVICE_REGISTER_RESPONSE, result);
	return out_.write();
}

std::string CJsonOpt::JsonJoinGetUserDevices(const int result, const USER_DEVICES_LIST& user_devices_list)
{
	JsonJoinPublic(GET_USER_DEVICES_RESPONSE, result);

	//JSONNode devicesinfo_json;
	JSONNode devices_json(JSON_ARRAY);
	devices_json.set_name(JK_DEVICE_LIST);
	for (unsigned i = 0; i < user_devices_list.vec_user_device_info.size() && i < DEVICE_LIST_LIMIT; ++i)
	{
		JSONNode device_json;
		device_json.push_back(JSONNode(JK_DEVICE_GUID, user_devices_list.vec_user_device_info.at(i).device_info.device_guid));
		device_json.push_back(JSONNode(JK_DEVICE_TYPE, user_devices_list.vec_user_device_info.at(i).device_info.device_type));
		//device_json.push_back(JSONNode(JK_DEVICE_SUB_TYPE, (*iter).device_info.device_sub_type_str));
		//device_json.push_back(JSONNode(JK_DEVICE_SUB_TYPE_INT, (*iter).device_info.device_sub_type));
		//device_json.push_back(JSONNode(JK_DEVICE_SOFT_VERSION, (*iter).device_info.device_version));
		device_json.push_back(JSONNode(JK_DEVICE_WIFI_FLAG, user_devices_list.vec_user_device_info.at(i).device_info.wifi_flag));
		device_json.push_back(JSONNode(JK_DEVICE_NAME, user_devices_list.vec_user_device_info.at(i).user_device_config.device_name));
		//device_json.push_back(JSONNode(JK_DEVICE_USERNAME, (*iter).device_info.device_username));
		//device_json.push_back(JSONNode(JK_DEVICE_PASSWORD, (*iter).device_info.device_password));
		//device_json.push_back(JSONNode(JK_DEVICE_IP, (*iter).device_info.device_ip));
		//device_json.push_back(JSONNode(JK_DEVICE_NET_STATE, (*iter).device_info.device_netstat));
		//device_json.push_back(JSONNode(JK_NET_STORAGE_SWITCH, (*iter).device_info.net_storage_switch));
		//device_json.push_back(JSONNode(JK_TF_STORAGE_SWITCH, (*iter).device_info.tf_storage_switch));
		//device_json.push_back(JSONNode(JK_ALARM_SWITCH, (*iter).device_info.alarm_switch));
		//device_json.push_back(JSONNode(JK_ALARM_VIDEO_FTP, (*iter).device_info.alarm_video_ftp_url));
		//device_json.push_back(JSONNode(JK_ALARM_SNAP_FTP, (*iter).device_info.alarm_snap_ftp_url));
		//device_json.push_back(JSONNode(JK_ALARM_FTP_ACC, (*iter).device_info.alarm_ftp_acc));
		//device_json.push_back(JSONNode(JK_ALARM_FTP_PWD, (*iter).device_info.alarm_ftp_pwd));
		//device_json.push_back(JSONNode(JK_ALARM_TIME, (*iter).device_info.alarm_time));
		//device_json.push_back(JSONNode(JK_PIC_FTP_BIG, (*iter).device_info.jpeg_ftp_url_big));
		//device_json.push_back(JSONNode(JK_PIC_FTP_SMALL, (*iter).device_info.jpeg_ftp_url_small));
		//device_json.push_back(JSONNode(JK_PIC_FTP_ACC, (*iter).device_info.jpeg_ftp_acc));
		//device_json.push_back(JSONNode(JK_PIC_FTP_PWD, (*iter).device_info.jpeg_ftp_pwd));
		//device_json.push_back(JSONNode(JK_PIC_UPLOAD_TIMEING, (*iter).device_info.jpeg_upload_timing));
		//device_json.push_back(JSONNode(JK_VIDEO_FLUENCY, (*iter).device_info.video_fluency));
		device_json.push_back(JSONNode(JK_DEVICES_ONLINE_STATUS, user_devices_list.vec_user_device_info.at(i).device_info.online_status));
		//device_json.push_back(JSONNode(JK_DEVICE_BABY_MODE, (*iter).device_info.baby_mode));
		//device_json.push_back(JSONNode(JK_DEVICE_FULL_ALARM_MODE, (*iter).device_info.full_alarm_mode));
		//device_json.push_back(JSONNode(JK_DEVICE_RESET_FLAG, (*iter).device_info.reset_flag));
		device_json.push_back(JSONNode(JK_VIDEO_LINK_TYPE, user_devices_list.vec_user_device_info.at(i).user_device_config.video_link_type));
		device_json.push_back(JSONNode(JK_DEVICE_VIDEO_USERNAME, user_devices_list.vec_user_device_info.at(i).user_device_config.video_username));
		device_json.push_back(JSONNode(JK_DEVICE_VIDEO_PASSWORD, user_devices_list.vec_user_device_info.at(i).user_device_config.video_password));
		device_json.push_back(JSONNode(JK_DEVICE_VIDEO_IP, user_devices_list.vec_user_device_info.at(i).user_device_config.video_ip));
		device_json.push_back(JSONNode(JK_DEVICE_VIDEO_PORT, user_devices_list.vec_user_device_info.at(i).user_device_config.video_port));
		devices_json.push_back(device_json);
	}

	out_.push_back(devices_json);

	// to utf8
	std::string out_utf = out_.write();
	utils::Unicode2UTF8(out_utf);

	return out_utf;
}

bool CJsonOpt::JsonParseGetDeviceInfo(std::string& device_guid)
{
	if (!VerifyJsonField(in_, JK_DEVICE_GUID))
	{
		return false;
	}

	device_guid = in_[JK_DEVICE_GUID].as_string();

	return true;
}

std::string CJsonOpt::JsonJoinGetDeviceUsers(const int result, const std::vector<std::string>& vec_un, const std::string& device_name, const std::string& alarm_time, const int full_alarm_mode)
{
	JsonJoinPublic(GET_DEVICE_USERNAMES_RESPONSE, result);

	out_.push_back(JSONNode(JK_DEVICE_NAME, device_name));
	out_.push_back(JSONNode(JK_ALARM_TIME, alarm_time));
	out_.push_back(JSONNode(JK_DEVICE_FULL_ALARM_MODE, full_alarm_mode));

	JSONNode n(JSON_ARRAY);
	n.set_name(JK_USERNAME);
	std::vector<std::string>::const_iterator it;
	for(it = vec_un.begin(); it != vec_un.end(); ++it)
		n.push_back(JSONNode("", *it));

	out_.push_back(n);

	return out_.write();
}

std::string CJsonOpt::JsonJoinGetDeviceInfo(const int result, const DEVICE_INFO& device_info)
{
	JsonJoinPublic(GET_DEVICE_INFO_RESPONSE, result);

	JSONNode json_deviceinfo(JSON_NODE);
	json_deviceinfo.set_name(JK_DEVICE_INFO);

	json_deviceinfo.push_back(JSONNode(JK_DEVICE_NAME, device_info.device_name));
	json_deviceinfo.push_back(JSONNode(JK_DEVICE_TYPE, device_info.device_type));
	//json_deviceinfo.push_back(JSONNode(JK_DEVICE_SUB_TYPE, device_info.device_sub_type_str));
	//json_deviceinfo.push_back(JSONNode(JK_DEVICE_SUB_TYPE_INT, device_info.device_sub_type));
	//json_deviceinfo.push_back(JSONNode(JK_DEVICE_SOFT_VERSION, device_info.device_version));
	//json_deviceinfo.push_back(JSONNode(JK_DEVICE_USERNAME, device_info.device_username));
	//json_deviceinfo.push_back(JSONNode(JK_DEVICE_PASSWORD, device_info.device_password));
	//json_deviceinfo.push_back(JSONNode(JK_DEVICE_IP, device_info.device_ip));
	//json_deviceinfo.push_back(JSONNode(JK_DEVICE_NET_STATE, device_info.device_netstat));
	//json_deviceinfo.push_back(JSONNode(JK_NET_STORAGE_SWITCH, device_info.net_storage_switch));
	//json_deviceinfo.push_back(JSONNode(JK_TF_STORAGE_SWITCH, device_info.tf_storage_switch));
	//json_deviceinfo.push_back(JSONNode(JK_ALARM_SWITCH, device_info.alarm_switch));
	//json_deviceinfo.push_back(JSONNode(JK_ALARM_VIDEO_FTP, device_info.alarm_video_ftp_url));
	//json_deviceinfo.push_back(JSONNode(JK_ALARM_SNAP_FTP, device_info.alarm_snap_ftp_url));
	//json_deviceinfo.push_back(JSONNode(JK_ALARM_FTP_ACC, device_info.alarm_ftp_acc));
	//json_deviceinfo.push_back(JSONNode(JK_ALARM_FTP_PWD, device_info.alarm_ftp_pwd));
	//json_deviceinfo.push_back(JSONNode(JK_ALARM_TIME, device_info.alarm_time));
	//json_deviceinfo.push_back(JSONNode(JK_PIC_FTP_BIG, device_info.jpeg_ftp_url_big));
	//json_deviceinfo.push_back(JSONNode(JK_PIC_FTP_SMALL, device_info.jpeg_ftp_url_small));
	//json_deviceinfo.push_back(JSONNode(JK_PIC_FTP_ACC, device_info.jpeg_ftp_acc));
	//json_deviceinfo.push_back(JSONNode(JK_PIC_FTP_PWD, device_info.jpeg_ftp_pwd));
	//json_deviceinfo.push_back(JSONNode(JK_PIC_UPLOAD_TIMEING, device_info.jpeg_upload_timing));
	//json_deviceinfo.push_back(JSONNode(JK_VIDEO_FLUENCY, device_info.video_fluency));
	//json_deviceinfo.push_back(JSONNode(JK_DEVICE_BABY_MODE, device_info.baby_mode));
	//json_deviceinfo.push_back(JSONNode(JK_DEVICE_FULL_ALARM_MODE, device_info.full_alarm_mode));

	out_.push_back(json_deviceinfo);

	// to utf8
	std::string out_utf = out_.write();
	utils::Unicode2UTF8(out_utf);

	return out_utf;
}

bool CJsonOpt::JsonParseGetUserDeviceInfo(std::string& device_guid)
{
	if (!VerifyJsonField(in_, JK_DEVICE_GUID))
	{
		return false;
	}

	device_guid = in_[JK_DEVICE_GUID].as_string();

	return true;
}

std::string CJsonOpt::JsonJoinGetUserDeviceInfo(const int result, const USER_DEVICE_INFO& user_device_info)
{
	JsonJoinPublic(GET_USER_DEVICE_INFO_RESPONSE, result);

	JSONNode json_deviceinfo(JSON_NODE);
	json_deviceinfo.set_name(JK_DEVICE_INFO);

	json_deviceinfo.push_back(JSONNode(JK_DEVICE_NAME, user_device_info.user_device_config.device_name));
	json_deviceinfo.push_back(JSONNode(JK_DEVICE_TYPE, user_device_info.device_info.device_type));
	//json_deviceinfo.push_back(JSONNode(JK_DEVICE_SUB_TYPE, user_device_info.device_info.device_sub_type_str));
	//json_deviceinfo.push_back(JSONNode(JK_DEVICE_SUB_TYPE_INT, user_device_info.device_info.device_sub_type));
	//json_deviceinfo.push_back(JSONNode(JK_DEVICE_SOFT_VERSION, user_device_info.device_info.device_version));
	//json_deviceinfo.push_back(JSONNode(JK_DEVICE_USERNAME, user_device_info.device_info.device_username));
	//json_deviceinfo.push_back(JSONNode(JK_DEVICE_PASSWORD, user_device_info.device_info.device_password));
	//json_deviceinfo.push_back(JSONNode(JK_DEVICE_IP, user_device_info.device_info.device_ip));
	//json_deviceinfo.push_back(JSONNode(JK_DEVICE_NET_STATE, user_device_info.device_info.device_netstat));
	//json_deviceinfo.push_back(JSONNode(JK_NET_STORAGE_SWITCH, user_device_info.device_info.net_storage_switch));
	//json_deviceinfo.push_back(JSONNode(JK_TF_STORAGE_SWITCH, user_device_info.device_info.tf_storage_switch));
	//json_deviceinfo.push_back(JSONNode(JK_ALARM_SWITCH, user_device_info.device_info.alarm_switch));
	//json_deviceinfo.push_back(JSONNode(JK_ALARM_VIDEO_FTP, user_device_info.device_info.alarm_video_ftp_url));
	//json_deviceinfo.push_back(JSONNode(JK_ALARM_SNAP_FTP, user_device_info.device_info.alarm_snap_ftp_url));
	//json_deviceinfo.push_back(JSONNode(JK_ALARM_FTP_ACC, user_device_info.device_info.alarm_ftp_acc));
	//json_deviceinfo.push_back(JSONNode(JK_ALARM_FTP_PWD, user_device_info.device_info.alarm_ftp_pwd));
	//json_deviceinfo.push_back(JSONNode(JK_ALARM_TIME, user_device_info.device_info.alarm_time));
	//json_deviceinfo.push_back(JSONNode(JK_PIC_FTP_BIG, user_device_info.device_info.jpeg_ftp_url_big));
	//json_deviceinfo.push_back(JSONNode(JK_PIC_FTP_SMALL, user_device_info.device_info.jpeg_ftp_url_small));
	//json_deviceinfo.push_back(JSONNode(JK_PIC_FTP_ACC, user_device_info.device_info.jpeg_ftp_acc));
	//json_deviceinfo.push_back(JSONNode(JK_PIC_FTP_PWD, user_device_info.device_info.jpeg_ftp_pwd));
	//json_deviceinfo.push_back(JSONNode(JK_PIC_UPLOAD_TIMEING, user_device_info.device_info.jpeg_upload_timing));
	//json_deviceinfo.push_back(JSONNode(JK_VIDEO_FLUENCY, user_device_info.device_info.video_fluency));
	//json_deviceinfo.push_back(JSONNode(JK_DEVICE_BABY_MODE, user_device_info.device_info.baby_mode));
	//json_deviceinfo.push_back(JSONNode(JK_DEVICE_FULL_ALARM_MODE, user_device_info.device_info.full_alarm_mode));
	//json_deviceinfo.push_back(JSONNode(JK_DEVICES_ONLINE_STATUS, user_device_info.device_info.online_status));
	json_deviceinfo.push_back(JSONNode(JK_VIDEO_LINK_TYPE, user_device_info.user_device_config.video_link_type));
	json_deviceinfo.push_back(JSONNode(JK_DEVICE_VIDEO_USERNAME, user_device_info.user_device_config.video_username));
	json_deviceinfo.push_back(JSONNode(JK_DEVICE_VIDEO_PASSWORD, user_device_info.user_device_config.video_password));
	json_deviceinfo.push_back(JSONNode(JK_DEVICE_VIDEO_IP, user_device_info.user_device_config.video_ip));
	json_deviceinfo.push_back(JSONNode(JK_DEVICE_VIDEO_PORT, user_device_info.user_device_config.video_port));

	out_.push_back(json_deviceinfo);

	// to utf8
	std::string out_utf = out_.write();
	utils::Unicode2UTF8(out_utf);

	return out_utf;
}

bool CJsonOpt::JsonParseModifyDeviceInfoVideoLink(std::string& device_guid, int& video_link_type, std::string& video_username, std::string& video_password, std::string& video_ip, int& video_port)
{
	if (VerifyJsonField(in_, JK_DEVICE_GUID) && VerifyJsonField(in_, JK_VIDEO_LINK_TYPE) && VerifyJsonField(in_, JK_DEVICE_VIDEO_USERNAME) && VerifyJsonField(in_, JK_DEVICE_VIDEO_PASSWORD) && VerifyJsonField(in_, JK_DEVICE_VIDEO_IP) && VerifyJsonField(in_, JK_DEVICE_VIDEO_PORT))
	{
		device_guid = in_[JK_DEVICE_GUID].as_string();
		video_link_type = in_[JK_VIDEO_LINK_TYPE].as_int();
		video_username = in_[JK_DEVICE_VIDEO_USERNAME].as_string();
		video_password = in_[JK_DEVICE_VIDEO_PASSWORD].as_string();
		video_ip = in_[JK_DEVICE_VIDEO_IP].as_string();
		video_port = in_[JK_DEVICE_VIDEO_PORT].as_int();

		return true;
	}

	return false;
}

std::string CJsonOpt::JsonJoinModifyDeviceInfoVideoLink(const int result)
{
	JsonJoinCommon(MODIFY_DEVICE_INFO_VIDEO_LINK_RESPONSE, result);
	return out_.write();
}

bool CJsonOpt::JsonParseUserBindDevice(std::string& device_guid, std::string& video_username, std::string& video_password)
{
	if (!(VerifyJsonField(in_, JK_DEVICE_GUID) && VerifyJsonField(in_, JK_DEVICE_VIDEO_USERNAME) && VerifyJsonField(in_, JK_DEVICE_VIDEO_PASSWORD)))
	{
		return false;
	}

	device_guid = in_[JK_DEVICE_GUID].as_string();
	video_username = in_[JK_DEVICE_VIDEO_USERNAME].as_string();
	video_password = in_[JK_DEVICE_VIDEO_PASSWORD].as_string();

	return true;
}

std::string CJsonOpt::JsonJoinUserBindDevice(const int result, const int reset_flag)
{
	JsonJoinCommon(USER_BIND_DEVICE_RESPONSE, result);
	//out_.push_back(JSONNode(JK_DEVICE_RESET_FLAG, reset_flag));
	return out_.write();
}

bool CJsonOpt::JsonParseUserRemoveBindDevice(std::string& device_guid)
{
	if (!VerifyJsonField(in_, JK_DEVICE_GUID))
	{
		return false;
	}

	device_guid = in_[JK_DEVICE_GUID].as_string();

	return true;
}

std::string CJsonOpt::JsonJoinUserRemoveBindDevice(const int result)
{
	JsonJoinCommon(USER_REMOVE_BIND_DEVICE_RESPONSE, result);
	return out_.write();
}

bool CJsonOpt::JsonParseGetDeviceHumiture(std::string& device_guid, int& getNum)
{
	if(VerifyJsonField(in_, JK_DEVICE_GUID) && VerifyJsonField(in_, JK_DEVICE_HUMITURE_NUM))
	{
		device_guid = in_[JK_DEVICE_GUID].as_string();
		getNum = in_[JK_DEVICE_HUMITURE_NUM].as_int();

		return true;
	}

	return false;
}

std::string CJsonOpt::JsonJoinGetDeviceHumiture(const int result, const std::vector<HUMITURE_STAT_INFO>& vec_humiture_stat_info)
{
	JsonJoinCommon(GET_DEVICE_HUMITURE_STAT_RESPONSE, result);

	JSONNode arr_humiture_json(JSON_ARRAY);
	arr_humiture_json.set_name(JK_DEVICE_HUMITURE_LIST);
	for (unsigned i = 0; i < vec_humiture_stat_info.size(); ++i)
	{
		JSONNode humiture_json;
		humiture_json.push_back(JSONNode(JK_DEVICE_HUMITURE_DATE, vec_humiture_stat_info.at(i).date));
		humiture_json.push_back(JSONNode(JK_DEVICE_HUMITURE_HOUR, vec_humiture_stat_info.at(i).hour));
		humiture_json.push_back(JSONNode(JK_DEVICE_TEMPERATURE, vec_humiture_stat_info.at(i).statTemperature));
		humiture_json.push_back(JSONNode(JK_DEVICE_HUMIDNESS, vec_humiture_stat_info.at(i).statHumidity));
		humiture_json.push_back(JSONNode(JK_DEVICE_HUMITURE_SCORE, vec_humiture_stat_info.at(i).score));
		humiture_json.push_back(JSONNode(JK_DEVICE_HUMITURE_ASSESSMENT, vec_humiture_stat_info.at(i).assessment));
		arr_humiture_json.push_back(humiture_json);
	}

	out_.push_back(arr_humiture_json);
	return out_.write();
}

bool CJsonOpt::JsonParseGetDeviceHumitureOntime(std::string& device_guid)
{
	if (!(VerifyJsonField(in_, JK_DEVICE_GUID)))
	{
		return false;
	}

	device_guid = in_[JK_DEVICE_GUID].as_string();

	return true;
}

std::string CJsonOpt::JsonJoinGetDeviceHumitureOntime(const int result, const DEVICE_ENV_INFO& device_env_info)
{
	JsonJoinCommon(GET_DEVICE_HUMITURE_ONTIME_RESPONSE, result);

	char timestamp[32] = {0};
	strftime(timestamp, sizeof (timestamp), "%Y-%m-%d %H:%M:%S", &device_env_info.timestamp);
	out_.push_back(JSONNode(JK_DEVICE_TIMESTAMP, std::string(timestamp)));
	out_.push_back(JSONNode(JK_DEVICE_TEMPERATURE, device_env_info.temperature));
	out_.push_back(JSONNode(JK_DEVICE_HUMIDNESS, device_env_info.humidity));

	return out_.write();
}

std::string CJsonOpt::JsonJoinGetUserDevicesStatusInfo(const int result, const USER_DEVICES_LIST& userDevicesList)
{
	JsonJoinCommon(GET_USER_DEVICES_STATUS_INFO_RESPONSE, result);
	
	JSONNode devices_json(JSON_ARRAY);
	devices_json.set_name(JK_DEVICE_LIST);

	//char timestamp[32] = {0};
	for (unsigned i = 0; i < userDevicesList.vec_user_device_info.size() && i < DEVICE_LIST_LIMIT; ++i)
	{
		//memset(timestamp, 0, sizeof (timestamp));
		//strftime(timestamp, sizeof (timestamp), "%Y-%m-%d %H:%M:%S", &userDevicesList.vec_user_device_info.at(i).device_info.timestamp);

		JSONNode device_json;
		device_json.push_back(JSONNode(JK_DEVICE_GUID, userDevicesList.vec_user_device_info.at(i).device_info.device_guid));
		device_json.push_back(JSONNode(JK_DEVICE_WIFI_FLAG, userDevicesList.vec_user_device_info.at(i).device_info.wifi_flag));
		//device_json.push_back(JSONNode(JK_DEVICE_NAME, (*iter).user_device_config.device_name));
		//device_json.push_back(JSONNode(JK_DEVICE_NET_STATE, (*iter).device_netstat));
		//device_json.push_back(JSONNode(JK_NET_STORAGE_SWITCH, (*iter).net_storage_switch));
		//device_json.push_back(JSONNode(JK_TF_STORAGE_SWITCH, (*iter).tf_storage_switch));
		//device_json.push_back(JSONNode(JK_ALARM_SWITCH, (*iter).alarm_switch));
		device_json.push_back(JSONNode(JK_DEVICES_ONLINE_STATUS, userDevicesList.vec_user_device_info.at(i).device_info.online_status));
		//device_json.push_back(JSONNode(JK_DEVICE_TIMESTAMP, std::string(timestamp)));
		//device_json.push_back(JSONNode(JK_DEVICE_TEMPERATURE, (*iter).temperature));
		//device_json.push_back(JSONNode(JK_DEVICE_HUMIDNESS, (*iter).humidity));
		//device_json.push_back(JSONNode(JK_DEVICE_RESET_FLAG, (*iter).reset_flag));
		//device_json.push_back(JSONNode(JK_DEVICE_VERIFY, (*iter).device_verify));
		
		devices_json.push_back(device_json);
	}

	out_.push_back(devices_json);
	
	// to utf8
	std::string out_utf = out_.write();
	utils::Unicode2UTF8(out_utf);

	return out_utf;
}

bool CJsonOpt::JsonParseGetDeviceHumitureScore(std::string& device_guid)
{
	if (!(VerifyJsonField(in_, JK_DEVICE_GUID)))
	{
		return false;
	}

	device_guid = in_[JK_DEVICE_GUID].as_string();

	return true;
}

std::string CJsonOpt::JsonJoinGetDeviceHumitureScore(const int result, const int top, const std::string& ratio, const int last_score, const int last_top, const DEVICE_ENV_INFO& device_env_info)
{
	JsonJoinCommon(GET_DEVICE_HUMITURE_SCORE_RESPONSE, result);

	out_.push_back(JSONNode(JK_DEVICE_ENV_SCORE, device_env_info.score));
	out_.push_back(JSONNode(JK_DEVICE_HUMITURE_SCORE, device_env_info.score));
	out_.push_back(JSONNode(JK_DEVICE_HUMITURE_TOP, top));
	out_.push_back(JSONNode(JK_DEVICE_HUMITURE_RATIO, ratio));
	out_.push_back(JSONNode(JK_DEVICE_TEMPERATURE, device_env_info.temperature));
	out_.push_back(JSONNode(JK_DEVICE_HUMIDNESS, device_env_info.humidity));
	out_.push_back(JSONNode(JK_DEVICE_HUMITURE_LAST_SCORE, last_score));
	out_.push_back(JSONNode(JK_DEVICE_HUMITURE_LAST_TOP, last_top));
	out_.push_back(JSONNode(JK_DEVICE_HUMITURE_ASSESSMENT, device_env_info.assessment));

	return out_.write();
}

bool CJsonOpt::JsonParseModifyDeviceConfInfo(std::string& device_guid, std::string& device_name, std::string& video_username, std::string& video_password)
{
	if (!(VerifyJsonField(in_, JK_DEVICE_GUID) && VerifyJsonField(in_, JK_DEVICE_NAME) && VerifyJsonField(in_, JK_DEVICE_VIDEO_USERNAME) && VerifyJsonField(in_, JK_DEVICE_VIDEO_PASSWORD)))
	{
		return false;
	}

	device_guid = in_[JK_DEVICE_GUID].as_string();
	device_name = in_[JK_DEVICE_NAME].as_string();
	video_username = in_[JK_DEVICE_VIDEO_USERNAME].as_string();
	video_password = in_[JK_DEVICE_VIDEO_PASSWORD].as_string();

	return true;
}

std::string CJsonOpt::JsonJoinModifyDeviceConfInfo(const int result)
{
	JsonJoinCommon(MODIFY_DEVICE_CONF_INFO_RESPONSE, result);

	return out_.write();
}

bool CJsonOpt::JsonParseModifyDeviceInfoAdvanced(std::string& device_guid, int& net_storage_switch, int& tf_storage_switch, int& alarm_switch, std::string& alarm_time, int& baby_mode, int& full_alarm_mode)
{
	if (!VerifyJsonField(in_, JK_DEVICE_GUID))
	{
		return false;
	}
	device_guid = in_[JK_DEVICE_GUID].as_string();

	if (VerifyJsonField(in_, JK_DEVICE_NET_STATE))
	{
		net_storage_switch = in_[JK_DEVICE_NET_STATE].as_int();
	}
	if (VerifyJsonField(in_, JK_TF_STORAGE_SWITCH))
	{
		tf_storage_switch = in_[JK_TF_STORAGE_SWITCH].as_int();
	}
	if (VerifyJsonField(in_, JK_ALARM_SWITCH))
	{
		alarm_switch = in_[JK_ALARM_SWITCH].as_int();
	}
	if (VerifyJsonField(in_, JK_ALARM_TIME))
	{
		alarm_time = in_[JK_ALARM_TIME].as_string();
	}
	if (VerifyJsonField(in_, JK_DEVICE_BABY_MODE))
	{
		baby_mode = in_[JK_DEVICE_BABY_MODE].as_int();
	}
	if (VerifyJsonField(in_, JK_DEVICE_FULL_ALARM_MODE))
	{
		full_alarm_mode = in_[JK_DEVICE_FULL_ALARM_MODE].as_int();
	}

	return true;
}

std::string CJsonOpt::JsonJoinModifyDeviceInfoAdvanced(const int result)
{
	JsonJoinCommon(MODIFY_DEVICE_INFO_ADVANCED_RESPONSE, result);

	return out_.write();
}

bool CJsonOpt::JsonParseGetDeviceUpdateInfo(std::string& client_version, int& device_type, int& device_sub_type)
{
	if (!(VerifyJsonField(in_, JK_DEVICE_SOFT_VERSION) && VerifyJsonField(in_, JK_DEVICE_TYPE) && VerifyJsonField(in_, JK_DEVICE_SUB_TYPE)))
		return false;

	client_version 	= in_[JK_DEVICE_SOFT_VERSION].as_string();
	device_type = in_[JK_DEVICE_TYPE].as_int();
	device_sub_type = in_[JK_DEVICE_SUB_TYPE].as_int();

	return true;
}

std::string CJsonOpt::JsonJoinGetDeviceUpdateInfo(const int result, const UPDATE_FILE_INFO& updateFileInfo)
{
	JsonJoinCommon(GET_DEVICE_UPDATE_INFO_RESPONSE, result);

	JSONNode out_out;
	out_out.set_name(JK_UPDATE_FILE_INFO);
	out_out.push_back(JSONNode(JK_UPGRADE_FILE_VERSION, updateFileInfo.update_version));
	out_out.push_back(JSONNode(JK_UPGRADE_FILE_URL, updateFileInfo.file_url));
	out_out.push_back(JSONNode(JK_UPGRADE_FILE_SIZE, updateFileInfo.file_size));
	out_out.push_back(JSONNode(JK_UPGRADE_FILE_CHECKSUM, updateFileInfo.file_checksum));
	out_out.push_back(JSONNode(JK_UPGRADE_FILE_DESCRIPTION, updateFileInfo.file_description));

	out_.push_back(out_out);

	// to utf8
	std::string out_utf = out_.write();
	utils::Unicode2UTF8(out_utf);

	return out_utf;
}

bool CJsonOpt::JsonParseModifyDevicePassword(std::string& device_guid, std::string& device_username, std::string& device_password)
{
	if (!(VerifyJsonField(in_, JK_DEVICE_GUID) && VerifyJsonField(in_, JK_DEVICE_USERNAME) && VerifyJsonField(in_, JK_DEVICE_PASSWORD)))
		return false;

	device_guid = in_[JK_DEVICE_GUID].as_string();
	device_username = in_[JK_DEVICE_USERNAME].as_string();
	device_password = in_[JK_DEVICE_PASSWORD].as_string();

	return true;
}

std::string CJsonOpt::JsonJoinModifyDevicePassword(const int result)
{
	JsonJoinCommon(MODIFY_DEVICE_PASSWORD_RESPONSE, result);

	return out_.write();
}

bool CJsonOpt::JsonParseAddDeviceChannel(std::string& device_guid, int& add_sum, std::string& channel_name)
{
	if (!VerifyJsonField(in_, JK_DEVICE_GUID))
		return false;

	device_guid = in_[JK_DEVICE_GUID].as_string();

	if (VerifyJsonField(in_, JK_DEVICE_CHANNEL_SUM))
	{
		add_sum = in_[JK_DEVICE_CHANNEL_SUM].as_int();
	}
	else
	{
		add_sum = 1;
	}
	
	if (VerifyJsonField(in_, JK_DEVICE_CHANNEL_NAME))
	{
		channel_name = in_[JK_DEVICE_CHANNEL_NAME].as_string();
	}
	else
	{
		channel_name = "";
	}

	return true;
}

std::string CJsonOpt::JsonJoinAddDeviceChannel(const int result)
{
	JsonJoinCommon(ADD_DEVICE_CHANNEL_RESPONSE, result);
	return out_.write();
}

bool CJsonOpt::JsonParseDeleteDeviceChannel(std::string& device_guid, int& channel_no)
{
	if (!(VerifyJsonField(in_, JK_DEVICE_GUID) && VerifyJsonField(in_, JK_DEVICE_CHANNEL_NO)))
		return false;

	device_guid = in_[JK_DEVICE_GUID].as_string();
	channel_no = in_[JK_DEVICE_CHANNEL_NO].as_int();

	return true;
}

std::string CJsonOpt::JsonJoinDeleteDeviceChannel(const int result)
{
	JsonJoinCommon(DELETE_DEVICE_CHANNEL_RESPONSE, result);
	return out_.write();
}

bool CJsonOpt::JsonParseGetDeviceChannel(std::string& device_guid)
{
	if (!VerifyJsonField(in_, JK_DEVICE_GUID))
		return false;

	device_guid = in_[JK_DEVICE_GUID].as_string();

	return true;
}

std::string CJsonOpt::JsonJoinGetDeviceChannel(const int result, const std::vector<DEVICE_CHANNEL_INFO>& vec_device_channel)
{
	JsonJoinPublic(GET_DEVICE_CHANNEL_RESPONSE, result);

	JSONNode channels_json(JSON_ARRAY);
	channels_json.set_name(JK_CHANNEL_LIST);

	for (unsigned i = 0; i < vec_device_channel.size(); ++i)
	{
		JSONNode channel_json;
		channel_json.push_back(JSONNode(JK_DEVICE_CHANNEL_NO, vec_device_channel.at(i).channel_no));
		channel_json.push_back(JSONNode(JK_DEVICE_CHANNEL_NAME, vec_device_channel.at(i).channel_name));
		channels_json.push_back(channel_json);
	}
	out_.push_back(channels_json);

	// to utf8
	std::string out_utf = out_.write();
	utils::Unicode2UTF8(out_utf);

	return out_utf;
}

bool CJsonOpt::JsonParseGetUserChannels(std::string& username)
{
	if (!VerifyJsonField(in_, JK_USERNAME))
		return false;

	username = in_[JK_USERNAME].as_string();

	return true;
}

std::string CJsonOpt::JsonJoinGetUserChannels(const int result, const std::vector<DEVICE_CHANNEL_INFO>& vec_user_channels)
{
	JsonJoinPublic(GET_USER_CHANNELS_RESPONSE, result);

	JSONNode channels_json(JSON_ARRAY);
	channels_json.set_name(JK_CHANNEL_LIST);

	for (unsigned i = 0; i < vec_user_channels.size(); ++i)
	{
		JSONNode channel_json;
		channel_json.push_back(JSONNode(JK_DEVICE_GUID, vec_user_channels.at(i).device_guid));
		channel_json.push_back(JSONNode(JK_DEVICE_CHANNEL_NO, vec_user_channels.at(i).channel_no));
		channel_json.push_back(JSONNode(JK_DEVICE_CHANNEL_NAME, vec_user_channels.at(i).channel_name));
		channels_json.push_back(channel_json);
	}
	out_.push_back(channels_json);

	// to utf8
	std::string out_utf = out_.write();
	utils::Unicode2UTF8(out_utf);

	return out_utf;
}

bool CJsonOpt::JsonParseModifyDeviceChannelName(std::string& device_guid, int& channel_no, std::string& channel_name)
{
	if (!(VerifyJsonField(in_, JK_DEVICE_GUID) && VerifyJsonField(in_, JK_DEVICE_CHANNEL_NO) && VerifyJsonField(in_, JK_DEVICE_CHANNEL_NAME)))
		return false;

	device_guid = in_[JK_DEVICE_GUID].as_string();
	channel_no = in_[JK_DEVICE_CHANNEL_NO].as_int();
	channel_name = in_[JK_DEVICE_CHANNEL_NAME].as_string();

	return true;
}

std::string CJsonOpt::JsonJoinModifyDeviceChannelName(const int result)
{
	JsonJoinCommon(MODIFY_DEVICE_CHANNEL_NAME_RESPONSE, result);
	return out_.write();
}

bool CJsonOpt::JsonParseModifyDeviceWifiFlag(std::string& device_guid, int& wifi_flag)
{
	if (!(VerifyJsonField(in_, JK_DEVICE_GUID) && VerifyJsonField(in_, JK_DEVICE_WIFI_FLAG)))
		return false;

	device_guid = in_[JK_DEVICE_GUID].as_string();
	wifi_flag = in_[JK_DEVICE_WIFI_FLAG].as_int();

	return true;
}

std::string CJsonOpt::JsonJoinModifyDeviceWifiFlag(const int result)
{
	JsonJoinCommon(MODIFY_DEVICE_WIFI_FLAG_RESPONSE, result);
	return out_.write();
}
