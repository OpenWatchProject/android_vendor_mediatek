/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
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

#ifndef __RFX_SOCKET_STATE_MANAGER_H__
#define __RFX_SOCKET_STATE_MANAGER_H__

/*****************************************************************************
 * Include
 *****************************************************************************/
#include <utils/Looper.h>
#include <utils/threads.h>

#include "Rfx.h"
#include "RfxController.h"
#include "RfxStatusDefs.h"
#include "RfxVariant.h"
#include "RfxSocketState.h"
#include "RfxSocketStateMessage.h"

using ::android::Message;

/*****************************************************************************
 * Class RfxSocketStateManager
 *****************************************************************************/
class RfxSocketStateManager : public RfxController
{
    RFX_DECLARE_CLASS(RfxSocketStateManager);
    RFX_OBJ_DECLARE_SINGLETON_CLASS(RfxSocketStateManager);

public:
    RfxSocketStateManager();
    void onInit();
    void onDeinit();
    using RfxController::processMessage;
    void processMessage(const sp<RfxSocketStateMessage>& message);
    static void notifySocketState(RILD_RadioTechnology_Group group,
            int slotId, int fd, bool isConnected);
    void setCdmaSocketSlotChange(int c2kSlot);

private:
    void setSocketState(RILD_RadioTechnology_Group groupId,
            bool isConnected, int slotId, int socFd);

private:
    bool mIsC2kSocketConnected;
    int mSocketState[MAX_RFX_SLOT_COUNT];
    int mC2kSlotId;
};

#endif /* __RFX_SOCKET_STATE_MANAGER_H__ */

