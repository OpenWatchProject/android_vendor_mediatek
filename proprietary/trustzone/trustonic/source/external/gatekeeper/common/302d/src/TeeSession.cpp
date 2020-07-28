/*
 * Copyright (c) 2015 TRUSTONIC LIMITED
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

#include "buildTag.h"
#include <cstdlib>
#include <time.h>
#include <inttypes.h>
#include <unistd.h>

#include <sstream>
#include <fstream>
#include <utility>
#include <iomanip>

#include "tci.h"

#include "TeeSession.h"
#include "log.h"
#include <gatekeeper/gatekeeper_messages.h>
#include <gatekeeper/gatekeeper_utils.h>
#include <gatekeeper/password_handle.h>


/* Global definitions */
static const __attribute__((used)) char* buildtag = MOBICORE_COMPONENT_BUILD_TAG;
static const uint32_t DEVICE_ID = MC_DEVICE_ID_DEFAULT;
static const mcUuid_t uuid = TEE_GATEKEEPER_TL_UUID;

#define BREAK_IF_ERR(expr)                                  \
    ret = (expr);                                           \
    if (ret != TEE_OK) {                                    \
        LOG_D("%s: " #expr " returned %d", __func__, ret);  \
        break;                                              \
    }

TeeSession::TeeSession()
    : session_handle_({0,0})
    , is_device_opened_(false)
    , is_password_key_cached_(false)
{
    memset(password_key_,0,ENC_PASSWORD_KEY_LEN);
    tci_.response.header.returnCode = TEE_OK;
}

TEE_ReturnCode_t TeeSession::Open()
{
    mcResult_t          mcRet   = 0;
    TEE_ReturnCode_t    ret     = TEE_OK;

    do
    {
        /* Open MobiCore device */
        mcRet = mcOpenDevice(DEVICE_ID);
        if (MC_DRV_OK != mcRet)
        {
            LOG_E("TeeSession::Open(): mcOpenDevice returned: %d\n", mcRet);
            ret = TEE_ERR_MC_DEVICE;
            break;
        }

        /* Open session the TEE Keymaster trustlet */
        session_handle_.deviceId = DEVICE_ID;
        mcRet = mcOpenSession(&session_handle_,
                              &uuid,
                              (uint8_t *) &tci_,
                              (uint32_t) sizeof(tciMessage_t));
        if (MC_DRV_OK != mcRet)
        {
            LOG_E("TeeSession::Open(): mcOpenSession returned: %d\n", mcRet);
            (void)mcCloseDevice(DEVICE_ID);
            ret = TEE_ERR_SESSION;
            break;
        }
        is_device_opened_ = true;
    }
    while (false);
    return ret;
}

TEE_ReturnCode_t TeeSession::Close()
{
    mcResult_t          mcRet   = 0;
    TEE_ReturnCode_t    ret     = TEE_OK;

    do {
        mcRet = mcCloseSession(&session_handle_);
        if (MC_DRV_OK != mcRet)
        {
            LOG_E("TeeSession::Close(): mcCloseSession returned: %d\n", mcRet);
            ret = TEE_ERR_SESSION;
            break;
        }

        mcRet = mcCloseDevice(DEVICE_ID);
        if (MC_DRV_OK != mcRet)
        {
            LOG_E("TeeSession::Close(): mcCloseDevice returned: %d\n", mcRet);
                ret = TEE_ERR_MC_DEVICE;
        }
        is_device_opened_ = false;
    } while (false);
    return ret;
}

TEE_ReturnCode_t TeeSession::ComputePasswordSignature(
    uint8_t *signature,
    uint32_t signature_length,
    const uint8_t *key,
    uint32_t key_length,
    const uint8_t *data,
    uint32_t data_length,
    uint64_t salt)
{
    if( (NULL == signature) ||
        (0    == signature_length) ||
        (NULL == key ) ||
        (0    == key_length) ||
        (NULL == data ) ||
        (0    == data_length) )
    {
        return TEE_ERR_INVALID_INPUT;
    }

    TEE_ReturnCode_t ret = TEE_OK;
    mcBulkMap_t signature_mm     = {0 /*sVirtualAddr*/,0 /*sVirtualLen*/};
    mcBulkMap_t key_mm           = {0 /*sVirtualAddr*/,0 /*sVirtualLen*/};
    mcBulkMap_t data_mm          = {0 /*sVirtualAddr*/,0 /*sVirtualLen*/};

    do
    {
        BREAK_IF_ERR(MapBuffer(signature, signature_length, &signature_mm));
        BREAK_IF_ERR(MapBuffer(const_cast<uint8_t*>(key), key_length, &key_mm));
        BREAK_IF_ERR(MapBuffer(const_cast<uint8_t*>(data), data_length, &data_mm));

        tci_.command.header.commandId = CMD_ID_TEE_COMPUTE_PASS_SIGNATURE;
        tci_.signature.hmac.data = (uint32_t) signature_mm.sVirtualAddr;
        tci_.signature.hmac.data_length = signature_length;
        tci_.signature.data.data = (uint32_t) data_mm.sVirtualAddr;
        tci_.signature.data.data_length = data_length;
        tci_.signature.key.data = (uint32_t) key_mm.sVirtualAddr;
        tci_.signature.key.data_length = key_length;
        tci_.signature.salt = salt;
        BREAK_IF_ERR( SecureCall() );
    } while(false);

    UnmapBuffer(signature, &signature_mm);
    UnmapBuffer(const_cast<uint8_t*>(key), &key_mm);
    UnmapBuffer(const_cast<uint8_t*>(data), &data_mm);
    return ret;
}

TEE_ReturnCode_t TeeSession::GetMillisecondsSinceBoot(uint64_t& uptime)
{
    TEE_ReturnCode_t    ret = TEE_OK;

    do
    {
        tci_.command.header.commandId = CMD_ID_TEE_MILLI_SINCE_BOOT;

        BREAK_IF_ERR( SecureCall() );
        uptime = tci_.uptime;
    } while(false);

    return ret;
}

TEE_ReturnCode_t TeeSession::GetRandom(
                                void*    random,
                                uint32_t requested_size)
{
    TEE_ReturnCode_t ret = TEE_OK;
    mcBulkMap_t dataInfo = {0 /*sVirtualAddr*/,0 /*sVirtualLen*/};

    if( (NULL == random) ||
        (0    == requested_size) )
    {
        return TEE_ERR_INVALID_INPUT;
    }

    do
    {
        BREAK_IF_ERR( MapBuffer(random, requested_size, &dataInfo) );

        tci_.command.header.commandId = CMD_ID_TEE_GET_RANDOM;
        tci_.random.data = (uint32_t)dataInfo.sVirtualAddr;
        tci_.random.data_length = requested_size;
        BREAK_IF_ERR( SecureCall() );
    } while(false);

    UnmapBuffer(random, &dataInfo);
    return ret;
}

TEE_ReturnCode_t TeeSession::GetAuthTokenKey(
                                const uint8_t** auth_token_key,
                                uint32_t*       length)
{
    // Input validation
    if( (NULL == auth_token_key) ||
        (NULL == length) )
    {
        return TEE_ERR_INVALID_INPUT;
    }
    uint8_t local_auth_key[ENC_AUTH_TOKEN_KEY_LEN] = {};
    TEE_ReturnCode_t ret = getKey(  TEE_GK_Key_Auth,
                                    &(local_auth_key[0]),
                                    ENC_AUTH_TOKEN_KEY_LEN);

    if(TEE_OK==ret)
    {
        uint8_t* buf = new uint8_t[ENC_AUTH_TOKEN_KEY_LEN];
        memcpy(buf, local_auth_key, ENC_AUTH_TOKEN_KEY_LEN);
        *auth_token_key = buf;
        *length = ENC_AUTH_TOKEN_KEY_LEN;
    }
    return ret;
}

TEE_ReturnCode_t TeeSession::GetPasswordKey(
                                const uint8_t** password_key,
                                uint32_t*       length)
{
    // Input validation
    if( (NULL == password_key) ||
        (NULL == length) )
    {
        return TEE_ERR_INVALID_INPUT;
    }

    TEE_ReturnCode_t ret = TEE_OK;
    if(!is_password_key_cached_)
    {
        ret = getKey(   TEE_GK_Key_Password,
                        &(password_key_[0]),
                        ENC_PASSWORD_KEY_LEN);
        if(TEE_OK == ret)
        {
            is_password_key_cached_ = true;
        }
    }

    if(TEE_OK == ret)
    {
        *password_key = &(password_key_[0]);
        *length = ENC_PASSWORD_KEY_LEN;
    }
    return ret;
}

TEE_ReturnCode_t TeeSession::EncodeFailureRecord(
    const tee_failure_record_t&       failure_record,
    uint8_t*                          secure_object,
    size_t                            secure_object_len)
{
    if(NULL == secure_object)
    {
        return TEE_ERR_INVALID_INPUT;
    }

    TEE_ReturnCode_t ret = TEE_OK;
    mcBulkMap_t encoded_record_mm  = {0 /*sVirtualAddr*/,0 /*sVirtualLen*/};
    do
    {
        BREAK_IF_ERR(MapBuffer(secure_object, secure_object_len, &encoded_record_mm));

        tci_.command.header.commandId = CMD_ID_TEE_FAILURE_RECORD_ENCODE;
        tci_.failure_record.version = failure_record.version;
        tci_.failure_record.suid = failure_record.suid;
        tci_.failure_record.last_access_timestamp = failure_record.last_access_timestamp;
        tci_.failure_record.failure_counter = failure_record.failure_counter;
        tci_.failure_record.encrypted_failure_record.data = (uint32_t)encoded_record_mm.sVirtualAddr;
        tci_.failure_record.encrypted_failure_record.data_length = secure_object_len;
        BREAK_IF_ERR( SecureCall() );
    } while(false);

    UnmapBuffer(secure_object, &encoded_record_mm);
    return ret;
}

TEE_ReturnCode_t TeeSession::DecodeFailureRecord(
    const uint64_t                    suid,
    uint8_t*                          secure_object,
    size_t                            secure_object_len,
    tee_failure_record_t&             failure_record)
{
    if( (NULL == secure_object) )
    {
        return TEE_ERR_INVALID_INPUT;
    }

    TEE_ReturnCode_t ret = TEE_OK;
    mcBulkMap_t encoded_record_mm  = {0 /*sVirtualAddr*/,0 /*sVirtualLen*/};
    do
    {
        BREAK_IF_ERR(MapBuffer( secure_object, secure_object_len, &encoded_record_mm));

        tci_.command.header.commandId = CMD_ID_TEE_FAILURE_RECORD_DECODE;
        tci_.failure_record.suid = suid;
        tci_.failure_record.encrypted_failure_record.data = (uint32_t)encoded_record_mm.sVirtualAddr;
        tci_.failure_record.encrypted_failure_record.data_length = secure_object_len;
        BREAK_IF_ERR( SecureCall() );

        failure_record.version = tci_.failure_record.version;
        failure_record.suid = tci_.failure_record.suid;
        failure_record.last_access_timestamp = tci_.failure_record.last_access_timestamp;
        failure_record.failure_counter= tci_.failure_record.failure_counter;
    } while(false);

    UnmapBuffer(secure_object, &encoded_record_mm);
    return ret;
}

TEE_ReturnCode_t TeeSession::StoreFailureRecord(
    const uint32_t                    uid,
    const tee_failure_record_t&       failure_record)
{
    TEE_ReturnCode_t ret = TEE_OK;
    do
    {
        tci_.command.header.commandId = CMD_ID_TEE_FAILURE_RECORD_STORE;
        tci_.failure_record.version = failure_record.version;
        tci_.failure_record.suid = failure_record.suid;
        tci_.failure_record.last_access_timestamp = failure_record.last_access_timestamp;
        tci_.failure_record.failure_counter = failure_record.failure_counter;
        tci_.failure_record.encrypted_failure_record.data = uid;
        tci_.failure_record.encrypted_failure_record.data_length = sizeof(tee_failure_record_t);
        BREAK_IF_ERR( SecureCall() );
    } while(false);

    return ret;
}

TEE_ReturnCode_t TeeSession::RetrieveFailureRecord(
    const uint32_t                    uid,
    const uint64_t                    suid,
    tee_failure_record_t&             failure_record)
{
    TEE_ReturnCode_t ret = TEE_OK;
    do
    {
        tci_.command.header.commandId = CMD_ID_TEE_FAILURE_RECORD_RETRIEVE;
        tci_.failure_record.suid = suid;
        tci_.failure_record.encrypted_failure_record.data = uid;
        tci_.failure_record.encrypted_failure_record.data_length = sizeof(tee_failure_record_t);
        BREAK_IF_ERR( SecureCall() );

        memcpy(&failure_record, &(tci_.failure_record), sizeof(tee_failure_record_t));
        /*
        failure_record.version = tci_.failure_record.version;
        failure_record.suid = tci_.failure_record.suid;
        failure_record.last_access_timestamp = tci_.failure_record.last_access_timestamp;
        failure_record.failure_counter = tci_.failure_record.failure_counter;
        failure_record.encrypted_failure_record.data = tci_.failure_record.encrypted_failure_record.data;
        failure_record.encrypted_failure_record.data_length = tci_.failure_record.encrypted_failure_record.data_length;
        */
    } while(false);

    return ret;
}


/**
 * Map a buffer.
 */
TEE_ReturnCode_t TeeSession::MapBuffer(
                                void *buf,
                                uint32_t buflen,
                                mcBulkMap_t *bufinfo)
{
    if (NULL != buf) {
        mcResult_t mcRet = mcMap(   &session_handle_,
                                    buf,
                                    buflen,
                                    bufinfo);
        if (mcRet != MC_DRV_OK) {
            LOG_E("%s: mcMap() returned 0x%08x", __func__, mcRet);
            return TEE_ERROR_SECURE_HW_COMMUNICATION_FAILED;
        }
    }
    return TEE_OK;
}

/**
 * Unmap a buffer.
 */
void TeeSession::UnmapBuffer(
                    void *buf,
                    mcBulkMap_t *bufinfo)
{
    if (bufinfo->sVirtualAddr != 0) {
        mcResult_t mcRet = mcUnmap( &session_handle_,
                                    buf,
                                    bufinfo);
        if (mcRet != MC_DRV_OK) {
            LOG_E("%s: mcUnmap() returned 0x%08x", __func__, mcRet);
        }
    }
}

/**
 * Notify the trusted application and wait for response.
 */

TEE_ReturnCode_t TeeSession::SecureCall()
{
    TEE_ReturnCode_t ret = TEE_OK;
    do
    {
        mcResult_t mcRet;
        mcRet = mcNotify(&session_handle_);
        if (mcRet != MC_DRV_OK) {
            LOG_E("%s: mcNotify() returned 0x%08x", __func__, mcRet);
            ret = TEE_ERROR_SECURE_HW_COMMUNICATION_FAILED;
            break;
        }
        mcRet = mcWaitNotification( &session_handle_,
                                    MC_INFINITE_TIMEOUT);
        if (mcRet != MC_DRV_OK) {
            LOG_E("%s: mcWaitNotification() returned 0x%08x", __func__, mcRet);
            if(mcRet == MC_DRV_INFO_NOTIFICATION)
            {
                int32_t ta_termination_code=0;
                mcGetSessionErrorCode(&session_handle_, &ta_termination_code);
                LOG_E("TA termination code: %d", ta_termination_code);
            }

            ret = TEE_ERROR_SECURE_HW_COMMUNICATION_FAILED;
            break;
        }
        BREAK_IF_ERR( tci_.response.header.returnCode );
    }
    while(false);
    return ret;
}

TEE_ReturnCode_t TeeSession::getKey(
    TEE_GK_Key_t  key_type,
    uint8_t*      key,
    uint32_t      key_length)
{
    TEE_ReturnCode_t ret = TEE_OK;
    mcBulkMap_t key_mm   = {0 /*sVirtualAddr*/,0 /*sVirtualLen*/};

    if( (NULL == key) ||
        (0    == key_length))
    {
        return TEE_ERR_INVALID_INPUT;
    }

    switch(key_type)
    {
        case TEE_GK_Key_Password:
            tci_.command.header.commandId = CMD_ID_TEE_GET_PASSWORD_KEY;
            break;
        case TEE_GK_Key_Auth:
            tci_.command.header.commandId = CMD_ID_TEE_GET_AUTH_TOKEN_KEY;
            break;
        default:
            return TEE_ERR_INVALID_INPUT;
    }

    do
    {
        BREAK_IF_ERR( MapBuffer(key, key_length, &key_mm) );

        tci_.symmetric_key.data = (uint32_t) key_mm.sVirtualAddr;
        tci_.symmetric_key.data_length = key_length;
        BREAK_IF_ERR( SecureCall() );
    } while(false);

    UnmapBuffer(key, &key_mm);
    return ret;
}
