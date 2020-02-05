#include "net/WebRequestPS4.h"

tbsg::WebRequest::WebRequest()
{

}

static void printSslError(SceInt32 sslErr, SceUInt32 sslErrDetail)
{
	switch (sslErr) {
	case (SCE_HTTPS_ERROR_CERT):			/* Verify error */
		/* Internal error at verifying certificate*/
		if (sslErrDetail & SCE_HTTPS_ERROR_SSL_INTERNAL) {
			printf("ssl verify error: unexpcted error\n");
		}
		/* Error of server certificate or CA certificate */
		if (sslErrDetail & SCE_HTTPS_ERROR_SSL_INVALID_CERT) {
			printf("ssl verify error: invalid server cert or CA cert\n");
		}
		/* Server hostname and server certificate are mismatched*/
		if (sslErrDetail & SCE_HTTPS_ERROR_SSL_CN_CHECK) {
			printf("ssl verify error: invalid server hostname\n");
		}
		/* Server certificate or CA certificate is expired.*/
		if (sslErrDetail & SCE_HTTPS_ERROR_SSL_NOT_AFTER_CHECK) {
			printf("ssl verify error: server cert or CA cert had expired\n");
		}
		/* Server certificate or CA certificate is before validated.*/
		if (sslErrDetail & SCE_HTTPS_ERROR_SSL_NOT_BEFORE_CHECK) {
			printf("ssl verify error: server cert or CA cert isn't validated yet.\n");
		}
		/* Unknown CA error */
		if (sslErrDetail & SCE_HTTPS_ERROR_SSL_UNKNOWN_CA) {
			printf("ssl verify error: unknown CA\n");
		}
		break;
	case (SCE_HTTPS_ERROR_HANDSHAKE):		/* fail to ssl-handshake  */
		printf("ssl error: handshake error\n");
		break;
	case (SCE_HTTPS_ERROR_IO):				/* Error of Socket IO */
		printf("ssl error: io error\n");
		break;
	case (SCE_HTTP_ERROR_OUT_OF_MEMORY):	/* Out of memory*/
		printf("ssl error: out of memory\n");
		break;
	case (SCE_HTTPS_ERROR_INTERNAL):		/* Unexpected Internal Error*/
		printf("ssl error: unexpcted error\n");
		break;
	default:
		break;
	}
	return;
}

static void printSslCertInfo(int libsslCtxId, SceSslCert *sslCert)
{
	SceInt32 ret;
	SceUChar8 *sboData = NULL;
	SceSize sboLen, counter;

	ret = sceSslGetSerialNumber(libsslCtxId, sslCert, NULL, &sboLen);
	if (ret < 0) {
		printf("sceSslGetSerialNumber() returns 0x%x\n", ret);
	}
	else {
		sboData = (SceUChar8*)malloc(sboLen);
		if (sboData != NULL) {
			ret = sceSslGetSerialNumber(libsslCtxId, sslCert, sboData, &sboLen);
			if (ret < 0) {
				printf("sceSslGetSerialNumber() returns 0x%x\n", ret);
			}
			else {
				printf("Serial number=");
				for (counter = 0; counter < sboLen; counter++) {
					printf("%02X", sboData[counter]);
				}
				printf("\n");
			}
			free(sboData);
		}
	}
}



static SceInt32 sslCallback(int libsslCtxId, unsigned int verifyErr, SceSslCert * const sslCert[], int certNum, void *userArg)
{
	SceInt32 i;
	(void)userArg;

	printf("Ssl callback:\n");
	printf("\tbase tmpl[%x]\n", (*(SceInt32*)(userArg)));

	if (verifyErr != 0) {
		printSslError((SceInt32)SCE_HTTPS_ERROR_CERT, verifyErr);
	}
	for (i = 0; i < certNum; i++) {
		printSslCertInfo(libsslCtxId, sslCert[i]);
	}
	if (verifyErr == 0) {
		return SCE_OK;
	}
	else {
		return -1;
	}
}

bool tbsg::WebRequest::SetupWebConnection(const ptl::string& baseWebURL)
{
	return false;
}

