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
#include "sqlite_opt.h"

std::string CSqlOpt::GlobalUserName="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789_-";

extern CSqlConnPool g_OptUserDataDB;
extern CSqlConnPool g_OptDeviceDataDB;
extern SqliteOpt g_OptSqliteDB;

CSqlOpt::CSqlOpt()
{

}

CSqlOpt::~CSqlOpt()
{
}

int CSqlOpt::IsUserExistFromDB(const int account_type, const std::string& account_name)
{
	int b_ok = USER_NOT_EXIST;

	sql::Connection* con;
	sql::PreparedStatement* prep_stmt = NULL;
	sql::ResultSet* res = NULL;

	con = g_OptUserDataDB.GetConnection();
	if (con == NULL)
	{
		LOG4CXX_ERROR(g_logger, "CSqlOpt::IsUserExistFromDB:optUserDataDB.GetConnection = NULL");
		return MY_SQL_ERROR;
	}

	try
	{
		prep_stmt = con->prepareStatement(std::string("select password from tb_user where ") + JoinAccountNameCondition(account_type));
		prep_stmt->setString(1, account_name);
		res = prep_stmt->executeQuery();
		if (res->next())
		{
			b_ok = USER_HAS_EXIST;
		}
	}
	catch (sql::SQLException& e)
	{
		LOG4CXX_ERROR(g_logger, "CSqlOpt::IsUserExistFromDB:optUserDataDB = " << e.what());
		b_ok = MY_SQL_ERROR;
	}

	SAFE_CLOSE2(res, prep_stmt);
	g_OptUserDataDB.ReleaseConnection(con);

	return b_ok;
}

bool CSqlOpt::GetUsername(std::string& username)
{
	int count=0, pos=0;
	srand(time(0));
	pos = rand()%64;
 	while(USER_NOT_EXIST != IsUserExistFromDB(ACCOUNT_USERNAME,username))
	{
 		if(count++ > 8)
 			break;

 		username += GlobalUserName[pos];

		srand(time(0) + count);
		pos = rand()%64;
	}

 	if(count <= 8)
 		return true;
 	else
 		return false;
}

