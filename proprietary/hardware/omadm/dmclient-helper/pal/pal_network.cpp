/* Copyright Statement:
*
* This software/firmware and related documentation ("MediaTek Software") are
* protected under relevant copyright laws. The information contained herein
* is confidential and proprietary to MediaTek Inc. and/or its licensors.
* Without the prior written permission of MediaTek inc. and/or its licensors,
* any reproduction, modification, use or disclosure of MediaTek Software,
* and information contained herein, in whole or in part, shall be strictly prohibited.
*
* MediaTek Inc. (C) 2017. All rights reserved.
*
* BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
* THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
* RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
* AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
* NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
* SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
* SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
* THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
* THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
* CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
* SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
* STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
* CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
* AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
* OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
* MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
*
* The following software/firmware and/or related documentation ("MediaTek Software")
* have been modified by MediaTek Inc. All revisions are subject to any receiver's
* applicable license agreements with MediaTek Inc.
*/

#include <curl/curl.h>
#include <openssl/crypto.h>
#include <openssl/thread.h>
#include <sys/stat.h>
#include <time.h>
#include<pthread.h>
#include <unistd.h>

#include "pal.h"
#include "./pal_internal.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "PAL_NETWORK"

static bool g_inited = false;

/// HTTP response code
#define HTTP_SUCC            200
#define HTTP_FORBIDDEN       403
#define HTTP_NOT_FOUND           404
#define HTTP_SERVICE_UNAVAILABLE 503

#define MUTEX_TYPE       pthread_mutex_t
#define MUTEX_SETUP(x)   pthread_mutex_init(&(x), NULL)
#define MUTEX_CLEANUP(x) pthread_mutex_destroy(&(x))
#define MUTEX_LOCK(x)    pthread_mutex_lock(&(x))
#define MUTEX_UNLOCK(x)  pthread_mutex_unlock(&(x))
#define THREAD_ID        pthread_self()

#define MAX_RETRY_COUNT 3
#define RETRY_TIMEOUT   3
#define RETRY_RATIO     2

/* This array will store all of the mutexes available to OpenSSL. */
static MUTEX_TYPE *mutex_buffer= NULL;

static void lock_clbk(int mode, int type, const char *file, int line)
{
    UNUSED(file);
    UNUSED(line);
    if (mode & CRYPTO_LOCK) {
        MUTEX_LOCK(mutex_buffer[type]);
    } else {
        MUTEX_UNLOCK(mutex_buffer[type]);
    }
}

static unsigned long thread_id(void)
{
    return ((unsigned long)THREAD_ID);
}

static void init_mutexs(void)
{
    mutex_buffer = (MUTEX_TYPE *)OPENSSL_malloc(CRYPTO_num_locks()*sizeof(MUTEX_TYPE));
    for (int i = 0; i < CRYPTO_num_locks(); i++) {
        MUTEX_SETUP(mutex_buffer[i]);
    }
    CRYPTO_set_id_callback(thread_id);
    CRYPTO_set_locking_callback(lock_clbk);
}

static void release_mutexs(void)
{
    if (!mutex_buffer) {
        return;
    }
    CRYPTO_set_id_callback(NULL);
    CRYPTO_set_locking_callback(NULL);
    for (int i = 0; i < CRYPTO_num_locks(); i++) {
        MUTEX_CLEANUP(mutex_buffer[i]);
    }
    OPENSSL_free(mutex_buffer);
    mutex_buffer = NULL;
}

/// Bundle of CA Root Certificates store in PEM format
//#define CA_INFO_NAME "/system/etc/security/cacerts/ca-bundle.crt"
#define CA_INFO_NAME "/vendor/operator/verizon/dmclient/init/ca-bundle.crt"
// TODO: The OEM shall implement own root certificate from the chain of
///       certificates available on the server.

static size_t store_reply_clbk(void * contents, size_t size, size_t nmemb, void * userp)
{
    size_t total = size * nmemb;
    if( contents == NULL || userp == NULL ) {
        ALOGE("store_reply_clbk - invalid parameters!");
        return 0;
    }
    if (userp) {
        data_buffer_t * reply = (data_buffer_t *)userp;
        reply->data = (char*)realloc(reply->data, reply->size + total);
        if (reply->data == NULL) {
            ALOGE("ERROR: Not enough memory");
            return 0;
        }
        memcpy(&(reply->data[reply->size]), contents, total);
        reply->size += total;
        return total;
    }
    return 0;
}

int pal_network_initialize()
{
    curl_global_init(CURL_GLOBAL_ALL);
    init_mutexs();
    return 0;
}

