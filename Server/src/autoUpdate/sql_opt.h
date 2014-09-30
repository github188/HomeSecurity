/*
 * sql_opt.h
 *
 *  Created on: 2012-11-26
 *      Author: yaowei
 */

#ifndef SQL_OPT_H_
#define SQL_OPT_H_

#include <string>
#include <map>
#include "../public/user_interface_defines.h"

class CSqlOpt
{
public:
	CSqlOpt();
	virtual ~CSqlOpt();

public:

	int GetLastSoftVersionFromDB(SoftVersionInfo& last_versionInfo);

};

#endif /* SQL_OPT_H_ */
