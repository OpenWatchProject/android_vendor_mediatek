/* Copyright Statement:
*
* This software/firmware and related documentation ("MediaTek Software") are
* protected under relevant copyright laws. The information contained herein
* is confidential and proprietary to MediaTek Inc. and/or its licensors.
* Without the prior written permission of MediaTek inc. and/or its licensors,
* any reproduction, modification, use or disclosure of MediaTek Software,
* and information contained herein, in whole or in part, shall be strictly prohibited.
*
* MediaTek Inc. (C) 2018. All rights reserved.
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

package com.mediatek.omadm;

import android.content.ContentValues;
import android.content.Context;
import android.database.Cursor;
import android.provider.Telephony;
import android.util.Slog;

import com.android.internal.telephony.TelephonyProperties;
import com.mediatek.omadm.PalConstDefs;

public class Apns {
    public static boolean DEBUG = true;

    private static final String TAG = Apns.class.getSimpleName();
    private static final int APN_CLASS_1 = 1;
    private static final int APN_CLASS_2 = 2;
    private static final int APN_CLASS_3 = 3;
    private static final int APN_CLASS_4 = 4;
    private static final int APN_CLASS_5 = 5;
    private static final String CLASS_TYPE_1 = "'%ims%'";
    private static final String CLASS_TYPE_2 = "'%fota%'";
    private static final String CLASS_TYPE_3 = "'%default%'";
    private static final String CLASS_TYPE_4 = "'%cbs,mms%'";
    private static final String CLASS_TYPE_5 = "'%800%'";
    private static final int IPV4_MASK = 0x00000001;
    private static final int IPV6_MASK = 0x00000002;
    private static final int IPV4V6_MASK = IPV4_MASK | IPV6_MASK;

    private static final String[] PROJECT_PROT = new String[]{
        Telephony.Carriers.PROTOCOL
    };

    private static final String[] CLASSES_TYPES = new String[] {
        CLASS_TYPE_1,
        CLASS_TYPE_2,
        CLASS_TYPE_3,
        CLASS_TYPE_4,
        CLASS_TYPE_5
    };

    private static final String[] PROJECT_APN = new String[]{
        Telephony.Carriers.APN
    };

    private static final String[] PROJECT_CIDS = new String[]{
        Telephony.Carriers._ID
    };

    private static final String[] PROJECT_CENABLED = new String[]{
        Telephony.Carriers.CARRIER_ENABLED
    };

    private static String createApnClsSelect(int apnCls) {
        if (apnCls < APN_CLASS_1 || apnCls > APN_CLASS_5) {
            return null;
        }

        String classTypes = CLASSES_TYPES[apnCls - 1];
        StringBuilder sel = new StringBuilder(Telephony.Carriers.TYPE);
        sel.append(" like ");
        sel.append(classTypes);

        if (apnCls < APN_CLASS_5) {
            sel.append(" and ");
            sel.append(Telephony.Carriers.NUMERIC);
            sel.append("='");
            sel.append(android.os.SystemProperties.get(
                    TelephonyProperties.PROPERTY_ICC_OPERATOR_NUMERIC, ""));
            sel.append("'");
        }

        return sel.toString();
    }

    public static int enable(Context ctx, int apnCls, int enable) {
        int err = PalConstDefs.RET_SUCC;
        String sel = createApnClsSelect(apnCls);

        if (DEBUG) Slog.d(TAG, "isEnabled.selection[" + apnCls + "] = " + sel);
        if (sel == null) {
            PalConstDefs.throwEcxeption(PalConstDefs.RET_ERR_ARGS);
        }

        ContentValues cv = new ContentValues(1);
        cv.put(Telephony.Carriers.CARRIER_ENABLED, enable);
        int numRows = ctx.getContentResolver().update(
                Telephony.Carriers.CONTENT_URI, cv,
                sel, null);

        if (numRows < 1) {
            err = PalConstDefs.RET_ERR_ARGS;
        }

        PalConstDefs.throwEcxeption(err);

        return err;
    }

    public static int isEnabled(Context ctx, int apnCls) {
        int err = PalConstDefs.RET_SUCC;
        int enabled = -1;
        String sel = createApnClsSelect(apnCls);

        if (sel == null) {
            err = PalConstDefs.RET_ERR_ARGS;
            Slog.d(TAG, "isEnabled.selection is NULL!!");
        }
        else{
            Slog.d(TAG, "isEnabled.selection[" + apnCls + "] = "+ sel);
        }

        Cursor c = ctx.getContentResolver().query(
                Telephony.Carriers.CONTENT_URI, PROJECT_CENABLED, sel, null, null);

        if (c != null) {
            if (c.moveToFirst()) {
                enabled = c.getInt(0);
            } else {
                err = PalConstDefs.RET_ERR_ARGS;
                Slog.d(TAG, "isEnabledValue got invalid content");
            }
            c.close();
        }

        PalConstDefs.throwEcxeption(err);
        return enabled;
    }

    public static int getId(Context ctx, int apnCls) {
        if (apnCls < APN_CLASS_1 || apnCls > APN_CLASS_5) {
            PalConstDefs.throwEcxeption( PalConstDefs.RET_ERR_ARGS);
        }

        if (APN_CLASS_5 == apnCls) {
            return apnCls + 1;
        }

        return apnCls;
    }

    public static int getIpVersions(Context ctx, int apnCls) {
        int ipVMask = 0;
        String ipVStr = null;
        int err = PalConstDefs.RET_SUCC;
        String sel = createApnClsSelect(apnCls);

        if (DEBUG) Slog.d(TAG, "getIpVersions.selection[" + apnCls + "] = " + sel);
        if (sel == null) {
            PalConstDefs.throwEcxeption(PalConstDefs.RET_ERR_ARGS);
        }

        Cursor c = ctx.getContentResolver().query(
                Telephony.Carriers.CONTENT_URI, PROJECT_PROT, sel, null, null);

        if (c != null) {
            if (c.moveToFirst()) {
                ipVStr = c.getString(0);
            } else {
                err = PalConstDefs.RET_ERR_ARGS;
            }
            c.close();
        }
        if (ipVStr != null) {
            if (DEBUG) Slog.d(TAG, "IPVersions[" + apnCls + "].string = " + ipVStr);
            if (ipVStr.equalsIgnoreCase("IP")) {
                ipVMask |= IPV4_MASK;
            } else {
                if (ipVStr.contains("V4")) {
                    ipVMask |= IPV4_MASK;
                }
                if (ipVStr.contains("V6")) {
                    ipVMask |= IPV6_MASK;
                }
            }
        } else if (DEBUG) {
            Slog.d(TAG, "IPVersions[" + apnCls + "] = null");
            err = PalConstDefs.RET_ERR_STATE;
        }

        PalConstDefs.throwEcxeption(err);
        return ipVMask;
    }

    public static String getName(Context ctx, int apnCls) {
        int err = 0;
        String name = null;
        String sel = createApnClsSelect(apnCls);
        if (DEBUG) Slog.d(TAG, "getName.selection [" + apnCls + "] = " + sel);
        if (sel == null) {
            PalConstDefs.throwEcxeption(PalConstDefs.RET_ERR_ARGS);
        }

        Cursor c = ctx.getContentResolver().query(
                Telephony.Carriers.CONTENT_URI, PROJECT_APN, sel, null, null);

        if (c != null) {
            if (c.moveToFirst()) {
                name = c.getString(0);
            } else {
                err = PalConstDefs.RET_ERR_STATE;
            }
            c.close();
        }

        PalConstDefs.throwEcxeption(err);
        if (DEBUG) Slog.d(TAG, "getName[" + apnCls + "] = " + name);
        return name;
    }

    public static int setIpVersions(Context ctx, int apnCls, int ipvFlags) {
        int err = 0;
        int ipVMask = 0;
        String sel = createApnClsSelect(apnCls);
        String ipvStr = null;

        if (sel == null) {
            PalConstDefs.throwEcxeption(PalConstDefs.RET_ERR_ARGS);
        }

        if (DEBUG) Slog.d(TAG, "getIpVersions.selection[" + apnCls + "] = " + sel);

        switch (ipvFlags) {
            case IPV4_MASK:
                ipvStr = "IP";
                break;

            case IPV6_MASK:
                ipvStr = "IPV6";
                break;

            case IPV4V6_MASK:
                ipvStr = "IPV4V6";
                break;

            default:
                break;
        }
        if (ipvStr != null) {
            ContentValues cv = new ContentValues(1);
            cv.put(Telephony.Carriers.PROTOCOL, ipvStr);
            int numRows = ctx.getContentResolver().update(
                    Telephony.Carriers.CONTENT_URI, cv, sel, null);
            if (numRows < 1){
                err = PalConstDefs.RET_ERR_STATE;
            }
        }

        PalConstDefs.throwEcxeption(err);
        return err;
    }

    public static int setName(Context ctx, int apnCls, String apnName) {
        int err = 0;
        String name = null;
        ContentValues cv = new ContentValues(1);
        cv.put(Telephony.Carriers.APN, apnName);
        String sel = createApnClsSelect(apnCls);

        if (DEBUG) Slog.d(TAG, "setName.selection[" + apnCls + "] = " + sel);
        if (sel == null) {
            PalConstDefs.throwEcxeption(PalConstDefs.RET_ERR_ARGS);
        }

        int rows = ctx.getContentResolver().update(
                Telephony.Carriers.CONTENT_URI, cv, sel, null);

        if (DEBUG) Slog.d(TAG, "setName[" + apnCls + "] = " + name + ", rows = " + rows);
        if (DEBUG) {
            apnName = getName(ctx, apnCls);
            Slog.d(TAG, "check.setName[" + apnCls + "] = " + apnName);
        }

        if (rows < 1) {
            err = PalConstDefs.RET_ERR_STATE;
        }

        PalConstDefs.throwEcxeption(err);
        return err;
    }

}
