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

#ifndef __RTC_CAPABILITY_SWITCH_CHECKER_H__
#define __RTC_CAPABILITY_SWITCH_CHECKER_H__

/*****************************************************************************
 * Include
 *****************************************************************************/
#include <telephony/mtk_ril.h>
#include "RfxIntsData.h"
#include "RfxController.h"
#include "RfxDefs.h"
#include "RfxStatusManager.h"
#include "RfxStringData.h"
#include "RfxStringsData.h"
#include "RfxVoidData.h"

/*****************************************************************************
 * Define
 *****************************************************************************/
#define ENHANCEMENT_T_PLUS_T  (0x01)
#define ENHANCEMENT_T_PLUS_W  (0x02)
#define ENHANCEMENT_T_PLUS_C  (0x04)
#define ENHANCEMENT_W_PLUS_C  (0x08)
#define ENHANCEMENT_W_PLUS_W  (0x10)
#define ENHANCEMENT_W_PLUS_NA (0x20)
#define ENHANCEMENT_C_PLUS_C  (0x40)

#define CURRENT_SLOT (-2)
#define UNKNOWN_SLOT (-1)
#define SLOT_0       (0)
#define SLOT_1       (1)
#define SLOT_2       (2)

#define SIM_ON       (11)

/*****************************************************************************
 * Class RtcCapabilitySwitchChecker
 *****************************************************************************/
class RtcCapabilitySwitchChecker : public RfxController {
    // Required: declare this class
    RFX_DECLARE_CLASS(RtcCapabilitySwitchChecker);

  public:
    RtcCapabilitySwitchChecker();
    virtual ~RtcCapabilitySwitchChecker();

    bool isInCalling(bool log);
    bool isInPukLock(bool log);
    bool isReadyForSwitch(bool log);
    virtual bool isSkipCapabilitySwitch(int new_major_slot);
    virtual int getHigherPrioritySlot();
    bool isCapabilitySwitching();

  protected:
    bool isCdmaOnlySim(int slot_id);
    bool isCdmaDualModeSim(int slot_id);
    bool isCommonGsmSim(int slot_id);
    bool imsiReady(bool log);
    const String8& getImsi(int slot_id);
    bool isSupportSimSwitchEnhancement(int sim_type);
    bool isLteNetworkType(int nw_type);

  private:
    bool radioLockReady(bool log);
    bool eccStateReady(bool log);
    bool modemOffStateReady(bool log);
    bool simStateReady(bool log);
    bool waitingFlagReady(bool log);
    void getWaiveEnhancementConfig();

  protected:
    long int m_waive_enhance_config;
    // add one more status manage for no-slot controllers
    RfxStatusManager *m_status_managers[MAX_RFX_SLOT_ID + 1];
};

#endif /* __RTC_CAPABILITY_SWITCH_CHECKER_H__ */
