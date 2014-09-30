#ifndef MESSAGE_H
#define MESSAGE_H

/* JK(JSON KEY) */
#define JK_MESSAGE_ID			"mid"
#define JK_MESSAGE_TYPE 		"mt"
#define JK_PROTO_VERSION		"pv"
#define JK_LOGIC_PROCESS_NO		"lpn"
#define JK_RESULT    			"rt"

#define JK_USERNAME				"username"
#define JK_PASSWORD				"password"

#define JK_DEVICE_LIST			"dlist"
#define JK_DEVICE_NAME			"dname"
#define JK_DEVICE_GUID			"dguid"
#define JK_DEVICE_TYPE			"dtype"
#define JK_DEVICE_NICKNAME		"dnickname"
#define JK_DEVICE_USERNAME		"dusername"
#define JK_DEVICE_PASSWORD		"dpassword"
#define JK_DEVICE_PIC_BIG		"dpicb"
#define JK_DEVICE_PIC_SMALL		"dpics"
#define JK_DEVICE_NET_STATE		"dnst"
#define JK_DEVICE_VIDEO_IP		"dvip"
#define JK_DEVICE_VIDEO_PORT	"dvport"
#define JK_DEVICE_VIDEO_USERNAME	"dvusername"
#define JK_DEVICE_VIDEO_PASSWORD	"dvpassword"
#define JK_NET_STORAGE_SWITCH	"netss"
#define JK_TF_STORAGE_SWITCH	"tfss"
#define JK_SECURITY_SWITCH		"secs"
#define JK_SECURITY_TIME		"sect"

#define JK_DEVICES_ONLINE_STATUS	"dsls"
#define JK_ONLINE_STATUS        "ols"

#define JK_DEVICES_PIC			"dspic"


enum MessageType_DeviceInfo
{
	GET_USER_DEVICES = 2001,
	GET_USER_DEVICES_RESPONSE = 2002,

	SET_DEVICE_INFO = 2003,
	SET_DEVICE_INFO_RESPONSE = 2004,

	GET_DEVICE_ONLINE_STATE = 2005,
	GET_DEVICE_ONLINE_STATE_RESPONSE = 2006,

	GET_DEVICE_PIC = 2007,
	GET_DEVICE_PIC_RESPONSE = 2008
};
enum MessageType_DeviceOnline
{
	DEVICE_ONLINE = 2201,
	DEVICE_ONLINE_RESPONSE = 2202,

	DEVICE_HEARTBEAT = 2203,
	DEVICE_HEARTBEAT_RESPONSE = 2204,

	DEVICE_OFFLINE = 2205,
	DEVICE_OFFLINE_RESPONSE = 2206,
};

enum TcpConnectFlag
{
	SHORT_CONNECTION,
	PERSIST_CONNECTION,
};

enum LogicProcessNo
{
	USER_LOGIN_PRO = 0,
	USER_INFO_PRO = 1,
	DEV_INFO_PRO = 2,
	AUTO_UPDATE_PRO = 3,
	MAIL_INFO_PRO = 4,	//邮件信息
	FEEDBACK_INFO_PRO = 5,
	IM_SERVER_DIRECT = 6,
	IM_SERVER_RELAY = 7,
	IM_SERVER_RELAY_REQUEST = 8,
	USER_INFO_CUSTOMIZATION_PRO = 9,
};




