/*
 * sqlite_opt.h
 *
 *  Created on: Jun 5, 2014
 *      Author: zhangcq
 */

#ifndef SQLITE_OPT_H_
#define SQLITE_OPT_H_

#include <string>
#include <sqlite3.h>
#include "../public/user_interface_defines.h"

class SqliteOpt {
public:
	SqliteOpt();
	virtual ~SqliteOpt();
	bool OpenSqliteDB(const std::string& DBName);
	int InsertModifyMsgSqliteDB(const std::string& new_name,const std::string& old_name);
private:
	bool IsTableExit();
	bool CreateTable();
	pthread_mutex_t mutex_;
	sqlite3* con_;
	char *ErrMsg_;
};



#endif /* SQLITE_OPT_H_ */
