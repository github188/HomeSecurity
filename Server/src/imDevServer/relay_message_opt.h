/*
 * relay_message_opt.h
 *
 *  Created on: 2014-2-13
 *      Author: zhangs
 */

#ifndef RELAY_MESSAGE_OPT_H_
#define RELAY_MESSAGE_OPT_H_

#include "json_opt.h"
#include "redis_opt.h"
#include "sql_opt.h"
#include "defines.h"

class CRelayMessageOpt
{
public:
	CRelayMessageOpt(const int sfd);
	virtual ~CRelayMessageOpt();

	void StartRelayMessageOpt(std::string& message);

private:

	bool VerifyDeviceSfd(const std::string& device_guid, int& sfd);

	bool SendToClient(const int sfd, std::string& message);

	void OptRelayDeviceModifyInfo();

	void OptRelayDeviceUpdateCMD();

	void OptRelayDeviceCancelCMD();

	void OptRelayGetDeviceUpdateStep();

	void OptRelayDeviceRebootCMD();

	void OptRelayDeviceModifyPassword();

private:

	std::string message_;
	int sfd_;

	CJsonOpt 	jsonOpt_;
	CRedisOpt	redisOpt_;
	CSqlOpt		sqlOpt_;
};

#endif /* RELAY_MESSAGE_OPT_H_ */
