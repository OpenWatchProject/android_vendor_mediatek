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
#define LOG_TAG "vendor.trustonic.tee@1.0-service[TUI]"

//#define LOG_NDEBUG 0
//#define TEESERVICE_DEBUG
//#define TEESERVICE_MEMLEAK_CHECK

#include <mutex>
#include <vector>

#include <utils/Log.h>
#include <hidlmemory/mapping.h>

#if defined (USER_ALLOCATE)
#include <linux/ion.h>
#include <cutils/native_handle.h>
#define ION_FILE_NODE "/dev/ion"
#endif
#include "TuiManager.h"
#include "driver.h"
#include "mc_types.h"
#include "gp_types.h" /* Needed for ALOGH use */

namespace vendor {
namespace trustonic {
namespace tee {
namespace tui {
namespace V1_0 {
namespace implementation {

struct TuiManager::Impl {

    std::mutex serverThreadMutex;
    std::thread serverThread;
    int tui_driver_fd = -1;

    ::android::sp<::vendor::trustonic::tee::tui::V1_0::ITuiCallback> tui_callback;
    ::android::sp<::vendor::trustonic::tee::V1_0::ITeeCallback> tee_callback;

    bool startTuiThread(void);
    void serverThreadFunction();
    bool tlcWaitCmdFromDriver(tlc_tui_command_t *tui_cmd);
    bool tlcProcessCmd(struct tlc_tui_command_t tui_cmd);

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

#ifdef USER_ALLOCATE
    /* Today ION allocation is only supported on QC */
    int ion_device_fd = -1;
    int ion_buffer_fd[MAX_BUFFER_NUMBER] = {0};
    android::hardware::hidl_handle *ion_handles[MAX_BUFFER_NUMBER];
    struct ion_fd_data ion_data[MAX_BUFFER_NUMBER];
    uint32_t num_of_buffers = 0;

    static uint32_t get_ion_handle(int ion_dev,
                                   int ion_buffer_fd,
                                   struct ion_fd_data *data) {
        uint32_t ret = TLC_TUI_ERROR;
        int ret_ioctl = -1;
        struct ion_fd_data tmp_ion_fd = {0, 0};

        ALOGH("%s: Open ION device", __func__);
        // get the handle corresponding to the ion fd
        tmp_ion_fd.fd = ion_buffer_fd;

        ALOGH("%s Calling  ION_IOC_IMPORT with ion_buffer_fd=%i",
                __func__, tmp_ion_fd.fd );
        ret_ioctl = ioctl(ion_dev, ION_IOC_IMPORT, &tmp_ion_fd);
        if (ret_ioctl != 0){
            ALOGE("ION_IOC_IMPORT ioctl failed with errno: %s.(%d)",
                    strerror(errno), ret_ioctl);
            return ret;
        }
        ret = TLC_TUI_OK;
        ALOGH("%s ION_IOC_IMPORT returned handle=%d fd=%i", __func__,
                tmp_ion_fd.handle, tmp_ion_fd.fd);

        data->fd = tmp_ion_fd.fd;
        data->handle = tmp_ion_fd.handle;

        return ret;
    }

    struct anwInitOut {
        uint32_t status;
        int32_t min_undequeued_buffers;
        int32_t pid;

        void callback(uint32_t status, int32_t min_undequeued_buffers,
                int32_t pid) {
            this->status = status;
            this->min_undequeued_buffers = min_undequeued_buffers;
            this->pid = pid;
        }
    };

    struct anwDequeueBuffersOut {
        uint32_t status;
        android::hardware::hidl_handle  handle;
        uint32_t width;
        uint32_t height;
        uint32_t stride;

