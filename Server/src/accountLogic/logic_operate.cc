/*
 * logic_operate.cc
 *
 *  Created on: Mar 11, 2013
 *      Author: yaowei
 */

#include "logic_operate.h"
#include "json_opt.h"
#include "sql_opt.h"
#include "md5.h"
#include "mail_send.h"
#include "redis_conn_pool.h"
#include "redis_opt.h"
#include "../public/utils.h"
#include "../public/message.h"
#include "../public/redis_key.h"
#include "../public/socket_wrapper.h"

extern struct ClientInfo g_clientInfo;


CLogicOperate::CLogicOperate()
{
	jsonOpt_ptr_ = NULL;
	sqlOpt_ptr_ = NULL;
	redisOpt_ptr_ = NULL;
	jsonOpt_ptr_ = new CJsonOpt();
	assert(jsonOpt_ptr_ != NULL);
	sqlOpt_ptr_  = new CSqlOpt;
	assert(sqlOpt_ptr_ != NULL);
	redisOpt_ptr_ = new CRedisOpt;
	assert(redisOpt_ptr_ != NULL);
	result_ = SUCCESS;
}

CLogicOperate::~CLogicOperate()
{
	utils::SafeDelete(jsonOpt_ptr_);
	utils::SafeDelete(sqlOpt_ptr_);
	utils::SafeDelete(redisOpt_ptr_);
}

void CLogicOperate::StartLogicOpt(const std::string& message)
{
	/** 恢复客户端进行的转义处理 */
	std::string string = utils::ReplaceString(message, "\\\\r\\\\n", "\\r\\n");

	jsonOpt_ptr_->setJsonString(string);
	int message_type = 0;
	if(!jsonOpt_ptr_->JsonParseMessageType(message_type))
	{
		LOG4CXX_ERROR(g_logger, "CLogicOperate::StartLogicOpt:JsonParseMessageType invalid :" << string);
		return;
	}

	LOG4CXX_TRACE(g_logger, "CLogicOperate::StartLogicOpt:json = " << string);

	redisContext* con = CRedisConnPool::GetInstance()->GetRedisContext();
	redisOpt_ptr_->SetRedisContext(con);
	redisOpt_ptr_->SelectDB(SESSION);

	result_ = SUCCESS;

	if(message_type != IS_USER_EXIST && message_type != USER_REGISTER
	   && message_type != LOGIN && message_type != SEND_RESET_PASSWORD_MAIL
	   && message_type != RESET_PASSWORD_NOSESSION && message_type != GET_USER_DETAIL_INFO
	   && message_type != JUDGE_USER_PASSWORD_STRENGTH && message_type != GET_ACCOUNT_MAIL_NOSESSEION)
	{
		if(!jsonOpt_ptr_->JsonParseUserSessionId(session_id_))
		{
			result_ = JSON_PARSE_ERROR;
		}
		else
		{
			if (!redisOpt_ptr_->Get(session_id_, username_))
					result_ = SESSION_NOT_EXSIT;
		}
	}

	switch(message_type)
	{
	case IS_USER_EXIST:
		IsUserExist();
		break;
	case USER_REGISTER:
		UserResgister();
		break;
	case SET_ACCOUNT_INFO:
		SetAccountInfo();
		break;
	case GET_ACCOUNT_INFO:
		GetAccountInfo();
		break;
	case GET_ACCOUNT_MAIL_NOSESSEION:
		GetAccountMailWithNoSession();
		break;
	case JUDGE_USER_PASSWORD_STRENGTH:
		JudgeUserPasswordStrength();
		break;
	case LOGIN:
		UserLogin();
		break;
	case RESET_USERNAME_PASSWORD:
		ResetUserNameAndPassword();
		break;
	case RESET_PASSWORD_BY_MOBILE:
		ResetUserPasswordByMobile();
		break;
	case BIND_MAIL_OR_PHONE:
		BindMailOrPhone();
		break;
	case REPORT_CLIENT_PLATFORM_INFO:
		ReportClientPlatformInfo();
		break;
	case VERIFY_USERPASS:
		VerifyUserPassword();
		break;
	case MODIFY_USERPASS:
		ModifyUserPassword();
		break;
	case LOGOUT:
		UserLogout();
		break;
	case SEND_FEEDBACK_TO_MAIL:
		SendFeedbackToMail();
		break;
	case SEND_RESET_PASSWORD_MAIL:
		SendToSecurityMail();
		break;
	case RESET_PASSWORD_NOSESSION:
		ResetUserPasswordWithNoSession();
		break;
	case GET_ALARM_FLAG:
		GetCurrentAlarmFlag();
		break;
	case SET_ALARM_FLAG:
		SetCurrentAlarmFlag();
		break;
	case GET_USER_DETAIL_INFO:
		GetUserDetailInfo();
		break;
	default:
		LOG4CXX_WARN(g_logger, "CLogicOperate::StartLogicOpt:message_type invadlid.");
		break;
	}

	SendToDispatch();
	CRedisConnPool::GetInstance()->ReleaseRedisContext(con);
}

