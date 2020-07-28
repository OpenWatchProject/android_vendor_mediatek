/*
 * Copyright (c) 2013-2016 TRUSTONIC LIMITED
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

#include <stdlib.h>
#include <stdint.h>
#include <jni.h>
#include <pthread.h>
#include <fcntl.h>
#include <sys/syscall.h>   /* For SYS_xxx definitions */
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/ioctl.h>

#if defined(QC_SOURCE_TREE)
#include <linux/ion.h>
#endif

#include "tui_ioctl.h"

#include "tlcTui.h"
#include "tlcTuiJni.h"

#ifdef QC_SOURCE_TREE
#include "tlcNativeWindow.h"
#endif

#define LOG_TAG "TlcTui"
#include "log.h"

/* ------------------------------------------------------------- */
/* Globals */
bool testGetEvent = false;
/* ------------------------------------------------------------- */
/* Static */
static pthread_t threadId = 0;
static pthread_mutex_t tlcThreadMutex = PTHREAD_MUTEX_INITIALIZER;
static int32_t drvFd = -1;
static int ionDevFd = -1;

#ifdef QC_SOURCE_TREE
struct ion_fd_data ion_data[MAX_BUFFER_NUMBER];
#endif
unsigned int numOfBuff = 0;

/* ------------------------------------------------------------- */
/* Static functions */
static void *mainThread(void *);

#ifdef TEST_SECURITY
#include <sched.h>
static void *testThread(void *);
static pthread_t testThreadId = 0;
#endif

/* Functions */
/* ------------------------------------------------------------- */

#ifdef QC_SOURCE_TREE
static uint32_t get_ion_handle(int ion_dev,
                               int ion_buffer_fd,
                               struct ion_fd_data *data)
{
    uint32_t ret = TLC_TUI_ERROR;
    int ret_ioctl = -1;
    struct ion_fd_data tmp_ion_fd = {0, 0};

    LOG_D("%s: Open ION device", __func__);
    // get the handle corresponding to the ion fd
    tmp_ion_fd.fd = ion_buffer_fd;

    LOG_D("%s Calling  ION_IOC_IMPORT with ion_buffer_fd=%i", __func__, tmp_ion_fd.fd );
    ret_ioctl = ioctl(ion_dev, ION_IOC_IMPORT, &tmp_ion_fd);
    if (ret_ioctl != 0){
        LOG_E("ION_IOC_IMPORT ioctl failed with errno: %s.(%d)",
                strerror(errno), ret_ioctl);
        return ret;
    }
    ret = TLC_TUI_OK;
    LOG_D("%s ION_IOC_IMPORT returned handle=%d fd=%i", __func__,
            tmp_ion_fd.handle, tmp_ion_fd.fd);

    data->fd = tmp_ion_fd.fd;
    data->handle = tmp_ion_fd.handle;

    return ret;
}
#endif

/**
 * TODO.
 */
static void *mainThread(void *) {
    struct tlc_tui_command_t tuiCmd;

    LOG_D("mainThread: TlcTui start!");

    drvFd = open("/dev/" TUI_DEV_NAME, O_NONBLOCK);
    if (drvFd < 0) {
        LOG_E("mainThread: open k-tlc device failed with errno %s.",
                strerror(errno));
        exit(1);
    }

    /* TlcTui main thread loop */
    for (;;) {
        /* Wait for a command from the k-TLC*/
        if (false == tlcWaitCmdFromDriver(&tuiCmd)) {
            break;
        }
        /* Something has been received, process it. */
        if (false == tlcProcessCmd(tuiCmd)) {
            break;
        }
    }

    // Close
    LOG_E("mainThread: Close drvFd.");
    close(drvFd);

    // close_module() ?
    return NULL;
}

/* ------------------------------------------------------------- */
#ifdef TEST_SECURITY
static void *testThread(void *)
{

    cpu_set_t  mask;

    LOG_I("%s: thread is running!", __func__);

    CPU_ZERO(&mask); // Clear mask
    CPU_SET(0, &mask); // Mask specifies CPU #1

    // Force current thread to be run on CPU specified in mask
//    pthread_attr_setaffinity_np(0, sizeof(cpu_set_t), &mask);
    pthread_t current_thread = pthread_self();
    sched_setaffinity(current_thread, sizeof(cpu_set_t), &mask);

    sleep(10);

    uint32_t ret = TLC_TUI_ERROR;
    LOG_I("%s: Try to unsecure the display", __func__);
    ret = tlcFinishTuiSession();

    if (ret == TUI_JNI_OK) {
        ret = TLC_TUI_OK;
    } else {
        ret = TLC_TUI_ERROR;
        LOG_E("%s: tlcFinishTuiSession() returned %d", __func__, ret);
    }
    LOG_I("%s: Clear the native window!", __func__);
    tlcAnwClear();

    /* Test thread loop */
    for (;;) {
        // TODO read FB and test if white inside
        //readFB();
        sleep(1);
    }

    LOG_E("%s: thread is going to exit!", __func__);
    return (void*)1;
}
#endif // TEST_SECURITY

