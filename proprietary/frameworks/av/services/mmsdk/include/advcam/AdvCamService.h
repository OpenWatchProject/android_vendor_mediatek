/*
**
** Copyright (C) 2008, The Android Open Source Project
**
** Licensed under the Apache License, Version 2.0 (the "License");
** you may not use this file except in compliance with the License.
** You may obtain a copy of the License at
**
**     http://www.apache.org/licenses/LICENSE-2.0
**
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS,
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
** See the License for the specific language governing permissions and
** limitations under the License.
*/

#ifndef _MEDIATEK_SERVERS_ADVCAM_ADVCAMSERVICE_H_
#define _MEDIATEK_SERVERS_ADVCAM_ADVCAMSERVICE_H_

#include <utils/Vector.h>
#include <binder/BinderService.h>
#include <advcam/IAdvCamService.h>

/* NOTE !!  PLEASE DONT INCLUDE other AdvCam related header
 * The reason is AOSP/framework/av/camera/cameraserver may has include this file.
 * (If advcam service running in cameraserver)
*/

namespace android {
extern volatile int32_t gLogLevel;

namespace NSAdvCam {

class AdvCamService :
    public BinderService<AdvCamService>,
    public BnAdvCamService,
    public IBinder::DeathRecipient
{
    friend class BinderService<AdvCamService>;
public:
    // Implementation of BinderService<T>
    static char const* getServiceName() { return "media.advcam"; }

                        AdvCamService();
    virtual             ~AdvCamService();

    virtual status_t setConfigureParam( uint32_t openId, CaptureRequest& request);

    // Extra permissions checks
    virtual status_t    onTransact(uint32_t code, const Parcel& data,
                                   Parcel* reply, uint32_t flags);

    virtual status_t    dump(int fd, const Vector<String16>& args);

    /////////////////////////////////////////////////////////////////////


private:

    // Delay-lofor the service , will be called by refs->mBase->onFirstRef()
    virtual void onFirstRef();

    // IBinder::DeathRecipient implementation
    virtual void        binderDied(const wp<IBinder> &who);

};

}; // namespace NSAdvCam

}; // namespace android

#endif
