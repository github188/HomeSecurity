#ifndef user_interface_impl_h__
#define user_interface_impl_h__


#include "net_interface.h"
#include "net_interface_defines.h"

class CNetDataLayer;
class CNetInterLayer;
class CImplTimer;

class  CClientNetInterfaceImpl:public INetChannelInterface
{
public:
	CClientNetInterfaceImpl();
	~CClientNetInterfaceImpl();

#ifndef USE_LOG4CXX
	virtual bool Init(const std::string& ip, const int port);
#else
#ifdef USE_LOG4CXX_PTR
	virtual bool Init(const std::string& ip, const int port, const LoggerPtr loggerPrt);
#else
	virtual bool Init(const std::string& ip, const int port, const std::string& log_path);
#endif
#endif

	int EstablishPersistentChannel(LiveStatusCB liveStatusCb);

	void RegisterServerPushFunc(ServerPushCallBack_Info sp_cb_info);
	void ServerPushMessageOpt(const std::string& server_push_message);

	int GetResponseByRequestPersistentConnection(const std::string& request, std::string& response);

	int GetResponseByRequestShortConnection(const std::string& request, std::string& response);

private:

	/**
	* @brief 心跳检测与服务器的连接状态是否正常（对服务器后台起到防止半开连接的作用）
	* @param [in] 心跳状态回调函数
	* @return
	*/
	int HeartBeatDetect(LiveStatusCB liveStatusCb);
	/**
	* @brief 获取与服务器的连接状态是否正常
	* @return
	*/
	int GetLiveStatus();
	void StopHeartBeat();
	int ClearSession();
	static void OnTimeGetLiveStatus(void* param);

private:

	CImplTimer*  implTimer_;
	LiveStatusCB liveStatusCb_;

	CNetDataLayer*	pNetDataOpt_;
	CNetInterLayer* pInterLayer_;

	ServerPushCallBack_Info sp_cb_info_;

	bool b_callback_register_;
};


#endif
