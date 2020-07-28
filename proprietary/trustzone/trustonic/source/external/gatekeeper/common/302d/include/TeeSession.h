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

#ifndef _TeeSession_H_
#define _TeeSession_H_

#include <hardware/gatekeeper.h>
#include <gatekeeper/gatekeeper.h>
#include <gatekeeper/password_handle.h>

#include "MobiCoreDriverApi.h"
#include "tci.h"

#undef  LOG_TAG
#define LOG_TAG "TlcTeeGatekeeper"
#include "log.h"

namespace gatekeeper
{
    struct EnrollRequest;
    struct EnrollResponse;
    struct VerifyRequest;
    struct VerifyResponse;
    struct failure_record_t;
    struct SizedBuffer;
}

// -----------------------------------------------------------------------------
// TeeSession:
// =============================================================================
//
// Goal of this class is to wrap TEE session management and implement all
// interactions with Kinibi TEE.
//
// -----------------------------------------------------------------------------
class TeeSession
{
public:
    TeeSession();

    virtual ~TeeSession() {}


    /* -----------------------------------------------------------------------------
     * @brief   Indicates whether TEE session can be used for gatekeeper operations.
     *
     * @return  true if previous call to Open() has been successful,
     *          otherwise false
     *
    -------------------------------------------------------------------------------- */
    bool IsOpened() const;

    /* -----------------------------------------------------------------------------
     * @brief   Open session to the TEE Gatekeeper trusted application
     *
     * @return  TEE_OK: success. Session with GK trustlet is correctly created
     *          and p_session_handle initialized.
     *          TEE_ERR_MC_DEVICE: Trustlet with DEVICE_ID couldn't be opened
     *          TEE_ERR_SESSION: Session to the GK trustlet couldn't be opened
     *          see logcat for more details
     *
    -------------------------------------------------------------------------------- */
    TEE_ReturnCode_t Open();

    /* -----------------------------------------------------------------------------
     * @brief   Method correctly closes session with TA and closes TA itself. Must
     *          called after successful call to TEE_Open.
     *
     * @return  TEE_OK: success. Session and device are correctly closed
     *          TEE_ERR_SESSION: Session couldn't be close (see logs). Session
     *          object is in unknown state and TA device is still opened.
     *          TEE_ERR_MC_DEVICE: TA device couldn't correctly close. Session is
     *          closed and TA is in unknown state.
     *
    -------------------------------------------------------------------------------- */
    TEE_ReturnCode_t Close();

    TEE_ReturnCode_t ComputePasswordSignature(
                        uint8_t*                        signature,
                        uint32_t                        signature_length,
                        const uint8_t*                  key,
                        uint32_t                        key_length,
                        const uint8_t*                  password,
                        uint32_t                        password_length,
                        uint64_t                        salt);

    /* -----------------------------------------------------------------------------
     * @brief   Retrieves milliseconds since boot from secure driver on SWd side.
     *          Values returned from this function are monotonically nondecreasing.
     *
     * @param   [out] pUptime: Pointer to value to be populated with uptime.
     *
     * @return  TEE_OK: success
     *          TEE_FAIL: unknown error occured, see logs. pUptime is unchanged
     *
    -------------------------------------------------------------------------------- */
    TEE_ReturnCode_t GetMillisecondsSinceBoot(uint64_t &uptime);

    /* -----------------------------------------------------------------------------
     * @brief   Retrieves secure random value
     *
     * @param   [out] random: Pointer to allocated buffer of length
     *          [in] requested_size: Requested length of random value
     *
     * @return  TEE_OK: Success
     *          TEE_ERR_INVALID_INPUT: any of input paramters is NULL or length == 0
     *          TEE_ERR_MEMORY_ALLOCATION_FAILED: Heap exhausted
     *          TEE_ERROR_SECURE_HW_COMMUNICATION_FAILED: Problem while communicating
     *          with secure world (see logs).
     *          In case of any error output parametrs are unchanged.
     *
    -------------------------------------------------------------------------------- */
    TEE_ReturnCode_t GetRandom(
                        void*                           random,
                        uint32_t                        requested_size);

