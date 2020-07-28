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
package com.mediatek.contacts.simservice;

import android.content.Context;
import android.content.Intent;
import android.database.Cursor;
import android.os.UserHandle;
import android.provider.ContactsContract.Groups;
import android.provider.ContactsContract.RawContacts;
import android.provider.Settings;
import android.telephony.SubscriptionInfo;
import android.text.TextUtils;

import com.android.internal.telephony.PhoneConstants;

import com.mediatek.contacts.simcontact.SimCardUtils;
import com.mediatek.contacts.simcontact.SubInfoUtils;
import com.mediatek.contacts.util.Log;
import com.mediatek.provider.MtkContactsContract;

import java.util.ArrayList;
import java.util.List;

public class SimServiceUtils {
    private static final String TAG = "SIMServiceUtils";

    public static final String SERVICE_SUBSCRIPTION_KEY = "subscription_key";
    public static final String SERVICE_SLOT_KEY = "which_slot";
    public static final String SERVICE_WORK_TYPE = "work_type";
    private static final String IMPORT_REMOVE_RUNNING = "import_remove_running";

    public static final int SERVICE_WORK_NONE = 0;
    public static final int SERVICE_WORK_EDIT = 1;
    public static final int SERVICE_WORK_DELETE = 2;
    public static final int SERVICE_WORK_GROUP =3;
    public static final int SERVICE_WORK_UNKNOWN = -1;
    public static final int SERVICE_IDLE = 0;
    public static final int SERVICE_FORCE_REMOVE_SUB_ID = -20;

    public static final int SERVICE_DELETE_CONTACTS = 1;
    public static final int SERVICE_QUERY_SIM = 2;
    public static final int SERVICE_IMPORT_CONTACTS = 3;

    public static final String KEY_SIM_DATA = "simData";
    public static final String KEY_OLD_SIM_DATA = "oldsimData";

    public static class ServiceWorkData {
        public int mSubId = -1;
        public String mSimType = SimCardUtils.SimType.SIM_TYPE_UNKNOWN_TAG;
        public Cursor mSimCursor = null;

        ServiceWorkData() {
        }

        ServiceWorkData(int subId, String simType, Cursor simCursor) {
            mSubId = subId;
            mSimType = simType;
            mSimCursor = simCursor;
        }
    }

    /**
     * check PhoneBook State is ready if ready, then return true.
     *
     * @param subId
     * @return
     */
    static boolean checkPhoneBookState(final int subId) {
        return SimCardUtils.isPhoneBookReady(subId);
    }

    public static boolean isServiceRunning(Context context, int subId) {
        String isRunning = Settings.System.getString(
                context.getContentResolver(), IMPORT_REMOVE_RUNNING);
        return "true".equals(isRunning);
    }

}
