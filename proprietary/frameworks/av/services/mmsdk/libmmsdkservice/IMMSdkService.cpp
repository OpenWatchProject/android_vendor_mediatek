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

#define LOG_TAG "mmsdk/BpMMSdkService"
#include <utils/Log.h>

#include <stdint.h>
#include <sys/types.h>

#include <binder/Parcel.h>
#include <binder/IPCThreadState.h>
#include <binder/IServiceManager.h>
#include <binder/IInterface.h>
//
#include <mmsdk/IMMSdkService.h>

//
#include <mmsdk/IFeatureManager.h>


using namespace android;
using namespace NSMMSdk;

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

class BpMMSdkService: public BpInterface<IMMSdkService>
{
public:
    BpMMSdkService(const sp<IBinder>& impl)
        : BpInterface<IMMSdkService>(impl)
    {
    }
    //
    virtual status_t connectFeatureManager(sp<NSCam::IFeatureManager> & featureManager)
    {
        Parcel data, reply;
        data.writeInterfaceToken(IMMSdkService::getInterfaceDescriptor());
        remote()->transact(BnMMSdkService::GET_MMSDK_FEAUTRE_MANAGER, data, &reply);

        if (readExceptionCode(reply)) return -EPROTO;
        status_t status = reply.readInt32();
        if (reply.readInt32() != 0)
        {
            featureManager = interface_cast<NSCam::IFeatureManager>(reply.readStrongBinder());
        }

        return status;
    }
    virtual status_t existCallbackClient()
    {
        Parcel data, reply;
        data.writeInterfaceToken(IMMSdkService::getInterfaceDescriptor());
        remote()->transact(BnMMSdkService::EXIST_CALLBACK_CLIENT, data, &reply);

        if (readExceptionCode(reply)) return -EPROTO;
        status_t status = reply.readInt32();
        return status;
    }

};

IMPLEMENT_META_INTERFACE(MMSdkService, "com.mediatek.mmsdk.IMMSdkService");

// ----------------------------------------------------------------------

status_t BnMMSdkService::onTransact(
    uint32_t code, const Parcel& data, Parcel* reply, uint32_t flags)
{
    switch(code) {
        case GET_MMSDK_FEAUTRE_MANAGER: {
            CHECK_INTERFACE(IMMSdkService, data, reply);
            sp<NSCam::IFeatureManager> client;
            status_t status = connectFeatureManager(client);
            reply->writeNoException();
            reply->writeInt32(status);
            if (client != 0)
            {
                reply->writeInt32(1);
                reply->writeStrongBinder(IInterface::asBinder(client));
            }
            else
            {
                reply->writeInt32(0);
            }
            return NO_ERROR;
        } break;
        case EXIST_CALLBACK_CLIENT: {
            CHECK_INTERFACE(IMMSdkService, data, reply);
            status_t status = existCallbackClient();
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
