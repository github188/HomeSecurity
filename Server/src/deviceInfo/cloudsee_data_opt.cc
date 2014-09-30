/*****************************************************************
版权声明:	(C), 2012-2014, Jovision Tech. Co., Ltd.

文件名  :	cloudsee_data_opt.cc

作者    :	张帅   版本: v1.0    创建日期: 2014-05-26

功能描述:	云视通设备信息数据交互

其他说明:   

修改记录:
*****************************************************************/
#include "cloudsee_data_opt.h"
#include <iostream>
#include "../public/utils.h"
#include "../public/user_interface_defines.h"

CCloudSeeDataOpt::CCloudSeeDataOpt()
{

}

CCloudSeeDataOpt::CCloudSeeDataOpt(const char* data)
{
	message_data_ = data;
}

CCloudSeeDataOpt::~CCloudSeeDataOpt()
{
}

void CCloudSeeDataOpt::SetMessageData(const char* data)
{
	message_data_ = data;
}

bool CCloudSeeDataOpt::ParseMessageType(int& message_type)
{
	if (message_data_[4] != ST_DEVICESERVER)
	{
		LOG4CXX_ERROR(g_logger, "CCloudSeeDataOpt::ParseMessageType service_type error. service_type=" << message_data_[4]);
		return false;
	}

	message_type = message_data_[5];
	return true;
}

bool CCloudSeeDataOpt::ParseMessageId(int& message_id)
{
	uint16_t mid = 0;
	memcpy(&mid, message_data_ + 6, 2);
	message_id = ntohs(mid);
	if (message_id > 65535)
	{
		return false;
	}

	return true;
}

void CCloudSeeDataOpt::PackHeartBeat(PACKET_HEAD& packet_head)
{
	packet_head.packet_len = htonl(sizeof (packet_head));
	packet_head.service_type = ST_DEVICESERVER;
	packet_head.message_type = MT_HEARTBEAT;
}

bool CCloudSeeDataOpt::ParseDeviceInfoFull(std::map<std::string,DEVICE_INFO_CLOUDSEE>& device_info_map)
{
	const char *pointer_index = message_data_;

	pointer_index += sizeof (PACKET_HEAD);

	// 4字节分组数
	uint32_t group_count = 0;
	memcpy(&group_count, pointer_index, 4);
	group_count = ntohl(group_count);
	if (group_count > 1000)
	{
		LOG4CXX_ERROR(g_logger, "CCloudSeeDataOpt::ParseDeviceInfoFull group_count error. group_count=" << group_count);
		return false;
	}
	pointer_index += 4;
	//
	std::cout << "********** group_count=" << group_count << std::endl;
	//

	for (unsigned i = 0; i < group_count; ++i)
	{
		// 4字节分组名
		char group_name[5] = {0};
		memcpy(group_name, pointer_index, 4);
		//
		std::cout << "********** group_name=" << group_name << std::endl;
		//
		pointer_index += 4;

		// 4字节分组在线号码数
		uint32_t cloud_count = 0;
		memcpy(&cloud_count, pointer_index, 4);
		cloud_count = ntohl(cloud_count);
		//
		std::cout << "********** cloud_count=" << cloud_count << std::endl;
		//
		pointer_index += 4;

		DEVICE_INFO_CLOUDSEE device_info;
		for (unsigned j = 0; j < cloud_count; ++j)
		{
			// 4字节号码
			uint32_t cloud = 0;
			memcpy(&cloud, pointer_index, 4);
			cloud = ntohl(cloud);
			std::string cloudnum = std::string(group_name).append(utils::int2str(cloud));
			bzero(&device_info, sizeof (device_info));
			device_info.online_flag = ONLINE;
			pointer_index += 4;

			// 2字节通道
			uint16_t channel_num = 0;
			memcpy(&channel_num, pointer_index, 2);
			channel_num = ntohl(channel_num);
			device_info.channel_num = channel_num;
			device_info_map.insert(std::pair<std::string,DEVICE_INFO_CLOUDSEE>(cloudnum, device_info));
			pointer_index += 2;
		}
	}

	return true;
}

