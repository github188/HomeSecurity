/*
 * logic_operate.h
 *
 *  Created on: Feb 12, 2014
 *      Author: zhangs
 */

#ifndef LOGIC_OPERATE_H_
#define LOGIC_OPERATE_H_

#include "defines.h"
#include "../public/message.h"
#include "../public/user_interface_defines.h"

class CJsonOpt;
class CSqlOpt;
class CRedisOpt;
class CLogicOperate
{
public:
	CLogicOperate();
	virtual ~CLogicOperate();

public:

	void StartLogicOpt(const std::string& message);

private:

	void DeviceRegister();
	void GetUserDevices();
	void GetDeviceInfo();
	void GetUserDeviceInfo();
	void ModifyDeviceInfoVideoLink();
	void UserBindDevice();
	void UserRemoveBindDevice();
	void GetDeviceHumiture();
	void GetDeviceHumitureOntime();
	void GetUserDevicesStatusInfo();
	void GetDeviceHumitureScore();
	void GetDeviceUsers();
	void ModifyDeviceConfInfo();
	void ModifyDeviceInfoAdvanced();
	void GetDeviceUpdateInfo();
	void ModifyDevicePassword();
	void AddDeviceChannel();
	void DeleteDeviceChannel();
	void GetDeviceChannel();
	void ModifyDeviceChannelName();
	void GetDeviceRelationNum();
	void SetApConfFlag();
	void GetDeviceOnlineState();
	void CheckDeviceBindState();
	void ReportDeviceCloudSeeOnline();
	void ReportDeviceReset();

	void SendToDispatch();

private:

	bool SessionNotExist() {return result_ == SESSION_NOT_EXSIT; }
	int GetDeviceOnlineStatus(const std::string& device_guid, int& online_flag);
	int GetLatestDeviceHumitureStatus(const std::string& device_guid, struct tm& timestamp, double& temperature, double& humidity);
	std::string GetDeviceTimestampFromRedisKey(const std::string& redisKey);
	int GetOnlineDevicesGuid(std::vector<std::string>& vec_online_guid);
	int GetDeviceOnlineStatusByRedisKey(const std::string& redisKey);
	std::string GetDeviceGuidByRedisKey(const std::string& redisKey);
	int GetHumitureScore(const double temperature, const double humidity);
	int GetHumitureAssessment(const double temperature, const double humidity);
	int GetDeviceResetFlag(const std::string& device_guid);
	int RemoveBindByDevice(const std::string& device_guid);
	int RemoveBindByDeviceHoldUser(const std::string& device_guid, const std::string& hold_user);
	int VerifyVedioAndDevicePassword(const std::string& username, const std::string& device_guid, int& verify_flag);
	int VerifyPasswordBeforeBind(const std::string& device_guid, std::string& video_username, std::string& video_password, int& verify_flag);
	int VerifyDeviceExist(const std::string& device_guid);

private:

	CJsonOpt 	*jsonOpt_ptr_;
	CSqlOpt		*sqlOpt_ptr_;
	CRedisOpt	*redisOpt_ptr_;

	std::string username_;
	std::string session_id_;
	int 		result_;
	pthread_mutex_t write_mutex;
	std::string responseToDispatch_;
};

#endif /* LOGIC_OPERATE_H_ */
