/*
 * Copyright (c) 2013 TRUSTONIC LIMITED
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the TRUSTONIC LIMITED nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <string.h>
#include <stdlib.h>
#include <wrapper.h>
#include <time.h>
#include <math.h>
#ifdef TIZEN
#include <string.h>
#endif

#include <curl/curl.h>

#include "logging.h"
#include "rootpaErrors.h"
#include "seclient.h"
#include "cacerts.h"

#define HTTP_CODE_MOVED                 301
#define HTTP_CODE_BAD_REQUEST           400
#define HTTP_CODE_NOT_FOUND             404
#define HTTP_CODE_METHOD_NOT_ALLOWED    405
#define HTTP_CODE_NOT_ACCEPTABLE        406
#define HTTP_CODE_REQUEST_TIMEOUT       408
#define HTTP_CODE_CONFLICT              409
#define HTTP_CODE_LENGTH_REQUIRED       411
#define HTTP_CODE_TOO_LONG              414
#define HTTP_CODE_UNSUPPORTED_MEDIA     415
#define HTTP_CODE_INVALID_DATA          422
#define HTTP_CODE_FAILED_DEPENDENCY     424
#define HTTP_CODE_INTERNAL_ERROR        500
#define HTTP_CODE_CMP_VERSION           501
#define HTTP_CODE_SERVICE_UNAVAILABLE   503
#define HTTP_CODE_HTTP_VERSION          505

#ifdef __DEBUG
#define NONEXISTENT_TEST_URL "http://10.255.255.253/"
#endif

#define CERT_PATH_MAX_LEN 256
#define CECERT_FILENAME "cacert.pem"
static char certificatePath_[CERT_PATH_MAX_LEN];
static char certificateFilePath_[CERT_PATH_MAX_LEN];
static long int SE_CONNECTION_DEFAULT_TIMEOUT=58L; // timeout after 58 seconds
static int MAX_ATTEMPTS=30; //30x0.3 = 9 seconds
#ifdef WIN32
	static const DWORD SLEEPTIME_MS=300; // 0.3 seconds
#else
	static const struct timespec SLEEPTIME={0,300*1000*1000}; // 0.3 seconds
#endif

rootpaerror_t httpCommunicate(const char* const inputP, const char** linkP, const char** relP, const char** commandP, httpMethod_t method);

#ifdef WIN32

	char* strcasestr(char const *s, char const *find)
	{
		char* pos;
		char* ret;
		char* ls=_strdup(s);
		char* lfind=_strdup(find);

		ls=_strlwr(ls);
		lfind=_strlwr(lfind);
		pos = strstr(ls, lfind);
		ret = pos == NULL ? NULL : (char *)(s + (pos-ls));
		free(ls);
		free(lfind);
		return ret;
	}


#endif

#ifdef TIZEN
#include <sys/types.h>
#include <string.h>
#include <ctype.h>
/*
 * Find the first occurrence of find in s, ignore case.
 */
char *
strcasestr(s, find)
	const char *s, *find;
{
	char c, sc;
	size_t len;

	if ((c = *find++) != 0) {
		c = tolower((unsigned char)c);
		len = strlen(find);
		do {
			do {
				if ((sc = *s++) == 0)
					return (NULL);
			} while ((char)tolower((unsigned char)sc) != c);
		} while (strncasecmp(s, find, len) != 0);
		s--;
	}
	return ((char *)s);
}

/*
 * Appends src to string dst of size siz (unlike strncat, siz is the
 * full size of dst, not space left).  At most siz-1 characters
 * will be copied.  Always NUL terminates (unless siz <= strlen(dst)).
 * Returns strlen(src) + MIN(siz, strlen(initial dst)).
 * If retval >= siz, truncation occurred.
 */
size_t
strlcat(char *dst, const char *src, size_t siz)
{
	register char *d = dst;
	register const char *s = src;
	register size_t n = siz;
	size_t dlen;

	/* Find the end of dst and adjust bytes left but don't go past end */
	while (n-- != 0 && *d != '\0')
		d++;
	dlen = d - dst;
	n = siz - dlen;

	if (n == 0)
		return(dlen + strlen(s));
	while (*s != '\0') {
		if (n != 1) {
			*d++ = *s;
			n--;
		}
		s++;
	}
	*d = '\0';

	return(dlen + (s - src));	/* count does not include NUL */
}