int CSqlOpt::UserRegisterToDB(const int account_type, const std::string& account_name, const std::string& password, const std::string& custom_type)
{
	int b_ok = SUCCESS;
	std::string username, mail, phone;
	std::string timestr, timeyear;

	timestr = utils::GetCurrentDayString();
	timeyear = timestr.substr(0,4);

	b_ok = IsUserExistFromDB(account_type, account_name);
	if(USER_NOT_EXIST != b_ok)
		return b_ok;

	sql::Connection* con;
	sql::PreparedStatement* prep_stmt = NULL;

	con = g_OptUserDataDB.GetConnection();
	if (con == NULL)
	{
		LOG4CXX_ERROR(g_logger, "CSqlOpt::UserRegisterToDB:optUserDataDB.GetConnection = NULL");
		return MY_SQL_ERROR;
	}

	switch(account_type)
	{
	case ACCOUNT_USERNAME:
		username = account_name;
		mail = "";
		phone = "";
		break;
	case ACCOUNT_MAIL:
		username = utils::ReplaceString(account_name, "@", "_");
		username = utils::ReplaceString(username, "\\.", "_");
		mail = account_name;
		phone = "";
		break;
	case ACCOUNT_PHONE:
		username = account_name + std::string("_p");
		mail = "";
		phone = account_name;
		break;
	}

	if(!GetUsername(username))
	{
		b_ok = USER_HAS_EXIST;
		goto END;
	}

	try
	{
		b_ok = SUCCESS;

		con->setAutoCommit(false);

		prep_stmt = con->prepareStatement("INSERT INTO tb_user (username, mail, phone, password, pwdlevel, custom_type) VALUES (?,?,?,?,?,?)");
		prep_stmt->setString(1, username);
		if (!mail.empty())
		{
			prep_stmt->setString(2, mail);
		}
		else
		{
			prep_stmt->setNull(2, sql::DataType::VARCHAR);
		}
		if(!phone.empty())
		{
			prep_stmt->setString(3, phone);
		}
		else
		{
			prep_stmt->setNull(3, sql::DataType::CHAR);
		}
		prep_stmt->setString(4, password);
		prep_stmt->setInt(5, PWD_HIGH);
		if(!custom_type.empty())
		{
			prep_stmt->setString(6, custom_type);
		}
		else
		{
			prep_stmt->setNull(6, sql::DataType::VARCHAR);
		}

		prep_stmt->execute();
		SAFE_CLOSE(prep_stmt);

		prep_stmt = con->prepareStatement(
				"INSERT INTO tb_user_behavior (username, register_date) VALUES (?, ?)");
		prep_stmt->setString(1, username);
		prep_stmt->setString(2, utils::GetCurrentDayString());
		prep_stmt->execute();
		SAFE_CLOSE(prep_stmt);

		prep_stmt = con->prepareStatement("INSERT INTO tb_user_detail (username) VALUES (?)");
		prep_stmt->setString(1, username);
		prep_stmt->execute();
		SAFE_CLOSE(prep_stmt);

		prep_stmt = con->prepareStatement(std::string("INSERT INTO tb_user_month_activity_")+ timeyear + " (username) VALUES(?)");
		prep_stmt->setString(1, username);
		prep_stmt->execute();
		SAFE_CLOSE(prep_stmt);

		con->commit();
	}
	catch (sql::SQLException& e)
	{
		try
		{
			con->rollback();
		}
		catch (sql::SQLException& e)
		{
		}
		LOG4CXX_ERROR(g_logger, "CSqlOpt::UserRegisterToDB:optUserDataDB = " << e.what());
		b_ok = MY_SQL_ERROR;
	}
END:
	con->setAutoCommit(true);
	SAFE_CLOSE( prep_stmt);
	g_OptUserDataDB.ReleaseConnection(con);

	return b_ok;
}

int CSqlOpt::SetAccountInfoToDB(const AccountInfo& account_info)
{
	int b_ok = SUCCESS;
	std::string username, mail, phone ,nickname,account_name;

	username = account_info.username;
	mail = account_info.mail;
	phone = account_info.phone;
	nickname = account_info.nickname;

	b_ok = IsUserExistFromDB(ACCOUNT_MAIL, mail);
	if(USER_HAS_EXIST == b_ok)
		return b_ok;

	b_ok = IsUserExistFromDB(ACCOUNT_PHONE, phone);
	if(USER_HAS_EXIST == b_ok)
		return b_ok;

	sql::Connection* con;
	sql::PreparedStatement* prep_stmt = NULL;

	con = g_OptUserDataDB.GetConnection();
	if (con == NULL)
	{
		LOG4CXX_ERROR(g_logger, "CSqlOpt::SetAccountInfoToDB:optUserDataDB.GetConnection = NULL");
		return MY_SQL_ERROR;
	}

	if(!mail.empty())
	{
		account_name = utils::ReplaceString(account_name, "@", "_");
		account_name = utils::ReplaceString(username, "\\.", "_");
	}
	else if(!phone.empty())
	{
		account_name = account_name + std::string("_p");
	}

	//todo:username,session
	try
	{
		b_ok = SUCCESS;

		con->setAutoCommit(false);

		prep_stmt = con->prepareStatement(std::string("UPDATE tb_user,tb_user_detail SET tb_user.username=?,tb_user_detail.username=?,mail=?,phone=?,nickname=? WHERE ") +
										"tb_user.username=? and tb_user.username=tb_user_detail.username");

		if(!account_name.empty())
		{
			prep_stmt->setString(1, account_name);
			prep_stmt->setString(2, account_name);
		}
		else
		{
			prep_stmt->setNull(1, sql::DataType::VARCHAR);
			prep_stmt->setNull(2, sql::DataType::VARCHAR);
		}
		if (!mail.empty())
		{
			prep_stmt->setString(3, mail);
		}
		else
		{
			prep_stmt->setNull(3, sql::DataType::VARCHAR);
		}
		if(!phone.empty())
		{
			prep_stmt->setString(4, phone);
		}
		else
		{
			prep_stmt->setNull(4, sql::DataType::CHAR);
		}
		if(!nickname.empty())
		{
			prep_stmt->setString(5, nickname);
		}
		else
		{
			prep_stmt->setNull(5, sql::DataType::CHAR);
		}

		prep_stmt->setString(6, username);
		prep_stmt->execute();
		SAFE_CLOSE(prep_stmt);

		con->commit();
	}
	catch (sql::SQLException& e)
	{
		try
		{
			con->rollback();
		}
		catch (sql::SQLException& e)
		{
		}
		LOG4CXX_ERROR(g_logger, "CSqlOpt::SetAccountInfoToDB:optUserDataDB = " << e.what());
		b_ok = MY_SQL_ERROR;
	}

	con->setAutoCommit(true);
	SAFE_CLOSE( prep_stmt);
	g_OptUserDataDB.ReleaseConnection(con);

	return b_ok;
}