void CLogicOperate::SendToDispatch()
{
	responseToDispatch_.append(CRLF);
	if(responseToDispatch_.length() >= DATA_BUFFER_SIZE)
	{
		LOG4CXX_WARN(g_logger, "CLogicOperate::SendToDispatch data too large. data = " << responseToDispatch_);
		return;
	}
	if (!SocketOperate::WriteSfd(g_clientInfo.sfd, responseToDispatch_.c_str(), responseToDispatch_.length()))
	{
		LOG4CXX_WARN(g_logger, "CLogicOperate::SendToDispatch:sendto failed.");
	}
	LOG4CXX_TRACE(g_logger, "CLogicOperate::SendToDispatch:sendto = " << responseToDispatch_);
}

void CLogicOperate::GetAccountType(const std::string& account_name, int& account_type)
{
	bool flag = true;
	/* 邮件 */
	std::string szReg;
	szReg = "^[A-Za-z0-9\\-\\_\\.]+@[A-Za-z0-9]+([\\.-][A-Za-z0-9]+)*\\.[A-Za-z0-9]+$";
	boost::regex reg_mail(szReg);
	flag = boost::regex_match(account_name, reg_mail);
	if (flag)
	{
		account_type = ACCOUNT_MAIL;
		return;
	}

	/* 电话号码 */
	szReg = "^(1(([357][0-9])|(47)|[8][0123456789]))\\d{8}$";
	boost::regex reg_phone(szReg);
	flag = boost::regex_match(account_name, reg_phone);
	if (flag)
	{
		account_type = ACCOUNT_PHONE;
		return;
	}

	/* 全为数字 */
	szReg = "^([+-]?)\\d*\\.?\\d+$";
	boost::regex reg_number(szReg);
	flag = boost::regex_match(account_name, reg_number);
	if (flag)
	{
		account_type = ACCOUNT_INVALID;
		return;
	}

	/* 用户名，仿照京东 */
	szReg = "^[A-Za-z0-9\\-\\_]+$";
	boost::regex reg_normal(szReg);
	flag = boost::regex_match(account_name, reg_normal);
	if (flag)
	{
		account_type = ACCOUNT_USERNAME;
		return;
	}

	account_type = ACCOUNT_INVALID;
	return;
}

bool CLogicOperate::CheckAccountType(const std::string& account_name, int& account_type)
{
	bool flag = true;
	/* 邮件 */
	std::string szReg;
	szReg = "^[A-Za-z0-9\\-\\_\\.]+@[A-Za-z0-9]+([\\.-][A-Za-z0-9]+)*\\.[A-Za-z0-9]+$";
	boost::regex reg_mail(szReg);
	flag = boost::regex_match(account_name, reg_mail);
	if (flag)
	{
		account_type = ACCOUNT_MAIL;
		return true;
	}

	/* 电话号码 */
	szReg = "^(1(([357][0-9])|(47)|[8][0123456789]))\\d{8}$";
	boost::regex reg_phone(szReg);
	flag = boost::regex_match(account_name, reg_phone);
	if (flag)
	{
		account_type = ACCOUNT_PHONE;
		return true;
	}

	/* 全为数字 */
	szReg = "^([+-]?)\\d*\\.?\\d+$";
	boost::regex reg_number(szReg);
	flag = boost::regex_match(account_name, reg_number);
	if (flag)
	{
		account_type = ACCOUNT_INVALID;
		return true;
	}

	/* 用户名，仿照京东 */
	szReg = "^[A-Za-z0-9_\\-\\u4e00-\\u9fa5]+$";
	boost::regex reg_normal(szReg);
	flag = boost::regex_match(account_name, reg_normal);
	if (flag)
	{
		account_type = ACCOUNT_USERNAME;
		return true;
	}

	account_type = ACCOUNT_INVALID;

	if(USER_NOT_EXIST == sqlOpt_ptr_->IsUserExistFromDB(ACCOUNT_USERNAME, account_name))
	{
		return false;
	}

	return true;
}

