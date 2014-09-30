/*
 * relay_message_opt.h
 *
 *  Created on: Mar 22, 2013
 *      Author: yaowei
 */

#ifndef RELAY_MESSAGE_OPT_H_
#define RELAY_MESSAGE_OPT_H_

#include "json_opt.h"
#include "redis_opt.h"
#include "defines.h"

class CRelayMessageOpt
{
public:
	CRelayMessageOpt(const int sfd);
	virtual ~CRelayMessageOpt();

	void StartRelayMessageOpt(std::string& message);

private:

	bool VerifyUserSfd(const std::string& peer_name, int& sfd);

	bool SendToUser();

	void OptReOnlineProblem();

	void SendMessageToUser();

	void OptRelayDeviceModifyResult();

	void OptRelayDeviceUpdateCMDResult();

	void OptRelayDeviceCancelCMDResult();

	void OptRelayGetDeviceUpdateStepResult();

	void OptRelayDeviceModifyPasswordResult();

	void OptRelayAlarmMessageToUser();

	sem_t* FindEventByMessageIdAndSetResponse(const int message_id, const std::string& response);

	bool SendToClient(const int sfd, std::string& message);

private:

	std::string message_;
	int sfd_;

	CJsonOpt 	jsonOpt_;
	CRedisOpt	redisOpt_;
};

#endif /* RELAY_MESSAGE_OPT_H_ */
