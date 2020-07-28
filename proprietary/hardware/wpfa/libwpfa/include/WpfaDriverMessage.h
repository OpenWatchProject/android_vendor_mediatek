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

#ifndef WPFA_DRIVER_MESSAGE_H
#define WPFA_DRIVER_MESSAGE_H

// #include "utils/String8.h"
#include "String8.h"
// #include "utils/RefBase.h"
#include "RefBase.h"

#include <stdint.h>
#include <mtk_log.h>

#include "FilterStructure.h"

#include "WpfaDriverAcceptData.h"
#include "WpfaDriverBaseData.h"
#include "WpfaDriverRegFilterData.h"
#include "WpfaDriverDeRegFilterData.h"
#include "WpfaDriverVersionData.h"
#include "WpfaDriverULIpPkt.h"
#include "WpfaCcciDataHeaderEncoder.h"

using ::android::String8;
using ::android::RefBase;
using ::android::sp;

/*
 * =============================================================================
 *                     MACRO
 * =============================================================================
 */
#define CCCI_HEADER_SIZE (sizeof(ccci_msg_hdr_t))

#define CCCI_BODY_REG_FILTER_SIZE (sizeof(wifiproxy_m2a_reg_dl_filter_t))
#define CCCI_BODY_DEREG_FILTER_SIZE (sizeof(wifiproxy_m2a_dereg_dl_filter_t))
#define CCCI_BODY_REG_ACCEPT_SIZE (sizeof(wifiproxy_a2m_reg_reply_t))
#define CCCI_BODY_DEREG_ACCEPT_SIZE (sizeof(wifiproxy_a2m_dereg_reply_t))
#define CCCI_BODY_FILTER_VER_SIZE (sizeof(wifiproxy_ap_md_filter_ver_t))

/*
 * =============================================================================
 *                     typedef
 * =============================================================================
 */


/*
 * =============================================================================
 *                     class
 * =============================================================================
 */

class WpfaDriverMessage : public virtual RefBase {
public:
    uint16_t getMsgId() const {
        return mMsgId;
    }

    uint16_t getTid() const {
        return mTid;
    }

    uint16_t getType() const {
        return mType;
    }

    uint16_t getParams() const {
        return mParams;
    }

    WpfaDriverBaseData *getData() const {
        return mData;
    }

    static sp<WpfaDriverMessage> obtainMessage(uint16_t msgId, uint16_t tId, uint16_t type,
            uint16_t params, const WpfaDriverBaseData &data);

    static sp<WpfaDriverMessage> obtainMessage(uint16_t msgId, uint16_t tId, uint16_t type,
            uint16_t params);

    static int getCcciMsgBodySize(uint16_t msgId);
    static WpfaDriverBaseData* copyData(uint16_t msgId, const WpfaDriverBaseData *data);


    String8 toString() const;

protected:

private:
    WpfaDriverMessage();
    virtual ~WpfaDriverMessage();

    uint16_t mMsgId;
    uint16_t mTid;
    uint16_t mType;
    uint16_t mParams;
    WpfaDriverBaseData *mData;

};

#endif /* end of WPFA_DRIVER_MESSAGE_H */
