/*
 * cloudsee_transport.cpp
 *
 *  Created on: Mar 30, 2014
 *      Author: zhangs
 */

#include "cloudsee_transport.h"
#include "global_settings.h"
#include "cloudsee_message_opt.h"
#include "../public/utils.h"
#include "../public/socket_wrapper.h"

CCloudSeeTransport* CCloudSeeTransport::cloudsee_transport_ptr_ = NULL;
boost::mutex CCloudSeeTransport::instance_mutex_;
bool CCloudSeeTransport::message_head_flag_ = false;
uint32_t CCloudSeeTransport::message_len_ = 0;


CCloudSeeTransport::CCloudSeeTransport()
{
	connect_flag_ = false;
}

CCloudSeeTransport::~CCloudSeeTransport()
{
}

CCloudSeeTransport* CCloudSeeTransport::GetInstance()
{
	if (NULL == cloudsee_transport_ptr_)
	{
		boost::unique_lock<boost::mutex> lock(instance_mutex_);
		if (NULL == cloudsee_transport_ptr_)
		{
			cloudsee_transport_ptr_ = new CCloudSeeTransport;
		}
	}
	return cloudsee_transport_ptr_;
}

bool CCloudSeeTransport::SetupChannelWithCloudSeeServer()
{
	main_base_	= NULL;
	sfd_		= 0;
	connect_flag_ = false;

	CreateThread(SetupThread, (void*) this);

	return true;
}

void *CCloudSeeTransport::SetupThread(void *arg)
{
	CCloudSeeTransport* pThis = static_cast<CCloudSeeTransport*>(arg);

	/* 当和云视通进程的连接未正常建立时定时进行重试 */
	while (!pThis->connect_flag_)
	{
		boost::asio::deadline_timer t(pThis->io_, boost::posix_time::seconds(5));
		t.wait();
		LOCAL_REV_DATA* ptr_recv_data = new LOCAL_REV_DATA;
		bzero(ptr_recv_data->buf, DATA_BUFFER_SIZE);
		ptr_recv_data->base = NULL;
		ptr_recv_data->len = 0;
		ptr_recv_data->sfd = 0;

		if(!pThis->InitSocketFd(ptr_recv_data->sfd))
		{
			continue;
		}

		pThis->sfd_ = ptr_recv_data->sfd;

		if(!pThis->AddNewConnToEvent(ptr_recv_data))
		{
			continue;
		}

		pThis->connect_flag_ = true;

		LOG4CXX_INFO(g_logger, "SetupChannelWithCloudSeeServer done.");

		int ret = event_base_dispatch(ptr_recv_data->base);
		if (-1 == ret)
		{
			LOG4CXX_WARN(g_logger,
					"CCloudSeeTransport::Run():event_base_dispatch description = " << evutil_socket_error_to_string(EVUTIL_SOCKET_ERROR()));
		} else if (1 == ret)
		{
			LOG4CXX_WARN(g_logger,
					"CCloudSeeTransport::Run():no events were registered.");
		}

		pThis->connect_flag_ = false;

	}
	return NULL;
}

void CCloudSeeTransport::CreateThread(void *(*func)(void *), void *arg)
{
	pthread_t thread;
	pthread_attr_t attr;
	int ret;

	pthread_attr_init(&attr);

	if ((ret = pthread_create(&thread, &attr, func, arg)) != 0)
	{
		LOG4CXX_FATAL(g_logger, "CCloudSeeTransport::CreateThread:Can't create thread:" << strerror(ret));
		exit(1);
	}
}

bool CCloudSeeTransport::InitSocketFd(evutil_socket_t& sfd)
{
	sfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sfd < 0)
	{
		LOG4CXX_ERROR(g_logger, "CCloudSeeTransport::InitSocketFd:SocketFd error = " << strerror(errno));
		return false;
	}

	int flags = 1;
	if (setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, (void *) &flags, sizeof(flags)) != 0)
	{
		LOG4CXX_ERROR(g_logger, "CCloudSeeTransport::InitSocketFd:setsockopt SO_REUSEADDR error = " << strerror(errno));
		close(sfd);
		return false;
	}

	sockaddr_in servaddr;
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = inet_addr((utils::G<CGlobalSettings>().cloudsee_ip_).c_str());
	servaddr.sin_port = htons(utils::G<CGlobalSettings>().cloudsee_port_);

	if (connect_nonb(sfd, (struct sockaddr*) &servaddr, sizeof(servaddr), 5) < 0)
	{
		LOG4CXX_ERROR(g_logger, "CCloudSeeTransport::InitSocketFd:connect error = " << strerror(errno));
		close(sfd);
		return false;
	}

	return true;
}

bool CCloudSeeTransport::AddNewConnToEvent(LOCAL_REV_DATA* ptr_recv_data)
{
	ptr_recv_data->base = event_base_new();
	assert(ptr_recv_data->base  != NULL);
	int flag = EV_READ | EV_PERSIST;
	struct bufferevent *logic_pro_bev = bufferevent_socket_new(ptr_recv_data->base , ptr_recv_data->sfd, BEV_OPT_CLOSE_ON_FREE);
	if(logic_pro_bev == NULL)
	{
		return false;
	}
	bufferevent_setcb(logic_pro_bev, ReadCb, NULL, ErrorCb, (void*) ptr_recv_data);
	bufferevent_enable(logic_pro_bev, flag);
	return true;
}

