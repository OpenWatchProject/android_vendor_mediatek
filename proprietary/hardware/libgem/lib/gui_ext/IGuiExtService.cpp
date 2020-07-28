#define LOG_TAG "GuiExt"

#define MTK_LOG_ENABLE 1
#include <utils/Errors.h>
#include <utils/RefBase.h>
#include <utils/Vector.h>
#include <utils/Timers.h>
#include <utils/String8.h>

#include <binder/Parcel.h>
#include <binder/IInterface.h>

#include <cutils/log.h>

#include <ui/GraphicBuffer.h>

#include <mediatek/IDumpTunnel.h>
#include "gui_ext/IGuiExtService.h"

namespace android {

// client : proxy GuiEx class
class BpGuiExtService : public BpInterface<IGuiExtService>
{
public:
    BpGuiExtService(const sp<IBinder>& impl) : BpInterface<IGuiExtService>(impl)
    {
    }

    virtual status_t regDump(const sp<IDumpTunnel>& tunnel, const String8& key)
    {
        Parcel data, reply;
        data.writeInterfaceToken(IGuiExtService::getInterfaceDescriptor());
        data.writeStrongBinder(tunnel->asBinder(tunnel));
        data.writeString8(key);
        status_t result = remote()->transact(GUI_EXT_REGDUMP, data, &reply);
        if (result != NO_ERROR)
        {
            ALOGE("regDump() error(%d) in %s\n", result, __func__);
            return result;
        }
        result = reply.readInt32();
        return result;
    }

    virtual status_t unregDump(const String8& key)
    {
        Parcel data, reply;
        data.writeInterfaceToken(IGuiExtService::getInterfaceDescriptor());
        data.writeString8(key);
        status_t result = remote()->transact(GUI_EXT_UNREGDUMP, data, &reply);
        if (result != NO_ERROR)
        {
            ALOGE("unregDump() error(%d) in %s\n", result, __func__);
            return result;
        }
        result = reply.readInt32();
        return result;
    }
};

IMPLEMENT_META_INTERFACE(GuiExtService, "GuiExtService");

status_t BnGuiExtService::onTransact(uint32_t code, const Parcel& data, Parcel* reply, uint32_t flags)
{
    //ALOGD("receieve the command code %d", code);

    switch(code)
    {
        case GUI_EXT_REGDUMP:
        {
            CHECK_INTERFACE(IGuiExtService, data, reply);
            sp<IDumpTunnel> tunnel = interface_cast<IDumpTunnel>(data.readStrongBinder());
            String8 key = data.readString8();
            status_t result = regDump(tunnel, key);
            reply->writeInt32(result);
            return NO_ERROR;
        }
        break;
        case GUI_EXT_UNREGDUMP:
        {
            CHECK_INTERFACE(IGuiExtService, data, reply);
            String8 key = data.readString8();
            status_t result = unregDump(key);
            reply->writeInt32(result);
            return NO_ERROR;
        }
        break;
    }
    return BBinder::onTransact(code, data, reply, flags);
}

sp<IGuiExtService> checkGuiExtService()
{
    const sp<IServiceManager> sm = defaultServiceManager();
    if (sm != NULL) {
        // use non-blocking way to get GuiExtService with ServiceManager
        sp<IBinder> binder = sm->checkService(String16("GuiExtService"));
        if (binder != NULL) {
            return interface_cast<IGuiExtService>(binder);
        }
        ALOGW("Cannot find GuiExtService");
        return NULL;
    }
    ALOGW("Cannot find default ServiceManager");
    return NULL;
}
};

using namespace android;

bool regDump(const sp<IDumpTunnel>& tunnel, const String8& key)
{
#ifndef MTK_DO_NOT_USE_GUI_EXT
    bool result = false;
    sp<IGuiExtService> guiExt = checkGuiExtService();
    if (guiExt != NULL) {
        result = (guiExt->regDump(tunnel, key) == NO_ERROR);
    }
    return result;
#endif
}

bool unregDump(const String8& key)
{
#ifndef MTK_DO_NOT_USE_GUI_EXT
    bool result = false;
    sp<IGuiExtService> guiExt = checkGuiExtService();
    if (guiExt != NULL) {
        result = (guiExt->unregDump(key) == NO_ERROR);
    }
    return result;
#endif
}