tbsg::WebRequestResult tbsg::WebRequest::RequestWebData(const ptl::string& webDomain, const ptl::string& subDirectory, const tbsg::WebMethod method, const ptl::vector<tbsg::postDataPair> arguments)
{
	SceInt32 ret;
	//SceInt32 returnCode = SCE_OK;
	char* formData;
	size_t formDataLen;
	int libnetMemId = 0;
	int libsslCtxId = 0;
	int libhttpCtxId = 0;
	tbsg::WebRequestResult result{};

	ret = netInit();
	if (ret < 0) {
		goto term;
	}
	libnetMemId = ret;

	if (enableSSL)
	{
		ret = sceSslInit(sslHeapSize);
		if (ret < 0) {
			printf("sceSslInit() error: 0x%08X\n", ret);
			goto net_term;
		}
		libsslCtxId = ret;
	}

	ret = sceHttpInit(libnetMemId, libsslCtxId, httpHeapSize);
	if (ret < 0) {
		printf("sceHttpInit() error: 0x%08X\n", ret);
		if (enableSSL)
		{
			goto ssl_term;
		}
		else
		{
			goto net_term;
		}
	}
	libhttpCtxId = ret;

	// Message for SDK sample auto test
	printf("## api_libhttp/post: INIT SUCCEEDED ##\n");

	formData = ConvertArgumentsToString(&formDataLen, arguments);
	if (formData == nullptr) {
		goto ssl_term;
	}

	if (method == WebMethod::GET)
	{
		ptl::string fullURL = subDirectory;
		const ptl::string argumentData = formData;
		if (!argumentData.empty())
		{
			fullURL += "?" + argumentData;
			GetWebRequest(libhttpCtxId, fullURL, method, result, formData, formDataLen);
		}
		
	}
	else
	{
		GetWebRequest(libhttpCtxId, subDirectory, method, result, formData, formDataLen);
	}

	free(formData);

	ret = sceHttpTerm(libhttpCtxId);
	if (ret < 0) {
		printf("[WebRequestPS4] sceHttpEnd() error: 0x%08X\n", ret);
	}

	if (enableSSL)
	{
	ssl_term:
		ret = sceSslTerm(libsslCtxId);
		if (ret < 0) {
			printf("[WebRequestPS4] sceSslEnd() error: 0x%08X\n", ret);
		}

	}
net_term:
	netShutdown(libnetMemId);
term:
	// Message for SDK sample auto test
	printf("[WebRequestPS4] ## api_libhttp/post: FINISHED ##\n");
	return result;
}

int tbsg::WebRequest::netInit()
{
	int ret;
	int libnetMemId;

	/* libnet */
	ret = sceNetInit();
	if (ret < 0) {
		printf("[WebRequestPS4] %s,%d ret=%x\n", __FUNCTION__, __LINE__, ret);
		goto error;
	}
	ret = sceNetPoolCreate("simple", netHeapSize, 0);
	if (ret < 0) {
		goto net_term;
	}
	libnetMemId = ret;

	return libnetMemId;

net_term:
	sceNetTerm();
error:
	return ret;
}

char* tbsg::WebRequest::ConvertArgumentsToString(size_t* size, const ptl::vector<tbsg::postDataPair>& pairs)
{
	char* buf = nullptr;
	size_t bufSize = 0;
	int i;
	int ret;
	const size_t postDataSize = pairs.size();

	for (i = 0; i < postDataSize; i++) {
		size_t reqkey;
		size_t reqval;
		char* extbuf = nullptr;
		size_t extsize;
		ret = sceHttpUriEscape(nullptr, &reqkey, 0, pairs[i].key.c_str());
		if (ret < 0) {
			printf("[WebRequestPS4] sceHttpUriEscape() error : 0x%08X\n", ret);
			goto error;
		}
		ret = sceHttpUriEscape(nullptr, &reqval, 0, pairs[i].value.c_str());
		if (ret < 0) {
			printf("[WebRequestPS4] sceHttpUriEscape() error : 0x%08X\n", ret);
			goto error;
		}
		extsize = bufSize + reqkey + reqval;
		extbuf = (char*)realloc(buf, extsize);
		if (extbuf == nullptr) {
			goto error;
		}
		buf = extbuf;
		extbuf += bufSize;
		ret = sceHttpUriEscape(extbuf, nullptr, reqkey, pairs[i].key.c_str());
		if (ret < 0) {
			printf("[WebRequestPS4] sceHttpUriEscape() error : 0x%08X\n", ret);
			goto error;
		}
		extbuf += reqkey - 1;
		*extbuf = '=';
		extbuf++;
		ret = sceHttpUriEscape(extbuf, nullptr, reqval, pairs[i].value.c_str());
		if (ret < 0) {
			printf("[WebRequestPS4] sceHttpUriEscape() error : 0x%08X\n", ret);
			goto error;
		}
		extbuf += reqval - 1;
		*extbuf = '&';
		bufSize = extsize;
	}
	buf[bufSize - 1] = '\0';
	if (size != nullptr) {
		*size = bufSize - 1;
	}
error:
	return buf;
	return nullptr;
}

