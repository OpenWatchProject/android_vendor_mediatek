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

package com.mediatek.engineermode.forceant;

import android.app.Activity;
import android.content.SharedPreferences;
import android.os.AsyncResult;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.util.Log;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemSelectedListener;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.EditText;
import android.widget.RadioButton;
import android.widget.RadioGroup;
import android.widget.RadioGroup.OnCheckedChangeListener;
import android.widget.Spinner;
import android.widget.TextView;
import android.widget.Toast;

import com.mediatek.engineermode.Elog;
import com.mediatek.engineermode.EmUtils;
import com.mediatek.engineermode.FeatureSupport;
import com.mediatek.engineermode.ModemCategory;
import com.mediatek.engineermode.R;

public class ForceTx extends Activity implements OnClickListener, OnCheckedChangeListener {

    private static final String TAG = "ForceTx";
    private static final int FORCE_TYPE_BY_RAT = 0;
    private static final int FORCE_TYPE_BY_BAND = 1;

    private static final int FORCE_TX_DISABLE_RAT = 0;
    private static final int FORCE_TX_ENABLE_RAT = 1;
    private static final int FORCE_TX_QUERY_RAT = 2;
    private static final int FORCE_TX_DISABLED_BAND = 3;
    private static final int FORCE_TX_ENABLE_BAND = 4;
    private static final int FORCE_TX_QUERY_BAND = 5;

    private static final int FORCE_TX_RAT_GSM = 1;
    private static final int FORCE_TX_RAT_UMTS = 2;
    private static final int FORCE_TX_RAT_LTE = 3;
    private static final int FORCE_TX_RAT_C2K = 4;

    private Spinner mForceAntSpinner;
    private Spinner mTasVerSpinner;
    private Spinner mIdxSpinner;

    private Spinner mTasRatSpinner;
    private Spinner mTasnvramSpinner;

    private RadioGroup mRGForceType = null;
    private RadioButton radioBtnForceRat = null;
    private RadioButton radioBtnforceBand = null;

    private RadioGroup mRGForceDpdt = null;
    private RadioButton radioBtnForceDpdtSingle = null;
    private RadioButton radioBtnForceDpdtDouble = null;

    private TextView mTasNvramText;
    private EditText mTasBandEdit;
    private TextView mTasBandText;
    private EditText mTasAntIdxEdit;
    private TextView mTasAntIdxView;

    private Button mSetButton;
    private ArrayAdapter<String> mVersionAdatper1;

    private static final String SHREDPRE_NAME = "ForceTx";
    private static final String TAS_TYPE = "TasType";
    private static final String TAS_DPDT = "TasDpdt";
    private static final String TAS_MODE = "TasMode";
    private static final String TAS_VERSION = "TasVersion";
    private static final String TAS_RAT = "TasRat";
    private static final String TAS_NVRAM = "TasNvram";
    private static final String TAS_BAND = "TasBand";

    private static int mForceType = FORCE_TYPE_BY_RAT;
    private static int mForceDpdt = 0;
    private static int mtasMode = 0;
    private static int mtasVersion = 0;
    private static int mtasRat = 1;
    private static String mtasNvram = "0";
    private static String mtasBand = "1";
    private static String mtasidx = "1";

    private static String[] mDpdtArray = { "signal dpdt", "double dpdt" };
    private static String[] mTypeArray = { "by rat", "by band" };
    private static String[] mModeArray = { "OFF", "ON", "QUERY" };
    private static String[] mVersionArray = { "V1.0", "V2.0" };
    private static String[] mRatArray = { "GSM", "UTMS", "LTE", "C2K" };

