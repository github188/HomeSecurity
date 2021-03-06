/*
 * init_configure.h
 *
 *  Created on: 2014-2-13
 *      Author: zhangs
 */

#ifndef INIT_LOG4CXX_H_
#define INIT_LOG4CXX_H_

#include <string>

class CInitConfig {
public:
	CInitConfig();
	virtual ~CInitConfig();

public:

	void InitLog4cxx(const std::string& project_name);

	bool LoadConfiguration(const std::string& project_name);

	void SetConfigFilePath(const std::string& config_file_path) {config_file_path_ = config_file_path;};

private:

	std::string config_file_path_;
};

#endif /* INIT_LOG4CXX_H_ */