int CSqlOpt::GetAccountInfoFromDB(const std::string& account_name, AccountInfo& account_info)
{
	int b_ok = SUCCESS;
	std::string username = account_name;

	sql::Connection* con;
	sql::PreparedStatement* prep_stmt = NULL;
	sql::ResultSet* res = NULL;

	con = g_OptUserDataDB.GetConnection();
	if (con == NULL)
	{
		LOG4CXX_ERROR(g_logger, "CSqlOpt::GetAccountInfoFromDB:optUserDataDB.GetConnection = NULL");
		return MY_SQL_ERROR;
	}

	try
	{
		prep_stmt = con->prepareStatement(std::string("select tb_user.username, mail, phone, nickname from tb_user,tb_user_detail where ") +
											 "tb_user.username=? and tb_user.username=tb_user_detail.username");
		prep_stmt->setString(1, account_name);
		res = prep_stmt->executeQuery();
		if (res->next())
		{
			account_info.username = res->getString("username");
			account_info.mail = res->getString("mail");
			account_info.phone = res->getString("phone");
			account_info.nickname = res->getString("nickname");
		}
		else
		{
			b_ok = SQL_NOT_FIND;
		}
	}
	catch (sql::SQLException& e)
	{
		LOG4CXX_ERROR(g_logger, "CSqlOpt::GetAccountInfoFromDB:optUserDataDB = " << e.what());
		b_ok = MY_SQL_ERROR;
	}

	SAFE_CLOSE2(res, prep_stmt);
	g_OptUserDataDB.ReleaseConnection(con);

	return b_ok;
}

int CSqlOpt::GetAccountMailWithNoSessionFromDB(const int account_type, const std::string& account_name, std::string& mail)
{
	int b_ok = SUCCESS;
	std::string username = account_name;

	sql::Connection* con;
	sql::PreparedStatement* prep_stmt = NULL;
	sql::ResultSet* res = NULL;

	con = g_OptUserDataDB.GetConnection();
	if (con == NULL)
	{
		LOG4CXX_ERROR(g_logger, "CSqlOpt::GetAccountMailWithNoSessionFromDB:optUserDataDB.GetConnection = NULL");
		return MY_SQL_ERROR;
	}

	try
	{
		prep_stmt = con->prepareStatement(std::string("select mail from tb_user where ") + JoinAccountNameCondition(account_type));
		prep_stmt->setString(1, account_name);
		res = prep_stmt->executeQuery();
		if (res->next())
		{
			mail = res->getString("mail");
		}
		else
		{
			b_ok = SQL_NOT_FIND;
		}
	}
	catch (sql::SQLException& e)
	{
		LOG4CXX_ERROR(g_logger, "CSqlOpt::GetAccountMailWithNoSessionFromDB:optUserDataDB = " << e.what());
		b_ok = MY_SQL_ERROR;
	}

	SAFE_CLOSE2(res, prep_stmt);
	g_OptUserDataDB.ReleaseConnection(con);

	return b_ok;
}

