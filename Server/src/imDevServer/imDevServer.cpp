//============================================================================
// Name        : iMDevServer.cpp
// Author      : zhangs
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C, Ansi-style
//============================================================================

#include <stdlib.h>
#include <signal.h>
#include "defines.h"
#include "init_configure.h"
#include "net_core.h"
#include "redis_conn_pool.h"
#include "sql_conn_pool.h"
#include "global_settings.h"
#include "../public/utils.h"
#include "../public/config_file.h"
#include "net_interface.h"

CSqlConnPool  g_OptDeviceDataDB;
INetChannelInterface *g_LogicDispatch_ptr;
static void InitConfigure();
static void SettingsAndPrint();
static void InitSql();
static void InitRedis();
static void InitLogicDispatchChannel();
static void Run();
static void SigUsr(int signo);

int main(void)
{
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

	InitLogicDispatchChannel();

	Run();

	return EXIT_SUCCESS;
}

void Run()
{
	CNetCore netCoreObj;
	if(!netCoreObj.InitNetCore())
	{
		LOG4CXX_FATAL(g_logger, "InitNetCore failed.");
		exit(EXIT_FAILURE);
	}

	netCoreObj.Run();
}

void SigUsr(int signo)
{
	if(signo == SIGUSR1)
	{
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
	initConfObj.InitLog4cxx("imDevServer");
	if (!initConfObj.LoadConfiguration("imDevServer"))
	{
		LOG4CXX_FATAL(g_logger, "LoadConfiguration failed.");
		exit(EXIT_FAILURE);
	}
}

void InitRedis()
{
	RedisConnInfo redisConnInfo;
	redisConnInfo.max_conn_num = 4;
	redisConnInfo.ip = utils::G<ConfigFile>().read<std::string> ("redis.ip", "127.0.0.1");
	redisConnInfo.port = utils::G<ConfigFile>().read<int> ("redis.port", 6379);
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

	g_OptDeviceDataDB.Init(sqlConnInfo);
	g_OptDeviceDataDB.SetDBName("DeviceSys");
	if (NULL == g_OptDeviceDataDB.GetConnection())
	{
		LOG4CXX_FATAL(g_logger, "Initialize mysql DeviceSys failed.");
		exit(EXIT_FAILURE);
	}
}

void InitLogicDispatchChannel()
{
	std::string logic_dispatch_ip = utils::G<ConfigFile>().read<std::string> ("logicDispatch.ip", "58.56.19.187");
	int logic_dispatch_port = utils::G<ConfigFile>().read<int> ("logicDispatch.port", 12002);
	g_LogicDispatch_ptr = INetChannelInterface::GetImplInstance();
	if (!g_LogicDispatch_ptr->Init(logic_dispatch_ip, logic_dispatch_port, g_logger))
	{
		LOG4CXX_FATAL(g_logger, "Init LogicDispatchChannel failed.");
		exit(EXIT_FAILURE);
	}
}

void SettingsAndPrint()
{
	utils::G<CGlobalSettings>().remote_listen_port_ = utils::G<ConfigFile>().read<int> ("imDevServer.remote.listen.port", 12008);
	utils::G<CGlobalSettings>().thread_num_ = utils::G<ConfigFile>().read<int> ("imDevServer.worker.thread.num", 4);
	utils::G<CGlobalSettings>().relay_server_ip_ = utils::G<ConfigFile>().read<std::string> ("imRelay.server.ip","127.0.0.1");
	utils::G<CGlobalSettings>().relay_server_port_ = utils::G<ConfigFile>().read<int> ("imRelay.server.listen.port", 6111);
	utils::G<CGlobalSettings>().bind_port_ = utils::G<ConfigFile>().read<int> ("imDevServer.bind.port", 8001);
	utils::G<CGlobalSettings>().client_heartbeat_timeout_ = utils::G<ConfigFile>().read<int>("imDevServer.client.timeout.s", 70);
	utils::G<CGlobalSettings>().ftp_server_domain_	= utils::G<ConfigFile>().read<std::string> ("ftp.server.domain","127.0.0.1");
	utils::G<CGlobalSettings>().ftp_server_port_	= utils::G<ConfigFile>().read<int> ("ftp.server.port", 80);

	LOG4CXX_INFO(g_logger, "******imDevServer.remote.listen.port = " << utils::G<CGlobalSettings>().remote_listen_port_ << "******");
	LOG4CXX_INFO(g_logger, "******imDevServer.worker.thread.num = " << utils::G<CGlobalSettings>().thread_num_ << "******");
	LOG4CXX_INFO(g_logger, "******imRelay.server.ip = " << utils::G<CGlobalSettings>().relay_server_ip_ << "******");
	LOG4CXX_INFO(g_logger, "******rimRelay.server.listen.port = " << utils::G<CGlobalSettings>().relay_server_port_ << "******");
	LOG4CXX_INFO(g_logger, "******imDevServer.bind.port =" << utils::G<CGlobalSettings>().bind_port_ << "******");
	LOG4CXX_INFO(g_logger, "******imDevServer.client.timeout.s = " << utils::G<CGlobalSettings>().client_heartbeat_timeout_ << "******");
}

