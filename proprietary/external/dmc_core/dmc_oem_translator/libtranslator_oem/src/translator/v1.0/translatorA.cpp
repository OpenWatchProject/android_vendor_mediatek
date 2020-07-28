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
 * EXAMPLE: A translator listen to a local event
 *
 * In this Translator example, translatorA was linked to OID 1.1.1 in OemKpiMap.cpp.
 * Macro TRANSLATE(OEM, translatorA) is to define translateEx() function, and
 * macro IF_MSG(KPI_SOURCE_TYPE_LOCAL, (MSGID)OEM_KPI_LOCAL_TYPE_event1) declare this translator
 * listen to a "local event". DMC will call translateEx() when the OID (1.1.1) is
 * being subscribed (one-shot)for "local event".
 * As the result, OEM_KPI_LOCAL_TYPE_event1 is not used for "local event".
 *
 * In this case, we read system property "ro.product.model" and write to output_payload buffer,
 * then this buffer will be returned to DMC and forward to DMC client.
 */

#include "TranslatorManager.h"
#include "LocalKpiDefs.h"
#include "log_utils.h"
#include <sys/system_properties.h>

static const char *TAG_KPI = "translatorA";

TRANSLATE(OEM, translatorA) {
    IF_MSG(KPI_SOURCE_TYPE_LOCAL, (MSGID)OEM_KPI_LOCAL_TYPE_event1) {
        uint8_t *output_payload = NULL;

        DMC_LOGD(TAG_KPI, "Enter translatorA");

        char buffer[PROP_VALUE_MAX] = {'\0'};
        __system_property_get("ro.product.model", buffer);
        unsigned int size = strlen(buffer);

        if (size <= 0) {
            // Call this macro to skip translate, not return translation data back to DMC client.
            SKIP_TRANSLATE();
            return;
        }

        output_payload = (uint8_t *)calloc(1, size + 1);
        if (output_payload == NULL) {
            SKIP_TRANSLATE();
            return;
        }

        snprintf((char *)output_payload, size + 1, "%s", buffer);

        // Output encoded data
        obj.SetByteArray(output_payload, size + 1);
    }
}