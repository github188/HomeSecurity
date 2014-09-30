/*****************************************************************
版权声明:	(C), 2012-2014, Jovision Tech. Co., Ltd.

文件名  :	cloudsee_data_opt.h

作者    :	张帅   版本: v1.0    创建日期: 2014-05-26

功能描述:	云视通设备信息数据交互

其他说明:   

修改记录:
*****************************************************************/
#ifndef CLOUDSEE_DATA_OPERATE_H_
#define CLOUDSEE_DATA_OPERATE_H_

#include <string>
#include <map>
#include "defines.h"

class CCloudSeeDataOpt
{
public:
	CCloudSeeDataOpt();
	explicit CCloudSeeDataOpt(const char* data);
	virtual ~CCloudSeeDataOpt();

public:
	void SetMessageData(const char* data);
	bool ParseMessageType(int& message_type);
	bool ParseMessageId(int& message_id);

	void PackHeartBeat(PACKET_HEAD& packet_head);
	bool ParseDeviceInfoFull(std::map<std::string,DEVICE_INFO_CLOUDSEE>& device_info_map);

	void CreateRequestDeviceInfo(const int message_id, const std::vector<std::string>& vec_device, char* request, uint32_t& request_len);
	bool ParseResponseDeviceInfo(const char* cloudsee_response, std::vector<DEVICE_INFO_CLOUDSEE>& vec_dinfo_cloudsee);

private:
	const char* message_data_;

};

#endif /* CLOUDSEE_DATA_OPERATE_H_ */
