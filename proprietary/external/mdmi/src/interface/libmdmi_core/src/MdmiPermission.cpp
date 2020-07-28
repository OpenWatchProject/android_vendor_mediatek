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

#include "MdmiPermission.h"
#include "Mdmi_utils.h"

namespace android {

const static char* TAG = "MDMI-Permission";

/*
// TODO: OEM required to defined their Linux group ID and
// Update this table
static const struct PermissionGidInfo_t sMdmiGids[] = {
    {MDMI_LIB_DEBUG, AID_VENDOR_VZW_DEBUG},
    {MDMI_LIB_LTE, AID_VENDOR_VZW_LTE},
    {MDMI_LIB_GSM,  AID_VENDOR_VZW_GSM},
    {MDMI_LIB_UMTS, AID_VENDOR_VZW_UMTS},
    {MDMI_LIB_WIFI, AID_VENDOR_VZW_WIFI},
    {MDMI_LIB_IMS, AID_VENDOR_VZW_IMS},
    {MDMI_LIB_COMMANDS, AID_VENDOR_VZW_COMMANDS},
    {MDMI_LIB_WCDMA, AID_VENDOR_VZW_WCDMA},
    {MDMI_LIB_HSUPA, AID_VENDOR_VZW_HSUPA},
    {MDMI_LIB_HSDPA, AID_VENDOR_VZW_HSDPA},
    {MDMI_LIB_EMBMS, AID_VENDOR_VZW_LTE_EMBMS}
};
*/
static const PermissionGidInfo_t sMdmiGids[] = {
    {MDMI_LIB_DEBUG, AID_DIAG},
    {MDMI_LIB_LTE, AID_DIAG},
    {MDMI_LIB_GSM,  AID_DIAG},
    {MDMI_LIB_UMTS, AID_DIAG},
    {MDMI_LIB_WIFI, AID_DIAG},
    {MDMI_LIB_IMS, AID_DIAG},
    {MDMI_LIB_COMMANDS, AID_DIAG},
    {MDMI_LIB_WCDMA, AID_DIAG},
    {MDMI_LIB_HSUPA, AID_DIAG},
    {MDMI_LIB_HSDPA, AID_DIAG},
    {MDMI_LIB_EMBMS, AID_DIAG}
};

static bool isValidAid(MDMI_LIB_TYPE libType, gid_t currentAid) {
    if (libType < MDMI_LIB_MAX) {
        if (currentAid == sMdmiGids[libType].aid) {
            MDMI_LOGD(TAG, "Grant MDMI permission for MDMI_LIB_TYPE(%d) with AID = %d", libType, currentAid);
            return true;
        }
    }
    return false;
}

/* Determine the gid credentials of the caller */
bool checkPermission(MDMI_LIB_TYPE libType) {
    int i = getgroups((size_t) 0, NULL);
    if (i > 0) {
        gid_t list[i];
        getgroups(i, list);
        while (--i >= 0) {
            if (isValidAid(libType, list[i])) {
                return true;
            }
        }
    }
    MDMI_LOGE(TAG, "Denied MDMI permission for MDMI_LIB_TYPE(%d)", libType);
    return false;
}

}; // namespace android
