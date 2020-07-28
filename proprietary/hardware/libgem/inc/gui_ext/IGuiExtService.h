#pragma GCC system_header
#ifndef __IGUIEXTSERVICE_H__
#define __IGUIEXTSERVICE_H__

#include <binder/IInterface.h>
#include <binder/Parcel.h>
#include <binder/BinderService.h>


namespace android
{
//
//  Holder service for pass objects between processes.
//

class IDumpTunnel;
class String8;

class IGuiExtService : public IInterface
{
protected:
    enum {
        GUI_EXT_REGDUMP,
        GUI_EXT_UNREGDUMP,
    };

public:
    DECLARE_META_INTERFACE(GuiExtService);

    // for dump tunnel
    virtual status_t regDump(const sp<IDumpTunnel>& tunnel, const String8& key) = 0;
    virtual status_t unregDump(const String8& key) = 0;
};

class BnGuiExtService : public BnInterface<IGuiExtService>
{
    virtual status_t onTransact(uint32_t code,
                                const Parcel& data,
                                Parcel* reply,
                                uint32_t flags = 0);
};

// an easy way to check and get the GuiExtService without blocking
sp<IGuiExtService> checkGuiExtService();
};

using namespace android;

extern "C"
{
    // The circular link must be broken because libgui.so and libgui_ext.so needs to link each other
    // libgui.so uses dlopen() to load libgui_ext.so at runtime
    bool regDump(const sp<IDumpTunnel>& tunnel, const String8& key);
    bool unregDump(const String8& key);
    extern void createGuiExtService();
}
#endif
