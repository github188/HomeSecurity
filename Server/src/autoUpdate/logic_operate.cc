/*
 * logic_operate.cc
 *
 *  Created on: Mar 11, 2013
 *      Author: yaowei
 */

#include "logic_operate.h"
#include "json_opt.h"
#include "sql_opt.h"
#include "../public/utils.h"
#include "../public/message.h"
#include "../public/redis_key.h"
#include "../public/socket_wrapper.h"

extern struct ClientInfo g_clientInfo;


CLogicOperate::CLogicOperate()
{
	jsonOpt_ptr_ = NULL;
	sqlOpt_ptr_ = NULL;
	jsonOpt_ptr_ = new CJsonOpt();
	assert(jsonOpt_ptr_ != NULL);
	sqlOpt_ptr_  = new CSqlOpt;
	result_ = SUCCESS;
}

CLogicOperate::~CLogicOperate()
{
	utils::SafeDelete(jsonOpt_ptr_);
	utils::SafeDelete(sqlOpt_ptr_);
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

	result_ = SUCCESS;

	switch(message_type)
	{
	case GET_SOFT_VERSION:
		GetLastSoftVersion();
		break;
	default:
		LOG4CXX_WARN(g_logger, "CLogicOperate::StartLogicOpt:message_type invadlid.");
		break;
	}

	SendToDispatch();
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

void CLogicOperate::GetLastSoftVersion()
{
	std::string current_version;
	SoftVersionInfo last_versionInfo;
	std::vector<std::string> cur_verstr, verstr;

	if(!jsonOpt_ptr_->JsonParseGetLastSoftVersion(current_version))
	{
		result_ = JSON_PARSE_ERROR;
		goto GET_RESPONSE;
	}

	result_ = sqlOpt_ptr_->GetLastSoftVersionFromDB(last_versionInfo);
	if(MY_SQL_ERROR == result_)
	{
		goto GET_RESPONSE;
	}

	cur_verstr = utils::SplitString(current_version,".");
	verstr = utils::SplitString(last_versionInfo.soft_version,".");
	for(unsigned int i = 1; i < verstr.size(); ++i)
	{
		if(atoi(verstr.at(i).c_str()) > atoi(cur_verstr.at(i).c_str()))
		{
			result_ = NEED_UPDATE;
			break;
		}
	}

GET_RESPONSE:
	responseToDispatch_ = jsonOpt_ptr_->JsonJoinGetLastSoftVersion(GET_SOFT_VERSION_RESPONSE, result_, last_versionInfo);

}



