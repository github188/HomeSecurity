/*
 * deviceInfo_HomeCloud.cpp
 *
 *  Created on: July 18, 2014
 *      Author: zhangs
 */
#include <stdlib.h>
#include <signal.h>
#include "defines.h"
#include "init_configure.h"
#include "redis_conn_pool.h"
#include "sql_conn_pool.h"
#include "local_transport.h"
#include "global_settings.h"
#include "ontime_thread.h"
#include "../public/utils.h"
#include "../public/config_file.h"
#include "evp_crypt.h"

CSqlConnPool  g_OptAccountDataDB;
CSqlConnPool  g_OptDeviceDataDB;
static void InitConfigure();
static void SettingsAndPrint();
static void InitSql();
static void InitRedis();
static void Run();
static void SigUsr(int signo);

int main(int argc, char *argv[])
{
	/* process arguments */
	int c;
	std::string version = std::string("1.0.196:227");
	while (-1 != (c = getopt(argc, argv,
		"v" 	/* 获取程序版本号，配合svn */
		)))
	{
		switch (c)
		{
		case 'v':
			printf("The version is %s\n", version.c_str());
			return EXIT_SUCCESS;
		default:
			break;
		}
	}

	InitConfigure();

	SettingsAndPrint();

	if (signal(SIGUSR1, SigUsr) == SIG_ERR )
	{
		LOG4CXX_FATAL(g_logger, "Configure signal failed.");
		exit(EXIT_FAILURE);
	}

	InitRedis();

	InitSql();

	if (daemon(1, 0) == -1)
	{
		LOG4CXX_FATAL(g_logger, "daemon failed.");
	}

	Run();

	return EXIT_SUCCESS;
}

void Run()
{
	// 启动定时线程
	COntimeThread onTimeThread;
	onTimeThread.Start();

	CLocalTransport localTransport;
	if (!localTransport.SetupTransport())
	{
		LOG4CXX_FATAL(g_logger, "SetupTransport failed.");
		exit(EXIT_FAILURE);
	}

	localTransport.Run();
}

void SigUsr(int signo)
{
	if(signo == SIGUSR1)
	{
		/* 重新加载应用配置文件（仅仅是连接超时时间），log4cxx日志配置文件*/
		InitConfigure();
		SettingsAndPrint();
		LOG4CXX_INFO(g_logger, "reload configure.");
		return;
	}
}

void InitConfigure()
{
	CInitConfig initConfObj;
	initConfObj.SetConfigFilePath(std::string("/home/deviceSys/conf/"));
	initConfObj.InitLog4cxx("deviceInfo_HomeCloud");
	if (!initConfObj.LoadConfiguration("deviceInfo_HomeCloud"))
	{
		LOG4CXX_FATAL(g_logger, "LoadConfiguration failed.");
		exit(EXIT_FAILURE);
	}
}

void InitRedis()
{
	RedisConnInfo redisConnInfo;
	redisConnInfo.max_conn_num = 4;
	redisConnInfo.ip 	= utils::G<ConfigFile>().read<std::string> ("redis.ip", "127.0.0.1");
	redisConnInfo.port 	= utils::G<ConfigFile>().read<int> ("redis.port", 6379);
	if (!CRedisConnPool::GetInstance()->Init(redisConnInfo))
	{
		LOG4CXX_FATAL(g_logger, "Init redisConnPool failed.");
		exit(EXIT_FAILURE);
	}
}

void InitSql()
{
	SqlConnInfo sqlConnInfo;
	sqlConnInfo.max_conn_num = utils::G<ConfigFile>().read<int> ("sqldb.connpool.max", 4);
	sqlConnInfo.ip = utils::G<ConfigFile>().read<std::string> ("sqldb.ip", "127.0.0.1");
	sqlConnInfo.port = utils::G<ConfigFile>().read<std::string> ("sqldb.port", "3306");
	sqlConnInfo.user_name = utils::G<ConfigFile>().read<std::string> ("sqldb.username", "root");
	sqlConnInfo.password = utils::G<ConfigFile>().read<std::string> ("sqldb.password", "testbgp");

	/*
	EvpCrypt evpOpt;
	std::string password;
	evpOpt.setKey("jovision");
	if (evpOpt.DoEvpDecode(encryption, password))
	{
		sqlConnInfo.password = password;
	}
	*/

	g_OptAccountDataDB.Init(sqlConnInfo);
	g_OptAccountDataDB.SetDBName("AccountSys_USER");
	if (NULL == g_OptAccountDataDB.GetConnection())
	{
		LOG4CXX_FATAL(g_logger, "Initialize mysql AccountSys_USER failed.");
		exit(EXIT_FAILURE);
	}

	g_OptDeviceDataDB.Init(sqlConnInfo);
	g_OptDeviceDataDB.SetDBName("DeviceSys");
	if (NULL == g_OptDeviceDataDB.GetConnection())
	{
		LOG4CXX_FATAL(g_logger, "Initialize mysql DeviceSys failed.");
		exit(EXIT_FAILURE);
	}
}

void SettingsAndPrint()
{
	utils::G<CGlobalSettings>().dispatch_ip_	= utils::G<ConfigFile>().read<std::string> ("logicDispatch.local.ip","127.0.0.1");
	utils::G<CGlobalSettings>().dispatch_port_	= utils::G<ConfigFile>().read<int> ("logicDispatch.local.listen.port", 5111);
	utils::G<CGlobalSettings>().bind_port_		= utils::G<ConfigFile>().read<int> ("deviceInfo_HomeCloud.bind.port", 5659);
	utils::G<CGlobalSettings>().thread_num_		= utils::G<ConfigFile>().read<int> ("deviceInfo_HomeCloud.worker.thread.num", 4);
	utils::G<CGlobalSettings>().queue_num_		= utils::G<ConfigFile>().read<int> ("deviceInfo_HomeCloud.worker.queue.num", 16);
	utils::G<CGlobalSettings>().stats_listen_port_ = utils::G<ConfigFile>().read<int>("deviceInfo_HomeCloud.stats.listen.port", 8024);

	LOG4CXX_INFO(g_logger, "******logicDispatch.local.ip =" 		<< utils::G<CGlobalSettings>().dispatch_ip_  << "******");
	LOG4CXX_INFO(g_logger, "******logicDispatch.local.listen.port ="<< utils::G<CGlobalSettings>().dispatch_port_<< "******");
	LOG4CXX_INFO(g_logger, "******deviceInfo.bind.port ="			<< utils::G<CGlobalSettings>().bind_port_    << "******");
	LOG4CXX_INFO(g_logger, "******deviceInfo.worker.thread.num =" 	<< utils::G<CGlobalSettings>().thread_num_   << "******");
	LOG4CXX_INFO(g_logger, "******deviceInfo.worker.queue.num =" 	<< utils::G<CGlobalSettings>().queue_num_	 << "******");
}
