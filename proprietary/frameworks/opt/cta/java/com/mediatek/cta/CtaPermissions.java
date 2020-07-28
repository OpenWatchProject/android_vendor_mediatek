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
 * MediaTek Inc. (C) 2017. All rights reserved.
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

package com.mediatek.cta;

import java.util.ArrayList;
import java.util.List;
import java.util.Set;

import android.Manifest;
import android.util.ArrayMap;
import android.util.ArraySet;

/**
 *  @hide
 */
class CtaPermissions {

    static final ArrayMap<String, List<String>> MAP =
            new ArrayMap<String, List<String>>();
    /** Mapping permission -> group for all dangerous platform permissions of CTA */
    static final ArrayMap<String, String> CTA_PLATFORM_PERMISSIONS = new ArrayMap<>();
    static final Set<String> CTA_ONLY_PERMISSIONS = new ArraySet<>();
    static final Set<String> CTA_MONITOR_PERMISSIONS = new ArraySet<>();
    static final Set<String> CTA_ADDED_PERMISSION_GROUPS = new ArraySet<>();

    static {
        // init permission mapping between AOSP and CTA
        List<String> subs;
        subs = new ArrayList<String>();
        // Here we think if the app need CALL_PHONE permission, it will also need
        // permission:CTA_CONFERENCE_CALL and CTA_CALL_FORWARD.
        subs.add(com.mediatek.Manifest.permission.CTA_CONFERENCE_CALL);
        subs.add(com.mediatek.Manifest.permission.CTA_CALL_FORWARD);
        MAP.put(Manifest.permission.CALL_PHONE, subs);

        CTA_MONITOR_PERMISSIONS.add(Manifest.permission.READ_CALENDAR);
        CTA_MONITOR_PERMISSIONS.add(Manifest.permission.WRITE_CALENDAR);
        CTA_MONITOR_PERMISSIONS.add(Manifest.permission.CAMERA);
        CTA_MONITOR_PERMISSIONS.add(Manifest.permission.READ_CONTACTS);
        CTA_MONITOR_PERMISSIONS.add(Manifest.permission.WRITE_CONTACTS);
        CTA_MONITOR_PERMISSIONS.add(Manifest.permission.ACCESS_FINE_LOCATION);
        CTA_MONITOR_PERMISSIONS.add(Manifest.permission.ACCESS_COARSE_LOCATION);
        CTA_MONITOR_PERMISSIONS.add(Manifest.permission.RECORD_AUDIO);
        CTA_MONITOR_PERMISSIONS.add(Manifest.permission.READ_PHONE_STATE);
        CTA_MONITOR_PERMISSIONS.add(Manifest.permission.CALL_PHONE);
        CTA_MONITOR_PERMISSIONS.add(Manifest.permission.READ_CALL_LOG);
        CTA_MONITOR_PERMISSIONS.add(Manifest.permission.WRITE_CALL_LOG);
        CTA_MONITOR_PERMISSIONS.add(Manifest.permission.SEND_SMS);
        CTA_MONITOR_PERMISSIONS.add(Manifest.permission.RECEIVE_SMS);
        CTA_MONITOR_PERMISSIONS.add(Manifest.permission.READ_SMS);
        CTA_MONITOR_PERMISSIONS.add(Manifest.permission.RECEIVE_MMS);
        if (CtaUtils.isCtaSupported()) {
            for (String parentPerm : MAP.keySet()) {
                for (String subPerm : MAP.get(parentPerm)) {
                    CTA_ONLY_PERMISSIONS.add(subPerm);
                }
            }
            CTA_ONLY_PERMISSIONS.add(com.mediatek.Manifest.permission.CTA_SEND_EMAIL);
            CTA_ONLY_PERMISSIONS.add(com.mediatek.Manifest.permission.CTA_SEND_MMS);

            CTA_MONITOR_PERMISSIONS.add(com.mediatek.Manifest.permission.CTA_SEND_EMAIL);
            CTA_MONITOR_PERMISSIONS.add(com.mediatek.Manifest.permission.CTA_SEND_MMS);
            CTA_MONITOR_PERMISSIONS.add(com.mediatek.Manifest.permission.CTA_CALL_FORWARD);
            CTA_MONITOR_PERMISSIONS.add(com.mediatek.Manifest.permission.CTA_CONFERENCE_CALL);

            CTA_ADDED_PERMISSION_GROUPS.add(com.mediatek.Manifest.permission_group.EMAIL);

            CTA_PLATFORM_PERMISSIONS.put(com.mediatek.Manifest.permission.CTA_CONFERENCE_CALL,
                    Manifest.permission_group.PHONE);
            CTA_PLATFORM_PERMISSIONS.put(com.mediatek.Manifest.permission.CTA_CALL_FORWARD,
                    Manifest.permission_group.PHONE);
            CTA_PLATFORM_PERMISSIONS.put(com.mediatek.Manifest.permission.CTA_SEND_EMAIL,
                    com.mediatek.Manifest.permission_group.EMAIL);
            CTA_PLATFORM_PERMISSIONS.put(com.mediatek.Manifest.permission.CTA_SEND_MMS,
                    Manifest.permission_group.SMS);
        }
    }

}
