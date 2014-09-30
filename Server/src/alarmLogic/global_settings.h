/*
 * global_settings.h
 *
 *  Created on: Mar 12, 2013
 *      Author: yaowei
 */

#ifndef GLOBAL_SETTINGS_H_
#define GLOBAL_SETTINGS_H_
#include <string>

class CGlobalSettings
{
public:
	std::string dispatch_ip_;
	int dispatch_port_;
	int stats_listen_port_;
	int bind_port_;
	int thread_num_;
	int queue_num_;
	std::string redis_ip_;
	int redis_port_;

	std::string alarm_server_ip_;
	int alarm_server_port_;
};

#endif /* GLOBAL_SETTINGS_H_ */
