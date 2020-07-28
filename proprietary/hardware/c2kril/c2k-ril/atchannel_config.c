/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

/* //device/libs/telephony/ril.cpp
**
** Copyright 2006, The Android Open Source Project
**
** Licensed under the Apache License, Version 2.0 (the "License");
** you may not use this file except in compliance with the License.
** You may obtain a copy of the License at
**
**     http://www.apache.org/licenses/LICENSE-2.0
**
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS,
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
** See the License for the specific language governing permissions and
** limitations under the License.
*/
#ifdef C2K_RIL_LEGACY
#include <c2kutils.h>
#endif
#include <assert.h>
#include <string.h>
#include <errno.h>
#include <utils/Log.h>
#include <termios.h>
#include <private/android_filesystem_config.h>
#include <pthread.h>
#include <cutils/sockets.h>
#include <linux/serial.h>
#include "hardware/ccci_intf.h"
#include "atchannel_config.h"
#include <sys/types.h>
#include <sys/stat.h>

#define ARRAY_LENGTH(array) (sizeof(array)/sizeof(array[0]))

#define BAUDRATE B460800

#define LOG_TAG "C2K_ATConfig"

#define SYSCHECK(c) do{if((c)<0){ \
                        RLOGD("system-error: '%s' (code: %d)", strerror(errno), errno);\
                        }\
                    }while(0)

#define PHYSICAL_MAX_CHANNEL_NUM 8

static int INITIAL_VALUE = -1;

typedef enum {
    STATE_INITIALIZING,
    STATE_MUXING,
    STATE_ERROR,
    STATE_CLOSING,
} ChannelState;

typedef enum {
    NO_ERROR,
    ERROR_DEVICE_DISCONNECT,
    ERROR_IO,
} ChannelError;

#ifdef MTK_ECCCI_C2K
int s_mux_fd_property_list[] = {
    USR_C2K_AT,
    USR_C2K_AT2,
    USR_C2K_AT3,
    USR_C2K_AT4,
    USR_C2K_AT5,
    USR_C2K_AT6,
    USR_C2K_AT7,
    USR_C2K_AT8
};
#elif C2K_RIL_LEGACY
int s_mux_fd_property_list[] = {
    VIATEL_CHANNEL_AT,
    VIATEL_CHANNEL_AT2,
    VIATEL_CHANNEL_AT3,
    VIATEL_CHANNEL_AT4,
    VIATEL_CHANNEL_AT5,
    VIATEL_CHANNEL_AT6,
    VIATEL_CHANNEL_AT7,
    VIATEL_CHANNEL_AT8
};
#endif /* MTK_ECCCI_C2K */

ChannelState channelState;
int pseudoChannelNumber;
int physicalChannelNumber = ARRAY_LENGTH(s_mux_fd_property_list);
PhysicalChannel* physicalChannels;
PseudoChannel* pseudoChannels;

pthread_t pseudoChannelRead;
pthread_t physicalChannelRead;

ChannelError physicalChannelError;
ChannelError pseudoChannelError;

#define CHANNEL_NOT_READY 0
#define CHANNEL_READY 1

int isPhysicalChannelReady = CHANNEL_NOT_READY;
int isPseudoChannelReady = CHANNEL_NOT_READY;

//function prototype
void openDevicePath(int index, char* devicePath);
void initPhysicalChannel(int index);
int initPseudoChannel(int index);
void initPhysicalChannelValue();
void deInitPhysicalChannelValue();
void initPseudoChannelValue();
void deInitPseudoChannelValue();
void constructChannelMapping();
void handleStateInitializing();
int createThread(pthread_t * thread_id,void * thread_function, void * thread_function_arg);
int writeToChannel(int fd, const char* buffer);
void physicalChannelLoop();
void pseudoChannelLoop();
void handleStateMuxing();
void resetChannelError();
void handleStateClosing();
void resetErrno();
void transitToState(ChannelState state);
int triggerCCCIIoctl(int ioctlId, int* param);
int updatePseudoChannelMapping(int index);
int updatePhysicalChannelMapping(int index);

