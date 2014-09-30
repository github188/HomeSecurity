/*
 * cloudsee_message_opt.h
 *
 *  Created on: Mar 30, 2014
 *      Author: zhangs
 */

#ifndef CLOUDSEE_MESSAGE_OPT_H_
#define CLOUDSEE_MESSAGE_OPT_H_

#include "redis_opt.h"
#include "cloudsee_data_opt.h"
#include "defines.h"

class CCloudSeeMessageOpt
{
public:
	CCloudSeeMessageOpt(const int sfd);
	virtual ~CCloudSeeMessageOpt();

	void StartCloudSeeMessageOpt(char* message);

private:
	void HeartBeat();
	void GetCloudSeeDeviceInfo();

private:
	sem_t* FindEventByMessageIdAndSetResponse(const int message_id, const char* response);

private:

	char* message_;
	int sfd_;

	CCloudSeeDataOpt dataOpt_;
	CRedisOpt	redisOpt_;
};

#endif /* CLOUDSEE_MESSAGE_OPT_H_ */