long pal_send_http_post_request (char *server_url, void *options, void * data, void * reply)
{
    CURL *curl;
    struct curl_slist * headerList = NULL;
    long status = HTTP_SERVICE_UNAVAILABLE;

    pal_http_options_t *opt = NULL;
    if(options)
        opt = (pal_http_options_t *)options;
    data_buffer_t *data_out = NULL;
    if(data)
        data_out = (data_buffer_t*)data;
    if (!g_inited) {
        pal_network_initialize();
        g_inited = true;
    }
    curl = curl_easy_init();

    if(curl) {
        curl_easy_setopt(curl, CURLOPT_URL,server_url);
        if(opt) {
            curl_easy_setopt(curl, CURLOPT_TIMEOUT, opt->timeout);
            if(opt->hdr_content_data)
                headerList = curl_slist_append(headerList, opt->hdr_content_data);
            if(opt->hdr_auth_data)
                headerList = curl_slist_append(headerList, opt->hdr_auth_data);
            if(opt->auth_type == AUTH_BASIC)
                curl_easy_setopt(curl, CURLOPT_HTTPAUTH,  (long)CURLAUTH_BASIC);
            else if(opt->auth_type == AUTH_DIGEST)
                curl_easy_setopt(curl, CURLOPT_HTTPAUTH,  (long)CURLAUTH_DIGEST);
            // add to HTTP header
            if(headerList)
                curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headerList);
        }
        // specify path to Certificate Authority bundle
        curl_easy_setopt(curl, CURLOPT_CAINFO, CA_INFO_NAME);
        // curl verifies the authenticity of the peer's certificate
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
        // verify the certificate's name against host
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);

        // request using TLS1.2 for the transfer (Verizon req, 30.3 Security)
        curl_easy_setopt(curl, CURLOPT_USE_SSL, CURLUSESSL_ALL);
        curl_easy_setopt(curl, CURLOPT_SSLVERSION,CURL_SSLVERSION_TLSv1_2);
        /*
         * set cipher list according to OMA Device Management Security,
         * 5.5.1.1 Transport protocols that support encryption
         */
        curl_easy_setopt(curl,CURLOPT_SSL_CIPHER_LIST,
                        "AES128-SHA:ECDHE-ECDSA-AES128-SHA256:DES-CBC3-SHA:RC4-SHA");
        /*
         *  TLS1.0 cipher suite names:
         *  AES128-SHA   - TLS_RSA_WITH_AES_128_CBC_SHA
         *  DES-CBC3-SHA - TLS_RSA_WITH_3DES_EDE_CBC_SHA
         *  RC4-SHA      - TLS_RSA_WITH_RC4_128_SHA
         */
        // debug info
        // request SSL certificate information
        // curl_easy_setopt(curl, CURLOPT_CERTINFO, 1L);
        curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
        // request a HTTP POST
        if(data_out && data_out->size > 0) {
            curl_easy_setopt(curl, CURLOPT_POST, 1);
            curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE_LARGE, (curl_off_t)(data_out->size));
            curl_easy_setopt(curl, CURLOPT_COPYPOSTFIELDS, data_out->data);
        }
        if(reply) {
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, store_reply_clbk);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, reply);
        }

        // To prevent sending not thread safe signals in case when we work outside of main thread
        curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);

        int sleepTimer = RETRY_TIMEOUT;
        for (int retry=0; retry < MAX_RETRY_COUNT; retry++) {
            CURLcode res = curl_easy_perform(curl);
            if (CURLE_OK == res) {
                // RESPONSE_CODE
                curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &status);
                ALOGD("curl - response code: [%ld]", status);
                // PRIMARY_IP & PRIMARY_PORT
                char *str_primary_ip;
                long port;
                curl_easy_getinfo(curl, CURLINFO_PRIMARY_IP, &str_primary_ip);
                curl_easy_getinfo(curl, CURLINFO_PRIMARY_PORT, &port);
                ALOGD("curl - primary ip: [%s], port: [%ld]", str_primary_ip, port);
                // LOCAL_IP & LOCAL_PORT
                char *str_local_ip;
                curl_easy_getinfo(curl, CURLINFO_LOCAL_IP, &str_local_ip);
                curl_easy_getinfo(curl, CURLINFO_LOCAL_PORT , &port);
                ALOGD("curl - local ip: [%s], port: [%ld]", str_local_ip, port);
                // SSL_VERIFYRESULT
                long crt_res = 0;
                curl_easy_getinfo(curl, CURLINFO_SSL_VERIFYRESULT, &crt_res);
                if(crt_res!=0) {
                    ALOGE("certificate verify error [%ld]",crt_res);
                }
                retry = MAX_RETRY_COUNT;
            } else if ( /* Couldn't resolve host. The given remote host was not resolved */
                        res == CURLE_COULDNT_RESOLVE_HOST ||
                        /* Failed to connect() to host or proxy */
                        res == CURLE_COULDNT_CONNECT      ||
                        /* A problem occurred somewhere in the SSL/TLS handshake */
                        res == CURLE_SSL_CONNECT_ERROR ) {
                ALOGD("CURL can't perform command, res = %d, retry counter = %d", res, retry);
                sleep(sleepTimer);
                sleepTimer = sleepTimer * RETRY_RATIO;
            } else {
                ALOGE("CURL can't perform command, res = %d", res);
                retry = MAX_RETRY_COUNT;
            }
        }

        if(headerList)
           curl_slist_free_all(headerList);

        curl_easy_cleanup(curl);
    }

    if (g_inited) {
        pal_network_deinitialize();
        g_inited = false;
    }

    return status;
}

int pal_network_deinitialize()
{
    release_mutexs();
    curl_global_cleanup();
    return 0;
}
