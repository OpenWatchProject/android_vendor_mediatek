/*
 * Copyright (c) 2013-2018 TRUSTONIC LIMITED
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

#if defined(QC_SOURCE_TREE)
#include <gui/ISurfaceComposer.h>
#include <gui/Surface.h>
#include <gui/SurfaceComposerClient.h>
#include <utils/String8.h>
#include <qcom/display/gralloc_priv.h>

#include "tlcNativeWindow.h"

#define  LOG_TAG    "TlcNativeWindow"
#include "log.h"

// Needed to access Surface* functions
using namespace android;

/*
 * MM heap is a carveout heap for video that can be secured.  For MSM8996, it
 * should not be usefull anymore as HAL should allocate unconditionaly in SD
 * pool in case PRIVATE_SECURE_DISPLAY is used (the old pool was actually not
 * contiguous!?).  For MSM8992, this flag is mandatory.
 */
// flags to mark the Native Window for SD
#if defined GRALLOC1_CONSUMER_USAGE_PRIVATE_SECURE_DISPLAY
// use gralloc1 library
#define SD_WINDOW_FLAGS GRALLOC1_CONSUMER_USAGE_PRIVATE_SECURE_DISPLAY | \
            GRALLOC1_PRODUCER_USAGE_PROTECTED | \
            GRALLOC1_PRODUCER_USAGE_PRIVATE_MM_HEAP | \
            GRALLOC1_PRODUCER_USAGE_PRIVATE_1 | \
            GRALLOC_USAGE_HW_TEXTURE | \
            GRALLOC_USAGE_EXTERNAL_DISP
#else
#define SD_WINDOW_FLAGS GRALLOC_USAGE_PRIVATE_SECURE_DISPLAY | \
			GRALLOC_USAGE_PROTECTED | \
			GRALLOC_USAGE_PRIVATE_MM_HEAP | \
			GRALLOC_USAGE_PRIVATE_UNCACHED |  \
			GRALLOC_USAGE_HW_TEXTURE | \
			GRALLOC_USAGE_EXTERNAL_DISP
#endif

// number of buffers to ask from surface flinger
// we will actually ask for NATIVE_WINDOW_MIN_UNDEQUEUED_BUFFERS + REQUIRED_BUFFERS
#define REQUIRED_BUFFERS    MAX_BUFFER_NUMBER

// flags for Normal Window , for test purposes
#define NORMAL_WINDOW_FLAGS              GRALLOC_USAGE_SW_READ_OFTEN | GRALLOC_USAGE_SW_WRITE_OFTEN

/*
 * global Vars
 */
// for creating the native window
sp<Surface>		  gSurface;
sp<SurfaceComposerClient> gComposerClient;
sp<SurfaceControl>        gSurfaceControl;
sp<ANativeWindow>         gNativeWindow;

//native buffers
ANativeWindowBuffer* gNativeBuffer[MAX_BUFFER_NUMBER];

/*
 * Hide the native window.
 * After this function returns, the TUI framebuffers are no longer displayed on
 * screen, but the screen remains black because it is still in secure mode.
 * NWd window can be displayed after calling tlcAnwClear()
 */
void tlcAnwHide(void)
{
	gComposerClient->openGlobalTransaction();
	gSurfaceControl->hide();
	gComposerClient->closeGlobalTransaction();
}

void tlcAnwClear(void)
{
	//native_window_api_disconnect(gNativeWindow.get(),  NATIVE_WINDOW_API_EGL);
	gSurface.clear();
	gSurfaceControl.clear();
}


// same sequence for init as /frameworks/native/libs/gui/tests/Surface_test.cpp
uint32_t tlcAnwInit(uint32_t max_dequeued_buffers,
                    int *min_undequeued_buffers, uint32_t width, uint32_t height)
{
    int err = TLC_TUI_ERROR;

	gComposerClient = new SurfaceComposerClient;
	gComposerClient->initCheck();
	gSurfaceControl = gComposerClient->createSurface(
			String8("secure-ui"), width, height,
            PIXEL_FORMAT_RGBA_8888, 0);

	gNativeWindow = gSurfaceControl->getSurface();

	err = native_window_api_connect(gNativeWindow.get(), NATIVE_WINDOW_API_CPU);
	if (err != 0) {
		LOG_E("ERROR %s:%d native_window_api_connect failed with err %d", __func__, __LINE__,
		      err);
	}
	native_window_set_buffers_format(gNativeWindow.get(),
                                     PIXEL_FORMAT_RGBA_8888);

	LOG_D("Set window usage for anw=%p", gNativeWindow.get());
	err = native_window_set_usage(gNativeWindow.get(), SD_WINDOW_FLAGS );
	if (err != 0) {
		LOG_E("ERROR %s:%d set usage failed with err %d", __func__, __LINE__,
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
                             min_undequeued_buffers);
        LOG_D("NATIVE_WINDOW_MIN_UNDEQUEUED_BUFFERS query = %d",
              *min_undequeued_buffers);

        if (*min_undequeued_buffers < 0) {
            LOG_E("ERROR %s:%d min_undequeued_buffers =  %d", __func__, __LINE__,
                  *min_undequeued_buffers);
            err = TLC_TUI_ERROR;
        } else {
            native_window_set_buffer_count(gNativeWindow.get(),
                                           max_dequeued_buffers + *min_undequeued_buffers);
            LOG_D("Set buffer count to %d", max_dequeued_buffers + *min_undequeued_buffers);
            err = TLC_TUI_OK;
        }
    }

    return err;
 }

