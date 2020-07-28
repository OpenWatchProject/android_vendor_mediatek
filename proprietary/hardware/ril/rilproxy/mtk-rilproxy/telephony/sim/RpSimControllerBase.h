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

#ifndef __RP_SIM_CONTROLLER_BASE_H__
#define __RP_SIM_CONTROLLER_BASE_H__

/*****************************************************************************
 * Include
 *****************************************************************************/
#include "RfxController.h"

/*****************************************************************************
 * Class RpSimControllerBase
 *****************************************************************************/
const char PROPERTY_RIL_FULL_UICC_TYPE[4][35] = {
    "vendor.gsm.ril.fulluicctype",
    "vendor.gsm.ril.fulluicctype.2",
    "vendor.gsm.ril.fulluicctype.3",
    "vendor.gsm.ril.fulluicctype.4",
};

const char PROPERTY_ICCID_SIM[4][35] = {
    "vendor.ril.iccid.sim1",
    "vendor.ril.iccid.sim2",
    "vendor.ril.iccid.sim3",
    "vendor.ril.iccid.sim4",
};

const char PROPERTY_RIL_CDMA_CARD_TYPE[4][35] = {
    "vendor.ril.cdma.card.type.1",
    "vendor.ril.cdma.card.type.2",
    "vendor.ril.cdma.card.type.3",
    "vendor.ril.cdma.card.type.4",
};

const char PROPERTY_RIL_CT3G[4][35] = {
    "vendor.gsm.ril.ct3g",
    "vendor.gsm.ril.ct3g.2",
    "vendor.gsm.ril.ct3g.3",
    "vendor.gsm.ril.ct3g.4",
};

const char PROPERTY_UIM_SUBSCRIBER_ID[4][35] = {
    "vendor.ril.uim.subscriberid.1",
    "vendor.ril.uim.subscriberid.2",
    "vendor.ril.uim.subscriberid.3",
    "vendor.ril.uim.subscriberid.4",
};

enum ENUM_ICC_TYPE {
    ICC_TYPE_SIM = 0,
    ICC_TYPE_USIM,
    ICC_TYPE_ISIM,
    ICC_TYPE_CSIM,
    ICC_TYPE_RUIM,
    ICC_TYPE_MAX
};

#define ICC_SIM    0x01
#define ICC_USIM   0x02
#define ICC_CSIM   0x04
#define ICC_RUIM   0x08

#define AID_PREFIX_LEN 14
#define LOG_CHECK_COUNT 2

class RpSimControllerBase : public RfxController {
    RFX_DECLARE_CLASS(RpSimControllerBase);  // Required: declare this class

public:
    RpSimControllerBase();

    virtual ~RpSimControllerBase();

protected:
    // Utility functions
    char* strdupReadString(Parcel *p);
    void writeStringToParcel(Parcel *p, const char *s);
    const char* requestToString(int reqId);
    const char* urcToString(int reqId);
    RILD_RadioTechnology_Group choiceDestViaAid(char* aid);
    RILD_RadioTechnology_Group choiceDestViaCurrCardType();
    bool supportCardType(const int icc_types);
    int isSupportCommonSlot();
    bool isOp09Card(const char *iccid);
    bool isSimSlotLockSupport();
};

#endif /* __RP_SIM_CONTROLLER_BASE_H__ */