int CCloudSeeTransport::connect_nonb( int sockfd, struct sockaddr* saptr, socklen_t salen, int nsec )
{
	int flags, n, error = 0;
	socklen_t len;
	fd_set rset, wset;
	struct timeval tval;

	flags = fcntl(sockfd, F_GETFL, 0);
	fcntl(sockfd, F_SETFL, flags | O_NONBLOCK);

	if ((n = ::connect(sockfd, saptr, salen) < 0))
	{
		if (EINPROGRESS != errno)
		{
			LOG4CXX_ERROR(g_logger, "CNetCoreLayer::connect_nonb:connect failed. errorcode = " << errno);
			return (-1);
		}
	}

	if(0 == n)
		goto done;

	FD_ZERO(&rset);
	FD_SET(sockfd, &rset);
	wset = rset;
	tval.tv_sec = nsec;
	tval.tv_usec = 0;

	if ((n = select(sockfd+1, &rset, &wset, NULL, nsec?&tval:NULL)) == 0)
	{
		close(sockfd);
		errno = ETIMEDOUT;
		LOG4CXX_ERROR(g_logger, "CNetCoreLayer::connect_nonb:select failed. errorcode = " << errno);
		return -1;
	}

	if (FD_ISSET(sockfd, &rset) || FD_ISSET(sockfd, &wset))
	{
		len = sizeof(error);
		if (getsockopt(sockfd, SOL_SOCKET, SO_ERROR, (char*)&error, &len) < 0)
		{
			LOG4CXX_ERROR(g_logger, "CNetCoreLayer::connect_nonb:getsockopt failed. errorcode = " << errno);
			return -1;
		}
	}
	else
		LOG4CXX_FATAL(g_logger, "CNetCoreLayer::connect_nonb:select error: sockfd not set");

done:
	if (error)
	{
		close(sockfd);
		errno = error;
		LOG4CXX_ERROR(g_logger, "CNetCoreLayer::connect_nonb:ioctlsocket1 failed. errorcode = " << errno);
		return -1;
	}

	return 0;
}


void CCloudSeeTransport::ReadCb(struct bufferevent *bev, void *arg)
{
	LOCAL_REV_DATA* ptr_data = static_cast<LOCAL_REV_DATA*> (arg);

	int sfd = bufferevent_getfd(bev);

	int recv_size = 0;
	if ((recv_size = bufferevent_read(bev, ptr_data->buf + ptr_data->len, DATA_BUFFER_SIZE - ptr_data->len)) > 0)
	{
		ptr_data->len = ptr_data->len + recv_size;
	}

	//trace log
	printf("CCloudSeeTransport::ReadCb data ptr_data->len:%d\n", ptr_data->len);
	for (unsigned i = 0; i < ptr_data->len; ++i)
	{
		printf("%0#x ", ptr_data->buf[i]);
	}
	printf("\n");
	//

	if (!message_head_flag_)
	{
		uint32_t recv_message_len = 0;
		uint8_t service_type = 0;
		memcpy(&recv_message_len, ptr_data->buf, 4);
		message_len_ = ntohl(recv_message_len);
		service_type = (ptr_data->buf)[4];
		if (service_type != ST_DEVICESERVER)
		{
			LOG4CXX_WARN(g_logger, "CCloudSeeTransport::ReadCb data error.");
			bzero(ptr_data->buf, DATA_BUFFER_SIZE);
			ptr_data->len = 0;
			message_head_flag_ = false;
		}
		else
		{
			message_head_flag_ = true;
		}
	}

	if (ptr_data->len >= message_len_)
	{
		if (message_len_ <= DATA_BUFFER_SIZE)
		{
			CCloudSeeMessageOpt cloudSeeOpt(sfd);
			cloudSeeOpt.StartCloudSeeMessageOpt(ptr_data->buf);
		}

		bzero(ptr_data->buf, DATA_BUFFER_SIZE);
		ptr_data->len = 0;

		message_head_flag_ = false;
	}
}

void CCloudSeeTransport::ErrorCb(struct bufferevent *bev, short event, void *arg)
{
	LOCAL_REV_DATA* ptr_data = static_cast<LOCAL_REV_DATA*> (arg);
	utils::SafeDelete(ptr_data);

	if (event & BEV_EVENT_EOF)
	{
		LOG4CXX_ERROR(g_logger, "connection closed." << evutil_socket_error_to_string(EVUTIL_SOCKET_ERROR()));
	}
	else if (event & BEV_EVENT_ERROR)
	{
		LOG4CXX_ERROR(g_logger, "some other error." << evutil_socket_error_to_string(EVUTIL_SOCKET_ERROR()));
	}
	bufferevent_free(bev);
}

bool CCloudSeeTransport::SendToCloudSeeServer(const char* message, const uint32_t request_len)
{
	boost::mutex::scoped_lock Lock(mutex_);
	
	return SocketOperate::WriteSfd(sfd_, message, request_len);
}
