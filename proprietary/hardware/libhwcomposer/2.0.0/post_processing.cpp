#define DEBUG_LOG_TAG "POSTPROCESSING"
#define ATRACE_TAG ATRACE_TAG_GRAPHICS

#include "post_processing.h"

#include <utils/Log.h>
#include <ui/GraphicBufferMapper.h>

PostProcessingEngine::PostProcessingEngine(int dpy, sp<OverlayEngine> ovl)
    : m_id(-1)
{
    m_dpy = dpy;
    m_overlay = ovl;
}

PostProcessingEngine::~PostProcessingEngine()
{
}

void PostProcessingEngine::releaseResource()
{
    m_overlay = NULL;
}

void PostProcessingEngine::onBufferQueued()
{
    if (m_overlay != NULL)
    {
        m_overlay->updateInput(m_id);
    }
}

void PostProcessingEngine::onVSync()
{
}

void PostProcessingEngine::setChannelId(int id)
{
    m_id = id;
}

void PostProcessingEngine::registerVSyncListener(const sp<VSyncListener>& listener)
{
    m_vsync_callbacks.add(listener);
}
