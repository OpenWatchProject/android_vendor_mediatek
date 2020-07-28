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
 * EXAMPLE: A translator listen to multiple MD Monitor(MDM) events
 *
 * In this Translator example, translatorC was linked to OID 1.2.1 in OemKpiMap.cpp.
 * Macro TRANSLATE(OEM, translatorC) is to define translateEx() function.
 * In this example, translatorC monitoring 2 types of modem events
 * 1.) EM: Use macro IF_EM("MSG_ID_EM_XXX") to specify MDM EM events
 * 2.) ICD: Use macro IF_ICD(TRAP_TYPE_ICD_AAA, ICD_ERRC_SAMPLE_EVENT) to specify MDM ICD events
 * Note: MediaTek EM and ICD are confidential to customer, and license required.
 * 
 * DMC will call translateEx() when
 * 1.) The OID (1.2.1) is being subscribed by DMC client.
 * 2.) Modem submission layer sent MDM event MSG_ID_EM_XXX or ICD_ERRC_SAMPLE_EVENT
 *
 * In this case, we output payload for EM event and skip for ICD event.
 */


#include "TranslatorManager.h"
#include "log_utils.h"

static const char *TAG_KPI = "translatorC";

#pragma pack(push, 1)
    typedef struct {
        long ue_mode;
        bool is_sms_over_ip;
    } racInfoData_t;
#pragma pack(pop)

TRANSLATE(OEM, translatorC) {
    IF_EM("MSG_ID_EM_RAC_INFO_IND") {
        racInfoData_t output_data = {.ue_mode = 0, .is_sms_over_ip = false};
        unsigned int length = sizeof(racInfoData_t);

        output_data.ue_mode = FIELD_VALUE("ue_mode", false);
        output_data.is_sms_over_ip = FIELD_VALUE("is_sms_over_ip", false);

        DMC_LOGD(TAG_KPI, "Received MSG_ID_EM_RAC_INFO_IND, ue_mode = %ld, is_sms_over_ip = %d, length = %u",
                output_data.ue_mode, output_data.is_sms_over_ip, length);

        // Write some data into output payload buffer
        obj.SetByteArray((uint8_t *)&output_data, length);
    }

    // Allow translator to subscribe multiple events
    #define ICD_ERRC_SAMPLE_EVENT 0x9527
    IF_ICD(TRAP_TYPE_ICD_RECORD, ICD_ERRC_SAMPLE_EVENT) {
        const uint8_t* payload = GET_ICD_MSG_BUFFER();
        uint8_t version = GET_ICD_VERSION();

        if (payload == NULL) {
            SKIP_TRANSLATE();
            return;
        }

        switch (version) {
            case 1:
                // Parse the ICD data, the data structure is MediaTek confidential.
                break;
            default:
                DMC_LOGE(TAG_KPI, "Invalid ICD_ERRC_SAMPLE_EVENT version = %" PRIu8, version);
                break;
        }

        // Call this macro to skip translate, not return translation data back to DMC client.
        SKIP_TRANSLATE();
    }
}