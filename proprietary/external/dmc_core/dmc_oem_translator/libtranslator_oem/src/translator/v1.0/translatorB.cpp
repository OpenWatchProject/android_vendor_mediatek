/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * MediaTek Inc. (C) 2019. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

/*
 * EXAMPLE: A translator listen to a AP Monitor(APM) event
 *
 * In this Translator example, translatorB was linked to OID 1.1.2 in OemKpiMap.cpp.
 * Macro TRANSLATE(OEM, translatorB) is to define translateEx() function, and
 * macro IF_MSG(KPI_SOURCE_TYPE_APM, (MSGID)APM_MSG_FOO_BAR) declare this translator
 * listen to a "APM event". DMC will call translateEx() when
 * 1.) The OID (1.1.2) is being subscribed by DMC client.
 * 2.) AP submission layer sent APM event(APM_MSG_FOO_BAR), defined in apm_msg_defs.h, a debug event.
 *
 * We query parameters for event APM_MSG_FOO_BAR by calling the following macros
 * 1.) GET_TRANSLATE_DATA(): get APM event data.
 * 2.) GET_TRANSLATE_DATA_LEN(): get APM event data length.
 * 3.) GET_TRANSLATE_DATA_VERSION(): get APM event version code.
 *     Reference to our APM ICD to get the definition of event data structure.
 *
 * In this case, we skip this translation request by calling macro SKIP_TRANSLATE(), and
 * just print out the fields of this APM event.
 */

#include "TranslatorManager.h"
#include "apm_msg_defs.h"
#include "log_utils.h"

static const char *TAG_KPI = "translatorB";

#pragma pack(push, 1)
    typedef struct {
        int32_t    version;
        uint8_t    byte_;
        int32_t    byteLen;
        uint8_t    bool_;
        int16_t    short_;
        int32_t    int_;
        float      float_;
        int64_t    long_;
        double     double_;
        int32_t    strLen;
        int32_t    listSize;
    } fooBar_v1_t;

    typedef struct {
        int32_t foo;
        int32_t bar;
    } fooBarInner_v1_t;
#pragma pack(pop)

TRANSLATE(OEM, translatorB) {
    IF_MSG(KPI_SOURCE_TYPE_APM, (MSGID)APM_MSG_FOO_BAR) {
        UNUSED(obj);

        uint8_t *raw = (uint8_t *)GET_TRANSLATE_DATA();
        uint8_t *pData = raw;
        uint32_t data_len = GET_TRANSLATE_DATA_LEN();
        int32_t version = GET_TRANSLATE_DATA_VERSION();

        DMC_LOGD(TAG_KPI, "Received APM_MSG_FOO_BAR, data_len = %d, version = %d", data_len, version);

        switch(version) {
            case 1: {
                fooBar_v1_t *pFooBar = (fooBar_v1_t *)pData;
                DMC_LOGD(TAG_KPI, "version=%d", pFooBar->version);
                DMC_LOGD(TAG_KPI, "byte=%x, bool=%d, short=%hd, int=%d, float=%f, long=%" PRId64", double=%lf",
                        pFooBar->byte_, (bool)pFooBar->bool_, pFooBar->short_, pFooBar->int_,
                        pFooBar->float_, pFooBar->long_, pFooBar->double_);
                DMC_LOGD(TAG_KPI, "byteLen=%d, strLen=%d, listSize=%d",
                        pFooBar->byteLen, pFooBar->strLen, pFooBar->listSize);
                break;
            }
            default: {
                DMC_LOGE(TAG_KPI, "Not supoprted version = %d", version);
                break;
            }
        }

        // Call this macro to skip translate, not return translation data back to DMC client.
        SKIP_TRANSLATE();
    }
}