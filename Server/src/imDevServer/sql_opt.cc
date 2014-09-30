#include "sql_opt.h"
#include "defines.h"
#include "sql_conn_pool.h"
#include "../public/user_interface_defines.h"
#include "../public/utils.h"

extern CSqlConnPool g_OptDeviceDataDB;

CSqlOpt::CSqlOpt()
{

}

CSqlOpt::~CSqlOpt()
{
}

int CSqlOpt::GetDevicePasswordFromDB(const std::string& device_guid, std::string& device_username, std::string& device_password)
{
	sql::Connection* con;
	con = g_OptDeviceDataDB.GetConnection();
	if (con == NULL)
	{
		LOG4CXX_ERROR(g_logger, "CSqlOpt::GetDevicePasswordFromDB.GetConnection = NULL");
		return MY_SQL_ERROR;
	}

	int b_ok = SUCCESS;
	sql::PreparedStatement* prep_stmt = NULL;
	sql::ResultSet* res = NULL;

	try
	{
		prep_stmt = con->prepareStatement("SELECT device_username,device_password FROM tb_device_IPC WHERE device_guid=?");
		prep_stmt->setString(1, device_guid);
		res = prep_stmt->executeQuery();
		if (res->next())
		{
			device_username = res->getString("device_username");
			device_password = res->getString("device_password");
		}
		else
		{
			b_ok = DEVICE_NOT_EXIST;
		}
		SAFE_CLOSE2(res, prep_stmt);
	}
	catch (sql::SQLException& e)
	{
		LOG4CXX_ERROR(g_logger, "CSqlOpt::GetDevicePasswordFromDB = " << e.what());
		b_ok = MY_SQL_ERROR;
	}

	SAFE_CLOSE2(res, prep_stmt);
	g_OptDeviceDataDB.ReleaseConnection(con);

	return b_ok;
}

int CSqlOpt::GetDeviceUsersFromDB(const std::string& device_guid, std::vector<std::string> &vec_un, std::string &device_name, std::string &alarm_time, int &full_alarm_mode, const std::string& device_username, const std::string& device_password)
{
	std::string video_username, video_password;

	if (device_guid.empty())
	{
		LOG4CXX_ERROR(g_logger, "CSqlOpt::GetDeviceUsersFromDB arg error.");
		return FAILED;
	}

	sql::Connection* con;
	con = g_OptDeviceDataDB.GetConnection();
	if (con == NULL)
	{
		LOG4CXX_ERROR(g_logger, "CSqlOpt::GetDeviceUsersFromDB:GetConnection = NULL");
		return MY_SQL_ERROR;
	}

	int b_ok = SUCCESS;
	sql::PreparedStatement* prep_stmt = NULL;
	sql::ResultSet* res = NULL;

	try
	{
		prep_stmt
			= con->prepareStatement(
			"SELECT user_name, device_name, video_username, video_password FROM tb_user_relation_device_homecloud WHERE device_guid=? AND device_type=?");
		prep_stmt->setString(1, device_guid);
		prep_stmt->setInt(2, DEV_TYPE_HOME_IPC);
		res = prep_stmt->executeQuery();
		std::string usrname;
		while (res->next())
		{
			video_username = res->getString("video_username");
			video_password = res->getString("video_password");
			if(video_username == device_username && video_password == device_password)
			{
				usrname = res->getString("user_name");
				vec_un.push_back(usrname);

				device_name = res->getString("device_name");
			}
		}
		SAFE_CLOSE2(prep_stmt,res);

		prep_stmt
			= con->prepareStatement(
			"SELECT alarm_time, full_alarm_mode FROM tb_device_IPC WHERE device_guid=? ");
		prep_stmt->setString(1, device_guid);
		res = prep_stmt->executeQuery();
		if (res->next())
		{
			alarm_time = res->getString("alarm_time");
			full_alarm_mode = res->getInt("full_alarm_mode");
		}

		SAFE_CLOSE2(prep_stmt,res);
	}
	catch (sql::SQLException& e)
	{
		LOG4CXX_ERROR(g_logger, "CSqlOpt::GetDeviceUsersFromDB:optDataDB = " << e.what());
		b_ok = MY_SQL_ERROR;
	}

	SAFE_CLOSE2(res, prep_stmt);

	g_OptDeviceDataDB.ReleaseConnection(con);

	return b_ok;
}

int CSqlOpt::GetDeviceResetFlagFromDB(const std::string& device_guid, int& reset_flag)
{
	sql::Connection* con;
	con = g_OptDeviceDataDB.GetConnection();
	if (con == NULL)
	{
		LOG4CXX_ERROR(g_logger, "CSqlOpt::GetDeviceResetFlagFromDB.GetConnection = NULL");
		return MY_SQL_ERROR;
	}

	int b_ok = SQL_NOT_FIND;
	sql::PreparedStatement* prep_stmt = NULL;
	sql::ResultSet* res = NULL;

	try
	{
		prep_stmt = con->prepareStatement("SELECT reset_flag FROM tb_device_basic WHERE device_guid=?");
		prep_stmt->setString(1, device_guid);
		res = prep_stmt->executeQuery();
		if (res->next())
		{
			b_ok = SUCCESS;
			reset_flag = res->getInt("reset_flag");
		}
		else
		{
			b_ok = DEVICE_NOT_EXIST;
		}
	}
	catch (sql::SQLException& e)
	{
		LOG4CXX_ERROR(g_logger, "CSqlOpt::GetDeviceResetFlagFromDB = " << e.what());
		b_ok = MY_SQL_ERROR;
	}

	SAFE_CLOSE2(res, prep_stmt);
	g_OptDeviceDataDB.ReleaseConnection(con);

	return b_ok;
}

int CSqlOpt::GetDeviceSubTypeFromDB(const std::string& device_guid, int& device_sub_type)
{
	sql::Connection* con;
	con = g_OptDeviceDataDB.GetConnection();
	if (con == NULL)
	{
		LOG4CXX_ERROR(g_logger, "CSqlOpt::GetDeviceSubTypeFromDB.GetConnection = NULL");
		return MY_SQL_ERROR;
	}

	int b_ok = SQL_NOT_FIND;
	sql::PreparedStatement* prep_stmt = NULL;
	sql::ResultSet* res = NULL;

	try
	{
		prep_stmt = con->prepareStatement("SELECT device_sub_type FROM tb_device_IPC WHERE device_guid=?");
		prep_stmt->setString(1, device_guid);
		res = prep_stmt->executeQuery();
		if (res->next())
		{
			b_ok = SUCCESS;
			device_sub_type = res->getInt("device_sub_type");
		}
		else
		{
			b_ok = DEVICE_NOT_EXIST;
		}
	}
	catch (sql::SQLException& e)
	{
		LOG4CXX_ERROR(g_logger, "CSqlOpt::GetDeviceSubTypeFromDB = " << e.what());
		b_ok = MY_SQL_ERROR;
	}

	SAFE_CLOSE2(res, prep_stmt);
	g_OptDeviceDataDB.ReleaseConnection(con);

	return b_ok;
}