    /* -----------------------------------------------------------------------------
     * @brief   Retrieves authentication token wrapped into secure object.
     *
     * @param   [in]  key_type: one of two values:
     *                - TEE_GK_Key_Password: Used to retrieve password
     *                - TEE_GK_Key_Auth: Used to retrieve authentication token
     *          [out] key: Buffer with a key
     *                     Ownership of the buffer is maintained by the caller.
     *          [out] length: Length of a key buffer.
     *
     * @return  TEE_OK: Success
     *          TEE_ERR_INVALID_INPUT: any of input paramters is NULL
     *          TEE_ERROR_SECURE_HW_COMMUNICATION_FAILED: Problem while communicating
     *          with secure world (see logs).
     *          In case of any error output parametrs are unchanged.
     *
    -------------------------------------------------------------------------------- */

    TEE_ReturnCode_t GetAuthTokenKey(
                        const uint8_t**                 auth_token_key,
                        uint32_t*                       length);

    /* -----------------------------------------------------------------------------
     * @brief   Retrieves password wrapped into secure object. After first call
     *          to the function password is cached and subsequent calls do not
     *          trigger TA instantiation.
     *
     * @param   [out] password_key: Pointer to a buffer with password.
     *                              Ownership of the buffer is maintained by TeeSession
     *                              class
     *          [out] length: Length of a password buffer
     *
     * @return  TEE_OK: Success
     *          TEE_ERR_INVALID_INPUT: any of input paramters is NULL
     *          TEE_ERROR_SECURE_HW_COMMUNICATION_FAILED: Problem while communicating
     *          with secure world (see logs).
     *          In case of any error output parametrs are unchanged.
     *
    -------------------------------------------------------------------------------- */

    TEE_ReturnCode_t GetPasswordKey(
                        const uint8_t**                 password_key,
                        uint32_t*                       length);

    /* -----------------------------------------------------------------------------
     * @brief   Wraps failure record into secure object
     *
     * @param   [in]  failure_record: Failure record to be encrypted
     *          [out] secure_object: Pointer to allocated secure object
     *          [in]  secure_object_len: Size of memory allocated for SO
     * @return  TEE_OK if success,
     *          TEE_ERR_INVALID_INPUT: If secure_object null
     *          Error returned by tlCryptoHandler_FailureRecordEncode
     *
    -------------------------------------------------------------------------------- */

    TEE_ReturnCode_t EncodeFailureRecord(
                        const tee_failure_record_t&     failure_record,
                        uint8_t*                        secure_object,
                        size_t                          secure_object_len);

    /* -----------------------------------------------------------------------------
     * @brief   Unwraps failure record from secure object
     *
     * @param   [in] suid: Secure UID of a user requesting to decode record
     *          [in] secure_object: Pointer to secure object
     *          [in] secure_object_len: Size of memory allocated for SO
     *          [out] failure_record: structure to be populated
     * @return  TEE_OK if success,
     *          TEE_ERR_INVALID_INPUT: If secure_object null
     *          Error returned by tlCryptoHandler_FailureRecordDecode
     *
    -------------------------------------------------------------------------------- */

    TEE_ReturnCode_t DecodeFailureRecord(
                        uint64_t                        suid,
                        uint8_t*                        secure_object,
                        size_t                          secure_object_len,
                        tee_failure_record_t&           failure_record);

    TEE_ReturnCode_t StoreFailureRecord(
                        const uint32_t                  uid,
                        const tee_failure_record_t&     failure_record);

    TEE_ReturnCode_t RetrieveFailureRecord(
                        const uint32_t                  uid,
                        const uint64_t                  suid,
                        tee_failure_record_t&           failure_record);


private:

 /* -------------------------------------------------------------------------
    Class non-copyable
    -------------------------------------------------------------------------*/
    TeeSession(const TeeSession&);
    TeeSession& operator=(const TeeSession&);

    TEE_ReturnCode_t MapBuffer(
                        void*                           buf,
                        uint32_t                        buflen,
                        mcBulkMap_t*                    bufinfo);

    void UnmapBuffer(void *buf, mcBulkMap_t* bufinfo);

    TEE_ReturnCode_t SecureCall();

    TEE_ReturnCode_t getKey(TEE_GK_Key_t   key_type,
                            uint8_t*       key,
                            uint32_t       key_length);

    tciMessage_t        tci_;
    mcSessionHandle_t   session_handle_;
    bool                is_device_opened_;
    // Stores values returned by GetPasswordKey
    uint8_t             password_key_[ENC_PASSWORD_KEY_LEN];
    bool                is_password_key_cached_;

};

inline bool TeeSession::IsOpened() const
{ return is_device_opened_; }

#endif /* _GateKeeperImpl_ */