int triggerCCCIIoctl(int ioctlId, int* param) {
    char nodeName[32];
    int fd = -1;
    int ret = -1;
    snprintf(nodeName, 32, "%s", ccci_get_node_name(USR_RILD_IOCTL, MD_SYS3));
    fd = open(nodeName, O_RDWR);
    if(fd >= 0) {
        ret = ioctl(fd, ioctlId, param);
        RLOGI("CCCI IOCTL, id: %d, ret: %d, errno: %d, param: %d", ioctlId, ret, errno, *param);
        close(fd);
    } else {
        RLOGE("open ccci node fail, errno: %d", errno);
    }
    return ret;
}

void getPseudoChannelPathes(int index, char devicePath[], int devicePathLen) {
    RLOGD("devicePathLen = %d, deviceLink = %s", devicePathLen, pseudoChannels[index].clientLink);
    memset(devicePath, 0, devicePathLen);
    strlcat(devicePath, pseudoChannels[index].clientLink, devicePathLen * sizeof(char));
    RLOGD("devicePath[%d] = %s", index, devicePath);
}

pthread_mutex_t* getPseudoChannelMutex(int index) {
    return pseudoChannels[index].p_channelMutex;
}

void openDevicePath(int index, char* devicePath) {
    int err_cnt = 0;
    while (physicalChannels[index].fd < 0) {
        do {
            physicalChannels[index].fd = open(devicePath, O_RDWR);
        } while (physicalChannels[index].fd < 0 && errno == EINTR);

        RLOGI("Channel %d path=%s, fd=%d", index, devicePath, physicalChannels[index].fd);
        if (physicalChannels[index].fd < 0) {
            RLOGD("opening AT interface. retrying...");
            RLOGE("could not connect to %s: %s", devicePath,
                strerror(errno));
            sleep(10);
            /* never returns */
        } else {
            err_cnt = 0;
#ifdef MTK_ECCCI_C2K
            /* disable echo on serial ports */
            struct termios ios;
            tcgetattr(physicalChannels[index].fd, &ios);
            ios.c_lflag = 0; /* disable ECHO, ICANON, etc... */
            ios.c_iflag = 0;
            tcsetattr(physicalChannels[index].fd, TCSANOW, &ios);
#else /* MTK_ECCCI_C2K */
            /* disable echo on serial ports */
            struct termios ios;
            if (tcgetattr(physicalChannels[index].fd, &ios) < 0) {
                RLOGD("setMuxPorts : ERROR GET DEVICE TTY ATTR (fd = %d)", physicalChannels[index].fd);
            }

            ios.c_cflag &= ~(CSIZE | CSTOPB | PARENB | PARODD);
            ios.c_cflag |= CREAD | CLOCAL | CS8;
            ios.c_cflag &= ~(CRTSCTS);
            ios.c_lflag &= ~(ICANON | ECHO | ECHOE | ECHOK | ECHONL
                            | ISIG);
            ios.c_iflag &= ~(INPCK | IGNPAR | PARMRK | ISTRIP | IXANY
                            | ICRNL);
            ios.c_iflag &= ~(IXON | IXOFF);
            ios.c_oflag &= ~(OPOST | OCRNL);
            ios.c_cc[VMIN] = 1;
            ios.c_cc[VTIME] = 0;
            cfsetispeed(&ios, B115200);
            cfsetospeed(&ios, B115200);
#ifndef USE_STATUSD
            tcflush(physicalChannels[index].fd, TCIFLUSH);
#endif
            if (tcsetattr(physicalChannels[index].fd, TCSANOW, &ios) < 0) {
                // affect at now
                RLOGE("setMuxPorts: ERROR SET ATTR (fd = %d)", physicalChannels[index].fd);
            }

            struct serial_struct ss;
            if (ioctl(physicalChannels[index].fd, TIOCGSERIAL, &ss) < 0) {
            RLOGE("%s: ERROR GET SERIAL STRUCT(fd = %d)",
                __FUNCTION__, physicalChannels[index].fd);
            }
            ss.closing_wait = 3 * 100;
            if (ioctl(physicalChannels[index].fd, TIOCSSERIAL, &ss) < 0) {
                RLOGE("%s: ERROR SET SERIAL STRUCT(fd = %d), errno is %d",
                    __FUNCTION__, physicalChannels[index].fd, errno);
            }
#endif /* MTK_ECCCI_C2K */
        }
    }
}

