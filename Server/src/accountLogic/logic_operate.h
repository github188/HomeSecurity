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

	void GetAccountType(const std::string& account_name, int& account_type);
	bool CheckAccountType(const std::string& account_name, int& account_type);

	void ResetUserNameAndPassword();
	void ResetUserPasswordByMobile();

	void IsUserExist();

	void UserResgister();

	void SetAccountInfo();
	void GetAccountInfo();

	void GetAccountMailWithNoSession();

	void JudgeUserPasswordStrength();

	void UserLogin();
	bool CreateUserSession(const std::string& username, std::string& session_id);

	void BindMailOrPhone();

	void ReportClientPlatformInfo();

	bool ReportClientInfoToCache(const std::string& username, const CLIENT_PLATFORM_INFO& clientPlatformInfo);

	bool SessionNotExist() {return result_ == SESSION_NOT_EXSIT; }

	void GetCurrentAlarmFlag();
	void SetCurrentAlarmFlag();
	void CancelServerPushFunc();

	void VerifyUserPassword();

	void ModifyUserPassword();

	void SendFeedbackToMail();
	static void *SendFeedbackToMailThread(void *arg);


	void UserLogout();
	bool DeleteUserSession(const std::string& session_id);

	void SendToSecurityMail();
	bool CreateUserResetPasswordSession(const std::string& username, std::string& session_id);
	std::string JsonString(const std::string& current_time, const std::string& username, const std::string& session_id);
	bool CreateThreadForSendMail(void *(*func)(void *), void *arg);
	static void *SendSecurityMailThread(void *arg);

	void ResetUserPasswordWithNoSession();

	void GetUserDetailInfo();

	void SendToDispatch();

private:

	CJsonOpt *jsonOpt_ptr_;
	CSqlOpt		 *sqlOpt_ptr_;
	CRedisOpt	*redisOpt_ptr_;
	int result_;
	std::string responseToDispatch_;

	typedef struct SendMail_Info_
	{
		std::string username, security_mail, session_id;
	}SENDMAIL_INFO;

	SENDMAIL_INFO sendmailInfo_;

	typedef struct SendFeedback_info_
	{
		std::string username;
		std::string mail_address;
		std::string feedback;
	}SEND_FEEDBACK_INFO;

	SEND_FEEDBACK_INFO sendFeedbackInfo_;

	std::string session_id_;
	std::string username_;

};

#endif /* LOGIC_OPERATE_H_ */
