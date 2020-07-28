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

package com.mediatek.engineermode.lterxmimoconfigure;

import android.app.Activity;
import android.content.Intent;
import android.os.AsyncResult;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.TableLayout;
import android.widget.TableRow;

import com.mediatek.engineermode.Elog;
import com.mediatek.engineermode.EmUtils;
import com.mediatek.engineermode.R;

import java.util.ArrayList;

public class LteSingleBandSetting extends Activity implements OnClickListener {
    public static final int LTE_BAND_SETTING_MODE_4X4MIMO = 0;
    public static final int LTE_BAND_SETTING_MODE_4RX = 1;
    public static final String CMD_SB_MIMO_SET = "\"sb_mimo_set\"";
    public static final String CMD_SB_4RX_SET = "\"sb_4rx_set\"";
    private static final int LTE_BAND_QUERY_MODE = 0;
    private static final String TAG = "LteRx/SingleBandSetting";
    private static final int LTE_BAND_NUM = 256;
    public static int LteBandSettingModeHistory = 0;
    public static String[] mSbMimoSetString = new String[2];
    private static int LteBandSettingMode = 0;
    private final ArrayList<BandModeMap> mModeArray = new ArrayList<BandModeMap>();
    private long[][] mSbMimoSet = new long[2][8];
    private Button mBtnSet;
    private Handler mResponseHander = new Handler() {
        public void handleMessage(final Message msg) {
            AsyncResult asyncResult;
            switch (msg.what) {
                case LTE_BAND_QUERY_MODE:
                    asyncResult = (AsyncResult) msg.obj;
                    if (asyncResult.exception == null) {
                        final String[] result = (String[]) asyncResult.result;
                        if (result == null || result.length < 0) {
                            EmUtils.showToast("Query lte band mode error1");
                            Elog.e(TAG, "Query lte band mode error1");
                        } else {
                            Elog.d(TAG, "Query lte band mode succeed");
                            updateCurrentMode(result[0]);
                        }
                    } else {
                        if (LteBandSettingMode == 0)
                            updateCurrentMode("0,1,2,3,4,5,6,7,8");
                        else
                            updateCurrentMode("0,8,7,6,5,4,3,2,1");
                        EmUtils.showToast("Query lte band mode error2");
                        Elog.e(TAG, "Query lte band mode error2");
                    }
                    break;
                default:
                    break;
            }
        }
    };


    private ArrayList<CheckBox> addCheckboxToTable(int tableResId) {
        TableLayout table = (TableLayout) findViewById(tableResId);
        ArrayList<CheckBox> ret = new ArrayList<CheckBox>(LTE_BAND_NUM);

        for (int i = 0; i < LTE_BAND_NUM; i++) {
            TableRow row = new TableRow(this);
            CheckBox checkBox = new CheckBox(this);
            checkBox.setText("band " + (i + 1));
            row.addView(checkBox);
            table.addView(row);
            ret.add(checkBox);
        }
        return ret;
    }

    private void initLteArray() {
        ArrayList<CheckBox> checkBox = addCheckboxToTable(R.id.TableLayout_LTE_Band_Configure);
        for (int i = 0; i < checkBox.size(); i++) {
            mModeArray.add(new BandModeMap(checkBox.get(i), i));
        }
    }

    @Override
    public void onCreate(final Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.lte_rx_single_band_setting);
        mBtnSet = (Button) findViewById(R.id.BandSel_Btn_Set);
        Intent intent = getIntent();
        LteBandSettingMode = intent.getIntExtra("band_setting_type",
                LteSingleBandSetting.LTE_BAND_SETTING_MODE_4X4MIMO);
        LteBandSettingModeHistory |= 1 << LteBandSettingMode;
        Elog.d(TAG, "LteBandSettingMode = " + LteBandSettingMode);
        Elog.d(TAG, "LteBandSettingModeHistory = " + LteBandSettingModeHistory);
        initLteArray();
        mBtnSet.setOnClickListener(this);
    }

    @Override
    protected void onResume() {
        super.onResume();
        Elog.v(TAG, "onResume");
        queryCurrentMode();
    }

    private void queryCurrentMode() {
        String queryCmd = "";
        if (LteBandSettingMode == LTE_BAND_SETTING_MODE_4X4MIMO)
            queryCmd = CMD_SB_MIMO_SET;
        else {
            queryCmd = CMD_SB_4RX_SET;
        }
        String[] modeString = {"AT+EGMC=0," + queryCmd, "+EGMC:"};
        sendATCommand(modeString, LTE_BAND_QUERY_MODE);
    }

    private void sendATCommand(String[] atCommand, int msg) {
        Elog.d(TAG, "atCommand = " + atCommand[0]);
        EmUtils.invokeOemRilRequestStringsEm(atCommand, mResponseHander.obtainMessage(msg));
    }

    private void getValFromBox() {
        int count = 0;
        Elog.d(TAG, "getValFromBox:");
        for (final BandModeMap m : mModeArray) {
            mSbMimoSet[LteBandSettingMode][count] = 0;
            mSbMimoSetString[LteBandSettingMode] = "";
        }
        for (final BandModeMap m : mModeArray) {
            if (m.mChkBox.isChecked()) {
                mSbMimoSet[LteBandSettingMode][count] |= 1L << m.mBit;
            }
            if (m.mBit == 31) {
                // Elog.d(TAG, "mSbMimoSet[" + count + "] = " + mSbMimoSet[count]);
                mSbMimoSetString[LteBandSettingMode] += "," + mSbMimoSet[LteBandSettingMode][count];
                count++;
            }
        }
        Elog.d(TAG, "mSbMimoSetString: " + mSbMimoSetString[LteBandSettingMode]);

    }

    private void updateCurrentMode(String result) {
        int count = 1;
        String info[] = null;
        try {
            info = result.split(",");
        } catch (Exception e) {
            e.printStackTrace();
            return;
        }
        Elog.v(TAG, "LteBandSettingMode = " + LteBandSettingMode);
        mSbMimoSetString[LteBandSettingMode] = "";
        for (int i = 0; i < info.length - 1; i++) {
            mSbMimoSet[LteBandSettingMode][i] = Long.valueOf(info[i + 1]);
            mSbMimoSetString[LteBandSettingMode] += "," + (mSbMimoSet[LteBandSettingMode][i]);
            // Elog.v(TAG, "mSbMimoSet = " + mSbMimoSet[LteBandSettingMode][i]);
        }
        Elog.v(TAG, "mSbMimoSetString = " + mSbMimoSetString[LteBandSettingMode]);
        for (final BandModeMap m : mModeArray) {
            if ((Long.valueOf(info[count]) & (1L << m.mBit)) == 0) {
                m.mChkBox.setChecked(false);
            } else {
                m.mChkBox.setChecked(true);
            }
            if (m.mBit == 31) {
                count++;
            }
        }
    }


    public void onClick(final View arg0) {
        Elog.d(TAG, "Band setting button click");
        getValFromBox();
    }

    @Override
    public void onDestroy() {
        Elog.v(TAG, "onDestroy");
        super.onDestroy();
    }

    private static class BandModeMap {
        public CheckBox mChkBox;
        public int mBit;

        BandModeMap(final CheckBox chkbox, final int bit) {
            mChkBox = chkbox;
            mBit = bit % 32;
        }
    }
}