void CLogicOperate::IsUserExist()
{
	std::string account_name;
	int account_type = ACCOUNT_INVALID;

	if (!jsonOpt_ptr_->JsonParseUsername(account_name))
	{
		result_ = JSON_PARSE_ERROR;
		goto GET_RESPONSE;
	}

	if(!CheckAccountType(account_name, account_type))
	{
		result_ = USER_NOT_EXIST;
		goto GET_RESPONSE;
	}

	if(ACCOUNT_INVALID == account_type)
	{
		result_ = sqlOpt_ptr_->IsUserExistFromDB(ACCOUNT_USERNAME, account_name);
	}
	else
	{
		result_ = sqlOpt_ptr_->IsUserExistFromDB(account_type, account_name);
	}

GET_RESPONSE:
	responseToDispatch_ = jsonOpt_ptr_->JsonJoinCommonOpt(IS_USER_EXIST_RESPONSE, result_);

}

void CLogicOperate::UserResgister()
{
	std::string account_name, password, custom_type;
	int account_type = ACCOUNT_INVALID;

	if(!jsonOpt_ptr_->JsonParseUserRegister(account_name, password, custom_type))
	{
		result_ = JSON_PARSE_ERROR;
		goto GET_RESPONSE;
	}

	GetAccountType(account_name, account_type);

	if(ACCOUNT_INVALID == account_type)
	{
		result_ = ACCOUNTNAME_INVALID;
		goto GET_RESPONSE;
	}

	result_ = sqlOpt_ptr_->UserRegisterToDB(account_type, account_name, password, custom_type);

GET_RESPONSE:
	responseToDispatch_ = jsonOpt_ptr_->JsonJoinCommonOpt(USER_REGISTER_RESPONSE, result_);

}

void CLogicOperate::SetAccountInfo()
{
	AccountInfo account_info;
	account_info.username = username_;

	if(!jsonOpt_ptr_->JsonParseSetAccountInfo(account_info))
	{
		result_ = JSON_PARSE_ERROR;
		goto GET_RESPONSE;
	}

	result_ = sqlOpt_ptr_->SetAccountInfoToDB(account_info);

GET_RESPONSE:
	responseToDispatch_ = jsonOpt_ptr_->JsonJoinCommonOpt(SET_ACCOUNT_INFO_RESPONSE, result_);
}

void CLogicOperate::GetAccountInfo()
{
	AccountInfo account_info;

	result_ = sqlOpt_ptr_->GetAccountInfoFromDB(username_, account_info);

	responseToDispatch_ = jsonOpt_ptr_->JsonJoinGetAccountInfo(GET_ACCOUNT_INFO_RESPONSE, result_, account_info);
}

void CLogicOperate::GetAccountMailWithNoSession()
{
	std::string account_name, mail;
	int account_type = ACCOUNT_INVALID;

	if (!jsonOpt_ptr_->JsonParseUsername(account_name))
	{
		result_ = JSON_PARSE_ERROR;
		goto GET_RESPONSE;
	}

	if(!CheckAccountType(account_name, account_type))
	{
		result_ = USER_NOT_EXIST;
		goto GET_RESPONSE;
	}

	if(ACCOUNT_INVALID == account_type)
	{
		result_ = sqlOpt_ptr_->GetAccountMailWithNoSessionFromDB(ACCOUNT_USERNAME, account_name, mail);
	}
	else
	{
		result_ = sqlOpt_ptr_->GetAccountMailWithNoSessionFromDB(account_type, account_name, mail);
	}



GET_RESPONSE:

	responseToDispatch_ = jsonOpt_ptr_->JsonJoinGetAccountMailWithNoSession(GET_ACCOUNT_MAIL_NOSESSEION_RESPONSE, result_, mail);
}

