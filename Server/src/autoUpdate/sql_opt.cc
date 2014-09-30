/*
 * sql_opt.cc
 *
 *  Created on: 2012-11-26
 *      Author: yaowei
 */

#include "sql_opt.h"
#include "defines.h"
#include "sql_conn_pool.h"
#include "../public/config_file.h"
#include "../public/message.h"
#include "../public/utils.h"

extern CSqlConnPool g_OptClientSoftDataDB;

CSqlOpt::CSqlOpt()
{

}

CSqlOpt::~CSqlOpt()
{
}

int CSqlOpt::GetLastSoftVersionFromDB(SoftVersionInfo& last_versionInfo)
{
	sql::Connection* con;
	con = g_OptClientSoftDataDB.GetConnection();
	if (con == NULL)
	{
		LOG4CXX_ERROR(g_logger, "CSqlOpt::getLastSoftVersionFromDB:optUserDataDB.GetConnection = NULL");
		return MY_SQL_ERROR;
	}

	int b_ok = SUCCESS;
	sql::PreparedStatement* prep_stmt = NULL;
	sql::ResultSet* res = NULL;


	try
	{
		prep_stmt = con->prepareStatement("select soft_version, soft_version_addr, soft_version_desp from tb_soft_version where version_id=0");
		res = prep_stmt->executeQuery();
		if (res->next())
		{
			last_versionInfo.soft_version = res->getString("soft_version");
			last_versionInfo.soft_version_addr = res->getString("soft_version_addr");
			last_versionInfo.soft_version_desp = res->getString("soft_version_desp");
		}
	}
	catch (sql::SQLException& e)
	{
		LOG4CXX_ERROR(g_logger, "CSqlOpt::getLastSoftVersionFromDB:optUserDataDB = " << e.what());
		b_ok = MY_SQL_ERROR;
	}

	SAFE_CLOSE2(res, prep_stmt);
	g_OptClientSoftDataDB.ReleaseConnection(con);

	return b_ok;
}
