/*
 * Copyright (c) 2013-2017 TRUSTONIC LIMITED
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

#include <sys/ioctl.h>

#include "TuiManager.h"

namespace vendor {
namespace trustonic {
namespace tee {
namespace V1_0 {
namespace implementation {

struct TuiManager::Impl {

    std::mutex serverThreadMutex;
    std::thread serverThread;
    int32_t drvFd;
    int ionDevFd;

    ::android::sp<::vendor::trustonic::tee::V1_0::ITuiCallback> tui_callback;

    bool startTuiThread(void);
    void serverThreadFunction();
    bool tlcWaitCmdFromDriver(tlc_tui_command_t *tuiCmd);
    bool tlcProcessCmd(struct tlc_tui_command_t tuiCmd);

    struct GetResolutionOut {
        uint32_t status;
        uint32_t width;
        uint32_t height;

        void callback(uint32_t status, uint32_t width, uint32_t height) {
            this->status = status;
            this->width = width;
            this->height = height;
        }
    };
};

void TuiManager::Impl::serverThreadFunction()
{
    struct tlc_tui_command_t tuiCmd;

    ALOGD("%s: TlcTui start!", __func__);

    drvFd = ::open("/dev/" TUI_DEV_NAME, O_NONBLOCK);
    if (drvFd < 0) {
        ALOGE("%s: open k-tlc device failed with errno %s.",
                __func__, strerror(errno));
        return;
    }

    /* TlcTui main thread loop */
    for (;;) {
        /* Wait for a command from the k-TLC*/
        if (!tlcWaitCmdFromDriver(&tuiCmd)) {
            break;
        }
        /* Something has been received, process it. */
        if (!tlcProcessCmd(tuiCmd)) {
            break;
        }
    }
}

bool TuiManager::Impl::tlcWaitCmdFromDriver(tlc_tui_command_t *tuiCmd)
{
    uint32_t cmdId = 0;
    int ioctlRet = 0;

    tuiCmd->id = 0;
    tuiCmd->data[0] = 0;
    tuiCmd->data[1] = 0;

    /* Wait for ioctl to return from k-tlc with a command ID */
    /* Loop if ioctl has been interrupted. */
    do {
        ioctlRet = ::ioctl(drvFd, TUI_IO_WAITCMD, tuiCmd);
    } while((EINTR == errno) && (-1 == ioctlRet));

    if (-1 == ioctlRet) {
        ALOGE("TUI_IO_WAITCMD ioctl failed with errno %s.", strerror(errno));
        return false;
    } else {
        cmdId = tuiCmd->id;
        ALOGI("TUI_IO_WAITCMD ioctl returnd with cmdId %d.", cmdId);
    }

    return true;
}

