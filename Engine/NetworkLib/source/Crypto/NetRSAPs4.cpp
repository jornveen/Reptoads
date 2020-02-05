#include "Crypto/NetRSA.h"
#include <libsecure.h>

namespace net
{
	NetRSA::NetRSA()
	{
		GenerateKeyPair(keyLength, publicKey, privateKey);
	}

	NetRSA::NetRSA(NetRSAKey privateKey, NetRSAKey publicKey)
	{
		this->privateKey = privateKey;
		this->publicKey = publicKey;

		Equalize(this->publicKey);
		Equalize(this->privateKey);

	}

	void NetRSA::GenerateKeyPair(const int keySize, NetRSAKey & publicKey, NetRSAKey & privateKey)
	{
		SceLibSecureAsymmetricKey akey = { static_cast<size_t>(keySize >> 3), nullptr, nullptr };

		size_t memSize;
		int err = sceLibSecureCryptographyGetKeySize(SCE_LIBSECURE_CIPHER_RSA, akey.key_size, &memSize);
		if (err != SCE_LIBSECURE_OK)
		{
			printf("sceLibSecureCryptographyGetKeySize() failed (%s:%d) error code 0x%x\n", __FUNCTION__, __LINE__, err);
			return;
		}

		void* rsaPrivateKey = (void*)malloc(memSize);
		void* rsaPublicKey = (void*)malloc(memSize);
		if (rsaPrivateKey == nullptr || rsaPublicKey == nullptr)
		{
			printf("Memory already malloc'd (%s:%d)", __FUNCTION__, __LINE__);
			err = (SceLibSecureErrorType)-1;
			return;
		}

		akey.private_key = rsaPrivateKey;
		akey.public_key = rsaPublicKey;

		err = sceLibSecureCryptographyGenerateKey(SCE_LIBSECURE_CIPHER_RSA, (SceLibSecureCipherKey*)&akey);
		if (err != SCE_LIBSECURE_OK)
		{
			printf("sceLibSecureCryptographyGenerateKey() failed (%s:%d) error code 0x%x\n", __FUNCTION__, __LINE__, err);
		}

		size_t halfKey = akey.key_size;

		std::shared_ptr<unsigned char> modulusBuffer = std::shared_ptr<unsigned char>{ new unsigned char[halfKey], [](unsigned char *p) { delete[] p; } };
		std::shared_ptr<unsigned char> publicExponentBuffer = std::shared_ptr<unsigned char>{ new unsigned char[halfKey], [](unsigned char *p) { delete[] p; } };
		std::shared_ptr<unsigned char> privateExponentBuffer = std::shared_ptr<unsigned char>{ new unsigned char[halfKey], [](unsigned char *p) { delete[] p; } };

		memcpy(modulusBuffer.get(), static_cast<unsigned char*>(akey.public_key), halfKey * sizeof(unsigned char));
		memcpy(publicExponentBuffer.get(), &static_cast<unsigned char*>(akey.public_key)[halfKey], halfKey * sizeof(unsigned char));
		memcpy(privateExponentBuffer.get(), &static_cast<unsigned char*>(akey.private_key)[halfKey], halfKey * sizeof(unsigned char));
		printf("1: %u, 2: %u\n", publicExponentBuffer.get()[0], privateExponentBuffer.get()[0]);

		publicKey.bitSize = keySize;
		publicKey.modBitSize = 0;
		publicKey.crtBitSize = 0;
		publicKey.modulus = modulusBuffer;
		publicKey.exponent = publicExponentBuffer;

		privateKey.bitSize = keySize;
		privateKey.modBitSize = 0;
		privateKey.crtBitSize = 0;
		privateKey.modulus = modulusBuffer;
		privateKey.exponent = privateExponentBuffer;

		free(rsaPrivateKey);
		free(rsaPublicKey);
	}



	void NetRSA::Encrypt(const unsigned char * data, const size_t dataSize, std::unique_ptr<unsigned char[]>& encryptedData, size_t& encryptedDataSize)
	{
		SceLibSecureBlock memBlockContext = { nullptr, 0 };
		SceLibSecureMessage messageBlock = { nullptr, 0 };
		SceLibSecurePaddingOption padding = { SCE_LIBSECURE_PADDING_PKCS1 };
		SceLibSecureErrorType err = SCE_LIBSECURE_OK;
		size_t memSize = 0;

		size_t keySizeBytes = publicKey.bitSize >> 3;

		unsigned char* publicKeyData = new unsigned char[keySizeBytes * 2];
		memcpy(publicKeyData, &publicKey.modulus.get()[0], keySizeBytes);
		memcpy(&publicKeyData[keySizeBytes], &publicKey.exponent.get()[0], keySizeBytes);

		SceLibSecureAsymmetricKey key{
			publicKey.bitSize >> 3,
			nullptr,
			publicKeyData
		};

		err = sceLibSecureCryptographyGetContextSize(SCE_LIBSECURE_CIPHER_RSA, SCE_LIBSECURE_BLOCKCIPHERMODE_ECB,
			(SceLibSecureCipherKey*)&key, 0, &memSize);

		memBlockContext.pointer = malloc(memSize);

		memset(memBlockContext.pointer, 0, sizeof(char)*memSize);
		memBlockContext.length = memSize;

		err = sceLibSecureCryptographySetContext(&memBlockContext, SCE_LIBSECURE_CIPHER_RSA, SCE_LIBSECURE_BLOCKCIPHERMODE_ECB,
			(SceLibSecureCipherKey*)&key, 0, nullptr, 0);

		messageBlock.pointer = new unsigned char[dataSize];
		memcpy(messageBlock.pointer, data, dataSize);
		messageBlock.length = dataSize;

		err = sceLibSecureCryptographyMessagePaddingSize(&memBlockContext, &messageBlock, &padding, &memSize);

		delete[] static_cast<unsigned char*>(messageBlock.pointer);
		messageBlock.pointer = new unsigned char[memSize + dataSize];

		memset(messageBlock.pointer, 0, sizeof(char)*memSize + dataSize);
		messageBlock.length = memSize + dataSize;

		memcpy(messageBlock.pointer, data, dataSize);

		err = sceLibSecureCryptographyMessagePadding(&memBlockContext, &messageBlock, &padding, dataSize);


		err = sceLibSecureCryptographyEncrypt(&memBlockContext, &messageBlock, &padding);

		encryptedData = std::unique_ptr<unsigned char[]>{ static_cast<unsigned char*>(messageBlock.pointer) };
		encryptedDataSize = messageBlock.length;

		free(memBlockContext.pointer);
		delete[] publicKeyData;
		err = sceLibSecureCryptographyDeleteContext(&memBlockContext);
	}

