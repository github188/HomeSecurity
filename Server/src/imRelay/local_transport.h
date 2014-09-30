/*
 * local_transport.h
 *
 *  Created on: Mar 7, 2013
 *      Author: yaowei
 */

#ifndef LOCAL_TRANSPORT_H_
#define LOCAL_TRANSPORT_H_

#include "defines.h"

class CLocalTransport
{

public:

	void SetupLocalTransport();

	static std::map<int, int>  map_no_sfd_;

private:

	bool CheckLibeventVersion();

	bool InitLocalListenSocket(evutil_socket_t& listen_socket);

	static void AcceptCb(evutil_socket_t listen_socket, short event, void* arg);

	static void ReadCb(struct bufferevent *bev, void *arg);

	static void ErrorCb(struct bufferevent *bev, short event, void *arg);

private:

	evutil_socket_t sfd_;

	struct event_base 	*main_base_;
	struct event		*local_listen_event_;
	evutil_socket_t 	local_listen_socket_;

};

#endif /* LOCAL_TRANSPORT_H_ */
