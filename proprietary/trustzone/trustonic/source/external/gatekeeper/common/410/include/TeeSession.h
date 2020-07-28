/*
 * Copyright (c) 2015-2016 TRUSTONIC LIMITED
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
#include <gatekeeper/gatekeeper_messages.h>
#include "MobiCoreDriverApi.h"
#include "tci.h"

#undef  LOG_TAG
#define LOG_TAG "TlcTeeGatekeeper"
#include "log.h"

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
     * @brief   Indicates whether TEE session is set up and can be used for
     *          gatekeeper operations.
    -------------------------------------------------------------------------------- */
    bool IsOpened() const;

    /* -----------------------------------------------------------------------------
     * @brief   Open session to the TEE Gatekeeper trusted application
     *
     * @return  MC_DRV_OK on success,
     *          MC_DRV_ERR_INVALID_OPERATION if session is already opened
     *          one of mcResult_t error codes in any other cases
    -------------------------------------------------------------------------------- */
    mcResult_t Open();

    /* -----------------------------------------------------------------------------
     * @brief   Method closes session with TA and closes TA itself. Must be called
     *          after successful call to TEE_Open.
     *
     * @return  MC_DRV_OK on success
     *          MC_DRV_ERR_INVALID_OPERATION if session is not opened
     *          one of mcResult_t error codes in any other cases
    -------------------------------------------------------------------------------- */
    mcResult_t Close();

    /* -----------------------------------------------------------------------------
     * @brief   Get Gatekeeper SWd version.
     *
     * @return ERROR_NONE if succeed, ERROR_UNKNOWN in any other cases
     *
     -------------------------------------------------------------------------------- */
    gatekeeper_error_t CheckVersion();

    /* -----------------------------------------------------------------------------
     * @brief   Enrolls desired_password. Before calling this method user has to ensure
     *          IsOpened() return true.
     *          Arguments and return values are same as described in the
     *          HalAdaptationLayer::enroll()
     *
     -------------------------------------------------------------------------------- */
    gatekeeper_error_t Enroll(
        uint32_t uid,
        const uint8_t *current_password_handle, uint32_t current_password_handle_length,
        const uint8_t *current_password, uint32_t current_password_length,
        const uint8_t *desired_password, uint32_t desired_password_length,
        uint8_t **enrolled_password_handle, uint32_t *enrolled_password_handle_length,
        int& throttle_ms);

    /* -----------------------------------------------------------------------------
     * @brief   Verifies provided_password. Before calling this method user has to ensure
     *          IsOpened() return true.
     *
     * @param   [out] wait_time_ms: time to wait before reattempting the call.
     *
     *          All the other parameters and return values are same as described in the
     *          HalAdaptationLayer::verify()
     *
     -------------------------------------------------------------------------------- */
    gatekeeper_error_t Verify(
        uint32_t uid, uint64_t challenge,
        const uint8_t *enrolled_password_handle, uint32_t enrolled_password_handle_length,
        const uint8_t *provided_password, uint32_t provided_password_length,
        uint8_t **auth_token, uint32_t *auth_token_length, bool &request_reenroll, int &wait_time_ms);

    /* -----------------------------------------------------------------------------
     * @brief   Deletes failure record for single or all the users. Method works only
     *          for failure records stored on persistent secure storage. After this
     *          operation call to verify() will always fail for user specified by uid parameter
     *          or for all the users if delete_all was set to true.
     *
     * @param   uid UID of the user of which failure record must be deleted. Ignored if
     *          delete_all was set to true
     * @param   delete_all delete failure records for all the users.
     *
     *          All the other parameters and return values are same as described in the
     *          HalAdaptationLayer::verify()
     *
     -------------------------------------------------------------------------------- */
    gatekeeper_error_t DeleteUser(user_id_t uid, bool delete_all);

private:

 /* -------------------------------------------------------------------------
    Class non-copyable
    -------------------------------------------------------------------------*/
    TeeSession(const TeeSession&);
    TeeSession& operator=(const TeeSession&);

    mcResult_t MapBuffer(void*         buf,
                         uint32_t      buflen,
                         mcBulkMap_t*  bufinfo);

    void UnmapBuffer(void *buf, mcBulkMap_t* bufinfo);
#ifdef __FAILURE_RECORD_STORAGE_IN_SECURE_OBJECT__
    /* -----------------------------------------------------------------------------
     * @brief   Saves a buffer into a file.
     *
     * @param   pPath Path of the file to be saved. The file is created if it does not
     *          exists.
     * @param   ppContent Buffer to be saved
     * @param   size Size of the buffer to be saved
     *
     * @return  0 on success
     *          1 if any error
     -------------------------------------------------------------------------------- */
    int saveFile(const char *pPath, uint8_t* ppContent, size_t size);

    /* -----------------------------------------------------------------------------
     * @brief   Fill a buffer with data read from a file.
     *
     * @param   pPath Path of the file to be saved.
     * @param   ppContent Buffer to be saved
     * @param   size Size of the buffer to be saved
     *
     * @return  The size of the data read
     -------------------------------------------------------------------------------- */
    size_t readFile(const char* pPath, uint8_t** ppContent);
#endif
    mcResult_t SecureCall();

    tciMessage_t        tci_;
    mcSessionHandle_t   session_handle_;
    bool                is_device_opened_;

};

inline bool TeeSession::IsOpened() const
{ return is_device_opened_; }

#endif /* _GateKeeperImpl_ */
