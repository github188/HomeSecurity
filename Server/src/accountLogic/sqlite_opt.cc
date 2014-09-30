/*
 * sqlit_opt.cc
 *
 *  Created on: Jun 5, 2014
 *      Author: zhangcq
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "defines.h"
#include "../public/utils.h"
#include "sqlite_opt.h"

SqliteOpt::SqliteOpt() {
	// TODO Auto-generated constructor stub
	int err;
	ErrMsg_ = NULL;
	if (0 != (err = pthread_mutex_init(&mutex_, NULL)))
	{
		LOG4CXX_ERROR(g_logger, "Could not create mutex. Errno: " << strerror(errno));
	    exit(1);
	}
}

SqliteOpt::~SqliteOpt() {
	// TODO Auto-generated destructor stub
	sqlite3_close(con_);
	con_ = NULL;
}

bool SqliteOpt::OpenSqliteDB(const std::string& DBName)
{
	int ret = sqlite3_open(DBName.c_str(), &con_);

	if (ret) {
		LOG4CXX_ERROR(g_logger, "sqlite_opt::openSqliteDB Can't open database: "<< sqlite3_errmsg(con_));
		return false;
	}

	if( !IsTableExit())
	{
		if(CreateTable())
		{
			LOG4CXX_TRACE(g_logger, "sqlite_opt::openSqliteDB open database success(table not exist)");
			return true;
		}
		else
		{
			LOG4CXX_ERROR(g_logger, "sqlite_opt::openSqliteDB open database failed");
			return false;
		}
	}

	LOG4CXX_TRACE(g_logger, "sqlite_opt::openSqliteDB open database success(table exist)");
	return true;
}

bool SqliteOpt::IsTableExit()
{
	int count;
	sqlite3_stmt *stmt = NULL;
	sqlite3_prepare(con_, "SELECT count(*) FROM sqlite_master WHERE type='table' AND name='tb_replace_old_username' ", -1, &stmt, NULL);
	while (SQLITE_ROW == sqlite3_step(stmt))
	{
		count = sqlite3_column_int(stmt, 0);
	}

	sqlite3_finalize(stmt);
	stmt = NULL;

	if (1 == count) {
		LOG4CXX_TRACE(g_logger, "sqlite_opt::IsTableExit Table is exist count=" << count );
		return true;
	}

	LOG4CXX_TRACE(g_logger, "sqlite_opt::IsTableExit Table is not exist count= " << count);

	return false;
}

bool SqliteOpt::CreateTable()
{
	std::string sql = "CREATE TABLE [tb_replace_old_username]([old_name] varchar(20), [new_name] varchar(20), [modify_time] datetime)";

	int ret = sqlite3_exec(con_, sql.c_str(), 0, 0, &ErrMsg_);
	if (ret != SQLITE_OK)
	{
		LOG4CXX_ERROR(g_logger, "sqlite_opt::CreateTable SQL error: "<< ErrMsg_);
		sqlite3_free(ErrMsg_);
		ErrMsg_ = NULL;
		return false;
	}
	LOG4CXX_TRACE(g_logger, "sqlite_opt::CreateTable success");

	return true;
}

int SqliteOpt::InsertModifyMsgSqliteDB(const std::string& new_name,const std::string& old_name)
{
	char sql[200] = {0};
	pthread_mutex_lock(&mutex_);
	std::string time_now = utils::NowtimeString();
	sprintf(sql, "INSERT INTO [tb_replace_old_username]([old_name], [new_name], [modify_time]) VALUES('%s', '%s', '%s')",
			old_name.c_str(), new_name.c_str(), time_now.c_str());
	int ret = sqlite3_exec(con_, sql, 0, 0, &ErrMsg_);
	if (ret != SQLITE_OK)
	{
		LOG4CXX_ERROR(g_logger, "sqlite_opt::InsertModifyMsgSqliteDB SQL error: "<< ErrMsg_);
		sqlite3_free(ErrMsg_);
		ErrMsg_ = NULL;
		return MY_SQL_ERROR;
	}
	pthread_mutex_unlock(&mutex_);
	return SUCCESS;
}
