//============================================================================
// Name        : alarmInfo.cpp
// Author      : Steven Zhang
// Version     :
// Copyright   : 2010-2014 Jovision
// Description : Hello World in C, Ansi-style
//============================================================================

#include <stdio.h>
#include <stdlib.h>
#include "defines.h"
#include "init_configure.h"
#include "local_transport.h"
#include "redis_conn_pool.h"
#include "global_settings.h"
#include "../public/config_file.h"
#include "../public/utils.h"

static void InitConfigure();
static void InitRedis();
static void SettingsAndPrint();
static void Run();

int main(int argc, char **argv)
{
	/* process arguments */
	int c;
	std::string version = std::string("1.0.470");
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
	InitRedis();
	SettingsAndPrint();

	if (daemon(1, 0) == -1)
	{
		LOG4CXX_FATAL(g_logger, "daemon failed.");
	}

	Run();

	return EXIT_SUCCESS;
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
	initConfObj.SetConfigFilePath(std::string("/home/alarmSys/conf/"));
	std::string project_name = "alarmInfo";
	initConfObj.InitLog4cxx(project_name);
	if (!initConfObj.LoadConfiguration(project_name))
	{
		LOG4CXX_FATAL(g_logger, "LoadConfiguration failed.");
		exit(EXIT_FAILURE);
	}
}

void InitRedis()
{
	RedisConnInfo rcInfo;
	rcInfo.ip = utils::G<ConfigFile>().read<std::string>("redis.ip", "127.0.0.1");
	rcInfo.port = utils::G<ConfigFile>().read<int>("redis.port", 6379);
	rcInfo.max_conn_num = 4;
	if(!CRedisConnPool::GetInstance()->Init(rcInfo)){
		LOG4CXX_ERROR(g_logger, "Init Redis connpool failed");
		exit(EXIT_FAILURE);
	}
}

void SettingsAndPrint()
{
	utils::G<CGlobalSettings>().dispatch_ip_ 	= utils::G<ConfigFile>().read<std::string> ("logicDispatch.local.ip","127.0.0.1");
	utils::G<CGlobalSettings>().dispatch_port_ 	= utils::G<ConfigFile>().read<int> ("logicDispatch.local.listen.port", 5111);
	utils::G<CGlobalSettings>().bind_port_ 		= utils::G<ConfigFile>().read<int> ("alarmInfo.bind.port", 6201);
	utils::G<CGlobalSettings>().thread_num_ 	= utils::G<ConfigFile>().read<int> ("alarmInfo.worker.thread.num", 4);
	utils::G<CGlobalSettings>().queue_num_ 		= utils::G<ConfigFile>().read<int> ("alarmInfo.worker.queue.num", 32);

	LOG4CXX_INFO(g_logger, "******logicDispatch.local.ip = " 	<< utils::G<CGlobalSettings>().dispatch_ip_ << "******");
	LOG4CXX_INFO(g_logger, "******dispatch.proc.port =" 		<< utils::G<CGlobalSettings>().dispatch_port_<< "******");
	LOG4CXX_INFO(g_logger, "******alarmInfo.bind.port=" 		<< utils::G<CGlobalSettings>().bind_port_ << "******");
	LOG4CXX_INFO(g_logger, "******alarmInfo.worker.thread.num ="<< utils::G<CGlobalSettings>().thread_num_<< "******");
	LOG4CXX_INFO(g_logger, "******alarmInfo.worker.queue.num =" << utils::G<CGlobalSettings>().queue_num_<< "******");
}