#ifdef MTK_ECCCI_C2K
void initPhysicalChannel(int index){
    int interface_num = s_mux_fd_property_list[index];
    int atChannelCnt = 0;
    int sleepTime = 10; /*s*/
    int maxRetryTime = 5;
    char *atDevPath = NULL;
    for (;;) {
        RLOGD("waitForAdjustPortPath interface_num=%d", interface_num);
        atDevPath = ccci_get_node_name(interface_num, MD_SYS3);
        if (NULL == atDevPath && atChannelCnt < maxRetryTime) {
            RLOGI("failed to get at device node");
            atChannelCnt++;
            sleep(sleepTime);
            continue;
        } else {
            if (NULL == atDevPath) {
                LOG_ALWAYS_FATAL("waitForAdjustPortPath interface_num %d get fail", interface_num);
            } else {
                atChannelCnt = 0;
                strlcat(physicalChannels[index].devicePath, atDevPath,
                        sizeof(physicalChannels[index].devicePath));
                openDevicePath(index, atDevPath);
                atDevPath = NULL;
                RLOGD("PhysicalChannel[%d].devicePath = %s", index, physicalChannels[index].devicePath);
                break;
            }
        }
    }
}
#elif C2K_RIL_LEGACY
void initPhysicalChannel(int index){
    int interface_num = s_mux_fd_property_list[index];
#ifdef USB_FS_EXCEPTION
    int atChannelCnt = 0;
    int atResetCnt = 0;
    int otherResetCnt = 0;
    int sleepTime = 10; /*s*/
    int powerWaitTime = 10; /*s*/
    int maxRetryTime = 5;
#endif
    char *atDevPath = NULL;
    for (;;) {
        RLOGD("waitForAdjustPortPath interface_num=%d", interface_num);
        atDevPath = viatelAdjustDevicePathFromProperty(interface_num);
        if (NULL == atDevPath) {
            RLOGI("failed to get at device from property");
#ifdef USB_FS_EXCEPTION
            atChannelCnt++;
            RLOGE("adjust AT port error, retrying %d...\n", atChannelCnt);
            if ((atChannelCnt >= maxRetryTime) && (!CmpBypassMode())) {
                atResetCnt++;
                RLOGE("retry %d more times, reset CP for %d more times...\n",
                        atChannelCnt, atResetCnt);
                atChannelCnt = 0;
                reset_cbp(NO_DEVICE_PORT);
            }
            if ((atResetCnt >= maxRetryTime) && (!CmpBypassMode())) {
                RLOGE("reset CP for %d more times\n", atResetCnt);
                atResetCnt = 0;
                RLOGE("start power off cbp...\n");
                powerCbp(0);
                sleep(powerWaitTime);
                RLOGE("start power on cbp...\n");
                powerCbp(1);
            }
            sleep(sleepTime);
#endif /* USB_FS_EXCEPTION */
            continue;
        }
#ifdef USB_FS_EXCEPTION
        atChannelCnt = 0;
        atResetCnt = 0;
#endif /* USB_FS_EXCEPTION */
        strlcat(physicalChannels[index].devicePath, atDevPath,
                sizeof(physicalChannels[index].devicePath));
        openDevicePath(index, atDevPath);
        RLOGD("PhysicalChannel[%d].devicePath = %s", index, physicalChannels[index].devicePath);
        atDevPath = NULL;
        break;
    }
}
#endif /* MTK_ECCCI_C2K */

