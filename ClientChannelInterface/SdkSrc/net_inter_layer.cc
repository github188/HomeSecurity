#include "net_inter_layer.h"
#include "defines.h"
#include "message.h"
#include "net_data_layer.h"
#include "net_core_layer.h"
#include "utils.h"

CNetInterLayer::CNetInterLayer(void)
{
	message_id_ = 0;
	request_reponse_timeout_ = 0;
	pUserInterfaceImpl_ = NULL;
	pNetCore_ = new CNetCoreLayer;
	pNetDataOpt_ = new CNetDataLayer;
}

CNetInterLayer::~CNetInterLayer(void)
{
	utils::SafeDelete(pNetCore_);
}

bool CNetInterLayer::Init(CClientNetInterfaceImpl* pUserInterfaceImpl, const std::string& ip, const int port)
{
	pUserInterfaceImpl_ = pUserInterfaceImpl;

	/** 初始化网络库，传入“接收数据回调函数”以及“业务层当前指针”，用于在网络层接到数据后传给业务层 */
	if(!pNetCore_->InitNetCore(this, ip, port))
	{
		LOG4CXX_ERROR(g_logger_sdk, " CNetInterLayer::Init:InitNetCore failed.");
		return false;
	}
	LOG4CXX_INFO(g_logger_sdk, " CNetInterLayer::Init:InitNetCore success.");

	request_reponse_timeout_ = 10000;
	LOG4CXX_TRACE(g_logger_sdk, " CNetInterLayer::Init:request_reponse_timeout.s = " << request_reponse_timeout_/1000);

	/** 开启libevent处理线程 */
	pthread_t threadId;
	int hThrd = pthread_create(&threadId, NULL, ThreadFunc, this);
	if (0 != hThrd) {
		LOG4CXX_ERROR(g_logger_sdk, " CNetInterLayer::Init:ThreadFunc failed.");
		return false;
	}

	/** 开启推送消息回调函数处理线程 */
	hThrd = pthread_create(&threadId, NULL, ThreadPushFunc, this);
	if (0 != hThrd)
	{
		LOG4CXX_ERROR(g_logger_sdk, " CNetInterLayer::Init:ThreadPushFunc failed.");
		return false;
	}

	LOG4CXX_INFO(g_logger_sdk, " CNetInterLayer::Init:Thread:netCore:Run() success.");

	return true;
}

void* CNetInterLayer::ThreadFunc(void* param)
{
	CNetInterLayer* pThis = static_cast<CNetInterLayer*>(param);
	pThis->pNetCore_->Run();
	return NULL;
}


int CNetInterLayer::GetMessageId()
{
	boost::mutex::scoped_lock oLock(mutex_);
	if (message_id_ >= 65535)
	{
		message_id_ = 0;
	}

	return message_id_ = message_id_ + 1;
}

void* CNetInterLayer::ThreadPushFunc(void* param)
{
	CNetInterLayer* pThis = static_cast<CNetInterLayer*>(param);
	pThis->CallServerPushMessageOpt();
	return NULL;
}

void  CNetInterLayer::CallServerPushMessageOpt()
{
	boost::mutex::scoped_lock oLock(mutex_push_);
	std::string push_message;
	while(b_push_thread_run_)
	{
		cond_push_.wait(mutex_push_);
		while(!list_push_message_.empty())
		{
			list_push_message_.pop_front(push_message);
			pUserInterfaceImpl_->ServerPushMessageOpt(push_message);
		}
	}

	return;
}

void CNetInterLayer::ReciveData(const std::string& response, const int connection_type)
{

	/* 恢复转义处理*/
	std::string recover_string = utils::ReplaceString(response, "\\\\r\\\\n", "\\r\\n");

	int message_id = 0;
	/** 判断是否为推送消息,没有异步过程标识(message_id)的为推送消息 */
	if (PERSIST_CONNECTION == connection_type)
	{
		if (!pNetDataOpt_->JsonParseMessageId(recover_string, message_id))
		{
			list_push_message_.push_back(recover_string);
			cond_push_.notify_one();
			return;
		}
	}

	/* 解析消息ID */
	if (!pNetDataOpt_->JsonParseMessageId(recover_string, message_id))
	{
		LOG4CXX_WARN(g_logger_sdk, "CNetInterLayer::ReciveData invalid, message = " << recover_string);
		return;
	}

	/* 根据传回的请求消息id找到相应的请求阻塞事件，激活事件读取response */
	sem_t* h_event = FindEventByMessageIdAndSetResponse(message_id, recover_string);
	if (h_event)
	{
		sem_post(h_event);
		//LOG4CXX_INFO(g_logger_sdk, "CNetInterLayer::ReciveData: after call sem_post = " << strerror(errno));
	}
	else
	{
		LOG4CXX_WARN(g_logger_sdk, "CNetInterLayer::ReciveData other message or timeout message. message_id = "  
					 << message_id << ", response = " << recover_string << ", connection_type = " << connection_type);
	}

}

