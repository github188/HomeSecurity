/*
 * evp_crypt.h
 *
 *  Created on: Jun 24, 2014
 *      Author: zhangcq
 */

#ifndef EVP_CRYPT_H_
#define EVP_CRYPT_H_

#include <string>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/x509.h>
class EvpCrypt
{
public:
	EvpCrypt();
	~EvpCrypt();
	void setKey(const std::string& key);
	void getKey(std::string& key);
	/* 对密码进行解密
	 * @param1 [in] 加密的字符串
	 * @param2 [out] 解密后的字符串
	 *  */
	bool DoEvpDecode(const std::string& encryption, std::string& decode);
	/* 对密码进行加密
	 * @param1 [in] 未进行加密的字符串
	 * @param2 [out] 加密后的字符串
	 *  */
	bool DoEvpEncrypt(const std::string& decode, std::string& encryption);
private:
	EVP_CIPHER_CTX ctx; //密文上下文
	std::string key_;
};



#endif /* EVP_CRYPT_H_ */
