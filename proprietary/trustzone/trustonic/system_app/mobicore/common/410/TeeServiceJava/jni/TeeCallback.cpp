/*
 * Copyright (c) 2019 TRUSTONIC LIMITED
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the TRUSTONIC LIMITED nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#define LOG_TAG "com.trustonic.teeservice [ANW]"

#include <utils/Log.h>
#include "TeeCallback.h"
#include "tui_ioctl.h"

#include <gui/Surface.h>
#include <gui/SurfaceComposerClient.h>
#include <utils/String8.h>

#if defined QC_GRALLOC

#include <gralloc_priv.h>

#if defined GRALLOC1_CONSUMER_USAGE_PRIVATE_SECURE_DISPLAY
// use gralloc1 library
#define SD_WINDOW_FLAGS GRALLOC1_CONSUMER_USAGE_PRIVATE_SECURE_DISPLAY | \
            GRALLOC1_PRODUCER_USAGE_PROTECTED | \
            GRALLOC1_PRODUCER_USAGE_PRIVATE_MM_HEAP | \
            GRALLOC1_PRODUCER_USAGE_PRIVATE_1 | \
            GRALLOC_USAGE_HW_TEXTURE | \
            GRALLOC_USAGE_EXTERNAL_DISP
#else  /* GRALLOC1_CONSUMER_USAGE_PRIVATE_SECURE_DISPLAY */
#define SD_WINDOW_FLAGS GRALLOC_USAGE_PRIVATE_SECURE_DISPLAY | \
            GRALLOC_USAGE_PROTECTED | \
            GRALLOC_USAGE_PRIVATE_MM_HEAP | \
            GRALLOC_USAGE_PRIVATE_UNCACHED |  \
            GRALLOC_USAGE_HW_TEXTURE | \
            GRALLOC_USAGE_EXTERNAL_DISP
#endif /* GRALLOC1_CONSUMER_USAGE_PRIVATE_SECURE_DISPLAY */
#else /* QC_GRALLOC */
#include <hardware/gralloc.h>
// flags for Normal Window , for test purposes
#define NORMAL_WINDOW_FLAGS  GRALLOC_USAGE_SW_READ_OFTEN | GRALLOC_USAGE_SW_WRITE_OFTEN
#define SD_WINDOW_FLAGS NORMAL_WINDOW_FLAGS
#endif /* QC_GRALLOC */

// number of buffers to ask from surface flinger
// we will actually ask for NATIVE_WINDOW_MIN_UNDEQUEUED_BUFFERS + REQUIRED_BUFFERS
#define REQUIRED_BUFFERS    MAX_BUFFER_NUMBER

// Needed to access Surface* functions
using namespace android;
using Transaction = SurfaceComposerClient::Transaction;

using namespace vendor::trustonic::tee::V1_0;

/*
 * global Vars
 */
// for creating the native window
sp<Surface>          gSurface;
sp<SurfaceComposerClient> gComposerClient;
sp<SurfaceControl>        gSurfaceControl;
sp<ANativeWindow>         gNativeWindow;

