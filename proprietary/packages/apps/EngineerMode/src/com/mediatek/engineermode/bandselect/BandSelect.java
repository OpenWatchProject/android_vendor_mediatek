/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
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

package com.mediatek.engineermode.bandselect;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.Dialog;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.SharedPreferences;
import android.os.AsyncResult;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.telephony.RadioAccessFamily;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.TableLayout;
import android.widget.TableRow;

import com.android.internal.telephony.PhoneConstants;
import com.mediatek.engineermode.Elog;
import com.mediatek.engineermode.EmUtils;
import com.mediatek.engineermode.FeatureSupport;
import com.mediatek.engineermode.ModemCategory;
import com.mediatek.engineermode.R;

import java.util.ArrayList;

public class BandSelect extends Activity implements OnClickListener {
    private static final String TAG = "BandSelect";
    private static final int WCDMA = 1;
    private static final int TDSCDMA = 2;
    private static final String PREF_FILE = "band_select_";
    private static final String PREF_KEYS[] = {"gsm", "umts", "lte_fdd", "lte_tdd",
            "lte_96", "lte_128", "lte_160", "lte_192",
            "lte_224", "lte_256", "cdma"};

    private static final int INDEX_GSM_BAND = 0;
    private static final int INDEX_UMTS_BAND = 1;
    private static final int INDEX_LTE_FDD_BAND = 2;
    private static final int INDEX_LTE_TDD_BAND = 3;
    private static final int INDEX_LTE_BAND_96 = 4;
    private static final int INDEX_LTE_BAND_128 = 5;
    private static final int INDEX_LTE_BAND_160 = 6;
    private static final int INDEX_LTE_BAND_192 = 7;
    private static final int INDEX_LTE_BAND_224 = 8;
    private static final int INDEX_LTE_BAND_256 = 9;
    private static final int INDEX_CDMA_BAND = 10;
    private static final int INDEX_BAND_MAX = 11;
    private static final int INDEX_NR_BAND = 12;
    private static final int NR_VALUES = 3;

    private static int mSimType;
    private final ArrayList<BandModeMap> mGsmModeArray = new ArrayList<BandModeMap>();
    private final ArrayList<BandModeMap> mCdmaModeArray = new ArrayList<BandModeMap>();
    private final ArrayList<BandModeMap> mNRModeArray = new ArrayList<BandModeMap>();
    private long mSetGsmValues[] = new long[INDEX_BAND_MAX];
    private long mSetCdmaValues = 0;
    private long mSetNRValues[] = new long[NR_VALUES];
    private long mCurrentGsmValues[] = new long[INDEX_BAND_MAX];
    private long mCurrentCdmaValues = 0;
    private long mCurrentNRValues[] = new long[NR_VALUES];
    private long mDefaultNRValues[] = new long[NR_VALUES];
    private boolean mIsNRValid = true;
    private Button mBtnSet;
    private Button mBtnReset;
    private boolean mIsThisAlive = true;
    private boolean mIsCdmaValid = true;
    private boolean mIsLteValid = true;
    private boolean mIsLteExtend = false;
    private final Handler mResponseHander = new Handler() {
        public void handleMessage(final Message msg) {
            if (!mIsThisAlive) {
                return;
            }
            AsyncResult asyncResult;
            switch (msg.what) {
                case BandModeContent.EVENT_QUERY_SUPPORTED_GSM:
                    asyncResult = (AsyncResult) msg.obj;
                    if (asyncResult.exception == null) {
                        showBandModeGsm(asyncResult, BandModeContent.EVENT_QUERY_SUPPORTED_GSM);
                    } else {
                        EmUtils.showToast("Query GSM Supported Mode Failed.");
                    }
                    break;
                case BandModeContent.EVENT_QUERY_CURRENT_GSM:
                    asyncResult = (AsyncResult) msg.obj;
                    if (asyncResult.exception == null) {
                        showBandModeGsm(asyncResult, BandModeContent.EVENT_QUERY_CURRENT_GSM);
                    } else {
                        EmUtils.showToast("Query GSM Current Mode Failed.");
                    }
                    break;
                case BandModeContent.EVENT_QUERY_SUPPORTED_NR:
                    asyncResult = (AsyncResult) msg.obj;
                    if (asyncResult.exception == null) {
                        showBandModeNR(asyncResult, BandModeContent.EVENT_QUERY_SUPPORTED_NR);
                    } else {
                        EmUtils.showToast("Query NR Supported Mode Failed.", true);
                    }
                    break;
                case BandModeContent.EVENT_QUERY_CURRENT_NR:
                    asyncResult = (AsyncResult) msg.obj;
                    if (asyncResult.exception == null) {
                        showBandModeNR(asyncResult, BandModeContent.EVENT_QUERY_CURRENT_NR);
                    } else {
                        EmUtils.showToast("Query NR Current Mode Failed.", true);
                    }
                    break;
                case BandModeContent.EVENT_QUERY_CURRENT_CDMA:
                    asyncResult = (AsyncResult) msg.obj;
                    if (asyncResult.exception == null) {
                        showBandModeCdma(asyncResult, BandModeContent.EVENT_QUERY_CURRENT_CDMA);
                    } else {
                        EmUtils.showToast("Query CDMA Current Mode Failed.", true);
                    }
                    break;
                case BandModeContent.EVENT_SET_CDMA:
                    asyncResult = (AsyncResult) msg.obj;
                    if (asyncResult.exception == null) {
                        EmUtils.showToast("set CDMA BandMode succeed", true);
                        Elog.v(TAG, "set CDMA BandMode succeed");
                    } else {
                        EmUtils.showToast("set CDMA BandMode failed", true);
                        Elog.v(TAG, "set CDMA BandMode failed");
                    }
                    if (FeatureSupport.is93Modem()) {
                        EmUtils.setAirplaneModeEnabled(true);
                        EmUtils.setAirplaneModeEnabled(false);
                    }
                    break;
                case BandModeContent.EVENT_SET_NR:
                    asyncResult = (AsyncResult) msg.obj;
                    if (asyncResult.exception == null) {
                        EmUtils.showToast("set NR BandMode succeed", true);
                        Elog.v(TAG, "set NR BandMode succeed");
                    } else {
                        EmUtils.showToast("set NR BandMode failed", true);
                        Elog.v(TAG, "set NR BandMode failed");
                    }
                    break;
                case BandModeContent.EVENT_SET_BUTTON:
                    asyncResult = (AsyncResult) msg.obj;
                    if (asyncResult.exception == null) {
                        EmUtils.showToast("set Gsm BandMode succeed", true);
                        Elog.v(TAG, "set Gsm BandMode succeed");
                        if (ModemCategory.CheckViceSimNRCapability(mSimType)) {
                            setBandModeNR(mSetNRValues);
                        }
                        if (ModemCategory.isCdma() && !FeatureSupport.is90Modem()) {
                            if (ModemCategory.CheckViceSimCdmaCapability(mSimType)) {
                                setBandModeCdma(mSetCdmaValues);
                            }
                        }
                    } else {
                        EmUtils.showToast("set Gsm BandMode failed", true);
                        Elog.v(TAG, "set Gsm BandMode failed");
                    }
                    break;
                default:
                    break;
            }
        }
    };