/*
 * Copy src to string dst of size siz.  At most siz-1 characters
 * will be copied.  Always NUL terminates (unless siz == 0).
 * Returns strlen(src); if retval >= siz, truncation occurred.
 */
size_t
strlcpy(char *dst, const char *src, size_t siz)
{
	register char *d = dst;
	register const char *s = src;
	register size_t n = siz;

	/* Copy as many bytes as will fit */
	if (n != 0 && --n != 0) {
		do {
			if ((*d++ = *s++) == 0)
				break;
		} while (--n != 0);
	}

	/* Not enough room in dst, add NUL and traverse rest of src */
	if (n == 0) {
		if (siz != 0)
			*d = '\0';		/* NUL-terminate dst */
		while (*s++)
			;
	}

	return(s - src - 1);	/* count does not include NUL */
}
#endif

rootpaerror_t httpPostAndReceiveCommand(const char* const inputP, const char** linkP, const char** relP, const char** commandP)
{
    LOGD("httpPostAndReceiveCommand %ld", (long int) inputP);
    return httpCommunicate(inputP, linkP, relP, commandP, httpMethod_POST);
}

rootpaerror_t httpPutAndReceiveCommand(const char* const inputP, const char** linkP, const char** relP, const char** commandP)
{
    LOGD("httpPutAndReceiveCommand %ld", (long int) inputP);
    if(NULL==inputP)
    {
        return ROOTPA_ERROR_ILLEGAL_ARGUMENT;
    }
    LOGD("%s", inputP);
    return httpCommunicate(inputP, linkP, relP, commandP, httpMethod_PUT);
}


rootpaerror_t httpGetAndReceiveCommand(const char** linkP, const char** relP, const char** commandP)
{
    LOGD("httpGetAndReceiveCommand");
    return httpCommunicate(NULL, linkP, relP, commandP, httpMethod_GET);
}

rootpaerror_t httpDeleteAndReceiveCommand(const char** linkP, const char** relP, const char** commandP)
{
    LOGD("httpDeleteAndReceiveCommand");
    return httpCommunicate(NULL, linkP, relP, commandP, httpMethod_DELETE);
}


typedef struct
{
    char*  memoryP;
    size_t    size;
} MemoryStruct;


typedef struct
{
    char*  linkP;
    size_t    linkSize;
    char*  relP;
    size_t    relSize;
} HeaderStruct;

typedef struct
{
    const char*      responseP;
    size_t           size;
    uint32_t         offset;
} ResponseStruct;


static size_t readResponseCallback(void *ptr, size_t size, size_t nmemb, void *userp)
{
    size_t totalSize=nmemb*size;
    size_t readSize;
    ResponseStruct* rspP=(ResponseStruct*) userp;
    LOGD(">>readResponseCallback %d %d %d\n", (int) totalSize, (int) rspP->size, rspP->offset);

    if(rspP->offset>=rspP->size) return 0;

    if(totalSize<((rspP->size)))
    {
        readSize=totalSize;
    }
    else
    {
        readSize=rspP->size;
    }

    memcpy(ptr, (rspP->responseP+rspP->offset), readSize);

    rspP->offset+=readSize;

    LOGD("<<readResponseCallback %d %d %d\n", (int) readSize, (int) rspP->size, rspP->offset);
    return readSize;
}

