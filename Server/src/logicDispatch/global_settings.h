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
	int remote_listen_port_;
	int local_listen_port_;
	unsigned int thread_num_;
	int client_timeout_;

	int accountLogic_port_;
	int deviceInfo_port_;
	int alarmLogic_port;
	int deviceInfo_HomeCloud_port_;
};

#endif /* GLOBAL_SETTINGS_H_ */
