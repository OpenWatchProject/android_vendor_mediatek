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

import android.content.Context;
import android.database.Cursor;
import android.database.sqlite.SqliteWrapper;
import android.net.NetworkUtils;
import android.net.Uri;
import android.provider.Telephony;
import android.text.TextUtils;

import com.android.internal.telephony.PhoneConstants;
import com.mediatek.omadm.FotaException;

import java.net.URI;
import java.net.URISyntaxException;
import android.util.Log;


public class FotaApnSettings {

    private final String mSrvUrl;
    private final String mProxyAddr;
    private final int mProxyPort;
    private final String mText;
    private static final String mAdmApnType = PhoneConstants.APN_TYPE_FOTA;

    private static final int FIELD_TYPE = 0;
    private static final int FIELD_PROXY = 12;
    private static final int FIELD_PORT = 13;
    private static final int FIELD_SERVER = 14;

    private static final String[] APNPROJ = {
            Telephony.Carriers.TYPE,
            Telephony.Carriers.MMSC,
            Telephony.Carriers.MMSPROXY,
            Telephony.Carriers.MMSPORT,
            Telephony.Carriers.NAME,
            Telephony.Carriers.APN,
            Telephony.Carriers.BEARER_BITMASK,
            Telephony.Carriers.PROTOCOL,
            Telephony.Carriers.ROAMING_PROTOCOL,
            Telephony.Carriers.AUTH_TYPE,
            Telephony.Carriers.MVNO_TYPE,
            Telephony.Carriers.MVNO_MATCH_DATA,
            Telephony.Carriers.PROXY,
            Telephony.Carriers.PORT,
            Telephony.Carriers.SERVER,
            Telephony.Carriers.USER,
            Telephony.Carriers.PASSWORD,
    };

    private static String getText(Cursor cursor) {
        final StringBuilder str = new StringBuilder();
        str.append("APN [");

        for (int i = 0; i < cursor.getColumnCount(); i++) {
            final String name = cursor.getColumnName(i);
            final String val = cursor.getString(i);

            if (TextUtils.isEmpty(val)) {
                continue;
            }
            if (i > 0) {
                str.append(' ');
            }
            str.append(name).append('=').append(val);
        }

        str.append("]");
        return str.toString();
    }

    private static boolean checkApnType(String types) {
        if (TextUtils.isEmpty(types)) {
            return false;
        }

        for (String type : types.split(",")) {
            type = type.trim();
            if (type.equals(mAdmApnType)) {
                return true;
            }
        }

        return false;
    }

    private static String valueNullCheck(String value) {
        return value != null ? value.trim() : null;
    }

    public static FotaApnSettings load(Context context, String apnName, int subId,
                                       String logTag, boolean chCurr)
            throws FotaException {
        Log.i(logTag, "Load APN name = " + apnName);

        final StringBuilder selBuilder = new StringBuilder();
        String[] selArgs = null;

        if (chCurr) {
            selBuilder.append(Telephony.Carriers.CURRENT).append(" IS NOT NULL");
        }

        apnName = valueNullCheck(apnName);
        if (!TextUtils.isEmpty(apnName)) {
            if (selBuilder.length() > 0) {
                selBuilder.append(" AND ");
            }
            selBuilder.append(Telephony.Carriers.APN).append("=?");
            selArgs = new String[] { apnName };
        }

        Cursor cur = null;
        try {
            cur = SqliteWrapper.query(
                    context,
                    context.getContentResolver(),
                    Uri.withAppendedPath(Telephony.Carriers.CONTENT_URI, "/subId/" + subId),
                    APNPROJ,
                    selBuilder.toString(),
                    selArgs,
                    null);

            if (cur != null) {
                String ser_url = null;
                String proxy_addr = null;
                int proxy_port = 80;

                while (cur.moveToNext()) {
                    if (checkApnType(cur.getString(FIELD_TYPE))) {
                        ser_url = valueNullCheck(cur.getString(FIELD_SERVER));
                        if (!TextUtils.isEmpty(ser_url)) {
                            ser_url = NetworkUtils.trimV4AddrZeros(ser_url);
                            try {
                                new URI(ser_url);
                            } catch (URISyntaxException e) {
                                throw new FotaException("Invalid Server url " + ser_url);
                            }
                        }
                        proxy_addr = valueNullCheck(cur.getString(FIELD_PROXY));
                        if (!TextUtils.isEmpty(proxy_addr)) {
                            proxy_addr = NetworkUtils.trimV4AddrZeros(proxy_addr);
                            final String portString =
                                    valueNullCheck(cur.getString(FIELD_PORT));
                            if (!TextUtils.isEmpty(portString)) {
                                try {
                                    proxy_port = Integer.parseInt(portString);
                                } catch (NumberFormatException e) {
                                    Log.e(logTag, "Invalid port " + portString + ", use 80");
                                }
                            }
                        }
                        return new FotaApnSettings(ser_url, proxy_addr, proxy_port, getText(cur));
                    }
                }

            }
        } finally {
            if (cur != null) {
                cur.close();
            }
        }
        throw new FotaException("Can not find valid APN");
    }

    public FotaApnSettings(String srvUrl, String proxyAddr, int proxyPort, String debugText) {
        mSrvUrl = srvUrl;
        mProxyAddr = proxyAddr;
        mProxyPort = proxyPort;
        mText = debugText;
   }

    public String toString() {
        return mText;
    }
}