static size_t writeMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
    size_t realsize = size * nmemb;
    MemoryStruct* mem = (MemoryStruct *)userp;
    mem->memoryP = (char*)realloc(mem->memoryP, mem->size + realsize + 1);
    if (mem->memoryP == NULL) {
        /* out of memory! */
        LOGE("not enough memory (realloc returned NULL)\n");
        return 0; // returning anything different from what was passed to this function indicates an error
    }

    memcpy(&(mem->memoryP[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->memoryP[mem->size] = 0;

    return realsize;
}

#ifdef __DEBUG
int debug_function (CURL * curl_handle, curl_infotype info, char* debugMessageP, size_t debugMessageSize, void * extrabufferP)
{
    if(debugMessageP!=NULL && debugMessageSize!=0)
    {
        char* msgP=(char*)malloc(debugMessageSize+1);
        if(NULL==msgP)return 0;
        memcpy(msgP, debugMessageP, debugMessageSize);
        msgP[debugMessageSize]=0;
        LOGD("curl: %d %s",info, msgP);
        free(msgP);
    }
    else
    {
        LOGD("curl: no debug msg %d %d", info, (int) debugMessageSize);
    }
    return 0;
}
#endif

bool copyHeader(void *contents, size_t length, char** headerP)
{
    *headerP = (char *)malloc(length + 1);
    if (*headerP == NULL) {
        /* out of memory! */
        LOGE("not enough memory (malloc returned NULL)\n");
        return false;
    }

    memcpy(*headerP , contents, length);
    (*headerP)[length] = 0;
    return true;
}

//
// The header format is as follow
// Link <https://se.cgbe.trustonic.com:8443/activity/00000000-4455-6677-8899-aabbccddeeff>;rel="http://10.0.2.2/relation/system_info"
// parse out uri's specified in Link and rel
//
bool updateLinkAndRel(HeaderStruct* memP, void* ptr)
{
    char* startP=NULL;
    char* endP=NULL;

    // first update link

    startP=strcasestr((char*) ptr, "Link");
    if(NULL==startP) return false;

    startP=strstr(startP,"<");
    if(NULL==startP) return false;
    startP++;

    endP=strstr(startP,">");
    if(NULL==endP) return false;

    memP->linkSize=endP-startP;
    if(copyHeader(startP, memP->linkSize, &(memP->linkP))==false)
    {
        return false;
    }

    // then update rel, we will be successful even if it is not found

    startP=strcasestr(endP, "rel=");
    if(NULL==startP)
    {
        return true;
    }
    startP+=5; // sizeof "rel="

    endP=strstr(startP,"\"");
    if(NULL==endP)
    {
        return true;
    }
    memP->relSize=endP-startP;
    if(copyHeader(startP, memP->relSize, &(memP->relP))==false)
    {
        LOGE("could not copy rel, but since we are this far, continuing anyway");
    }

    return true;
}

static size_t writeHeaderCallback( void *ptr, size_t size, size_t nmemb, void *userp)
{
    size_t realSize = size * nmemb;
    HeaderStruct* memP = (HeaderStruct *)userp;

    if(realSize>=sizeof("Link:") && memcmp(ptr, "Link:", sizeof("Link:")-1)==0)
    {
        if(updateLinkAndRel(memP, ptr)==false)
        {
            LOGE("Problems in updating Link and rel");
        }
    }

    return realSize;
}

uint32_t shorter(uint32_t first, uint32_t second)
{
    return (first>second?second:first);
}

void setCertPath(const char* localPathP, const char* certPathP)
{
    memset(certificatePath_, 0, CERT_PATH_MAX_LEN);
    memset(certificateFilePath_, 0, CERT_PATH_MAX_LEN);

    if (certPathP!=NULL && (strlen(certPathP)+1)<CERT_PATH_MAX_LEN)
    {
#ifdef TIZEN
        strlcpy(certificatePath_, certPathP, sizeof(certificatePath_));
#else
/* Fix coverity */
//        strncpy(certificatePath_, certPathP, sizeof(certificatePath_));
        strncpy(certificatePath_, certPathP, sizeof(certificatePath_)-1);
#endif
    }

    if (localPathP!=NULL && (strlen(localPathP)+1+sizeof(CECERT_FILENAME))<CERT_PATH_MAX_LEN)
    {
#ifdef TIZEN
        strlcpy(certificateFilePath_, localPathP,sizeof(certificateFilePath_));
        strlcat(certificateFilePath_, "/",sizeof(certificateFilePath_));
#else
/* Fix coverity */
//        strncpy(certificateFilePath_, localPathP,sizeof(certificateFilePath_));
//        strncat(certificateFilePath_, "/",sizeof(certificateFilePath_));
        strncpy(certificateFilePath_, localPathP,sizeof(certificateFilePath_)-1);
        strncat(certificateFilePath_, "/",strlen("/"));
#endif
    }
#ifdef TIZEN
    strlcat(certificateFilePath_, CECERT_FILENAME,sizeof(certificateFilePath_));
#else
//    strncat(certificateFilePath_, CECERT_FILENAME,sizeof(certificateFilePath_));
    strncat(certificateFilePath_, CECERT_FILENAME,strlen(CECERT_FILENAME));
#endif
}
//
// TODO-refactor: saveCertFile is duplicate from saveFile in xmlMessageHandler.c, move these to common place
//
void saveCertFile(char* filePath, char* fileContent)
{
    FILE* fh;
    LOGD(">>saveCertFile %s", filePath);
    if ((fh = fopen(filePath, "w")) != NULL) // recreating the file every time, this is not the most efficient way, but ensures
	{                                        // the file is updated in case rootpa and the required content is updated
        fprintf(fh, "%s", fileContent);
        fclose(fh);
	}
    else
    {
        LOGE("saveCertFile no handle %s", filePath);
    }
    LOGD("<<saveCertFile");
}

bool setBasicOpt(CURL* curl_handle, MemoryStruct* chunkP, HeaderStruct* headerChunkP, const char* linkP,  struct curl_slist* headerListP)
{
    long int se_connection_timeout=SE_CONNECTION_DEFAULT_TIMEOUT;

    if(curl_easy_setopt(curl_handle, CURLOPT_URL, linkP)!=CURLE_OK)
    {
        LOGE("curl_easy_setopt CURLOPT_URL failed");
        return false;
    }

    /* reading response to memory instead of file */
    if(curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, writeMemoryCallback)!=CURLE_OK)
    {
        LOGE("curl_easy_setopt CURLOPT_WRITEFUNCTION failed");
        return false;
    }

    if(curl_easy_setopt(curl_handle, CURLOPT_HEADERFUNCTION, writeHeaderCallback)!=CURLE_OK)
    {
        LOGE("curl_easy_setopt CURLOPT_HEADERFUNCTION failed");
        return false;
    }

    if(curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *) chunkP)!=CURLE_OK)
    {
        LOGE("curl_easy_setopt CURLOPT_WRITEDATA failed");
        return false;
    }

    if(curl_easy_setopt(curl_handle, CURLOPT_WRITEHEADER, (void *) headerChunkP)!=CURLE_OK)
    {
        LOGE("curl_easy_setopt CURLOPT_WRITEHEADER failed");
        return false;
    }


    if(curl_easy_setopt(curl_handle, CURLOPT_HTTPHEADER, headerListP)!=CURLE_OK)
    {
        LOGE("curl_easy_setopt CURLOPT_HTTPHEADER failed");
        return false;
    }

    /* some servers don't like requests that are made without a user-agent
       field, so we provide one */
    if(curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "rpa/1.0")!=CURLE_OK)
    {
        LOGE("curl_easy_setopt CURLOPT_USERAGENT failed");
        return false;
    }

    if(curl_easy_setopt(curl_handle, CURLOPT_NOPROGRESS, 1L)!=CURLE_OK)
    {
        LOGE("curl_easy_setopt CURLOPT_USERAGENT failed");
        return false;
    }


    saveCertFile(certificateFilePath_, CA_CERTIFICATES);

    LOGD("curl_easy_setopt CURLOPT_CAINFO %s", certificateFilePath_);
    if(curl_easy_setopt(curl_handle, CURLOPT_CAINFO,  certificateFilePath_)!=CURLE_OK)
    {
        LOGE("curl_easy_setopt CURLOPT_CAINFO failed");
        return false;
    }

    LOGD("curl_easy_setopt CURLOPT_CAPATH %s", certificatePath_);
    if(curl_easy_setopt(curl_handle, CURLOPT_CAPATH,  certificatePath_)!=CURLE_OK)
    {
        LOGE("curl_easy_setopt CURLOPT_CAPATH failed");
        return false;
    }

