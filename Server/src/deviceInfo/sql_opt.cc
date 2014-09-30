#include "sql_opt.h"
#include "defines.h"
#include "sql_utils.h"
#include "sql_conn_pool.h"
#include "../public/user_interface_defines.h"
#include "../public/utils.h"

extern CSqlConnPool g_OptAccountDataDB;
extern CSqlConnPool g_OptDeviceDataDB;

CSqlOpt::CSqlOpt()
{

}

CSqlOpt::~CSqlOpt()
{
}

int CSqlOpt::DeviceRegisterToDB(const DEVICE_INFO &device_info)
{
	if (device_info.device_guid.empty())
	{
		LOG4CXX_ERROR(g_logger, "CSqlOpt::DeviceRegisterToDB arg error.");
		return FAILED;
	}

	sql::Connection* con;
	con = g_OptDeviceDataDB.GetConnection();
	if(con == NULL)
	{
		LOG4CXX_ERROR(g_logger, "CSqlOpt::DeviceRegisterToDB:g_OptDeviceDataDB.GetConnection = NULL");
		return MY_SQL_ERROR;
	}

	int b_ok = SUCCESS;
	int b_hasexist = 0;
	sql::PreparedStatement* prep_stmt = NULL;
	sql::ResultSet* res = NULL;

	try
	{
		con->setAutoCommit(false);

		prep_stmt = con->prepareStatement("select device_guid from tb_device_basic where device_guid=?");
		prep_stmt->setString(1, device_info.device_guid);
		res = prep_stmt->executeQuery();
		if(res->next())
		{
			b_hasexist = DEVICE_HAS_EXIST;
		}
		SAFE_CLOSE2(res, prep_stmt);

		char register_time[32] = {0};
		time_t tnow = time(NULL);
		struct tm *tm = localtime(&tnow);
		strftime(register_time, sizeof (register_time), "%Y-%m-%d %H:%M:%S", tm);

		if (b_hasexist != DEVICE_HAS_EXIST)
		{
			prep_stmt = con->prepareStatement("INSERT INTO tb_device_basic (device_guid, device_type, device_name, register_time, reset_flag) VALUES (?, ?, ?, ?, ?)");
			prep_stmt->setString(1, device_info.device_guid);
			prep_stmt->setInt(2, device_info.device_type);
			prep_stmt->setString(3, device_info.device_name);
			prep_stmt->setDateTime(4, std::string(register_time));
			prep_stmt->setInt(5, device_info.reset_flag);
			prep_stmt->execute();
		}
		else
		{
			prep_stmt = con->prepareStatement("UPDATE tb_device_basic SET device_type=?, device_name=?, register_time=?, reset_flag=? WHERE device_guid=?");
			prep_stmt->setInt(1, device_info.device_type);
			prep_stmt->setString(2, device_info.device_name);
			prep_stmt->setDateTime(3, std::string(register_time));
			prep_stmt->setInt(4, device_info.reset_flag);
			prep_stmt->setString(5, device_info.device_guid);
			prep_stmt->execute();			
		}
		SAFE_CLOSE(prep_stmt);
		
		std::string detail_sql;
		switch (device_info.device_type)
		{
		case DEV_TYPE_HOME_IPC:
			b_hasexist = 0;

			prep_stmt = con->prepareStatement("select device_guid from tb_device_IPC where device_guid=?");
			prep_stmt->setString(1, device_info.device_guid);
			res = prep_stmt->executeQuery();
			if(res->next())
			{
				b_hasexist = DEVICE_HAS_EXIST;
			}
			SAFE_CLOSE2(res, prep_stmt);

			if (b_hasexist != DEVICE_HAS_EXIST)
			{
				detail_sql = 
					"INSERT INTO tb_device_IPC \
					(device_guid, device_sub_type, device_version, device_username, device_password, device_name, device_ip, device_netstat, \
					net_storage_switch, tf_storage_switch, alarm_switch, alarm_video_ftp_url, alarm_snap_ftp_url, alarm_ftp_acc, \
					alarm_ftp_pwd, alarm_time, jpeg_ftp_url_big, jpeg_ftp_url_small, jpeg_ftp_acc, jpeg_ftp_pwd, jpeg_upload_timing, video_fluency, baby_mode, full_alarm_mode) \
					values (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)";
				prep_stmt = con->prepareStatement(detail_sql);
				prep_stmt->setString(1, device_info.device_guid);
				prep_stmt->setInt(2, device_info.device_sub_type);
				prep_stmt->setString(3, device_info.device_version);
				prep_stmt->setString(4, device_info.device_username);
				prep_stmt->setString(5, device_info.device_password);
				prep_stmt->setString(6, device_info.device_name);
				prep_stmt->setString(7, device_info.device_ip);
				prep_stmt->setInt(8, device_info.device_netstat);
				prep_stmt->setInt(9, device_info.net_storage_switch);
				prep_stmt->setInt(10, device_info.tf_storage_switch);
				prep_stmt->setInt(11, device_info.alarm_switch);
				prep_stmt->setString(12, device_info.alarm_video_ftp_url);
				prep_stmt->setString(13, device_info.alarm_snap_ftp_url);
				prep_stmt->setString(14, device_info.alarm_ftp_acc);
				prep_stmt->setString(15, device_info.alarm_ftp_pwd);
				prep_stmt->setString(16, device_info.alarm_time);
				prep_stmt->setString(17, device_info.jpeg_ftp_url_big);
				prep_stmt->setString(18, device_info.jpeg_ftp_url_small);
				prep_stmt->setString(19, device_info.jpeg_ftp_acc);
				prep_stmt->setString(20, device_info.jpeg_ftp_pwd);
				prep_stmt->setInt(21, device_info.jpeg_upload_timing);
				prep_stmt->setInt(22, device_info.video_fluency);
				prep_stmt->setInt(23, device_info.baby_mode);
				prep_stmt->setInt(24, device_info.full_alarm_mode);
				prep_stmt->execute();
			}
			else
			{
				detail_sql = 
					"UPDATE tb_device_IPC SET \
					device_username=?, device_sub_type=?, device_version=?, device_password=?, device_name=?, device_ip=?, device_netstat=?, \
					net_storage_switch=?, tf_storage_switch=?, alarm_switch=?, alarm_video_ftp_url=?, alarm_snap_ftp_url=?, alarm_ftp_acc=?, \
					alarm_ftp_pwd=?, alarm_time=?, jpeg_ftp_url_big=?, jpeg_ftp_url_small=?, jpeg_ftp_acc=?, jpeg_ftp_pwd=?, jpeg_upload_timing=?, video_fluency=?, baby_mode=?, full_alarm_mode=? \
					WHERE device_guid=?";
				prep_stmt = con->prepareStatement(detail_sql);
				prep_stmt->setString(1, device_info.device_username);
				prep_stmt->setInt(2, device_info.device_sub_type);
				prep_stmt->setString(3, device_info.device_version);
				prep_stmt->setString(4, device_info.device_password);
				prep_stmt->setString(5, device_info.device_name);
				prep_stmt->setString(6, device_info.device_ip);
				prep_stmt->setInt(7, device_info.device_netstat);
				prep_stmt->setInt(8, device_info.net_storage_switch);
				prep_stmt->setInt(9, device_info.tf_storage_switch);
				prep_stmt->setInt(10, device_info.alarm_switch);
				prep_stmt->setString(11, device_info.alarm_video_ftp_url);
				prep_stmt->setString(12, device_info.alarm_snap_ftp_url);
				prep_stmt->setString(13, device_info.alarm_ftp_acc);
				prep_stmt->setString(14, device_info.alarm_ftp_pwd);
				prep_stmt->setString(15, device_info.alarm_time);
				prep_stmt->setString(16, device_info.jpeg_ftp_url_big);
				prep_stmt->setString(17, device_info.jpeg_ftp_url_small);
				prep_stmt->setString(18, device_info.jpeg_ftp_acc);
				prep_stmt->setString(19, device_info.jpeg_ftp_pwd);
				prep_stmt->setInt(20, device_info.jpeg_upload_timing);
				prep_stmt->setInt(21, device_info.video_fluency);
				prep_stmt->setInt(22, device_info.baby_mode);
				prep_stmt->setInt(23, device_info.full_alarm_mode);
				prep_stmt->setString(24, device_info.device_guid);
				prep_stmt->execute();
			}
			SAFE_CLOSE(prep_stmt);
			break;
		default:
			break;
		}

		con->commit();
	}
	catch(sql::SQLException& e)
	{
		con->rollback();
		LOG4CXX_ERROR(g_logger, "CSqlOpt::DeviceRegisterToDB: exception = " << e.what());
		b_ok = MY_SQL_ERROR;
	}

	SAFE_CLOSE2(res, prep_stmt);
	con->setAutoCommit(true);
	g_OptDeviceDataDB.ReleaseConnection(con);

	return b_ok;
}