    private ArrayList<CheckBox> addCheckboxToTable(int tableResId, int arrayResId) {
        TableLayout table = (TableLayout) findViewById(tableResId);
        String[] labels = getResources().getStringArray(arrayResId);
        ArrayList<CheckBox> ret = new ArrayList<CheckBox>(labels.length);

        for (int i = 0; i < labels.length; i++) {
            TableRow row = new TableRow(this);
            CheckBox checkBox = new CheckBox(this);
            checkBox.setText(labels[i]);
            row.addView(checkBox);
            table.addView(row);
            ret.add(checkBox);
        }

        return ret;
    }

    private void initGsmArray() {
        findViewById(R.id.TableLayout_GSM).setVisibility(View.VISIBLE);
        ArrayList<CheckBox> checkBox = addCheckboxToTable(R.id.TableLayout_GSM,
                R.array.band_mode_gsm);
        for (int i = 0; i < checkBox.size(); i++) {
            mGsmModeArray.add(new BandModeMap(checkBox.get(i), INDEX_GSM_BAND,
                    BandModeContent.GSM_BAND_BIT[i]));
        }
    }

    private void initWcdmaArray() {
        findViewById(R.id.TableLayout_UTMS).setVisibility(View.VISIBLE);
        ArrayList<CheckBox> checkBox = addCheckboxToTable(R.id.TableLayout_UTMS,
                R.array.band_mode_wcdma);
        for (int i = 0; i < checkBox.size(); i++) {
            mGsmModeArray.add(new BandModeMap(checkBox.get(i), INDEX_UMTS_BAND, i));
        }
    }

    private void initTdscdmaArray() {
        findViewById(R.id.TableLayout_UTMS).setVisibility(View.VISIBLE);
        ArrayList<CheckBox> checkBox = addCheckboxToTable(R.id.TableLayout_UTMS,
                R.array.band_mode_tdscdma);
        for (int i = 0; i < checkBox.size(); i++) {
            mGsmModeArray.add(new BandModeMap(checkBox.get(i), INDEX_UMTS_BAND, i));
        }
    }

