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

#ifndef _MEDIATEK_MMSDK_IMMSDKSERVICE_H_
#define _MEDIATEK_MMSDK_IMMSDKSERVICE_H_

#include <utils/RefBase.h>
#include <binder/IInterface.h>
#include <binder/Parcel.h>

/* NOTE !!  PLEASE DONT INCLUDE other MMSDK related header
 * The reason is AOSP/framework/av/camera/cameraserver has include this file.
*/


namespace NSCam {
    class IFeatureManager;
};

namespace android {
namespace NSMMSdk {

class IMMSdkService : public IInterface
{
public:
    enum {
        GET_MMSDK_FEAUTRE_MANAGER = IBinder::FIRST_CALL_TRANSACTION,
        EXIST_CALLBACK_CLIENT,
    };

public:
    DECLARE_META_INTERFACE(MMSdkService);

    /// Interface
    virtual status_t connectFeatureManager(sp<NSCam::IFeatureManager> & featureManager) = 0;
    virtual status_t existCallbackClient() = 0;

};

// ----------------------------------------------------------------------------

class BnMMSdkService: public BnInterface<IMMSdkService>
{
public:
    virtual status_t    onTransact( uint32_t code,
                                    const Parcel& data,
                                    Parcel* reply,
                                    uint32_t flags = 0);
};

}; // namespace NSMMSdk
}; // namespace android

#endif  //_MEDIATEK_MMSDK_IMMSDKSERVICE_H_