int CSqlOpt::GetUserDevicesInfoFromDB(const std::string& username, std::vector<std::string>& vec_device, USER_DEVICES_LIST& user_devices_list)
{
	if (username.empty())
	{
		LOG4CXX_ERROR(g_logger, "CSqlOpt::GetUserDevicesInfoFromDB arg error.");
		return FAILED;
	}

	sql::Connection* con;
	con = g_OptDeviceDataDB.GetConnection();
	if (con == NULL)
	{
		LOG4CXX_ERROR(g_logger, "CSqlOpt::GetUserDevicesInfoFromDB:optDataDB.GetConnection = NULL");
		return MY_SQL_ERROR;
	}

	int b_ok = SUCCESS;
	sql::PreparedStatement* prep_stmt = NULL;
	sql::ResultSet* res = NULL;

	try
	{
		prep_stmt = con->prepareStatement("SELECT device_guid, device_type FROM tb_user_relation_device WHERE user_name=?");
		prep_stmt->setString(1, username);
		res = prep_stmt->executeQuery();
		while (res->next())
		{
			std::string guid = res->getString("device_guid");
			vec_device.push_back(guid);
			//test
			std::cout << "^^^^^^^^^^ GetUserDevicesInfoFromDB-guid=" << guid << std::endl;
			//test
		}
		SAFE_CLOSE2(prep_stmt,res);

		USER_DEVICE_INFO user_device_info;
		for (unsigned i = 0; i < vec_device.size(); ++i)
		{
			if ((b_ok = GetDeviceInfoFromDB(vec_device.at(i), user_device_info.device_info)) == SUCCESS)
			{
				if ((b_ok = GetUserDeviceConfigFromDB(username, vec_device.at(i), user_device_info.user_device_config)) == SUCCESS)
				{
					user_devices_list.vec_user_device_info.push_back(user_device_info);
				}
				else
				{
					LOG4CXX_TRACE(g_logger, "CSqlOpt::GetUserDevicesInfoFromDB:GetUserDeviceConfigFromDB failed.");
				}
			}
			else
			{
				LOG4CXX_ERROR(g_logger, "CSqlOpt::GetUserDevicesInfoFromDB error.");
			}
		}
	}
	catch (sql::SQLException& e)
	{
		LOG4CXX_ERROR(g_logger, "CSqlOpt::GetUserDevicesInfoFromDB:optDataDB = " << e.what());
		b_ok = MY_SQL_ERROR;
	}

	SAFE_CLOSE2(res, prep_stmt);
	g_OptDeviceDataDB.ReleaseConnection(con);

	return b_ok;
}

int CSqlOpt::GetDeviceUsersFromDB(const std::string& device_guid, std::vector<std::string> &vec_un, std::string &device_name, std::string &alarm_time, int &full_alarm_mode, std::string& device_username, std::string& device_password)
{
	std::string video_username, video_password;

	if (device_guid.empty())
	{
		LOG4CXX_ERROR(g_logger, "CSqlOpt::GetDeviceUsersFromDB arg error.");
		return FAILED;
	}

	sql::Connection* con;
	con = g_OptDeviceDataDB.GetConnection();
	if (con == NULL)
	{
		LOG4CXX_ERROR(g_logger, "CSqlOpt::GetDeviceUsersFromDB:GetConnection = NULL");
		return MY_SQL_ERROR;
	}

	int b_ok = SUCCESS;
	sql::PreparedStatement* prep_stmt = NULL;
	sql::ResultSet* res = NULL;

	try
	{
		prep_stmt
			= con->prepareStatement(
			"SELECT user_name, device_name, video_username, video_password FROM tb_user_relation_device WHERE device_guid=? ");
		prep_stmt->setString(1, device_guid);
		res = prep_stmt->executeQuery();
		std::string usrname;
		while (res->next())
		{
			video_username = res->getString("video_username");
			video_password = res->getString("video_password");
			if(video_username == device_username && video_password == device_password)
			{
				usrname = res->getString("user_name");
				vec_un.push_back(usrname);

				device_name = res->getString("device_name");
			}
		}
		SAFE_CLOSE2(prep_stmt,res);

		prep_stmt
			= con->prepareStatement(
			"SELECT alarm_time, full_alarm_mode FROM tb_device_IPC WHERE device_guid=? ");
		prep_stmt->setString(1, device_guid);
		res = prep_stmt->executeQuery();
		if (res->next())
		{
			alarm_time = res->getString("alarm_time");
			full_alarm_mode = res->getInt("full_alarm_mode");
		}

		SAFE_CLOSE2(prep_stmt,res);
	}
	catch (sql::SQLException& e)
	{
		LOG4CXX_ERROR(g_logger, "CSqlOpt::GetDeviceUsersFromDB:optDataDB = " << e.what());
		b_ok = MY_SQL_ERROR;
	}

	SAFE_CLOSE2(res, prep_stmt);

	g_OptDeviceDataDB.ReleaseConnection(con);

	return b_ok;
}

int CSqlOpt::GetDeviceInfoFromDB(const std::string& device_guid, DEVICE_INFO& device_info)
{
	if (device_guid.empty())
	{
		LOG4CXX_ERROR(g_logger, "CSqlOpt::GetDeviceInfoFromDB arg error.");
		return FAILED;
	}

	sql::Connection* con;
	con = g_OptDeviceDataDB.GetConnection();
	if (con == NULL)
	{
		LOG4CXX_ERROR(g_logger, "CSqlOpt::GetDeviceInfoFromDB:GetConnection = NULL");
		return MY_SQL_ERROR;
	}

	int b_ok = SUCCESS;
	sql::PreparedStatement* prep_stmt = NULL;
	sql::ResultSet* res = NULL;

	try
	{
		prep_stmt = con->prepareStatement("SELECT device_name,device_type,wifi_flag,reset_flag FROM tb_device_basic WHERE device_guid=?");
		prep_stmt->setString(1, device_guid);
		res = prep_stmt->executeQuery();

		if (res->next())
		{
			device_info.device_guid = device_guid;
			device_info.device_type = res->getInt("device_type");
			device_info.wifi_flag = res->getInt("wifi_flag");
			device_info.reset_flag = res->getInt("reset_flag");
			device_info.device_name = res->getString("device_name");
		}
		else
		{
			LOG4CXX_ERROR(g_logger, "CSqlOpt::GetDeviceInfoFromDB:SELECT tb_device_basic null.");
			b_ok = SQL_NOT_FIND;
		}

		SAFE_CLOSE2(prep_stmt, res);
		
		std::string detail_sql;
		switch (device_info.device_type)
		{
		case DEV_TYPE_HOME_IPC:
			detail_sql = "SELECT \
						 device_sub_type, device_username, device_password, device_name, device_ip, device_netstat, \
						 net_storage_switch, tf_storage_switch, alarm_switch, alarm_video_ftp_url, alarm_snap_ftp_url, \
						 alarm_ftp_acc, alarm_ftp_pwd, alarm_time, jpeg_ftp_url_big, jpeg_ftp_url_small, jpeg_ftp_acc, \
						 jpeg_ftp_pwd, jpeg_upload_timing, video_fluency, device_version, baby_mode, full_alarm_mode \
						 FROM tb_device_IPC WHERE device_guid=?";
			prep_stmt = con->prepareStatement(detail_sql);
			prep_stmt->setString(1, device_guid);
			res = prep_stmt->executeQuery();
			if (res->next())
			{
				device_info.device_sub_type = res->getInt("device_sub_type");
				device_info.device_username = res->getString("device_username");
				device_info.device_password = res->getString("device_password");
				device_info.device_name = res->getString("device_name");
				device_info.device_ip = res->getString("device_ip");
				device_info.device_netstat = res->getInt("device_netstat");
				device_info.net_storage_switch = res->getInt("net_storage_switch");
				device_info.tf_storage_switch = res->getInt("tf_storage_switch");
				device_info.alarm_switch = res->getInt("alarm_switch");
				device_info.alarm_video_ftp_url = res->getString("alarm_video_ftp_url");
				device_info.alarm_snap_ftp_url = res->getString("alarm_snap_ftp_url");
				device_info.alarm_ftp_acc = res->getString("alarm_ftp_acc");
				device_info.alarm_ftp_pwd = res->getString("alarm_ftp_pwd");
				device_info.alarm_time = res->getString("alarm_time");
				device_info.jpeg_ftp_url_big = res->getString("jpeg_ftp_url_big");
				device_info.jpeg_ftp_url_small = res->getString("jpeg_ftp_url_small");
				device_info.jpeg_ftp_acc = res->getString("jpeg_ftp_acc");
				device_info.jpeg_ftp_pwd = res->getString("jpeg_ftp_pwd");
				device_info.jpeg_upload_timing = res->getInt("jpeg_upload_timing");
				device_info.video_fluency = res->getInt("video_fluency");
				device_info.device_version = res->getString("device_version");
				device_info.baby_mode = res->getInt("baby_mode");
				device_info.full_alarm_mode = res->getInt("full_alarm_mode");
			}
			SAFE_CLOSE2(res, prep_stmt);
			
			prep_stmt = con->prepareStatement("SELECT sub_type FROM tb_device_sub_type WHERE sub_type_id=?");
			prep_stmt->setInt(1, device_info.device_sub_type);
			res = prep_stmt->executeQuery();
			if (res->next())
			{
				device_info.device_sub_type_str = res->getString("sub_type");
			}
			SAFE_CLOSE2(res, prep_stmt);

			break;

		default:
			break;
		}
	}
	catch (sql::SQLException& e)
	{
		LOG4CXX_ERROR(g_logger, "CSqlOpt::GetDeviceInfoFromDB:optDataDB = " << e.what());
		b_ok = MY_SQL_ERROR;
	}

	SAFE_CLOSE2(res, prep_stmt);
	g_OptDeviceDataDB.ReleaseConnection(con);

	return b_ok;
}

