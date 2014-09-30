/*
 * net_core.cc
 *
 *  Created on: 2014-2-13
 *      Author: zhangs
 */

#include "net_core.h"
#include "work_thread.h"
#include "global_settings.h"
#include "local_transport.h"
#include "../public/utils.h"
#include "../public/socket_wrapper.h"


int CNetCore::id_ = 0;
CThreadSafeMap<int, int> CNetCore::map_csfd_id_;

CNetCore::CNetCore()
{
	work_thread_ptr_ = new CWorkThread;
}

CNetCore::~CNetCore()
{
	utils::SafeDelete(work_thread_ptr_);
}

bool CNetCore::CheckLibeventVersion()
{
	const char* libevent_version = event_get_version();
	assert(libevent_version != NULL);

	LOG4CXX_TRACE(g_logger, "The libevent version is " << libevent_version);

	if (strncmp(libevent_version, "2", 1) == 0)
		return true;
	else
		return false;
}

bool CNetCore::InitNetCore()
{
	if(!CheckLibeventVersion())
	{
		LOG4CXX_ERROR(g_logger, "CNetCore::InitNetCore The libevent version Require at  2.0.*");
		return false;
	}

	main_base_ = event_base_new();
	assert(main_base_ != NULL);

	/* 监听来自客户端的短连接 */
	if(!InitRemoteListenSocket(remote_listen_socket_))
		return false;

	evutil_make_socket_nonblocking(remote_listen_socket_);

	listen_event_ = event_new(main_base_, remote_listen_socket_, EV_READ|EV_PERSIST, DoAccept, (void*)this);
	assert(listen_event_ != NULL);

	if(event_add(listen_event_, NULL) == -1)
	{
		int error_code = EVUTIL_SOCKET_ERROR();
		LOG4CXX_ERROR(g_logger, "CNetCore::InitNetCore:event_add errorCode = " << error_code
				      	  	  	 << ", description = " << evutil_socket_error_to_string(error_code));
		return false;
	}

	/* 创建worker线程，用来处理来自客户端的连接 */
	if(!work_thread_ptr_->InitThread())
		return false;

	/* 建立 localTransport(TCP) 来和转发进程进行通信 */
	if(!CLocalTransport::GetInstance()->SetupChannelWithRelayServer())
	{
		return false;
	}
	return true;
}

void CNetCore::Run()
{

	LOG4CXX_INFO(g_logger, "CNetCore::Run:Master thread has start...");

	int ret = event_base_dispatch(main_base_);
	if (-1 == ret)
	{
		int error_code = EVUTIL_SOCKET_ERROR();
		LOG4CXX_FATAL(g_logger, "CNetCore::Run():event_base_dispatch errorCode = " << error_code
					 << ", description = " << evutil_socket_error_to_string(error_code));
		exit(1);
	}
	else if(1 == ret)
	{
		LOG4CXX_FATAL(g_logger, "CNetCore::Run():no events were registered.");
		exit(1);
	}
}

void CNetCore::DoAccept(evutil_socket_t listen_socket, short event, void* arg)
{
	evutil_socket_t sfd;
	struct sockaddr_in sin;
	socklen_t slen = sizeof(sin);

	sfd = accept(listen_socket, (struct sockaddr *) &sin, &slen);
	if (-1 == sfd)
	{
		LOG4CXX_WARN(g_logger, "CNetCore::DoAccept:accept error = " << strerror(errno));
		return;
	}

	// trance
	LOG4CXX_TRACE(g_logger, "CNetCore::DoAccept accept:" << listen_socket);
	//


	if (!SocketOperate::SetSocketNoBlock(sfd))
	{
		LOG4CXX_WARN(g_logger, "CNetCore::DoAccept:SetSocketNoBlock error = " << strerror(errno))
		close(sfd);
		return;
	}

	id_ = GetClientfdMapId();
	map_csfd_id_.insert(sfd, id_);

	/* dispatch new client-connection to worker-thread */
	CWorkThread::DispatchSfdToWorker(sfd, id_);

}

bool CNetCore::InitRemoteListenSocket(evutil_socket_t& listen_socket)
{
	listen_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (listen_socket < 0)
	{
		LOG4CXX_ERROR(g_logger, "CNetCore::InitRemoteListenSocket:socket error = " << strerror(errno));
		return false;
	}

	int flags = 1;
	if (setsockopt(listen_socket, SOL_SOCKET, SO_REUSEADDR, (void *) &flags, sizeof(flags)) != 0)
	{
		LOG4CXX_ERROR(g_logger, "CNetCore::InitRemoteListenSocket:setsockopt SO_REUSEADDR error = " << strerror(errno));
		close(listen_socket);
		return false;
	}

	if (setsockopt(listen_socket, SOL_SOCKET, SO_KEEPALIVE, (void *) &flags, sizeof(flags)) != 0)
	{
		LOG4CXX_ERROR(g_logger, "CNetCore::InitRemoteListenSocket:setsockopt SO_KEEPALIVE error = " << strerror(errno));
		close(listen_socket);
		return false;
	}

	struct linger ling = { 0, 0 };
	if (setsockopt(listen_socket, SOL_SOCKET, SO_LINGER, (void *) &ling, sizeof(ling)) != 0)
	{
		LOG4CXX_ERROR(g_logger, "CNetCore::InitRemoteListenSocket:setsockopt SO_LINGER error = " << strerror(errno));
		close(listen_socket);
		return false;
	}

	if (setsockopt(listen_socket, IPPROTO_TCP, TCP_NODELAY, (void *) &flags, sizeof(flags)) != 0)
	{
		LOG4CXX_ERROR(g_logger, "CNetCore::InitRemoteListenSocket:setsockopt TCP_NODELAY error = " << strerror(errno));
		close(listen_socket);
		return false;
	}

	sockaddr_in servaddr;
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(utils::G<CGlobalSettings>().remote_listen_port_ );

	if (bind(listen_socket, (const sockaddr*)&servaddr, sizeof(servaddr)) != 0)
	{
		LOG4CXX_ERROR(g_logger, "CNetCore::InitRemoteListenSocket:bind error = " << strerror(errno));
		close(listen_socket);
		return false;
	}

	if(listen(listen_socket, BACKLOG) != 0)
	{
	    LOG4CXX_ERROR(g_logger, "CNetCore::InitRemoteListenSocket:Listen error = " << strerror(errno));
	    close(listen_socket);
	    return false;
	}

	return true;
}

int  CNetCore::GetClientfdMapId()
{
	if(id_ > 65535)
	{
		id_ = 0;
	}
	else
	{
		id_ = id_ + 1;
	}

	return id_;
}

