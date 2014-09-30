/*
 * message_opt.h
 *
 *  Created on: Mar 22, 2013
 *      Author: yaowei
 */

#ifndef MESSAGE_OPT_H_
#define MESSAGE_OPT_H_

#include "defines.h"
#include "json_opt.h"
#include "redis_opt.h"

class CLogicOpt
{
public:
	CLogicOpt();
	virtual ~CLogicOpt();

	void StartMessageOpt(const std::string& message, const int sfd);

private:

	int  SendToImServer(const std::string& username, std::string& message);

	int	 SendToImDevServer(const std::string& device_guid, std::string& message);

	bool SendToSpecifyImServer(const int im_server_no, std::string& message);

	void SendToPeer(std::string& message);

	void HeatbeatDetect();

	void Im2RelayNotifyOffline();

	void GetAccountLiveInfo();

	void Im2RelayDeviceModifyInfo();
	void Im2RelayDeviceModifyResult();

	void SendMessageToUser();

	void Im2RelayDeviceUpdateCMD();
	void Im2RelayDeviceUpdateCMDResult();

	void Im2RelayDeviceCancelCMD();
	void Im2RelayDeviceCancelCMDResult();

	void Im2RelayGetDeviceUpdateStep();
	void Im2RelayGetDeviceUpdateStepResult();

	void Im2RelayDeviceRebootCMD();

	void Im2RelayDeviceModifyPassword();
	void Im2RelayDeviceModifyPasswordResult();

	void ImDev2RelayAlarmMessage();

private:

	std::string message_;
	CJsonOpt 	jsonOpt_;
	CRedisOpt 	redisOpt_;
	int	current_sfd_;
	int result_;
	std::string responseToPeer_;

};

#endif /* MESSAGE_OPT_H_ */
