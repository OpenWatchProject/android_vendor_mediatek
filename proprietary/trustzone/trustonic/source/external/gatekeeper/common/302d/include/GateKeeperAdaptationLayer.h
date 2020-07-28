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

#ifndef _GateKeeperAdaptationLayer_H_
#define _GateKeeperAdaptationLayer_H_

#include <hardware/gatekeeper.h>
#include <gatekeeper/gatekeeper.h>
#include <gatekeeper/password_handle.h>

#include "tci.h"
#include "TeeSession.h"

#ifndef RECORD_DIRECTORY
#define RECORD_DIRECTORY "/mnt/vendor/persist/mcRegistry/"
#endif

class GateKeeperAdaptationLayer : public gatekeeper::GateKeeper
{
public:

    GateKeeperAdaptationLayer();
    virtual ~GateKeeperAdaptationLayer();

    bool Open();
    bool Close();

/* -------------------------------------------------------------------------
   Implementation of Android's gatekeeper interface. For description of
   methods below please see:
   https://android.googlesource.com/platform/system/gatekeeper/+/master/include/gatekeeper/gatekeeper.h
   -------------------------------------------------------------------------*/

    virtual void ComputePasswordSignature(  uint8_t *signature,
                                            uint32_t signature_length,
                                            const uint8_t *key,
                                            uint32_t key_length,
                                            const uint8_t *password,
                                            uint32_t password_length,
                                            gatekeeper::salt_t salt) const;

    virtual bool GetAuthTokenKey(   const uint8_t **auth_token_key,
                                    uint32_t *length) const;

    virtual void GetPasswordKey(const uint8_t **password_key,
                                uint32_t *length);


    virtual void GetRandom( void *random,
                            uint32_t requested_size) const;
    virtual void ComputeSignature(  uint8_t *signature,
                                    uint32_t signature_length,
                                    const uint8_t *key,
                                    uint32_t key_length,
                                    const uint8_t *message,
                                    const uint32_t length) const;

    virtual uint64_t GetMillisecondsSinceBoot() const;

    virtual bool GetFailureRecord(  uint32_t uid,
                                    gatekeeper::secure_id_t user_id,
                                    gatekeeper::failure_record_t *record,
                                    bool secure);

    virtual bool ClearFailureRecord(uint32_t uid,
                                    gatekeeper::secure_id_t user_id,
                                    bool secure);

    virtual bool WriteFailureRecord(uint32_t uid,
                                    gatekeeper::failure_record_t *record,
                                    bool secure);

    virtual bool IsHardwareBacked() const;


protected:

/* -------------------------------------------------------------------------
   Class non-copyable
   -------------------------------------------------------------------------*/
   GateKeeperAdaptationLayer(const GateKeeperAdaptationLayer&);
   GateKeeperAdaptationLayer& operator=(const GateKeeperAdaptationLayer&);

private:
    bool doWriteFailureRecord(
            uint32_t uid,
            const tee_failure_record_t& failure_record,
            bool check_exists);

    bool ResetFailureRecord(
            uint32_t uid,
            gatekeeper::secure_id_t user_id,
            gatekeeper::failure_record_t *record,
            bool secure);

    // PIMPL
    TeeSession* const tee_;
};

/* -------------------------------------------------------------------------
   Inline implementations
   -------------------------------------------------------------------------*/

inline
bool GateKeeperAdaptationLayer::GetAuthTokenKey(   const uint8_t **auth_token_key,
                        uint32_t *length) const
{
    TEE_ReturnCode_t res = TEE_OK;
    res = tee_->GetAuthTokenKey(auth_token_key, length);
    return (TEE_OK == res);
}

inline
void GateKeeperAdaptationLayer::GetPasswordKey(const uint8_t **password_key,
                                uint32_t *length)
{
    tee_->GetPasswordKey(password_key, length);
}

inline
void GateKeeperAdaptationLayer::ComputePasswordSignature(  uint8_t *signature,
                                            uint32_t signature_length,
                                            const uint8_t *key,
                                            uint32_t key_length,
                                            const uint8_t *password,
                                            uint32_t password_length,
                                            gatekeeper::salt_t salt) const
{
    tee_->ComputePasswordSignature(
                signature,
                signature_length,
                key,
                key_length,
                password,
                password_length,
                (uint64_t)salt);
}

inline
void GateKeeperAdaptationLayer::GetRandom(
            void*       random,
            uint32_t    requested_size) const
{
    tee_->GetRandom(random, requested_size);
}

inline
void GateKeeperAdaptationLayer::ComputeSignature(
                                    uint8_t *signature,
                                    uint32_t signature_length,
                                    const uint8_t *key,
                                    uint32_t key_length,
                                    const uint8_t *message,
                                    const uint32_t length) const
{
    tee_->ComputePasswordSignature(
                signature,
                signature_length,
                key,
                key_length,
                message,
                length,
                0);
}


inline
bool GateKeeperAdaptationLayer::IsHardwareBacked() const
{ return true; }

#endif /* _GateKeeperAdaptationLayer_H_ */