    private static String[][] mTasDpdt1Label = {
            { "LANT  X", "UANT  X" }, // 0
            { "LANT  UANT", "UANT LANT" }, // 1
            { "LANT X", "UANT X", "LANT(') X" }, // 2
            { "LANT  UANT", "UANT  LANT", "LANT(') UANT", "UANT LANT(') " },// 3
            { "LANT  X  X  X", "UANT  X  X  X" }, // 4
            { "LANT  X  UANT  X", "UANT  X  UANT  X" }, // 5
            { "LANT  LANT  UANT  UANT", "LANT  UANT  UANT  LANT", "UANT  LANT  LANT  UANT",
                    "UANT  UANT  LANT  LANT" }, // 6
            { "LANT  X  UANT  X", "UANT  X  LANT  X", "LANT(')  X  UANT  X",
                    "UANT  X  LANT(')  X" }, // 7
            { "LANT  LANT  UANT  UANT", "LANT  UANT  UANT  LANT", "UANT  LANT  LANT  UANT",
                    "UANT  UANT  LANT  LANT",
                    "LANT(')  LANT(')  UANT  UANT", "LANT(')  UANT  UANT  LANT(')",
                    "UANT  LANT(')  LANT(')  UANT", "UANT  UANT  LANT(')  LANT(')",
            // 8
            }

    };

    private static int[][] mTasDpdt1 = {
            { 0, 2 },
            { 1, 3 },
            { 1, 3 },
            { 1, 3 }
    };

    private static int[][] mTasDpdt2 = {
            { 4, 2 },
            { 5, 7 },
            { 6, 8 },
            { 5, 7 }
    };

    public void queryTasIdxLabels() {
        int status_index = 0;
        mVersionAdatper1.clear();
        String[] values = null;

        Elog.d(TAG, "\nupdate TAS Idx: ");
        Elog.d(TAG, "mtasRat = " + mtasRat);
        Elog.d(TAG, "mtasVersion = " + mtasVersion);

        if (mForceDpdt == 0) {
            status_index = mTasDpdt1[mtasRat - 1][mtasVersion - 1];
            values = mTasDpdt1Label[status_index];
        }
        else {
            status_index = mTasDpdt2[mtasRat - 1][mtasVersion - 1];
            values = mTasDpdt1Label[status_index];
        }

        Elog.d(TAG, "status_index = " + status_index);

        if (status_index > 3) {
            mTasAntIdxView.setText("LM_Main H_Main LM_DRX H_DRX");
        } else {
            mTasAntIdxView.setText("LMH_Main LMH_DRX");
        }

        for (int i = 0; i < values.length; i++) {
            Elog.d(TAG, "mTasDpdt1Label = " + values[i]);
        }

        mVersionAdatper1.addAll(values);
        mIdxSpinner.setAdapter(mVersionAdatper1);

    }

    private OnItemSelectedListener mSpinnerListener = new OnItemSelectedListener() {
        @Override
        public void onItemSelected(AdapterView<?> parent, View arg1, int pos, long arg3) {

            if (parent == mForceAntSpinner) {
                Elog.d(TAG, "mForceAntModeSpinner changed, pos = " + pos);
                if (pos == 0) {             //disable
                    ;
                } else if (pos == 1) {      //set
                    ;
                } else if (pos == 2) {      //query
                    mTasAntIdxEdit.setText("");
                    return;
                }

            }
            else if (parent == mTasVerSpinner) {
                Elog.d(TAG, "mtasVersion changed, pos = " + pos);
                mtasVersion = (pos == 0) ? 1 : 2;
            }
            else if (parent == mTasRatSpinner) {
                Elog.d(TAG, "mTasRatSpinner changed, pos = " + pos);
                mtasRat = pos + 1;
            }
            else if (parent == mIdxSpinner) {
                Elog.d(TAG, "mIdxSpinner changed, pos = " + pos);
                mTasAntIdxEdit.setText(pos + "");
                return;
            }
            queryTasIdxLabels();
        }

        @Override
        public void onNothingSelected(AdapterView parent) {
        }
    };

