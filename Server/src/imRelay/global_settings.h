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
	int local_listen_port_;

	std::string redis_ip_;
	int redis_port_;

};

#endif /* GLOBAL_SETTINGS_H_ */