int tbsg::WebRequest::GetWebRequest(int libhttpCtxId, const ptl::string& webDomain, const tbsg::WebMethod& method, tbsg::WebRequestResult& resultToPasteIN, const char* postFormData, size_t postFormLen)
{
	int ret = 0;
	int tmplId = 0;
	int connId = 0;
	int reqId = 0;

	uint64_t	contentLength;
	SceBool		finFlag = SCE_FALSE;
	unsigned char recvBuf[1500];

	ret = sceHttpCreateTemplate(libhttpCtxId, testUserAgent.c_str(), SCE_HTTP_VERSION_1_1, SCE_TRUE);
	if (ret < 0) {
		printf("[WebRequestPS4] sceHttpCreateTemplate() error: 0x%08X\n", ret);
		goto error;
	}
	tmplId = ret;

	if (enableSSL)
	{
		ret = sceHttpsDisableOption(tmplId, SCE_HTTPS_FLAG_SERVER_VERIFY);
		if (ret < 0) {
			printf("sceHttpsDisableOption() error: 0x%08X\n", ret);
			goto error;
		}

		/* Register SSL callback */
		ret = sceHttpsSetSslCallback(tmplId, sslCallback, (void*)&tmplId);
		if (ret < 0) {
			printf("sceHttpsSetSslCallback() error: 0x%08X\n", ret);
			goto error;
		}
	}

	ret = sceHttpCreateConnectionWithURL(tmplId, webDomain.c_str(), false);
	if (ret < 0) {
		printf("[WebRequestPS4] sceHttpCreateConnectionWithURL() error: 0x%08X\n", ret);
		goto error;
	}
	connId = ret;

	if (method == WebMethod::POST) 
	{
		ret = sceHttpCreateRequestWithURL(connId, SCE_HTTP_METHOD_POST, webDomain.c_str(), postFormLen);
		if (ret < 0) {
			printf("[WebRequestPS4] sceHttpCreateRequestWithURL() error: 0x%08X\n", ret);
			goto error;
		}
		reqId = ret;

		ret = sceHttpAddRequestHeader(reqId, "Content-Type", "application/x-www-form-urlencoded", SCE_HTTP_HEADER_OVERWRITE);
		if (ret < 0) {
			printf("[WebRequestPS4] sceHttpAddRequestHeader() error: 0x%08X\n", ret);
			goto error;
		}
	}
	else
	{
		ret = sceHttpCreateRequestWithURL(connId, SCE_HTTP_METHOD_GET, webDomain.c_str(), 0);
		if (ret < 0) {
			printf("[WebRequestPS4] sceHttpCreateRequestWithURL() error: 0x%08X\n", ret);
			goto error;
		}
		reqId = ret;

		ret = sceHttpAddRequestHeader(reqId, "Content-Type", "application/x-www-form-urlencoded", SCE_HTTP_HEADER_OVERWRITE);
		if (ret < 0) {
			printf("[WebRequestPS4] sceHttpAddRequestHeader() error: 0x%08X\n", ret);
			goto error;
		}
	}

	if (method == WebMethod::POST)
	{
		ret = sceHttpSendRequest(reqId, postFormData, postFormLen);
	}
	else
	{
		ret = sceHttpSendRequest(reqId, NULL, 0);
	}
	if (ret < 0) {
		printf("[WebRequestPS4] sceHttpSendRequest() error: 0x%08X\n", ret);
		goto error;
	}

	ret = sceHttpGetStatusCode(reqId, &resultToPasteIN.statusCode);
	if (ret < 0) {
		printf("[WebRequestPS4] sceHttpGetStatusCode() error: 0x%08X\n", ret);
		goto error;
	}
	printf("response code = %d\n", resultToPasteIN.statusCode);

	if (resultToPasteIN.statusCode == 200) {
		int contentLengthType;
		ret = sceHttpGetResponseContentLength(reqId, &contentLengthType, &contentLength);
		if (ret < 0) {
			printf("[WebRequestPS4] sceHttpGetContentLength() error: 0x%08X\n", ret);
			goto error;
		}
		else {
			if (contentLengthType == SCE_HTTP_CONTENTLEN_EXIST) {
				printf("Content-Length = %lu\n", contentLength);
			}
		}
		while (finFlag != SCE_TRUE) {
			memset(recvBuf, 0, sizeof(recvBuf));
			ret = sceHttpReadData(reqId, recvBuf, sizeof(recvBuf));
			if (ret < 0) {
				goto error;
			}
			else if (ret == 0) {
				finFlag = SCE_TRUE;
			}
			else {
				printf("%s", recvBuf);
				ptl::string str(recvBuf, recvBuf + sizeof recvBuf);
				resultToPasteIN.content += str;
			}
			printf("\n [WebRequestPS4] sceHttpReadData() read %d bytes\n", ret);
		}
	}


error:
	if (reqId > 0) {
		ret = sceHttpDeleteRequest(reqId);
		if (ret < 0) {
			printf("[WebRequestPS4] sceHttpDeleteRequest() error: 0x%08X\n", ret);
		}
	}
	if (connId > 0) {
		ret = sceHttpDeleteConnection(connId);
		if (ret < 0) {
			printf("[WebRequestPS4] sceHttpDeleteConnection() error: 0x%08X\n", ret);
		}
	}
	if (tmplId > 0) {
		ret = sceHttpDeleteTemplate(tmplId);
		if (ret < 0) {
			printf("[WebRequestPS4] sceHttpDeleteTemplate() error: 0x%08X\n", ret);
		}
	}
	return ret;
}

int tbsg::WebRequest::netShutdown(int libnetMemId)
{
	SceInt32 ret;

	/* libnet */
	ret = sceNetPoolDestroy(libnetMemId);
	if (ret < 0) {
		printf("[WebRequestPS4] %s,%d ret=%x\n", __FUNCTION__, __LINE__, ret);
	}
	ret = sceNetTerm();
	if (ret < 0) {
		printf("[WebRequestPS4] %s,%d ret=%x\n", __FUNCTION__, __LINE__, ret);
	}

	return ret;
}



