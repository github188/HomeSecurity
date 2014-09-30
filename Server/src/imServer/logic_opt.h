/*
 * message_opt.h
 *
 *  Created on: Mar 18, 2013
 *      Author: yaowei
 */

#ifndef MESSAGE_OPT_H_
#define MESSAGE_OPT_H_

#include "defines.h"
#include "redis_opt.h"
#include "../public/message.h"
#include "../public/user_interface_defines.h"

class CJsonOpt;
class CRedisOpt;
class CLogicOpt
{
public:
	explicit CLogicOpt(conn* c);
	virtual ~CLogicOpt();

	void StartLogicOpt(const std::string& message);

private:

	bool SessionNotExist() {return result_ == SESSION_NOT_EXSIT; }

	void UserOnline();

	void GetLiveStatus();

	void SetOnlineStatus();

	// 客户端向设备推送设备修改信息
	void PushDeviceModifyInfo();

	// 客户端向设备推送升级命令
	void PushDeviceUpdateCMD();

	// 客户端向设备推送取消下载命令
	void PushDeviceCancelCMD();

	// 获取设备更新下载进度
	void GetUpdateDownloadStep();

	// 获取设备烧写进度
	void GetUpdateWriteStep();

	// 设备更新结束发送重启命令
	void PushDeviceRebootCMD();
	
	// 修改设备用户名密码
	void PushDeviceModifyPassword();

private:

	void SendToClient();

	bool SendToRelayServer(const int relay_type);

	void SendNotifyOfflineToRelayServer(std::string& message);

	void maketimeout(struct timespec *tsp, long milliseconds);

	std::string FindResponseByMessageId(const int message_id);

	int GetResponseFromRelay(const int p2rmt, const int message_id, std::string& response);

	int GetDeviceOnlineStatus(const std::string& device_guid, int& online_flag);

private:

	CJsonOpt*	jsonOpt_ptr_;
	CRedisOpt*  redisOpt_ptr_;
	std::string username_;
	int 		result_;
	conn* 		conn_;
	redisContext* redis_con_;
	std::string responseToClient_;
};

#endif /* MESSAGE_OPT_H_ */