/* ------------------------------------------------------------- */
bool tlcLaunch(void) {

    bool ret = false;
    pthread_mutex_lock(&tlcThreadMutex);
    /* Create the TlcTui Main thread */
    if (threadId == 0 && pthread_create(&threadId, NULL, &mainThread, NULL) != 0) {
        LOG_E("tlcLaunch: pthread_create failed!");
        ret = false;
    } else {
        ret = true;
    }
    pthread_mutex_unlock(&tlcThreadMutex);

    return ret;
}

/* ------------------------------------------------------------- */
bool tlcWaitCmdFromDriver(tlc_tui_command_t *tuiCmd) {
    uint32_t cmdId = 0;
    int ioctlRet = 0;

    tuiCmd->id = 0;
    tuiCmd->data[0] = 0;
    tuiCmd->data[1] = 0;

    /* Wait for ioctl to return from k-tlc with a command ID */
    /* Loop if ioctl has been interrupted. */
    do {
        ioctlRet = ioctl(drvFd, TUI_IO_WAITCMD, tuiCmd);
    } while((EINTR == errno) && (-1 == ioctlRet));

    if (-1 == ioctlRet) {
        LOG_E("TUI_IO_WAITCMD ioctl failed with errno %s.", strerror(errno));
        return false;
    } else {
        cmdId = tuiCmd->id;
        LOG_I("TUI_IO_WAITCMD ioctl returnd with cmdId %d.", cmdId);
    }

    return true;
}

/* ------------------------------------------------------------- */
bool tlcNotifyEvent(uint32_t eventType) {

    if (-1 == ioctl(drvFd, TUI_IO_NOTIFY, eventType)) {
        LOG_E("TUI_IO_NOTIFY ioctl failed with errno %s.", strerror(errno));
        return false;
    }

    return true;
}

/* ------------------------------------------------------------- */
bool tlcProcessCmd(struct tlc_tui_command_t tuiCmd) {
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

    memset(&response, 0, sizeof(tlc_tui_response_t));

    switch (commandId) {
        case TLC_TUI_CMD_NONE:
            LOG_I("tlcProcessCmd: TLC_TUI_CMD_NONE.");
            acknowledge = false;
            break;

        case TLC_TUI_CMD_START_ACTIVITY:
            LOG_D("tlcProcessCmd: TLC_TUI_CMD_START_ACTIVITY.");
            ret = tlcStartTuiSession();
            LOG_D("tlcStartTuiSession returned %d", ret);
            if (ret == TUI_JNI_OK) {
                ret = TLC_TUI_OK;
            } else {
                ret = TLC_TUI_ERROR;
                acknowledge = false;
            }

#ifdef TEST_SECURITY
            if (pthread_create(&testThreadId, NULL, &testThread, NULL) != 0) {
                LOG_E("ERROR %s:%i pthread_create failed!", __func__, __LINE__);
            }
#endif
            LOG_I("TLC_TUI_CMD_START_ACTIVITY returned %d", ret);
            break;

        case TLC_TUI_CMD_STOP_ACTIVITY:
            LOG_D("tlcProcessCmd: TLC_TUI_CMD_STOP_ACTIVITY.");
            ret = tlcFinishTuiSession();
            LOG_D("tlcFinishTuiSession returned %d", ret);
            if (ret == TUI_JNI_OK) {
                ret = TLC_TUI_OK;
            } else {
                ret = TLC_TUI_ERROR;
            }
            break;

        case TLC_TUI_CMD_ALLOC_FB:
            LOG_I("tlcProcessCmd: TLC_TUI_CMD_ALLOC_FB.");

            numOfBuff = tuiCmd.data[0];
            sizeOfBuff = tuiCmd.data[1];
            LOG_I("%s: numOfBuff = %u", __func__, numOfBuff);
            LOG_I("%s: sizeOfBuff = %u", __func__, sizeOfBuff);
            LOG_I("%s: Init Native Window", __func__);
            // Init native window

            ret = tlcGetResolution(&width, &height);
            if (ret != 0) {
                LOG_E("%s returned an error (%d)", "tlcGetResolution", ret);
                ret = TLC_TUI_ERROR;
            }

#ifdef QC_SOURCE_TREE
            /* Assuming that the FBs management scheme will always keep at least
             * one buffer un-dequeued:
             * max_dequeued_buffer_number is numOfBuff-1 */
            ret = tlcAnwInit(numOfBuff-1, &min_undequeued_buff, width, height);
            if (ret != TLC_TUI_OK) {
                LOG_E("tlcAnwInit returned %d", ret);
                break;
            }
#else
            ret = -1;
#endif

            LOG_I("%s: Dequeue buffers", __func__);
#ifdef QC_SOURCE_TREE
            ret = tlcAnwDequeueBuffers(ion_buffer_fd,
                                       numOfBuff-1 + min_undequeued_buff,
                                       &width, &height, &stride);
#else
            ret = TLC_TUI_ERROR;
#endif
            LOG_I("%s: width = %u height = %u stride = %u", __func__, width,
                  height, stride);
            response.screen_metrics[0] = width;
            response.screen_metrics[1] = height;
            response.screen_metrics[2] = stride;
            if (ret != TLC_TUI_OK) {
                LOG_E("tlcAnwDequeueBuffers returned %d", ret);
                break;
            }

            // Open ion device driver
            if (ionDevFd < 0) {
                ionDevFd = open("/dev/ion", O_RDONLY);
            }

            if (ionDevFd < 0) {
                LOG_E("mainThread: open ION device failed with errno %s.",
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
                    LOG_E("could not get ion phys addr for handle %d", i);
                    break;
                }
#ifdef QC_SOURCE_TREE
                LOG_I("%s ion_data[%d].handle=%d ion_data[%d].fd=%d", __func__,
                    i, ion_data[i].handle, i, ion_data[i].fd);
                response.ion_fd[i] = ion_data[i].fd;
#endif

            }
            break;

        case TLC_TUI_CMD_FREE_FB:
            LOG_I("%s: TLC_TUI_CMD_FREE_FB", __func__);
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
                    LOG_I("%s: ION_IOC_FREE failed (%d)", __func__, ret_ioctl);
                }
                else {
                    LOG_I("%s: ION_IOC_FREE succeed", __func__);
                }
            }