//native buffers
ANativeWindowBuffer* gNativeBuffer[MAX_BUFFER_NUMBER];
// Methods from ::vendor::trustonic::tee::V1_0::ITeeCallback follow.
Return<void> TeeCallback::anwInit(uint32_t max_dequeued_buffers,
        uint32_t width, uint32_t height, anwInit_cb _hidl_cb) {
    int err = TLC_TUI_ERROR;
    int min_undequeued_buffers = 0;
    gComposerClient = new SurfaceComposerClient;
    gComposerClient->initCheck();
    gSurfaceControl = gComposerClient->createSurface(
            String8("secure-ui"), width, height,
            PIXEL_FORMAT_RGBA_8888, 0);

    gNativeWindow = gSurfaceControl->getSurface();

    err = native_window_api_connect(gNativeWindow.get(), NATIVE_WINDOW_API_CPU);
    if (err != 0) {
        ALOGE("ERROR %s:%d native_window_api_connect failed with err %d", __func__, __LINE__,
              err);
    }
    native_window_set_buffers_format(gNativeWindow.get(),
                                     PIXEL_FORMAT_RGBA_8888);

    ALOGD("Set window usage for anw=%p", gNativeWindow.get());
    err = native_window_set_usage(gNativeWindow.get(), SD_WINDOW_FLAGS );
    if (err != 0) {
        ALOGE("ERROR %s:%d set usage failed with err %d", __func__, __LINE__,
              err);
        err = TLC_TUI_ERROR;
    } else {
        /* Comments for NATIVE_WINDOW_MIN_UNDEQUEUED_BUFFERS from window.h:
         *
         * The minimum number of buffers that must remain un-dequeued after a buffer
         * has been queued.  This value applies only if set_buffer_count was used to
         * override the number of buffers and if a buffer has since been queued.
         * Users of the set_buffer_count ANativeWindow method should query this
         * value before calling set_buffer_count.  If it is necessary to have N
         * buffers simultaneously dequeued as part of the steady-state operation,
         * and this query returns M then N+M buffers should be requested via
         * native_window_set_buffer_count.
         *
         * Note that this value does NOT apply until a single buffer has been
         * queued.  In particular this means that it is possible to:
         *
         * 1. Query M = min undequeued buffers
         * 2. Set the buffer count to N + M
         * 3. Dequeue all N + M buffers
         * 4. Cancel M buffers
         * 5. Queue, dequeue, queue, dequeue, ad infinitum
         */
        /* N is max_dequeued_buffers and M is min_undequeued_buffers */
        gNativeWindow->query(gNativeWindow.get(),
                             NATIVE_WINDOW_MIN_UNDEQUEUED_BUFFERS,
                             &min_undequeued_buffers);
        ALOGD("NATIVE_WINDOW_MIN_UNDEQUEUED_BUFFERS query = %d",
              min_undequeued_buffers);

        if (min_undequeued_buffers < 0) {
            ALOGE("ERROR %s:%d min_undequeued_buffers =  %d", __func__, __LINE__,
                  min_undequeued_buffers);
            err = TLC_TUI_ERROR;
        } else {
            native_window_set_buffer_count(gNativeWindow.get(),
                                           max_dequeued_buffers + min_undequeued_buffers);
            ALOGD("Set buffer count to %d", max_dequeued_buffers + min_undequeued_buffers);
            err = TLC_TUI_OK;
        }
    }
    _hidl_cb(err, min_undequeued_buffers, 0);
    return Void();

}

Return<void> TeeCallback::anwDequeueBuffers(uint32_t buffId, anwDequeueBuffers_cb _hidl_cb) {
    int ret = TLC_TUI_OK;
    ANativeWindowBuffer* anb;

    ret = native_window_dequeue_buffer_and_wait(gNativeWindow.get(), &anb);
    if (ret != TLC_TUI_OK) {
        ALOGE("ERROR %s:%d native_window_dequeue_buffer_and_wait returned =%d",
                __func__, __LINE__, ret);
        tlcAnwClear();
        ret = TLC_TUI_ERROR;
        goto end;
    }
    //store native buffer in a global var
    gNativeBuffer[buffId] = anb;

    ALOGD("\tgNativeBuffer[%d]= %p ", buffId, gNativeBuffer[buffId]);
    ALOGD("\thandle_version= %08x, handle_numFds= %08x, handle_numInts= %08x ",
            gNativeBuffer[buffId]->handle->version, gNativeBuffer[buffId]->handle->numFds,
            gNativeBuffer[buffId]->handle->numInts);
    ALOGD("\tgNativeBuffer[%d]->handle->data[0]= %i gNativeBuffer[%d]->handle->data[1]= %i",
            buffId, gNativeBuffer[buffId]->handle->data[0], buffId, gNativeBuffer[buffId]->handle->data[1]);

end:
    _hidl_cb(ret, gNativeBuffer[buffId]->handle, gNativeBuffer[buffId]->width, gNativeBuffer[buffId]->height, gNativeBuffer[buffId]->stride); 
    return Void();
}

