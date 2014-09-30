/*****************************************************************************
**
** Copyright (C) 2014 jovision Corporation and/or its subsidiary(-ies).
** All rights reserved.
** @author : yaocoder
** @Contact: yaocoder@gmail.com
**
** @brief: This file is network interface library for linux client.
**
****************************************************************************/
#ifndef user_interface_h__
#define user_interface_h__

#include "net_interface_defines.h"

class  INetChannelInterface
{
public:

	virtual ~INetChannelInterface() {}
	
	/********************************通用接口********************************/

	static INetChannelInterface* GetImplInstance();

	static void DestroyImplInstance(const INetChannelInterface* pImpl);

	/**
	* @brief 初始化接口SDK（初始化日志，网络配置）
	*/
#ifndef USE_LOG4CXX
	virtual bool Init(const std::string& ip, const int port) = 0;
#else
	#ifdef USE_LOG4CXX_PTR
		virtual bool Init(const std::string& ip, const int port, const LoggerPtr loggerPrt) = 0;
	#else
		virtual bool Init(const std::string& ip, const int port, const std::string& log_path) = 0;
	#endif
#endif

	/**
	 * @brief 设置网络行为超时时间（暴露给应用层）
	 * @param [in]	连接超时时间
	 * @param [in]	得到回应超时时间
	 * @return
	 */
	virtual void SetTimeoutSecond(const int connect_timeout_second, const int rq_timeout_second) = 0;

	/**
	* @brief 与对端主动建立长连接
	* @param [in] 心跳状态回调函数(心跳检测与服务器的连接状态是否正常,对服务器后台起到防止半开连接的作用），
				  需要在上层实现LiveStatusCB来实时判断与服务器通信是否正常，累计统计达到一定次数时认为和
				  服务器连接中断
	*/
	virtual int EstablishPersistentChannel(const LiveStatusCB liveStatusCb) = 0;

	/**
	 * @brief 通过此接口发出网络请求，得到具体响应（告警服务主动拉取告警信息时使用）
	 * @param [in] 服务类型
	 * @param [in] 用户名
	 * @param [in] 请求数据
	 * @param [out] 响应数据
	 * @return
	 */
	virtual int GetResponseByRequestPersistConnectionServer(const int app_type, const std::string& username, const std::string& request, std::string& response) = 0;

	/**
	* @brief 主动接收通道消息
	* @param [in] 推送消息回调函数
	*/
	virtual void RegisterServerPushFunc(ServerPushCallBack_Info sp_cb_info) = 0;

	/********************************定制业务接口********************************/
	/**
	 * @brief 获取帐号在线信息
	 * @param [in] 用户名
	 * @param [out]用户上线相关信息
	 */
	virtual int GetAccountLiveInfo(const std::string& username, ACCOUNT_LIVE_INFO& accountLiveInfo) = 0;

	/**
	 * @brief 发送消息给客户端
	 * @param [in] 用户名
	 * @param [out]用户上线相关信息
	 */
	virtual int SendMessageToUser(const std::string& username, const std::string& json_string) = 0;

private:

	static INetChannelInterface*  instance_;

};

#endif // user_interface_h__
