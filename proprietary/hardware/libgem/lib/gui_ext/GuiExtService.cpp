#define LOG_TAG "GuiExt"

#define MTK_LOG_ENABLE 1
#include <dlfcn.h>
#include <fcntl.h>
#include <math.h>
#include <string.h>
#include <cutils/log.h>
#include <cutils/properties.h>
#include <utils/SortedVector.h>
#include <binder/PermissionCache.h>

#include <gui/ISurfaceComposer.h>
#include <gui/SurfaceComposerClient.h>

#include <ui/DisplayInfo.h>

#include <mediatek/IDumpTunnel.h>

#include <cutils/memory.h>

#include "gui_ext/GuiExtService.h"

namespace android {

#define GUIEXT_LOGV(x, ...) ALOGV("[GuiExtS] " x, ##__VA_ARGS__)
#define GUIEXT_LOGD(x, ...) ALOGD("[GuiExtS] " x, ##__VA_ARGS__)
#define GUIEXT_LOGI(x, ...) ALOGI("[GuiExtS] " x, ##__VA_ARGS__)
#define GUIEXT_LOGW(x, ...) ALOGW("[GuiExtS] " x, ##__VA_ARGS__)
#define GUIEXT_LOGE(x, ...) ALOGE("[GuiExtS] " x, ##__VA_ARGS__)

GuiExtService::GuiExtService()
    : mNeedKickDump(false)
{
    GUIEXT_LOGI("GuiExtService ctor");
}

GuiExtService::~GuiExtService()
{
}

static const String16 sDump("android.permission.DUMP");
status_t GuiExtService::dump(int fd, const Vector<String16>& args)
{
    String8 result;

    if (!PermissionCache::checkCallingPermission(sDump)) {
        result.appendFormat(
                "Permission Denial: can't dump SurfaceFlinger from pid=%d, uid=%d\n",
                IPCThreadState::self()->getCallingPid(),
                IPCThreadState::self()->getCallingUid());
    } else {
        // Try to get the main lock, but don't insist if we can't
        // (this would indicate GuiExtService is stuck, but we want to be able to
        // print something in dumpsys).
        int retry = 3;
        while ((mLock.tryLock() < 0) && (--retry >= 0)) {
            usleep(1000000);
        }
        const bool locked(retry >= 0);
        if (!locked) {
            result.appendFormat(
                    "GuiExtService appears to be unresponsive, dumping anyways (no locks held)\n");
        }

        result.appendFormat(
                "GuiExtService[pid=%d] state:\n\n",
                getpid());

        if (locked) {
            mLock.unlock();
        }

        {
            Mutex::Autolock l(mDumpLock);
            parseArgs(args);
            result.appendFormat(
                    "\n\nRegistered Tunnels state: (total %zu tunnels)\n",
                    mDumpTunnels.size());

            // into groups, currently for BufferQueue, RefBase, others
            KeyedVector<String8, sp<IDumpTunnel> > zombieTunnels;            // TODO: make it not happen !
            KeyedVector<String8, sp<IDumpTunnel> > bufferQueueTunnels;
            KeyedVector<String8, sp<IDumpTunnel> > refBaseTunnels;
            KeyedVector<String8, sp<IDumpTunnel> > otherTunnels;
            for (uint32_t i = 0; i < mDumpTunnels.size(); ++i) {
                const String8& key = mDumpTunnels.keyAt(i);
                const sp<IDumpTunnel>& tunnel = mDumpTunnels.valueAt(i);

                if (!tunnel->asBinder(tunnel)->isBinderAlive()) {
                    zombieTunnels.add(key, tunnel);
                } else if (key.find("BQ") == 0) {
                    bufferQueueTunnels.add(key, tunnel);
                } else if (key.find("RB") == 0) {
                    refBaseTunnels.add(key, tunnel);
                } else {
                    otherTunnels.add(key, tunnel);
                }
            }

            result.appendFormat(
                    "\nZOMBIE: %zu\n"
                    "--------------------------------------------------\n",
                    zombieTunnels.size());
            for (uint32_t i = 0; i < zombieTunnels.size(); ++i) {
                const String8& key = zombieTunnels.keyAt(i);
                const sp<IDumpTunnel>& tunnel = zombieTunnels.valueAt(i);
                result.appendFormat("+ %s\n", key.string());
                if (mNeedKickDump)
                {
                    tunnel->kickDump(result, "    ");
                }
            }
            result.append("--------------------------------------------------\n");

            result.appendFormat(
                    "\nBufferQueue: %zu\n"
                    "--------------------------------------------------\n",
                    bufferQueueTunnels.size());
            for (uint32_t i = 0; i < bufferQueueTunnels.size(); ++i) {
                const String8& key = bufferQueueTunnels.keyAt(i);
                const sp<IDumpTunnel>& tunnel = bufferQueueTunnels.valueAt(i);
                result.appendFormat("+ %s\n", key.string());
                if (mNeedKickDump)
                {
                    tunnel->kickDump(result, "    ");
                }
            }
            result.append("--------------------------------------------------\n");

            result.appendFormat(
                    "\nRefBase: %zu\n"
                    "--------------------------------------------------\n",
                    refBaseTunnels.size());
            for (uint32_t i = 0; i < refBaseTunnels.size(); i++) {
                const String8& key = refBaseTunnels.keyAt(i);
                const sp<IDumpTunnel>& tunnel = refBaseTunnels.valueAt(i);
                result.appendFormat("+ %s\n", key.string());
                if (mNeedKickDump)
                {
                    tunnel->kickDump(result, "    ");
                }
            }
            result.append("--------------------------------------------------\n");

            result.appendFormat(
                    "\nOthers: %zu\n"
                    "--------------------------------------------------\n",
                    otherTunnels.size());
            for (uint32_t i = 0; i < otherTunnels.size(); i++) {
                const String8& key = otherTunnels.keyAt(i);
                const sp<IDumpTunnel>& tunnel = otherTunnels.valueAt(i);
                result.appendFormat("+ %s\n", key.string());
                if (mNeedKickDump)
                {
                    tunnel->kickDump(result, "    ");
                }
            }
        } // Mutex::Autolock l(mDumpLock);
    }
    write(fd, result.string(), result.size());
    return NO_ERROR;
}

status_t GuiExtService::regDump(const sp<IDumpTunnel>& tunnel, const String8& key)
{
    if (!tunnel->asBinder(tunnel)->isBinderAlive())
        return BAD_VALUE;

    class DeathNotifier : public IBinder::DeathRecipient
    {
    private:
        const String8 mKey;
        const wp<GuiExtService> mService;
    public:
        DeathNotifier(const String8& key, const wp<GuiExtService>& service)
            : mKey(key)
            , mService(service)
        {
            // since DeathNotifier will not be kept by linkToDeath
            // we have to extend its lifetime manually
            extendObjectLifetime(OBJECT_LIFETIME_WEAK);
        }

        virtual void binderDied(const wp<IBinder>& /*who*/)
        {
            GUIEXT_LOGD("binder of dump tunnel(%s) died", mKey.string());

            sp<GuiExtService> service = mService.promote();
            if (service != NULL)
                service->unregDump(mKey);
        }
    };
    sp<IBinder::DeathRecipient> notifier = new DeathNotifier(key, this);
    if (notifier != NULL)
        tunnel->asBinder(tunnel)->linkToDeath(notifier);

    {
        Mutex::Autolock l(mDumpLock);

        if (mDumpTunnels.size() > 200)
        {
            const int32_t before = mDumpTunnels.size();

            // loop and remove zombie objects
            for (int32_t i = (before - 1); i >= 0; i--)
            {
                const sp<IDumpTunnel>& t = mDumpTunnels[i];
                if (!t->asBinder(t)->isBinderAlive())
                {
                    mDumpTunnels.removeItemsAt(i);
                }
            }

            GUIEXT_LOGI("mDumpTunnels checked (before=%d, after:%zu", before, mDumpTunnels.size());
        }

        mDumpTunnels.add(key, tunnel);
    }
    return NO_ERROR;
}

status_t GuiExtService::unregDump(const String8& key)
{
    Mutex::Autolock l(mDumpLock);
    mDumpTunnels.removeItem(key);
    return NO_ERROR;
}

void GuiExtService::parseArgs(const Vector<String16>& args)
{
    mNeedKickDump = false;

    for (size_t i = 0; i < args.size(); i++)
    {
        if (args[i] == String16("--detail"))
        {
            mNeedKickDump = true;
        }
    }
}

extern "C" void createGuiExtService()
{
#ifndef MTK_DO_NOT_USE_GUI_EXT
    const sp<IServiceManager> sm = defaultServiceManager();
    if (sm != NULL) {
        sp<IBinder> binder = sm->checkService(String16(GuiExtService::getServiceName()));
        if (binder != NULL) {
            ALOGI("GuiExtService exists");
        } else {
            sp<GuiExtService> guiext = new GuiExtService();
            sm->addService(String16(GuiExtService::getServiceName()), guiext, false);
        }
    } else {
        ALOGW("Cannot find default ServiceManager");
    }
#endif
}
};