    private void initCdmaArray() {
        findViewById(R.id.TableLayout_CDMA).setVisibility(View.VISIBLE);
        ArrayList<CheckBox> checkBox = addCheckboxToTable(R.id.TableLayout_CDMA,
                R.array.band_mode_cdma);
        for (int i = 0; i < checkBox.size(); i++) {
            mCdmaModeArray.add(new BandModeMap(checkBox.get(i), INDEX_CDMA_BAND, i));
        }
    }

    private void initNRArray() {
        findViewById(R.id.TableLayout_NR).setVisibility(View.VISIBLE);
        ArrayList<CheckBox> checkBox = addCheckboxToTable(R.id.TableLayout_NR,
                R.array.band_mode_nr);
        for (int i = 0; i < checkBox.size(); i++) {
            mNRModeArray.add(new BandModeMap(checkBox.get(i), INDEX_NR_BAND, i));
        }
    }

    private void initLteArray() {
        findViewById(R.id.TableLayout_LTE).setVisibility(View.VISIBLE);
        ArrayList<CheckBox> checkBox =
                addCheckboxToTable(R.id.TableLayout_LTE, R.array.band_mode_lte_fdd);
        for (int i = 0; i < checkBox.size(); i++) {
            mGsmModeArray.add(new BandModeMap(checkBox.get(i), INDEX_LTE_FDD_BAND, i));
        }
        checkBox = addCheckboxToTable(R.id.TableLayout_LTE, R.array.band_mode_lte_tdd);
        for (int i = 0; i < checkBox.size(); i++) {
            mGsmModeArray.add(new BandModeMap(checkBox.get(i), INDEX_LTE_TDD_BAND, i));
        }
        checkBox = addCheckboxToTable(R.id.TableLayout_LTE, R.array.band_mode_lte_96);
        for (int i = 0; i < checkBox.size(); i++) {
            mGsmModeArray.add(new BandModeMap(checkBox.get(i), INDEX_LTE_BAND_96, i));
        }
      /*  checkBox = addCheckboxToTable(R.id.TableLayout_LTE, R.array.band_mode_lte_128);
        for (int i = 0; i < checkBox.size(); i++) {
            mGsmModeArray.add(new BandModeMap(checkBox.get(i), INDEX_LTE_BAND_128, i));
        }
        checkBox = addCheckboxToTable(R.id.TableLayout_LTE, R.array.band_mode_lte_160);
        for (int i = 0; i < checkBox.size(); i++) {
            mGsmModeArray.add(new BandModeMap(checkBox.get(i), INDEX_LTE_BAND_160, i));
        }
        checkBox = addCheckboxToTable(R.id.TableLayout_LTE, R.array.band_mode_lte_192);
        for (int i = 0; i < checkBox.size(); i++) {
            mGsmModeArray.add(new BandModeMap(checkBox.get(i), INDEX_LTE_BAND_192, i));
        }
        checkBox = addCheckboxToTable(R.id.TableLayout_LTE, R.array.band_mode_lte_224);
        for (int i = 0; i < checkBox.size(); i++) {
            mGsmModeArray.add(new BandModeMap(checkBox.get(i), INDEX_LTE_BAND_224, i));
        }
        checkBox = addCheckboxToTable(R.id.TableLayout_LTE, R.array.band_mode_lte_256);
        for (int i = 0; i < checkBox.size(); i++) {
            mGsmModeArray.add(new BandModeMap(checkBox.get(i), INDEX_LTE_BAND_256, i));
        }*/
    }

    @Override
    public void onCreate(final Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        Intent intent = getIntent();
        mSimType = intent.getIntExtra("mSimType", PhoneConstants.SIM_ID_1);
        int modemType = ModemCategory.getModemType();

        setContentView(R.layout.bandmodeselect);
        mBtnSet = (Button) findViewById(R.id.BandSel_Btn_Set);
        mBtnReset = (Button) findViewById(R.id.BandSel_Btn_Reset);
        mBtnSet.setOnClickListener(this);
        mBtnReset.setOnClickListener(this);

        initGsmArray();

        if (ModemCategory.isCapabilitySim(mSimType)) {
            if (modemType == TDSCDMA) {
                initTdscdmaArray();
                if (ModemCategory.isLteSupport()) {
                    initLteArray();
                }
            } else if (modemType == WCDMA) {
                initWcdmaArray();
                if (ModemCategory.isLteSupport()) {
                    initLteArray();
                }
            }
        } else {
            if (ModemCategory.checkViceSimCapability(mSimType, RadioAccessFamily.RAF_UMTS)) {
                initWcdmaArray();
            }
            if (ModemCategory.checkViceSimCapability(mSimType, RadioAccessFamily.RAF_LTE)
                    && ModemCategory.isLteSupport()) {
                    initLteArray();
            }
        }
        if (ModemCategory.isCdma() && !FeatureSupport.is90Modem()) {
            if (ModemCategory.CheckViceSimCdmaCapability(mSimType)) {
                initCdmaArray();
                queryCurrentModeCdma();
            }
        }

        querySupportModeGsm();
        queryCurrentModeGsm();

        if (ModemCategory.CheckViceSimNRCapability(mSimType)) {
            initNRArray();
            querySupportModeNR();
            queryCurrentModeNR();
        }

    }