void CLogicOperate::JudgeUserPasswordStrength()
{
	std::string account_name;
	int account_type = ACCOUNT_INVALID;

	if (!jsonOpt_ptr_->JsonParseUsername(account_name))
	{
		result_ = JSON_PARSE_ERROR;
		goto GET_RESPONSE;
	}

	if(!CheckAccountType(account_name, account_type))
	{
		result_ = USER_NOT_EXIST;
		goto GET_RESPONSE;
	}

	if(ACCOUNT_INVALID == account_type)
	{
		result_ = sqlOpt_ptr_->GetUserPasswordStrength(ACCOUNT_USERNAME, account_name);
	}
	else
	{
		result_ = sqlOpt_ptr_->GetUserPasswordStrength(account_type, account_name);
	}

GET_RESPONSE:
	responseToDispatch_ = jsonOpt_ptr_->JsonJoinCommonOpt(JUDGE_USER_PASSWORD_STRENGTH_RESPONSE, result_);
}

void CLogicOperate::UserLogin()
{
	std::string account_name, username, password_in, password, session_id;
	int account_type = ACCOUNT_INVALID;
	int pwdlevel;

	if (!jsonOpt_ptr_->JsonParseUserLogin(account_name, password_in))
	{
		result_ = JSON_PARSE_ERROR;
		goto GET_RESPONSE;
	}

	if (!CheckAccountType(account_name, account_type))
	{
		result_ = USER_NOT_EXIST;
		goto GET_RESPONSE;
	}

	if(ACCOUNT_INVALID == account_type)
	{
		result_ = sqlOpt_ptr_->GetUserpassFromDB(ACCOUNT_USERNAME, account_name, username, password, pwdlevel);
	}
	else
	{
		result_ = sqlOpt_ptr_->GetUserpassFromDB(account_type, account_name, username, password, pwdlevel);
	}

	if (SUCCESS != result_)
	{
		if (SQL_NOT_FIND == result_)
		{
			result_ = USER_NOT_EXIST;
		}
		goto GET_RESPONSE;
	}

	if (password_in.compare(password) == 0)
	{
		if(CreateUserSession(username, session_id))
		{
			if(ACCOUNT_INVALID == account_type)
			{
				result_ = ACCOUNTNAME_OTHER;
			}
			else if(PWD_LOW == pwdlevel)
			{
				result_ = PASSWORD_DANGER;
			}
			else
			{
				result_ = SUCCESS;
			}
			sqlOpt_ptr_->UpdateLastLoginTime(username);
		}
		else
			result_ = REDIS_OPT_ERROR;
	}
	else
		result_ = PASSWORD_ERROR;

GET_RESPONSE:

	responseToDispatch_ = jsonOpt_ptr_->JsonJoinUserLogin(result_, session_id);
}

void CLogicOperate::ResetUserPasswordByMobile()
{
	std::string password, account_name;

	if (!jsonOpt_ptr_->JsonParseResetUserPassword(account_name, password))
	{
		result_ = JSON_PARSE_ERROR;
		goto GET_RESPONSE;
	}

	/* 重置用户密码 */
	result_ = sqlOpt_ptr_->ResetUserPasswordToDB(ACCOUNT_USERNAME, username_, password, account_name);

GET_RESPONSE:
	responseToDispatch_ = jsonOpt_ptr_->JsonJoinCommonOpt(RESET_PASSWORD_BY_MOBILE_RESPONSE, result_);
}

void CLogicOperate::ResetUserNameAndPassword()
{
	std::string account_name,password;
	int account_type;

	if(!jsonOpt_ptr_->JsonParseModifyUserName(account_name,password))
	{
		result_ = JSON_PARSE_ERROR;
		goto GET_RESPONSE;
	}

	GetAccountType(account_name, account_type);

	if(ACCOUNT_INVALID == account_type)
	{
		result_ = ACCOUNTNAME_INVALID;
		goto GET_RESPONSE;
	}

	result_ = sqlOpt_ptr_->ModifyUserNameToDB(account_type, account_name, username_, password);

GET_RESPONSE:
	responseToDispatch_ = jsonOpt_ptr_->JsonJoinCommonOpt(RESET_USERNAME_PASSWORD_RESPONSE, result_);
}

