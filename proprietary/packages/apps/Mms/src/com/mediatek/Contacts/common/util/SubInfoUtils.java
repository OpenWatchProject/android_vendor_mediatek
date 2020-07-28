/*
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2011. All rights reserved.
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

package com.mediatek.contacts.common.util;

import android.content.Context;
import android.graphics.Bitmap;
import android.Manifest.permission;
import android.telephony.SubscriptionInfo;
import android.telephony.SubscriptionManager;
import android.util.Log;

import com.mediatek.mms.util.PermissionCheckUtil;

import java.util.List;

public class SubInfoUtils {
    private static final String TAG = "SubInfoUtils";

    public static final String MTK_PHONE_BOOK_SERVICE_NAME = "mtksimphonebook";

    private static final String[] REQUIRED_PERMISSIONS = new String[]{
        permission.READ_CONTACTS,
        permission.READ_PHONE_STATE
    };

    public static List<SubscriptionInfo> getActivatedSubInfoList() {
        Context context = GlobalEnv.getApplicationContext();
        if (PermissionCheckUtil.checkPermissions(context, REQUIRED_PERMISSIONS)) {
            return SubscriptionManager.from(context).getActiveSubscriptionInfoList();
        } else {
            Log.w(TAG, "getActivatedSubInfoList has no basic permissions!");
            return null;
        }
    }

    public static SubscriptionInfo getSubInfoUsingSubId(int subId) {
        if (!checkSubscriber(subId)) {
            return null;
        }
        List<SubscriptionInfo> subscriptionInfoList = getActivatedSubInfoList();
        if (subscriptionInfoList != null && subscriptionInfoList.size() > 0) {
            for (SubscriptionInfo subscriptionInfo : subscriptionInfoList) {
                if (subscriptionInfo.getSubscriptionId() == subId) {
                    return subscriptionInfo;
                }
            }
        }
        return null;
    }

    public static int[] getActiveSubscriptionIdList() {
        Context context = GlobalEnv.getApplicationContext();
        // If has no basic permission of Phone, it shouldn't call getActiveSubscriptionIdList.
        if (PermissionCheckUtil.checkPermissions(context, REQUIRED_PERMISSIONS)) {
            return SubscriptionManager.from(context).getActiveSubscriptionIdList();
        } else {
            Log.w(TAG, "getActiveSubscriptionIdList has no basic permissions!");
            return null;
        }
    }

    public static boolean iconTintChange(int iconTint, int subId) {
        Log.d(TAG, "[iconTintChange] iconTint = " + iconTint + ",subId = " + subId);
        boolean isChanged = true;
        List<SubscriptionInfo> activeList = getActivatedSubInfoList();
        if (activeList == null) {
            isChanged = false;
            return isChanged;
        }
        // TODO:: Check here,it may cause performance poor than L0
        for (SubscriptionInfo subInfo : activeList) {
            if (subInfo.getSubscriptionId() == subId && iconTint == subInfo.getIconTint()) {
                isChanged = false;
                break;
            }
        }
        return isChanged;
    }

    public static int getColorUsingSubId(int subId) {
        if (!checkSubscriber(subId)) {
            return -1;
        }
        SubscriptionInfo subscriptionInfo = getSubInfoUsingSubId(subId);
        return subscriptionInfo == null ? -1 : subscriptionInfo.getIconTint();
    }

    public static boolean checkSubscriber(int subId) {
        if (subId < 1) {
            Log.w(TAG, "[checkSubscriber], invalid subId: " + subId);
            return false;
        }
        return true;
    }

    public static Bitmap getIconBitmap(int subId) {
        if (!checkSubscriber(subId)) {
            return null;
        }
        ///M-TODO: temp mark for debug
        //MtkSubscriptionInfo info = MtkSubscriptionManager.getSubscriptionInfo(subId);
        //return info == null ? null : info.createIconBitmap(GlobalEnv
        //     .getApplicationContext(), -1, ContactsSystemProperties.MTK_GEMINI_SUPPORT);
        SubscriptionInfo subscriptionInfo = getSubInfoUsingSubId(subId);
        return subscriptionInfo == null ? null : subscriptionInfo.createIconBitmap(
                                         GlobalEnv.getApplicationContext());
    }
}
