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

	void GetLastSoftVersion();

	void SendToDispatch();

private:

	CJsonOpt *jsonOpt_ptr_;
	CSqlOpt		 *sqlOpt_ptr_;
	int result_;
	std::string responseToDispatch_;

};

#endif /* LOGIC_OPERATE_H_ */