sem_t* CNetInterLayer::FindEventByMessageIdAndSetResponse( const int message_id, const std::string& response)
{
	NET_MSG newNetMsg;
	newNetMsg.h_event = NULL;
	newNetMsg.response = response;
	NET_MSG oldNetMsg;
	if (map_message_.findAndSet(message_id, newNetMsg, oldNetMsg))
		return oldNetMsg.h_event;
	else
		return NULL;
}

std::string CNetInterLayer::FindResponseByMessageId(const int message_id)
{
	NET_MSG netMsg;
	map_message_.find(message_id, netMsg);
	return netMsg.response;
}


void CNetInterLayer::ClearMapByMessageId( const int message_id )
{
		map_message_.erase(message_id);
}

void CNetInterLayer::maketimeout(struct timespec *tsp, long milliseconds)
{
	struct timeval now;

	/* get current times */
	gettimeofday(&now, NULL);
	tsp->tv_sec = now.tv_sec;
	tsp->tv_nsec = now.tv_usec * 1000;
	tsp->tv_sec += (milliseconds/1000);
	tsp->tv_nsec += (milliseconds%1000) * 1000000;
}
int CNetInterLayer::GetResponseByRequest(const int message_id, const int tcp_connect_flag, const std::string& resquest, std::string& response )
{
	/** 阻塞在服务端回复处，创建一个请求id以便对应匹配的回复数据 */
	int ret = SUCCESS;

	sem_t cond;
	ret = sem_init(&cond, 0, 0);
	if(0 != ret)
	{
		LOG4CXX_ERROR(g_logger_sdk, "CNetInterLayer::GetResponseByRequest:CreateEvent failed. errorcode = " << ret);
		return ret;
	}

	NET_MSG net_msg;
	net_msg.h_event = &cond;
	net_msg.response = "";

	map_message_.insert(message_id, net_msg);

	if (SHORT_CONNECTION == tcp_connect_flag)
	{
		ret = pNetCore_->AddShortConnectionResquest(resquest);
		if (SUCCESS != ret)
		{
			LOG4CXX_ERROR(g_logger_sdk, "CNetInterLayer::GetResponseByRequest:AddShortConnectionResquest failed. message_id = " << message_id);
			ClearMapByMessageId(message_id);
			return ret;
		}
	}

	if (PERSIST_CONNECTION == tcp_connect_flag)
	{
		ret = pNetCore_->AddPersistConnectionRequest(resquest);
		if (SUCCESS != ret)
		{
			LOG4CXX_ERROR(g_logger_sdk, "CNetInterLayer::GetResponseByRequest:AddPersistConnectionRequest failed. message_id = " << message_id);
			ClearMapByMessageId(message_id);
			return ret;
		}
	}

	struct timespec timestruct = {0, 0};
	maketimeout(&timestruct, request_reponse_timeout_);
	int dw = sem_timedwait(&cond, &timestruct);
	if(dw != 0) dw = errno;
	sem_destroy(&cond);
	switch(dw)
	{
	case 0:
		/** 回应 */
		response = FindResponseByMessageId(message_id);
		if (response.empty())
		{
			LOG4CXX_WARN(g_logger_sdk,"CNetInterLayer::GetResponseByRequest:FindResponseByMessageId empty. message_id = " << message_id);
		}
		break;
	case ETIMEDOUT:
		LOG4CXX_WARN(g_logger_sdk, "CNetInterLayer::GetResponseByRequest TIMEOUT."<< ", message_id = " << message_id);
		ret  = REQ_RES_TIMEOUT;
		break;
	default:
		LOG4CXX_ERROR(g_logger_sdk, "CNetInterLayer::GetResponseByRequest error. errorcode = " << strerror(errno) << ", message_id = " << message_id);
		ret  = REQ_RES_OTHER_ERROR;
		break;
	}
	ClearMapByMessageId(message_id);
	return ret;
}

int CNetInterLayer::ClosePersistConnection()
{
	return pNetCore_->ClosePersistConnection();
}