int initPseudoChannel(int index) {
    struct termios options;
    char *pts;

    pseudoChannels[index].serverFd = open("/dev/ptmx", O_RDWR | O_NONBLOCK);
    if (pseudoChannels[index].serverFd < 0) {
        RLOGE("open ptmx error: %d, transit to state closing", errno);
        return -1;
    }
    pts = ptsname(pseudoChannels[index].serverFd);
    if (pts == NULL) {
        RLOGE("pts null, transit to state closing");
        return -1;
    }
    strncpy(pseudoChannels[index].clientPath, pts, strlen(pts) + 1);
    tcgetattr(pseudoChannels[index].serverFd, &options); //get the parameters
    bzero(&options, sizeof(options));
    options.c_cflag = BAUDRATE | CREAD | CLOCAL | CS8;
    options.c_iflag = IGNPAR;
    options.c_lflag = 0;    /* disable CANON, ECHO*, etc */
    options.c_oflag = 0;    /* set raw output */
    /* no timeout but request at least one character per read */
    options.c_cc[VTIME] = 0;
    options.c_cc[VMIN] = 1;

    SYSCHECK(tcflush(pseudoChannels[index].serverFd, TCIFLUSH));
    SYSCHECK(tcsetattr(pseudoChannels[index].serverFd, TCSANOW, &options));
    if (!strcmp(pseudoChannels[index].serverPath, "/dev/ptmx")) {
        //Otherwise programs cannot access the pseudo terminals
        SYSCHECK(grantpt(pseudoChannels[index].serverFd));
        SYSCHECK(unlockpt(pseudoChannels[index].serverFd));

        if (strlen(pseudoChannels[index].clientLink) > 0) {
                        chown(pseudoChannels[index].clientPath, AID_RADIO, AID_RADIO);
            unlink(pseudoChannels[index].clientLink);
            SYSCHECK(chmod(pseudoChannels[index].clientPath, 0660));
            SYSCHECK(symlink(pseudoChannels[index].clientPath, pseudoChannels[index].clientLink));
            RLOGI("Link[%d] %s with %s, errno = %d", index, pseudoChannels[index].clientPath,
                    pseudoChannels[index].clientLink, errno);
        }
    }
    return 0;
}

void initChannelConfiguration(int channelNumber) {
    pseudoChannelNumber = channelNumber;
    transitToState(STATE_INITIALIZING);
}

void initPhysicalChannelValue() {
    int i;
    for (i = 0; i < physicalChannelNumber; i++) {
        memset(physicalChannels[i].devicePath, 0, ARRAY_LENGTH(physicalChannels[i].devicePath));
        physicalChannels[i].fd = INITIAL_VALUE;
        physicalChannels[i].pseudoIndexMapped = updatePhysicalChannelMapping(i);
        physicalChannels[i].p_channelMutex = malloc(sizeof(pthread_mutex_t));
        assert(physicalChannels[i].p_channelMutex);
        pthread_mutex_init(physicalChannels[i].p_channelMutex, NULL);
    }
}

void deInitPhysicalChannelValue() {
    int i;
    for (i = 0; i < physicalChannelNumber; i++) {
        memset(physicalChannels[i].devicePath, 0, ARRAY_LENGTH(physicalChannels[i].devicePath));
        close(physicalChannels[i].fd);
        physicalChannels[i].fd = INITIAL_VALUE;
        if (physicalChannels[i].p_channelMutex != NULL) {
            free(physicalChannels[i].p_channelMutex);
        }
    }
}