int CSqlOpt::GetUserPasswordStrength(const int account_type, const std::string& account_name)
{
	int b_ok = USER_NOT_EXIST;

	sql::Connection* con;
	sql::PreparedStatement* prep_stmt = NULL;
	sql::ResultSet* res = NULL;

	con = g_OptUserDataDB.GetConnection();
	if (con == NULL)
	{
		LOG4CXX_ERROR(g_logger, "CSqlOpt::GetUserPasswordStrength:optUserDataDB.GetConnection = NULL");
		return MY_SQL_ERROR;
	}

	try
	{
		prep_stmt = con->prepareStatement(std::string("select pwdlevel from tb_user where ") + JoinAccountNameCondition(account_type));
		prep_stmt->setString(1, account_name);
		res = prep_stmt->executeQuery();
		if (res->next())
		{
			int pwdlevel = res->getInt("pwdlevel");
			if(PWD_LOW == pwdlevel)
				b_ok = LOW_STRENGTH_PASSWORD;
			else if(PWD_HIGH == pwdlevel)
				b_ok = HIGH_STRENGTH_PASSWORD;
		}
	}
	catch (sql::SQLException& e)
	{
		LOG4CXX_ERROR(g_logger, "CSqlOpt::GetUserPasswordStrength:optUserDataDB = " << e.what());
		b_ok = MY_SQL_ERROR;
	}

	SAFE_CLOSE2(res, prep_stmt);
	g_OptUserDataDB.ReleaseConnection(con);

	return b_ok;
}

int CSqlOpt::ModifyUserNameToDB(const int account_type,
		const std::string& account_name, const std::string& old_username,
		const std::string& password)
{
	int b_ok = SUCCESS;
	std::string username, mail, phone;
	std::string timestr, timeyear;
	timestr = utils::GetCurrentDayString();
	timeyear = timestr.substr(0,4);

	b_ok = IsUserExistFromDB(account_type, account_name);
	if (USER_HAS_EXIST == b_ok)
		return b_ok;

	sql::Connection* con;
	sql::PreparedStatement* prep_stmt = NULL;

	con = g_OptUserDataDB.GetConnection();
	if (con == NULL)
	{
		LOG4CXX_ERROR(g_logger,
				"CSqlOpt::ModifyUserNameToDB:optUserDataDB.GetConnection = NULL");
		return MY_SQL_ERROR;
	}

	switch (account_type)
	{
	case ACCOUNT_USERNAME:
		username = account_name;
		mail = "";
		phone = "";
		break;
	case ACCOUNT_MAIL:
		username = utils::ReplaceString(account_name, "@", "_");
		username = utils::ReplaceString(username, "\\.", "_");
		mail = account_name;
		phone = "";
		break;
	case ACCOUNT_PHONE:
		username = account_name + std::string("_p");
		mail = "";
		phone = account_name;
		break;
	}

	try
	{
		b_ok = SUCCESS;

		con->setAutoCommit(false);

		prep_stmt =
				con->prepareStatement(
						"UPDATE tb_user SET username=?, mail=?, phone=?, password=?,pwdlevel=? WHERE username=?");
		prep_stmt->setString(1, username);
		if (!mail.empty())
		{
			prep_stmt->setString(2, mail);
		} else
		{
			prep_stmt->setNull(2, sql::DataType::VARCHAR);
		}
		if (!phone.empty())
		{
			prep_stmt->setString(3, phone);
		} else
		{
			prep_stmt->setNull(3, sql::DataType::CHAR);
		}
		prep_stmt->setString(4, password);
		prep_stmt->setInt(5, PWD_HIGH);
		prep_stmt->setString(6, old_username);
		prep_stmt->execute();
		SAFE_CLOSE(prep_stmt);

		prep_stmt = con->prepareStatement(
				"UPDATE tb_user_behavior SET username=? WHERE username=?");
		prep_stmt->setString(1, username);
		prep_stmt->setString(2, old_username);
		prep_stmt->execute();
		SAFE_CLOSE(prep_stmt);

		prep_stmt = con->prepareStatement(
				"UPDATE tb_user_detail SET username=? WHERE username=?");
		prep_stmt->setString(1, username);
		prep_stmt->setString(2, old_username);
		prep_stmt->execute();
		SAFE_CLOSE(prep_stmt);

		prep_stmt = con->prepareStatement(std::string("UPDATE tb_user_month_activity_")+ timeyear + " SET username=? WHERE username=?");
		prep_stmt->setString(1, username);
		prep_stmt->setString(2, old_username);
		prep_stmt->execute();
		SAFE_CLOSE(prep_stmt);

		con->commit();
	} catch (sql::SQLException& e)
	{
		try
		{
			con->rollback();
		} catch (sql::SQLException& e)
		{
		}
		LOG4CXX_ERROR(g_logger,
				"CSqlOpt::ModifyUserNameToDB:optUserDataDB = " << e.what());
		b_ok = MY_SQL_ERROR;
		goto END;
	}

	b_ok = UpdateUsernameToDeviceDB(username, old_username);

END:
	con->setAutoCommit(true);
	SAFE_CLOSE(prep_stmt);
	g_OptUserDataDB.ReleaseConnection(con);

	return b_ok;
}