int CSqlOpt::GetUserDeviceInfoFromDB(const std::string& username, const std::string& device_guid, USER_DEVICE_INFO& user_device_info)
{
	int b_ok = SUCCESS;

	if (SUCCESS == GetDeviceInfoFromDB(device_guid, user_device_info.device_info))
	{
		if (SUCCESS == GetUserDeviceConfigFromDB(username, device_guid, user_device_info.user_device_config))
		{
			LOG4CXX_TRACE(g_logger, "CSqlOpt::GetUserDevicesInfoFromDB:GetVideoLinkTypeFromDB OK.");
		}
		else
		{
			LOG4CXX_TRACE(g_logger, "CSqlOpt::GetUserDevicesInfoFromDB:GetVideoLinkTypeFromDB failed.");
		}
	}
	else
	{
		LOG4CXX_ERROR(g_logger, "CSqlOpt::GetUserDevicesInfoFromDB error.");
		b_ok = FAILED;
	}

	return b_ok;
}

int CSqlOpt::GetVideoLinkTypeFromDB(const std::string& user_name, const std::string& device_guid, int& video_link_type, std::string& video_username, std::string& video_password, std::string& video_ip, int& video_port)
{
	if (user_name.empty() || device_guid.empty())
	{
		LOG4CXX_ERROR(g_logger, "CSqlOpt::GetVideoLinkTypeFromDB arg error.");
		return FAILED;
	}

	sql::Connection* con;
	con = g_OptDeviceDataDB.GetConnection();
	if (con == NULL)
	{
		LOG4CXX_ERROR(g_logger, "CSqlOpt::GetVideoLinkTypeFromDB:GetConnection = NULL");
		return MY_SQL_ERROR;
	}

	int b_ok = SUCCESS;
	sql::PreparedStatement* prep_stmt = NULL;
	sql::ResultSet* res = NULL;

	try 
	{
		prep_stmt = con->prepareStatement("SELECT video_link_type,video_username,video_password,video_ip,video_port FROM tb_user_relation_device WHERE user_name=? AND device_guid=?");
		prep_stmt->setString(1, user_name);
		prep_stmt->setString(2, device_guid);
		res = prep_stmt->executeQuery();

		if (res->next())
		{
			video_link_type = res->getInt("video_link_type");
			video_username = res->getString("video_username");
			video_password = res->getString("video_password");
			video_ip = res->getString("video_ip");
			video_port = res->getInt("video_port");
		}

		SAFE_CLOSE2(prep_stmt, res);
	}
	catch (sql::SQLException& e)
	{
		LOG4CXX_ERROR(g_logger, "CSqlOpt::GetVideoLinkTypeFromDB:optDataDB = " << e.what());
		b_ok = MY_SQL_ERROR;
	}

	SAFE_CLOSE2(res, prep_stmt);
	g_OptDeviceDataDB.ReleaseConnection(con);

	return b_ok;
}

int CSqlOpt::ModifyDeviceInfoVideoLinkToDB(const std::string& user_name, const std::string& device_guid, const int video_link_type, const std::string& video_username, const std::string& video_password, const std::string& video_ip, const int video_port)
{
	if (user_name.empty() || device_guid.empty())
	{
		LOG4CXX_ERROR(g_logger, "CSqlOpt::ModifyDeviceInfoVideoLinkToDB arg error.");
		return FAILED;
	}

	sql::Connection* con;
	con = g_OptDeviceDataDB.GetConnection();
	if (con == NULL)
	{
		LOG4CXX_ERROR(g_logger, "CSqlOpt::ModifyDeviceInfoVideoLinkToDB:GetConnection = NULL.");
		return MY_SQL_ERROR;
	}

	int b_ok = SUCCESS;
	sql::PreparedStatement* prep_stmt = NULL;
	sql::ResultSet* res = NULL;

	try
	{
		int device_type = 0;

		prep_stmt = con->prepareStatement("SELECT device_type FROM tb_device_basic WHERE device_guid=?");
		prep_stmt->setString(1, device_guid);
		res = prep_stmt->executeQuery();
		if (res->next())
		{
			device_type = res->getInt("device_type");
		}

		SAFE_CLOSE2(prep_stmt, res);
		
		prep_stmt = con->prepareStatement("UPDATE tb_user_relation_device SET video_link_type=?, video_username=?, video_password=?, video_ip=?, video_port=? WHERE user_name=? AND device_guid=?");
		prep_stmt->setInt(1, video_link_type);
		prep_stmt->setString(2, video_username);
		prep_stmt->setString(3, video_password);
		prep_stmt->setString(4, video_ip);
		prep_stmt->setInt(5, video_port);
		prep_stmt->setString(6, user_name);
		prep_stmt->setString(7, device_guid);
		prep_stmt->executeUpdate();
	}
	catch (sql::SQLException& e)
	{
		LOG4CXX_ERROR(g_logger, "CSqlOpt::ModifyDeviceInfoVideoLinkToDB = " << e.what());
		b_ok = MY_SQL_ERROR;
	}
	
	SAFE_CLOSE(prep_stmt);
	g_OptDeviceDataDB.ReleaseConnection(con);

	return b_ok;
}

int CSqlOpt::UserBindDeviceToDB(const std::string& username, const std::string& device_guid, const std::string& video_username, const std::string& video_password)
{
	if (username.empty() || device_guid.empty())
	{
		LOG4CXX_ERROR(g_logger, "CSqlOpt::UserBindDeviceToDB arg error.");
		return FAILED;
	}

	sql::Connection* con;
	con = g_OptDeviceDataDB.GetConnection();
	if (con == NULL)
	{
		LOG4CXX_ERROR(g_logger, "CSqlOpt::UserBindDeviceToDB:GetConnection = NULL.");
		return MY_SQL_ERROR;
	}

	int b_ok = SUCCESS;
	sql::PreparedStatement* prep_stmt = NULL;
	sql::ResultSet* res = NULL;

	try
	{
		con->setAutoCommit(false);

		int device_type = 0;
		std::string device_name;

		prep_stmt = con->prepareStatement("SELECT device_type,device_name FROM tb_device_basic WHERE device_guid=?");
		prep_stmt->setString(1, device_guid);
		res = prep_stmt->executeQuery();
		if (res->next())
		{
			device_type = res->getInt("device_type");
			device_name = res->getString("device_name");
		}
		else
		{
			/*
			b_ok = DEVICE_NOT_EXIST;
			goto OPT_CONN;
			*/
			SAFE_CLOSE2(prep_stmt, res);
			prep_stmt = con->prepareStatement("INSERT INTO tb_device_basic (device_guid,device_name,device_type) VALUES (?,?,?)");
			prep_stmt->setString(1, device_guid);
			prep_stmt->setString(2, device_guid);
			prep_stmt->setInt(3, device_type);
			prep_stmt->execute();
		}
		SAFE_CLOSE2(prep_stmt, res);

		prep_stmt = con->prepareStatement("INSERT INTO tb_user_relation_device (user_name,device_guid,device_type,device_name,video_username,video_password) VALUES (?,?,?,?,?,?)");
		prep_stmt->setString(1, username);
		prep_stmt->setString(2, device_guid);
		prep_stmt->setInt(3, device_type);
		prep_stmt->setString(4, device_guid);
		prep_stmt->setString(5, video_username);
		prep_stmt->setString(6, video_password);
		prep_stmt->execute();
		SAFE_CLOSE(prep_stmt);

		con->commit();
	}
	catch (sql::SQLException& e)
	{
		con->rollback();
		LOG4CXX_ERROR(g_logger, "CSqlOpt::ModifyDeviceInfoVideoLinkToDB = " << e.what());
		b_ok = MY_SQL_ERROR;
	}

//OPT_CONN:
	SAFE_CLOSE2(prep_stmt, res);
	con->setAutoCommit(true);
	g_OptDeviceDataDB.ReleaseConnection(con);

	return b_ok;
}

int CSqlOpt::UserRemoveBindToDB(const std::string& username, const std::string& device_guid)
{
	if (username.empty() || device_guid.empty())
	{
		LOG4CXX_ERROR(g_logger, "CSqlOpt::UserRemoveBindToDB arg error.");
		return FAILED;
	}

	sql::Connection* con;
	con = g_OptDeviceDataDB.GetConnection();
	if (con == NULL)
	{
		LOG4CXX_ERROR(g_logger, "CSqlOpt::UserRemoveBindToDB:GetConnection = NULL.");
		return MY_SQL_ERROR;
	}

	int b_ok = SUCCESS;
	sql::PreparedStatement* prep_stmt = NULL;

	try 
	{
		con->setAutoCommit(false);

		prep_stmt = con->prepareStatement("DELETE FROM tb_user_relation_device WHERE user_name=? AND device_guid=?");
		prep_stmt->setString(1, username);
		prep_stmt->setString(2, device_guid);
		prep_stmt->execute();
		SAFE_CLOSE(prep_stmt);

		prep_stmt = con->prepareStatement("DELETE FROM tb_user_relation_channel WHERE user_name=? AND device_guid=?");
		prep_stmt->setString(1, username);
		prep_stmt->setString(2, device_guid);
		prep_stmt->execute();
		SAFE_CLOSE(prep_stmt);

		con->commit();
	}
	catch (sql::SQLException& e)
	{
		con->setAutoCommit(true);
		LOG4CXX_ERROR(g_logger, "CSqlOpt::UserRemoveBindToDB exception = " << e.what());
		b_ok = MY_SQL_ERROR;
		goto OPT_CONN;
	}

OPT_CONN:
	SAFE_CLOSE(prep_stmt);
	con->setAutoCommit(true);
	g_OptDeviceDataDB.ReleaseConnection(con);

	return b_ok;
}