void initPseudoChannelValue() {
    int i;
    for (i = 0; i < pseudoChannelNumber; i++) {
        memset(pseudoChannels[i].clientLink, 0, ARRAY_LENGTH(pseudoChannels[i].clientLink));
        sprintf(pseudoChannels[i].clientLink, "%s%d", "/dev/radio/pttyc2kat", i);
        RLOGD("Init: Channel %d link is %s", i, pseudoChannels[i].clientLink);
        memset(pseudoChannels[i].clientPath, 0, ARRAY_LENGTH(pseudoChannels[i].clientPath));
        memset(pseudoChannels[i].serverPath, 0, ARRAY_LENGTH(pseudoChannels[i].serverPath));
        strlcat(pseudoChannels[i].serverPath, "/dev/ptmx", sizeof(pseudoChannels[i].serverPath));
        RLOGD("Init: Sever Paht is %s", pseudoChannels[i].serverPath);
        pseudoChannels[i].serverFd = INITIAL_VALUE;
        pseudoChannels[i].physicalIndexMapped = INITIAL_VALUE;
    }
}

void deInitPseudoChannelValue() {
    int i;
    for (i = 0; i < pseudoChannelNumber; i++) {
        unlink(pseudoChannels[i].clientPath);
        close(pseudoChannels[i].serverFd);
        memset(pseudoChannels[i].clientPath, 0, ARRAY_LENGTH(pseudoChannels[i].clientPath));
        pseudoChannels[i].serverFd = INITIAL_VALUE;
        pseudoChannels[i].physicalIndexMapped = INITIAL_VALUE;
    }
}

void constructChannelMapping() {
    int i;
    for(i = 0; i < pseudoChannelNumber; i++) {
        int indexMapped = i;
        // If unsupport 8 channel in MD3. We need handle 2 CC, 3 NW, 6 ATCI, 7 DEFAULT
        indexMapped = updatePseudoChannelMapping(i);
        pseudoChannels[i].physicalIndexMapped = indexMapped;
        pseudoChannels[i].p_channelMutex = physicalChannels[indexMapped].p_channelMutex;
        RLOGI("constructChannelMapping, pseudoChannels[%d] = %d, indexMapped = %d",
                i, physicalChannels[indexMapped].pseudoIndexMapped, indexMapped);
    }
}

void handleStateInitializing() {
    int i;

#ifdef MTK_ECCCI_C2K
    // dynamic load physical channel number
    triggerCCCIIoctl(CCCI_IOC_GET_AT_CH_NUM, &physicalChannelNumber);
    RLOGI("init %d physical channels", physicalChannelNumber);
#endif /* MTK_ECCCI_C2K */

    physicalChannels = malloc(physicalChannelNumber * sizeof(PhysicalChannel));
    assert(physicalChannels);
    initPhysicalChannelValue();
    pseudoChannels = malloc(pseudoChannelNumber * sizeof(PseudoChannel));
    assert(pseudoChannels);
    initPseudoChannelValue();
    for(i = 0; i < physicalChannelNumber; i++) {
        initPhysicalChannel(i);
    }
    isPhysicalChannelReady = CHANNEL_READY;
    for(i = 0; i < pseudoChannelNumber; i++) {
        if (initPseudoChannel(i) < 0) {
            transitToState(STATE_CLOSING);
            return;
        }
    }
    isPseudoChannelReady = CHANNEL_READY;
    constructChannelMapping();
    transitToState(STATE_MUXING);
}

int createThread(pthread_t * thread_id,void * thread_function,
                  void * thread_function_arg)
{
    int ret;
    pthread_attr_t thread_attr;
    pthread_attr_init(&thread_attr);
    pthread_attr_setdetachstate(&thread_attr, PTHREAD_CREATE_DETACHED);
    ret = pthread_create(thread_id, &thread_attr, thread_function, thread_function_arg);
    if (ret < 0) {
        RLOGE("createThread failed, errno:%d", errno);
    }
    return 0;
}