int CSqlOpt::GetUserpassFromDB(const int account_type, const std::string& account_name,
							   std::string& username, std::string& password, int& pwdlevel)
{
	int b_ok = SUCCESS;

	sql::Connection* con;
	sql::PreparedStatement* prep_stmt = NULL;
	sql::ResultSet* res = NULL;

	con = g_OptUserDataDB.GetConnection();
	if (con == NULL)
	{
		LOG4CXX_ERROR(g_logger, "CSqlOpt::GetuserpassFromDBUser:optUserDataDB.GetConnection = NULL");
		return MY_SQL_ERROR;
	}

	try
	{
		prep_stmt = con->prepareStatement(std::string("select username, password, pwdlevel from tb_user where ") + JoinAccountNameCondition(account_type));
		prep_stmt->setString(1, account_name);
		res = prep_stmt->executeQuery();
		if (res->next())
		{
			username = res->getString("username");
			password = res->getString("password");
			pwdlevel = res->getInt("pwdlevel");
		}
		else
		{
			b_ok = SQL_NOT_FIND;
		}
	}
	catch (sql::SQLException& e)
	{
		LOG4CXX_ERROR(g_logger, "CSqlOpt::GetuserpassFromDBUser:optUserDataDB = " << e.what());
		b_ok = MY_SQL_ERROR;
	}

	SAFE_CLOSE2(res, prep_stmt);
	g_OptUserDataDB.ReleaseConnection(con);

	return b_ok;
}

int CSqlOpt::BindMailOrPhoneToDB(const int account_type, const std::string& mailOrPhone, const std::string& username)
{
	int b_ok = SUCCESS;

	b_ok = IsUserExistFromDB(account_type, mailOrPhone);
	if (USER_HAS_EXIST == b_ok)
		return b_ok;

	if(ACCOUNT_USERNAME == account_type)
		return ACCOUNT_INVALID;

	sql::Connection* con;
	sql::PreparedStatement* prep_stmt = NULL;

	con = g_OptUserDataDB.GetConnection();
	if (con == NULL)
	{
		LOG4CXX_ERROR(g_logger,"CSqlOpt::BindMailOrPhoneToDB:optUserDataDB.GetConnection = NULL");
		return MY_SQL_ERROR;
	}

	try
	{
		b_ok = SUCCESS;

		if(ACCOUNT_MAIL == account_type)
		{
			prep_stmt = con->prepareStatement("UPDATE tb_user SET mail = ? WHERE username = ?");

		}
		else if(ACCOUNT_PHONE == account_type)
		{
			prep_stmt = con->prepareStatement("UPDATE tb_user SET phone = ? WHERE username = ?");
		}
		prep_stmt->setString(1, mailOrPhone);
		prep_stmt->setString(2, username);
		prep_stmt->execute();
	}
	catch (sql::SQLException& e)
	{
		LOG4CXX_ERROR(g_logger, "CSqlOpt::BindMailOrPhoneToDB:optUserDataDB = " << e.what());
		b_ok = MY_SQL_ERROR;
	}

	SAFE_CLOSE(prep_stmt);
	g_OptUserDataDB.ReleaseConnection(con);

	return b_ok;
}

