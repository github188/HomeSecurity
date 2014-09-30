#include "ontime_thread.h"
#include "../public/utils.h"
#include "defines.h"
#include "sql_opt.h"
#include "redis_opt.h"
#include "redis_conn_pool.h"
#include "../public/redis_key.h"
#include "../public/user_interface_defines.h"
#include <time.h>
#include <math.h>

COntimeThread::COntimeThread()
{
	sqlOpt_ptr_	 = NULL;
	redisOpt_ptr_= NULL;
	sqlOpt_ptr_  = new CSqlOpt;
	assert(sqlOpt_ptr_ != NULL);
	redisOpt_ptr_ = new CRedisOpt;
	assert(redisOpt_ptr_ != NULL);
}

COntimeThread::~COntimeThread()
{
	utils::SafeDelete(sqlOpt_ptr_);
	utils::SafeDelete(redisOpt_ptr_);
}

void COntimeThread::Run()
{
	LOG4CXX_INFO(g_logger, "COntimeThread::Run...");

	redisContext* con = CRedisConnPool::GetInstance()->GetRedisContext();
	redisOpt_ptr_->SetRedisContext(con);

	while (true)
	{
		time_t tnow = time(NULL);
		struct tm *tm = localtime(&tnow);
		int hour = tm->tm_hour;
		int min = tm->tm_min;
		int sec = tm->tm_sec;
		char date[16] = {0};
		strftime(date, sizeof (date), "%Y-%m-%d", tm);

		if (sec == 0 && min == 0)
		{
			//
			LOG4CXX_TRACE(g_logger, "COntimeThread::Run: I'm start...");
			LOG4CXX_TRACE(g_logger, "********** date:" << date << " hour:" << hour);
			//
			if (hour == 0)
			{
				hour = 24;
			}
			if (HumitureStatisticsPHour(date, hour - 1) != SUCCESS)
			{
				LOG4CXX_ERROR(g_logger, "COntimeThread::Run:HumitureStatisticsPHour error.");
			}
		}

		sleep(1);
	}
}

int COntimeThread::HumitureStatisticsPHour(const std::string& date, int hour)
{
	int ret = SUCCESS;

	std::vector<std::string> vec_online_guid;
	if ((ret = GetOnlineDevicesGuid(vec_online_guid)) != SUCCESS)
	{
		LOG4CXX_ERROR(g_logger, "COntimeThread::HumitureStatisticsPHour:GetOnlineDevicesGuid error.");
		return ret;
	}

	std::vector<DEVICE_ENV_INFO> vec_device_env_info;
	std::vector<HUMITURE_STAT_INFO> vec_humiture_stat_info;
	HUMITURE_STAT_INFO humiture_stat_info;
	for (unsigned i = 0; i < vec_online_guid.size(); ++i)
	{
		GetDeviceEnvByGuid(vec_online_guid.at(i), vec_device_env_info);
		//统计函数
		double avgTem = 0;
		double avgHum = 0;
		if (GetAvgHumiture(vec_online_guid.at(i), date, hour, vec_device_env_info, avgTem, avgHum) == DEVICE_HUMITURE_NOT_EXIST)
		{
			// 没有数据就跳过
			continue;
		}
		humiture_stat_info.device_guid = vec_online_guid.at(i);
		humiture_stat_info.date = date;
		humiture_stat_info.hour = hour;
		humiture_stat_info.statTemperature = avgTem;
		humiture_stat_info.statHumidity = avgHum;
		humiture_stat_info.score = GetHumitureScore(avgTem, avgHum);
		humiture_stat_info.assessment = GetHumitureAssessment(avgTem, avgHum);

		vec_humiture_stat_info.push_back(humiture_stat_info);
	}

	//入库
	if ((ret = sqlOpt_ptr_->DeviceHumitureStatToDB(vec_humiture_stat_info)) != SUCCESS)
	{
		LOG4CXX_ERROR(g_logger, "COntimeThread::HumitureStatisticsPHour:DeviceHumitureStatToDB error.");
		return ret;
	}

	return ret;
}

int COntimeThread::GetOnlineDevicesGuid(std::vector<std::string>& vec_online_guid)
{
	redisOpt_ptr_->SelectDB(DEVICE);

	int ret = SUCCESS;

	std::vector<std::string> vec_keys;
	std::string keys_pattern = std::string("*") + ONLINE_FLAG;
	if (!redisOpt_ptr_->Keys(keys_pattern, vec_keys))
	{
		LOG4CXX_ERROR(g_logger, "COntimeThread::GetOnlineDevicesGuid:Keys error.");
		return REDIS_OPT_ERROR;
	}

	std::string online_guid;
	for (unsigned i = 0; i < vec_keys.size(); ++i)
	{
		if (GetDeviceOnlineStatus(vec_keys.at(i)) == ONLINE)
		{
			online_guid = GetDeviceGuidByRedisKey(vec_keys.at(i));
			if (!online_guid.empty())
			{
				vec_online_guid.push_back(online_guid);
			}
		}
	}

	return ret;
}

int COntimeThread::GetDeviceOnlineStatus(const std::string& redisKey)
{
	redisOpt_ptr_->SelectDB(DEVICE);

	int online_flag = OFFLINE;
	std::string str_online_flag;

	if (!redisOpt_ptr_->Get(redisKey, str_online_flag))
	{
		LOG4CXX_ERROR(g_logger, "COntimeThread::GetDeviceOnlineStatus:Get error.");
		return REDIS_OPT_ERROR;
	}
	online_flag = atoi(str_online_flag.c_str());

	return online_flag;
}