int CSqlOpt::DeviceHumitureStatToDB(const std::vector<HUMITURE_STAT_INFO>& vec_humiture_stat_info)
{
	sql::Connection* con;
	con = g_OptDeviceDataDB.GetConnection();
	if (con == NULL)
	{
		LOG4CXX_ERROR(g_logger, "CSqlOpt::DeviceHumitureStatToDB:GetConnection = NULL.");
		return MY_SQL_ERROR;
	}

	int b_ok = SUCCESS;
	sql::PreparedStatement* prep_stmt = NULL;

	try 
	{
		for (unsigned i = 0; i < vec_humiture_stat_info.size(); ++i)
		{
			prep_stmt = con->prepareStatement("INSERT INTO tb_device_humiture (device_guid,date,hour,temperature,humidity,score,assessment) VALUES (?,?,?,?,?,?,?)");
			prep_stmt->setString(1, vec_humiture_stat_info.at(i).device_guid);
			prep_stmt->setString(2, vec_humiture_stat_info.at(i).date);
			prep_stmt->setInt(3, vec_humiture_stat_info.at(i).hour);
			prep_stmt->setDouble(4, vec_humiture_stat_info.at(i).statTemperature);
			prep_stmt->setDouble(5, vec_humiture_stat_info.at(i).statHumidity);
			prep_stmt->setInt(6, vec_humiture_stat_info.at(i).score);
			prep_stmt->setInt(7, vec_humiture_stat_info.at(i).assessment);
			prep_stmt->execute();
			SAFE_CLOSE(prep_stmt);
		}
	}
	catch (sql::SQLException& e)
	{
		LOG4CXX_ERROR(g_logger, "CSqlOpt::DeviceHumitureStatToDB exception = " << e.what());
		b_ok = MY_SQL_ERROR;
		goto OPT_CONN;
	}

OPT_CONN:
	SAFE_CLOSE(prep_stmt);
	g_OptDeviceDataDB.ReleaseConnection(con);

	return b_ok;
}

int CSqlOpt::GetDeviceHumitureFromDB(const std::string& device_guid, const int& getNum, std::vector<HUMITURE_STAT_INFO>& vec_humiture_stat_info)
{
	if (device_guid.empty())
	{
		LOG4CXX_ERROR(g_logger, "CSqlOpt::GetDeviceHumitureFromDB arg error.");
		return FAILED;
	}

	sql::Connection* con;
	con = g_OptDeviceDataDB.GetConnection();
	if (con == NULL)
	{
		LOG4CXX_ERROR(g_logger, "CSqlOpt::GetDeviceHumitureFromDB:optDataDB.GetConnection = NULL");
		return MY_SQL_ERROR;
	}

	int b_ok = SUCCESS;
	sql::PreparedStatement* prep_stmt = NULL;
	sql::ResultSet* res = NULL;

	try
	{
		std::string strSql = "SELECT date,hour,temperature,humidity,score,assessment FROM tb_device_humiture WHERE device_guid=? ORDER BY date DESC,hour DESC";
		if (getNum > 0)
		{
			char strNum[8];
			snprintf(strNum, sizeof(strNum)-1, "%d", getNum);
			strSql += std::string(" LIMIT ") + std::string(strNum);
		}
		prep_stmt = con->prepareStatement(strSql);
		prep_stmt->setString(1, device_guid);

		res = prep_stmt->executeQuery();
		
		HUMITURE_STAT_INFO humiture_stat_info;
		while (res->next())
		{
			std::string date = res->getString("date");
			int hour = res->getInt("hour");
			double tem = res->getDouble("temperature");
			double hum = res->getDouble("humidity");
			int score = res->getInt("score");
			int assessment = res->getInt("assessment");

			humiture_stat_info.device_guid = device_guid;
			humiture_stat_info.date = date;
			humiture_stat_info.hour = hour;
			humiture_stat_info.statTemperature = tem;
			humiture_stat_info.statHumidity = hum;
			humiture_stat_info.score = score;
			humiture_stat_info.assessment = assessment;
			vec_humiture_stat_info.push_back(humiture_stat_info);
		}

		SAFE_CLOSE2(prep_stmt,res);
	}
	catch (sql::SQLException& e)
	{
		LOG4CXX_ERROR(g_logger, "CSqlOpt::GetUserDevicesInfoFromDB:optDataDB = " << e.what());
		b_ok = MY_SQL_ERROR;
	}

	SAFE_CLOSE2(res, prep_stmt);
	g_OptDeviceDataDB.ReleaseConnection(con);

	return b_ok;
}

int CSqlOpt::GetUserDevicesStatusInfoFromDB(const std::string& username, std::vector<std::string>& vec_device, USER_DEVICES_LIST& user_devices_list)
{
	if (username.empty())
	{
		LOG4CXX_ERROR(g_logger, "CSqlOpt::GetUserDevicesStatusInfoFromDB arg error.");
		return FAILED;
	}

	sql::Connection* con;
	con = g_OptDeviceDataDB.GetConnection();
	if (con == NULL)
	{
		LOG4CXX_ERROR(g_logger, "CSqlOpt::GetUserDevicesStatusInfoFromDB:optDataDB.GetConnection = NULL");
		return MY_SQL_ERROR;
	}

	int b_ok = SUCCESS;
	sql::PreparedStatement* prep_stmt = NULL;
	sql::ResultSet* res = NULL;

	try
	{
		prep_stmt = con->prepareStatement("SELECT device_guid, device_type, device_name FROM tb_user_relation_device WHERE user_name=?");
		prep_stmt->setString(1, username);
		res = prep_stmt->executeQuery();
		while (res->next())
		{
			std::string guid = res->getString("device_guid");
			vec_device.push_back(guid);
		}

		SAFE_CLOSE2(prep_stmt, res);

		USER_DEVICE_INFO user_device_info;
		for (unsigned i = 0; i < vec_device.size(); ++i)
		{
			if (SUCCESS == GetDeviceInfoFromDB(vec_device.at(i), user_device_info.device_info))
			{
				if (SUCCESS == GetUserDeviceConfigFromDB(username, vec_device.at(i), user_device_info.user_device_config))
				{
					user_devices_list.vec_user_device_info.push_back(user_device_info);
				}
				else
				{
					LOG4CXX_TRACE(g_logger, "CSqlOpt::GetUserDevicesStatusInfoFromDB:GetUserDeviceConfigFromDB failed.");
				}
			}
			else
			{
				LOG4CXX_ERROR(g_logger, "CSqlOpt::GetUserDevicesStatusInfoFromDB error.");
			}
		}
	}
	catch (sql::SQLException& e)
	{
		LOG4CXX_ERROR(g_logger, "CSqlOpt::GetUserDevicesStatusInfoFromDB:optDataDB = " << e.what());
		b_ok = MY_SQL_ERROR;
	}

	SAFE_CLOSE2(res, prep_stmt);
	g_OptDeviceDataDB.ReleaseConnection(con);

	return b_ok;
}

int CSqlOpt::DeviceHumitureScoreToDB(const std::string& device_guid, const std::string& timestamp, const int score, const int top)
{
	if (device_guid.empty())
	{
		LOG4CXX_ERROR(g_logger, "CSqlOpt::DeviceHumitureScoreToDB arg error.");
		return FAILED;
	}

	sql::Connection* con;
	con = g_OptDeviceDataDB.GetConnection();
	if(con == NULL)
	{
		LOG4CXX_ERROR(g_logger, "CSqlOpt::DeviceHumitureScoreToDB:g_OptDeviceDataDB.GetConnection = NULL");
		return MY_SQL_ERROR;
	}

	int b_ok = SUCCESS;
	sql::PreparedStatement* prep_stmt = NULL;
	sql::ResultSet* res = NULL;
	
	std::string updateSql;
	bool isHadHisScore = false;
	try 
	{
		prep_stmt = con->prepareStatement("SELECT id FROM tb_device_humiture_score WHERE device_guid=? LIMIT 1");
		prep_stmt->setString(1, device_guid);
		res = prep_stmt->executeQuery();
		if (res->next())
		{
			updateSql = "UPDATE tb_device_humiture_score SET timestamp=?,score=?,top=? WHERE device_guid=?";
			isHadHisScore = true;
		}
		else
		{
			updateSql = "INSERT INTO tb_device_humiture_score (device_guid,timestamp,score,top) VALUES (?,?,?,?)";
			isHadHisScore = false;
		}

		SAFE_CLOSE2(res, prep_stmt);

		prep_stmt = con->prepareStatement(updateSql);
		if (isHadHisScore)
		{
			prep_stmt->setDateTime(1, timestamp);
			prep_stmt->setInt(2, score);
			prep_stmt->setInt(3, top);
			prep_stmt->setString(4, device_guid);
		}
		else
		{
			prep_stmt->setString(1, device_guid);
			prep_stmt->setDateTime(2, timestamp);
			prep_stmt->setInt(3, score);
			prep_stmt->setInt(4, top);
		}
		prep_stmt->execute();

		SAFE_CLOSE(prep_stmt);
	}
	catch (sql::SQLException& e)
	{
		LOG4CXX_ERROR(g_logger, "CSqlOpt::DeviceHumitureScoreToDB exception = " << e.what());
		b_ok = MY_SQL_ERROR;
		goto OPT_CONN;
	}

OPT_CONN:
	SAFE_CLOSE2(res, prep_stmt);
	g_OptDeviceDataDB.ReleaseConnection(con);

	return b_ok;
}