int CSqlOpt::VerifyUserPasswordFromDB(const std::string& username, const std::string& password_in)
{
	int b_ok = SUCCESS;
	std::string password;

	sql::Connection* con;
	sql::PreparedStatement* prep_stmt = NULL;
	sql::ResultSet* res = NULL;

	con = g_OptUserDataDB.GetConnection();
	if (con == NULL)
	{
		LOG4CXX_ERROR(g_logger, "CSqlOpt::VerifyUserPasswordFromDB:optUserDataDB.GetConnection = NULL");
		return MY_SQL_ERROR;
	}

	try
	{
		prep_stmt = con->prepareStatement("select password from tb_user where username=?");
		prep_stmt->setString(1, username);
		res = prep_stmt->executeQuery();
		if (res->next())
		{
			password = res->getString("password");
		}

		if(password.compare(password_in) != 0)
			b_ok = PASSWORD_ERROR;
	}
	catch (sql::SQLException& e)
	{
		LOG4CXX_ERROR(g_logger, "CSqlOpt::VerifyUserPasswordFromDB:optUserDataDB = " << e.what());
		b_ok = MY_SQL_ERROR;
	}

	SAFE_CLOSE2(res, prep_stmt);
	g_OptUserDataDB.ReleaseConnection(con);

	return b_ok;
}

int CSqlOpt::ModifyUserPasswordToDB(const std::string& username, const std::string& password,
									const std::string& new_password)
{
	int b_ok = SQL_NOT_FIND;

	sql::Connection* con;
	sql::PreparedStatement* prep_stmt = NULL;

	con = g_OptUserDataDB.GetConnection();
	if (con == NULL)
	{
		LOG4CXX_ERROR(g_logger, "CSqlOpt::ModifyUserPasswordToDB:GetConnection = NULL.");
		return MY_SQL_ERROR;
	}

	try
	{
		prep_stmt = con->prepareStatement(
				"UPDATE tb_user SET password = ? WHERE username=? AND password=?");
		prep_stmt->setString(1, new_password);
		prep_stmt->setString(2, username);
		prep_stmt->setString(3, password);
		int result = prep_stmt->executeUpdate();
		if(1 == result)
			b_ok = SUCCESS;
		else if(0 == result)
			b_ok = PASSWORD_ERROR;
	}
	catch (sql::SQLException& e)
	{
		LOG4CXX_ERROR(g_logger, "CSqlOpt::ModifyUserPasswordToDB:optUserDataDB = " << e.what());
		b_ok = MY_SQL_ERROR;
	}

	SAFE_CLOSE(prep_stmt);
	g_OptUserDataDB.ReleaseConnection(con);

	return b_ok;
}

