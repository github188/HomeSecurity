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

enum
{
	ACCOUNT_INVALID = -1,
	ACCOUNT_USERNAME = 0,
	ACCOUNT_MAIL,
	ACCOUNT_PHONE,
};

class CSqlOpt
{
public:
	CSqlOpt();
	virtual ~CSqlOpt();

public:

	int IsUserExistFromDB(const int account_type, const std::string& account_name);

	bool GetUsername(std::string& username);

	int UserRegisterToDB(const int account_type, const std::string& account_name, const std::string& password, const std::string& custom_type);

	int SetAccountInfoToDB(const AccountInfo& account_info);

	int GetAccountInfoFromDB(const std::string& account_name, AccountInfo& account_info);

	int GetAccountMailWithNoSessionFromDB(const int account_type, const std::string& account_name, std::string& mail);

	int GetUserPasswordStrength(const int account_type, const std::string& account_name);

	int GetUserpassFromDB(const int account_type, const std::string& account_name, std::string& username, std::string& password, int& pwdlevel);

	int BindMailOrPhoneToDB(const int account_type, const std::string& mailOrPhone, const std::string& username);

	int VerifyUserPasswordFromDB(const std::string& username, const std::string& password_in);

	int ModifyUserPasswordToDB(const std::string& username, const std::string& password, const std::string& new_password);

	int ResetUserPasswordToDB(const int account_type, const std::string& account_name, const std::string& new_password, const std::string& old_username);

	int WebResetUserPasswordToDB(const int account_type, const std::string& account_name, const std::string& new_password);

	int UpdateLastLoginTime(const std::string& username);

	int GetUserDetailInfoFromDB(const int account_type, const std::string& account_name, USER_INFO& userInfo);

	int ModifyUserNameToDB(const int account_type,const std::string& account_name,const std::string& old_username,const std::string& password);

private:

	std::string JoinAccountNameCondition(const int account_type);
	int UpdateUsernameToDeviceDB(const std::string& account_name, const std::string& old_username);
	static std::string GlobalUserName;
};

#endif /* SQL_OPT_H_ */