int CSqlOpt::GetLastHumitureScoreFromDB(const std::string& device_guid, int& score, int& top)
{
	if (device_guid.empty())
	{
		LOG4CXX_ERROR(g_logger, "CSqlOpt::GetLastHumitureScoreFromDB arg error.");
		return FAILED;
	}

	sql::Connection* con;
	con = g_OptDeviceDataDB.GetConnection();
	if (con == NULL)
	{
		LOG4CXX_ERROR(g_logger, "CSqlOpt::GetLastHumitureScoreFromDB:optDataDB.GetConnection = NULL");
		return MY_SQL_ERROR;
	}

	int b_ok = SUCCESS;
	sql::PreparedStatement* prep_stmt = NULL;
	sql::ResultSet* res = NULL;

	try
	{
		prep_stmt = con->prepareStatement("SELECT score,top FROM tb_device_humiture_score WHERE device_guid=? ORDER BY timestamp DESC LIMIT 1");
		prep_stmt->setString(1, device_guid);

		res = prep_stmt->executeQuery();

		if (res->next())
		{
			score = res->getInt("score");
			top = res->getInt("top");
		}
		else
		{
			b_ok = DEVICE_HIS_NOT_EXIST;
			LOG4CXX_WARN(g_logger, "CSqlOpt::GetLastHumitureScoreFromDB select null.Didn't has history score info.");
			goto OPT_CONN;
		}

		SAFE_CLOSE2(prep_stmt, res);
	}
	catch (sql::SQLException& e)
	{
		LOG4CXX_ERROR(g_logger, "CSqlOpt::GetLastHumitureScoreFromDB:optDataDB = " << e.what());
		b_ok = MY_SQL_ERROR;
	}

OPT_CONN:
	SAFE_CLOSE2(res, prep_stmt);
	g_OptDeviceDataDB.ReleaseConnection(con);

	return b_ok;
}

int CSqlOpt::ModifyDeviceConfInfoToDB(const std::string& username, const std::string& device_guid, const std::string& device_name, const std::string& video_username, const std::string& video_password)
{
	if (username.empty() || device_guid.empty())
	{
		LOG4CXX_ERROR(g_logger, "CSqlOpt::ModifyDeviceConfInfoToDB arg error.");
		return FAILED;
	}

	sql::Connection* con;
	con = g_OptDeviceDataDB.GetConnection();
	if (con == NULL)
	{
		LOG4CXX_ERROR(g_logger, "CSqlOpt::ModifyDeviceConfInfoToDB:GetConnection = NULL.");
		return MY_SQL_ERROR;
	}

	int b_ok = SUCCESS;
	sql::PreparedStatement* prep_stmt = NULL;
	sql::ResultSet* res = NULL;

	try
	{
		int device_type = 0;

		prep_stmt = con->prepareStatement("SELECT device_type FROM tb_device_basic WHERE device_guid=?");
		prep_stmt->setString(1, device_guid);
		res = prep_stmt->executeQuery();
		if (res->next())
		{
			device_type = res->getInt("device_type");
		}
		SAFE_CLOSE2(prep_stmt, res);

		prep_stmt = con->prepareStatement("UPDATE tb_user_relation_device SET device_name=?,video_username=?,video_password=? WHERE device_guid=? AND user_name=?");
		prep_stmt->setString(1, device_name);
		prep_stmt->setString(2, video_username);
		prep_stmt->setString(3, video_password);
		prep_stmt->setString(4, device_guid);
		prep_stmt->setString(5, username);
		prep_stmt->executeUpdate();
	}
	catch (sql::SQLException& e)
	{
		LOG4CXX_ERROR(g_logger, "CSqlOpt::ModifyDeviceConfInfoToDB = " << e.what());
		b_ok = MY_SQL_ERROR;
	}

	SAFE_CLOSE(prep_stmt);
	g_OptDeviceDataDB.ReleaseConnection(con);

	return b_ok;
}

int CSqlOpt::ModifyDeviceInfoAdvancedToDB(const std::string& device_guid, const int& net_storage_switch, const int& tf_storage_switch, const int& alarm_switch, const std::string& alarm_time, const int& baby_mode, const int& full_alarm_mode)
{
	if (device_guid.empty())
	{
		LOG4CXX_ERROR(g_logger, "CSqlOpt::ModifyDeviceInfoAdvancedToDB arg error.");
		return FAILED;
	}

	sql::Connection* con;
	con = g_OptDeviceDataDB.GetConnection();
	if (con == NULL)
	{
		LOG4CXX_ERROR(g_logger, "CSqlOpt::ModifyDeviceInfoAdvancedToDB:GetConnection = NULL.");
		return MY_SQL_ERROR;
	}

	int b_ok = SUCCESS;
	sql::PreparedStatement* prep_stmt = NULL;
	sql::ResultSet* res = NULL;

	try
	{
		con->setAutoCommit(false);

		int device_type = 0;

		prep_stmt = con->prepareStatement("SELECT device_type FROM tb_device_basic WHERE device_guid=?");
		prep_stmt->setString(1, device_guid);
		res = prep_stmt->executeQuery();
		if (res->next())
		{
			device_type = res->getInt("device_type");
		}
		SAFE_CLOSE2(prep_stmt, res);

		switch (device_type)
		{
		case DEV_TYPE_HOME_IPC:
			if (net_storage_switch != DEVICE_CONF_NOT_SET)
			{
				prep_stmt = con->prepareStatement("UPDATE tb_device_IPC SET net_storage_switch=? WHERE device_guid=?");
				prep_stmt->setInt(1, net_storage_switch);
				prep_stmt->setString(2, device_guid);
				prep_stmt->executeUpdate();
				SAFE_CLOSE(prep_stmt);
			}
			if (tf_storage_switch != DEVICE_CONF_NOT_SET)
			{
				prep_stmt = con->prepareStatement("UPDATE tb_device_IPC SET tf_storage_switch=? WHERE device_guid=?");
				prep_stmt->setInt(1, tf_storage_switch);
				prep_stmt->setString(2, device_guid);
				prep_stmt->executeUpdate();
				SAFE_CLOSE(prep_stmt);
			}
			if (alarm_switch != DEVICE_CONF_NOT_SET)
			{
				prep_stmt = con->prepareStatement("UPDATE tb_device_IPC SET alarm_switch=? WHERE device_guid=?");
				prep_stmt->setInt(1, alarm_switch);
				prep_stmt->setString(2, device_guid);
				prep_stmt->executeUpdate();
				SAFE_CLOSE(prep_stmt);
			}
			if (!alarm_time.empty())
			{
				prep_stmt = con->prepareStatement("UPDATE tb_device_IPC SET alarm_time=? WHERE device_guid=?");
				prep_stmt->setString(1, alarm_time);
				prep_stmt->setString(2, device_guid);
				prep_stmt->executeUpdate();
				SAFE_CLOSE(prep_stmt);
			}
			if (baby_mode != DEVICE_CONF_NOT_SET)
			{
				prep_stmt = con->prepareStatement("UPDATE tb_device_IPC SET baby_mode=? WHERE device_guid=?");
				prep_stmt->setInt(1, baby_mode);
				prep_stmt->setString(2, device_guid);
				prep_stmt->executeUpdate();
				SAFE_CLOSE(prep_stmt);
			}
			if (full_alarm_mode != DEVICE_CONF_NOT_SET)
			{
				prep_stmt = con->prepareStatement("UPDATE tb_device_IPC SET full_alarm_mode=? WHERE device_guid=?");
				prep_stmt->setInt(1, full_alarm_mode);
				prep_stmt->setString(2, device_guid);
				prep_stmt->executeUpdate();
				SAFE_CLOSE(prep_stmt);
			}
			
			break;

		default:
			break;
		}

		con->commit();
	}
	catch (sql::SQLException& e)
	{
		con->rollback();
		LOG4CXX_ERROR(g_logger, "CSqlOpt::ModifyDeviceConfInfoToDB = " << e.what());
		b_ok = MY_SQL_ERROR;
	}

	SAFE_CLOSE(prep_stmt);
	con->setAutoCommit(true);
	g_OptDeviceDataDB.ReleaseConnection(con);

	return b_ok;
}

int CSqlOpt::GetUpdateInfo(const int device_type, const int device_sub_type, const std::string& client_version, UPDATE_FILE_INFO& updateFileInfo)
{
	sql::Connection* con;
	con = g_OptDeviceDataDB.GetConnection();
	if (con == NULL)
	{
		LOG4CXX_ERROR(g_logger, "CSqlOpt::GetUpdateInfo.GetConnection = NULL");
		return MY_SQL_ERROR;
	}

	int b_ok = SQL_NOT_FIND;
	sql::PreparedStatement* prep_stmt = NULL;
	sql::ResultSet* res = NULL;

	try
	{
		prep_stmt = con->prepareStatement("SELECT update_version,file_url,file_size,file_checksum,file_discription FROM tb_device_update WHERE device_type=? AND device_sub_type=? AND client_version=?");
		prep_stmt->setInt(1, device_type);
		prep_stmt->setInt(2, device_sub_type);
		prep_stmt->setString(3, client_version);
		res = prep_stmt->executeQuery();
		if (res->next())
		{
			b_ok = SUCCESS;
			updateFileInfo.update_version = res->getString("update_version");
			updateFileInfo.file_url= res->getString("file_url");
			updateFileInfo.file_size = res->getInt("file_size");
			updateFileInfo.file_checksum = res->getString("file_checksum");
			updateFileInfo.file_description = res->getString("file_discription");
		}
		else
		{
			b_ok = DEVICE_HAS_NO_UPDATE;
		}
	}
	catch (sql::SQLException& e)
	{
		LOG4CXX_ERROR(g_logger, "CSqlOpt::GetUpdateInfo = " << e.what());
		b_ok = MY_SQL_ERROR;
	}

	SAFE_CLOSE2(res, prep_stmt);
	g_OptDeviceDataDB.ReleaseConnection(con);

	return b_ok;
}