bool CLogicOperate::CreateUserSession(const std::string& username, std::string& session_id)
{
	session_id = ::md5(username + utils::NowtimeString());
	if(!redisOpt_ptr_->Set(session_id, username))
	{
		LOG4CXX_WARN(g_logger, "CLogicOperate::CreateUserSession failed. username = " << username);
		return false;
	}

	/* 对于一次访问在3天内session有效 */
	int valid_sec = 3*12*60*60;
	if(!redisOpt_ptr_->Expire(session_id, valid_sec))
	{
		LOG4CXX_WARN(g_logger, "CLogicOperate::Session Expire failed. username = " << username);
	}

	return true;
}

void CLogicOperate::BindMailOrPhone()
{
	std::string mailOrPhone;
	int account_type = ACCOUNT_INVALID;

	if (!jsonOpt_ptr_->JsonParseMailOrPhone(mailOrPhone))
	{
		result_ = JSON_PARSE_ERROR;
		goto GET_RESPONSE;
	}

	GetAccountType(mailOrPhone, account_type);

	if(ACCOUNT_INVALID == account_type)
	{
		result_ = ACCOUNT_INVALID;
		goto GET_RESPONSE;
	}

	result_ = sqlOpt_ptr_->BindMailOrPhoneToDB(account_type, mailOrPhone, username_);

GET_RESPONSE:
	responseToDispatch_ = jsonOpt_ptr_->JsonJoinCommonOpt(BIND_MAIL_OR_PHONE_RESPONSE, result_);
}

void CLogicOperate::ReportClientPlatformInfo()
{
	CLIENT_PLATFORM_INFO clientPlatformInfo;

	if (!jsonOpt_ptr_->JsonParseReportClientPlatformInfo(clientPlatformInfo))
	{
		result_ = JSON_PARSE_ERROR;
		goto GET_RESPONSE;
	}

	/* 上报客户端的登录信息 */
	if (!ReportClientInfoToCache(username_, clientPlatformInfo))
		result_ = REDIS_OPT_ERROR;

GET_RESPONSE:
	responseToDispatch_ = jsonOpt_ptr_->JsonJoinCommonOpt(REPORT_CLIENT_PLATFORM_INFO_RESPONSE, result_);
}

bool CLogicOperate::ReportClientInfoToCache(const std::string& username, const CLIENT_PLATFORM_INFO& clientPlatformInfo)
{
	if (!redisOpt_ptr_->SelectDB(STATUS))
		return false;

	/* 处理苹果离线推送混乱问题 */
	std::string get_username;
	if(redisOpt_ptr_->Get(clientPlatformInfo.moblie_id, get_username))
	{
		if(get_username.compare(username) != 0)
		{
			redisOpt_ptr_->Hdel(RedisKeyClientLoginInfo(get_username), KEY_MOBILE_ID);
		}
	}

	// clientLoginInfo hold 7 days
	if(!(redisOpt_ptr_->Hset(RedisKeyClientLoginInfo(username), KEY_LOGIN_PLATFORM, clientPlatformInfo.terminal_type)
		&& redisOpt_ptr_->Hset(RedisKeyClientLoginInfo(username), KEY_MOBILE_ID, clientPlatformInfo.moblie_id)
		&& redisOpt_ptr_->Hset(RedisKeyClientLoginInfo(username), KEY_LANGUAGE_TYPE, clientPlatformInfo.language_type)
		&& redisOpt_ptr_->Expire(RedisKeyClientLoginInfo(username), 3600*24*7)))
		return false;


	/* 处理苹果离线推送混乱问题 */
	redisOpt_ptr_->Set(clientPlatformInfo.moblie_id, username);

	return true;
}