int CSqlOpt::WebResetUserPasswordToDB(const int account_type, const std::string& account_name, const std::string& new_password)
{
	int b_ok = SUCCESS;

	sql::Connection* con;
	sql::PreparedStatement* prep_stmt = NULL;

	con = g_OptUserDataDB.GetConnection();
	if (con == NULL)
	{
		LOG4CXX_ERROR(g_logger, "CSqlOpt::ResetUserPasswordToDB:GetConnection = NULL.");
		return MY_SQL_ERROR;
	}

	try
	{
		prep_stmt = con->prepareStatement(std::string("UPDATE tb_user SET password = ?, pwdlevel=? WHERE ") + JoinAccountNameCondition(account_type));
		prep_stmt->setString(1, new_password);
		prep_stmt->setInt(2, PWD_HIGH);
		prep_stmt->setString(3, account_name);
		prep_stmt->executeUpdate();
	}
	catch (sql::SQLException& e)
	{
		LOG4CXX_ERROR(g_logger, "CSqlOpt::ResetUserPasswordToDB:optUserDataDB = " << e.what());
		b_ok = MY_SQL_ERROR;
	}

	SAFE_CLOSE(prep_stmt);
	g_OptUserDataDB.ReleaseConnection(con);

	return b_ok;
}

int CSqlOpt::ResetUserPasswordToDB(const int account_type, const std::string& account_name, const std::string& new_password, const std::string& old_username)
{
	int b_ok = SUCCESS;

	sql::Connection* con;
	sql::PreparedStatement* prep_stmt = NULL;

	con = g_OptUserDataDB.GetConnection();
	if (con == NULL)
	{
		LOG4CXX_ERROR(g_logger, "CSqlOpt::ResetUserPasswordToDB:GetConnection = NULL.");
		return MY_SQL_ERROR;
	}

	try
	{
		prep_stmt = con->prepareStatement(std::string("UPDATE tb_user SET password = ?, pwdlevel=? WHERE ") + JoinAccountNameCondition(account_type));
		prep_stmt->setString(1, new_password);
		prep_stmt->setInt(2, PWD_HIGH);
		prep_stmt->setString(3, account_name);
		prep_stmt->executeUpdate();
	}
	catch (sql::SQLException& e)
	{
		LOG4CXX_ERROR(g_logger, "CSqlOpt::ResetUserPasswordToDB:optUserDataDB = " << e.what());
		b_ok = MY_SQL_ERROR;
		goto END;
	}

	b_ok = UpdateUsernameToDeviceDB(account_name, old_username);

END:
	SAFE_CLOSE(prep_stmt);
	g_OptUserDataDB.ReleaseConnection(con);

	return b_ok;
}

int CSqlOpt::UpdateLastLoginTime(const std::string& username)
{
	int b_ok = SUCCESS;
	std::string timestr,timeyear,timemonth;
	timestr = utils::GetCurrentDayString();
	timeyear = timestr.substr(0,4);
	timemonth = utils::GetMonthString(timestr.substr(5,2));

	sql::Connection* con;
	sql::PreparedStatement* prep_stmt = NULL;

	con = g_OptUserDataDB.GetConnection();
	if (con == NULL)
	{
		LOG4CXX_ERROR(g_logger, "CSqlOpt::UpdateLastLoginTime:GetConnection = NULL.");
		return MY_SQL_ERROR;
	}

	try
	{
		con->setAutoCommit(false);

		prep_stmt = con->prepareStatement("UPDATE tb_user_behavior SET last_login_date = ? WHERE username=?");
		prep_stmt->setString(1, utils::GetCurrentDayString());
		prep_stmt->setString(2, username);
		prep_stmt->executeUpdate();
		SAFE_CLOSE(prep_stmt);

		std::string setmonthstr = timemonth + "=" + timemonth + "+1";
		prep_stmt = con->prepareStatement(std::string("UPDATE tb_user_month_activity_") + timeyear
				+ " SET login_frequency = login_frequency + 1, " + setmonthstr + " WHERE username=?");
		prep_stmt->setString(1, username);
		prep_stmt->executeUpdate();
		SAFE_CLOSE(prep_stmt);

		con->commit();
	}
	catch (sql::SQLException& e)
	{
		try
		{
			con->rollback();
		}
		catch(sql::SQLException& e)
		{
		}

		LOG4CXX_ERROR(g_logger, "CSqlOpt::UpdateLastLoginTime:optUserDataDB = " << e.what());
		b_ok = MY_SQL_ERROR;
	}
	
	con->setAutoCommit(true);
	SAFE_CLOSE(prep_stmt);
	g_OptUserDataDB.ReleaseConnection(con);

	return b_ok;
}

