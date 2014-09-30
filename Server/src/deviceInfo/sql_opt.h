#ifndef SQL_OPT_H_
#define SQL_OPT_H_

#include "../public/user_interface_defines.h"

class CSqlOpt {
public:
	CSqlOpt();
	virtual ~CSqlOpt();

public:

	int DeviceRegisterToDB(const DEVICE_INFO& device_info);
	int GetUserDevicesInfoFromDB(const std::string& username, std::vector<std::string>& vec_device, USER_DEVICES_LIST& user_devices_list);
	int GetDeviceInfoFromDB(const std::string& device_guid, DEVICE_INFO& device_info);
	int GetUserDeviceInfoFromDB(const std::string& username, const std::string& device_guid, USER_DEVICE_INFO& user_device_info);
	int ModifyDeviceInfoVideoLinkToDB(const std::string& user_name, const std::string& device_guid, const int video_link_type, const std::string& video_username, const std::string& video_password, const std::string& video_ip, const int video_port);
	int UserBindDeviceToDB(const std::string& username, const std::string& device_guid, const std::string& video_username, const std::string& video_password);
	int UserRemoveBindToDB(const std::string& username, const std::string& device_guid);
	int DeviceHumitureStatToDB(const std::vector<HUMITURE_STAT_INFO>& vec_humiture_stat_info);
	int GetDeviceHumitureFromDB(const std::string& device_guid, const int& getNum, std::vector<HUMITURE_STAT_INFO>& vec_humiture_stat_info);
	int GetUserDevicesStatusInfoFromDB(const std::string& username, std::vector<std::string>& vec_device, USER_DEVICES_LIST& user_devices_list);
	int DeviceHumitureScoreToDB(const std::string& device_guid, const std::string& timestamp, const int score, const int top);
	int GetLastHumitureScoreFromDB(const std::string& device_guid, int& score, int& top);
	int ModifyDeviceConfInfoToDB(const std::string& username, const std::string& device_guid, const std::string& device_name, const std::string& video_username, const std::string& video_password);
	int ModifyDeviceInfoAdvancedToDB(const std::string& device_guid, const int& net_storage_switch, const int& tf_storage_switch, const int& alarm_switch, const std::string& alarm_time, const int& baby_mode, const int& full_alarm_mode);
	int GetUpdateInfo(const int device_type, const int device_sub_type, const std::string& client_version, UPDATE_FILE_INFO& updateFileInfo);
	int ModifyDevicePasswordToDB(const std::string& username, const std::string& device_guid, const std::string& device_username, const std::string& device_password);
	int GetDeviceResetFlagFromDB(const std::string& device_guid, int& reset_flag);
	int RemoveBindByDeviceToDB(const std::string& device_guid);
	int RemoveBindByDeviceToDB_HomeCloud(const std::string& device_guid);
	int RemoveBindByDeviceHoldUserToDB_HomeCloud(const std::string& device_guid, const std::string& hold_user);
	int GetDevicePasswordFromDB(const std::string& device_guid, std::string& device_username, std::string& device_password);
	int GetUserDevicePasswordFromDB(const std::string& username, const std::string& device_guid, std::string& video_username, std::string& video_password);
	int GetDeviceUsersFromDB(const std::string& device_guid, std::vector<std::string> &vec_un, std::string &device_name, std::string &alarm_time, int &full_alarm_mode, std::string& device_username, std::string& device_password);
	int VerifyDeviceExistFromDB(const std::string& device_guid);
	int AddDeviceChannelToDB(const std::string& username, const std::string& device_guid, const int add_sum, const std::string& channel_name);
	int DeleteDeviceChannelToDB(const std::string& username, const std::string& device_guid, const int channel_no);
	int GetDeviceChannelFromDB(const std::string& username, const std::string& device_guid, std::vector<DEVICE_CHANNEL_INFO>& vec_device_channel);
	int GetUserChannelsFromDB(const std::string& username, std::vector<DEVICE_CHANNEL_INFO>& vec_user_channels);
	int ModifyDeviceChannelNameToDB(const std::string& username, const std::string& device_guid, const int channel_no, const std::string& channel_name);
	int ModifyDeviceWifiFlagToDB(const std::string& device_guid, const int wifi_flag);

private:
	int GetVideoLinkTypeFromDB(const std::string& user_name, const std::string& device_guid, int& video_link_type, std::string& video_username, std::string& video_password, std::string& video_ip, int& video_port);
	int GetUserDeviceConfigFromDB(const std::string& user_name, const std::string& device_guid, USER_DEVICE_CONFIG& user_device_config);

};

#endif /* SQL_OPT_H_ */
