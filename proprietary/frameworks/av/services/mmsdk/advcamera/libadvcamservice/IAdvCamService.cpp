/*
**
** Copyright 2008, The Android Open Source Project
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

#define LOG_TAG "advcam/IAdvCamService"
#include <utils/Log.h>

#include <stdint.h>
#include <sys/types.h>

#include <gui/Surface.h> // for Capture Request use
#include <camera/camera2/CaptureRequest.h>
#include <binder/Parcel.h>
#include <binder/IPCThreadState.h>
#include <binder/IServiceManager.h>
#include <binder/IInterface.h>
//
#include <advcam/IAdvCamService.h>

using namespace android;
using namespace NSAdvCam;

namespace android {
namespace {

enum {
    EX_SECURITY = -1,
    EX_BAD_PARCELABLE = -2,
    EX_ILLEGAL_ARGUMENT = -3,
    EX_NULL_POINTER = -4,
    EX_ILLEGAL_STATE = -5,
    EX_HAS_REPLY_HEADER = -128,  // special; see below
};

static bool readExceptionCode(Parcel& reply) {
    int32_t exceptionCode = reply.readExceptionCode();

    if (exceptionCode != 0) {
        const char* errorMsg;
        switch(exceptionCode) {
            case EX_SECURITY:
                errorMsg = "Security";
                break;
            case EX_BAD_PARCELABLE:
                errorMsg = "BadParcelable";
                break;
            case EX_NULL_POINTER:
                errorMsg = "NullPointer";
                break;
            case EX_ILLEGAL_STATE:
                errorMsg = "IllegalState";
                break;
            // Binder should be handling this code inside Parcel::readException
            // but lets have a to-string here anyway just in case.
            case EX_HAS_REPLY_HEADER:
                errorMsg = "HasReplyHeader";
                break;
            default:
                errorMsg = "Unknown";
        }

        ALOGE("Binder transmission error %s (%d)", errorMsg, exceptionCode);
        return true;
    }

    return false;
}

};

class BpAdvCamService: public BpInterface<IAdvCamService>
{
public:
    BpAdvCamService(const sp<IBinder>& impl)
        : BpInterface<IAdvCamService>(impl)
    {
    }
    //
    virtual status_t setConfigureParam( uint32_t openId, CaptureRequest& request)
    {
        Parcel data, reply;
        data.writeInterfaceToken(IAdvCamService::getInterfaceDescriptor());
        data.writeInt32(openId);
        data.writeInt32(1); // object is not null
        request.writeToParcel(&data);
        remote()->transact(BnAdvCamService::SET_CONFIGURE_PARAM, data, &reply);

        if (readExceptionCode(reply)) return -EPROTO;

        return OK;
    }

};

IMPLEMENT_META_INTERFACE(AdvCamService, "com.mediatek.advcam.IAdvCamService");

// ----------------------------------------------------------------------

status_t BnAdvCamService::onTransact(
    uint32_t code, const Parcel& data, Parcel* reply, uint32_t flags)
{
    switch(code) {
        case SET_CONFIGURE_PARAM: {
            CHECK_INTERFACE(IAdvCamService, data, reply);
            CaptureRequest capReq;
            uint32_t openId = data.readInt32();
            if(data.readInt32() == 1)
                capReq.readFromParcel(&data);
            else
                ALOGE("Get NULL CaptureRequest in setConfigureParam() IPC");

            status_t status = setConfigureParam(openId, capReq);
            reply->writeNoException();
            reply->writeInt32(status);

            return NO_ERROR;
        } break;

        default:
            return BBinder::onTransact(code, data, reply, flags);
    }
}

// ----------------------------------------------------------------------------

}; // namespace android
