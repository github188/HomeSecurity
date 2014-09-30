/*
 * local_transport.h
 *
 *  Created on: Mar 7, 2013
 *      Author: yaowei
 */

#ifndef LOCAL_TRANSPORT_H_
#define LOCAL_TRANSPORT_H_

#include "defines.h"
#include <boost/asio.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

class CLocalTransport
{
public:

	virtual ~CLocalTransport();

public:

	static CLocalTransport* GetInstance();

	bool SetupChannelWithRelayServer();

	bool SendToRelay(std::string& message);

private:

	CLocalTransport();

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
	static CLocalTransport* local_transport_ptr_;
	static boost::mutex     instance_mutex_;

	bool	local_connect_flag_;
};


#endif /* LOCAL_TRANSPORT_H_ */