    private void showBandModeGsm(AsyncResult aSyncResult, int msg) {
        final String[] result = (String[]) aSyncResult.result;

        for (final String value : result) {
            if (msg == BandModeContent.EVENT_QUERY_SUPPORTED_GSM) {
                Elog.v(TAG, "gsm support --.>" + value);
            } else {
                Elog.v(TAG, "gsm current --.>" + value);
            }
            final String splitString = value.substring(BandModeContent.SAME_COMMAND.length());
            final String[] getDigitalVal = splitString.split(",");

            if (getDigitalVal != null && getDigitalVal.length > 1) {
                long[] values = new long[INDEX_BAND_MAX];
                for (int i = 0; i < values.length; i++) {
                    if (getDigitalVal.length <= i || getDigitalVal[i] == null) {
                        values[i] = 0;
                        continue;
                    }
                    try {
                        values[i] = Long.valueOf(getDigitalVal[i].trim());
                    } catch (NumberFormatException e) {
                        values[i] = 0;
                    }
                }
                if (msg == BandModeContent.EVENT_QUERY_SUPPORTED_GSM) {
                    setSupportedModeGsm(values);
                    if (getDigitalVal.length > 5) {
                        Elog.v(TAG, "The Modem support Lte extend band");
                        mIsLteExtend = true;
                    } else {
                        Elog.v(TAG, "The Modem not support Lte extend band");
                        mIsLteExtend = false;
                    }
                } else {
                    setCurrentModeGsm(values);
                    mCurrentGsmValues = getValFromBoxGsm(false);
                    saveDefaultValueIfNeedGsm(values);
                }
            }
        }
    }

    private void showBandModeNR(AsyncResult aSyncResult, int msg) {
        final String[] result = (String[]) aSyncResult.result;

        for (final String value : result) {
            if (msg == BandModeContent.EVENT_QUERY_SUPPORTED_NR) {
                Elog.v(TAG, "NR support --.>" + value);
            } else {
                Elog.v(TAG, "NR current --.>" + value);
            }
            final String splitString = value.substring(BandModeContent.SAME_NR_COMMAND.length());
            final String[] getDigitalVal = splitString.split(",");
            //  getDigitalVal[3] = String.format("%08x", mCurrentGsmValues[2]) +
            //          String.format("%08x", mCurrentGsmValues[3]) +
            //          String.format("%08x", mCurrentGsmValues[4]);

            long bandNRValue[] = new long[3];
            try {
                bandNRValue[0] = Long.parseLong(getDigitalVal[3].substring(0, 8), 16);
                bandNRValue[1] = Long.parseLong(getDigitalVal[3].substring(8, 16), 16);
                bandNRValue[2] = Long.parseLong(getDigitalVal[3].substring(16, 24), 16);
            } catch (Exception e) {
                Elog.e(TAG, "NR parse failed," + e.getMessage());
            }

            if (msg == BandModeContent.EVENT_QUERY_SUPPORTED_NR) {
                setSupportedModeNR(bandNRValue);
                saveDefaultValueNR(bandNRValue);
            } else {
                setCurrentModeNR(bandNRValue);
                mCurrentNRValues = bandNRValue;
            }
        }
    }

    /**
     * Query 2G/3G/4G Modem supported band modes.
     */
    private void querySupportModeGsm() {
        final String[] modeString = {BandModeContent.QUERY_SUPPORT_COMMAND,
                BandModeContent.SAME_COMMAND};
        Elog.v(TAG, "querySupportGsmMode AT String:" + modeString[0]);
        sendATCommand(modeString, BandModeContent.EVENT_QUERY_SUPPORTED_GSM);
    }

    /**
     * Query 2G/3G/4GModem is being used band modes.
     */
    private void queryCurrentModeGsm() {
        final String[] modeString = {BandModeContent.QUERY_CURRENT_COMMAND,
                BandModeContent.SAME_COMMAND};
        Elog.v(TAG, "queryCurrentGSMMode AT String:" + modeString[0]);
        sendATCommand(modeString, BandModeContent.EVENT_QUERY_CURRENT_GSM);
    }

