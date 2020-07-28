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

#ifndef WPAF_DRIVER_H
#define WPAF_DRIVER_H

#include <map>
#include <mtk_log.h>
// #include "utils/Mutex.h"
#include "Mutex.h"

#include "WpfaDriverAdapter.h"
#include "WpfaDriverMessage.h"
#include "WpfaRingBuffer.h"
#include "WpfaShmSynchronizer.h"

using ::android::Mutex;

/*
 * =============================================================================
 *                     Defines
 * =============================================================================
 */
#define DRIVER_TID_START (0)
#define DRIVER_TID_END   (99)

#ifndef WPFA_D_ASSERT
#define WPFA_D_ASSERT(_expr)                                                        \
        do {                                                                        \
            if (!(_expr)) {                                                         \
                mtkLogE("Wpfa", "DRIVER_ASSERT:%s, %d", __FILE__, __LINE__);        \
                mtkAssert(NULL);                                                    \
                exit(0);                                                            \
            }                                                                       \
        } while(0)
#endif

/*
 * =============================================================================
 *                     typedef
 * =============================================================================
 */

 /**
 * callback function prototype
 */
typedef int(*CallbackFunc)(void *);

 /**
 * callback function structure
 */
struct CallbackStruc {
    CallbackFunc cb;
};

/*
 * =============================================================================
 *                     class
 * =============================================================================
 */
class WpfaDriver {
public:
    WpfaDriver();
    virtual ~WpfaDriver();
    static WpfaDriver *getInstance();
    void init();

    int registerCallback(event_id_enum eventId, CallbackFunc callbackFunc);
    int unregisterCallback(event_id_enum eventId);
    int notifyCallback(event_id_enum eventId, void *notifyArg);

    int notifyWpfaInit();
    int notifyWpfaVersion(uint16_t apVer, uint16_t mdVer);

    int sendDataPackageToModem(WpfaRingBuffer *ringBuffer);

protected:


private:
    int checkDriverAdapterState();
    int checkShmControllerState();
    int addCallback(event_id_enum eventId, CallbackFunc callbackFunc);

    // generate transaction id for SHM writer
    uint16_t generateDriverTid();

    /**
     * singleton pattern
     */
    static WpfaDriver *sInstance;
    static Mutex sWpfaDriverInitMutex;

    std::map<int, CallbackStruc> mapCallbackFunc;

    WpfaDriverAdapter *mWpfaDriverAdapter;
    WpfaShmSynchronizer *mWpfaShmSynchronizer;

    uint16_t mDriverTid;

};

#endif /* end of WPAF_DRIVER_H */
