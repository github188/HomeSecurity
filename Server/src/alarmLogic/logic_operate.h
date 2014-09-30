/*
 * logic_operate.h
 *
 *  Created on: Mar 11, 2013
 *      Author: yaowei
 */

#ifndef LOGIC_OPERATE_H_
#define LOGIC_OPERATE_H_

#include "defines.h"
#include "net_interface.h"

class CJsonOpt;
class CRedisOpt;
class CLogicOperate
{
public:
	CLogicOperate();
	virtual ~CLogicOperate();

public:

	void StartLogicOpt(const std::string& message);

private:

	int	 GetMessageId();

	void SendToDispatch();

private:

	CJsonOpt 	*jsonOpt_ptr_;
	CRedisOpt	*redisOpt_ptr_;
	int result_;
	std::string username_;

	pthread_mutex_t write_mutex;
	std::string responseToDispatch_;


};

#endif /* LOGIC_OPERATE_H_ */