Return<uint32_t> TeeCallback::anwQueueBuffer(int32_t buffId) {

    uint32_t ret;
    /* check buffId */
    if ((buffId >= MAX_BUFFER_NUMBER) || (buffId < 0))
    {
        ALOGE("ERROR %s: bufferId is incorrect \n",__func__);
        return TLC_TUI_ERROR;
    }
    ALOGD("%s: queue buffer %d , anb=%p, fd = %u \n", __func__, buffId,
            gNativeBuffer[buffId], gNativeBuffer[buffId]->handle->data[0]);

    ret = gNativeWindow->queueBuffer(gNativeWindow.get(), gNativeBuffer[buffId], -1);
    if (ret != 0) {
        ALOGE("ERROR %s: queueBuffer() failed(%d) \n",__func__, ret);
    }
    gSurface = gSurfaceControl->getSurface();

    Transaction().setLayer(gSurfaceControl, 0x7ffffff).apply();

    return TLC_TUI_OK;
}

Return<uint32_t> TeeCallback::anwCancelBuffer(int32_t buffId) {

    uint32_t ret;
    /* check buffId */
    if ((buffId >= MAX_BUFFER_NUMBER) || (buffId < 0))
    {
        ALOGE("ERROR %s: bufferId is incorrect \n",__func__);
        return TLC_TUI_ERROR;
    }
    ALOGD("%s: queue buffer %d , anb=%p, fd = %u \n", __func__, buffId,
            gNativeBuffer[buffId], gNativeBuffer[buffId]->handle->data[0]);

    ret = gNativeWindow->queueBuffer(gNativeWindow.get(), gNativeBuffer[buffId], -1);
    if (ret != 0) {
        ALOGE("ERROR %s: queueBuffer() failed(%d) \n",__func__, ret);
    }
    gSurface = gSurfaceControl->getSurface();

    Transaction().setLayer(gSurfaceControl, 0x7ffffff).apply();
    native_handle_close(gNativeBuffer[buffId]->handle);
    gNativeBuffer[buffId] = NULL;

    return TLC_TUI_OK;
}

Return<uint32_t> TeeCallback::anwQueueDequeueBuffer(int32_t buffId) {
    uint32_t ret = TLC_TUI_ERROR;
    int ret_native_window = -1;
    ANativeWindowBuffer* anb;

    /* check buffId */
    if ((buffId >= MAX_BUFFER_NUMBER) || (buffId < 0))
    {
        ALOGE("ERROR %s: bufferId is incorrect \n",__func__);
        return TLC_TUI_ERROR;
    }
    ALOGD("%s: queue buffer %d , anb=%p, fd = %i \n", __func__, buffId,
            gNativeBuffer[buffId],
            gNativeBuffer[buffId]->handle->data[0]);

    ret_native_window = gNativeWindow->queueBuffer(gNativeWindow.get(),
            gNativeBuffer[buffId], -1);
    if (0 != ret_native_window) {
        ALOGE("ERROR %s:%d queueBuffer ret =%i", __func__,
                __LINE__, ret_native_window);
        return TLC_TUI_ERROR;
    }
    ALOGD("%s Buffer %i queued ret =%i", __func__, buffId,
            ret_native_window);
    ret_native_window = native_window_dequeue_buffer_and_wait(
            gNativeWindow.get(), &anb);
    if (0 != ret_native_window) {
        ALOGE("ERROR %s:%d dequeueBuffer ret =%d ",
                __func__, __LINE__, ret_native_window);
        ret = TLC_TUI_ERROR;
    }
    ALOGD("%s dequeue buff anb= %p ", __func__,anb);
    return TLC_TUI_OK;
}

/*
 * Hide the native window.
 * After this function returns, the TUI framebuffers are no longer displayed on
 * screen, but the screen remains black because it is still in secure mode.
 * NWd window can be displayed after calling tlcAnwClear()
 */
Return<void> TeeCallback::anwHide() {
    Transaction().hide(gSurfaceControl).apply();
    return Void();
}

void TeeCallback::tlcAnwClear(void)
{
    //native_window_api_disconnect(gNativeWindow.get(),  NATIVE_WINDOW_API_EGL);
    gSurface.clear();
    gSurfaceControl.clear();
}

Return<void> TeeCallback::anwClear() {
    tlcAnwClear();
    return Void();
}