/**
 *
 * ===============================================================  用户通信消息 ===============================================================

所有请求消息共有	
JK_MESSAGE_ID	: 	<int>
JK_PROTO_VERSION: 	<char>
JK_LOGIC_PROCESS_NO  : <int>

所有回复消息共有
JK_MESSAGE_ID	: <int>
JK_PROTO_VERSION: <char>	


用户 <---------------------------------------------------------------------------------------------->基础服务器
获取版本升级信息
request:
{
	JK_MESSAGE_TYPE : 	<int>,  (TRY_GET_UPDATE_INFO)
	JK_CLIENT_VERSION:  <char>
{
response:
{
	JK_MESSAGE_TYPE : 	<int>,  (TRY_GET_UPDATE_INFO_RESPONSE)
	JK_RESULT :       	<int>   
	JK_UPDATE_FILE_INFO:
						{
							JK_FILE_VERSION:<char>,
							JK_FILE_URL:<char>, 
							JK_FILE_DESCRIPTION:<char>,
							JK_FILE_CHECKSUM:<char>
						}
}

Android获取版本升级信息
request:
{
JK_MESSAGE_TYPE : 	<int>,  (TRY_GET_UPDATE_INFO_ANDROID)
JK_CLIENT_VERSION:  <char>
{
response:
{
JK_MESSAGE_TYPE : 	<int>,  (TRY_GET_UPDATE_INFO_ANDROID_RESPONSE)
JK_RESULT :       	<int>   
JK_UPDATE_FILE_INFO:
{
JK_FILE_VERSION:<char>,
JK_FILE_URL:<char>, 
JK_FILE_DESCRIPTION:<char>,
JK_FILE_CHECKSUM:<char>
}
}


taglogin************************************用户注册，登录，注销  JK_LOGIC_PROESS_NO = USER_LOGIN************************************
判断用户是否存在
request:
{
	JK_MESSAGE_TYPE : 	<int>,  (IS_USER_EXIST_RESPONSE)
	JK_USERNAME :     	<char>,
}
response:
{
	JK_MESSAGE_TYPE : 	<int>,  (USER_REGISTER_RESPONSE)
	JK_RESULT :       	<int>   (maybe:USER_HAS_EXIST)
}

 注册
 request:
 {
	JK_MESSAGE_TYPE : 	<int>,  (USER_REGISTER)
	JK_USERNAME :     	<char>,
	JK_PASSWORD :     	<char>,
	JK_USER_OTHER_INFO:
					{
						JK_USER_GENDER	   :  <int>,
						JK_USER_BIRTH_DATE :  <char>
						JK_USER_ADDRESS    :  <char>
					}
 }
 response:
 {
	JK_MESSAGE_TYPE : 	<int>,  (USER_REGISTER_RESPONSE)
	JK_RESULT :       	<int>   (maybe:USER_HAS_EXIST)
 }
 
 登录
 request:
 {
	JK_MESSAGE_TYPE : 	<int>,  (LOGIN)
	JK_USERNAME :     	<char>,
	JK_PASSWORD :     	<char>
 }
 response:
 {
	JK_MESSAGE_TYPE : 	<int>,  (LOGIN_RESPONSE)
	JK_RESULT :       	<int>   (maybe:USER_NOT_EXIST ,PASSWORD_ERROR)
	JK_SESSION_ID :   	<char>  (md5 username + time )
 }

 注销
 request:
 {
	JK_MESSAGE_TYPE : 	<int>,  (LOGOUT)
	JK_SESSION_ID :   	<char>,
 }
 response:
 {
	JK_MESSAGE_TYPE : 	<int>,  (LOGOUT_RESPONSE)
	JK_RESULT :       	<int>   
 }

修改密码
 request:
 {
	 JK_MESSAGE_TYPE : 	<int>,  (MODIFY_USERPASS_RESPONSE)
	 JK_SESSION_ID :   	<char>,
	 JK_NEW_PASSWORD:   <char>,
	 JK_PASSWORD :     	<char>
 }
 response:
 {
	 JK_MESSAGE_TYPE : 	<int>,  (MODIFY_USERPASS_RESPONSE)
	 JK_RESULT :       	<int>   
 }

tagfriend************************************用户对好友相关操作逻辑  JK_LOGIC_PROESS_NO = USER_INFO************************************
得到用户的详细信息
 request:
 {
	JK_MESSAGE_TYPE : 	<int>, (GET_USER_DETAIL_INFO)
	JK_SESSION_ID :   	<char>
	JK_USERNAME:		<char>
 }
 response:
 {
	JK_MESSAGE_TYPE : 	<int>, (GET_USER_DETAIL_INFO_RESPONSE)
	JK_RESULT :       	<int>  
	JK_USER_OTHER_INFO:
				{
					JK_USER_NICKNAME   :  <char>
					JK_NAME			   :  <char>
					JK_USER_GENDER	   :  <int>
					JK_USER_BIRTH_DATE :  <char>
					JK_USER_ADDRESS    :  <char>
					JK_CHINESE_BIRTH   :  <char>
					JK_WESTERN_BIRTH   :  <char>
					JK_CITY			   :  <char>
					JK_PHONE		   :  <char>
					JK_JOB			   :  <char>
				}
 }

 修改用户的详细信息
 request:
 {
	 JK_MESSAGE_TYPE : 	<int>, (MODIFY_USER_DETAIL_INFO)
	 JK_SESSION_ID :   	<char>
	 JK_USERNAME:		<char>
	 JK_USER_OTHER_INFO:
		 {
		 	 JK_USER_NICKNAME   :  <char>
			 JK_NAME			:  <char>
			 JK_USER_GENDER	    :  <int>
			 JK_USER_BIRTH_DATE :  <char>
			 JK_USER_ADDRESS    :  <char>
			 JK_CHINESE_BIRTH   :  <char>
			 JK_WESTERN_BIRTH   :  <char>
			 JK_CITY			:  <char>
			 JK_PHONE		    :  <char>
			 JK_JOB			    :  <char>
		 }
 }
 response:
 {
	 JK_MESSAGE_TYPE : 	<int>, (MODIFY_USER_DETAIL_INFO_RESPONSE)
	 JK_RESULT :       	<int>  
 }

得到好友列表
 request:
 {
	JK_MESSAGE_TYPE : 	<int>, (GET_ALL_FRIENDS)
	JK_SESSION_ID :   	<char>
 }
 response:
 {
	JK_MESSAGE_TYPE : 	<int>, (GET_ALL_FRIENDS_RESPONSE)
	JK_RESULT :       	<int>  
	JK_FRIENDS_INFO :
	{
		JK_FRIEND_TEAM_DESCRIBE :<char>,        (自定义描述文件）
		JK_FRIEND_LIST:
		[
			{JK_USERNAME : <char>, JK_FRIEND_TEAM_ID : <int>, JK_USER_NICKNAME : <char>},
			......
		]
	}
 }
 
 精确查找用户
 request:
 {
	 JK_MESSAGE_TYPE : 	<int>, (EXACT_SEARCH_USERS)
	 JK_SESSION_ID   : 	<char>
	 JK_USERNAME :		<char>

 }
 response:
 {
	 JK_MESSAGE_TYPE : 	<int>, (EXACT_SEARCH_USERS_RESPONSE)
	 JK_RESULT :       	<int>,
	 JK_USER_INFO :	
				 {
					 JK_USERNAME		:  <char>,
					 JK_USER_NICKNAME   :  <char>,
					 JK_USER_GENDER		:  <int>,
					 JK_USER_BIRTH_DATE :  <char>
					 JK_USER_ADDRESS    :  <char>
				 }
 }

根据条件查找用户(返回一定数量的好友）
 request:
 {
	JK_MESSAGE_TYPE : 	<int>, (SEARCH_USERS)
	JK_SESSION_ID   : 	<char>
	JK_SEARCH_COUNT:	<int>,
	JK_SEARCH_INDEX :   <int>
	JK_SEARCH_CONDITION :
				{
					 JK_USER_NICKNAME:<char>
					 JK_USER_GENDER	: <int>,
					 JK_USER_AGE:	  <int>,
					 JK_USER_ADDRESS: <char>
				}
 }
 response:
 {
	JK_MESSAGE_TYPE : 	<int>, (SEARCH_USERS_RESPONSE)
	JK_RESULT :       	<int> 
	JK_USER_COUNT:		<int>
	JK_USELIST :
	[
		{JK_USERNAME : <char>, JK_USER_NICKNAME:<char>, JK_USER_GENDER :<int>,JK_USER_BIRTH_DATE : <char>, JK_USER_ADDRESS : <char>},
		......
	]
 }

 添加好友（默认组或其他组）
 request:
 {
	JK_MESSAGE_TYPE : 	<int>, (ADD_FRIEND)
	JK_SESSION_ID :   	<char>
	JK_USER_NICKNAME  : <char>
	JK_PEER_USERNAME :  <char>
	JK_PEER_NICKNAME:	<char>
	JK_FRIEND_TEAM_ID: 	<int>
 }
 response:
 {
	JK_MESSAGE_TYPE : 	<int>, (ADD_FRIEND_RESPONSE)
	JK_RESULT :       	<int>  
 }

 删除好友
 request:
 {
	JK_MESSAGE_TYPE : 	<int>, (DELETE_FRIEND)
	JK_SESSION_ID :   	<char>
	JK_PEER_USERNAME :  <char> (字符串)
 }
 response:
 {
	JK_MESSAGE_TYPE : 	<int>, (DELETE_FRIEND_RESPONSE)
	JK_RESULT :      	<int>  
 }
 
修改好友分组（添加组，删除组，修改组）
 request:
 {
	JK_MESSAGE_TYPE : 	<int>, (MODIFY_FRIENDSTEAM)
	JK_SESSION_ID :   	<char>
	JK_FRIEND_TEAM_DESCRIBE : <char> 
 }
 response:
 { 
	JK_MESSAGE_TYPE : 	<int>, (MODIFY_FRIENDSTEAM_RESPONSE)
	JK_RESULT :     	<int> 
 }
 
 将好友移入某个组
 request:
 {
	JK_MESSAGE_TYPE : 	<int>, (MOVE_FRIEND_TO_TEAM)
	JK_SESSION_ID :   	<char>
	JK_USERNAME:   		<char>
	JK_FRIEND_TEAM_ID :	<int>
 }
 response:
 {
	JK_MESSAGE_TYPE : 	<int>, (MOVE_FRIEND_TO_TEAM_RESPONSE)
	JK_RESULT :      	<int> 
 }

 tagdevice************************************ 用户设备操作逻辑 JK_LOGIC_PROESS_NO = USER_INFO************************************
  得到用户（包括自己的，好友的）的设备
 request:
 {
	JK_MESSAGE_TYPE : 	<int>, (GET_USER_DEVICES)
	JK_SESSION_ID :   	<char>
 }
 response:
 {
	JK_MESSAGE_TYPE : 	<int>, (GET_USER_DEVICES_RESPONSE)
	JK_RESULT :      	<int>,
	JK_DEVICES_INFO :
	{
		JK_DEVICE_TEAM_DESCRIBE : <char>,
		JK_DEVICE_LIST:
		[
			JK_DEVICE_GUID:<char>, JK_DEVICE_TYPE : <int> , JK_DEVICE_NAME : <char>, JK_DEVICE_TEAM_ID:<int>, JK_DEVICE_PERMISSION:<int>},
			......
		]
	}
 }
 
 修改设备的名称
 request:
 {
	JK_MESSAGE_TYPE : 	<int>, (MODIFY_DEVICE_NAME)
	JK_SESSION_ID :   	<char>
	JK_DEVICE_GUID :  	<char>
	JK_DEVICE_NAME : 	<char>
 }
 response:
 {
	JK_MESSAGE_TYPE : 	<int>, (MODIFY_DEVICE_NAME_RESPONSE)
	JK_RESULT :       	<int> 
 }
 
 修改设备组（添加组，删除组，修改组）
 request:
 {
	JK_MESSAGE_TYPE : 	<int>, (MODIFY_DEVICETEAM)
	JK_SESSION_ID :   	<char>
	JK_DEVICE_TEAM_DESCRIBE : <char> 
 }
 response:
 {
	JK_MESSAGE_TYPE : 	<int>, (MODIFY_DEVICETEAM_RESPONSE)
	JK_RESULT :       	<int> 
 }

 添加设备进入某个组
 request:
 {
	JK_MESSAGE_TYPE : 	<int>, (MOVE_DEVICE_TO_TEAM)
	JK_SESSION_ID :   	<char>
	JK_DEVICE_GUID :  	<char>
	JK_DEVICE_TEAM_ID:	<int>
 }
 response:
 {
	JK_MESSAGE_TYPE : 	<int>, (MOVE_DEVICE_TO_TEAM_RESPONSE)
	JK_RESULT :     	<int> 
 }
 
 修改设备属性
 request:
 {
 	JK_MESSAGE_TYPE : 	<int>, (MODIFY_DEVICE_ATTRIBUTE)
	JK_SESSION_ID :   	<char>
	JK_DEVICE_GUID:   	<char>
	JK_DEVICE_PERMISSION: <int>
 }
 response:
 {
 	 JK_MESSAGE_TYPE : 	<int>, (MODIFY_DEVICE_ATTRIBUTE_RESPONSE)
	 JK_RESULT :      	<int> 
 }
 
 得到用户的设备通道
 request:
 {
	JK_MESSAGE_TYPE : 	<int>,  (GET_DEVICE_CHANNELS)
	JK_SESSION_ID :   	<char>,
	JK_DEVICE_GUID :  	<char>,
	JK_DEVICE_TYPE :  	<int>
 }
 response:
 {
	JK_MESSAGE_TYPE : 	<int>, (GET_DEVICE_CHANNELS_RESPONSE)
	JK_RESULT :      	<int> 
	JK_DEVICE_CHANNELS_INFO:  (需要从设备数据库相应表中查找）
	[
	   {JK_DEVICE_CHANNEL_NO : <int>, JK_DEVICE_CHANNEL_NAME: <char> ,JK_DEVICE_CHANNEL_PERMISSION : <int>},             
	   ......
	]
 }
 
 修改设备的通道名称
 request:
 {
 	JK_MESSAGE_TYPE : 	<int>, (MODIFY_DEVICE_CHANNEL_NAME)
	JK_SESSION_ID :   	<char>
	JK_DEVICE_GUID:   	<char>
	JK_DEVICE_CHANNEL_NO : <int>,
	JK_DEVICE_CHANNEL_NAME : <char>   
 }
 response:
 {
 	 JK_MESSAGE_TYPE : 	<int>, (MODIFY_DEVICE_CHANNEL_NAME_RESPONSE)
	 JK_RESULT :      	<int> 
 }
 
 修改设备通道属性
 request:
 {
   JK_MESSAGE_TYPE : 	<int>,  (MODIFY_DEVICE_CHANNEL_ATTRITUBE)
   JK_SESSION_ID :   	<char>,
   JK_DEVICE_GUID :  	<char>,
   JK_DEVICE_TYPE :  	<int>, 
   JK_DEVICE_CHANNEL_NO : <int>,
   JK_DEVICE_CHANNEL_PERMISSION : <int>,
   JK_DEVICE_CHANNEL_NAME : <char>   
 }
 response:
 {
	JK_MESSAGE_TYPE : 	<int>, (MODIFY_DEVICE_CHANNEL_ATTRITUBE_RESPONSE)
	JK_RESULT :      	<int> 
 }


 得到有公共设备的好友（提供好友界面加号展示）
 request:
 {
	JK_MESSAGE_TYPE : 	<int>, (GET_FRIENDS_HAS_PUBLIC_DEVICE)
	JK_SESSION_ID :   	<char>
 }
 response:
 {
	JK_MESSAGE_TYPE : 	<int>, (GET_FRIENDS_HAS_PUBLIC_DEVICE_RESPONSE)
	JK_RESULT :      	<int> 
	JK_USELIST
	      [
	      	  {JK_USERNAME :<char>} ,
	      	  ......
	      ]
 }

得到好友的公共设备
 request:
 {
	JK_MESSAGE_TYPE : 	<int>, (GET_FRIEND_PUBLIC_DEVICES)
	JK_SESSION_ID :   	<char>
	JK_USERNAME:   		<char>
 }
 response:
 {
	JK_MESSAGE_TYPE : 	<int>, (GET_FRIEND_PUBLIC_DEVICES_RESPONSE)
	JK_RESULT :     
	JK_DEVICE_LIST
	       [
				{JK_DEVICE_GUID :  <char>, JK_DEVICE_NAME, JK_DEVICE_PERMISSION : <int>...},
				......
		   ]
 }

 taggroup************************************ 用户群组逻辑 JK_LOGIC_PROESS_NO = USER_INFO************************************
创建群组（管理员）
 request:
 {
	JK_MESSAGE_TYPE : 	<int>, (CREATE_GROUP)
	JK_SESSION_ID :   	<char>
	JK_GROUP_INFO:
				{
					JK_GROUP_NAME: 		<char>,
					JK_GROUP_TYPE: 		<char>,
					JK_GROUP_DESCRIBE: 	<char>,
					JK_GROUP_TEAM_INFO: <char>
				}
 }
 response:
 {
	JK_MESSAGE_TYPE : 	<int>, (CREATE_GROUP_RESPONSE)
	JK_RESULT :      	<int> 
 }

精确查找群组（所有用户）
 request:
 {
	 JK_MESSAGE_TYPE : 	<int>, (EXACT_SEARCH_GROUPS)
	 JK_SESSION_ID :   	<char>
	 JK_GROUP_NUM :		<int>   从1000开始
 }
 response:
 {
	JK_MESSAGE_TYPE : 	<int>, (EXACT_SEARCH_GROUPS_RESPONSE)
	JK_RESULT :      	<int> （GeneralResponse::SUCCESS; 其他错误：错误码）
	JK_GROUPS_INFO：
					{
						JK_GROUP_ID: <char>,
						JK_GROUP_NUM: <int>,
						JK_GROUP_NAME: <char>, 
						JK_GROUP_TYPE: <char>, 
						JK_GROUP_CREATER:<char>
					}
 }

 条件查找群组（所有用户）
 request:
 {
	JK_MESSAGE_TYPE : 	<int>, (SEARCH_GROUPS)
	JK_SESSION_ID :   	<char>,
	JK_GROUP_NAME:		<char>,
	JK_GROUP_TYPE:		<char>,
	JK_GROUP_DESCRIBE:	<char>,
	JK_SEARCH_COUNT:	<int>,
	JK_SEARCH_INDEX:	<int>,
 }
 response:
 {
	JK_MESSAGE_TYPE : 	<int>, (SEARCH_GROUPS_RESPONSE)
	JK_RESULT :      	<int> （GeneralResponse::SUCCESS; 其他错误：错误码）
	JK_GROUP_COUNT:		<int>
	JK_GROUPS_INFO
	        [
	        	{JK_GROUP_ID: <char>, JK_GROUP_NUM: <int>,JK_GROUP_NAME: <char>, JK_GROUP_TYPE: <char>, JK_GROUP_CREATER:<char>},
	        	......
	        ]
 }

 得到群组信息（所有用户）
 request:
 {
	JK_MESSAGE_TYPE : 	<int>, (GET_GROUP_INFO)
	JK_SESSION_ID :   	<char>,
	JK_GROUP_ID	:   	<char>
 }
 response:
 {
	JK_MESSAGE_TYPE : 	<int>, (GET_GROUP_INFO_RESPONSE)
	JK_RESULT :      	<int> （GeneralResponse::SUCCESS; 其他错误：错误码）
	JK_GROUP_INFO:
				{
					JK_GROUP_CREATER: 	<char>,
					JK_GROUP_NAME: 		<char>,
					JK_GROUP_TYPE: 		<char>,
					JK_GROUP_DESCRIBE: 	<char>,
				}
 }

 把某人加入群组（管理员）
 request:
 {
	JK_MESSAGE_TYPE : 	<int>, (JOIN_GROUP)
	JK_SESSION_ID :   	<char>,
	JK_GROUP_ID	:   	<char>,
	JK_USERNAME  :   	<char>,
	JK_USER_NICKNAME :  <char>
	JK_GROUP_TEAM_ID : 	<int>,
 }
 response:
 {
	JK_MESSAGE_TYPE : 	<int>, ( JOIN_GROUP_RESPONSE)
	JK_RESULT :      	<int> 
 }

 退出群组（除管理员外所有用户）
 request:
 {
	JK_MESSAGE_TYPE : 	<int>, ( QUIT_GROUP)
	JK_SESSION_ID :   	<char>,
	JK_GROUP_ID	:   	<char>
 }
 response:
 {
	JK_MESSAGE_TYPE : 	<int>, ( QUIT_GROUP_RESPONSE)
	JK_RESULT :      	<int> 
 }

 查找自身所属群组（所有用户）
 request:
 {
	JK_MESSAGE_TYPE : 	<int>, ( GET_BELONG_GROUPS)
	JK_SESSION_ID :   	<char>,
 }
 response:
 {
	JK_MESSAGE_TYPE : 	<int>, ( GET_BELONG_GROUPS_RESPONSE)
	JK_RESULT :      	<int> 
	JK_GROUPS_INFO:
			[
				{JK_GROUP_ID:<char>, JK_GROUP_NAME:<char>}, JK_GROUP_CREATER:<char>}
				......
			]
 }

得到群组成员（所有用户）
 request:
 {
	JK_MESSAGE_TYPE : 	<int>, ( GET_GROUP_MEMBERS)
	JK_SESSION_ID :   	<char>,
	JK_GROUP_ID	:   	<char>
 }
 response:
 {
	JK_MESSAGE_TYPE : 	<int>, ( GET_GROUP_MEMBERS_RESPONSE)
	JK_RESULT :      	<int> 
	JK_GROUP_TEAM_INFO :<char>
	JK_GROUP_MEMBER_LIST:
					[
						{JK_GROUP_MEMBER_NAME : <char> , JK_USER_NICKNAME :  <char>, JK_GROUP_TEAM_ID: <int>...},
						......
					]
 }
 
得到有公共设备的群组成员（所有用户）
 request:
 {
	JK_MESSAGE_TYPE : 	<int>, ( GET_HAS_PUBLIC_DEVICES_GROUP_MEMBERS)
	JK_SESSION_ID :   	<char>,
	JK_GROUP_ID	:   	<char>
 }
 response:
 {
	JK_MESSAGE_TYPE : 	<int>, ( GET_HAS_PUBLIC_DEVICES_GROUP_MEMBERS_RESPONSE)
	JK_RESULT :      	<int> 
	JK_USELIST
					[
						{JK_USERNAME : <char> },
						......
					]
 }

 把某个群组人员移入某个群组team中（管理员）
 request:
 {
	JK_MESSAGE_TYPE : 	<int>, ( MOVE_USER_TO_GROUP_TEAM)
	JK_SESSION_ID :   	<char>,
	JK_GROUP_ID	:   	<char>,
	JK_GROUP_MEMBER_NAME :	<char>,
	JK_GROUP_TEAM_ID : 	<int>,
 }
 response:
 {
	JK_MESSAGE_TYPE : 	<int>, ( MOVE_USER_TO_GROUP_TEAM_RESPONSE)
	JK_RESULT :      	<int>
 }
 
 修改群组team描述信息(管理员）
  request:
 {
	JK_MESSAGE_TYPE : 	<int>, ( MODIFY_GROUP_TEAM_INFO)
	JK_SESSION_ID :   	<char>,
	JK_GROUP_ID	:   	<char>,
	JK_GROUP_TEAM_INFO :<char>,
 }
 response:
 {
	JK_MESSAGE_TYPE : 	<int>, ( MODIFY_GROUP_TEAM_INFO_RESPONSE)
	JK_RESULT :      	<int> 
 }

 判断当前登录用户是否为组管理员
 request:
 {
	 JK_MESSAGE_TYPE : 	<int>, ( JUDGE_IS_GROUP_ADMIN)
	 JK_SESSION_ID :   	<char>,
	 JK_GROUP_ID	:   <char>,
 }
 response:
 {
	 JK_MESSAGE_TYPE : 	<int>, ( JUDGE_IS_GROUP_ADMIN_RESPONSE)
	 JK_RESULT :      	<int> 
 }


用户 <----------------------------------------------------------------------------------------------> 业务服务器
taguseronline************************************ 用户上线逻辑 ************************************
设置用户上线
 request:
 {
	JK_MESSAGE_TYPE :	<int>, (USER_ONLINE)
	JK_SESSION_ID :   	<char>
 }
 response:
 {
	JK_MESSAGE_TYPE : 	<int>, (USER_ONLINE_RESPONSE)
	JK_RESULT :      	<int>
 }
 
 刷新用户好友列表(操作好友列表缓存）
 request:
 {
	JK_MESSAGE_TYPE : 	<int>, (FLUSH_USER_FIREND_LIST)
	JK_SESSION_ID :   	<char>
	JK_FRIEND_LIST:   	
						[
							"username",
							"username",
							...
						]
 }
 response:
 {
	JK_MESSAGE_TYPE : 	<int>, (FLUSH_USER_FIREND_LIST_RESPONSE)
	JK_RESULT :      	<int>
 }

刷新用户设备列表(操作设备列表缓存）
 request:
 {
	JK_MESSAGE_TYPE : 	<int>, (FLUSH_USER_DEVICE_LIST)
	JK_SESSION_ID :   	<char>
	JK_DEVICE_LIST:   	 
						[
							"guid",
							"guid",
							...
						]
 }
 response:
 {
	JK_MESSAGE_TYPE : 	<int>, (FLUSH_USER_DEVICE_LIST_RESPONSE)
	JK_RESULT :      	<int>
 }


得到好友在线状态
 request:
 {
	JK_MESSAGE_TYPE : 	<int>, (GET_ALL_FRIENDS_ONLINE_STATUS)
	JK_SESSION_ID :   	<char>
 }
 response:
 {
	JK_MESSAGE_TYPE : 	<int>, (GET_ALL_FRIENDS_ONLINE_STATUS_RESPONSE)
	JK_RESULT :      	<int>
	JK_FRIENDS_ONLINE_STATUS:
	 	 	 	 	 	 	 [
	 	 	 	 	 	 	 	 {JK_USERNAME:<char>,JK_ONLINE_STATUS:<char>},
	 	 	 	 	 	 	 	 ...
	 	 	 	 	 	 	 ]
 }
 
得到设备在线状态
 request:
 {
	JK_MESSAGE_TYPE : 	<int>, (GET_ALL_DEVICES_ONLINE_STATUS)
	JK_SESSION_ID :   	<char>
 }
 response:
 {
	JK_MESSAGE_TYPE : 	<int>, (GET_ALL_DEVICES_ONLINE_STATUS_RESPONSE)
	JK_RESULT :      	<int>
	JK_DEVICES_ONLINE_STATUS:
	 	 	 	 	 	 	 [
	 	 	 	 	 	 	 	 {JK_DEVICE_GUID:<char>,JK_ONLINE_STATUS:<char>},
	 	 	 	 	 	 	 	 ...
	 	 	 	 	 	 	 ]
 }

 得到某用户设备的在线状态
 request:
 {
	JK_MESSAGE_TYPE: 	<int>, (GET_SOMEONE_DEVICES_ONLINE_STATUS)
	JK_SESSION_ID:   	<char>
	JK_USERNAME:   		<char>
	JK_DEVICE_LIST:  
							[
								"guid",
								"guid",
								...
							]
 }
 response:
 {
	JK_MESSAGE_TYPE: 	<int>, (GET_SOMEONE_DEVICES_ONLINE_STATUS_RESPONSE)
	JK_RESULT:      	<int>
	JK_DEVICES_ONLINE_STATUS:
	 	 	 	 	 	 	 [
	 	 	 	 	 	 	 	 {JK_DEVICE_GUID:<char>,JK_ONLINE_STATUS:<char>},
	 	 	 	 	 	 	 	 ...
	 	 	 	 	 	 	 ]
 }

 刷新组成员列表(操作组成员缓存）
 request:
 {
	JK_MESSAGE_TYPE : 	<int>, (FLUSH_GROUP_MEMBER_LIST)
	JK_SESSION_ID :   	<char>
	JK_GROUP_ID:        <char>
	JK_GROUP_MEMBER_LIST: 
							[
								"username",
								"username",
								...
							]
 }
 response:
 {
	JK_MESSAGE_TYPE : 	<int>, (FLUSH_GROUP_MEMBER_LIST_RESPONSE)
	JK_RESULT :      	<int>
 }

 得到群组成员在线状态
 request:
 {
	 JK_MESSAGE_TYPE : 	<int>, (GET_GROUP_MEMBERS_ONLINE_STATUS)
	 JK_SESSION_ID :   	<char>
	 JK_GROUP_ID :   	<char>
 }
 response:
 {
	 JK_MESSAGE_TYPE : 	<int>, (GET_GROUP_MEMBERS_ONLINE_STATUS_RESPONSE)
	 JK_RESULT :      	<int>
	 JK_GROUP_ID   :   	<char>
	 JK_GROUP_MEMBERS_ONLINE_STATUS:
	 	 	 	 	 	 	 [
	 	 	 	 	 	 	 	 {JK_USERNAME:<char>,JK_ONLINE_STATUS:<char>},
	 	 	 	 	 	 	 	 ...
	 	 	 	 	 	 	 ]
 }

设置用户在线状态
 request:
 {
	JK_MESSAGE_TYPE : 	<int>, (SET_ONLINE_STATUS)
	JK_SESSION_ID :   	<char>
	JK_USER_ONLINE_STATUS: <int>
 }
 response:
 {
	JK_MESSAGE_TYPE : 	<int>, (SET_ONLINE_STATUS_RESPONSE)
	JK_RESULT :      	<int>
 }

获取离线消息的简要信息（对方ID，以便进一步获取详细信息）
request:
{
	JK_MESSAGE_TYPE : 	<int>, (GET_BRIEF_OFFLINE_MESSAGE)
	JK_SESSION_ID :   	<char>
}
response:
{
	JK_MESSAGE_TYPE : 	<int>, (GET_BRIEF_OFFLINE_MESSAGE_RESPONSE)
	JK_RESULT :      	<int>，
	JK_PEER_REQUEST_LIST：
						[
							username/deviceguid,
							...
						]
	JK_PEER_CHAT_LIST：
						[
						username,
						...
						]
	JK_PEER_GROUP_LIST：
						[
						groupid,
						...
						]
	
}

根据对方id获取请求类离线的消息
request:
{
	JK_MESSAGE_TYPE : 	<int>, (GET_REQUEST_OFFLINE_MESSAGE)
	JK_SESSION_ID :   	<char>
	JK_USERNAME:		<char>
}
response:
{
	JK_MESSAGE_TYPE : 	<int>, (GET_REQUEST_OFFLINE_MESSAGE_RESPONSE)
	JK_RESULT :      	<int> 
	JK_OFFLINE_MESSAGES:
					[
						转发消息                                    
						...
					]
}

根据对方用户名获取U2U聊天离线消息
request:
{
	JK_MESSAGE_TYPE : 	<int>, (GET_CHAT_OFFLINE_MESSAGE)
	JK_SESSION_ID :   	<char>
	JK_USERNAME:		<char>
}
response:
{
	JK_MESSAGE_TYPE : 	<int>, (GET_CHAT_OFFLINE_MESSAGE_RESPONSE)
	JK_RESULT :      	<int> 
	JK_OFFLINE_MESSAGES:
						[
							转发消息                                    
							...
						]
}

根据群组id获取U2G聊天离线消息
request:
{
	JK_MESSAGE_TYPE : 	<int>, (GET_GROUP_OFFLINE_MESSAGE)
	JK_SESSION_ID :   	<char>
	JK_USERNAME:		<char>
}
response:
{
	JK_MESSAGE_TYPE : 	<int>, (GET_GROUP_OFFLINE_MESSAGE_RESPONSE)
	JK_RESULT :      	<int> 
	JK_OFFLINE_MESSAGES:
						[
							转发消息                                     
							...
						]
}
 
请求添加好友
 request:
 {
	JK_MESSAGE_TYPE : 	<int>, (REQUEST_ADD_FRIEND)
	JK_SESSION_ID :   	<char>
	JK_USER_NICKNAME :  <char>
	JK_PEER_USERNAME:  	<char>
	JK_RELAY_MESSAGE:   <char> 
 }
 response:
 {
	JK_MESSAGE_TYPE : 	<int>, (REQUEST_ADD_FRIEND_RESPONSE)
	JK_RESULT :      	<int> 
 }

同意添加对方为好友的回复
 request:
 {
	JK_MESSAGE_TYPE : 	<int>, (REPLY_ADD_FRIEND)
	JK_SESSION_ID :   	<char>
	JK_USER_NICKNAME :  <char>
	JK_PEER_USERNAME :  <char> 
	JK_RELAY_MESSAGE: 	<char>
 }
 response:
 {
	JK_MESSAGE_TYPE : 	<int>, (REPLY_ADD_FRIEND_RESPONSE)
	JK_RESULT :      	<int> 
 }
 
 向管理员请求加入群组
 request:
 {
	JK_MESSAGE_TYPE : 	<int>, (REQUEST_JOIN_GROUP)
	JK_SESSION_ID :   	<char>
	JK_USER_NICKNAME :  <char>
	JK_PEER_USERNAME :  <char> （管理员姓名）
	JK_GROUP_ID :    	<char>
	JK_RELAY_MESSAGE: 	<char> 
 }
 response:
 {
	JK_MESSAGE_TYPE : 	<int>, (REQUEST_JOIN_GROUP_RESPONSE)
	JK_RESULT :      	<int> 
 }
 
作为管理员时同意添加对方为组成员的回复
 request:
 {
	JK_MESSAGE_TYPE : 	<int>, (REPLY_JOIN_GROUP)
	JK_SESSION_ID :   	<char>
	JK_USER_NICKNAME :  <char>
	JK_PEER_USERNAME :  <char>
	JK_GROUP_ID :     	<char>
	JK_RELAY_MESSAGE: 	<char>
 }
 response:
 {
	JK_MESSAGE_TYPE : 	<int>, (REPLY_JOIN_GROUP_RESPONSE)
	JK_RESULT :      	<int> 
 }

 向好友推送聊天消息
 request:
 {
	 JK_MESSAGE_TYPE : 	<int>, (PUSH_USER_CHAT_MESSAGE)
	 JK_SESSION_ID :   	<char>
	 JK_USER_NICKNAME :  <char>
	 JK_PEER_USERNAME : <char>
	 JK_RELAY_MESSAGE: 	<char> 
 }
 response:
 {
	 JK_MESSAGE_TYPE : 	<int>, (PUSH_USER_CHAT_MESSAGE_RESPONSE)
	 JK_RESULT :      	<int> 
 }

向群组成员推送聊天消息(在服务端从缓存中取出群成员团列表，分别进行消息发送)
 request:
 {
	 JK_MESSAGE_TYPE : 	<int>, (PUSH_GROUP_CHAT_MESSAGE)
	 JK_SESSION_ID :   	<char>
	 JK_USER_NICKNAME :  <char>
	 JK_GROUP_ID :     	<char>
	 JK_RELAY_MESSAGE: 	<char> 
 }
 response:
 {
	 JK_MESSAGE_TYPE : 	<int>, (PUSH_GROUP_CHAT_MESSAGE_RESPONSE)
	 JK_RESULT :      	<int> 
 }

向用户推送“设备绑定到该用户”的提示消息
request:
{
         JK_MESSAGE_TYPE:   <int>, (PUSH_USER_DEVICEBIND_MESSAGE)
         JK_PEER_USERNAME:  <char>
         JK_DEVICE_TYPE:    <int>
         JK_DEVICE_NAME:    <char>
         JK_RELAY_MESSAGE:  <char>
}
response:
{
         JK_MESSAGE_TYPE :      <int>, (PUSH_USER_DEVICEBIND_MESSAGE_RESPONSE)
         JK_RESULT :            <int>
}

 向用户推送“设备更新”的提示消息
request:
{
         JK_MESSAGE_TYPE:   <int>, (PUSH_USER_DEVICEUPDATE_MESSAGE)
         JK_PEER_USERNAME:  <char>
         JK_DEVICE_GUID:    <char>
         JK_DEVICE_NAME:    <char>
         JK_RELAY_MESSAGE:  <char>
}
response:
{
         JK_MESSAGE_TYPE :      <int>, (PUSH_USER_DEVICEUPDATE_MESSAGE_RESPONSE)
         JK_RESULT :            <int>
}
清理临时聊天组成员缓存
 request:
 {
	 JK_MESSAGE_TYPE : 	<int>, (CLEAR_TEMP_GROUP_CACHE)
	 JK_SESSION_ID :   	<char>
	 JK_GROUP_ID :     	<char>
 }
 response:
 {
	 JK_MESSAGE_TYPE : 	<int>, (CLEAR_TEMP_GROUP_CACHE_RESPONSE)
	 JK_RESULT :      	<int> 
 }

 ===============================================================  设备通信消息 ===============================================================
 设备 <---------------------------------------------------------------------------------------------->基础服务器
 tagregister************************************ 设备注册逻辑 ************************************
 设备注册
 request:
 {
	 JK_MESSAGE_TYPE : 	<int>, (DEVICE_REGISTER)
	 JK_DEVICE_GUID :  	<char>,
	 JK_DEVICE_NAME :  	<char>
 }
 response:
 {
	 JK_MESSAGE_TYPE : 	<int>, (DEVICE_REGISTER_RESPONSE)
	 JK_RESULT :       	<int>  
 }
 设备更新名称
 request:
 {
	 JK_MESSAGE_TYPE : 	<int>, (DEVICE_UPDATE_NAME)
	 JK_DEVICE_GUID :  	<char>,
	 JK_DEVICE_NAME :  	<char>
 }
 response:
 {
	 JK_MESSAGE_TYPE : 	<int>, (DEVICE_UPDATE_NAME_RESPONSE)
	 JK_RESULT :       	<int>
 }
 设备更新通道名称
 request:
 {
	 JK_MESSAGE_TYPE : 	<int>, (DEVICE_UPDATE_CHANNEL)
	 JK_DEVICE_GUID :  	<char>,
	 JK_DEVICE_NAME :  	<char>
	 JK_DEVICE_CHANNEL_NO: 	<int>
	 JK_DEVICE_CHANNEL_NAME: <char>
	 JK_DEVICE_CHANNEL_PERMISSION:<int>
 }
 response:
 {
	 JK_MESSAGE_TYPE : 	<int>, (DEVICE_UPDATE_CHANNEL_RESPONSE)
	 JK_RESULT :       	<int>
 }
 设备绑定
 request:
 {
	 JK_MESSAGE_TYPE : 	<int>, (DEVICE_BIND_USER)
	 JK_DEVICE_GUID  : 	<char>,
	 JK_USERNAME:     	<char>,
	 JK_DEVICE_TYPE :  	<int>,
	 JK_DEVICE_NAME :  	<char>
 }
 response:
 {
	 JK_MESSAGE_TYPE : 	<int>, (DEVICE_BIND_USER_RESPONSE)
	 JK_RESULT :      	<int> 
 }
 设备解除绑定
 request:
 {
	 JK_MESSAGE_TYPE : 	<int>, (DEVICE_REMOVE_BIND)
	 JK_DEVICE_GUID  : 	<char>,
 }
 response:
 {
	 JK_MESSAGE_TYPE : 	<int>, (DEVICE_REMOVE_BIND_RESPONSE)
	 JK_RESULT :      	<int>
 }

 设备上报详细信息
 request:
 {
	 JK_MESSAGE_TYPE : 	<int>, (DEVICE_REPORT_DETAIL)
	 JK_DEVICE_GUID :  	<char>,
	 JK_DEVICE_TYPE :  	<int>,
	 JK_DEVICE_CHANNELS_INFO :
						 [
						 {JK_DEVICE_CHANNEL_NO:	<int>, JK_DEVICE_CHANNEL_NAME : <char>}
						 ...
						 ]
 }
 response:
 {
	 JK_MESSAGE_TYPE : 	<int>, (DEVICE_REPORT_DETAIL_RESPONSE)
	 JK_RESULT :      	<int> 
 }
 
设备 <----------------------------------------------------------------------------------------------> 业务服务器
tagdeviceonline************************************ 设备上线逻辑 ************************************
设备上线
 request:
 {
	JK_MESSAGE_TYPE : 	<int>, (DEVICE_ONLINE)
	JK_DEVICE_GUID :  	<char>
 }
 response:
 {
	JK_MESSAGE_TYPE : 	<int>, (DEVICE_ONLINE_RESPONSE)
	JK_RESULT :      	<int>  
 }
 
设备心跳
request:
{
	JK_MESSAGE_TYPE : 	<int>, (DEVICE_HEARTBEAT)
	JK_DEVICE_GUID :  	<char>
}
response:
{
	JK_MESSAGE_TYPE : 	<int>, (DEVICE_HEARTBEAT_RESPONSE)
	JK_RESULT :      	<int>  
}

设备下线
 request:
 {
	JK_MESSAGE_TYPE : 	<int>, (DEVICE_OFFLINE)
	JK_DEVICE_GUID :  	<char>
 }
 response:
 {
	JK_MESSAGE_TYPE : 	<int>, (DEVICE_OFFLINE_RESPONSE)
	JK_RESULT :      	<int>  
 }
 
 ===============================================================  IM业务服务器 和 消息转发服务器 通信消息  ===============================================================
 tagrelay************************************ 设备上线逻辑 ************************************
IM业务服务器---------------------------------------------------------------------------------------------->消息转发服务器
客户端间请求消息的转发(如果对方不在线需要存为离线消息)
request:
 {
	JK_MESSAGE_TYPE :  <int>, (RELAY_REQUEST)
	"to" :  	<char>      (peername)
	"message":  <char>		 
 }
 {JK_MESSAGE_TYPE:3001,"to":"wht","message":"{\"messagetype\":1007,\"username\":\"ljf\",\"friendname\":\"wht\",\"verifymessage\":\"hello\"}"}
 不需要response

IM服务器收到向上线客户端发送消息的结果(如果接收到此消息，表明中转消息发送成功，可从消息队列中删除）
 request:
 {
	JK_MESSAGE_TYPE :  <int>, (RELAY_SEND_RESULT)
	"to" :  <char>
	"messageguid": <char>   (在缓存中某用户消息的唯一id）
 }

客户端间临时推送消息的转发(如果对方不在线"不"需要存为离线消息，不用有回应)
request:
 {
	JK_MESSAGE_TYPE :  <int>, (TEMP_RELAY_REQUEST)
	"to" :  <char>
	"message": <char>
 }


消息转发服务器---------------------------------------------------------------------------------------------->各个IM业务服务器
 客户端间请求消息的转发
 request:
 {
    JK_MESSAGE_TYPE :  <int>, (RELAY_REQUEST)
	"to" :           <char> (peername)
	“messageguid”  <char>   (在缓存中某用户消息的唯一id）
	"message": <char>
 }
 {JK_MESSAGE_TYPE:3001,"to":"wht","messageguid":"{F2AE6D95-F28D-9746-A013-E974AB0AF837}","message":"{\"messagetype\":1007,\"username\":\"ljf\",\"friendname\":\"wht\",\"verifymessage\":\"hello\"}"}

 客户端间临时推送消息的转发(如果对方不在线"不"需要存为离线消息)
 request:
 {
	JK_MESSAGE_TYPE :  <int>, (TEMP_RELAY_REQUEST)
	"to" :  <char>
	"message": <char>
 }

创建用户通知事件
 request:
 {
	 JK_MESSAGE_TYPE : 	<int>, (CREATE_NOTICE_EVENT)
	 JK_USERNAME :  	<char>
	 JK_USER_EVENT_TIME : <char>
	 JK_USER_EVENT_MSG : <char>
 }
 response:
 {
	 JK_MESSAGE_TYPE : 	<int>, (CREATE_NOTICE_EVENT_RESPONSE)
	 JK_RESULT :      	<int>
	 JK_USER_EVENT_ID : <id>:      	<int>
 }
删除用户通知事件
 request:
 {
	 JK_MESSAGE_TYPE : 	<int>, (DELETE_NOTICE_EVENT)
	 JK_USERNAME :  	<char>
	 JK_USER_EVENT_ID : <char>
 }
 response:
 {
	 JK_MESSAGE_TYPE : 	<int>, (DELETE_NOTICE_EVENT_RESPONSE)
	 JK_RESULT :      	<int>
 }

修改用户通知事件
 request:
 {
	 JK_MESSAGE_TYPE : 	<int>, (MODIFY_NOTICE_EVENT)
	 JK_USERNAME :  	<char>
	 JK_USER_EVENT_MSG :<char>
	 JK_USER_EVENT_ID : <char>
 }
 response:
 {
	 JK_MESSAGE_TYPE : 	<int>, (MODIFY_NOTICE_EVENT_RESPONSE)
	 JK_RESULT :      	<int>
 }

获取用户所有的通知事件
 request:
 {
	 JK_MESSAGE_TYPE : 	<int>, (GET_ALL_NOTICE_EVENT)
	 JK_USERNAME :  	<char>
 }
 response:
 {
	 JK_MESSAGE_TYPE : 	<int>, (GET_ALL_NOTICE_EVENT_RESPONSE)
	 JK_RESULT :      	<int>
 }

===================================邮件逻辑===================================
 获取未读邮件个数
 request:
 {
	JK_MESSAGE_TYPE : 	<int>,  (GET_MAIL_INFO)
	JK_USERNAME :	<string>
 }
 response:
 {
	JK_MESSAGE_TYPE : 	<int>,  (GET_MAIL_INFO_RESPONSE)
	JK_RESULT :       	<int>
	JK_MAIL_INFO:
	{
		JK_MAIL_NEW:<int>
	}
 }

 用户注册同步
 request:
 {
	JK_MESSAGE_TYPE :	<int>,  (REG_MAIL 3301)
	JK_USERNAME :		<string>
 }
 response:
 {
	JK_MESSAGE_TYPE :	<int>,  (REG_MAIL_RESPONSE 3302)
	JK_RESULT :			<int>   (SUCCESS 0, FAILED -1)
 }

 通讯录同步
 request：
 {
	JK_MESSAGE_TYPE :    <int>, (SYNC_ADD_FRIEND 3303)
	JK_USERNAME :        <char>,
	JK_USER_NICKNAME :   <char>,
	JK_PEER_USERNAME :   <char>,
	JK_PEER_NICKNAME :   <char>
 }
 response:
 {
	JK_MESSAGE_TYPE :	<int>, (SYNC_ADD_FRIEND_RESPONSE 3304)
	JK_RESULT :			<int>   (SUCCESS 0, FAILED -1)
 }

 request:
 {
	JK_MESSAGE_TYPE :	<int>, (SYNC_DEL_FRIEND 3305)
	JK_USERNAME :		<char>,
	JK_PEER_USERNAME :	<char>
 }
 response:
 {
	JK_MESSAGE_TYPE :	<int>, (SYNC_DEL_FRIEND_RESPONSE 3306)
	JK_RESULT :			<int>   (SUCCESS 0, FAILED -1)
 }

 web端获取用户信息
 request:
 {
	JK_MESSAGE_TYPE :	<int>, (GET_USER_INFO_NOSESSION 1117)
	JK_USERNAME :		<char>,
 }
 response:
 {
	JK_MESSAGE_TYPE :	<int>, (GET_USER_INFO_NOSESSION_RESPONSE 1118)
	JK_RESULT :			<int>, (SUCCESS 0, FAILED -1)
	JK_MESSAGE_ID :		<int>,
	JK_USER_INFO :
	{
		JK_SECURITY_MAIL : <char>,
		...
	}
 }
 */


#endif