    private final Handler mATCmdHander = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            AsyncResult ar = (AsyncResult) msg.obj;
            switch (msg.what) {
            case FORCE_TX_ENABLE_RAT:
            case FORCE_TX_DISABLE_RAT:
            case FORCE_TX_ENABLE_BAND:
            case FORCE_TX_DISABLED_BAND:

                if (ar.exception != null) {
                    Toast.makeText(ForceTx.this,
                            " AT cmd failed.", Toast.LENGTH_LONG)
                            .show();
                } else {
                    Toast.makeText(ForceTx.this,
                            " AT cmd successfully.", Toast.LENGTH_LONG)
                            .show();
                }
                break;
            case FORCE_TX_QUERY_RAT:
            case FORCE_TX_QUERY_BAND:

                if (ar.exception == null) {
                    String[] receiveDate = (String[]) ar.result;
                    int[] antStateInt = new int[4];
                    if (null == receiveDate) {
                        Toast.makeText(ForceTx.this, "Warning: Received data is null!",
                                Toast.LENGTH_SHORT).show();
                    } else {

                        Elog.d(TAG, "receiveDate[0] = " + receiveDate[0]);
                        int value = 0;
                        try {
                            final String[] getDigitalVal = receiveDate[0].split(",");
                            value = Integer.parseInt(getDigitalVal[1]);
                        } catch (Exception e) {
                            value = 0;
                        }

                        Elog.d(TAG, "value = " + value);

                        if (value != 255) {
                            queryTasIdxLabels();
                            if( value < mIdxSpinner.getCount() ){
                                mIdxSpinner.setSelection(value);
                            }
                            else{
                                String msg1 = "The return idx == " + value +
                                        " not match the version or dpdt ";
                                Toast.makeText(ForceTx.this, msg1, Toast.LENGTH_LONG).show();
                                Elog.e(TAG, msg1);
                            }
                        }
                        else {
                            mTasAntIdxEdit.setText("disabled");
                        }

                    }
                } else {
                    Toast.makeText(ForceTx.this,
                            " AT cmd failed.", Toast.LENGTH_LONG)
                            .show();
                }
                break;
            default:
                break;
            }
        }
    };

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.force_tx);

        mTasRatSpinner = (Spinner) findViewById(R.id.tas_rat);
        mTasnvramSpinner = (Spinner) findViewById(R.id.tas_nvram);
        mTasBandEdit = (EditText) findViewById(R.id.tas_band);
        mTasBandText = (TextView) findViewById(R.id.tas_band_text);
        mTasNvramText = (TextView) findViewById(R.id.tas_nvram_text);
        mTasAntIdxEdit = (EditText) findViewById(R.id.tas_ant_idx);
        mTasAntIdxEdit.setEnabled(false);

        mForceAntSpinner = (Spinner) findViewById(R.id.force_ant1);
        mTasVerSpinner = (Spinner) findViewById(R.id.tas_ver1);
        mIdxSpinner = (Spinner) findViewById(R.id.dpdt1_idx);
        mTasAntIdxView = (TextView) findViewById(R.id.tas_ant_idx_View);

        mSetButton = (Button) findViewById(R.id.force_tx_set);
        mSetButton.setOnClickListener(this);

        mVersionAdatper1 = new ArrayAdapter<String>(this,
                android.R.layout.simple_spinner_item);
        mVersionAdatper1
                .setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);

        mIdxSpinner.setAdapter(mVersionAdatper1);

        mRGForceType = (RadioGroup) findViewById(R.id.force_type);
        mRGForceType.setOnCheckedChangeListener(this);

        radioBtnForceRat = (RadioButton) findViewById(R.id.force_rat);
        radioBtnforceBand = (RadioButton) findViewById(R.id.force_band);

        mRGForceDpdt = (RadioGroup) findViewById(R.id.force_dpdt);
        mRGForceDpdt.setOnCheckedChangeListener(this);
        radioBtnForceDpdtSingle = (RadioButton) findViewById(R.id.force_dpdt_single);
        radioBtnForceDpdtDouble = (RadioButton) findViewById(R.id.force_dpdt_double);

    }

    @Override
    public void onResume() {
        super.onResume();
        mForceAntSpinner.setOnItemSelectedListener(mSpinnerListener);
        mTasVerSpinner.setOnItemSelectedListener(mSpinnerListener);
        mTasRatSpinner.setOnItemSelectedListener(mSpinnerListener);
        mIdxSpinner.setOnItemSelectedListener(mSpinnerListener);

        getTasSettingStatus();

        mVersionAdatper1.clear();
        mVersionAdatper1.addAll(mTasDpdt1Label[mTasDpdt1[mtasRat - 1][mtasVersion - 1]]);
        mIdxSpinner.setAdapter(mVersionAdatper1);

        if (mForceType == FORCE_TYPE_BY_RAT) {
            radioBtnForceRat.setChecked(true);
        }
        else {
            radioBtnforceBand.setChecked(true);
        }

        if (mForceDpdt == 0) {
            radioBtnForceDpdtSingle.setChecked(true);
        }
        else {
            radioBtnForceDpdtDouble.setChecked(true);
        }

        mForceAntSpinner.setSelection((mtasMode < 3) ? mtasMode : mtasMode - 3, true);

        mTasVerSpinner.setSelection(mtasVersion - 1, true);
        mTasRatSpinner.setSelection(mtasRat - 1, true);
        mTasnvramSpinner.setSelection(Integer.parseInt(mtasNvram), true);
        mTasBandEdit.setText(mtasBand);

        if (!FeatureSupport.is93Modem()) {
            mRGForceType.setVisibility(View.GONE);
            mTasBandEdit.setVisibility(View.GONE);
            mTasBandText.setVisibility(View.GONE);
            mTasNvramText.setVisibility(View.GONE);
            mTasnvramSpinner.setVisibility(View.GONE);
            radioBtnForceRat.setChecked(true);
        }
    }

    @Override
    public void onClick(final View arg0) {
        if (arg0 == mSetButton) {
            Elog.d(TAG, "forceAnt set: " );
            mtasMode = mForceAntSpinner.getSelectedItemPosition()
                    + ((mForceType == FORCE_TYPE_BY_BAND) ? 3 : 0);

            mtasBand = mTasBandEdit.getText().toString();
            mtasidx = mTasAntIdxEdit.getText().toString();

            mtasNvram = (mTasnvramSpinner.getSelectedItemPosition() == 0) ? "0" : "1";

            writeTasSettingStatus();

            if (mtasMode == FORCE_TX_DISABLE_RAT || mtasMode == FORCE_TX_QUERY_RAT ||
                    mtasMode == FORCE_TX_DISABLED_BAND || mtasMode == FORCE_TX_QUERY_BAND) {
                mtasidx = "";
                mtasNvram = "";
            }

            mtasBand = (mForceType == FORCE_TYPE_BY_BAND) ? mtasBand : "";
            mtasNvram = (mForceType == FORCE_TYPE_BY_BAND) ? "" : mtasNvram;

            Elog.d(TAG, "mForceType = " + mForceType + "," + mTypeArray[mForceType]);
            Elog.d(TAG, "mForceDpdt = " + mForceDpdt + "," + mDpdtArray[mForceDpdt]);
            Elog.d(TAG, "mtasMode = " + mtasMode + ","
                    + mModeArray[(mtasMode < 3) ? mtasMode : mtasMode - 3]);
            Elog.d(TAG, "mtasVersion = " + mtasVersion + "," + mVersionArray[mtasVersion - 1]);
            Elog.d(TAG, "mtasRat = " + mtasRat + "," + mRatArray[mtasRat - 1]);
            Elog.d(TAG, "mtasNvram = " + mtasNvram);
            Elog.d(TAG, "mtasBand = " + mtasBand);

            setTasForceIdx(mtasMode, mtasRat, mtasidx, mtasBand, mtasNvram);

        }
    }

    public void getTasSettingStatus() {
        final SharedPreferences tasVersionSh = getSharedPreferences(SHREDPRE_NAME,
                MODE_PRIVATE);

        mForceType = tasVersionSh.getInt(TAS_TYPE, FORCE_TYPE_BY_RAT);
        mForceDpdt = tasVersionSh.getInt(TAS_DPDT, 0);
        mtasMode = tasVersionSh.getInt(TAS_MODE, 0);
        mtasVersion = tasVersionSh.getInt(TAS_VERSION, 1);
        mtasRat = tasVersionSh.getInt(TAS_RAT, 1);
        mtasNvram = tasVersionSh.getString(TAS_NVRAM, "0");
        mtasBand = tasVersionSh.getString(TAS_BAND, "1");

        Elog.d(TAG, "\ngetTasSettingStatus: ");
        Elog.d(TAG, "mForceType = " + mForceType);
        Elog.d(TAG, "mForceDpdt = " + mForceDpdt);
        Elog.d(TAG, "mtasMode = " + mtasMode);
        Elog.d(TAG, "mtasVersion = " + mtasVersion);
        Elog.d(TAG, "mtasRat = " + mtasRat);
        Elog.d(TAG, "mtasNvram = " + mtasNvram);
        Elog.d(TAG, "mtasBand = " + mtasBand);

    }

    public void writeTasSettingStatus() {
        final SharedPreferences tasVersionSh = getSharedPreferences(SHREDPRE_NAME,
                MODE_PRIVATE);
        final SharedPreferences.Editor editor = tasVersionSh.edit();

        editor.putInt(TAS_TYPE, mForceType);
        editor.putInt(TAS_DPDT, mForceDpdt);
        editor.putInt(TAS_MODE, mtasMode);
        editor.putInt(TAS_VERSION, mtasVersion);
        editor.putInt(TAS_RAT, mtasRat);
        editor.putString(TAS_NVRAM, mtasNvram);
        editor.putString(TAS_BAND, mtasBand);
        editor.commit();
    }

    public void setTasForceIdx(int mode, int rat,
            String antenna_idx, String band, String nvram_write) {

        String[] cmd = new String[2];

        if (FeatureSupport.is93Modem()) {
            if (mForceType == FORCE_TYPE_BY_BAND) {
                cmd[0] = "AT+ETXANT=" + mode + "," + rat + "," + antenna_idx + "," + band;
            } else {
                cmd[0] = "AT+ETXANT=" + mode + "," + rat + "," + antenna_idx + "," + band + ","
                        + nvram_write;
            }
        }else {
            if(antenna_idx.equals(""))
                cmd[0] = "AT+ETXANT=" + mode + "," + rat  ;
            else
                cmd[0] = "AT+ETXANT=" + mode + "," + rat + "," + antenna_idx ;
        }


        cmd[1] = "+ETXANT:";

        if (rat == FORCE_TX_RAT_C2K) {
            String[] atCommandC2K = new String[3];
            atCommandC2K[0] = cmd[0];
            atCommandC2K[1] = cmd[1];
            atCommandC2K[2] = "DESTRILD:C2K";
            String[] cmd_s = ModemCategory.getCdmaCmdArr(atCommandC2K);
            Log.d(TAG, "sendAtCommand: " + cmd_s[0] + ",cmd_s.length = " + cmd_s.length);
            sendCdmaAtCommand(cmd_s, mode);
        } else {
            Log.d(TAG, "sendAtCommand: " + cmd[0]);
            sendLteAtCommand(cmd, mode);
        }

    }

    public void sendLteAtCommand(String[] atCommand, int msg) {

        EmUtils.invokeOemRilRequestStringsEm(atCommand, mATCmdHander.obtainMessage(msg));
    }

    public void sendCdmaAtCommand(String[] atCommand, int msg) {
        EmUtils.invokeOemRilRequestStringsEm(true,atCommand, mATCmdHander.obtainMessage(msg));
    }

    @Override
    public void onCheckedChanged(RadioGroup arg0, int checkedId) {
        // TODO Auto-generated method stub
        if (checkedId == R.id.force_rat) {
            mForceType = FORCE_TYPE_BY_RAT;
            mTasBandEdit.setVisibility(View.GONE);
            mTasBandText.setVisibility(View.GONE);
            mTasnvramSpinner.setVisibility(View.VISIBLE);
            mTasNvramText.setVisibility(View.VISIBLE);
        } else if (checkedId == R.id.force_band) {
            mForceType = FORCE_TYPE_BY_BAND;
            mTasBandEdit.setVisibility(View.VISIBLE);
            mTasBandText.setVisibility(View.VISIBLE);
            mTasnvramSpinner.setVisibility(View.GONE);
            mTasNvramText.setVisibility(View.GONE);
        }
        else if (checkedId == R.id.force_dpdt_single) {
            mForceDpdt = 0;
            queryTasIdxLabels();
        } else if (checkedId == R.id.force_dpdt_double) {
            mForceDpdt = 1;
            queryTasIdxLabels();
        }

    }
}