    /**
     * Set the selected modes.
     *
     * @param values the integers of mode values
     * @return false means set failed or success
     */
    private void setBandModeGsm(final long[] values) {
        String[] modeString = {
                BandModeContent.SET_COMMAND + values[0] + "," + values[1], ""};
        if (ModemCategory.isLteSupport()) {
            modeString[0] += "," + values[2] + "," + values[3];
            if (mIsLteExtend) {
                for (int i = 4; i < INDEX_BAND_MAX - 1; i++) {
                    modeString[0] += "," + values[i];
                }
            }
        }
        Elog.v(TAG, "setGsmBandMode AT String:" + modeString[0]);
        sendATCommand(modeString, BandModeContent.EVENT_SET_BUTTON);
    }

    private void sendATCommand(String[] atCommand, int msg) {
        EmUtils.invokeOemRilRequestStringsEm(mSimType,
                atCommand, mResponseHander.obtainMessage(msg));
    }

    /**
     * Query NR Modem supported band modes.
     */
    private void querySupportModeNR() {
        final String[] modeString = {BandModeContent.QUERY_SUPPORT_NR_COMMAND,
                BandModeContent.SAME_NR_COMMAND};
        Elog.v(TAG, "querySupportNRMode AT String:" + modeString[0]);
        sendATCommand(modeString, BandModeContent.EVENT_QUERY_SUPPORTED_NR);
    }

    /**
     * Query NR Modem is being used band modes.
     */
    private void queryCurrentModeNR() {
        final String[] modeString = {BandModeContent.QUERY_CURRENT_NR_COMMAND,
                BandModeContent.SAME_NR_COMMAND};
        Elog.v(TAG, "queryCurrentNRMode AT String:" + modeString[0]);
        sendATCommand(modeString, BandModeContent.EVENT_QUERY_CURRENT_NR);
    }

    // 1/3/7/8/20/28/38/41/77/78/79
    private void setBandModeNR(final long[] value) {
        String[] cmd = new String[2];
        cmd[0] = BandModeContent.SET_NR_COMMAND;
        cmd[0] += String.format("%08x", mSetGsmValues[0]) + ","
                + String.format("%08x", mSetGsmValues[1]) + ","
                + String.format("%08x", mSetGsmValues[2]) +
                String.format("%08x", mSetGsmValues[3]) +
                String.format("%08x", mSetGsmValues[4]) + ",";

        cmd[0] += String.format("%08x", mSetNRValues[0])
                + String.format("%08x", mSetNRValues[1])
                + String.format("%08x", mSetNRValues[2]);

        cmd[1] = "";
        Elog.v(TAG, "setNrBandMode AT String:" + cmd[0]);
        sendATCommand(cmd, BandModeContent.EVENT_SET_NR);
    }


    /**
     * Get the selected mode values.
     *
     * @return values from the selected boxes
     */
    private long[] getValFromBoxGsm(boolean judge) {
        long[] values = new long[INDEX_BAND_MAX];
        long[] values_temp = new long[INDEX_BAND_MAX];
        for (final BandModeMap m : mGsmModeArray) {
            if (m.mChkBox.isChecked()) {
                values[m.mIndex] |= 1L << m.mBit;
                values_temp[m.mIndex] |= 1L << m.mBit;
            }
        }

        if (judge) {
            // band64 to band256 belongs to lte fdd, so check null together
            for (int i = INDEX_LTE_BAND_96; i <= INDEX_LTE_BAND_256; i++) {
                values_temp[INDEX_LTE_FDD_BAND]
                        = values_temp[INDEX_LTE_FDD_BAND] | values_temp[i];
            }
            // check FDD and TDD ,only all null is invalid
            values_temp[INDEX_LTE_FDD_BAND] = values_temp[INDEX_LTE_FDD_BAND]
                    | values_temp[INDEX_LTE_TDD_BAND];
            values_temp[INDEX_LTE_TDD_BAND] = values_temp[INDEX_LTE_FDD_BAND];
            // LTE only needs to check gsm, umts, lte fdd, lte tdd
            for (int i = 0; i <= INDEX_LTE_TDD_BAND; i++) {
                Elog.v(TAG, "mCurrentGsmValues[" + i + "] = " + mCurrentGsmValues[i]
                        + ", to values[" + i + "] = " + values[i]);
                if ((values_temp[i] == 0) && (mCurrentGsmValues[i] != 0)) {
                    mIsLteValid = false;
                    break;
                } else if (!mIsLteValid) {
                    mIsLteValid = true;
                }
            }

            // null select is not allowed.
            if (values[0] == 0) {
                values[0] = BandModeContent.GSM_MAX_VALUE;
            }
            if (values[1] == 0) {
                values[1] = BandModeContent.UMTS_MAX_VALUE;
            }
            if (values_temp[2] == 0 && values_temp[3] == 0) {
                values[2] = BandModeContent.LTE_MAX_VALUE;
                values[3] = BandModeContent.LTE_MAX_VALUE;
                Elog.v(TAG, "lte not to null");
            }
        }
        return values;
    }

