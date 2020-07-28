/*
 * Copyright (c) 2013-2015 TRUSTONIC LIMITED
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
#include <jni.h>
#include <pthread.h>
#include <fcntl.h>
#include <sys/syscall.h>   /* For SYS_xxx definitions */
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/ioctl.h>

#include "tui_ioctl.h"

#include "tlcTui.h"
#include "tlcTuiJni.h"

#define LOG_TAG "TlcTui"
#include "log.h"

/* ------------------------------------------------------------- */
/* Globals */
bool testGetEvent = false;
/* ------------------------------------------------------------- */
/* Static */
static pthread_t threadId;
static int32_t drvFd = -1;

/* ------------------------------------------------------------- */
/* Static functions */
static void *mainThread(void *);

/* Functions */
/* ------------------------------------------------------------- */
/**
 * TODO.
 */
static void *mainThread(void *) {
//	int32_t fd;
//    struct stat st;
//    uint32_t size;
//    void *address;
    uint32_t cmdId;

    LOG_I("mainThread: TlcTui start!");

/* Android APP has no right to load a .ko. It must be loaded prior to starting the app.
    // Load the k-TLC
    fd = open("/data/app/tlckTuiPlay.ko", O_RDONLY);
    if (fd < 0) {
    	LOG_E("mainThread: Could not find k-tlc file!");
        exit(1);
    }
    else {
		fstat(fd, &st);
		size = st.st_size;
		address = mmap(0, size, PROT_READ, MAP_PRIVATE, fd, 0);
    	if (syscall(__NR_init_module, address, size, NULL)) {
    		int32_t errsv = errno;
			LOG_E("mainThread: Load k-tlc failed with errno %s!", strerror(errsv));
			exit(1);
        }
        close(fd);
    }
*/
    drvFd = open("/dev/" TUI_DEV_NAME, O_NONBLOCK);
    if (drvFd < 0) {
    	LOG_E("mainThread: open k-tlc device failed with errno %s.", strerror(errno));
        exit(1);
    }

    /* TlcTui main thread loop */
    for (;;) {
        /* Wait for a command from the k-TLC*/
        if (false == tlcWaitCmdFromDriver(&cmdId)) {
            break;
        }
        /* Something has been received, process it. */
        if (false == tlcProcessCmd(cmdId)) {
            break;
        }
    }

    // Close
    close(drvFd);

    // close_module() ?
    return NULL;
}
/* ------------------------------------------------------------- */
bool tlcLaunch(void) {

    bool ret = false;
    /* Create the TlcTui Main thread */
    if (pthread_create(&threadId, NULL, &mainThread, NULL) != 0) {
        LOG_E("tlcLaunch: pthread_create failed!");
        ret = false;
    } else {
        ret = true;
    }

    return ret;
}

/* ------------------------------------------------------------- */
bool tlcWaitCmdFromDriver(uint32_t *pCmdId) {
    uint32_t cmdId = 0;
    int ioctlRet = 0;

    /* Wait for ioctl to return from k-tlc with a command ID */
    /* Loop if ioctl has been interrupted. */
    do {
        ioctlRet = ioctl(drvFd, TUI_IO_WAITCMD, &cmdId);
    } while((EINTR == errno) && (-1 == ioctlRet));

    if (-1 == ioctlRet) {
        LOG_E("TUI_IO_WAITCMD ioctl failed with errno %s.", strerror(errno));
        return false;
    }
    *pCmdId = cmdId;
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
bool tlcProcessCmd(uint32_t commandId) {
    uint32_t ret = TLC_TUI_ERROR;
    struct tlc_tui_response_t response;

    bool acknowledge = true;

        switch (commandId) {
            case TLC_TUI_CMD_NONE:
                LOG_I("tlcProcessCmd: TLC_TUI_CMD_NONE.");
                acknowledge = false;
                break;

            case TLC_TUI_CMD_START_ACTIVITY:
                LOG_I("tlcProcessCmd: TLC_TUI_CMD_START_ACTIVITY.");
                ret = tlcStartTuiSession();
                LOG_I("tlcStartTuiSession returned %d", ret);
                if (ret == TUI_JNI_OK) {
                    ret = TLC_TUI_OK;
                } else {
                    ret = TLC_TUI_ERROR;
                }
                break;

            case TLC_TUI_CMD_STOP_ACTIVITY:
                LOG_I("tlcProcessCmd: TLC_TUI_CMD_STOP_ACTIVITY.");
                ret = tlcFinishTuiSession();
                LOG_I("tlcFinishTuiSession returned %d", ret);
                if (ret == TUI_JNI_OK) {
                    ret = TLC_TUI_OK;
                } else {
                    ret = TLC_TUI_ERROR;
                }
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

    LOG_I("tlcProcessCmd: ret = %d", ret);
    return true;
}