int CSqlOpt::GetUserDetailInfoFromDB(const int account_type, const std::string& account_name, USER_INFO& userInfo)
{
	int b_ok = SUCCESS;

	sql::Connection* con;
	sql::PreparedStatement* prep_stmt = NULL;
	sql::ResultSet* res = NULL;

	con = g_OptUserDataDB.GetConnection();
	if (con == NULL)
	{
		LOG4CXX_ERROR(g_logger, "CSqlOpt::GetUserDetailInfoFromDB:GetConnection = NULL.");
		return MY_SQL_ERROR;
	}

	try
	{
		prep_stmt = con->prepareStatement(std::string("SELECT * FROM tb_user WHERE ") + JoinAccountNameCondition(account_type));
		prep_stmt->setString(1, account_name);
		res = prep_stmt->executeQuery();

		while (res->next())
		{
			userInfo.security_mail = res->getString("mail");
		}

	}
	catch (sql::SQLException& e)
	{
		LOG4CXX_ERROR(g_logger, "CSqlOpt::GetUserDetailInfoFromDB:optDataDB = " << e.what());
		b_ok = MY_SQL_ERROR;
	}

	SAFE_CLOSE2(res, prep_stmt);
	g_OptUserDataDB.ReleaseConnection(con);

	return b_ok;
}

std::string CSqlOpt::JoinAccountNameCondition(const int account_type)
{
	std::string condition;
	switch(account_type)
	{
	case ACCOUNT_USERNAME:
		condition = " username=?";
		break;
	case ACCOUNT_MAIL:
		condition = " mail=?";
		break;
	case ACCOUNT_PHONE:
		condition = " phone=?";
		break;
	default:
		break;
	}

	return condition;
}

int CSqlOpt::UpdateUsernameToDeviceDB(const std::string& account_name, const std::string& old_username)
{
	int b_ok = SUCCESS;

	sql::Connection* DeviceCon;
	sql::PreparedStatement* prep_stmt = NULL;

	DeviceCon = g_OptDeviceDataDB.GetConnection();
	if (DeviceCon == NULL)
	{
		LOG4CXX_ERROR(g_logger, "CSqlOpt::UpdateDeviceDB:optDeviceSysDB.GetConnection = NULL");
		return MY_SQL_ERROR;
	}
	
	try
	{
		DeviceCon->setAutoCommit(false);
		prep_stmt = DeviceCon->prepareStatement("UPDATE tb_user_relation_channel SET user_name=? WHERE user_name=?");
		prep_stmt->setString(1, account_name);
		prep_stmt->setString(2, old_username);
		prep_stmt->execute();
		SAFE_CLOSE(prep_stmt);

		prep_stmt = DeviceCon->prepareStatement("UPDATE tb_user_relation_device SET user_name=? WHERE user_name=?");
		prep_stmt->setString(1, account_name);
		prep_stmt->setString(2, old_username);
		prep_stmt->execute();
		SAFE_CLOSE(prep_stmt);

		DeviceCon->commit();
	}
	catch(sql::SQLException& e)
	{
		try
		{
			DeviceCon->rollback();
		}
		catch(sql::SQLException& e)
		{
		}
		g_OptSqliteDB.InsertModifyMsgSqliteDB(account_name,old_username);
		LOG4CXX_ERROR(g_logger, "CSqlOpt::UpdateDeviceDB:optDeviceDataDB = UPDAT && UPDATE" << e.what()<<"\n old_username: "<< old_username<<"new_username: "<<account_name);
		b_ok = MY_SQL_ERROR;
	}

	DeviceCon->setAutoCommit(true);
	SAFE_CLOSE(prep_stmt);
	g_OptDeviceDataDB.ReleaseConnection(DeviceCon);

	return b_ok;
}