    private long[] getValFromBoxNR() {
        long[] values = new long[3];
        int temp = 0;
        for (final BandModeMap m : mNRModeArray) {
            if (m.mBit < 32) {
                if (m.mChkBox.isChecked()) {
                    values[0] |= 1L << m.mBit;
                }
            } else if (m.mBit < 64) {
                temp = m.mBit - 32;
                if (m.mChkBox.isChecked()) {
                    values[1] |= 1L << temp;
                }
            } else {
                temp = m.mBit - 64;
                if (m.mChkBox.isChecked()) {
                    values[2] |= 1L << temp;
                }
            }

            if (!mIsNRValid) {
                mIsNRValid = true;
            }
        }
        if (values[0] == 0 && values[1] == 0 && values[2] == 0 &&
                (mCurrentNRValues[0] != 0 || mCurrentNRValues[1] != 0
                        || mCurrentNRValues[0] != 0)) {
            mIsNRValid = false;
            Elog.v(TAG, "mIsNRValid:" + mIsNRValid);
        }
        return values;
    }


    private long getValFromBoxCdma() {
        long value = 0;
        for (final BandModeMap m : mCdmaModeArray) {
            if (m.mChkBox.isChecked()) {
                value |= 1L << m.mBit;
            }
            if (!mIsCdmaValid) {
                mIsCdmaValid = true;
            }
        }
        if ((value == 0) && (mCurrentCdmaValues != 0)) {
            mIsCdmaValid = false;
            Elog.v(TAG, "mIsCdmaValid:" + mIsCdmaValid);
        }
        return value;
    }


    private void setCurrentModeGsm(final long[] values) {
        for (final BandModeMap m : mGsmModeArray) {
            if ((values[m.mIndex] & (1L << m.mBit)) == 0) {
                m.mChkBox.setChecked(false);
            } else {
                if (m.mChkBox.isEnabled()) {
                    m.mChkBox.setChecked(true);
                }
            }
        }
    }

    private void setSupportedModeGsm(final long[] values) {
        for (final BandModeMap m : mGsmModeArray) {
            if ((values[m.mIndex] & (1L << m.mBit)) == 0) {
                m.mChkBox.setEnabled(false);
            } else {
                m.mChkBox.setEnabled(true);
            }
        }
    }

    private void setCurrentModeNR(final long[] values) {
        int temp = 0;
        for (final BandModeMap m : mNRModeArray) {
            if (m.mBit < 32) {
                if ((values[0] & (1L << m.mBit)) == 0) {
                    m.mChkBox.setChecked(false);
                } else {
                    if (m.mChkBox.isEnabled()) {
                        m.mChkBox.setChecked(true);
                    }
                }
            } else if (m.mBit < 64) {
                temp = m.mBit - 32;
                if ((values[1] & (1L << temp)) == 0) {
                    m.mChkBox.setChecked(false);
                } else {
                    if (m.mChkBox.isEnabled()) {
                        m.mChkBox.setChecked(true);
                    }
                }
            } else {
                temp = m.mBit - 64;
                if ((values[2] & (1L << temp)) == 0) {
                    m.mChkBox.setChecked(false);
                } else {
                    if (m.mChkBox.isEnabled()) {
                        m.mChkBox.setChecked(true);
                    }
                }
            }
        }
    }

    private void setSupportedModeNR(final long[] values) {
        int temp = 0;
        for (final BandModeMap m : mNRModeArray) {
            if (m.mBit < 32) {
                if ((values[0] & (1L << m.mBit)) == 0) {
                    m.mChkBox.setEnabled(false);
                } else {
                    m.mChkBox.setEnabled(true);
                }
            } else if (m.mBit < 64) {
                temp = m.mBit - 32;
                if ((values[1] & (1L << temp)) == 0) {
                    m.mChkBox.setEnabled(false);
                } else {
                    m.mChkBox.setEnabled(true);
                }
            } else {
                temp = m.mBit - 64;
                if ((values[2] & (1L << temp)) == 0) {
                    m.mChkBox.setEnabled(false);
                } else {
                    m.mChkBox.setEnabled(true);
                }
            }
        }
    }

    private void setCurrentModeCdma(final long value) {
        for (final BandModeMap m : mCdmaModeArray) {
            if ((value & (1L << m.mBit)) == 0) {
                m.mChkBox.setChecked(false);
            } else {
                if (m.mChkBox.isEnabled()) {
                    m.mChkBox.setChecked(true);
                }
            }
        }
    }