void CLogicOperate::GetCurrentAlarmFlag()
{
	std::string s_alarm_flag;
	int alarm_flag = 0;

	if (SessionNotExist())
	{
		goto GET_RESPONSE;
	}

	if (!redisOpt_ptr_->SelectDB(STATUS))
	{
		result_ = REDIS_OPT_ERROR;
		goto GET_RESPONSE;
	}

	if (!redisOpt_ptr_->Hget(RedisKeyClientLoginInfo(username_), KEY_ALARM_FLAG, s_alarm_flag))
	{

		result_ = REDIS_OPT_ERROR;
		goto GET_RESPONSE;
	}
	alarm_flag = atoi(s_alarm_flag.c_str());

GET_RESPONSE:
	responseToDispatch_ = jsonOpt_ptr_->JsonJoinGetCurrentAlarmFlag(result_, alarm_flag);

}

void CLogicOperate::SetCurrentAlarmFlag()
{
	int alarm_flag = 0;
	std::string mobile_id;

	if (SessionNotExist())
	{
		goto GET_RESPONSE;
	}

	if (!jsonOpt_ptr_->JsonParseSetCurrentAlarmFlag(alarm_flag, mobile_id))
	{
		result_ = JSON_PARSE_ERROR;
		goto GET_RESPONSE;
	}

	if (ALARM_ON == alarm_flag)
	{
		/* 增加缓存中用户的设备id */
		if (!redisOpt_ptr_->SelectDB(STATUS))
		{
			result_ = REDIS_OPT_ERROR;
			goto GET_RESPONSE;
		}
		if (!redisOpt_ptr_->Hset(RedisKeyClientLoginInfo(username_), KEY_MOBILE_ID, mobile_id))
		{
			result_ = REDIS_OPT_ERROR;
			goto GET_RESPONSE;
		}

		/* 开启推送标识 */
		if (!redisOpt_ptr_->Hset(RedisKeyClientLoginInfo(username_), KEY_ALARM_FLAG, alarm_flag))
		{
			result_ = REDIS_OPT_ERROR;
			goto GET_RESPONSE;
		}
	}
	else if(ALARM_OFF == alarm_flag)
	{
		/* 关闭推送标识 */
		if (!redisOpt_ptr_->SelectDB(STATUS))
		{
			result_ = REDIS_OPT_ERROR;
			goto GET_RESPONSE;
		}

		if (!redisOpt_ptr_->Hset(RedisKeyClientLoginInfo(username_), KEY_ALARM_FLAG, alarm_flag))
		{
			result_ = REDIS_OPT_ERROR;
			goto GET_RESPONSE;
		}

		/* 清除缓存中用户的设备id */
		if (!redisOpt_ptr_->Hdel(RedisKeyClientLoginInfo(username_), KEY_MOBILE_ID))
		{
			result_ = REDIS_OPT_ERROR;
			goto GET_RESPONSE;
		}
	}
	else
	{
		LOG4CXX_WARN(g_logger, "CLogicOperate::SetCurrentAlarmFlag alarm_flag invalid = " << alarm_flag);
	}

GET_RESPONSE:
	responseToDispatch_ = jsonOpt_ptr_->JsonJoinCommonOpt(SET_ALARM_FLAG_RESPONSE, result_);
}

void CLogicOperate::VerifyUserPassword()
{
	std::string password_in;

	if (SessionNotExist())
	{
		goto GET_RESPONSE;
	}

	if (!jsonOpt_ptr_->JsonParseUserPassword(password_in))
	{
		result_ = JSON_PARSE_ERROR;
		goto GET_RESPONSE;
	}

	result_ = sqlOpt_ptr_->VerifyUserPasswordFromDB(username_, password_in);

GET_RESPONSE:
	responseToDispatch_ = jsonOpt_ptr_->JsonJoinCommonOpt(VERIFY_USERPASS_RESPONSE, result_);
}

void CLogicOperate::ModifyUserPassword()
{
	std::string password, new_password;

	if (SessionNotExist())
	{
		goto GET_RESPONSE;
	}

	if (!jsonOpt_ptr_->JsonParseModifyUserPassword(password, new_password))
	{
		result_ = JSON_PARSE_ERROR;
		goto GET_RESPONSE;
	}

	result_ = sqlOpt_ptr_->ModifyUserPasswordToDB(username_, password, new_password);

GET_RESPONSE:
	responseToDispatch_ = jsonOpt_ptr_->JsonJoinCommonOpt(MODIFY_USERPASS_RESPONSE, result_);
}

