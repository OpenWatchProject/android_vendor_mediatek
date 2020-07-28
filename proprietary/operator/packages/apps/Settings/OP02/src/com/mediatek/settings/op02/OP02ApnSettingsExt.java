/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2017. All rights reserved.
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

package com.mediatek.settings.op02;

import android.content.ContentUris;
import android.content.ContentValues;
import android.content.Context;
import android.database.Cursor;
import android.net.Uri;
import android.provider.Telephony;
import androidx.preference.PreferenceScreen;
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;
import android.util.Log;

import com.android.internal.telephony.PhoneConstants;

import com.mediatek.settings.ext.DefaultApnSettingsExt;

/**
 * CU feature , CU apn settings can not edit.
 */
public class OP02ApnSettingsExt extends DefaultApnSettingsExt {

    private static final String TAG = "OP02ApnSettingsExt";
    private static final String CU_NUMERIC_1 = "46001";
    private static final String CU_NUMERIC_2 = "46009";
    /// CU sim in Hongkong
    private static final String CU_NUMERIC_3 = "45407";

    // -1 stands for the apn inserted fail
    private static final long APN_NO_UPDATE = -1;
    private static final int SOURCE_TYPE_DEFAULT = 0;
    private Context mContext;
    public OP02ApnSettingsExt(Context context) {
        mContext = context;
    }

    /**
     * CU spec request: 1.default cu apn can not be edit.
     * @param subid sub id
     * @param type unused in OP02 method
     * @param apn unused in OP02 method
     * @param numeric means simoperator,mcc+mnc read form sim card
     * @param sourcetype means default apn or user add apn
     * @return false means cu apn can not allow to be edit
     */
    @Override
    public boolean isAllowEditPresetApn(int subId, String type, String apn, int sourcetype) {
        final TelephonyManager tm
            = (TelephonyManager) mContext.getSystemService(Context.TELEPHONY_SERVICE);

        String numeric = tm.getSimOperator(subId);

        return (!isCUNumeric(numeric) || sourcetype != 0);
    }

    private boolean isCUNumeric(String numeric) {
        if (numeric != null) {
            return numeric.equals(CU_NUMERIC_1)
                || numeric.equals(CU_NUMERIC_2)
                || numeric.equals(CU_NUMERIC_3);
        } else {
            return false;
        }
    }

    /**
     * Check if the same name(apn item)exists, if it exists, replace it.
     * @param defaultReplacedNum the default replace number.
     * @param context ApnSettings context
     * @param uri to access database
     * @param apn profile apn
     * @param name profile carrier name
     * @param values new profile values to update
     * @param numeric selected numeric
     * @return the replaced profile id
     */
    @Override
    public long replaceApn(long defaultReplaceNum, Context context, Uri uri, String apn, String name,
            ContentValues values, String numeric) {
        long numReplaced = APN_NO_UPDATE;
        Log.d("@M_" + TAG, "params: apn = " + apn + " numeric = " + numeric);
        if (isCUNumeric(numeric)) {
            String where = "numeric=\"" + numeric + "\"";
            Cursor cursor = null;
            try {
                cursor = context.getContentResolver().query(uri,
                        new String[] {  Telephony.Carriers._ID, Telephony.Carriers.APN },
                        where, null, Telephony.Carriers.DEFAULT_SORT_ORDER);
                if (cursor == null || cursor.getCount() == 0) {
                    Log.d("@M_" + TAG, "cu card ,cursor is null ,return");
                    return APN_NO_UPDATE;
                }
                cursor.moveToFirst();
                while (!cursor.isAfterLast()) {
                    Log.d("@M_" + TAG, "apn = " + apn + " getApn = " + cursor.getString(1));
                    if (apn.equals(cursor.getString(1))) {
                        numReplaced = Integer.parseInt(cursor.getString(0));
                        Uri newUri = ContentUris.withAppendedId(uri, numReplaced);
                        context.getContentResolver().update(newUri, values, null, null);
                        break;
                    }
                    cursor.moveToNext();
                }
            } finally {
                if (cursor != null) {
                    cursor.close();
                }
            }
        } else {
            numReplaced = super.replaceApn(defaultReplaceNum, context, uri, apn, name, values,
                    numeric);
        }
        return numReplaced;
    }

    /**
     * if sort APN by the name.
     * @param order sort by the name.
     * @return the sort string.
     */
    @Override
    public String getApnSortOrder(String order) {
        return null;
    }

    /**
     * Update the customized status(enable , disable).
     * Called at update screen status
     * @param subId sub id
     * @param root PPP's parent
     */
    @Override
    public void updateFieldsStatus(int subId, int sourceType, PreferenceScreen root, String apnType) {
        TelephonyManager tm =
            (TelephonyManager) root.getContext().getSystemService(Context.TELEPHONY_SERVICE);
        String simMccMnc = tm.getSimOperator(subId);
        Log.d(TAG, "updateFieldsStatus subId =" + subId
                + " sourceType = " + sourceType
                + " simMccMnc = " + simMccMnc);
        if (sourceType == SOURCE_TYPE_DEFAULT && isCUNumeric(simMccMnc)) {
            root.setEnabled(false);
        }
    }

}