bool TuiManager::Impl::tlcProcessCmd(struct tlc_tui_command_t tuiCmd)
{
    unsigned int numOfBuff = 0;
    uint32_t ret = TLC_TUI_ERROR;
    struct tlc_tui_response_t response;
    uint32_t commandId = tuiCmd.id;
#ifdef QC_SOURCE_TREE
    int ion_buffer_fd[MAX_BUFFER_NUMBER] = {0};
    int min_undequeued_buff = -1;
#endif
    uint32_t sizeOfBuff = 0;
    uint32_t width=0, height=0, stride=0;
    bool acknowledge = true;
    GetResolutionOut get_res_out;

    memset(&response, 0, sizeof(tlc_tui_response_t));

    switch (commandId) {
        case TLC_TUI_CMD_NONE:
            ALOGI("tlcProcessCmd: TLC_TUI_CMD_NONE.");
            acknowledge = false;
            break;

        case TLC_TUI_CMD_START_ACTIVITY:
            ALOGD("tlcProcessCmd: TLC_TUI_CMD_START_ACTIVITY.");
            if (!tui_callback) {
                ALOGD("tlcProcessCmd: No callback registered.");
                break;
            }
            ret = tui_callback->startSession();
            ALOGD("tlcStartTuiSession returned %d", ret);
            if (ret) {
                acknowledge = false;
            }

#ifdef TEST_SECURITY
            if (pthread_create(&testThreadId, NULL, &testThread, NULL) != 0) {
                ALOGE("ERROR %s:%i pthread_create failed!", __func__, __LINE__);
            }
#endif
            ALOGI("TLC_TUI_CMD_START_ACTIVITY returned %d", ret);
            break;

        case TLC_TUI_CMD_STOP_ACTIVITY:
            ALOGD("tlcProcessCmd: TLC_TUI_CMD_STOP_ACTIVITY.");
            ret = tui_callback->stopSession();
            ALOGD("tlcFinishTuiSession returned %d", ret);
            break;

        case TLC_TUI_CMD_ALLOC_FB:
            ALOGI("tlcProcessCmd: TLC_TUI_CMD_ALLOC_FB.");

            numOfBuff = tuiCmd.data[0];
            sizeOfBuff = tuiCmd.data[1];
            ALOGI("%s: numOfBuff = %u", __func__, numOfBuff);
            ALOGI("%s: sizeOfBuff = %u", __func__, sizeOfBuff);
            ALOGI("%s: Init Native Window", __func__);
            // Init native window

            tui_callback->getResolution(std::bind(&GetResolutionOut::callback, &get_res_out,
                                                        std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));

            if (get_res_out.status != 0) {
                ALOGE("%s returned an error (%d)", "tlcGetResolution", ret);
                ret = TLC_TUI_ERROR;
            }

#ifdef QC_SOURCE_TREE
            /* Assuming that the FBs management scheme will always keep at least
             * one buffer un-dequeued:
             * max_dequeued_buffer_number is numOfBuff-1 */
            ret = tlcAnwInit(numOfBuff-1, &min_undequeued_buff, tui_dim.width, tui_dim.height);
            if (ret != TLC_TUI_OK) {
                ALOGE("tlcAnwInit returned %d", ret);
                break;
            }
#else
            ret = -1;
#endif

            ALOGI("%s: Dequeue buffers", __func__);
#ifdef QC_SOURCE_TREE
            ret = tlcAnwDequeueBuffers(ion_buffer_fd,
                                       numOfBuff-1 + min_undequeued_buff,
                                       &width, &height, &stride);
#else
            ret = TLC_TUI_ERROR;
#endif
            ALOGI("%s: width = %u height = %u stride = %u", __func__, width,
                  height, stride);
            response.screen_metrics[0] = width;
            response.screen_metrics[1] = height;
            response.screen_metrics[2] = stride;
            if (ret != TLC_TUI_OK) {
                ALOGE("tlcAnwDequeueBuffers returned %d", ret);
                break;
            }

            // Open ion device driver
            if (ionDevFd < 0) {
                ionDevFd = open("/dev/ion", O_RDONLY);
            }

            if (ionDevFd < 0) {
                ALOGE("mainThread: open ION device failed with errno %s.",
                      strerror(errno));
                exit(1);
            }

            for(uint32_t i=0; i<numOfBuff; i++) {
#ifdef QC_SOURCE_TREE
                ret = get_ion_handle(ionDevFd, ion_buffer_fd[i], &ion_data[i]);
#else
                ret = TLC_TUI_ERROR;
#endif
                if (TLC_TUI_OK != ret) {
                    ALOGE("could not get ion phys addr for handle %d", i);
                    break;
                }
#ifdef QC_SOURCE_TREE
                ALOGI("%s ion_data[%d].handle=%d ion_data[%d].fd=%d", __func__,
                      i, ion_data[i].handle, i, ion_data[i].fd);
                response.ion_fd[i] = ion_data[i].fd;
#endif

            }
            break;

        case TLC_TUI_CMD_FREE_FB:
            ALOGI("%s: TLC_TUI_CMD_FREE_FB", __func__);
#ifdef QC_SOURCE_TREE
            // clear native window
            tlcAnwClear();
            ret = TLC_TUI_OK;
#else
            ret = TLC_TUI_ERROR;
#endif

#ifdef QC_SOURCE_TREE
            for(uint32_t i=0; i<numOfBuff; i++) {
                struct ion_handle_data handle_data = {0};

                handle_data.handle = ion_data[i].handle;
                int ret_ioctl = ioctl(ionDevFd, ION_IOC_FREE, &handle_data);
                if (ret_ioctl) {
                    ALOGI("%s: ION_IOC_FREE failed (%d)", __func__, ret_ioctl);
                }
                else {
                    ALOGI("%s: ION_IOC_FREE succeed", __func__);
                }
            }
#endif
            break;

        case TLC_TUI_CMD_QUEUE:
            ALOGI("tlcProcessCmd: TLC_TUI_CMD_QUEUE.");
#ifdef QC_SOURCE_TREE
            //enqueue display buffer
            ALOGI("call tlcAnwQueueBuffers");
            ret = tlcAnwQueueBuffer(tuiCmd.data[0]);
            if (ret != 0) {
                ALOGE("tlcAnwQueueBuffer returned an error (%d)", ret);
                ret = TLC_TUI_ERROR;
            }
#else
            ret = TLC_TUI_ERROR;
#endif
            break;

        case TLC_TUI_CMD_QUEUE_DEQUEUE:
#ifdef QC_SOURCE_TREE
            ALOGI("%s TLC_TUI_CMD_QUEUE_DEQUEUE.", __func__);
            ALOGI("%s queue buffer %d.", __func__, tuiCmd.data[0]);
            ret = tlcAnwQueueDequeueBuffer(tuiCmd.data[0]);
            if (ret != 0) {
                ALOGE("tlcAnwQueueBuffer returned an error (%d)", ret);
                ret = TLC_TUI_ERROR;
            } else {
                ret = TLC_TUI_OK;
            }
#endif
            break;

        case TLC_TUI_CMD_HIDE_SURFACE:
            ALOGI("%s: TLC_TUI_CMD_HIDE_SURFACE", __func__);
#ifdef QC_SOURCE_TREE
            // clear native window
            tlcAnwHide();
            ret = TLC_TUI_OK;
#else
            ret = TLC_TUI_ERROR;
#endif
            break;

        case TLC_TUI_CMD_GET_RESOLUTION:
            ALOGI("%s: TLC_TUI_CMD_GET_RESOLUTION:", __func__);
            tui_callback->getResolution(std::bind(&GetResolutionOut::callback, &get_res_out,
                                                  std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
            if (get_res_out.status != 0) {
                ALOGE("%s returned an error (%d)", __func__, ret);
                ret = TLC_TUI_ERROR;
            } else {
                response.screen_metrics[0] = get_res_out.width;
                response.screen_metrics[1] = get_res_out.height;
                ret = TLC_TUI_OK;
            }
            break;

        default:
            ALOGE("tlcProcessCmd: Unknown command %d", commandId);
            acknowledge = false;
            ret = TLC_TUI_ERR_UNKNOWN_CMD;
            break;
    }

    // Send command return code to the k-tlc
    response.id = commandId;
    response.return_code = ret;
    if (acknowledge) {
        ALOGD("tlcProcessCmd: sending ACK to kernel");
        if (-1 == ioctl(drvFd, TUI_IO_ACK, &response)) {
            ALOGE("TUI_IO_ACK ioctl failed with errno %s.", strerror(errno));
            return false;
        }
    }

    ALOGD("tlcProcessCmd: ret = %d", ret);
    return true;
}

uint32_t TuiManager::notifyReeEvent(uint32_t eventType) {

    if (-1 == ::ioctl(pimpl_->drvFd, TUI_IO_NOTIFY, eventType)) {
        ALOGE("TUI_IO_NOTIFY ioctl failed with errno %s.", strerror(errno));
        return -1;
    }

    return 0;
}

bool TuiManager::Impl::startTuiThread(void)
{
    std::lock_guard<std::mutex> lock(serverThreadMutex);
    if (serverThread.joinable()) {
        ALOGW("Server thread is already running.");
        return false;
    }

    serverThread = std::thread(std::bind(&Impl::serverThreadFunction, this));
    return true;
}

void TuiManager::registerTuiCallback(const ::android::sp<::vendor::trustonic::tee::V1_0::ITuiCallback>& callback) {
    pimpl_->tui_callback = callback;

    pimpl_->startTuiThread();
    //LOG_ALWAYS_FATAL_IF(!success, "Could not start Tui thread");
}

// Using new here because std::make_unique is not part of C++11
TuiManager::TuiManager(): pimpl_(new Impl) {
}

TuiManager::~TuiManager() {
}

}  // namespace implementation
}  // namespace V1_0
}  // namespace tee
}  // namespace trustonic
}  // namespace vendor
