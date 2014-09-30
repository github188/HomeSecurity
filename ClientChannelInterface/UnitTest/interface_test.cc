#include <gtest/gtest.h>

#include "../SdkSrc/net_interface.h"
#include <boost/filesystem.hpp>

INetChannelInterface *pINetChannelInterface = INetChannelInterface::GetImplInstance();

#define USE_LOG4CXX
LoggerPtr g_logger_in;

void ServerPushInfo(const int message_type, const SERVER_PUSH_INFO& serverPushInfo)
{
	LOG4CXX_TRACE(g_logger_in, "+++++ServerPushInfo messagetype = " << message_type
							<< ", message = " << serverPushInfo.message << "+++++");
}

void InitLogger(const std::string& file_path, const std::string& project_name)
{
	PropertyConfigurator::configure(file_path);
	g_logger_in = Logger::getLogger(project_name);
}

bool GetCurrentPath(std::string& current_path);

TEST(UserClientSDK, reentrant_InitSDK)
{
	std::string ip = "58.56.19.187";
	int port = 12002;
	std::string file_path, project_name;
	GetCurrentPath(file_path);
	file_path = file_path + "/conf/log.conf";
	project_name = "NetChannelInterface";
	InitLogger(file_path, project_name);

	ASSERT_TRUE(pINetChannelInterface->Init(ip, port, g_logger_in));
}

void LiveStatus(const int ret)
{
	LOG4CXX_TRACE(g_logger_in, "******LiveStatus ret = " << ret << "******");
}

bool GetCurrentPath(std::string& current_path)
{
	try
	{
		boost::filesystem::path path = boost::filesystem::current_path();
		current_path = path.string();
		return true;
	}
	catch (boost::filesystem::filesystem_error & e)
	{
		//cout << "current_path : " << current_path << ", error description :" << e.what() << endl;
		return false;
	}
}


TEST(UserClientSDK, reentrant_GetResponseByRequestShortConnection)
{
	std::string request = "{\"pv\":\"1.0\",\"lpt\":5,\"mt\":2029,\"dguid\":\"S35084741\",\"mid\":2,\"sid\":\"\"}";
	std::string response;
	pINetChannelInterface->GetResponseByRequestShortConnection(request, response);
	std::cout << "GetResponseByRequestShortConnection response=" << response << std::endl;
}
