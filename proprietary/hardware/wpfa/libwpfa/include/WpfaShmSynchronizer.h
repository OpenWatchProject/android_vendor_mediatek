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

#ifndef WPAF_SHM_SYNCHRONIZER_H
#define WPAF_SHM_SYNCHRONIZER_H

#include <mtk_log.h>

// #include "utils/Mutex.h"
#include "Mutex.h"
#include <pthread.h>

#include "WpfaDriverMessage.h"
#include "WpfaRingBuffer.h"
#include "WpfaShmAccessController.h"

using ::android::Mutex;

/*
 * =============================================================================
 *                     Defines
 * =============================================================================
 */
#define SHM_WRITER_START_TID (10000)
#define SHM_WRITER_END_TID   (50000)

/*
 * =============================================================================
 *                     typedef
 * =============================================================================
 */

typedef int writer_state_enum;
enum {
    SHM_WRITER_STATE_IDLE    = 1,
    SHM_WRITER_STATE_READY   = 2,
    SHM_WRITER_STATE_MD_READING = 3
};

typedef struct ShmCtrlParam {
    writer_state_enum mState;
    bool isWriting;
    bool isStateUpdating;

    uint32_t writeIdx;
    uint32_t writeSize;

    uint32_t newWriteIdx;
    uint32_t newWriteSize;

    pthread_mutex_t mutex;
    pthread_cond_t cond_can_write;          // signaled when data packets writed to shm
    pthread_cond_t cond_can_update_state;    // signaled when mState updated done
} ShmCtrlParam;


/*
 * =============================================================================
 *                     class
 * =============================================================================
 */
class WpfaShmSynchronizer {
public:
    WpfaShmSynchronizer();
    virtual ~WpfaShmSynchronizer();
    static WpfaShmSynchronizer *getInstance();
    void init();

    // API of control param
    void lock(const char* user);
    void trylock(const char* user);
    void unlock(const char* user);
    void waitCanWrite(const char* user);
    void signalCanWrite(const char* user);
    void waitCanUpdateState(const char* user);
    void signalCanUpdateState(const char* user);
    void setState(writer_state_enum state);
    writer_state_enum getState();

    uint32_t getMaxDataBufferSize();
    uint32_t getRealDataBufferSize();
    int getCcciHandler();

    int processControlMessage(uint16_t tId, uint16_t msgId);
    int wirteDataToShm(WpfaRingBuffer *ringBuffer);

protected:


private:

    void setWritingFlag(bool isWriting);
    bool isWriting();
    void setStateUpdatingFlag(bool isUpdating);
    bool isStateUpdating();
    void dumpCtrlParams();

    // ccci ctrl message handle
    int onRequestData(uint16_t tId);
    int onRequestDataDone(uint16_t tId);

    // write data pkts
    int writeInIdleState(WpfaRingBuffer *ringBuffer);
    int writeInReadyState(WpfaRingBuffer *ringBuffer);
    int writeInMdReadingState(WpfaRingBuffer *ringBuffer);

    int sendMessageToModem(uint16_t tId, uint16_t msgId);



    // generate transaction id for SHM writer
    uint16_t generateShmTid();

    int checkDriverAdapterState();
    /**
     * singleton pattern
     */
    static WpfaShmSynchronizer *sInstance;
    static Mutex sWpfaShmSynchronizerInitMutex;

    static ShmCtrlParam mControlPara;
    uint16_t mTid;

    WpfaShmAccessController *mShmAccessController;
};

#endif /* end of WPAF_SHM_SYNCHRONIZER_H */