	void NetRSA::Decrypt(const unsigned char * data, const size_t dataSize, std::unique_ptr<unsigned char[]>& decryptedData, size_t& decryptedDataSize)
	{
		SceLibSecureBlock memBlockContext = { nullptr, 0 };
		SceLibSecureMessage messageBlock = { nullptr, 0 };
		SceLibSecurePaddingOption padding = { SCE_LIBSECURE_PADDING_PKCS1 };
		SceLibSecureErrorType err = SCE_LIBSECURE_OK;
		size_t memSize = 0;

		size_t keySizeBytes = privateKey.bitSize >> 3;

		unsigned char* publicKeyData = new unsigned char[keySizeBytes * 2];
		unsigned char* pvt = new unsigned char[keySizeBytes * 2];
		memcpy(publicKeyData, &publicKey.modulus.get()[0], keySizeBytes);
		memcpy(&publicKeyData[keySizeBytes], &publicKey.exponent.get()[0], keySizeBytes);

		memcpy(pvt, &privateKey.modulus.get()[0], keySizeBytes);
		memcpy(&pvt[keySizeBytes], &privateKey.exponent.get()[0], keySizeBytes);

		SceLibSecureAsymmetricKey key{
			publicKey.bitSize >> 3,
			pvt,
			publicKeyData
		};

		err = sceLibSecureCryptographyGetContextSize(SCE_LIBSECURE_CIPHER_RSA, SCE_LIBSECURE_BLOCKCIPHERMODE_ECB,
			(SceLibSecureCipherKey*)&key, 0, &memSize);

		memBlockContext.pointer = malloc(memSize);

		memset(memBlockContext.pointer, 0, sizeof(char)*memSize);
		memBlockContext.length = memSize;

		err = sceLibSecureCryptographySetContext(&memBlockContext, SCE_LIBSECURE_CIPHER_RSA, SCE_LIBSECURE_BLOCKCIPHERMODE_ECB,
			(SceLibSecureCipherKey*)&key, 0, nullptr, 0);

		messageBlock.pointer = new unsigned char[dataSize];
		memcpy(messageBlock.pointer, data, dataSize);
		messageBlock.length = dataSize;

		err = sceLibSecureCryptographyMessagePaddingSize(&memBlockContext, &messageBlock, &padding, &memSize);

		delete[] static_cast<unsigned char*>(messageBlock.pointer);

		messageBlock.pointer = new unsigned char[memSize + dataSize];

		memset(messageBlock.pointer, 0, sizeof(char)*memSize + dataSize);
		messageBlock.length = memSize + dataSize;

		memcpy(messageBlock.pointer, data, dataSize);

		err = sceLibSecureCryptographyMessagePadding(&memBlockContext, &messageBlock, &padding, dataSize);

		err = sceLibSecureCryptographyDecrypt(&memBlockContext, &messageBlock, &padding);

		err = sceLibSecureCryptographyMessageUnpadding(&memBlockContext, &messageBlock, &padding, &memSize);
		err = sceLibSecureCryptographyDeleteContext(&memBlockContext);

		decryptedData = std::unique_ptr<unsigned char[]>{ static_cast<unsigned char*>(messageBlock.pointer) };
		decryptedDataSize = memSize;

		free(memBlockContext.pointer);
		delete[] publicKeyData;
		delete[] pvt;



	}

	void NetRSA::Equalize(NetRSAKey& key)
	{

		if (key.bitSize < key.modBitSize)
		{
			std::shared_ptr<unsigned char> exponentBuffer = std::shared_ptr<unsigned char>{ new unsigned char[key.modBitSize >> 3], [](unsigned char *p) { delete[] p; } };

			size_t expBytes = key.bitSize >> 3;
			size_t modBytesDiff = (key.modBitSize >> 3) - expBytes;

			for (size_t i = 0; i < modBytesDiff; i++)
			{
				exponentBuffer.get()[i] = 0x00;
			}

			for (size_t i = modBytesDiff; i < modBytesDiff + expBytes; i++)
			{
				exponentBuffer.get()[i] = key.exponent.get()[i - modBytesDiff];
			}

			key.exponent = exponentBuffer;
			key.bitSize = (expBytes + modBytesDiff) << 3;
		}
	}
}
