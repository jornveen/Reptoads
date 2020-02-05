#pragma once
#include "memory/Containers.h"
#include "net/WebRequestHelper.h"


#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <kernel.h>
#include <libdbg.h>
#include <sceerror.h>
#include <scetypes.h>
#include <net.h>
#include <libhttp.h>
#include <libssl.h>

/*Most of the code comes from the sample_code\network\api_http\http_post\post project. */

namespace tbsg
{
	class WebRequest
	{
	public:
		WebRequest();
		~WebRequest() = default;

		bool SetupWebConnection(const ptl::string& baseWebURL);
		tbsg::WebRequestResult RequestWebData(const ptl::string& webDomain, const ptl::string& webDirectoryURL, const tbsg::WebMethod method, const ptl::vector<tbsg::postDataPair> arguments = {});
	private:
		const size_t sslHeapSize = (304 * 1024);
		const size_t httpHeapSize = (48 * 1024);
		const size_t netHeapSize = (16 * 1024);
		const std::string testUserAgent = "SimpleSample/1.00";
		bool enableSSL = true;

		int netInit();
		char* ConvertArgumentsToString(size_t* size, const ptl::vector<tbsg::postDataPair>& pairs);
		int GetWebRequest(int libhttpCtxId, const ptl::string& webDomain, const tbsg::WebMethod& method, tbsg::WebRequestResult& resultToPasteIN, const char* postFormData = nullptr, size_t postFormLen = 0);
		int netShutdown(int libnetMemId);
		//int GetRequestStatus(const ptl::shared_ptr<httplib::Response>);
		//ptl::string GetRequestBody(const ptl::shared_ptr<httplib::Response>);
	};


}