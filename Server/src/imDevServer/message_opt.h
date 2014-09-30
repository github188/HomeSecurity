/*
 * message_opt.h
 *
 *  Created on: 2014-2-13
 *      Author: zhangs
 */

#ifndef MESSAGE_OPT_H_
#define MESSAGE_OPT_H_

#include "defines.h"
#include "../public/message.h"

class CJsonOpt;
class CSqlOpt;
class CRedisOpt;
class CMessageOpt
{
public:
	explicit CMessageOpt(conn* c);
	virtual ~CMessageOpt();

	void StartLogicOpt(const std::string &message);

private:

	void SendToClient();

	void DeviceOnline();
	void DeviceOffline();
	void DeviceHeartBeat();
	void DeviceReportHumiture();
	void PushDeviceModifyInfoRes();
	void PushDeviceUpdateCMDRes();
	void PushDeviceCancelCMDRes();
	void GetDeviceUpdateStepRes();
	void PushDeviceModifyPasswordRes();
	void PushAlarmMessage();
	void PushAlarmMessageFtp();
	bool SendToRelayServer(const int relay_type);

private:
	int GetAccountLiveInfo(ACCOUNT_LIVE_INFO& accountLiveInfo);
	bool SetAlarmMsgToRedis(const std::string& username, const int alarm_solution, const ALARM_INFO& alarm_info);
	int GetAlarmLevel(int alarm_type);

private:

	CJsonOpt*	jsonOpt_ptr_;
	CSqlOpt*	sqlOpt_ptr_;
	CRedisOpt*  redisOpt_ptr_;
	std::string deviceguid_;
	int 		result_;
	conn* 		c_;
	std::string responseToClient_;
	bool isResponse_;
};

#endif /* MESSAGE_OPT_H_ */
