//============================================================================
// Name        : autoUpdate.cpp
// Author      : 
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C, Ansi-style
//============================================================================

#include <stdio.h>
#include <stdlib.h>
#include "defines.h"
#include "init_configure.h"
#include "local_transport.h"
#include "sql_conn_pool.h"
#include "global_settings.h"
#include "../public/config_file.h"
#include "../public/utils.h"

CSqlConnPool g_OptClientSoftDataDB;

static void InitConfigure();
static void SettingsAndPrint();
static void InitUserSql();
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

	SettingsAndPrint();

	InitUserSql();

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
	initConfObj.SetConfigFilePath(std::string("/home/accountSys/conf/"));
	std::string project_name = "autoUpdate";
	initConfObj.InitLog4cxx(project_name);
	if (!initConfObj.LoadConfiguration(project_name))
	{
		LOG4CXX_FATAL(g_logger, "LoadConfiguration failed.");
		exit(EXIT_FAILURE);
	}
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

	g_OptClientSoftDataDB.Init(sqlConnInfo);
	g_OptClientSoftDataDB.SetDBName("AutoUpdate_SoftVersion");
	if (NULL == g_OptClientSoftDataDB.GetConnection())
	{
		LOG4CXX_FATAL(g_logger, "Initialize mysql CloudSEEII_USER failed.");
		exit(EXIT_FAILURE);
	}

	LOG4CXX_INFO(g_logger, "InitUserSql success.")
}

void SettingsAndPrint()
{
	utils::G<CGlobalSettings>().dispatch_ip_ 	= utils::G<ConfigFile>().read<std::string> ("logicDispatch.local.ip","127.0.0.1");
	utils::G<CGlobalSettings>().dispatch_port_ 	= utils::G<ConfigFile>().read<int> ("logicDispatch.local.listen.port", 5111);
	utils::G<CGlobalSettings>().bind_port_ 		= utils::G<ConfigFile>().read<int> ("autoUpdate.bind.port", 5659);
	utils::G<CGlobalSettings>().thread_num_ 	= utils::G<ConfigFile>().read<int> ("autoUpdate.worker.thread.num", 4);
	utils::G<CGlobalSettings>().queue_num_ 		= utils::G<ConfigFile>().read<int> ("autoUpdate.worker.queue.num", 16);

	LOG4CXX_INFO(g_logger, "******logicDispatch.local.ip = " 	<< utils::G<CGlobalSettings>().dispatch_ip_ << "******");
	LOG4CXX_INFO(g_logger, "******dispatch.proc.port =" 		<< utils::G<CGlobalSettings>().dispatch_port_<< "******");
	LOG4CXX_INFO(g_logger, "******autoUpdate.bind.port=" 		<< utils::G<CGlobalSettings>().bind_port_ << "******");
	LOG4CXX_INFO(g_logger, "******autoUpdate.worker.thread.num ="<< utils::G<CGlobalSettings>().thread_num_<< "******");
	LOG4CXX_INFO(g_logger, "******autoUpdate.worker.queue.num =" << utils::G<CGlobalSettings>().queue_num_<< "******");
}
