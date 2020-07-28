/*
 * Copyright (C) 2008 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef _MEDIATEK_ADVCAM_IADVCAMSERVICE_H_
#define _MEDIATEK_ADVCAM_IADVCAMSERVICE_H_

#include <utils/RefBase.h>
#include <binder/IInterface.h>
#include <binder/Parcel.h>

/* NOTE !!  PLEASE DONT INCLUDE other AdvCam related header
 * The reason is AOSP/framework/av/camera/cameraserver may has include this file.
 * (If advcam service running in cameraserver)
*/

namespace android {
struct CaptureRequest;
namespace NSAdvCam {

class IAdvCamService : public IInterface
{
public:
    enum {
        SET_CONFIGURE_PARAM = IBinder::FIRST_CALL_TRANSACTION,
    };

public:
    DECLARE_META_INTERFACE(AdvCamService);

    /// Interface

    virtual status_t setConfigureParam( uint32_t openId, android::CaptureRequest& request) = 0;

};

// ----------------------------------------------------------------------------

class BnAdvCamService: public BnInterface<IAdvCamService>
{
public:
    virtual status_t    onTransact( uint32_t code,
                                    const Parcel& data,
                                    Parcel* reply,
                                    uint32_t flags = 0);
};

}; // namespace NSAdvCam
}; // namespace android

#endif  //_MEDIATEK_ADVCAM_IADVCAMSERVICE_H_
