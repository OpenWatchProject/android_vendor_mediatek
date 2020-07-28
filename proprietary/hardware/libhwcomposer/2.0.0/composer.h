#ifndef HWC_COMPOSER_H_
#define HWC_COMPOSER_H_

#include <utils/KeyedVector.h>

#include <ui/Rect.h>

using namespace android;

struct hwc_display_contents_1;
struct hwc_layer_1;
struct DispatcherJob;
struct DisplayData;
class OverlayEngine;
class SyncFence;

// ---------------------------------------------------------------------------

class LayerHandler : public LightRefBase<LayerHandler>
{
public:
    LayerHandler(int dpy, const sp<OverlayEngine>& ovl_engine);
    virtual ~LayerHandler();

    // set() is used to set data for next round
    virtual void set(const sp<HWCDisplay>& display, DispatcherJob* job) = 0;

    // process() is used to ask composer to work
    virtual void process(DispatcherJob* job) = 0;

    // nullop() is used to clear state
    virtual void nullop() { }

    // release fence by specified marker
    virtual void nullop(const int32_t& /*job_id*/) { }

    // release fence by specified marker
    virtual void nullop(int /*marker*/, int /*fd*/) { }

    // boost() is used to make sure if need to boost thread priority
    virtual void boost() { }

    // dump() is used for debugging purpose
    virtual int dump(char* /*buff*/, int /*buff_len*/, int /*dump_level*/) { return 0; }

    // cancelLayers() is used to cancel layers of dropped job
    virtual void cancelLayers(DispatcherJob* /*job*/) { }

protected:
    // m_disp_id is used to identify
    // LayerHandler needs to handle layers in which display
    int m_disp_id;

    // m_ovl_engine is used for LayerHandler to config overlay engine
    sp<OverlayEngine> m_ovl_engine;

    // m_sync_fence is used to create or wait fence
    sp<SyncFence> m_sync_fence;
};

class ComposerHandler : public LayerHandler
{
public:
    ComposerHandler(int dpy, const sp<OverlayEngine>& ovl_engine);

    // set() in ComposerHandler is used to get release fence from display driver
    virtual void set(const sp<HWCDisplay>& display, DispatcherJob* job);

    // process() in ComposerHandler is used to wait each layer is ready to use
    // and fill configuration to display driver
    virtual void process(DispatcherJob* job);

    // cancelLayers() in ComposerHandler is used to cancel each ui layers of job
    virtual void cancelLayers(DispatcherJob* job);
};

#endif // HWC_COMPOSER_H_