void CCloudSeeDataOpt::CreateRequestDeviceInfo(const int message_id, const std::vector<std::string>& vec_device, char* request, uint32_t& request_len)
{
	int byte_count = 0;
	PACKET_HEAD packet_head;	// 包头

	// 包头6字节
	packet_head.packet_len = 0;
	packet_head.service_type = ST_DEVICESERVER;
	packet_head.message_type = MT_GETDEVICEINFO;
	memcpy(request, &packet_head, 6);
	byte_count += 6;

	// 2字节message_id
	uint16_t mid = (uint16_t)message_id;
	mid = htons(mid);
	memcpy(request+byte_count, &mid, 2);
	byte_count += 2;

	// 4字节查询号码数
	uint32_t device_num = htonl(vec_device.size());
	memcpy(request+byte_count, &device_num, 4);
	byte_count += 4;

	// 4字节分组 4字节号码
	std::string device_guid;
	char group_name[4] = {0};
	uint32_t cloudnum = 0;
	for (unsigned i = 0; i < vec_device.size(); ++i)
	{
		device_guid = vec_device.at(i);
		group_name[0] = vec_device.at(i).at(0);
		memcpy(request+byte_count, group_name, 4);
		byte_count += 4;

		cloudnum = atoi(vec_device.at(i).substr(1).c_str());
		cloudnum = htonl(cloudnum);
		memcpy(request+byte_count, &cloudnum, 4);
		byte_count += 4;

		if (byte_count >= DATA_BUFFER_SIZE)
		{
			LOG4CXX_ERROR(g_logger, "CCloudSeeDataOpt::CreateRequestDeviceInfo big data. len=" << byte_count);
			packet_head.packet_len = htonl(byte_count) - 1;
			memcpy(request, &(packet_head.packet_len), 4);
			request[DATA_BUFFER_SIZE-1] = '\0';
			return;
		}
	}

	// 总包长
	request_len = byte_count;
	packet_head.packet_len = htonl(request_len);
	memcpy(request, &(packet_head.packet_len), 4);

	request[byte_count] = '\0';

	//trace log
	printf("CCloudSeeDataOpt::CreateRequestDeviceInfo len:%d\n", byte_count);
	for (int i = 0; i < byte_count; ++i)
	{
		printf("%#x ", request[i]);
	}
	printf("\n");
	//
}

bool CCloudSeeDataOpt::ParseResponseDeviceInfo(const char* cloudsee_response, std::vector<DEVICE_INFO_CLOUDSEE>& vec_dinfo_cloudsee)
{
	const char *pointer_index = cloudsee_response;

	// 6字节包头
	PACKET_HEAD packethead;
	memcpy(&packethead, pointer_index, sizeof (PACKET_HEAD));
	packethead.packet_len = ntohl(packethead.packet_len);
	if (packethead.packet_len < sizeof (PACKET_HEAD))
	{
		LOG4CXX_ERROR(g_logger, "CCloudSeeDataOpt::ParseResponseDeviceInfo packet_len = " << packethead.packet_len);
		return false;
	}
	if (packethead.service_type != ST_DEVICESERVER || packethead.message_type != MT_GETDEVICEINFO)
	{
		LOG4CXX_ERROR(g_logger, "CCloudSeeDataOpt::ParseResponseDeviceInfo service_type or message_type error.");
		return false;
	}
	pointer_index += sizeof (PACKET_HEAD);

	// 2字节message_id
	uint16_t message_id = 0;
	memcpy(&message_id, pointer_index, 2);
	message_id = ntohs(message_id);
	pointer_index += 2;

	// 4字节号码数
	uint32_t device_num = 0;
	memcpy(&device_num, pointer_index, 4);
	device_num = ntohl(device_num);
	pointer_index += 4;

	// 号码信息
	char group_name[4] = {0};
	uint32_t cloudnum = 0;
	DEVICE_INFO_CLOUDSEE device_info;
	for (unsigned i = 0; i < device_num; ++i)
	{
		bzero(group_name, 4);
		cloudnum = 0;

		// 4字节分组
		memcpy(group_name, pointer_index, 4);
		pointer_index += 4;

		// 4字节号码
		memcpy(&cloudnum, pointer_index, 4);
		cloudnum = ntohl(cloudnum);
		pointer_index += 4;

		device_info.device_guid = std::string(group_name).append(utils::int2str(cloudnum));

		// 1字节是否在线
		memcpy(&(device_info.online_flag), pointer_index, 1);
		if (!(device_info.online_flag == 0 || device_info.online_flag == 1))
		{
			device_info.online_flag = 0;
		}
		pointer_index += 1;

		// 1字节通道数
		memcpy(&(device_info.channel_num), pointer_index, 1);
		pointer_index += 1;

		// 1字节码流数
		memcpy(&(device_info.byte_stream), pointer_index, 1);
		pointer_index += 1;

		// 1字节主控网络类型
		memcpy(&(device_info.device_net_stat), pointer_index, 1);
		pointer_index += 1;

		// 4字节设备类型
		memcpy(&(device_info.device_type), pointer_index, 4);
		pointer_index += 4;

		vec_dinfo_cloudsee.push_back(device_info);
	}

	return true;
}
