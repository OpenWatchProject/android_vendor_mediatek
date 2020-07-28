/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2012. All rights reserved.
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

#include <stdint.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <poll.h>
#include <fcntl.h>

#include <sys/cdefs.h>
#include <sys/types.h>

#include <log/log.h>

#include "SensorEventReader.h"

#undef LOG_TAG
#define LOG_TAG "SensorEventReader"

SensorEventCircularReader::SensorEventCircularReader(size_t numEvents)
    : mBuffer(new struct sensor_event[numEvents * 2]),
      mBufferEnd(mBuffer + numEvents),
      mHead(mBuffer),
      mCurr(mBuffer),
      mFreeSpace(numEvents) {
    mReadFd = -1;
    mWriteFd = -1;
}

SensorEventCircularReader::~SensorEventCircularReader() {
    delete [] mBuffer;

    if (mWriteFd >= 0)
        close(mWriteFd);
    if (mReadFd >= 0)
        close(mReadFd);
}

ssize_t SensorEventCircularReader::fill() {
    size_t numEventsRead = 0;
    if (mFreeSpace) {
        const ssize_t nread = TEMP_FAILURE_RETRY(read(mReadFd, mHead, mFreeSpace * sizeof(struct sensor_event)));
        if (nread < 0 || nread % sizeof(struct sensor_event)) {
            return 0;
        }

        numEventsRead = nread / sizeof(struct sensor_event);
        if (numEventsRead) {
            mHead += numEventsRead;
            mFreeSpace -= numEventsRead;
            if (mHead > mBufferEnd) {
                size_t s = mHead - mBufferEnd;
                if (s < ULONG_MAX / sizeof(struct sensor_event)) {
                    memcpy(mBuffer, mBufferEnd, s * sizeof(struct sensor_event));
                    mHead = mBuffer + s;
                } else
                    ALOGE("SensorEventCircularReader fill data failed");
            }
        }
    }

    return numEventsRead;
}

ssize_t SensorEventCircularReader::readEvent(struct sensor_event const** events) {
    *events = mCurr;
    ssize_t available = (mBufferEnd - mBuffer) - mFreeSpace;
    return available ? 1 : 0;
}

ssize_t SensorEventCircularReader::writeEvent(struct sensor_event *events) {
    int ret = -1;
    ret = write(mWriteFd, events, sizeof(struct sensor_event));
    if (ret < 0)
        ALOGE("write event to mWriteFd failed fd=%d\n", mWriteFd);
    return ret;
}

bool SensorEventCircularReader::pendingEvent(void) {
    ssize_t available = (mBufferEnd - mBuffer) - mFreeSpace;
    return available ? true : false;
}

void SensorEventCircularReader::next() {
    mCurr++;
    mFreeSpace++;
    if (mCurr >= mBufferEnd) {
        mCurr = mBuffer;
    }
}
int SensorEventCircularReader::selectSensorEventFd(const char *path) {
    int ret = -1;
    int pipeFds[2] = {-1};

    if (path == NULL) {
        ret = pipe(pipeFds);
        if (ret < 0) {
            ALOGE("error creating pipe (%s)\n", strerror(errno));
            return -1;
        }
        ret = fcntl(pipeFds[0], F_SETFL, O_NONBLOCK);
        if (ret < 0) {
            ALOGE("error to set file1 flag (%d)\n", ret);
            return ret;
        }
        ret = fcntl(pipeFds[1], F_SETFL, O_NONBLOCK);
        if (ret < 0) {
            ALOGE("error to set file2 flag (%d)\n", ret);
            return ret;
        }
        mWriteFd = pipeFds[1];
        mReadFd = pipeFds[0];
    } else {
        mReadFd = TEMP_FAILURE_RETRY(open(path, O_RDONLY));
        if (mReadFd < 0) {
            ALOGE("couldn't find device path=%s", path);
            return -1;
        }
    }
    return 0;
}

int SensorEventCircularReader::getReadFd() {
    return mReadFd;
}

int SensorEventCircularReader::getWriteFd() {
    return mWriteFd;
}
