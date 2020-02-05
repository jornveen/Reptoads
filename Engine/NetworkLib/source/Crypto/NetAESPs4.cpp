#include "Crypto/NetAES.h"
#include <libsecure.h>
#include <memory>
#include <random>
#include <functional>
#include <algorithm>
#include <iostream>
#include <iterator>
#include <climits>
#include <stdio.h>
#include <cstring>

namespace net
{
	NetAES::NetAES()
	{
		this->key = GenerateKey(keyLength);
	}

	NetAES::NetAES(NetAESKey key)
	{
		this->key = key;
	}

	void NetAES::Encrypt(const unsigned char * data, const size_t dataSize, std::unique_ptr<unsigned char[]>& ivOut, std::unique_ptr<unsigned char[]>& encryptedData, size_t & encryptedDataSize, size_t& paddingSize)
	{
		unsigned char* paddedData = new unsigned char[dataSize];
		memcpy(paddedData, data, dataSize);

		paddedData = AddPadding(paddedData, dataSize, paddingSize);
		size_t paddedDataSize = dataSize + paddingSize;

		SceLibSecureBlock memBlkCxt = { nullptr, 0 }, memBlkIv = { nullptr, 0 }, *ivPtr = nullptr;
		SceLibSecureMessage messageBlk = { nullptr, 0 };
		SceLibSecurePaddingOption padding = { SCE_LIBSECURE_PADDING_NONE };
		SceLibSecureErrorType err = SCE_LIBSECURE_OK;
		off_t blockNumber = 0;
		size_t memSize = 0;
		unsigned int i = 0;

		SceLibSecureSymmetricKey aesKey{ key.bitSize >> 3, key.key.get() };

		err = sceLibSecureCryptographyGetBlockSize(SCE_LIBSECURE_CIPHER_AES, (SceLibSecureCipherKey*)&aesKey, &padding, &memSize);
		if (err != SCE_LIBSECURE_OK)
		{
			printf("sceLibSecureCryptographyGetBlockSize() failed (%s:%d) error code [%i]\n", __FUNCTION__, __LINE__, err);
		}

		auto ivData = GenerateIV(ivLength);

		memBlkIv.pointer = ivData.get();
		memBlkIv.length = ivLength >> 3;

		ivPtr = &memBlkIv;

		err = sceLibSecureCryptographyGetContextSize(SCE_LIBSECURE_CIPHER_AES, SCE_LIBSECURE_BLOCKCIPHERMODE_CBC, (SceLibSecureCipherKey*)&aesKey, 0, &memSize);
		if (err != SCE_LIBSECURE_OK)
		{
			printf("sceLibSecureCryptographyGetContextSize() failed (%s:%d) error code [%i]\n", __FUNCTION__, __LINE__, err);
		}

		memBlkCxt.pointer = malloc(memSize);
		memset(memBlkCxt.pointer, 0, sizeof(char)*memSize);
		memBlkCxt.length = memSize;

		err = sceLibSecureCryptographySetContext(&memBlkCxt, SCE_LIBSECURE_CIPHER_AES, SCE_LIBSECURE_BLOCKCIPHERMODE_CBC, (SceLibSecureCipherKey*)&aesKey, 0, ivPtr, blockNumber);
		if (err != SCE_LIBSECURE_OK)
		{
			printf("sceLibSecureCryptographySetContext() failed (%s:%d) error code [%i]\n", __FUNCTION__, __LINE__, err);
		}

		messageBlk.pointer = new unsigned char[paddedDataSize];
		memcpy(messageBlk.pointer, paddedData, paddedDataSize);
		messageBlk.length = paddedDataSize;

		err = sceLibSecureCryptographyEncrypt(&memBlkCxt, &messageBlk, &padding);
		if (err != SCE_LIBSECURE_OK)
		{
			printf("sceLibSecureCryptographyEncrypt() failed (%s:%d) error code [%i]\n", __FUNCTION__, __LINE__, err);
		}

		err = sceLibSecureCryptographyDeleteContext(&memBlkCxt);
		if (err != SCE_LIBSECURE_OK)
		{
			printf("sceLibSecureCryptographyDeleteContext() failed (%s:%d) error code [%i]\n", __FUNCTION__, __LINE__, err);
		}

		ivOut = std::unique_ptr<unsigned char[]>{ ivData.release() };
		encryptedData = std::unique_ptr<unsigned char[]>{ static_cast<unsigned char*>(messageBlk.pointer) };
		encryptedDataSize = messageBlk.length;

		free(memBlkCxt.pointer);

		delete[] paddedData;
	}

