/*
 * cloudsee_transport.h
 *
 *  Created on: Mar 30, 2014
 *      Author: zhangs
 */

#ifndef CLOUDSEE_TRANSPORT_H_
#define CLOUDSEE_TRANSPORT_H_

#include "defines.h"
#include <boost/asio.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

class CCloudSeeTransport
{
public:

	virtual ~CCloudSeeTransport();

public:

	static CCloudSeeTransport* GetInstance();

	bool SetupChannelWithCloudSeeServer();

	bool SendToCloudSeeServer(const char* message, const uint32_t request_len);

private:

	CCloudSeeTransport();

	static void *SetupThread(void *arg);

	void CreateThread(void *(*func)(void *), void *arg);

	bool InitSocketFd(evutil_socket_t& sfd);
	int connect_nonb( int sockfd, struct sockaddr* saptr, socklen_t salen, int nsec );

	bool AddNewConnToEvent(LOCAL_REV_DATA* ptr_recv_data);

	static void ReadCb(struct bufferevent *bev, void *arg);

	static void ErrorCb(struct bufferevent *bev, short event, void *arg);

private:

	struct event_base 	*main_base_;
	evutil_socket_t 	sfd_;
	char   recv_buf_[DATA_BUFFER_SIZE];
	boost::mutex     	mutex_;

	boost::asio::io_service io_;
	static CCloudSeeTransport* cloudsee_transport_ptr_;
	static boost::mutex     instance_mutex_;

	bool	connect_flag_;
	static bool	message_head_flag_;
	static uint32_t message_len_;
};


#endif /* CLOUDSEE_TRANSPORT_H_ */
