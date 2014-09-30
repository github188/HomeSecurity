/*
 * local_transport.cc
 *
 *  Created on: Mar 7, 2013
 *      Author: yaowei
 */

#include "local_transport.h"
#include "global_settings.h"
#include "logic_opt.h"
#include "redis_conn_pool.h"
#include "../public/utils.h"
#include "../public/message.h"
#include "../public/socket_wrapper.h"

std::map<int, int>  CLocalTransport::map_no_sfd_;

void CLocalTransport::SetupLocalTransport()
{
	if(!CheckLibeventVersion())
	{
		LOG4CXX_FATAL(g_logger, "The libevent version Require at  2.0.*");
		exit(1);
	}

	main_base_ = event_base_new();
	assert(main_base_ != NULL);

	/* 监听来自imServer进程的长连接 */
	if (!InitLocalListenSocket(local_listen_socket_))
		exit(1);

	evutil_make_socket_nonblocking(local_listen_socket_);

	local_listen_event_ = event_new(main_base_, local_listen_socket_, EV_READ | EV_PERSIST, AcceptCb, (void*) this);
	assert(local_listen_event_ != NULL);

	if (event_add(local_listen_event_, NULL) == -1)
	{
		int error_code = EVUTIL_SOCKET_ERROR();
		LOG4CXX_FATAL(g_logger, "CLocalTransport::SetupLocalTransport:event_add errorCode = " << error_code
					           << ", description = " << evutil_socket_error_to_string(error_code));
		exit(1);
	}

	LOG4CXX_INFO(g_logger, "CLocalTransport::ReadLibevent:has start." );

	event_base_dispatch(main_base_);
}


void CLocalTransport::AcceptCb(evutil_socket_t listen_socket, short event, void* arg)
{
	CLocalTransport *pThis = static_cast<CLocalTransport*>(arg);

	evutil_socket_t sfd;
	struct sockaddr_in sin;
	socklen_t slen = sizeof(sin);

	sfd = accept(listen_socket, (struct sockaddr *) &sin, &slen);
	if (-1 == sfd)
	{
		LOG4CXX_WARN(g_logger, "CLocalTransport::AcceptCb:accept error = " << strerror(errno));
		return;
	}

	/* 将IMServer的服务器标识号（绑定端口）与连接sfd做关联*/
	int im_server_no = ntohs(sin.sin_port);
	map_no_sfd_.insert(std::pair<int, int>(im_server_no, sfd));
	LOG4CXX_INFO(g_logger, "CLocalTransport::AcceptCb:has a ImServer connected. server_no = " << im_server_no << ", sfd = " << sfd);

	if (!SocketOperate::SetSocketNoBlock(sfd))
	{
		LOG4CXX_WARN(g_logger, "CLocalTransport::AcceptCb:SetSocketNoBlock error = " << strerror(errno))
		close(sfd);
		return;
	}

	LOCAL_REV_DATA* ptr_recv_data = new LOCAL_REV_DATA;
	bzero(ptr_recv_data->buf, DATA_BUFFER_SIZE);
	ptr_recv_data->len = 0;
	ptr_recv_data->sfd = sfd;

	struct bufferevent *logic_pro_bev = bufferevent_socket_new(pThis->main_base_, sfd, BEV_OPT_CLOSE_ON_FREE);
	bufferevent_setcb(logic_pro_bev, ReadCb, NULL, ErrorCb, (void*)ptr_recv_data);
	bufferevent_enable(logic_pro_bev, EV_READ|EV_PERSIST);
}

