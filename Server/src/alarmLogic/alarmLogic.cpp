//============================================================================
// Name        : alarmLogic.cpp
// Author      : 
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C, Ansi-style
//============================================================================

#include <stdio.h>
#include <stdlib.h>
#include "init_configure.h"
#include "local_transport.h"
#include "global_settings.h"
#include "redis_conn_pool.h"
#include "net_interface.h"
#include "../public/config_file.h"
#include "../public/utils.h"


static void InitConfigure();
static void SettingsAndPrint();
static void Run();
static void InitRedis();
static void InitAlarmServer();

INetChannelInterface* g_alarmSeverChannel;


void LiveStatus(const int ret)
{
	LOG4CXX_TRACE(g_logger, "******LiveStatus ret = " << ret << "******");
}

void ServerPushInfo(const SERVER_PUSH_INFO& serverPushInfo)
{
	LOG4CXX_TRACE(g_logger, "+++++++++++++++++++++++++++++++++++++++++ServerPushInfo  message_type = "
			                 << serverPushInfo.message_type	<< "+++++++++++++++++++++++++++++++++++++++++");
}

int main(int argc, char **argv)
{
	/* process arguments */
	int c;
	std::string version = std::string("1.0.");
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

	InitRedis();

	InitAlarmServer();

	if (daemon(1, 0) == -1)
	{
		LOG4CXX_FATAL(g_logger, "daemon failed.");
	}

	Run();

	return EXIT_SUCCESS;
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

void Run()
{
	CLocalTransport localTransport;
	if (!localTransport.SetupTransport())
	{
		LOG4CXX_FATAL(g_logger, "SetupTransport failed.");
		exit(EXIT_FAILURE);
	}

	localTransport.Run();
}

void InitConfigure()
{
	CInitConfig initConfObj;
	initConfObj.SetConfigFilePath(std::string("/home/yaowei/conf/"));
	std::string project_name = "alarmLogic";
	initConfObj.InitLog4cxx(project_name);
	if (!initConfObj.LoadConfiguration(project_name))
	{
		LOG4CXX_FATAL(g_logger, "LoadConfiguration failed.");
		exit(EXIT_FAILURE);
	}
}

void SettingsAndPrint()
{
	utils::G<CGlobalSettings>().dispatch_ip_ 	= utils::G<ConfigFile>().read<std::string> ("logicDispatch.local.ip","127.0.0.1");
	utils::G<CGlobalSettings>().dispatch_port_ 	= utils::G<ConfigFile>().read<int> ("logicDispatch.local.listen.port", 5111);
	utils::G<CGlobalSettings>().bind_port_ 		= utils::G<ConfigFile>().read<int> ("alarmLogic.bind.port", 6658);
	utils::G<CGlobalSettings>().thread_num_ 	= utils::G<ConfigFile>().read<int> ("alarmLogic.worker.thread.num", 4);
	utils::G<CGlobalSettings>().queue_num_ 		= utils::G<ConfigFile>().read<int> ("alarmLogic.worker.queue.num", 15);
	utils::G<CGlobalSettings>().alarm_server_ip_= utils::G<ConfigFile>().read<std::string> ("alarmServer.ip","127.0.0.1");
	utils::G<CGlobalSettings>().alarm_server_port_= utils::G<ConfigFile>().read<int> ("alarmServer.port",8766);
	utils::G<CGlobalSettings>().stats_listen_port_ = utils::G<ConfigFile>().read<int>("alarmLogic.stats.listen.port", 8022);

	LOG4CXX_INFO(g_logger, "******logicDispatch.local.ip = " 	<< utils::G<CGlobalSettings>().dispatch_ip_ << "******");
	LOG4CXX_INFO(g_logger, "******dispatch.proc.port =" 		<< utils::G<CGlobalSettings>().dispatch_port_<< "******");
	LOG4CXX_INFO(g_logger, "******alarmLogic.bind.port=" 		<< utils::G<CGlobalSettings>().bind_port_ << "******");
	LOG4CXX_INFO(g_logger, "******alarmLogic.worker.thread.num ="<< utils::G<CGlobalSettings>().thread_num_<< "******");
	LOG4CXX_INFO(g_logger, "******alarmLogic.worker.queue.num =" << utils::G<CGlobalSettings>().queue_num_<< "******");
	LOG4CXX_INFO(g_logger, "******alarmServer.ip = " 			<< utils::G<CGlobalSettings>().alarm_server_ip_ << "******");
	LOG4CXX_INFO(g_logger, "******alarmServer.port =" 			<< utils::G<CGlobalSettings>().alarm_server_port_<< "******");
	LOG4CXX_INFO(g_logger, "******alarmLogic.stats.listen.port = "  << utils::G<CGlobalSettings>().stats_listen_port_ << "******");
}

void InitAlarmServer()
{
	g_alarmSeverChannel = INetChannelInterface::GetImplInstance();

	std::string ip = utils::G<CGlobalSettings>().alarm_server_ip_;
	int port = utils::G<CGlobalSettings>().alarm_server_port_;
	if(!g_alarmSeverChannel->Init(ip, port, g_logger))
	{
		LOG4CXX_FATAL(g_logger, "Init alarmSeverChannel failed.");
		exit(EXIT_FAILURE);
	}

	g_alarmSeverChannel->EstablishPersistentChannel(LiveStatus);

	g_alarmSeverChannel->RegisterServerPushFunc(ServerPushInfo);
}