void CLogicOperate::SendFeedbackToMail()
{
	std::string feedback, mail_address;

	if (SessionNotExist())
	{
		goto GET_RESPONSE;
	}

	if (!jsonOpt_ptr_->JsonParseSendFeedbackToMail(feedback, mail_address))
	{
		result_ = JSON_PARSE_ERROR;
		goto GET_RESPONSE;
	}

	sendFeedbackInfo_.username = username_;
	sendFeedbackInfo_.mail_address = mail_address;
	sendFeedbackInfo_.feedback	= feedback;
	/* 发邮件 */
	if (!CreateThreadForSendMail(SendFeedbackToMailThread, (void*) this))
	{
		result_ = SEND_MAIL_FAILED;
	}

GET_RESPONSE:
	responseToDispatch_ = jsonOpt_ptr_->JsonJoinCommonOpt(SEND_FEEDBACK_TO_MAIL_RESPONSE, result_);

}

void* CLogicOperate::SendFeedbackToMailThread(void *arg)
{
	CLogicOperate *pThis = static_cast<CLogicOperate*>(arg);

	MAIL_INFO_ mail_info;
	memset(&mail_info, 0, sizeof(MAIL_INFO_));
	mail_info.charset = UTF8;
	strcpy(mail_info.toMailAddrList,pThis->sendFeedbackInfo_.mail_address.c_str());
	strcpy(mail_info.mailSubject, "问题反馈");

	std::string mail_body = std::string("<html>") + pThis->sendFeedbackInfo_.feedback + std::string("</html>");
	strcpy(mail_info.mailText, mail_body.c_str());

	if (SUCCESS != mail_send(&mail_info))
	{
		LOG4CXX_ERROR(g_logger,
				" CLogicOperate::SendFeedbackToMailThread:mail_send failed.");
	} else
	{
		LOG4CXX_INFO(g_logger,
				" CLogicOperate::SendFeedbackToMailThread:mail_send success.");
	}

	return NULL;
}

void CLogicOperate::UserLogout()
{
	if (SessionNotExist())
	{
		goto GET_RESPONSE;
	}

	if(!DeleteUserSession(session_id_))
	{
		result_ = REDIS_OPT_ERROR;
	}

GET_RESPONSE:
	responseToDispatch_ = jsonOpt_ptr_->JsonJoinCommonOpt(LOGOUT_RESPONSE, result_);
}

bool CLogicOperate::DeleteUserSession(const std::string& session_id)
{
	return redisOpt_ptr_->Del(session_id);
}

void CLogicOperate::SendToSecurityMail()
{
	std::string username, security_mail, session_id;
	if (!jsonOpt_ptr_->JsonParseSendToSecurityMail(username, security_mail))
	{
		result_ = JSON_PARSE_ERROR;
		goto GET_RESPONSE;
	}

	/* 创建重置密码的session */
	if (!CreateUserResetPasswordSession(username, session_id))
	{
		result_ = REDIS_OPT_ERROR;
		goto GET_RESPONSE;
	}

	sendmailInfo_.username 		= username;
	sendmailInfo_.session_id 	= session_id;
	sendmailInfo_.security_mail	= security_mail;

	/* 发邮件 */
	if(!CreateThreadForSendMail(SendSecurityMailThread, (void*)this))
	{
		result_ = SEND_MAIL_FAILED;
	}

GET_RESPONSE:
	responseToDispatch_ = jsonOpt_ptr_->JsonJoinCommonOpt(SEND_RESET_PASSWORD_MAIL_RESPONSE, result_);
}

bool CLogicOperate::CreateThreadForSendMail(void *(*func)(void *), void *arg)
{
	pthread_t thread;
	pthread_attr_t attr;
	int ret;

	pthread_attr_init(&attr);

	if ((ret = pthread_create(&thread, &attr, func, arg)) != 0)
	{
		LOG4CXX_FATAL(g_logger, "CMailOpt::CreateThreadForMailOpt:Can't create thread:" << strerror(ret));
		return false;
	}

	return true;
}

