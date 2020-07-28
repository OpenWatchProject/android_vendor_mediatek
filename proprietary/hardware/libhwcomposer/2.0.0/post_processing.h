#ifndef HWC_POST_PROCESSING_H_
#define HWC_POST_PROCESSING_H_

#include "sync.h"
#include "utils/tools.h"
#include "overlay.h"
#include "queue.h"
#include "dispatcher.h"

class PostProcessingEngine : public DisplayBufferQueue::ConsumerListener, public HWCDispatcher::VSyncListener
{
public:
    PostProcessingEngine(int dpy, sp<OverlayEngine> ovl);
    ~PostProcessingEngine();

    virtual void onBufferQueued();
    virtual void setChannelId(int id);

    void onVSync(void);
    void releaseResource();
    void registerVSyncListener(const sp<VSyncListener>& listener);

protected:
    int m_dpy;
    int m_id;
    sp<OverlayEngine> m_overlay;
    Vector<sp<VSyncListener> > m_vsync_callbacks;
};

#endif
