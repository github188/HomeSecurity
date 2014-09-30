/*
 * net_core.h
 *
 *  Created on: 2014-2-13
 *      Author: zhangs
 */

#ifndef NET_CORE_H_
#define NET_CORE_H_

#include "defines.h"
#include "threadSafe_map.h"

class CWorkThread;

class CNetCore
{
public:
	CNetCore();
	virtual ~CNetCore();

public:

	bool InitNetCore();

	void Run();

	static CThreadSafeMap<int, int> map_csfd_id_;          //客户端连接sfd-->随机唯一id，和业务进程传来的sfd和id对应，防止sfd被其他客户端重用导致错发

private:

	bool CheckLibeventVersion();

	bool InitRemoteListenSocket(evutil_socket_t& listen_socket);

	static void DoAccept(evutil_socket_t listen_socket, short event, void* arg);

	static int  GetClientfdMapId();

private:

	struct event_base 	*main_base_;
	evutil_socket_t 	remote_listen_socket_;
	struct event		*listen_event_;

	static int id_;

	CWorkThread			*work_thread_ptr_;
};


#endif /* NET_CORE_H_ */