    private void setSupportedModeCdma(final long value) {
        for (final BandModeMap m : mCdmaModeArray) {
            if ((value & (1L << m.mBit)) == 0) {
                m.mChkBox.setEnabled(false);
            } else {
                m.mChkBox.setEnabled(true);
            }
        }
    }

    /**
     * Button on click listener.
     *
     * @param arg0 the view of the button event
     */
    public void onClick(final View arg0) {
        if (arg0.getId() == mBtnSet.getId()) {
            mSetCdmaValues = getValFromBoxCdma();
            mSetNRValues = getValFromBoxNR();
            mSetGsmValues = getValFromBoxGsm(true);
            if (!mIsLteValid || !mIsCdmaValid || !mIsNRValid) {
                Elog.d(TAG, "!mIsLteValid || !mIsCdmaValid || !mIsNRValid");
                EmUtils.showToast("the band settings is forbid");
                return;
            }
            setBandModeGsm(mSetGsmValues);
        } else if (arg0.getId() == mBtnReset.getId()) {
            showDialog(BandModeContent.EVENT_RESET);
        }
    }

    @Override
    public void onDestroy() {
        Elog.v(TAG, "onDestroy");
        mIsThisAlive = false;
        super.onDestroy();
    }

    private void queryCurrentModeCdma() {
        String[] cmd = new String[3];
        cmd[0] = BandModeContent.QUERY_CURRENT_COMMAND_CDMA;
        cmd[1] = BandModeContent.SAME_COMMAND_CDMA;
        cmd[2] = "DESTRILD:C2K";
        String[] cmd_s = ModemCategory.getCdmaCmdArr(cmd);
        Elog.v(TAG, "queryCurrentModeCdma: ");
        sendATCommandCdma(cmd_s, BandModeContent.EVENT_QUERY_CURRENT_CDMA);
    }

    private void showBandModeCdma(AsyncResult aSyncResult, int msg) {
        final String[] result = (String[]) aSyncResult.result;
        for (final String string : result) {
            Elog.v(TAG, "cdma --.>" + string);
            String splitString = string.substring(BandModeContent.SAME_COMMAND_CDMA.length());
            if (msg == BandModeContent.EVENT_QUERY_CURRENT_CDMA) {
                final String[] getDigitalVal = splitString.split(",");
                long[] value = new long[2];
                try {
                    for (int i = 0; i < 2; i++) {
                        if (getDigitalVal[i] != null) {
                            value[i] = Integer.parseInt(getDigitalVal[i].substring(2), 16);
                        }
                    }
                } catch (Exception e) {
                    value[0] = 0;
                }
                setSupportedModeCdma(value[0]);
                setCurrentModeCdma(value[1]);
                mCurrentCdmaValues = value[1];
                long valueCur = (value[1] == 0) ? value[0] : value[1];
                saveDefaultValueIfNeedCdma(valueCur);
            }
        }
    }

    private void sendATCommandCdma(String[] atCommand, int msg) {
        Elog.v(TAG, "c2k AT String:" + atCommand[0]
                + ",atCommand.size = " + atCommand.length);
        EmUtils.invokeOemRilRequestStringsEm(true, atCommand, mResponseHander.obtainMessage(msg));
    }

    private void setBandModeCdma(final long value) {
        Elog.v(TAG, "setCdmaBandMode: ");
        if (!mIsCdmaValid && !mIsLteValid) {
            return;
        }
        String[] cmd = new String[3];
        cmd[0] = BandModeContent.SET_COMMAND_CDMA + value;
        cmd[1] = "";
        cmd[2] = "DESTRILD:C2K";
        String[] cmd_s = ModemCategory.getCdmaCmdArr(cmd);
        sendATCommandCdma(cmd_s, BandModeContent.EVENT_SET_CDMA);
    }

    /**
     * Get default mode values.
     *
     * @return saved default values
     */
    private long[] getDefaultValueGsm() {
        SharedPreferences pref = getSharedPreferences(PREF_FILE + mSimType, MODE_PRIVATE);
        long[] values = new long[INDEX_BAND_MAX];
        long[] values_temp = new long[INDEX_BAND_MAX];
        Elog.v(TAG, "getDefaultValueGsm: ");
        for (int i = 0; i < INDEX_BAND_MAX - 1; i++) {
            values[i] = pref.getLong(PREF_KEYS[i], 0);
            values_temp[i] = pref.getLong(PREF_KEYS[i], 0);
            Elog.v(TAG, "values[" + i + "] = " + values[i]);
        }

        for (int i = INDEX_LTE_BAND_96; i <= INDEX_LTE_BAND_256; i++) {
            values_temp[INDEX_LTE_FDD_BAND] = values_temp[INDEX_LTE_FDD_BAND]
                    | values_temp[i];
        }

        // null select is not allowed.
        if (values[0] == 0) {
            values[0] = BandModeContent.GSM_MAX_VALUE;
        }
        if (values[1] == 0) {
            values[1] = BandModeContent.UMTS_MAX_VALUE;
        }
        if (values_temp[2] == 0 && values_temp[3] == 0) {
            values[2] = BandModeContent.LTE_MAX_VALUE;
            values[3] = BandModeContent.LTE_MAX_VALUE;
            Elog.v(TAG, "getDefaultValue,lte not to null");
        }
        setCurrentModeGsm(values);
        return values;
    }


