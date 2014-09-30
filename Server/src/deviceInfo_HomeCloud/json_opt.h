/*
 * json_operate.h
 *
 *  Created on: Feb 12, 2014
 *      Author: zhangs
 */

#ifndef JSON_OPERATE_H_
#define JSON_OPERATE_H_

#include <libjson/libjson.h>
#include <libjson/Source/JSONNode.h>
#include "../public/user_interface_defines.h"
#include <set>

class CJsonOpt
{
public:
	CJsonOpt();
	explicit CJsonOpt(const std::string& json_string);
	virtual ~CJsonOpt();

public:

	bool JsonParseMessageType(int& message_type);
	bool JsonParseSessionId(std::string& session_id);
	std::string JsonJoinCommon(const int message_type, const int result);
	void setJsonString(const std::string &);

	bool JsonParseDeviceRegister(DEVICE_INFO &device_info);
	std::string JsonJoinDeviceRegister(const int result);

	std::string JsonJoinGetUserDevices(const int result, const USER_DEVICES_LIST& user_devices_list);

	bool JsonParseGetDeviceInfo(std::string& device_guid);
	std::string JsonJoinGetDeviceInfo(const int result, const DEVICE_INFO& device_info);

	bool JsonParseGetUserDeviceInfo(std::string& device_guid);
	std::string JsonJoinGetUserDeviceInfo(const int result, const USER_DEVICE_INFO& user_device_info);

	bool JsonParseModifyDeviceInfoVideoLink(std::string& device_guid, int& video_link_type, std::string& video_username, std::string& video_password, std::string& video_ip, int& video_port);
	std::string JsonJoinModifyDeviceInfoVideoLink(const int result);

	bool JsonParseUserBindDevice(std::string& device_guid, std::string& video_username, std::string& video_password);
	std::string JsonJoinUserBindDevice(const int result, const int reset_flag);

	bool JsonParseUserRemoveBindDevice(std::string& device_guid);
	std::string JsonJoinUserRemoveBindDevice(const int result);

	bool JsonParseGetDeviceHumiture(std::string& device_guid, int& getNum);
	std::string JsonJoinGetDeviceHumiture(const int result, const std::vector<HUMITURE_STAT_INFO>& vec_humiture_stat_info);

	bool JsonParseGetDeviceHumitureOntime(std::string& device_guid);
	std::string JsonJoinGetDeviceHumitureOntime(const int result, const DEVICE_ENV_INFO& device_env_info);

	std::string JsonJoinGetUserDevicesStatusInfo(const int result, const DEVICES_LIST& devicesList);

	bool JsonParseGetDeviceHumitureScore(std::string& device_guid);
	std::string JsonJoinGetDeviceHumitureScore(const int result, const int top, const std::string& ratio, const int last_score, const int last_top, const DEVICE_ENV_INFO& device_env_info);

	std::string JsonJoinGetDeviceUsers(const int result, const std::set<std::string>& set_un, const std::string& device_name, const std::string& alarm_time, const int full_alarm_mode);

	bool JsonParseModifyDeviceConfInfo(std::string& device_guid, std::string& device_name, std::string& video_username, std::string& video_password);
	std::string JsonJoinModifyDeviceConfInfo(const int result);

	bool JsonParseModifyDeviceInfoAdvanced(std::string& device_guid, int& net_storage_switch, int& tf_storage_switch, int& alarm_switch, std::string& alarm_time, int& baby_mode, int& full_alarm_mode);
	std::string JsonJoinModifyDeviceInfoAdvanced(const int result);

	bool JsonParseGetDeviceUpdateInfo(std::string& client_version, int& device_type, int& device_sub_type);
	std::string JsonJoinGetDeviceUpdateInfo(const int result, const UPDATE_FILE_INFO& updateFileInfo);

	bool JsonParseModifyDevicePassword(std::string& device_guid, std::string& device_username, std::string& device_password);
	std::string JsonJoinModifyDevicePassword(const int result);

	bool JsonParseAddDeviceChannel(std::string& device_guid, int& add_sum);
	std::string JsonJoinAddDeviceChannel(const int result);

	bool JsonParseDeleteDeviceChannel(std::string& device_guid, int& channel_no);
	std::string JsonJoinDeleteDeviceChannel(const int result);

	bool JsonParseGetDeviceChannel(std::string& device_guid);
	std::string JsonJoinGetDeviceChannel(const int result, const std::vector<DEVICE_CHANNEL_INFO>& vec_device_channel);

	bool JsonParseModifyDeviceChannelName(std::string& device_guid, int& channel_no, std::string& channel_name);
	std::string JsonJoinModifyDeviceChannelName(const int result);

	bool JsonParseGetDeviceRelationNum(std::string& device_guid);
	std::string JsonJoinGetDeviceRelationNum(const int result, const int relation_num);

	bool JsonParseSetApConfFlag(std::string& device_guid);
	std::string JsonJoinSetApConfFlag(const int result);

	bool JsonParseGetDeviceOnlineState(std::string& device_guid);
	std::string JsonJoinGetDeviceOnlineState(const int result, const int online_flag);

	bool JsonParseCheckDeviceBindState(std::string& device_guid);
	std::string JsonJoinCheckDeviceBindState(const int result, const int online_flag, const int relation_num);

	bool JsonParseReportDeviceCloudSeeOnline(std::string& device_guid);
	std::string JsonJoinReportDeviceCloudSeeOnline(const int result);

	bool JsonParseReportDeviceReset(std::string& device_guid);
	std::string JsonJoinReportDeviceReset(const int result);

private:

	void JsonJoinPublic(const int message_type, const int result);

	bool VerifyPublicJson(JSONNode& in);

	bool VerifyJsonField(JSONNode& in, const std::string& field);

private:

	std::string json_string_;
	JSONNode in_;
	JSONNode out_;
	int  message_id_;
	int sfd_;
	int sfd_id_;
	std::string version_;
};

#endif /* JSON_OPERATE_H_ */
