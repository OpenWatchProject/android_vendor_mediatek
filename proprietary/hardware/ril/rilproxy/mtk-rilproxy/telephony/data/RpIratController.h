/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2015. All rights reserved.
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

#ifndef __RP_IRAT_CONTROLLER_H__
#define __RP_IRAT_CONTROLLER_H__

/*****************************************************************************
 * Include
 *****************************************************************************/
#include "RfxController.h"

/*****************************************************************************
 * Class RpIratController
 *****************************************************************************/

class RpIratController : public RfxController {
    RFX_DECLARE_CLASS(RpIratController);  // Required: declare this class

public:
    RpIratController();
    virtual ~RpIratController();

// Override
protected:
    virtual void onInit();
    virtual void onDeinit();
    virtual bool onHandleUrc(const sp<RfxMessage>& message);
    virtual bool onHandleResponse(const sp<RfxMessage>& message);

private:
    bool mIsDuringIrat;
    // Private functions
    void handleConfirmIratResponse(const sp<RfxMessage>& message);
    void handleIratStateChangedUrc(const sp<RfxMessage>& message);
    void updateIratStatus(bool status);
    void confirmIrat(RILD_RadioTechnology_Group sourceRat);
    void onRadioStateChanged(RfxStatusKeyEnum key,
            RfxVariant oldValue, RfxVariant value);

private:
    // Private members
    RpDataController *mDataController;
    const int IRAT_CONFIRM_ACCEPTED = 1;
    const int IRAT_CONFIRM_DENIED = 0;
};

enum IratAction {
    IRAT_ACTION_UNKNOWN = 0,
    IRAT_ACTION_SOURCE_STARTED = 1,
    IRAT_ACTION_TARGET_STARTED = 2,
    IRAT_ACTION_SOURCE_FINISHED = 3,
    IRAT_ACTION_TARGET_FINISHED = 4
};

enum IratType{
    IRAT_TYPE_UNKNOWN = 0,
    IRAT_TYPE_LTE_EHRPD = 1,
    IRAT_TYPE_LTE_HRPD = 2,
    IRAT_TYPE_EHRPD_LTE = 3,
    IRAT_TYPE_HRPD_LTE = 4,
    IRAT_TYPE_FAILED = 5,
    IRAT_TYPE_LWCG_LTE_EHRPD = 6,
    IRAT_TYPE_LWCG_LTE_HRPD = 7,
    IRAT_TYPE_LWCG_EHRPD_LTE = 8,
    IRAT_TYPE_LWCG_HRPD_LTE = 9
};

#endif /* __RP_IRAT_CONTROLLER_H__ */