#ifdef __DEBUG
    curl_easy_setopt(curl_handle, CURLOPT_VERBOSE, 1L);
    curl_easy_setopt(curl_handle, CURLOPT_DEBUGFUNCTION, debug_function);

    if(strncmp(linkP, NONEXISTENT_TEST_URL, shorter(strlen(NONEXISTENT_TEST_URL), strlen(linkP)))==0)
    {
        se_connection_timeout=3L; // reducing the connection timeout for testing purposes
        MAX_ATTEMPTS=1; // this is for testint code, we are using nonexitent url here so no unncessary attempts
        LOGD("setBasicOpt timeout set to %ld", se_connection_timeout);
    }
#endif

    if(curl_easy_setopt(curl_handle, CURLOPT_TIMEOUT, se_connection_timeout)!=CURLE_OK)
    {
        LOGE("curl_easy_setopt CURLOPT_TIMEOUT failed");
        return false;
    }

/** libcurl uses the http_proxy and https_proxy environment variables for proxy settings.
    That variable is set in the OS specific wrapper. These are left here in order to make
    this comment earier to be found in searches.

    curl_easy_setopt(curl_handle,CURLOPT_PROXY, "http://proxyaddress");
    curl_easy_setopt(curl_handle,CURLOPT_PROXYPORT, "read_proxy_port");
    curl_easy_setopt(curl_handle,CURLOPT_PROXYUSERNAME, "read_proxy_username");
    curl_easy_setopt(curl_handle,CURLOPT_PROXYPASSWORD, "read_proxy_password");
*/

    return true;
}



