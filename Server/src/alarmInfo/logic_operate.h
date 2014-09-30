/*
 * logic_operate.h
 *
 *  Created on: Mar 11, 2013
 *      Author: yaowei
 */

#ifndef LOGIC_OPERATE_H_
#define LOGIC_OPERATE_H_

#include "defines.h"
#include "../public/user_interface_defines.h"

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
	void SendToDispatch();

	void GetAlarmInfo();
	void DelAlarmInfo();
	void CleanAlarmInfo();

private:
	bool SessionNotExist() {return result_ == SESSION_NOT_EXSIT; }

private:

	CJsonOpt	*jsonOpt_ptr_;
	CRedisOpt	*redisOpt_ptr_;
	std::string	username_;
	std::string session_id_;
	int result_;
	std::string responseToDispatch_;
};

#endif /* LOGIC_OPERATE_H_ */
