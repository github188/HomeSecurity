/*
 * logic_operate.cc
 *
 *  Created on: Mar 11, 2013
 *      Author: yaowei
 */

#include "logic_operate.h"
#include "json_opt.h"
#include "redis_conn_pool.h"
#include "redis_opt.h"
#include "../public/utils.h"
#include "../public/message.h"
#include "../public/redis_key.h"
#include "../public/socket_wrapper.h"

extern struct ClientInfo g_clientInfo;
extern INetChannelInterface* g_alarmSeverChannel;

CLogicOperate::CLogicOperate()
{
	jsonOpt_ptr_ = NULL;
	redisOpt_ptr_ = NULL;
	jsonOpt_ptr_ = new CJsonOpt();
	assert(jsonOpt_ptr_ != NULL);
	redisOpt_ptr_ = new CRedisOpt;
	assert(redisOpt_ptr_ != NULL);
	result_ = SUCCESS;
	assert(0 == pthread_mutex_init(&write_mutex, NULL));
}

CLogicOperate::~CLogicOperate()
{
	utils::SafeDelete(jsonOpt_ptr_);
	utils::SafeDelete(redisOpt_ptr_);
	pthread_mutex_destroy(&write_mutex);
}

void CLogicOperate::StartLogicOpt(const std::string& message)
{
	/** 恢复客户端进行的转义处理 */
	std::string message_in = utils::ReplaceString(message, "\\\\r\\\\n", "\\r\\n");

	LOG4CXX_TRACE(g_logger, "CLogicOperate::StartLogicOpt:json_message = " << message_in);

	std::string session_id, request_custom_string, to_alarm_string, response_custom_string;
	redisContext* con = CRedisConnPool::GetInstance()->GetRedisContext();
	redisOpt_ptr_->SetRedisContext(con);
	redisOpt_ptr_->SelectDB(SESSION);

	if(!jsonOpt_ptr_->JsonParseCommonField(message_in, session_id, request_custom_string))
	{
		result_ = SESSION_NOT_EXSIT;
		responseToDispatch_ = jsonOpt_ptr_->JsonJoinDirectToUserJson(result_);
		goto SEND_RESPONSE;
	}

	if (!redisOpt_ptr_->Get(session_id, username_))
	{
		result_ = SESSION_NOT_EXSIT;
		responseToDispatch_ = jsonOpt_ptr_->JsonJoinDirectToUserJson(result_);
		goto SEND_RESPONSE;
	}

	/* 重组发向告警服务器的json和告警服务器进行交互  */
	LOG4CXX_TRACE(g_logger, "CLogicOperate::StartLogicOpt:request_custom_string = " << request_custom_string);

	result_ = g_alarmSeverChannel->GetResponseByRequestPersistConnectionServer(ALARM_PROCESS, username_, request_custom_string, response_custom_string);

	LOG4CXX_TRACE(g_logger, "CLogicOperate::StartLogicOpt:response_custom_string = " << response_custom_string);

	/* 得到告警服务器的成功回复后重组json发向客户端 */
	responseToDispatch_ = jsonOpt_ptr_->JsonJoinToUserJson(result_, response_custom_string);

SEND_RESPONSE:
	SendToDispatch();
	CRedisConnPool::GetInstance()->ReleaseRedisContext(con);
}

void CLogicOperate::SendToDispatch()
{
	pthread_mutex_lock(&write_mutex);
	responseToDispatch_.append(CRLF);
	if (responseToDispatch_.length() >= DATA_BUFFER_SIZE)
	{
		LOG4CXX_WARN(g_logger, "CLogicOperate::SendToDispatch data too large. data = " << responseToDispatch_);
		return;
	}
	if (!SocketOperate::WriteSfd(g_clientInfo.sfd, responseToDispatch_.c_str(), responseToDispatch_.length()))
	{
		LOG4CXX_WARN(g_logger, "CLogicOperate::SendToDispatch:sendto failed.");
	}
	LOG4CXX_TRACE(g_logger, "CLogicOperate::SendToDispatch:sendto = " << responseToDispatch_);
	pthread_mutex_unlock(&write_mutex);
}