bool setPutOpt(CURL* curl_handle, ResponseStruct* responseChunk)
{
    long chunkSize=responseChunk->size;
    LOGD(">>setPutOpt");
    if (curl_easy_setopt(curl_handle, CURLOPT_READFUNCTION, readResponseCallback)!=CURLE_OK)
    {
        LOGE("curl_easy_setopt CURLOPT_READFUNCTION failed");
        return false;
    }

    if (curl_easy_setopt(curl_handle, CURLOPT_UPLOAD, 1L)!=CURLE_OK)
    {
        LOGE("curl_easy_setopt CURLOPT_UPLOAD failed");
        return false;
    }

    if (curl_easy_setopt(curl_handle, CURLOPT_PUT, 1L)!=CURLE_OK)
    {
        LOGE("curl_easy_setopt CURLOPT_PUT failed");
        return false;
    }

    if (curl_easy_setopt(curl_handle, CURLOPT_READDATA, responseChunk)!=CURLE_OK)
    {
        LOGE("curl_easy_setopt CURLOPT_READDATA failed");
        return false;
    }


    if (curl_easy_setopt(curl_handle, CURLOPT_INFILESIZE, chunkSize)!=CURLE_OK)
    {
        LOGE("curl_easy_setopt CURLOPT_INFILESIZE_LARGE failed");
        return false;
    }

    LOGD("<<setPutOpt");
    return true;
}

bool setPostOpt(CURL* curl_handle, const char* inputP)
{
    LOGD(">>setPostOpt %ld %d", (long int) inputP, inputP?(int)strlen(inputP):0);

    if (curl_easy_setopt(curl_handle, CURLOPT_POST, 1L)!=CURLE_OK)
    {
        LOGE("curl_easy_setopt CURLOPT_POST failed");
        return false;
    }

    if(NULL==inputP)
    {
        if (curl_easy_setopt(curl_handle, CURLOPT_POSTFIELDSIZE, 0L)!=CURLE_OK)
        {
            LOGE("curl_easy_setopt CURLOPT_POSTFIELDSIZE failed");
            return false;
        }
    }

    if (curl_easy_setopt(curl_handle, CURLOPT_POSTFIELDS, (void*) inputP)!=CURLE_OK)
    {
        LOGE("curl_easy_setopt CURLOPT_POSTFIELDS failed");
        return false;
    }

    LOGD("<<setPostOpt");
    return true;
}

bool setDeleteOpt(CURL* curl_handle, const char* inputP)
{
	(void) inputP;
    LOGD(">>setDeleteOpt %s", inputP);
    if (curl_easy_setopt(curl_handle, CURLOPT_CUSTOMREQUEST, "DELETE")!=CURLE_OK)
    {
        LOGE("curl_easy_setopt CURLOPT_CUSTOMREQUEST failed");
        return false;
    }

    LOGD("<<setDeleteOpt");
    return true;
}


CURL* curl_handle_=NULL;