void CLocalTransport::ReadCb(struct bufferevent *bev, void *arg)
{
	LOCAL_REV_DATA* ptr_data = static_cast<LOCAL_REV_DATA*>(arg);

	int sfd = bufferevent_getfd(bev);

	int recv_size = 0;
	if ((recv_size = bufferevent_read(bev, ptr_data->buf + ptr_data->len, DATA_BUFFER_SIZE - ptr_data->len)) > 0)
	{
		ptr_data->len = ptr_data->len + recv_size;
	}

	std::string str_recv(ptr_data->buf, ptr_data->len);
	if (utils::FindCRLF(str_recv))
	{
		/* 有可能同时收到多条信息 */
		std::vector<std::string> vec_str;
		utils::SplitData(str_recv, CRLF, vec_str);

		/* 对需转发消息的处理 */
		for (unsigned int i = 0; i < vec_str.size(); ++i)
		{
			CLogicOpt logicOpt;
			logicOpt.StartMessageOpt(vec_str.at(i), sfd);
		}

		int len = str_recv.find_last_of(CRLF) + 1;
		memmove(ptr_data->buf, ptr_data->buf + len, DATA_BUFFER_SIZE - len);
		ptr_data->len = ptr_data->len - len;
	}
}

void CLocalTransport::ErrorCb(struct bufferevent *bev, short event, void *arg)
{
	LOCAL_REV_DATA* ptr_data = static_cast<LOCAL_REV_DATA*> (arg);

	evutil_socket_t sfd = bufferevent_getfd(bev);

	if (event & BEV_EVENT_TIMEOUT)
	{
		LOG4CXX_WARN(g_logger, "CLocalTransport::DoLogicProcessError:TimeOut.");
	}
	else if (event & BEV_EVENT_EOF)
	{
		LOG4CXX_ERROR(g_logger, "CLocalTransport::DoClientTcpError:connection closed. sfd = " << sfd);
	}
	else if (event & BEV_EVENT_ERROR)
	{
		int error_code = EVUTIL_SOCKET_ERROR();
		LOG4CXX_ERROR(g_logger, "CLocalTransport::DoClientTcpError:some other errorCode = " << error_code
								 << ", description = " << evutil_socket_error_to_string(error_code));
	}

	std::map<int, int>::iterator iter;
	for(iter = map_no_sfd_.begin(); iter != map_no_sfd_.end(); ++iter)
	{
		if(sfd == iter->second)
		{
			map_no_sfd_.erase(iter);
		}
	}

	utils::SafeDelete(ptr_data);
	bufferevent_free(bev);
}

bool CLocalTransport::InitLocalListenSocket(evutil_socket_t& listen_socket)
{

	listen_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (listen_socket < 0)
	{
		LOG4CXX_ERROR(g_logger, "CLocalTransport::InitLocalListenSocket:SocketFd error = " << strerror(errno));
		return false;
	}

	int flags = 1;
	if (setsockopt(listen_socket, SOL_SOCKET, SO_REUSEADDR, (void *) &flags, sizeof(flags)) != 0)
	{
		LOG4CXX_ERROR(g_logger, "CLocalTransport::InitLocalListenSocket:setsockopt SO_REUSEADDR error = " << strerror(errno));
		close(listen_socket);
		return false;
	}

	sockaddr_in servaddr;
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(utils::G<CGlobalSettings>().local_listen_port_);

	if (bind(listen_socket, (const sockaddr*) &servaddr, sizeof(servaddr)) != 0)
	{
		LOG4CXX_ERROR(g_logger, "CLocalTransport::InitLocalListenSocket:bind error = " << strerror(errno));
		close(listen_socket);
		return false;
	}

	if (listen(listen_socket, BACKLOG) != 0)
	{
		LOG4CXX_ERROR(g_logger, "CLocalTransport::InitLocalListenSocket:Listen error = " << strerror(errno));
		close(listen_socket);
		return false;
	}

	return true;
}

bool CLocalTransport::CheckLibeventVersion()
{
	const char* libevent_version = event_get_version();
	assert(libevent_version != NULL);

	LOG4CXX_TRACE(g_logger, "The libevent version is " << libevent_version);

	if (strncmp(libevent_version, "2", 1) == 0)
		return true;
	else
		return false;
}