int writeToChannel(int fd, const char* buffer) {
    int ret;
    int writeErrno;
    do {
        ret = write(fd, buffer, strlen(buffer));
        writeErrno = errno;
    } while (ret < 0 && (writeErrno == EINTR || writeErrno == EAGAIN));
    //RLOGD("write to %d, errno = %d, ret = %d", writeErrno, ret);
    return ret;
}

void physicalChannelLoop() {
    fd_set rfds;
    struct timeval timeout;
    int sel;
    int i;
    int maxFd = 0;
    char buf[1024];
    int len;
    int ret;

    while (channelState != STATE_CLOSING) {
        FD_ZERO(&rfds);
        for(i = 0; i < physicalChannelNumber; i++) {
            FD_SET(physicalChannels[i].fd, &rfds);
            if (physicalChannels[i].fd > maxFd) {
                maxFd = physicalChannels[i].fd;
            }
        }
        timeout.tv_sec = 5;
        timeout.tv_usec = 0;
        sel = select(maxFd + 1, &rfds, NULL, NULL, &timeout);
        if (sel < 0) {
            if (errno == EINTR) {
                RLOGE("physical read: EINTR, continue");
                continue;
            } else {
                RLOGE("select failure!");
                break;
            }
        } else if (sel == 0) {
            RLOGD("select timeout! continue");
            continue;
        } else {
            //RLOGD("Selected %d", sel);
            for(i = 0; i < physicalChannelNumber; i++) {
                int fd = physicalChannels[i].fd;
                int pseudoIndexMapped = physicalChannels[i].pseudoIndexMapped;
                if (fd != INITIAL_VALUE && FD_ISSET(fd, &rfds)) {
                    memset(buf, '\0', sizeof(buf));
                    len = read(fd, buf, sizeof(buf) - 1);
                    SYSCHECK(len);
                    RLOGD("Read %d bytes from serial device, channel: %d, fd: %d, errno: %d", len,
                            i, fd, errno);
                    int mappingFd = pseudoChannels[pseudoIndexMapped].serverFd;
                    ret = writeToChannel(mappingFd, buf);
                    // Error Handling
                    if (errno == EIO) {
                        RLOGE("EIO is caught, wait 1s to sent again");
                        sleep(1);
                    } else if (isPseudoChannelReady == CHANNEL_NOT_READY) {
                        RLOGE("PseudoChannel closed, wait 2s for reconnect");
                        sleep(2);
                    }
                }
            }
        }
    }
}

void pseudoChannelLoop() {
    fd_set rfds;
    struct timeval timeout;
    int sel;
    int i;
    int maxFd = 0;
    char buf[1024];
    int len;
    int ret;
    while (1) {
        FD_ZERO(&rfds);
        for(i = 0; i < pseudoChannelNumber; i++) {
            FD_SET(pseudoChannels[i].serverFd, &rfds);
            if (pseudoChannels[i].serverFd > maxFd) {
                maxFd = pseudoChannels[i].serverFd;
            }
        }
        timeout.tv_sec = 5;
        timeout.tv_usec = 0;
        sel = select(maxFd + 1, &rfds, NULL, NULL, &timeout);
        if (sel < 0) {
            if (errno == EINTR) {
                RLOGE("physical read: EINTR, continue");
                continue;
            } else {
                RLOGE("select failure!");
                break;
            }
        } else if (sel == 0) {
            RLOGD("select timeout! continue");
            continue;
        } else {
            //RLOGD("Selected %d", sel);
            for(i = 0; i < pseudoChannelNumber; i++) {
                if (FD_ISSET(pseudoChannels[i].serverFd, &rfds)) {
                    physicalChannels[pseudoChannels[i].physicalIndexMapped].pseudoIndexMapped = i;
                    memset(buf, '\0', sizeof(buf));
                    len = read(pseudoChannels[i].serverFd, buf, sizeof(buf) - 1);
                    SYSCHECK(len);
                    //RLOGD("Read %d bytes from pseudo device", len);
                    int mappingFd = physicalChannels[pseudoChannels[i].physicalIndexMapped].fd;
                    RLOGD("Write %d bytes to serial device, channel: %d, fd: %d, errno: %d",
                        len, pseudoChannels[i].physicalIndexMapped, mappingFd, errno);
                    ret = writeToChannel(mappingFd, buf);
                    // error handling, write to physical channel, may encounter modem exception
                    if (errno==ETXTBSY || errno==ENODEV) {
                        RLOGE("errno %d, modem exception, transit to closing state", errno);
                        physicalChannelError = ERROR_DEVICE_DISCONNECT;
                        transitToState(STATE_CLOSING);
                        return;
                    }
                }
            }
        }
    }
}

