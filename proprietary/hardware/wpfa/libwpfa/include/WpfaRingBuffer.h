/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2016. All rights reserved.
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

#ifndef WPFA_RINGBUFFER_H
#define WPFA_RINGBUFFER_H

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include "stdint.h"
#include <mtk_log.h>

using namespace std;
/*
 * =============================================================================
 *                     ref struct
 * =============================================================================
 */


/*
 * =============================================================================
 *                     typedef
 * =============================================================================
 */
typedef struct region_info_t {
     uint32_t read_idx;
     uint16_t data_size;
} region_info_t;

typedef struct RingBufferControlParam {
    int mRbState;
    uint32_t mWriteIdx;
    uint32_t mReadIdx;
    uint32_t mReadDataSize;
    uint32_t mTempReadIdx;
    uint32_t mTempReadDataSize;

    pthread_mutex_t mutex;
    pthread_cond_t cond_can_write;  // signaled when items are raad done (removed)
    pthread_cond_t cond_can_read;   // signaled when items are writed (added)
} RingBufferControlParam;
/*
 * =============================================================================
 *                     class
 * =============================================================================
 */
#define RING_BUFFER_SIZE (10240)  /* Size = 10*1024 */

#define RING_BUFFER_STATE_NONE      (0)
#define RING_BUFFER_STATE_READING   (1)

#define INVALID_INDEX (RING_BUFFER_SIZE + 1)


class WpfaRingBuffer {
public:
    WpfaRingBuffer();
    virtual ~WpfaRingBuffer();

    void initRingBuffer();

    void lock(const char* user);
    void unlock(const char* user);
    void waitCanWrite(const char* user, uint16_t dataSize);
    void signalCanWrite(const char* user);
    void waitCanRead(const char* user);
    void signalCanRead(const char* user);

    int writeDataToRingBuffer(const void *src, uint16_t dataSize);
    void readDataFromRingBuffer(void *des, uint32_t readIdx, uint16_t dataSize);
    void readDataWithoutRegionCheck(void *des, uint32_t readIdx, uint16_t dataSize);

    //int notifyToReader(uint32_t readIdx, uint16_t dataSize);
    void getRegionInfoForReader(region_info_t* mRegion);
    uint32_t getReadIndex() {
        return mControlPara.mReadIdx;
    }
    uint32_t getReadDataSize() {
        return mControlPara.mReadDataSize;
    }

    void readDone();

    void setState(int state);
    int getState();
    int dump_hex(unsigned char *data, int len);

protected:


private:
    unsigned char mRingBuffer[RING_BUFFER_SIZE];
    RingBufferControlParam mControlPara;

    void updateReadDataSizeByState(uint16_t dataSize);

    void ringBuffer_memset(void *des, uint8_t value, uint16_t size);
    void ringBuffer_memcpy(void *des, const void *src, uint16_t size);

    bool isEmpty();
    bool isFull();

    uint16_t getFreeSize();
    uint16_t getDataCount();

    void toString();
    void dumpParam();
};

#endif /* end of WPFA_RINGBUFFER_H */

