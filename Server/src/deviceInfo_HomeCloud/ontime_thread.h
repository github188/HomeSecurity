#ifndef ONTIME_THREAD_H_
#define ONTIME_THREAD_H_

#include "thread.h"
#include "logic_operate.h"

class CSqlOpt;
class CRedisOpt;
class COntimeThread : public CThread
{
public:
	COntimeThread();
	virtual ~COntimeThread();

public:
	virtual void Run();

private:
	int HumitureStatisticsPHour(const std::string& date, int hour);
	int GetOnlineDevicesGuid(std::vector<std::string>& vec_online_guid);
	int GetDeviceOnlineStatus(const std::string& redisKey);
	std::string GetDeviceGuidByRedisKey(const std::string& redisKey);
	std::string GetDeviceTimestampByRedisKey(const std::string& redisKey);
	int GetDeviceEnvByGuid(const std::string& device_guid, std::vector<DEVICE_ENV_INFO>& vec_device_env_info);
	int GetAvgHumiture(const std::string& device_guid, const std::string& date, const int hour, const std::vector<DEVICE_ENV_INFO>& vec_device_env_info, double& avgTem, double& avgHum);
	int GetHumitureScore(const double temperature, const double humidity);
	int GetHumitureAssessment(const double temperature, const double humidity);

private:
	CSqlOpt		*sqlOpt_ptr_;
	CRedisOpt	*redisOpt_ptr_;
};

#endif