/*
 * Dequeue n numbers of buffers and store fd handles in ionHandle[]
 */
uint32_t tlcAnwDequeueBuffers(int *ionHandle,
                              uint32_t num_of_buff,
                              uint32_t *pWidth,
                              uint32_t *pHeight,
                              uint32_t *pStride)
{
    int ret = TLC_TUI_OK;
    uint32_t i;
    ANativeWindowBuffer* anb;

    if (MAX_BUFFER_NUMBER < num_of_buff) {
        LOG_E("ERROR %s:%d Can not dequeue more than max number of buffers (%d\n",
        __func__, __LINE__, MAX_BUFFER_NUMBER);
        return TLC_TUI_ERROR;
    }

    LOG_D("%s: dequeue %u buffs \n", __func__, num_of_buff);
    for (i = 0; i < num_of_buff; i++) {
        ret = native_window_dequeue_buffer_and_wait(gNativeWindow.get(), &anb);
        if (ret != TLC_TUI_OK) {
            LOG_E("ERROR %s:%d native_window_dequeue_buffer_and_wait returned =%d",
                  __func__, __LINE__, ret);
            tlcAnwClear();
            return TLC_TUI_ERROR;
        }
        //store native buffer in a global var
        gNativeBuffer[i] = anb;

        /* TODO Error check on ionHandle, negative values are errors */
        ionHandle[i] = anb->handle->data[0];
        LOG_D("\tanb= %p ", anb);
        LOG_D("\thandle_version= %08x, handle_numFds= %08x, handle_numInts= %08x ",
                anb->handle->version, anb->handle->numFds,
                anb->handle->numInts);
        LOG_D("\tanb->handle->data[0]= %i anb->handle->data[1]= %i",
                anb->handle->data[0], anb->handle->data[1]);
        LOG_D("\tanb %d x %d stride:%d - format:%08x usage: %08x", anb->width,
                anb->height, anb->stride, anb->format, anb->usage);
    }
    *pWidth = anb->width;
    *pHeight = anb->height;
    *pStride = anb->stride;

    return TLC_TUI_OK;
}

/*
 * Queue buffer with a specific id, buffId
 */
uint32_t tlcAnwQueueBuffer(int buffId)
{
	uint32_t ret = TLC_TUI_OK;

	LOG_D("%s: queue buffer %d , anb=%p, fd = %u \n", __func__, buffId,
			gNativeBuffer[buffId], gNativeBuffer[buffId]->handle->data[0]);
	/* check buffId */
	if ((buffId > MAX_BUFFER_NUMBER) || (buffId < 0))
	{
		LOG_E("ERROR %s: bufferId is incorrect \n",__func__);
		return TLC_TUI_ERROR;
	}

	ret = gNativeWindow->queueBuffer(gNativeWindow.get(), gNativeBuffer[buffId], -1);
	gSurface = gSurfaceControl->getSurface();

	gComposerClient->openGlobalTransaction();
	gSurfaceControl->setLayer(0x7fffffff);
	gSurfaceControl->show();
	gComposerClient->closeGlobalTransaction();

	return ret;
}

/*
 * Queue buffer with a specific id, buffId
 * And dequeue another one.
 */
uint32_t tlcAnwQueueDequeueBuffer(int buffId)
{
    uint32_t ret = TLC_TUI_ERROR;
    int ret_native_window = -1;
    ANativeWindowBuffer* anb;

    LOG_D("%s: queue buffer %d , anb=%p, fd = %i \n", __func__, buffId,
         gNativeBuffer[buffId],
         gNativeBuffer[buffId]->handle->data[0]);
    /* check buffId */
    if ((buffId > MAX_BUFFER_NUMBER) || (buffId < 0))
    {
        LOG_E("ERROR %s: bufferId is incorrect \n",__func__);
        ret = TLC_TUI_ERROR;
    } else {
        ret_native_window = gNativeWindow->queueBuffer(gNativeWindow.get(),
                                                    gNativeBuffer[buffId], -1);
        if (0 != ret_native_window) {
            LOG_E("ERROR %s:%d queueBuffer ret =%i", __func__,
                 __LINE__, ret_native_window);
            ret = TLC_TUI_ERROR;
        } else {
            LOG_D("%s Buffer %i queued ret =%i", __func__, buffId,
                 ret_native_window);
            ret_native_window =
                native_window_dequeue_buffer_and_wait(
                                                    gNativeWindow.get(), &anb);
            if (0 != ret_native_window) {
                LOG_E("ERROR %s:%d dequeueBuffer ret =%d ",
                     __func__, __LINE__, ret_native_window);
                ret = TLC_TUI_ERROR;
            } else {
                LOG_D("%s dequeue buff anb= %p ", __func__,anb);
                ret = TLC_TUI_OK;
            }
        }
    }

    return ret;
}

/*
 * Cancel buffer with a specific id, buffId
 */
uint32_t tlcAnwCancelBuffer(int buffId)
{
	uint32_t ret = TLC_TUI_OK;

	LOG_D("%s: cancel buffer %d , anb=%p, fd = %08x \n", __func__, buffId,
			gNativeBuffer[buffId], gNativeBuffer[buffId]->handle->data[0]);
	/* check buffId */
	if ((buffId > MAX_BUFFER_NUMBER) || (buffId < 0))
	{
		LOG_E("ERROR %s: bufferId is incorrect \n",__func__);
		return TLC_TUI_ERROR;
	}

	ret = gNativeWindow->cancelBuffer(gNativeWindow.get(), gNativeBuffer[buffId], -1);
	return ret;
}

#endif
