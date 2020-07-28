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
 * MediaTek Inc. (C) 2010. All rights reserved.
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

package com.mediatek.engineermode.bip;

import android.app.Activity;
import android.content.ContentValues;
import android.database.Cursor;
import android.net.Uri;
import android.os.Bundle;
import android.provider.Telephony;
import android.telephony.SubscriptionManager;
import android.view.View;
import android.widget.Button;
import android.widget.RadioButton;
import android.widget.Toast;

import com.mediatek.engineermode.Elog;
import com.mediatek.engineermode.R;

/**
 * Description: To Add BIP test APN.
 *
 */
public class BipAddApnActivity extends Activity {
    private static final String TAG = "BipAddApn";

    private RadioButton mRadioBtnAddDefaultBip;
    private RadioButton mRadioBtnAddBip;
    private RadioButton mRadioBtnRemoveAll;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        this.setContentView(R.layout.bip_add_apn_activity);

        mRadioBtnAddDefaultBip = (RadioButton) findViewById(R.id.bip_add_default_bip_radio);
        mRadioBtnAddBip = (RadioButton) findViewById(R.id.bip_add_bip_radio);
        mRadioBtnRemoveAll = (RadioButton) findViewById(R.id.bip_remove_all_added_radio);
        Button buttonOk = (Button) findViewById(R.id.bip_set_button);

        buttonOk.setOnClickListener(new View.OnClickListener() {

            public void onClick(View v) {
                if (mRadioBtnAddDefaultBip.isChecked()) {
                    setApnParams("Test", "default,bip");
                    Elog.i(TAG, "Add default+bip APN");
                } else if (mRadioBtnAddBip.isChecked()) {
                    setApnParams("Test", "bip");
                    Elog.i(TAG, "Add bip APN");
                } else if (mRadioBtnRemoveAll.isChecked()) {
                    deleteApnParams();
                    Elog.i(TAG, "Remove all added APN");
                } else {
                    Elog.e(TAG, "unknown selection");
                }
            }
        });
    }

    private static final String BIP_NAME = "TestSIM";

    private Uri getUri(Uri uri, int slodId) {
        int subId[] = SubscriptionManager.getSubId(slodId);

        if (SubscriptionManager.isValidSubscriptionId(subId[0])) {
            return Uri.withAppendedPath(uri, "/subId/" + subId[0]);
        } else {
            Elog.e(TAG, "BM-getUri: invalid subId.");
            return null;
        }
    }

    private void setApnParams(String apn, String apnType) {
        Elog.e(TAG, "BM-setApnParams: enter");
        if (apn == null) {
            Elog.e(TAG, "BM-setApnParams: No apn parameters");
            return;
        }

        Uri uri = getUri(Telephony.Carriers.CONTENT_URI, 0);
        String numeric = "00101";
        String mcc = "001";
        String mnc = "01";

        if (uri == null) {
            Toast.makeText(BipAddApnActivity.this, "uri null",
                                Toast.LENGTH_SHORT).show();
            return;
        }

        if (numeric != null && numeric.length() >= 4) {
            Cursor cursor = null;
            mcc = numeric.substring(0, 3);
            mnc = numeric.substring(3);
            Elog.e(TAG, "BM-setApnParams: apn = " + apn + "mcc = " + mcc + ", mnc = " + mnc);
//            String selection = Telephony.Carriers.APN + " = '" + apn + "'" +
//                    " and " + Telephony.Carriers.NUMERIC + " = '" + mcc + mnc + "'";

            String selection = Telephony.Carriers.APN + " = '" + apn + "'" +
                    " AND " + Telephony.Carriers.MCC + " = '" + mcc + "'" +
                    " AND " + Telephony.Carriers.MNC + " = '" + mnc + "'";

            cursor = getContentResolver().query(
                    uri, null, selection, null, null);

            if (cursor != null) {
                ContentValues values = new ContentValues();
                values.put(Telephony.Carriers.NAME, BIP_NAME);
                values.put(Telephony.Carriers.APN, apn);
                values.put(Telephony.Carriers.TYPE, apnType);
                values.put(Telephony.Carriers.MCC, mcc);
                values.put(Telephony.Carriers.MNC, mnc);
                values.put(Telephony.Carriers.NUMERIC, mcc + mnc);

                if (cursor.getCount() == 0) {
                    // int updateResult = getContentResolver().update(
                    // uri, values, selection, selectionArgs);
                    Elog.e(TAG, "BM-setApnParams: =>insert()");
                    Uri newRow = this.getContentResolver().insert(uri, values);
                    if (newRow != null) {
                        Toast.makeText(BipAddApnActivity.this, "Added APN: type=" + apnType
                                + ", URI: " + newRow,
                                Toast.LENGTH_SHORT).show();
                        Elog.e(TAG, "insert a new record into db");
                    } else {
                        Toast.makeText(BipAddApnActivity.this, "Failed to Add APN: " + apnType,
                                Toast.LENGTH_SHORT).show();
                        Elog.e(TAG, "Fail to insert apn params into db");
                    }
                } else {
                    Toast.makeText(BipAddApnActivity.this, "Test SIM APN already exist",
                                Toast.LENGTH_SHORT).show();
                    Elog.e(TAG, "BM-setApnParams: do not update one record");
                }
                cursor.close();
            }
        }
        Elog.e(TAG, "BM-setApnParams: exit");
    }

    private void deleteApnParams() {
        Uri uri = getUri(Telephony.Carriers.CONTENT_URI, 0);

        Elog.e(TAG, "BM-deleteApnParams: enter. ");

        if (uri == null) {
            Toast.makeText(BipAddApnActivity.this, "uri null",
                                Toast.LENGTH_SHORT).show();
            return;
        }
        String selection = "name = '" + BIP_NAME + "'";
        int rows = getContentResolver().delete(uri, selection, null);
        Elog.e(TAG, "BM-deleteApnParams:[" + rows + "] end");

        Toast.makeText(BipAddApnActivity.this, "Test SIM APN removed: " + rows,
                                Toast.LENGTH_SHORT).show();
    }
}
