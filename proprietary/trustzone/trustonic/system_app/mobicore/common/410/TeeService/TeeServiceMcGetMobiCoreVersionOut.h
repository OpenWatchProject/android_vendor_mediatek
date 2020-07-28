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

#ifndef __TEE_FWK_MC_GET_MOBICORE_VERSION_OUT_H__
#define __TEE_FWK_MC_GET_MOBICORE_VERSION_OUT_H__

#include <string.h>
#include <binder/Parcel.h>
#include "MobiCoreDriverApi.h"

namespace vendor {

namespace trustonic {

namespace teeservice {

struct TeeServiceMcGetMobiCoreVersionOut : public ::android::Parcelable {
    uint32_t mc_result = MC_DRV_ERR_DAEMON_UNREACHABLE;

    char productId[MC_PRODUCT_ID_LEN];
    uint32_t versionMci;
    uint32_t versionSo;
    uint32_t versionMclf;
    uint32_t versionContainer;
    uint32_t versionMcConfig;
    uint32_t versionTlApi;
    uint32_t versionDrApi;
    uint32_t versionCmp;

    virtual ::android::status_t writeToParcel(::android::Parcel* parcel) const {
        parcel->writeUint32(mc_result);
        parcel->writeCString(productId);
        parcel->writeUint32(versionMci);
        parcel->writeUint32(versionSo);
        parcel->writeUint32(versionMclf);
        parcel->writeUint32(versionContainer);
        parcel->writeUint32(versionMcConfig);
        parcel->writeUint32(versionTlApi);
        parcel->writeUint32(versionDrApi);
        parcel->writeUint32(versionCmp);
        return ::android::OK;
    }

    virtual ::android::status_t readFromParcel(const ::android::Parcel* parcel) {
        parcel->readUint32(&mc_result);
        ::strncpy(productId, parcel->readCString(), sizeof(productId) - 1);
        productId[sizeof(productId) - 1] = '\0';
        parcel->readUint32(&versionMci);
        parcel->readUint32(&versionSo);
        parcel->readUint32(&versionMclf);
        parcel->readUint32(&versionContainer);
        parcel->readUint32(&versionMcConfig);
        parcel->readUint32(&versionTlApi);
        parcel->readUint32(&versionDrApi);
        parcel->readUint32(&versionCmp);
        return ::android::OK;
    }
};

}

}

}

#endif // __TEE_FWK_MC_GET_MOBICORE_VERSION_OUT_H__