void* CLogicOperate::SendSecurityMailThread(void *arg)
{
	CLogicOperate *pThis = static_cast<CLogicOperate*>(arg);

	MAIL_INFO_ mail_info;
	memset(&mail_info, 0, sizeof(MAIL_INFO_));
	mail_info.charset = UTF8;
	strcpy(mail_info.toMailAddrList, pThis->sendmailInfo_.security_mail.c_str());
	strcpy(mail_info.mailSubject, "安易视——邮箱找回密码");

	std::string mail_body = pThis->JsonString(utils::GetCurrentTime(), pThis->sendmailInfo_.username, pThis->sendmailInfo_.session_id);
	strcpy(mail_info.mailText, mail_body.c_str());

	if (SUCCESS != mail_send(&mail_info))
	{
		LOG4CXX_ERROR(g_logger, " CLogicOperate::SendSecurityMailThread:mail_send failed.");
	}
	else
	{
		LOG4CXX_INFO(g_logger, " CLogicOperate::SendSecurityMailThread:mail_send success.");
	}

	return NULL;
}

bool CLogicOperate::CreateUserResetPasswordSession(const std::string& username, std::string& session_id)
{
	session_id = ::md5(username + utils::NowtimeString());
	if (!redisOpt_ptr_->Set(session_id, username))
	{
		LOG4CXX_WARN(g_logger, "CLogicOperate::CreateUserResetPasswordSession:Set failed. username = " << username);
		return false;
	}

	//session时长2小时
	if (!redisOpt_ptr_->Expire(session_id, 2 * 60 * 60))
	{
		LOG4CXX_WARN(g_logger, "CLogicOperate::CreateUserResetPasswordSession:Expire failed. username = " << username);
		return false;
	}
	return true;
}

std::string CLogicOperate::JsonString(const std::string& current_time, const std::string& username, const std::string& session_id)
{
	std::string link = std::string("https://www.jovemail.com/account/getpwd.php?user=") + username + std::string("&sid=") + session_id;
	std::string mail_body = std::string("<html>亲爱的用户：您好！您在") + current_time +
							std::string("提交了邮箱找回密码请求，请点击下面的链接修改密码。\r\n") +
							std::string("<a href=") + link + std::string(">") + link + std::string("</a>") +
							std::string("\r\n(如果您无法点击此链接，\
							请将它复制到浏览器地址栏后访问)为了保证您帐号的安全，该链接有效期为2小时，并且点击一次后失效！ （安易视管理员）</html>");

	return mail_body;
}

void CLogicOperate::ResetUserPasswordWithNoSession()
{
	std::string username, new_password;
	int account_type = ACCOUNT_INVALID;

	if (!jsonOpt_ptr_->JsonParseResetUserPassword(username, new_password))
	{
		result_ = JSON_PARSE_ERROR;
		goto GET_RESPONSE;
	}

	if(!CheckAccountType(username, account_type))
	{
		result_ = USER_NOT_EXIST;
		goto GET_RESPONSE;
	}

	if(ACCOUNT_INVALID == account_type)
	{
		/* 重置用户密码 */
		result_ = sqlOpt_ptr_->WebResetUserPasswordToDB(ACCOUNT_USERNAME, username, new_password);
	}
	else
	{
		/* 重置用户密码 */
		result_ = sqlOpt_ptr_->WebResetUserPasswordToDB(account_type, username, new_password);
	}

GET_RESPONSE:
	responseToDispatch_ = jsonOpt_ptr_->JsonJoinCommonOpt(RESET_PASSWORD_NOSESSION_RESPONSE, result_);

}

void CLogicOperate::GetUserDetailInfo()
{
	USER_INFO userInfo;
	std::string username;
	int account_type = ACCOUNT_INVALID;

	if(!jsonOpt_ptr_->JsonParseUsername(username))
	{
		result_ = JSON_PARSE_ERROR;
		goto GET_RESPONSE;
	}

	GetAccountType(username, account_type);

	if(ACCOUNT_INVALID == account_type)
	{
		result_ = ACCOUNTNAME_INVALID;
		goto GET_RESPONSE;
	}

	result_ = sqlOpt_ptr_->GetUserDetailInfoFromDB(account_type, username, userInfo);

GET_RESPONSE:
	responseToDispatch_ = jsonOpt_ptr_->JsonJoinGetUserDetailInfo(result_, userInfo);
}