#endif
            break;

        case TLC_TUI_CMD_QUEUE:
            LOG_I("tlcProcessCmd: TLC_TUI_CMD_QUEUE.");
#ifdef QC_SOURCE_TREE
            //enqueue display buffer
            LOG_I("call tlcAnwQueueBuffers");
            ret = tlcAnwQueueBuffer(tuiCmd.data[0]);
            if (ret != 0) {
                LOG_E("tlcAnwQueueBuffer returned an error (%d)", ret);
                ret = TLC_TUI_ERROR;
            }
#else
            ret = TLC_TUI_ERROR;
#endif
            break;

        case TLC_TUI_CMD_QUEUE_DEQUEUE:
#ifdef QC_SOURCE_TREE
            LOG_I("%s TLC_TUI_CMD_QUEUE_DEQUEUE.", __func__);
            LOG_I("%s queue buffer %d.", __func__, tuiCmd.data[0]);
            ret = tlcAnwQueueDequeueBuffer(tuiCmd.data[0]);
            if (ret != 0) {
                LOG_E("tlcAnwQueueBuffer returned an error (%d)", ret);
                ret = TLC_TUI_ERROR;
            } else {
                ret = TLC_TUI_OK;
            }
#endif
            break;

        case TLC_TUI_CMD_HIDE_SURFACE:
            LOG_I("%s: TLC_TUI_CMD_HIDE_SURFACE", __func__);
#ifdef QC_SOURCE_TREE
            // clear native window
            tlcAnwHide();
            ret = TLC_TUI_OK;
#else
            ret = TLC_TUI_ERROR;
#endif
            break;

        case TLC_TUI_CMD_GET_RESOLUTION:
            LOG_I("%s: TLC_TUI_CMD_GET_RESOLUTION:", __func__);
            ret = tlcGetResolution(&width, &height);
            response.screen_metrics[0] = width;
            response.screen_metrics[1] = height;

            if (ret != 0) {
                LOG_E("%s returned an error (%d)", __func__, ret);
                ret = TLC_TUI_ERROR;
            } else {
                ret = TLC_TUI_OK;
            }
            ret = TLC_TUI_OK;
            break;

        default:
            LOG_E("tlcProcessCmd: Unknown command %d", commandId);
            acknowledge = false;
            ret = TLC_TUI_ERR_UNKNOWN_CMD;
            break;
    }

    // Send command return code to the k-tlc
    response.id = commandId;
    response.return_code = ret;
    if (acknowledge) {
        if (-1 == ioctl(drvFd, TUI_IO_ACK, &response)) {
            LOG_E("TUI_IO_ACK ioctl failed with errno %s.", strerror(errno));
            return false;
        }
    }

    LOG_D("tlcProcessCmd: ret = %d", ret);
    return true;
}