int CSqlOpt::ModifyDevicePasswordToDB(const std::string& username, const std::string& device_guid, const std::string& device_username, const std::string& device_password)
{
	if (username.empty() || device_guid.empty())
	{
		LOG4CXX_ERROR(g_logger, "CSqlOpt::ModifyDevicePasswordToDB arg error.");
		return FAILED;
	}

	sql::Connection* con;
	con = g_OptDeviceDataDB.GetConnection();
	if (con == NULL)
	{
		LOG4CXX_ERROR(g_logger, "CSqlOpt::ModifyDevicePasswordToDB:GetConnection = NULL.");
		return MY_SQL_ERROR;
	}

	int b_ok = SUCCESS;
	sql::PreparedStatement* prep_stmt = NULL;
	sql::ResultSet* res = NULL;

	try
	{
		con->setAutoCommit(false);

		int device_type = 0;
		int reset_flag = 0;

		prep_stmt = con->prepareStatement("SELECT device_type,reset_flag FROM tb_device_basic WHERE device_guid=?");
		prep_stmt->setString(1, device_guid);
		res = prep_stmt->executeQuery();
		if (res->next())
		{
			device_type = res->getInt("device_type");
			reset_flag = res->getInt("reset_flag");
		}
		if (reset_flag == 0)
		{
			b_ok = DEVICE_UPDATE_ERROR;
			goto OPT_CONN;
		}

		SAFE_CLOSE2(prep_stmt, res);

		switch (device_type)
		{
		case DEV_TYPE_HOME_IPC:
			prep_stmt = con->prepareStatement("UPDATE tb_device_IPC SET device_username=?, device_password=? WHERE device_guid=?");
			prep_stmt->setString(1, device_username);
			prep_stmt->setString(2, device_password);
			prep_stmt->setString(3, device_guid);
			prep_stmt->executeUpdate();
			SAFE_CLOSE(prep_stmt);

			prep_stmt = con->prepareStatement("UPDATE tb_user_relation_device SET video_username=?, video_password=? WHERE user_name=? AND device_guid=?");
			prep_stmt->setString(1, device_username);
			prep_stmt->setString(2, device_password);
			prep_stmt->setString(3, username);
			prep_stmt->setString(4, device_guid);
			prep_stmt->executeUpdate();
			SAFE_CLOSE(prep_stmt);

			prep_stmt = con->prepareStatement("UPDATE tb_device_basic SET reset_flag=0 WHERE device_guid=?");
			prep_stmt->setString(1, device_guid);
			prep_stmt->executeUpdate();
			SAFE_CLOSE(prep_stmt);
			break;

		default:
			break;
		}

		con->commit();
	}
	catch (sql::SQLException& e)
	{
		con->rollback();
		LOG4CXX_ERROR(g_logger, "CSqlOpt::ModifyDevicePasswordToDB = " << e.what());
		b_ok = MY_SQL_ERROR;
	}

OPT_CONN:
	SAFE_CLOSE(prep_stmt);
	con->setAutoCommit(true);
	g_OptDeviceDataDB.ReleaseConnection(con);

	return b_ok;
}

int CSqlOpt::GetDeviceResetFlagFromDB(const std::string& device_guid, int& reset_flag)
{
	sql::Connection* con;
	con = g_OptDeviceDataDB.GetConnection();
	if (con == NULL)
	{
		LOG4CXX_ERROR(g_logger, "CSqlOpt::GetDeviceResetFlagFromDB.GetConnection = NULL");
		return MY_SQL_ERROR;
	}

	int b_ok = SQL_NOT_FIND;
	sql::PreparedStatement* prep_stmt = NULL;
	sql::ResultSet* res = NULL;

	try
	{
		prep_stmt = con->prepareStatement("SELECT reset_flag FROM tb_device_basic WHERE device_guid=?");
		prep_stmt->setString(1, device_guid);
		res = prep_stmt->executeQuery();
		if (res->next())
		{
			b_ok = SUCCESS;
			reset_flag = res->getInt("reset_flag");
		}
		else
		{
			b_ok = DEVICE_NOT_EXIST;
		}
	}
	catch (sql::SQLException& e)
	{
		LOG4CXX_ERROR(g_logger, "CSqlOpt::GetDeviceResetFlagFromDB = " << e.what());
		b_ok = MY_SQL_ERROR;
	}

	SAFE_CLOSE2(res, prep_stmt);
	g_OptDeviceDataDB.ReleaseConnection(con);

	return b_ok;
}

int CSqlOpt::RemoveBindByDeviceToDB(const std::string& device_guid)
{
	sql::Connection* con;
	con = g_OptDeviceDataDB.GetConnection();
	if (con == NULL)
	{
		LOG4CXX_ERROR(g_logger, "CSqlOpt::RemoveBindByDeviceToDB.GetConnection = NULL");
		return MY_SQL_ERROR;
	}

	int b_ok = SUCCESS;
	sql::PreparedStatement* prep_stmt = NULL;

	try
	{
		con->setAutoCommit(false);

		prep_stmt = con->prepareStatement("DELETE FROM tb_user_relation_device WHERE device_guid=?");
		prep_stmt->setString(1, device_guid);
		prep_stmt->executeUpdate();

		prep_stmt = con->prepareStatement("DELETE FROM tb_user_relation_channel WHERE device_guid=?");
		prep_stmt->setString(1, device_guid);
		prep_stmt->executeUpdate();
		SAFE_CLOSE(prep_stmt);

		con->commit();
	}
	catch (sql::SQLException& e)
	{
		con->rollback();
		LOG4CXX_ERROR(g_logger, "CSqlOpt::GetDeviceResetFlagFromDB = " << e.what());
		b_ok = MY_SQL_ERROR;
	}

	SAFE_CLOSE(prep_stmt);
	con->setAutoCommit(true);
	g_OptDeviceDataDB.ReleaseConnection(con);

	return b_ok;
}

int CSqlOpt::RemoveBindByDeviceToDB_HomeCloud(const std::string& device_guid)
{
	sql::Connection* con;
	con = g_OptDeviceDataDB.GetConnection();
	if (con == NULL)
	{
		LOG4CXX_ERROR(g_logger, "CSqlOpt::RemoveBindByDeviceToDB_HomeCloud.GetConnection = NULL");
		return MY_SQL_ERROR;
	}

	int b_ok = SUCCESS;
	sql::PreparedStatement* prep_stmt = NULL;

	try
	{
		con->setAutoCommit(false);

		prep_stmt = con->prepareStatement("DELETE FROM tb_user_relation_device_homecloud WHERE device_guid=?");
		prep_stmt->setString(1, device_guid);
		prep_stmt->executeUpdate();
		SAFE_CLOSE(prep_stmt);

		/*
		// 整合后要删除通道
		prep_stmt = con->prepareStatement("DELETE FROM tb_user_relation_channel WHERE device_guid=?");
		prep_stmt->setString(1, device_guid);
		prep_stmt->executeUpdate();
		SAFE_CLOSE(prep_stmt);
		*/

		con->commit();
	}
	catch (sql::SQLException& e)
	{
		con->rollback();
		LOG4CXX_ERROR(g_logger, "CSqlOpt::GetDeviceResetFlagFromDB = " << e.what());
		b_ok = MY_SQL_ERROR;
	}

	SAFE_CLOSE(prep_stmt);
	con->setAutoCommit(true);
	g_OptDeviceDataDB.ReleaseConnection(con);

	return b_ok;
}

int CSqlOpt::RemoveBindByDeviceHoldUserToDB_HomeCloud(const std::string& device_guid, const std::string& hold_user)
{
sql::Connection* con;
	con = g_OptDeviceDataDB.GetConnection();
	if (con == NULL)
	{
		LOG4CXX_ERROR(g_logger, "CSqlOpt::RemoveBindByDeviceHoldUserToDB_HomeCloud.GetConnection = NULL");
		return MY_SQL_ERROR;
	}

	int b_ok = SUCCESS;
	sql::PreparedStatement* prep_stmt = NULL;

	try
	{
		con->setAutoCommit(false);

		prep_stmt = con->prepareStatement("DELETE FROM tb_user_relation_device_homecloud WHERE device_guid=? AND user_name != ?");
		prep_stmt->setString(1, device_guid);
		prep_stmt->setString(2, hold_user);
		prep_stmt->executeUpdate();
		SAFE_CLOSE(prep_stmt);

		/*
		// 整合后要删除通道
		prep_stmt = con->prepareStatement("DELETE FROM tb_user_relation_channel WHERE device_guid=? AND user_name != ?");
		prep_stmt->setString(1, device_guid);
		prep_stmt->setString(2, hold_user);
		prep_stmt->executeUpdate();
		SAFE_CLOSE(prep_stmt);
		*/

		con->commit();
	}
	catch (sql::SQLException& e)
	{
		con->rollback();
		LOG4CXX_ERROR(g_logger, "CSqlOpt::RemoveBindByDeviceHoldUserToDB = " << e.what());
		b_ok = MY_SQL_ERROR;
	}

	SAFE_CLOSE(prep_stmt);
	con->setAutoCommit(true);
	g_OptDeviceDataDB.ReleaseConnection(con);

	return b_ok;
}

