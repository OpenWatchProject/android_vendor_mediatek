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

/*****************************************************************************
 * Include
 *****************************************************************************/
#include "rfx_properties.h"
#include "RfxRilUtils.h"
#include <Vector.h>
#include <RfxLog.h>
#include <string.h>
#include "utils/Mutex.h"

using ::android::Mutex;
using ::android::Vector;
using ::android::String8;


/*****************************************************************************
 * Implementation
 *****************************************************************************/

Mutex s_gt_data_mutex;

class GTDataEntry {
public:
    GTDataEntry(const char * _key, const char * _value) :
        key(_key), value(_value) {}

    String8 key;
    String8 value;
};

static Vector<GTDataEntry *> *s_gt_property_info = new Vector<GTDataEntry*>();

static bool rfx_get_value_from_GTPro(const char *key, char *value) {
    Mutex::Autolock autoLock(s_gt_data_mutex);
    Vector<GTDataEntry* >::iterator it;
    if (key == NULL || s_gt_property_info == NULL) {
        return false;
    }
    for (it = s_gt_property_info->begin(); it != s_gt_property_info->end(); it++) {
        if ((*it)->key == String8(key)) {
            strncpy(value, ((*it)->value).string(),
                    strlen(((*it)->value).string()));
            return true;
        }
    }
    return false;
}

int rfx_property_get(const char *key, char *value, const char *default_value)
{
    if (RfxRilUtils::getRilRunMode() == RilRunMode::RIL_RUN_MODE_MOCK) {
        if (!rfx_get_value_from_GTPro(key, value)) {
            if (mtk_property_get(key, value, default_value) > 0) {
                rfx_property_set(key, value);
            } else {
                strncpy(value, default_value, strlen(default_value));
            }
        }
        return 1;
    } else {
        return mtk_property_get(key, value, default_value);
    }
}


int rfx_property_set(const char *key, const char *value)
{
    if (RfxRilUtils::getRilRunMode() == RilRunMode::RIL_RUN_MODE_MOCK) {
        Mutex::Autolock autoLock(s_gt_data_mutex);
        if(s_gt_property_info!=NULL) {
            Vector<GTDataEntry* >::iterator it;
            for (it = s_gt_property_info->begin(); it != s_gt_property_info->end(); it++) {
                if ((*it)->key == String8(key)) {
                    ((*it)->value).setTo(value);
                    return 1;
                }
            }
            GTDataEntry* gtDataEntry = new GTDataEntry(key, value);
            s_gt_property_info->push_back(gtDataEntry);
            return 1;
        }
    } else {
        return mtk_property_set(key, value);
    }
    return false;
}

void rfx_property_release_info() {
    if (RfxRilUtils::getRilRunMode() == RilRunMode::RIL_RUN_MODE_MOCK) {
        Vector<GTDataEntry* >::iterator it;
        for (it = s_gt_property_info->begin(); it != s_gt_property_info->end();) {
            delete(*it);
            it = s_gt_property_info->erase(it);
        }
    }
}
