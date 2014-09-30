#ifdef __cplusplus
extern "C"
{
#endif

//字符集类型
typedef enum encode_type_
{
	UTF8,
	GBK,
	GB2312,
	ASCII
}ENCODE_TYPE;
//邮件信息
typedef struct mail_info_
{
	char toMailAddrList[1024];	//收件人地址列表 以;分隔
	char ccMailAddrList[1024];	//抄送人地址列表 以;分隔
	char mailSubject[64];		//邮件标题
	char mailText[2048];		//邮件内容
	char attachmentList[1024];	//附件列表 以;分隔
	ENCODE_TYPE charset;		//邮件编码字符集
}MAIL_INFO_;

int mail_send(const MAIL_INFO_ *mail_info);

#ifdef __cplusplus
}
#endif