int CSqlOpt::GetDevicePasswordFromDB(const std::string& device_guid, std::string& device_username, std::string& device_password)
{
	sql::Connection* con;
	con = g_OptDeviceDataDB.GetConnection();
	if (con == NULL)
	{
		LOG4CXX_ERROR(g_logger, "CSqlOpt::GetDevicePasswordFromDB.GetConnection = NULL");
		return MY_SQL_ERROR;
	}

	int b_ok = SUCCESS;
	sql::PreparedStatement* prep_stmt = NULL;
	sql::ResultSet* res = NULL;

	try
	{
		prep_stmt = con->prepareStatement("SELECT device_username,device_password FROM tb_device_IPC WHERE device_guid=?");
		prep_stmt->setString(1, device_guid);
		res = prep_stmt->executeQuery();
		if (res->next())
		{
			device_username = res->getString("device_username");
			device_password = res->getString("device_password");
		}
		else
		{
			b_ok = DEVICE_NOT_EXIST;
		}
		SAFE_CLOSE2(res, prep_stmt);
	}
	catch (sql::SQLException& e)
	{
		LOG4CXX_ERROR(g_logger, "CSqlOpt::GetDevicePasswordFromDB = " << e.what());
		b_ok = MY_SQL_ERROR;
	}

	SAFE_CLOSE2(res, prep_stmt);
	g_OptDeviceDataDB.ReleaseConnection(con);

	return b_ok;
}

int CSqlOpt::GetUserDevicePasswordFromDB(const std::string& username, const std::string& device_guid, std::string& video_username, std::string& video_password)
{
	sql::Connection* con;
	con = g_OptDeviceDataDB.GetConnection();
	if (con == NULL)
	{
		LOG4CXX_ERROR(g_logger, "CSqlOpt::GetUserDevicePasswordFromDB.GetConnection = NULL");
		return MY_SQL_ERROR;
	}

	int b_ok = SUCCESS;
	sql::PreparedStatement* prep_stmt = NULL;
	sql::ResultSet* res = NULL;

	try
	{
		prep_stmt = con->prepareStatement("SELECT video_username,video_password FROM tb_user_relation_device WHERE user_name=? AND device_guid=?");
		prep_stmt->setString(1, username);
		prep_stmt->setString(2, device_guid);
		res = prep_stmt->executeQuery();
		if (res->next())
		{
			video_username = res->getString("video_username");
			video_password = res->getString("video_password");
		}
		else
		{
			b_ok = DEVICE_NOT_BIND;
		}
		SAFE_CLOSE2(res, prep_stmt);
	}
	catch (sql::SQLException& e)
	{
		LOG4CXX_ERROR(g_logger, "CSqlOpt::GetUserDevicePasswordFromDB = " << e.what());
		b_ok = MY_SQL_ERROR;
	}

	SAFE_CLOSE2(res, prep_stmt);
	g_OptDeviceDataDB.ReleaseConnection(con);

	return b_ok;
}

int CSqlOpt::VerifyDeviceExistFromDB(const std::string& device_guid)
{
	if (device_guid.empty())
	{
		LOG4CXX_ERROR(g_logger, "CSqlOpt::VerifyDeviceExistFromDB arg error.");
		return FAILED;
	}

	sql::Connection* con;
	con = g_OptDeviceDataDB.GetConnection();
	if (con == NULL)
	{
		LOG4CXX_ERROR(g_logger, "CSqlOpt::VerifyDeviceExistFromDB:GetConnection = NULL.");
		return MY_SQL_ERROR;
	}

	int b_ok = SUCCESS;
	sql::PreparedStatement* prep_stmt = NULL;
	sql::ResultSet* res = NULL;

	try
	{
		std::string device_name;

		prep_stmt = con->prepareStatement("SELECT device_name FROM tb_device_basic WHERE device_guid=?");
		prep_stmt->setString(1, device_guid);
		res = prep_stmt->executeQuery();
		if (res->next())
		{
			device_name = res->getString("device_name");
		}
		else
		{
			b_ok = DEVICE_NOT_EXIST;
			goto OPT_CONN;
		}

		SAFE_CLOSE2(prep_stmt, res);
	}
	catch (sql::SQLException& e)
	{
		LOG4CXX_ERROR(g_logger, "CSqlOpt::VerifyDeviceExistFromDB = " << e.what());
		b_ok = MY_SQL_ERROR;
	}

OPT_CONN:
	SAFE_CLOSE2(prep_stmt, res);
	g_OptDeviceDataDB.ReleaseConnection(con);

	return b_ok;
}

int CSqlOpt::GetUserDeviceConfigFromDB(const std::string& user_name, const std::string& device_guid, USER_DEVICE_CONFIG& user_device_config)
{
	if (user_name.empty() || device_guid.empty())
	{
		LOG4CXX_ERROR(g_logger, "CSqlOpt::GetUserDeviceConfigFromDB arg error.");
		return FAILED;
	}

	sql::Connection* con;
	con = g_OptDeviceDataDB.GetConnection();
	if (con == NULL)
	{
		LOG4CXX_ERROR(g_logger, "CSqlOpt::GetUserDeviceConfigFromDB:GetConnection = NULL");
		return MY_SQL_ERROR;
	}

	int b_ok = SUCCESS;
	sql::PreparedStatement* prep_stmt = NULL;
	sql::ResultSet* res = NULL;

	try 
	{
		prep_stmt = con->prepareStatement("SELECT device_name,video_link_type,video_username,video_password,video_ip,video_port FROM tb_user_relation_device WHERE user_name=? AND device_guid=?");
		prep_stmt->setString(1, user_name);
		prep_stmt->setString(2, device_guid);
		res = prep_stmt->executeQuery();

		if (res->next())
		{
			user_device_config.device_name = res->getString("device_name");
			user_device_config.video_link_type = res->getInt("video_link_type");
			user_device_config.video_username = res->getString("video_username");
			user_device_config.video_password = res->getString("video_password");
			user_device_config.video_ip = res->getString("video_ip");
			user_device_config.video_port = res->getInt("video_port");
		}
		else
		{
			LOG4CXX_ERROR(g_logger, "CSqlOpt::GetDeviceInfoFromDB:SELECT tb_device_basic null.");
			b_ok = SQL_NOT_FIND;
		}

		SAFE_CLOSE2(prep_stmt, res);
	}
	catch (sql::SQLException& e)
	{
		LOG4CXX_ERROR(g_logger, "CSqlOpt::GetUserDeviceConfigFromDB:optDataDB = " << e.what());
		b_ok = MY_SQL_ERROR;
	}

	SAFE_CLOSE2(res, prep_stmt);
	g_OptDeviceDataDB.ReleaseConnection(con);

	return b_ok;
}

int CSqlOpt::AddDeviceChannelToDB(const std::string& username, const std::string& device_guid, const int add_sum, const std::string& channel_name)
{
	if (username.empty() || device_guid.empty())
	{
		LOG4CXX_ERROR(g_logger, "CSqlOpt::AddDeviceChannelToDB arg error.");
		return FAILED;
	}

	sql::Connection* con;
	con = g_OptDeviceDataDB.GetConnection();
	if (con == NULL)
	{
		LOG4CXX_ERROR(g_logger, "CSqlOpt::AddDeviceChannelToDB:GetConnection = NULL");
		return MY_SQL_ERROR;
	}

	int  b_ok = SUCCESS;
	sql::PreparedStatement* prep_stmt = NULL;
	sql::ResultSet* res = NULL;

	try 
	{
		prep_stmt = con->prepareStatement("SELECT device_guid FROM tb_device_basic WHERE device_guid=?");
		prep_stmt->setString(1, device_guid);
		res = prep_stmt->executeQuery();
		if (!res->next())
		{
			b_ok = DEVICE_NOT_EXIST;
			goto OPT_CONN;
		}
		SAFE_CLOSE2(prep_stmt, res);

		prep_stmt = con->prepareStatement("SELECT device_guid FROM tb_user_relation_device WHERE user_name=? AND device_guid=?");
		prep_stmt->setString(1, username);
		prep_stmt->setString(2, device_guid);
		res = prep_stmt->executeQuery();
		if (!res->next())
		{
			b_ok = DEVICE_NOT_BIND;
			goto OPT_CONN;
		}
		SAFE_CLOSE2(prep_stmt, res);

		prep_stmt = con->prepareStatement("SELECT channel_no FROM tb_user_relation_channel WHERE user_name=? AND device_guid=? ORDER BY channel_no ASC");
		prep_stmt->setString(1, username);
		prep_stmt->setString(2, device_guid);
		res = prep_stmt->executeQuery();

		std::vector<int> channel_no_vec, channel_no_add_vec;
		int channel_no = 0;
		for (int i = 0; res->next(); )
		{
			channel_no = res->getInt("channel_no");
			if (channel_no > 64)
			{
				b_ok = DEVICE_CHANNEL_LIMIT;
				goto OPT_CONN;
			}
			channel_no_vec.push_back(channel_no);
			if (channel_no > i + 1)
			{
				int diff = channel_no - (i + 1);
				for (int j = 0; j < diff; ++j)
				{
					channel_no_add_vec.push_back(i+1+j);
				}
				i += diff + 1;
			}
			else
			{
				++i;
			}
		}
		SAFE_CLOSE2(prep_stmt, res);

		if (channel_no_vec.size() == (unsigned)channel_no)
		{
			for (int i = 0; i < add_sum; ++i)
			{
				channel_no_add_vec.push_back(++channel_no);
				if (channel_no > 64)
				{
					b_ok = DEVICE_CHANNEL_LIMIT;
					goto OPT_CONN;
				}
			}
		}
		else
		{
			if (channel_no_add_vec.size() < (unsigned)add_sum)
			{
				int diff = add_sum - channel_no_add_vec.size();
				for (int i = 0; i < diff; ++i)
				{
					channel_no_add_vec.push_back(++channel_no);
					if (channel_no > 64)
					{
						b_ok = DEVICE_CHANNEL_LIMIT;
						goto OPT_CONN;
					}
				}
			}
		}

		for (unsigned i = 0; i < channel_no_add_vec.size(); ++i)
		{
			prep_stmt = con->prepareStatement("INSERT INTO tb_user_relation_channel (user_name,device_guid,channel_no,channel_name) VALUES (?,?,?,?)");
			prep_stmt->setString(1, username);
			prep_stmt->setString(2, device_guid);
			channel_no = channel_no_add_vec.at(i);
			prep_stmt->setInt(3, channel_no);
			if (channel_name.empty())
			{
				prep_stmt->setString(4, device_guid + "_" + utils::int2str(channel_no));
			}
			else
			{
				prep_stmt->setString(4, channel_name);
			}
			prep_stmt->executeUpdate();
			SAFE_CLOSE(prep_stmt);
		}
		
	}
	catch (sql::SQLException& e)
	{
		LOG4CXX_ERROR(g_logger, "CSqlOpt::AddDeviceChannelToDB:optDataDB = " << e.what());
		b_ok = MY_SQL_ERROR;
	}


OPT_CONN:
	SAFE_CLOSE2(res, prep_stmt);
	g_OptDeviceDataDB.ReleaseConnection(con);

	return b_ok;
}