        void callback(uint32_t status, android::hardware::hidl_handle handle,
                uint32_t width, uint32_t height, uint32_t stride) {
            this->status = status;
            this->handle = handle;
            this->width = width;
            this->height = height;
            this->stride = stride;
        }
    };
#endif
};

void TuiManager::Impl::serverThreadFunction()
{
    struct tlc_tui_command_t tui_cmd;
    if (tui_driver_fd == -1) {
        ALOGE("%s ", __func__);
        tui_driver_fd = ::open("/dev/" TUI_DEV_NAME, O_NONBLOCK);
        if (tui_driver_fd < 0) {
            ALOGE("%s: open k-tlc device failed with errno %s.",
                    __func__, strerror(errno));
            return;
        }
    }

    /* TlcTui main thread loop */
    for (;;) {
        /* Wait for a command from the k-TLC*/
        if (!tlcWaitCmdFromDriver(&tui_cmd)) {
            break;
        }
        /* Something has been received, process it. */
        if (!tlcProcessCmd(tui_cmd)) {
            break;
        }
    }
}

bool TuiManager::Impl::tlcWaitCmdFromDriver(tlc_tui_command_t *tui_cmd)
{
    uint32_t cmdId = 0;
    int ioctl_ret = 0;

    tui_cmd->id = 0;
    tui_cmd->data[0] = 0;
    tui_cmd->data[1] = 0;

    /* Wait for ioctl to return from k-tlc with a command ID */
    /* Loop if ioctl has been interrupted. */
    do {
        ioctl_ret = ::ioctl(tui_driver_fd, TUI_IO_WAITCMD, tui_cmd);
    } while((EINTR == errno) && (-1 == ioctl_ret));

    if (-1 == ioctl_ret) {
        ALOGE("TUI_IO_WAITCMD ioctl failed with errno %s.", strerror(errno));
        return false;
    } else {
        cmdId = tui_cmd->id;
        ALOGH("TUI_IO_WAITCMD ioctl returnd with cmdId %d.", cmdId);
    }

    return true;
}

bool TuiManager::Impl::tlcProcessCmd(struct tlc_tui_command_t tui_cmd)
{
    uint32_t ret = TLC_TUI_ERROR;
    struct tlc_tui_response_t response;
    uint32_t command_id = tui_cmd.id;
    bool acknowledge = true;
    GetResolutionOut get_res_out;
#ifdef USER_ALLOCATE
    unsigned int num_of_buffers = 0;
    int min_undequeued_buff = -1;
    uint32_t buffer_size = 0;
    uint32_t width=0, height=0, stride=0;
    anwInitOut anw_init;
    anwDequeueBuffersOut anw_dequeue_buffers;
#endif

    memset(&response, 0, sizeof(tlc_tui_response_t));

    switch (command_id) {
        case TLC_TUI_CMD_NONE:
            ALOGH("%s: TLC_TUI_CMD_NONE", __func__);
            acknowledge = false;
            break;

        case TLC_TUI_CMD_START_ACTIVITY:
            ALOGH("%s: TLC_TUI_CMD_START_ACTIVITY", __func__);
            if (!tui_callback) {
                ALOGE("No callback registered.");
                break;
            }
            ret = tui_callback->startSession();
            if (ret) {
                acknowledge = false;
                ALOGE("%s: startSession failed(%d)", __func__, ret);
            }

#ifdef TEST_SECURITY
            if (pthread_create(&testThreadId, NULL, &testThread, NULL) != 0) {
                ALOGE("ERROR %s:%i pthread_create failed!", __func__, __LINE__);
            }
#endif
            break;

        case TLC_TUI_CMD_STOP_ACTIVITY:
            ALOGD("%s: TLC_TUI_CMD_STOP_ACTIVITY", __func__);
            ret = tui_callback->stopSession();
            if (ret) {
                ALOGE("%s: stopSession failed(%d)", __func__, ret);
            }
            break;

        case TLC_TUI_CMD_ALLOC_FB:
            ALOGH("%s: TLC_TUI_CMD_ALLOC_FB", __func__);
            ret = TLC_TUI_ERROR;
#ifdef USER_ALLOCATE
            num_of_buffers = tui_cmd.data[0];
            buffer_size = tui_cmd.data[1];
            ALOGH("%s: num_of_buffers(%ue - buffer_size(%u)", __func__,
                    num_of_buffers,
                    buffer_size);

            /* First, try to open ION device driver.
             * If it fails, no need to try anything else.
             */
            if (ion_device_fd < 0) {
                ion_device_fd = open(ION_FILE_NODE, O_RDONLY);
            }

            if (ion_device_fd < 0) {
                ALOGE("%s: open %s failed (%s)",
                        __func__,
                        ION_FILE_NODE,
                        strerror(errno));
                break;
            }

            /* First we retrieve the current resolution so we can allocate
             * correctly sized buffers
             */
            tui_callback->getResolution(std::bind(&GetResolutionOut::callback,
                                                  &get_res_out,
                                                  std::placeholders::_1,
                                                  std::placeholders::_2,
                                                  std::placeholders::_3));

            if (get_res_out.status) {
                ALOGE("%s: getResolution failed (%d)",
                        __func__,
                        get_res_out.status);
                break;
            }

            /* Assuming that the FBs management scheme will always keep at least
             * one buffer un-dequeued:
             * max_dequeued_buffer_number is num_of_buffers-1 */
            tee_callback->anwInit(num_of_buffers-1,
                                  get_res_out.width,
                                  get_res_out.height,
                                  std::bind(&anwInitOut::callback,
                                            &anw_init,
                                            std::placeholders::_1,
                                            std::placeholders::_2,
                                            std::placeholders::_3));

            /* update local variables with callback results*/
            min_undequeued_buff = anw_init.min_undequeued_buffers;

            if (anw_init.status) {
                ALOGE("%s: anwInit failed (%d)",__func__, anw_init.status);
                break;
            }

            for(uint32_t i=0; i<num_of_buffers; i++) {
                /* TODO: We can probably update the HAL to allow variable-sized
                 * arrays to be returned. That would allow us to move below call
                 * outside the loop.
                 * Today, we dequeue buffers one at a time*/
                ALOGH("%s: Dequeueing buffer[%d]", __func__, i);
                tee_callback->anwDequeueBuffers(i,
                        std::bind(&anwDequeueBuffersOut::callback,
                            &anw_dequeue_buffers,
                            std::placeholders::_1,
                            std::placeholders::_2,
                            std::placeholders::_3,
                            std::placeholders::_4,
                            std::placeholders::_5));
                if (anw_dequeue_buffers.status) {
                    ALOGE("%s: anwDequeueBuffers failed(%d)",
                            __func__, anw_dequeue_buffers.status);
                    break;
                }
                /* update local variables with result */
                width = anw_dequeue_buffers.width;
                height = anw_dequeue_buffers.height;
                stride = anw_dequeue_buffers.stride;
                // Let's hope that magic happens here
                ion_buffer_fd[i] = anw_dequeue_buffers.handle.getNativeHandle()->data[0];
                auto handle = new android::hardware::hidl_handle(anw_dequeue_buffers.handle);

                ion_handles[i] = handle;
                ion_buffer_fd[i] = handle->getNativeHandle()->data[0];

                ALOGH("%s: ion_buffer_fd(%d) width(%u) height(%u) stride(%u)",
                        __func__, ion_buffer_fd[i], width, height, stride);
                response.screen_metrics[0] = width;
                response.screen_metrics[1] = height;
                response.screen_metrics[2] = stride;

                ret = get_ion_handle(ion_device_fd, ion_buffer_fd[i], &ion_data[i]);
                if (TLC_TUI_OK != ret) {
                    ALOGE("could not get ion phys addr for handle %d", i);
                    ALOGE("Delegating handle to k-tlc");
                    //response.pid = anw_init.pid;
                    response.ion_fd[i] = ion_buffer_fd[i];
                } else {
                    //response.pid = 0;
                    response.ion_fd[i] = ion_data[i].fd;
                    ALOGH("%s ion_data[%d] - handle(%d) fd(%d)", __func__,
                        i, ion_data[i].handle, ion_data[i].fd);
                }
            }
#endif /* USER_ALLOCATE */
            break;

        case TLC_TUI_CMD_FREE_FB:
            ALOGH("%s: TLC_TUI_CMD_FREE_FB", __func__);
            ret = TLC_TUI_ERROR;
#ifdef USER_ALLOCATE
            // clear native window
            tee_callback->anwClear();
            for(uint32_t i=0; i<num_of_buffers; i++) {
                struct ion_handle_data handle_data = {0};
                handle_data.handle = ion_data[i].handle;
                int ret_ioctl = ioctl(ion_device_fd,
                                      ION_IOC_FREE,
                                      &handle_data);
                if (ret_ioctl) {
                    ALOGH("%s: ION_IOC_FREE buffer [%d] failed(%d) - errno:%s",
                        __func__, i, ret_ioctl, strerror(errno));

                }
                //free the hidl_handle
                native_handle_close(ion_handles[i]->getNativeHandle());
                ion_handles[i] = NULL;
            }
            ret = TLC_TUI_OK;
#endif /* USER_ALLOCATE */
            break;

        case TLC_TUI_CMD_QUEUE:
            ALOGH("%s: TLC_TUI_CMD_QUEUE", __func__);
            ret = TLC_TUI_ERROR;
#ifdef USER_ALLOCATE
            ret = tee_callback->anwQueueBuffer(tui_cmd.data[0]);
            if (ret != 0) {
                ALOGE("tlcAnwQueueBuffer returned an error (%d)", ret);
                ret = TLC_TUI_ERROR;
            }
#endif /* USER_ALLOCATE */
            break;

        case TLC_TUI_CMD_QUEUE_DEQUEUE:
            ALOGH("%s TLC_TUI_CMD_QUEUE_DEQUEUE", __func__);
#ifdef USER_ALLOCATE
            ALOGH("%s queue buffer %d.", __func__, tui_cmd.data[0]);
            ret = tee_callback->anwQueueDequeueBuffer(tui_cmd.data[0]);
            if (ret) {
                ALOGE("%s: anwQueueDequeueBuffer failed (%d)", __func__, ret);
                ret = TLC_TUI_ERROR;
            }
#endif /* USER_ALLOCATE */
            break;

        case TLC_TUI_CMD_HIDE_SURFACE:
            ALOGH("%s: TLC_TUI_CMD_HIDE_SURFACE", __func__);
            ret = TLC_TUI_ERROR;
#ifdef USER_ALLOCATE
            // clear native window
            tee_callback->anwHide();
            ret = TLC_TUI_OK;
#endif /* USER_ALLOCATE */
            break;

        case TLC_TUI_CMD_GET_RESOLUTION:
            ALOGH("%s: TLC_TUI_CMD_GET_RESOLUTION:", __func__);
            tui_callback->getResolution(std::bind(&GetResolutionOut::callback,
                                      &get_res_out,
                                                  std::placeholders::_1,
                                                  std::placeholders::_2,
                                                  std::placeholders::_3));
            if (get_res_out.status != 0) {
                ALOGE("%s failed (%d)", __func__, ret);
                ret = TLC_TUI_ERROR;
            } else {
                response.screen_metrics[0] = get_res_out.width;
                response.screen_metrics[1] = get_res_out.height;
                ret = TLC_TUI_OK;
            }
            break;

        default:
            ALOGE("%s: Unknown command %d", __func__, command_id);
            acknowledge = false;
            ret = TLC_TUI_ERR_UNKNOWN_CMD;
            break;
    }

    // Send command return code to the k-tlc
    response.id = command_id;
    response.return_code = ret;
    if (acknowledge) {
        ALOGD("%s: sending TUI_IO_ACK to kernel", __func__);
        if (-1 == ioctl(tui_driver_fd, TUI_IO_ACK, &response)) {
            ALOGE("%s: ioctl TUI_IO_ACK failed (%s)",
                  __func__, strerror(errno));
            return false;
        }
    }
    ALOGH("%s: ret = %d", __func__, ret);
    return true;
}

uint32_t TuiManager::notifyReeEvent(uint32_t eventType) {

    if (-1 == ::ioctl(pimpl_->tui_driver_fd, TUI_IO_NOTIFY, eventType)) {
        ALOGE("TUI_IO_NOTIFY ioctl failed with errno %s.", strerror(errno));
        return -1;
    }

    return 0;
}

/* notifyScreenSizeUpdate is for specific platforms
 * that rely on onConfigurationChanged from the TeeService to set resolution
 * it has no effect on Trustonic reference implementaton. 
 */
uint32_t TuiManager::notifyScreenSizeUpdate(uint32_t width, uint32_t height) {
    ALOGE("TuiManager:%s called tui_driver_fd:(%x)", __func__, pimpl_->tui_driver_fd);
    struct tlc_tui_resolution_t res;
    res.width = width;
    res.height = height;
    if (!pimpl_->tui_driver_fd) {
        ALOGE("%s ", __func__);
        pimpl_->tui_driver_fd = ::open("/dev/" TUI_DEV_NAME, O_NONBLOCK);
        if (pimpl_->tui_driver_fd < 0) {
            ALOGE("%s: open k-tlc device failed with errno %s.",
                    __func__, strerror(errno));
            return -1;
        }
    }

    if (-1 == ::ioctl(pimpl_->tui_driver_fd, TUI_IO_SET_RESOLUTION, &res)) {
        ALOGE("TLC_TUI_CMD_SET_RESOLUTION ioctl failed with errno %s.", strerror(errno));
        return -1;
    }
    return 0;
}

bool TuiManager::Impl::startTuiThread(void)
{
    ALOGE("%s called", __func__);
    std::lock_guard<std::mutex> lock(serverThreadMutex);
    if (serverThread.joinable()) {
        ALOGW("Server thread is already running.");
        return false;
    }

    serverThread = std::thread(std::bind(&Impl::serverThreadFunction, this));
    return true;
}

void TuiManager::registerTuiCallback(const ::android::sp<::vendor::trustonic::tee::tui::V1_0::ITuiCallback>& callback) {
    pimpl_->tui_callback = callback;

    /*
     * Try to launch the Tui thread.
     *  The thread may already exist, if a previous instance from the framework
     *  already launched it.  Therefore, we ignore the return code.
     */
    (void) pimpl_->startTuiThread();
}

void TuiManager::registerTeeCallback(const ::android::sp<::vendor::trustonic::tee::V1_0::ITeeCallback>& callback) {
    pimpl_->tee_callback = callback;
    if (pimpl_->tee_callback) {
        ALOGH("%s: Successfully registered tee_callback", __func__);
    } else {
        ALOGE("%s: tee_callback is NULL", __func__);
    }
}
// Using new here because std::make_unique is not part of C++11
TuiManager::TuiManager(): pimpl_(new Impl) {
}

TuiManager::~TuiManager() {
}

}  // namespace implementation
}  // namespace V1_0
}  // namespace tui 
}  // namespace tee
}  // namespace trustonic
}  // namespace vendor