	void NetAES::Decrypt(const unsigned char * data, const size_t dataSize, unsigned char* ivIn, std::unique_ptr<unsigned char[]>& decryptedData, size_t & decryptedDataSize, size_t paddingSize)
	{
		SceLibSecureBlock memBlkCxt = { nullptr, 0 }, memBlkIv = { nullptr, 0 }, *ivPtr = nullptr;
		SceLibSecureMessage messageBlk = { nullptr, 0 };
		SceLibSecurePaddingOption padding = { SCE_LIBSECURE_PADDING_NONE };
		SceLibSecureErrorType err = SCE_LIBSECURE_OK;
		off_t blockNumber = 0;
		size_t memSize = 0;
		unsigned int i = 0;

		SceLibSecureSymmetricKey aesKey{ key.bitSize >> 3, key.key.get() };

		err = sceLibSecureCryptographyGetBlockSize(SCE_LIBSECURE_CIPHER_AES, (SceLibSecureCipherKey*)&aesKey, &padding, &memSize);
		if (err != SCE_LIBSECURE_OK)
		{
			printf("sceLibSecureCryptographyGetBlockSize() failed (%s:%d) error code [%i]\n", __FUNCTION__, __LINE__, err);
		}

		memBlkIv.pointer = ivIn;
		memBlkIv.length = ivLength >> 3;

		ivPtr = &memBlkIv;

		err = sceLibSecureCryptographyGetContextSize(SCE_LIBSECURE_CIPHER_AES, SCE_LIBSECURE_BLOCKCIPHERMODE_CBC, (SceLibSecureCipherKey*)&aesKey, 0, &memSize);
		if (err != SCE_LIBSECURE_OK)
		{
			printf("sceLibSecureCryptographyGetContextSize() failed (%s:%d) error code [%i]\n", __FUNCTION__, __LINE__, err);
		}

		memBlkCxt.pointer = malloc(memSize);
		memset(memBlkCxt.pointer, 0, sizeof(char)*memSize);
		memBlkCxt.length = memSize;

		err = sceLibSecureCryptographySetContext(&memBlkCxt, SCE_LIBSECURE_CIPHER_AES, SCE_LIBSECURE_BLOCKCIPHERMODE_CBC, (SceLibSecureCipherKey*)&aesKey, 0, ivPtr, blockNumber);
		if (err != SCE_LIBSECURE_OK)
		{
			printf("sceLibSecureCryptographySetContext() failed (%s:%d) error code [%i]\n", __FUNCTION__, __LINE__, err);
		}

		messageBlk.pointer = new unsigned char[dataSize];
		memcpy(messageBlk.pointer, data, dataSize);
		messageBlk.length = dataSize;

		err = sceLibSecureCryptographyDecrypt(&memBlkCxt, &messageBlk, &padding);
		if (err != SCE_LIBSECURE_OK)
		{
			printf("sceLibSecureCryptographyDecrypt() failed (%s:%d) error code [%i]\n", __FUNCTION__, __LINE__, err);
		}

		err = sceLibSecureCryptographyDeleteContext(&memBlkCxt);
		if (err != SCE_LIBSECURE_OK)
		{
			printf("sceLibSecureCryptographyDeleteContext() failed (%s:%d) error code [%i]\n", __FUNCTION__, __LINE__, err);
		}

		messageBlk.pointer = RemovePadding(static_cast<unsigned char*>(messageBlk.pointer), messageBlk.length, paddingSize);

		decryptedData = std::unique_ptr<unsigned char[]>{ static_cast<unsigned char*>(messageBlk.pointer) };
		decryptedDataSize = messageBlk.length - paddingSize;

		free(memBlkCxt.pointer);
	}

	NetAESKey NetAES::GenerateKey(const int keySize)
	{
		return NetAESKey{ static_cast<size_t>(keySize), std::shared_ptr<unsigned char>{ GenerateBytes(keySize >> 3).release(), [](unsigned char *p) { delete[] p; } } };
	}

	std::unique_ptr<unsigned char[]> NetAES::GenerateIV(const int ivSize)
	{
		return GenerateBytes(ivSize >> 3);
	}

	std::unique_ptr<unsigned char[]> NetAES::GenerateBytes(const int size)
	{
		std::random_device r;
		std::seed_seq seed{ r(), r(), r(), r(), r(), r(), r(), r() };
		std::mt19937 gen(seed);
		std::uniform_int_distribution<> dis(0, UCHAR_MAX);

		auto rand = [&gen, &dis]() { return static_cast<unsigned char>(dis(gen)); };

		std::unique_ptr<unsigned char[]> key = std::unique_ptr<unsigned char[]>{ new unsigned char[size] };

		std::generate_n(key.get(), static_cast<size_t>(size), rand);

		return key;
	}

	unsigned char* NetAES::AddPadding(unsigned char* data, const size_t dataSize, size_t& paddingSize)
	{
		int remaining = blockSize - (dataSize % blockSize);

		if (remaining < blockSize)
		{
			unsigned char* newData = new unsigned char[dataSize + remaining];
			memcpy(newData, data, dataSize);

			auto remainderBytes = GenerateBytes(remaining);
			memcpy(&newData[dataSize], remainderBytes.get(), remaining);

			paddingSize = static_cast<size_t>(remaining);
			delete[] data;
			data = newData;
		}
		else
		{
			paddingSize = 0;
		}

		return data;
	}

	unsigned char* NetAES::RemovePadding(unsigned char * data, const size_t dataSize, size_t paddingSize)
	{
		if (paddingSize > 0)
		{
			unsigned char* newData = new unsigned char[dataSize - paddingSize];
			memcpy(newData, data, dataSize - paddingSize);
			delete[] data;
			data = newData;
		}

		return data;
	}
}