int CSqlOpt::DeleteDeviceChannelToDB(const std::string& username, const std::string& device_guid, const int channel_no)
{
	if (username.empty() || device_guid.empty())
	{
		LOG4CXX_ERROR(g_logger, "CSqlOpt::DeleteDeviceChannelToDB arg error.");
		return FAILED;
	}

	sql::Connection* con;
	con = g_OptDeviceDataDB.GetConnection();
	if (con == NULL)
	{
		LOG4CXX_ERROR(g_logger, "CSqlOpt::DeleteDeviceChannelToDB:GetConnection = NULL");
		return MY_SQL_ERROR;
	}

	int b_ok = SUCCESS;
	sql::PreparedStatement* prep_stmt = NULL;

	try
	{
		prep_stmt = con->prepareStatement("DELETE FROM tb_user_relation_channel WHERE user_name=? AND device_guid=? AND channel_no=?");
		prep_stmt->setString(1, username);
		prep_stmt->setString(2, device_guid);
		prep_stmt->setInt(3, channel_no);
		prep_stmt->executeUpdate();
		SAFE_CLOSE(prep_stmt);
	}
	catch (sql::SQLException& e)
	{
		LOG4CXX_ERROR(g_logger, "CSqlOpt::DeleteDeviceChannelToDB:optDataDB = " << e.what());
		b_ok = MY_SQL_ERROR;
	}

	g_OptDeviceDataDB.ReleaseConnection(con);

	return b_ok;
}

int CSqlOpt::GetDeviceChannelFromDB(const std::string& username, const std::string& device_guid, std::vector<DEVICE_CHANNEL_INFO>& vec_device_channel)
{
	if (username.empty() || device_guid.empty())
	{
		LOG4CXX_ERROR(g_logger, "CSqlOpt::GetDeviceChannelFromDB arg error.");
		return FAILED;
	}

	sql::Connection* con;
	con = g_OptDeviceDataDB.GetConnection();
	if (con == NULL)
	{
		LOG4CXX_ERROR(g_logger, "CSqlOpt::GetDeviceChannelFromDB:GetConnection = NULL");
		return MY_SQL_ERROR;
	}

	int b_ok = SUCCESS;
	sql::PreparedStatement* prep_stmt = NULL;
	sql::ResultSet* res = NULL;

	try 
	{
		prep_stmt = con->prepareStatement("SELECT channel_no,channel_name FROM tb_user_relation_channel WHERE user_name=? AND device_guid=?");
		prep_stmt->setString(1, username);
		prep_stmt->setString(2, device_guid);
		res = prep_stmt->executeQuery();
		DEVICE_CHANNEL_INFO device_channel;
		while (res->next())
		{
			device_channel.channel_no = res->getInt("channel_no");
			device_channel.channel_name = res->getString("channel_name");
			vec_device_channel.push_back(device_channel);
		}
	}
	catch (sql::SQLException& e)
	{
		LOG4CXX_ERROR(g_logger, "CSqlOpt::GetDeviceChannelFromDB:optDataDB = " << e.what());
		b_ok = MY_SQL_ERROR;
	}

	SAFE_CLOSE2(res, prep_stmt);
	g_OptDeviceDataDB.ReleaseConnection(con);
	
	return b_ok;
}

int CSqlOpt::GetUserChannelsFromDB(const std::string& username, std::vector<DEVICE_CHANNEL_INFO>& vec_user_channels)
{
	if (username.empty())
	{
		LOG4CXX_ERROR(g_logger, "CSqlOpt::GetUserChannelsFromDB arg error.");
		return FAILED;
	}

	sql::Connection* con;
	con = g_OptDeviceDataDB.GetConnection();
	if (con == NULL)
	{
		LOG4CXX_ERROR(g_logger, "CSqlOpt::GetUserChannelsFromDB:GetConnection = NULL");
		return MY_SQL_ERROR;
	}

	int b_ok = SUCCESS;
	sql::PreparedStatement* prep_stmt = NULL;
	sql::ResultSet* res = NULL;

	try 
	{
		prep_stmt = con->prepareStatement("SELECT device_guid,channel_no,channel_name FROM tb_user_relation_channel WHERE user_name=?");
		prep_stmt->setString(1, username);
		res = prep_stmt->executeQuery();
		DEVICE_CHANNEL_INFO device_channel;
		while (res->next())
		{
			device_channel.device_guid = res->getString("device_guid");
			device_channel.channel_no = res->getInt("channel_no");
			device_channel.channel_name = res->getString("channel_name");
			vec_user_channels.push_back(device_channel);
		}
	}
	catch (sql::SQLException& e)
	{
		LOG4CXX_ERROR(g_logger, "CSqlOpt::GetUserChannelsFromDB:optDataDB = " << e.what());
		b_ok = MY_SQL_ERROR;
	}

	SAFE_CLOSE2(res, prep_stmt);
	g_OptDeviceDataDB.ReleaseConnection(con);

	return b_ok;
}

int CSqlOpt::ModifyDeviceChannelNameToDB(const std::string& username, const std::string& device_guid, const int channel_no, const std::string& channel_name)
{
	if (username.empty() || device_guid.empty())
	{
		LOG4CXX_ERROR(g_logger, "CSqlOpt::ModifyDeviceChannelNameToDB arg error.");
		return FAILED;
	}

	sql::Connection* con;
	con = g_OptDeviceDataDB.GetConnection();
	if (con == NULL)
	{
		LOG4CXX_ERROR(g_logger, "CSqlOpt::ModifyDeviceChannelNameToDB:GetConnection = NULL");
		return MY_SQL_ERROR;
	}

	int b_ok = SUCCESS;
	sql::PreparedStatement* prep_stmt = NULL;

	try
	{
		prep_stmt = con->prepareStatement("UPDATE tb_user_relation_channel SET channel_name=? WHERE user_name=? AND device_guid=? AND channel_no=?");
		prep_stmt->setString(1, channel_name);
		prep_stmt->setString(2, username);
		prep_stmt->setString(3, device_guid);
		prep_stmt->setInt(4, channel_no);
		prep_stmt->executeUpdate();
		SAFE_CLOSE(prep_stmt);
	}
	catch (sql::SQLException& e)
	{
		LOG4CXX_ERROR(g_logger, "CSqlOpt::ModifyDeviceChannelNameToDB:optDataDB = " << e.what());
		b_ok = MY_SQL_ERROR;
	}

	SAFE_CLOSE(prep_stmt);
	g_OptDeviceDataDB.ReleaseConnection(con);

	return b_ok;
}

int CSqlOpt::ModifyDeviceWifiFlagToDB(const std::string& device_guid, const int wifi_flag)
{
	if (device_guid.empty())
	{
		LOG4CXX_ERROR(g_logger, "CSqlOpt::ModifyDeviceWifiFlagToDB arg error.");
		return FAILED;
	}

	sql::Connection* con;
	con = g_OptDeviceDataDB.GetConnection();
	if (con == NULL)
	{
		LOG4CXX_ERROR(g_logger, "CSqlOpt::ModifyDeviceWifiFlagToDB:GetConnection = NULL");
		return MY_SQL_ERROR;
	}

	int b_ok = SUCCESS;
	sql::PreparedStatement* prep_stmt = NULL;

	try
	{
		prep_stmt = con->prepareStatement("UPDATE tb_device_basic SET wifi_flag=? WHERE device_guid=?");
		prep_stmt->setInt(1, wifi_flag);
		prep_stmt->setString(2, device_guid);
		prep_stmt->executeUpdate();
		SAFE_CLOSE(prep_stmt);
	}
	catch (sql::SQLException& e)
	{
		LOG4CXX_ERROR(g_logger, "CSqlOpt::ModifyDeviceWifiFlagToDB:optDataDB = " << e.what());
		b_ok = MY_SQL_ERROR;
	}

	SAFE_CLOSE(prep_stmt);
	g_OptDeviceDataDB.ReleaseConnection(con);

	return b_ok;
}
