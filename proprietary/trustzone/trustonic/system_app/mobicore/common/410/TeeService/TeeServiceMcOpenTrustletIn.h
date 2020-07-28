/*
 * Copyright (c) 2018 TRUSTONIC LIMITED
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

#ifndef __TEE_FWK_MC_OPEN_TRUSTLET_IN_H__
#define __TEE_FWK_MC_OPEN_TRUSTLET_IN_H__

#include <binder/Parcel.h>

namespace vendor {

namespace trustonic {

namespace teeservice {

struct TeeServiceMcOpenTrustletIn : public ::android::Parcelable {
    uint32_t spid = 0;
    int ta = -1;
    uint32_t ta_len = 0;
    int tci = -1;
    uint32_t tci_len = 0;

    virtual ::android::status_t writeToParcel(::android::Parcel* parcel) const {
        parcel->writeUint32(spid);
        parcel->writeFileDescriptor(ta);
        parcel->writeUint32(ta_len);
        parcel->writeFileDescriptor(tci);
        parcel->writeUint32(tci_len);
        return ::android::OK;
    }

    virtual ::android::status_t readFromParcel(const ::android::Parcel* parcel) {
        parcel->readUint32(&spid);
        ta = ::dup(parcel->readFileDescriptor());
        parcel->readUint32(&ta_len);
        tci = ::dup(parcel->readFileDescriptor());
        parcel->readUint32(&tci_len);
        return ::android::OK;
    }
};

}

}

}

#endif // __TEE_FWK_MC_OPEN_TRUSTLET_IN_H__