void handleStateMuxing() {
    createThread(&physicalChannelRead, physicalChannelLoop, NULL);
    createThread(&pseudoChannelRead, pseudoChannelLoop, NULL);
}

void resetChannelError() {
    RLOGD("Reset Channel Error");
    physicalChannelError = NO_ERROR;
    pseudoChannelError = NO_ERROR;
}

void handleStateError() {
    switch(pseudoChannelError) {
        default:
            RLOGD("Error not support, errno: %d", errno);
            break;
    }
    resetChannelError();
}

void handleStateClosing() {
    deInitPhysicalChannelValue();
    isPhysicalChannelReady = CHANNEL_NOT_READY;
    deInitPseudoChannelValue();
    isPseudoChannelReady = CHANNEL_NOT_READY;
    resetChannelError();
}

void resetErrno() {
    // reset errno because it will not be cleared
    errno = 0;
}

void transitToState(ChannelState state) {
    if (state < 0 || state > STATE_CLOSING) {
        RLOGD("Invalid State transition to %d, return", state);
        return;
    }

    channelState = state;
    switch(channelState) {
        case STATE_INITIALIZING:
            RLOGD("state initializing");
            resetErrno();
            handleStateInitializing();
            break;
        case STATE_MUXING:
            RLOGD("state muxing");
            resetErrno();
            handleStateMuxing();
            break;
        case STATE_ERROR:
            resetErrno();
            handleStateError();
            RLOGD("state error");
            break;
        case STATE_CLOSING:
            resetErrno();
            handleStateClosing();
            RLOGD("state closing");
            break;
        default:
            RLOGD("state not handled %d", channelState);
            break;
    }
}

int updatePseudoChannelMapping(int index) {
    int indexMapped = index;
    if (physicalChannelNumber == PHYSICAL_MAX_CHANNEL_NUM) {
        RLOGD("Max Channel Number, return index %d", index);
        return indexMapped;
    }
    switch (index) {
        case 0:
        case 2:
        case 3:
        case 6:
        case 7:
            indexMapped = 0;
            break;
        case 1:
            indexMapped = 3;
            break;
        case 4:
            indexMapped = 1;
            break;
        case 5:
            indexMapped = 2;
            break;
        default:
            RLOGD("UpdatePseudoChannelMapping, not handled %d", index);
            break;
    }
    RLOGD("Mapping pseudo Channel %d to physical channel %d", index, indexMapped);
    return indexMapped;
}

// Giving default value to the puseodochannel physicalchannel mapping
int updatePhysicalChannelMapping(int index) {
    int indexMapped = index;
    if (physicalChannelNumber == PHYSICAL_MAX_CHANNEL_NUM) {
        return indexMapped;
    }
    switch (index) {
        case 0:
            indexMapped = 0;
            break;
        case 1:
            indexMapped = 4;
            break;
        case 2:
            indexMapped = 5;
            break;
        case 3:
            indexMapped = 1;
            break;
        default:
            RLOGD("UpdatePhysicalChannelMapping, not handled %d", index);
            break;
        }
    return indexMapped;
}