rootpaerror_t openSeClientAndInit()
{
    if(curl_global_init(CURL_GLOBAL_ALL)!=CURLE_OK)
    {
        LOGE("curl_global_init failed");
        return ROOTPA_ERROR_NETWORK;
    }
    curl_handle_=curl_easy_init();
    if(NULL==curl_handle_)
    {
        LOGE("curl_easy_init failed");
        return ROOTPA_ERROR_NETWORK;
    }

    return ROOTPA_OK;
}

void closeSeClientAndCleanup()
{
    if(curl_handle_)
    {
        curl_easy_cleanup(curl_handle_);
        curl_handle_=NULL;
    }
    curl_global_cleanup();
}

rootpaerror_t httpCommunicate(const char * const inputP, const char** linkP, const char** relP, const char** commandP, httpMethod_t method)
{
    rootpaerror_t ret=ROOTPA_OK;
    long int curlRet=CURLE_COULDNT_CONNECT;
    long int http_code = 0;
    int attempts=0;
    struct curl_slist* httpHeaderP = NULL;
    time_t begintime=0;
    time_t endtime=0;
    int timediff=0;

    ResponseStruct responseChunk;
    MemoryStruct chunk;
    HeaderStruct headerChunk;
    headerChunk.linkSize = 0;
    headerChunk.relSize = 0;
    headerChunk.linkP = NULL;
    headerChunk.relP = NULL;

    LOGD(">>httpCommunicate");
    if(NULL==linkP || NULL==relP || NULL==commandP || NULL==*linkP)
    {
        return ROOTPA_ERROR_ILLEGAL_ARGUMENT;
    }
    LOGD("url %s", *linkP);
    *commandP=NULL;
    *relP=NULL;

    chunk.size = 0;    /* no data at this point */
    chunk.memoryP = (char *)malloc(1);  /* will be grown as needed by the realloc above */
    if(NULL==chunk.memoryP)
    {
        return ROOTPA_ERROR_OUT_OF_MEMORY;
    }
    chunk.memoryP[0]=0;

    LOGD("HTTP method %d", method);

    //Process HTTP methods
	if(method == httpMethod_PUT)
	{
		responseChunk.responseP=inputP;
		responseChunk.size=strlen(responseChunk.responseP);
		responseChunk.offset=0;
		if(setPutOpt(curl_handle_, &responseChunk)==false)
		{
			LOGE("setPutOpt failed");
			free(chunk.memoryP);
			return ROOTPA_ERROR_NETWORK;
		}
	}
	else if(method == httpMethod_POST)
	{
		if (setPostOpt(curl_handle_, inputP)==false)
		{
			LOGE("setPostOpt failed");
			free(chunk.memoryP);
			return ROOTPA_ERROR_NETWORK;
		}
	}
	else if(method == httpMethod_DELETE)
	{
	    LOGD("DELETE method. Calling setDeleteOpt..");
		if (setDeleteOpt(curl_handle_, inputP)==false)
		{
			LOGE("setDeleteOpt failed");
			free(chunk.memoryP);
			return ROOTPA_ERROR_NETWORK;
		}
	}
	else
	{
		if(method != httpMethod_GET)
		{
		LOGE("Unsupported HTTP method");
		free(chunk.memoryP);
		return ROOTPA_ERROR_NETWORK;
		}
	}

    /* disable Expect: 100-continue since it creates problems with some proxies, it is only related to post but we do it here for simplicity */
    httpHeaderP = curl_slist_append(httpHeaderP, "Expect:");
    httpHeaderP = curl_slist_append(httpHeaderP, "Content-Type: application/vnd.mcorecm+xml;v=1.0");
    httpHeaderP = curl_slist_append(httpHeaderP, "Accept: application/vnd.mcorecm+xml;v=1.0");
    if(setBasicOpt(curl_handle_, &chunk, &headerChunk, *linkP, httpHeaderP)==false)
    {
        LOGE("setBasicOpt failed");
        free(chunk.memoryP);
        return ROOTPA_ERROR_NETWORK;
    }

    begintime=time(NULL);
    while(curlRet!=CURLE_OK && attempts++ < MAX_ATTEMPTS)
    {
        curlRet=curl_easy_perform(curl_handle_);
        LOGD("curl_easy_perform %ld %d", curlRet, attempts );
        if(CURLE_OK==curlRet) break;
#ifdef WIN32
		Sleep(SLEEPTIME_MS);
#else
        nanosleep(&SLEEPTIME,NULL);
#endif
        endtime=time(NULL);
        timediff=(int)ceil(difftime(endtime, begintime));
        LOGD("timediff (ceil) %d", timediff);
        if(timediff>(SE_CONNECTION_DEFAULT_TIMEOUT/2))
        {
            LOGE("No connection to SE. Exiting retry loop for curl_easy_perform due to timeout %d", timediff);
            break;
        }
    }

    curl_easy_getinfo (curl_handle_, CURLINFO_RESPONSE_CODE, &http_code);
    if(curlRet!=CURLE_OK)
    {
        LOGE("curl_easy_perform failed %ld", curlRet);
        free(chunk.memoryP);
        free(headerChunk.linkP);
        free(headerChunk.relP);
        curl_easy_reset(curl_handle_);
        return ROOTPA_ERROR_NETWORK;
    }

    LOGD("http return code from SE %ld", (long int) http_code);
    if ((200 <= http_code &&  http_code < 300))
    {
        ret=ROOTPA_OK;
    }
    else if (HTTP_CODE_BAD_REQUEST == http_code ||
             HTTP_CODE_METHOD_NOT_ALLOWED == http_code ||
             HTTP_CODE_NOT_ACCEPTABLE == http_code ||
             HTTP_CODE_CONFLICT == http_code ||
             HTTP_CODE_LENGTH_REQUIRED == http_code ||
             HTTP_CODE_TOO_LONG == http_code ||
             HTTP_CODE_UNSUPPORTED_MEDIA == http_code ||
             HTTP_CODE_INVALID_DATA == http_code ||
             HTTP_CODE_INTERNAL_ERROR == http_code ||
             HTTP_CODE_HTTP_VERSION == http_code)
    {
        LOGE("SE returned http error %ld", (long int) http_code);
        ret=ROOTPA_ERROR_INTERNAL;
    }
    else if(HTTP_CODE_MOVED == http_code ||  // new URL would be in Location: header but RootPA does not support in currently (unless libcurl supports it transparently)
            HTTP_CODE_REQUEST_TIMEOUT == http_code  ||
            HTTP_CODE_SERVICE_UNAVAILABLE == http_code)
    {
        LOGE("SE returned http error %ld", (long int) http_code);
        ret=ROOTPA_ERROR_NETWORK;
    }
    else if (HTTP_CODE_CMP_VERSION == http_code)
    {
        LOGE("SE returned http error %ld", (long int) http_code);
        ret=ROOTPA_ERROR_SE_CMP_VERSION;
    }
    else if (HTTP_CODE_FAILED_DEPENDENCY == http_code)
    {
        LOGE("SE returned http error %ld", (long int) http_code);
        ret=ROOTPA_ERROR_SE_PRECONDITION_NOT_MET;
    }
    else if (HTTP_CODE_NOT_FOUND == http_code)
    {
        LOGE("SE returned http error %ld", (long int) http_code);
        ret=ROOTPA_ERROR_ILLEGAL_ARGUMENT; // since the arguments (spid, in some cases uuid) for the URL are received from the client,
                                           // this can be returned. It is also possible that suid is wrong (corrupted in device or info
                                           // from device binding missing from SE, but we can not detect that easily.
    }
    else
    {
        LOGE("unexpected http return code from SE %ld", (long int)http_code);
        ret=ROOTPA_ERROR_NETWORK;
    }

    /* cleanup curl stuff */

    *commandP=chunk.memoryP;  // this needs to be freed by client
    *linkP=headerChunk.linkP; // this needs to be freed by client
    *relP=headerChunk.relP;   // this needs to be freed by client
    if (httpHeaderP) curl_slist_free_all(httpHeaderP); // since we disabled some headers

    curl_easy_reset(curl_handle_);
    LOGD("%lu bytes retrieved\n", (long)chunk.size);

    LOGD("<<httpCommunicate %d %ld %ld", (int) ret, (long int) http_code, (long int) curlRet);
    return ret;
}