    private void saveDefaultValueNR(long[] values) {
        mDefaultNRValues[0] = values[0];
        mDefaultNRValues[1] = values[1];
        mDefaultNRValues[2] = values[2];
    }

    private void getDefaultValueNR() {
        mCurrentNRValues[0] = mDefaultNRValues[0];
        mCurrentNRValues[1] = mDefaultNRValues[1];
        mCurrentNRValues[2] = mDefaultNRValues[2];
        setCurrentModeNR(mCurrentNRValues);
    }

    /**
     * Save default mode values to preference if not exist.
     *
     * @param values the integer values from the modem
     */
    private void saveDefaultValueIfNeedGsm(long[] values) {
        SharedPreferences pref = getSharedPreferences(PREF_FILE + mSimType, MODE_PRIVATE);
        SharedPreferences.Editor editor = pref.edit();
        for (int i = 0; i < INDEX_BAND_MAX - 1; i++) {
            if (!pref.contains(PREF_KEYS[i])) {
                editor.putLong(PREF_KEYS[i], values[i]);
                Elog.v(TAG, "save gsm default values[" + i + "] = " + values[i]);
            }
        }
        editor.commit();
    }

    private long getDefaultValueCdma() {
        SharedPreferences pref = getSharedPreferences(PREF_FILE + mSimType, MODE_PRIVATE);
        long value = pref.getLong(PREF_KEYS[INDEX_CDMA_BAND], 0);
        Elog.v(TAG, "getDefaultValueCdma: " + value);
        setCurrentModeCdma(value);
        return value;
    }

    private void saveDefaultValueIfNeedCdma(long value) {
        SharedPreferences pref = getSharedPreferences(PREF_FILE + mSimType, MODE_PRIVATE);
        SharedPreferences.Editor editor = pref.edit();
        if (!pref.contains(PREF_KEYS[INDEX_CDMA_BAND])) {
            editor.putLong(PREF_KEYS[INDEX_CDMA_BAND], value);
            Elog.v(TAG, "save cdam default value: " + value);
        }
        editor.commit();
    }

    @Override
    protected Dialog onCreateDialog(final int dialogId) {
        if (BandModeContent.EVENT_RESET == dialogId) {
            final AlertDialog.Builder builder = new AlertDialog.Builder(
                    BandSelect.this);
            builder.setTitle(R.string.reset_title);
            builder.setMessage(R.string.reset_message);
            builder.setPositiveButton(android.R.string.ok,
                    new DialogInterface.OnClickListener() {
                        @Override
                        public void onClick(DialogInterface dialog, int which) {
                            mSetGsmValues = getDefaultValueGsm();
                            mSetCdmaValues = getDefaultValueCdma();
                            getDefaultValueNR();
                            setBandModeGsm(mSetGsmValues);
                            dialog.dismiss();
                        }
                    }
            );
            builder.setNegativeButton(android.R.string.cancel, null);
            builder.create().show();
        } else if (BandModeContent.EVENT_SET_FAIL == dialogId) {
            Elog.d(TAG, "band set failed");
            final AlertDialog.Builder builder = new AlertDialog.Builder(
                    BandSelect.this);
            builder.setTitle(R.string.wait_finish);
            builder.setMessage("please wait for reboot airplane finish and try again");
            builder.setPositiveButton(R.string.set_fail_text, null);
            builder.create().show();
        } else {
            final AlertDialog.Builder builder = new AlertDialog.Builder(
                    BandSelect.this);
            builder.setTitle(R.string.set_fail_title);
            builder.setMessage(R.string.set_fail_message);
            builder.setPositiveButton(R.string.set_fail_text, null);
            builder.create().show();
        }
        return super.onCreateDialog(dialogId);
    }

    private static class BandModeMap {
        public CheckBox mChkBox;
        public int mIndex;
        public int mBit;

        /**
         * @param chkbox the CheckBox from the layout
         * @param index  the integer of the modem value (index)
         * @param bit    the integer of the modem value (bit offset)
         */
        BandModeMap(final CheckBox chkbox, final int index, final int bit) {
            mChkBox = chkbox;
            mIndex = index;
            mBit = bit;
        }
    }
}
