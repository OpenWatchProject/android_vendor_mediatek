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

#ifndef __RP_IR_STRATEGY_H__
#define __RP_IR_STRATEGY_H__

/*****************************************************************************
 * Include
 *****************************************************************************/
#include "RfxController.h"
#include "IRpIrNwController.h"

/*****************************************************************************
 * Class RpIrStrategy
 *****************************************************************************/
class RpIrStrategy : public RfxController {
    RFX_DECLARE_CLASS(RpIrStrategy);

protected:
    RpIrController* m_IrController;
    IRpIrNwController* m_LwgController;
    IRpIrNwController* m_CdmaController;

    bool m_IsEnabled = false;
    bool m_IsCdmaRadioOn = false;
    bool m_IsLwgRadioOn = false;

public:
    RpIrStrategy(RpIrController* ric, IRpIrNwController* lwgController,
            IRpIrNwController* cdmaController);
    virtual ~RpIrStrategy();
    void setIfEnabled(bool enabled);
    bool getIfEnabled();
    bool getIsCdmaRadioOn() { return m_IsCdmaRadioOn; }
    bool getIsLwgRadioOn() { return m_IsLwgRadioOn; }
    void setIsCdmaRadioOn(bool radioOn) { m_IsCdmaRadioOn = radioOn; }
    void setIsLwgRadioOn(bool radioOn) { m_IsLwgRadioOn = radioOn; }
    IRpIrNwController* getLwgController() { return m_LwgController;}
    IRpIrNwController* getCdmaController() { return m_CdmaController;}
    virtual void onRatSwitchStart(const int newPrefNwType, const NwsMode newNwsMode,
            const RatSwitchCaller ratSwitchCaller) = 0;
    virtual void onRatSwitchDone(int curPrefNwType, int newPrefNwType) = 0;
    virtual int getHomeMcc() = 0;
    virtual void onWwopRatSwitchDone(int curPrefNwType, int newPrefNwType) = 0;

// Override
protected:
    virtual void onSetIfEnabled(bool enabled) = 0;
};

#endif /* __RP_IR_STRATEGY_H__ */

