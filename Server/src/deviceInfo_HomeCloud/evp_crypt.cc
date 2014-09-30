/*
 * evp_crypt.cc
 *
 *  Created on: Jun 24, 2014
 *      Author: zhangcq
 */

#include <stdio.h>
#include "evp_crypt.h"
#include "defines.h"
#include "../public/utils.h"
#define PWDLEN 20

EvpCrypt::EvpCrypt()
{
	EVP_CIPHER_CTX_init(&ctx);
}

EvpCrypt::~EvpCrypt()
{
	EVP_CIPHER_CTX_cleanup(&ctx);
}

void EvpCrypt::setKey(const std::string& key)
{
	key_ = key;
}

void EvpCrypt::getKey(std::string& key)
{
	key = key_;
}

bool EvpCrypt::DoEvpDecode(const std::string& encryption, std::string& decode)
{
	std::vector<std::string> vec;
	utils::SplitData(encryption, ",", vec);
	char cryption[PWDLEN];
	for (unsigned int i = 0; i < vec.size(); i++)
	{
		sprintf(&cryption[i], "%c", atoi(vec.at(i).c_str()));
	}

	int outlen;
	EVP_CipherInit_ex(&ctx, EVP_rc4(), NULL, NULL, NULL, 0);
	EVP_CIPHER_CTX_set_key_length(&ctx, key_.length());
	EVP_CipherInit_ex(&ctx, NULL, NULL, (unsigned char*)key_.c_str(), NULL, 0);

	if(!EVP_CipherUpdate(&ctx, (unsigned char*)decode.c_str(), &outlen, (unsigned char*)cryption, strlen(cryption)))
	{
		LOG4CXX_ERROR(g_logger, "EvpCrypt::DoEvpDecode EVP_CipherUpdate failed " << strerror(errno));
		return false;
	}

	if(!EVP_CipherFinal_ex(&ctx,(unsigned char*)decode.c_str(), &outlen))
	{
		LOG4CXX_ERROR(g_logger, "EvpCrypt::DoEvpDecode EVP_CipherFinal_ex failed" << strerror(errno));
		return false;
	}

	LOG4CXX_INFO(g_logger, "EvpCrypt::DoEvpDecode success!");
	return true;
}

bool EvpCrypt::DoEvpEncrypt(const std::string& decode, std::string& encryption)
{
	int outlen;
	char cryption[PWDLEN];

	EVP_CipherInit_ex(&ctx, EVP_rc4(), NULL, NULL, NULL, 1);
	EVP_CIPHER_CTX_set_key_length(&ctx, key_.length());
	EVP_CipherInit_ex(&ctx, NULL, NULL, (unsigned char*)key_.c_str(), NULL, 1);

	if(!EVP_CipherUpdate(&ctx, (unsigned char*)cryption, &outlen, (unsigned char*)decode.c_str(), decode.length()))
	{
		LOG4CXX_ERROR(g_logger, "EvpCrypt::DoEvpEncrypt EVP_CipherUpdate failed " << strerror(errno));
		return false;
	}

	if(!EVP_CipherFinal_ex(&ctx,(unsigned char*)cryption, &outlen))
	{
		LOG4CXX_ERROR(g_logger, "EvpCrypt::DoEvpEncrypt EVP_CipherFinal_ex failed" << strerror(errno));
		return false;
	}

	for(unsigned int i = 0; i < strlen(cryption); i++)
	{
		char tem[5] = {0};
		sprintf(tem, "%d,", cryption[i]);
		encryption +=tem;
	}

	LOG4CXX_INFO(g_logger, "EvpCrypt::DoEvpEncrypt success!" );
	return true;
}
