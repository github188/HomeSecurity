//============================================================================
// Name        : clientLogin.cpp
// Author      : yaocoder
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C, Ansi-style
//============================================================================
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include "defines.h"
#include "init_configure.h"
#include "local_transport.h"
#include "redis_conn_pool.h"
#include "sql_conn_pool.h"
#include "sqlite_opt.h"
#include "global_settings.h"
#include "../public/config_file.h"
#include "../public/utils.h"

CSqlConnPool g_OptUserDataDB;
CSqlConnPool  g_OptDeviceDataDB;
SqliteOpt g_OptSqliteDB;

static void InitConfigure();
static void SettingsAndPrint();
static void InitUserSql();
static void InitRedis();
static void InitSqlite();
static void Run();
static void SigUsr(int signo);

int main(int argc, char **argv)
{

	/* process arguments */
	int c;
	std::string version = std::string("1.0.404:412M");
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

	InitUserSql();

	InitSqlite();

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
	initConfObj.SetConfigFilePath(std::string("/home/accountSys/conf/"));
	std::string project_name = "accountLogic";
	initConfObj.InitLog4cxx(project_name);
	if (!initConfObj.LoadConfiguration(project_name))
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

	LOG4CXX_INFO(g_logger, "InitRedis success.")
}

void InitUserSql()
{
	SqlConnInfo sqlConnInfo;
	sqlConnInfo.max_conn_num 	= utils::G<ConfigFile>().read<int> ("sqldb.connpool.max", 4);
	sqlConnInfo.ip 				= utils::G<ConfigFile>().read<std::string> ("sqldb.ip", "127.0.0.1");
	sqlConnInfo.port 			= utils::G<ConfigFile>().read<std::string> ("sqldb.port", "3306");
	sqlConnInfo.user_name 		= utils::G<ConfigFile>().read<std::string> ("sqldb.username", "root");
	std::string encryption 		= utils::G<ConfigFile>().read<std::string> ("sqldb.password", "tdqwqps");

	sqlConnInfo.password = utils::decrypt(encryption.c_str());

	g_OptUserDataDB.Init(sqlConnInfo);
	g_OptUserDataDB.SetDBName("AccountSys_USER");
	if (NULL == g_OptUserDataDB.GetConnection())
	{
		LOG4CXX_FATAL(g_logger, "Initialize mysql CloudSEEII_USER failed.");
		exit(EXIT_FAILURE);
	}

	g_OptDeviceDataDB.Init(sqlConnInfo);
	g_OptDeviceDataDB.SetDBName("DeviceSys");
	if (NULL == g_OptDeviceDataDB.GetConnection())
	{
		LOG4CXX_FATAL(g_logger, "Initialize mysql DeviceSys failed.");
		exit(EXIT_FAILURE);
	}

	LOG4CXX_INFO(g_logger, "InitUserSql success.")
}

void InitSqlite()
{
	if(!g_OptSqliteDB.OpenSqliteDB("/home/accountSys/localDB/replaceOldUser.db"))
	{
		exit(EXIT_FAILURE);
	}

	LOG4CXX_INFO(g_logger, "InitSqlite success.");
}

void SettingsAndPrint()
{
	utils::G<CGlobalSettings>().dispatch_ip_ 	= utils::G<ConfigFile>().read<std::string> ("logicDispatch.local.ip","127.0.0.1");
	utils::G<CGlobalSettings>().dispatch_port_ 	= utils::G<ConfigFile>().read<int> ("logicDispatch.local.listen.port", 5111);
	utils::G<CGlobalSettings>().bind_port_ 		= utils::G<ConfigFile>().read<int> ("accountLogic.bind.port", 5656);
	utils::G<CGlobalSettings>().thread_num_ 	= utils::G<ConfigFile>().read<int> ("accountLogic.worker.thread.num", 4);
	utils::G<CGlobalSettings>().queue_num_ 		= utils::G<ConfigFile>().read<int> ("accountLogic.worker.queue.num", 16);
	utils::G<CGlobalSettings>().stats_listen_port_ = utils::G<ConfigFile>().read<int>("accountLogic.stats.listen.port", 8021);

	LOG4CXX_INFO(g_logger, "******logicDispatch.local.ip = " 	<< utils::G<CGlobalSettings>().dispatch_ip_ << "******");
	LOG4CXX_INFO(g_logger, "******dispatch.proc.port =" 		<< utils::G<CGlobalSettings>().dispatch_port_<< "******");
	LOG4CXX_INFO(g_logger, "******accountLogic.bind.port=" 		<< utils::G<CGlobalSettings>().bind_port_ << "******");
	LOG4CXX_INFO(g_logger, "******accountLogic.worker.thread.num ="<< utils::G<CGlobalSettings>().thread_num_<< "******");
	LOG4CXX_INFO(g_logger, "******accountLogic.worker.queue.num =" << utils::G<CGlobalSettings>().queue_num_<< "******");
	LOG4CXX_INFO(g_logger, "******accountLogic.stats.listen.port = "  << utils::G<CGlobalSettings>().stats_listen_port_ << "******");
}

