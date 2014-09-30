#ifndef SQL_OPT_H_
#define SQL_OPT_H_

#include "../public/user_interface_defines.h"

class CSqlOpt {
public:
	CSqlOpt();
	virtual ~CSqlOpt();

public:
	int GetDevicePasswordFromDB(const std::string& device_guid, std::string& device_username, std::string& device_password);
	int GetDeviceUsersFromDB(const std::string& device_guid, std::vector<std::string> &vec_un, std::string &device_name, std::string &alarm_time, int &full_alarm_mode, const std::string& device_username, const std::string& device_password);
	int GetDeviceResetFlagFromDB(const std::string& device_guid, int& reset_flag);
	int GetDeviceSubTypeFromDB(const std::string& device_guid, int& device_sub_type);

private:

};

#endif /* SQL_OPT_H_ */