std::string COntimeThread::GetDeviceGuidByRedisKey(const std::string& redisKey)
{
	unsigned i = redisKey.find_first_of(':',0);
	if (i != std::string::npos)
		return redisKey.substr(0, i);
	else
		return "";
}
std::string COntimeThread::GetDeviceTimestampByRedisKey(const std::string& redisKey)
{
	unsigned i = redisKey.find_first_of('&',0);
	if (i != std::string::npos)
		return redisKey.substr(i + 1);
	else
		return "";
}

int COntimeThread::GetDeviceEnvByGuid(const std::string& device_guid, std::vector<DEVICE_ENV_INFO>& vec_device_env_info)
{
	redisOpt_ptr_->SelectDB(DEVICE);

	int ret = SUCCESS;

	std::vector<std::string> vec_keys;
	std::string keys_pattern = device_guid + std::string("&*");
	if (!redisOpt_ptr_->Keys(keys_pattern, vec_keys))
	{
		LOG4CXX_ERROR(g_logger, "COntimeThread::GetDeviceEnvByGuid:Keys error.");
		return REDIS_OPT_ERROR;
	}

	DEVICE_ENV_INFO device_env_info;
	std::string strTem;
	std::string strHum;
	std::string strTimestamp;
	for (unsigned i = 0; i < vec_keys.size(); ++i)
	{
		if (redisOpt_ptr_->Hget(vec_keys.at(i), REDIS_HASH_FEILD_TEMPERATURE, strTem) && 
			redisOpt_ptr_->Hget(vec_keys.at(i), REDIS_HASH_FEILD_HUMIDNESS, strHum))
		{
			if (strTem.empty() || strHum.empty())
			{
				continue;
			}
			device_env_info.temperature = atof(strTem.c_str());
			device_env_info.humidity = atof(strHum.c_str());
			device_env_info.device_guid = device_guid;
			strTimestamp = GetDeviceTimestampByRedisKey(vec_keys.at(i));
			strptime(strTimestamp.c_str(), "%Y-%m-%d|%H:%M:%S", &(device_env_info.timestamp));

			vec_device_env_info.push_back(device_env_info);
		}
		else
		{
			continue;
		}
	}

	return ret;
}

int COntimeThread::GetAvgHumiture(const std::string& device_guid, const std::string& date, const int hour, const std::vector<DEVICE_ENV_INFO>& vec_device_env_info, double& avgTem, double& avgHum)
{
	int ret = SUCCESS;

	int count = 0;
	double sumTem = 0;
	double sumHum = 0;
	char strDate[16];
	for (unsigned i = 0; i < vec_device_env_info.size(); ++i)
	{
		if (vec_device_env_info.at(i).device_guid != device_guid)
		{
			continue;
		}

		strftime(strDate, sizeof (strDate), "%Y-%m-%d", &(vec_device_env_info.at(i).timestamp));
		if (std::string(strDate) == date && vec_device_env_info.at(i).timestamp.tm_hour == hour)
		{
			sumTem += vec_device_env_info.at(i).temperature;
			sumHum += vec_device_env_info.at(i).humidity;
			++count;
		}
	}

	if (count == 0)
	{
		return DEVICE_HUMITURE_NOT_EXIST;
	}
	
	avgTem = sumTem / count;
	avgHum = sumHum / count; 

	return ret;
}

int COntimeThread::GetHumitureScore(const double temperature, const double humidity)
{
	double res = 0;
	double len_of_point2best_point = sqrt(pow(temperature-25,2)+pow(humidity-50,2));

	if( (humidity < 227.85 - 6.57 * temperature) && (humidity < 1019 - 37 * temperature) && (humidity > 1317.5 -55 * temperature) && (humidity > 58.57 - 1.43 * temperature))
	{
		res = 100 - (len_of_point2best_point/30)*10;
	}
	else
	{
		double temScore = 0;
		double humScore = 0;

		if(temperature < 15) 
			temScore = 40 - (15 - temperature) / 2 * 5;
		else if(temperature < 18) 
			temScore = 42.5 - (18 - temperature) / 3 * 2.5;
		else if(temperature < 24) 
			temScore = 45 - (24 - temperature) / 6 * 2.5;
		else if(temperature < 27) 
			temScore = 45 - (temperature - 24) / 3 * 2.5;
		else
			temScore = 42.5 - (temperature - 27) /8 * 42.5;

		if(humidity < 40) 
			humScore = humidity;
		else if(humidity < 45) 
			humScore = 42.5 - (45 - humidity) / 5 * 2.5;
		else if(humidity < 55) 
			humScore = 45 - (55 - humidity) / 10 * 2.5;
		else if(humidity < 65) 
			humScore = 45 - (humidity - 55) / 10 * 2.5;
		else if(humidity < 70) 
			humScore = 42.5 - (humidity - 65) / 5 * 2.5;
		else
			humScore = 40 - (humidity - 70) / 30 * 40;

		res = temScore + humScore;
	}

	return (int)res;
}

int COntimeThread::GetHumitureAssessment(const double temperature, const double humidity)
{
	int res = 0;
	if(temperature < 22)
		res |= TEMPERATURE_TOO_LOW;
	if(temperature > 27)
		res |= TEMPERATURE_TOO_HIGH;
	if(humidity < 20)
		res |= HUMIDITY_TOO_LOW;
	if(humidity > 80)
		res |= HUMIDITY_TOO_HIGH;

	return res;
}
