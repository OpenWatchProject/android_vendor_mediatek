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
import android.app.AlertDialog;
import android.app.Dialog;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.SharedPreferences;
import android.os.AsyncResult;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemSelectedListener;
import android.widget.Button;
import android.widget.EditText;
import android.widget.RadioButton;
import android.widget.RadioGroup;
import android.widget.RadioGroup.OnCheckedChangeListener;
import android.widget.Spinner;
import android.widget.TableRow;
import android.widget.TextView;

import com.mediatek.engineermode.Elog;
import com.mediatek.engineermode.EmUtils;
import com.mediatek.engineermode.ModemCategory;
import com.mediatek.engineermode.R;

public class ForceTx95 extends Activity implements OnClickListener,
        OnCheckedChangeListener {

    private static final String TAG = "ForceAnt95";
    private static final int FORCE_TYPE_BY_RAT = 0;
    private static final int FORCE_TYPE_BY_BAND = 1;

    private static final int FORCE_TX_DISABLE_RAT = 0;
    private static final int FORCE_TX_ENABLE_RAT = 1;
    private static final int FORCE_TX_QUERY_RAT = 2;
    private static final int FORCE_TX_DISABLED_BAND = 3;
    private static final int FORCE_TX_ENABLE_BAND = 4;
    private static final int FORCE_TX_QUERY_BAND = 5;
    private static final int QUERY_ANT_INDEX_BY_STATUS = 6;
    private static final int QUERY_ANT_STATUS_BY_INDEX = 7;

    private static final int DIG_WARING_WRITE_NVRAM = 1;

    private static final int FORCE_TX_RAT_GSM = 1;
    private static final int FORCE_TX_RAT_UMTS = 2;
    private static final int FORCE_TX_RAT_LTE = 3;
    private static final int FORCE_TX_RAT_C2K = 4;
    private static final String SHREDPRE_NAME = "ForceAnt95";
    private static final String TAS_TYPE = "TasType";
    private static final String TAS_MODE = "TasMode";
    private static final String TAS_RAT = "TasRat";
    private static final String TAS_NVRAM = "TasNvram";
    private static final String TAS_BAND = "TasBand";
    private static int mForceType = FORCE_TYPE_BY_RAT;

    private static int mTasModeEnableStates = 0;

    private static int mtasMode = 0;
    private static int mtasRat = 1;
    private static String mtasNvram = "false";
    private static String mtasBand = "1";
    private static String mtasStates = "1";

    private static String[] mTypeArray = {"by rat", "by band"};
    private static String[] mModeArray = {"Disable", "Enable", "Read"};
    private static String[] mRatArray = {"GSM", "UTMS", "LTE", "C2K"};
    private String mQueryMode = "";
    private Spinner mForceAntModeSpinner;
    private Spinner mTasRatSpinner;
    private EditText mTasNvramEdit;
    private RadioGroup mRGForceType = null;
    private RadioButton mForceRatRadioBtn = null;
    private RadioButton mForceBandRadioBtn = null;

    private EditText mTasBandEdit;

    private TextView mTasAntTest;
    private TextView mTasAntSwitchLabel;
    private Spinner mTasAntTxPrx;
    private Spinner mTasAntDrx;
    private Spinner mTasAntDrx1;
    private Spinner mTasAntDrx2;
    private Spinner mTasAntStatesSpinner;

    private Button mQueryIndexOrStatesBtn;
    private Button mEnableStatesBtn;
    private TableRow mAntIndexDRxTableRow;
    private Handler mATCmdHander = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            AsyncResult ar = (AsyncResult) msg.obj;
            switch (msg.what) {
                case QUERY_ANT_INDEX_BY_STATUS:
                    if (ar.exception != null) {
                        EmUtils.showToast("The states not support");
                        Elog.d(TAG, "AT cmd failed,The States not support");
                    } else {
                        String[] receiveDate = (String[]) ar.result;
                        Elog.d(TAG, "receiveDate[0] = " + receiveDate[0]);
                        mTasAntTest.append("\r\n" + receiveDate[0]);
                        int result = parseAntIndex(receiveDate[0]);
                        if (result < 0) {
                            EmUtils.showToast("query ANT index by states failed.");
                            Elog.d(TAG, "query ANT index by states failed.");
                        } else {
                            EmUtils.showToast("query ANT index by states successfully.");
                            Elog.d(TAG, "query ANT index by states successfully.");
                        }
                    }
                    break;

                case QUERY_ANT_STATUS_BY_INDEX:
                    if (ar.exception != null) {
                        EmUtils.showToast("AT cmd failed or The Index not support");
                        Elog.d(TAG, "AT cmd failed or The Index not supportz");
                    } else {
                        String[] receiveDate = (String[]) ar.result;
                        if (receiveDate.length == 0) {
                            Elog.e(TAG, "The Index not support");
                            EmUtils.showToast("The Index not support, Please check the paramer");
                        } else {

                            Elog.d(TAG, "receiveDate[0] = " + receiveDate[0]);
                            mTasAntTest.append("\r\n" + receiveDate[0]);
                            int result = parseAntStaus(receiveDate[0]);
                            if (result < 0) {
                                EmUtils.showToast("Query ANT states by index failed.");
                                Elog.d(TAG, "Query ANT states by index failed.");
                            } else {
                                Elog.d(TAG, "Query ANT states by index successfully.");
                                EmUtils.showToast("Query ANT states by index successfully.");
                            }
                        }
                    }
                    break;

                case FORCE_TX_ENABLE_RAT:
                case FORCE_TX_DISABLE_RAT:
                case FORCE_TX_ENABLE_BAND:
                case FORCE_TX_DISABLED_BAND:
                    if (ar.exception != null) {
                        EmUtils.showToast("AT cmd failed.");
                        Elog.d(TAG, "AT cmd failed.");
                    } else {
                        EmUtils.showToast("AT cmd successfully.");
                        Elog.d(TAG, "AT cmd successfully.");
                    }
                    break;
                case FORCE_TX_QUERY_RAT:
                case FORCE_TX_QUERY_BAND:
                    if (ar.exception == null) {
                        String[] receiveDate = (String[]) ar.result;
                        if (null == receiveDate) {
                            EmUtils.showToast("Warning: Received data is null!");
                            Elog.e(TAG, "Received data is null");
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
                            EmUtils.showToast("Query ANT states succeed,value = " + value);
                            if (value >= 0 && value < 8) {
                                mTasAntStatesSpinner.setSelection(value);
                            } else {
                                mTasAntStatesSpinner.setSelection(8);//none
                            }
                        }
                    } else {
                        EmUtils.showToast("AT cmd failed.");
                        Elog.e(TAG, "Received data is null");
                    }
                    break;
                default:
                    break;
            }
        }
    };
    private OnItemSelectedListener mSpinnerListener = new OnItemSelectedListener() {
        @Override
        public void onItemSelected(AdapterView<?> parent, View arg1, int pos, long arg3) {

            if (parent == mForceAntModeSpinner) {
                Elog.d(TAG, "mForceAntModeSpinner changed, pos = " + pos);
                if (pos == 0) {             //disable

                    mQueryIndexOrStatesBtn.setEnabled(false);

                    mtasMode = (mForceType == FORCE_TYPE_BY_BAND) ?
                            FORCE_TX_DISABLED_BAND : FORCE_TX_DISABLE_RAT;

                    mTasAntTxPrx.setSelection(6);           //empty
                    mTasAntDrx.setSelection(6);             //empty
                    mTasAntDrx1.setSelection(6);            //empty
                    mTasAntDrx2.setSelection(6);            //empty
                    mTasAntStatesSpinner.setSelection(9);//empty
                } else if (pos == 1) {      //enable
                    mQueryIndexOrStatesBtn.setEnabled(true);

                    mtasMode = (mForceType == FORCE_TYPE_BY_BAND) ?
                            FORCE_TX_ENABLE_BAND : FORCE_TX_ENABLE_RAT;
                } else if (pos == 2) {      //read
                    mQueryIndexOrStatesBtn.setEnabled(false);

                    mtasMode = (mForceType == FORCE_TYPE_BY_BAND) ?
                            FORCE_TX_QUERY_BAND : FORCE_TX_QUERY_RAT;
                    mTasAntTxPrx.setSelection(6);
                    mTasAntDrx.setSelection(6);
                    mTasAntDrx1.setSelection(6);
                    mTasAntDrx2.setSelection(6);
                    mTasNvramEdit.setText("");
                    mTasAntStatesSpinner.setSelection(9);//empty
                }
                forceTypeUpdate();
            } else if (parent == mTasRatSpinner) {
                Elog.d(TAG, "mTasRatSpinner changed, pos = " + pos);
                mtasRat = pos + 1;
                if (mtasRat == FORCE_TX_RAT_LTE)
                    mAntIndexDRxTableRow.setVisibility(View.VISIBLE);
                else
                    mAntIndexDRxTableRow.setVisibility(View.GONE);

            } else if (parent == mTasAntStatesSpinner) {
                Elog.d(TAG, "mTasAntStatesSpinner changed, pos = " + pos);
                mtasStates = String.valueOf(pos);
                if (mTasModeEnableStates == 1) {    //query index by states
                    mTasAntTxPrx.setSelection(6);   //none
                    mTasAntDrx.setSelection(6);     //none
                    mTasAntDrx1.setSelection(6);    //none
                    mTasAntDrx2.setSelection(6);    //none
                }
            }
        }

        @Override
        public void onNothingSelected(AdapterView parent) {
        }
    };

    int parseAntStaus(String info) {
        try {
            String states[] = info.split(",");
            int states_value = Integer.parseInt(states[1]);
            if (states_value >= 0 && states_value < 6) {
                mTasAntStatesSpinner.setSelection(states_value);
            } else {
                mTasAntStatesSpinner.setSelection(6);
            }
            return 0;
        } catch (Exception e) {
            return -1;
        }

    }

    int parseAntIndex(String info) {
        try {
            String index[] = info.split(",");
            for (String value : index) {
                Elog.d(TAG, "value = " + value);
            }
            if (index[2].equals("255")) {
                mTasAntTxPrx.setSelection(5);   //ANT 255
            } else {
                mTasAntTxPrx.setSelection(Integer.parseInt(index[2]));
            }
            if (index[3].equals("255")) {
                mTasAntDrx.setSelection(5);   //ANT 255
            } else {
                mTasAntDrx.setSelection(Integer.parseInt(index[3]));
            }
            if (index[4].equals("255")) {
                mTasAntDrx1.setSelection(5);   //ANT 255
            } else {
                mTasAntDrx1.setSelection(Integer.parseInt(index[4]));
            }
            if (index[5].equals("255")) {
                mTasAntDrx2.setSelection(5);   //ANT 255
            } else {
                mTasAntDrx2.setSelection(Integer.parseInt(index[5]));
            }
            return 0;
        } catch (Exception e) {
            return -1;
        }

    }

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.force_tx_95);
        Intent intent = getIntent();
        mQueryMode = intent.getStringExtra("MODE");
        Elog.d(TAG, "MODE = " + mQueryMode);
        mRGForceType = (RadioGroup) findViewById(R.id.force_type);
        mForceRatRadioBtn = (RadioButton) findViewById(R.id.force_rat);
        mForceBandRadioBtn = (RadioButton) findViewById(R.id.force_band);
        mForceAntModeSpinner = (Spinner) findViewById(R.id.force_ant_mode);
        mTasRatSpinner = (Spinner) findViewById(R.id.tas_rat);
        mTasBandEdit = (EditText) findViewById(R.id.tas_band);
        mTasNvramEdit = (EditText) findViewById(R.id.tas_nvram);

        mTasAntStatesSpinner = (Spinner) findViewById(R.id.tas_ant_states);

        mQueryIndexOrStatesBtn = (Button) findViewById(R.id.get_ant_index_states);
        mEnableStatesBtn = (Button) findViewById(R.id.enable_ant_states);

        mTasAntTxPrx = (Spinner) findViewById(R.id.tas_ant_tx_prx);
        mTasAntDrx = (Spinner) findViewById(R.id.tas_ant_drx);
        mTasAntDrx1 = (Spinner) findViewById(R.id.tas_ant_drx1);
        mTasAntDrx2 = (Spinner) findViewById(R.id.tas_ant_drx2);

        mTasAntTest = (TextView) findViewById(R.id.tas_ant_test);
        mTasAntSwitchLabel = (TextView) findViewById(R.id.Ant_switch_label);
        mAntIndexDRxTableRow = (TableRow) findViewById(R.id.TableRow08);
        mTasNvramEdit.setEnabled(false);
        mQueryIndexOrStatesBtn.setOnClickListener(this);
        mEnableStatesBtn.setOnClickListener(this);

        mRGForceType.setOnCheckedChangeListener(this);
        mForceAntModeSpinner.setOnItemSelectedListener(mSpinnerListener);
        mTasRatSpinner.setOnItemSelectedListener(mSpinnerListener);
        mTasAntStatesSpinner.setOnItemSelectedListener(mSpinnerListener);
    }

    @Override
    public void onResume() {
        super.onResume();
        Elog.d(TAG, "onResume");
        getTasSettingStates();

        mForceAntModeSpinner.setSelection((mtasMode < 3) ? mtasMode : mtasMode - 3, true);
        mTasRatSpinner.setSelection(mtasRat - 1, true);

        forceTypeUpdate();

        if (mQueryMode.equals("by_states")) {
            mTasAntSwitchLabel.setText(getString(R.string.get_ant_index));
            mQueryIndexOrStatesBtn.setText(getString(R.string.get_ant_index));
            mTasModeEnableStates = 1;
            mTasAntTxPrx.setEnabled(false);
            mTasAntDrx.setEnabled(false);
            mTasAntDrx1.setEnabled(false);
            mTasAntDrx2.setEnabled(false);
            mTasAntStatesSpinner.setEnabled(true);
        } else if (mQueryMode.equals("by_index")) {
            mTasAntSwitchLabel.setText(getString(R.string.get_ant_states));
            mQueryIndexOrStatesBtn.setText(getString(R.string.get_ant_states));
            mTasModeEnableStates = 2;
            mTasAntTxPrx.setEnabled(true);
            mTasAntDrx.setEnabled(true);
            mTasAntDrx1.setEnabled(true);
            mTasAntDrx2.setEnabled(true);
            mTasAntStatesSpinner.setEnabled(false);
        }
    }

    @Override
    public void onClick(final View arg0) {
        if (arg0 == mEnableStatesBtn) {
            Elog.d(TAG, "Set ANT States click:.");
            mTasModeEnableStates = 0;
        } else if (arg0 == mQueryIndexOrStatesBtn) {
            if (mQueryMode.equals("by_states")) {
                mTasModeEnableStates = 1;
                Elog.d(TAG, "Query ANT index by states click.");
            } else {
                mTasModeEnableStates = 2;
                Elog.d(TAG, "Query ANT states by index click.");
            }
        }
        if (mtasNvram.equals("true") && mTasModeEnableStates == 0) {
            showDialog(DIG_WARING_WRITE_NVRAM);
        } else {
            execTheCmd();
        }
    }

    public void execTheCmd() {
        String tasBand = "";
        String tasNvram = "";
        String tasStates = "";

        if (mForceType == FORCE_TYPE_BY_BAND) {
            mtasBand = mTasBandEdit.getText().toString();
            tasBand = mtasBand;
            if (tasBand.equals("")) {
                EmUtils.showToast("By band mode,the band value should not be empty");
                return;
            }
        } else {
            tasBand = "";
        }

        mtasMode = mForceAntModeSpinner.getSelectedItemPosition()
                + ((mForceType == FORCE_TYPE_BY_BAND) ? 3 : 0);

        tasStates = mtasStates;
        if (mtasMode == FORCE_TX_DISABLE_RAT || mtasMode == FORCE_TX_QUERY_RAT ||
                mtasMode == FORCE_TX_DISABLED_BAND || mtasMode == FORCE_TX_QUERY_BAND) {
            tasStates = "";
        } else if ((mTasModeEnableStates != 2) &&
                (mtasMode == FORCE_TX_ENABLE_RAT || mtasMode == FORCE_TX_ENABLE_BAND)) {
            if (mtasStates.equals("8") || mtasStates.equals("9")) {
                EmUtils.showToast("The ANT states should be 0~7");
                return;
            }
        }

        tasNvram = (mtasNvram.equals("true") ? "1" : "0");
        Elog.d(TAG, "mForceType = " + mForceType + "," + mTypeArray[mForceType]);
        Elog.d(TAG, "mtasMode = " + mtasMode + ","
                + mModeArray[(mtasMode < 3) ? mtasMode : mtasMode - 3]);

        Elog.d(TAG, "mtasRat = " + mtasRat + "," + mRatArray[mtasRat - 1]);
        Elog.d(TAG, "mtasNvram = " + tasNvram);
        Elog.d(TAG, "tasBand = " + tasBand);
        Elog.d(TAG, "mtasStates = " + tasStates);

        writeTasSettingStates();

        if (mTasModeEnableStates == 1) {
            queryIndexByStates(mtasRat, tasBand, tasStates);
        } else if (mTasModeEnableStates == 2) {
            queryStatesByIndex(mtasRat, tasBand);
        } else {
            setTasForceIdx(mtasMode, mtasRat, tasStates, tasBand, tasNvram);
        }
    }

    public void setTasForceIdx(int mode, int rat,
                               String antenna_idx, String band, String nvram_write) {
        String[] cmd = new String[2];

        if (mForceType == FORCE_TYPE_BY_BAND) {
            cmd[0] = "AT+ETXANT=" + mode + "," + rat + "," + antenna_idx + "," + band;
        } else
            cmd[0] = "AT+ETXANT=" + mode + "," + rat + "," + antenna_idx + "," + band + "," +
                    nvram_write;

        cmd[1] = "+ETXANT:";
        Elog.d(TAG, "sendAtCommand: " + cmd[0]);
        mTasAntTest.setText(cmd[0]);
        senAtCommand(cmd, mode);
    }

    //AT+EGMC=1, “gsm_utas”,  <gsm_band> <rx_count>, <rx0_ant>, <rx1_ant>, <tx_ant>
    //AT+EGMC=1,“wcdma_utas",1 ,1(band), 2/4(rx-count), 1(rx0), 2(rx1), 0(rx2),0(rx3), 1(tx)
    //AT+EGMC=1, “t_utas_query", <band_info> 1<rx_count>, <rx0_ant>
    //AT+EGMC=1, “c2k_utas”, [<band_info> ,<rx_count>, <rx0_ant>, <rx1_ant>
    //AT+EGMC=1,“l_utas_query",1 ,1(band), 2/4(rx-count), 1(rx0), 2(rx1), 0(rx2),0(rx3), 1(tx)
    boolean isTasStatesValid(Spinner mTasAntTxPrx) {
        if (mTasAntTxPrx.getSelectedItemPosition() == 5
                || mTasAntTxPrx.getSelectedItemPosition() == 6) {
            return false;
        } else {
            return true;
        }
    }

    public void queryStatesByIndex(int rat, String band) {
        String[] cmd = new String[2];
        String configureName[] = {"\"gsm_utas\"", "\"wcdma_utas\"", "\"t_utas_query\"",
                "\"c2k_utas\"",
                "\"l_utas_query\""};
        String ratMapping[] = {"", "0", "1", "4", "3"};
        int modemType = ModemCategory.getModemType();
        if (modemType == ModemCategory.MODEM_FDD) {
            ratMapping[2] = "1";        //wcdma
            Elog.d(TAG, "3G WCDMA");
        } else {
            ratMapping[2] = "2";        //td
            Elog.d(TAG, "3G TDSCDMA");
        }
        int ratIndex = Integer.parseInt(ratMapping[rat]);
        int tx_count = 0;
        int tasAntTxPrx = 255;
        int tasAntDrx = 255;
        int tasAntDrx1 = 255;
        int tasAntDrx2 = 255;

        if (!isTasStatesValid(mTasAntTxPrx)) {
            EmUtils.showToast("The TxPrxshould not be 255 or null");
            return;
        }
        tasAntTxPrx = mTasAntTxPrx.getSelectedItemPosition();
        if (ratIndex == 0) {        //GSM
            tasAntTxPrx += 1;
            if (!isTasStatesValid(mTasAntDrx)) {
                tx_count = 1;
                tasAntDrx = 0;
            } else {
                tx_count = 2;
                tasAntDrx = mTasAntDrx.getSelectedItemPosition() + 1;
            }
            cmd[0] = "AT+EGMC=1," + configureName[ratIndex] + "," + band + "," + tx_count + ",";
            if (tx_count == 1) {
                cmd[0] += tasAntTxPrx + "," + "," + tasAntTxPrx;
            } else {
                cmd[0] += tasAntTxPrx + "," + tasAntDrx + "," + tasAntTxPrx;
            }

        } else if (ratIndex == 1) {     //wcdma
            tasAntTxPrx += 1;
            if (!isTasStatesValid(mTasAntDrx)) {
                tx_count = 1;
                tasAntDrx = 255;
                tasAntDrx1 = 0;
                tasAntDrx2 = 0;
            } else {
                tx_count = 2;
                tasAntDrx = mTasAntDrx.getSelectedItemPosition() + 1;
                tasAntDrx1 = 0;
                tasAntDrx2 = 0;
            }
            cmd[0] = "AT+EGMC=1," + configureName[ratIndex] + ",1," + band + "," + tx_count + ",";
            cmd[0] += tasAntTxPrx + "," + tasAntDrx + "," + tasAntDrx1 + "," + tasAntDrx2 + "," +
                    tasAntTxPrx;
        } else if (ratIndex == 2) {    //tdscdma
            tx_count = 1;
            if (isTasStatesValid(mTasAntDrx)) {
                EmUtils.showToast("The tdscdma only support 1 path： prx trx");
                return;
            }
            cmd[0] = "AT+EGMC=1," + configureName[ratIndex] + "," + band + "," + tx_count + "," +
                    tasAntTxPrx;
        } else if (ratIndex == 3) {    //c2k
            tasAntTxPrx += 1;
            if (!isTasStatesValid(mTasAntDrx)) {
                tx_count = 1;
                tasAntDrx = 255;
            } else {
                tx_count = 2;
                tasAntDrx = mTasAntDrx.getSelectedItemPosition() + 1;
            }
            cmd[0] = "AT+EGMC=1," + configureName[ratIndex] + "," + band + "," + tx_count + ",";
            cmd[0] += tasAntTxPrx + "," + tasAntDrx;
        } else if (ratIndex == 4) {             //lte
            if (!isTasStatesValid(mTasAntDrx)) {
                EmUtils.showToast("The mTasAntDrx not be 255 or null in lte mode");
                return;
            } else {
                tasAntTxPrx += 1;
                if ((isTasStatesValid(mTasAntDrx1) && !isTasStatesValid(mTasAntDrx2))
                        || (!isTasStatesValid(mTasAntDrx1) && isTasStatesValid(mTasAntDrx2))) {
                    EmUtils.showToast("TasAntDrx1 or TasAntDrx2 should both 255 or not");
                } else if (isTasStatesValid(mTasAntDrx1) && isTasStatesValid(mTasAntDrx2)) {
                    tx_count = 4;
                    tasAntDrx = mTasAntDrx.getSelectedItemPosition() + 1;
                    tasAntDrx1 = mTasAntDrx1.getSelectedItemPosition() + 1;
                    tasAntDrx2 = mTasAntDrx2.getSelectedItemPosition() + 1;
                } else {
                    tx_count = 2;
                    tasAntDrx = mTasAntDrx.getSelectedItemPosition() + 1;
                    tasAntDrx1 = 0;
                    tasAntDrx2 = 0;
                }
                cmd[0] = "AT+EGMC=1," + configureName[ratIndex] + ",1," + band + ","
                        + tx_count + ",";
                cmd[0] += tasAntTxPrx + "," + tasAntDrx + "," + tasAntDrx1 + "," + tasAntDrx2
                        + "," + tasAntTxPrx;
            }
        }

        cmd[1] = "+EGMC:";
        Elog.d(TAG, "sendAtCommand: " + cmd[0]);
        mTasAntTest.setText(cmd[0]);
        senAtCommand(cmd, QUERY_ANT_STATUS_BY_INDEX);
    }


    //AT+EGMC = 1, "utas_query_ant_port", <rat>, <band>, <ANT state>
    public void queryIndexByStates(int rat, String band, String tasStates) {
        String[] cmd = new String[2];
        String ratMapping[] = {"", "0", "1", "4", "3"};
        cmd[0] = "AT+EGMC=1,\"utas_query_ant_port\"" + "," + ratMapping[rat] + "," + band + "," +
                tasStates;
        cmd[1] = "+EGMC:";
        Elog.d(TAG, "sendAtCommand: " + cmd[0]);
        mTasAntTest.setText(cmd[0]);
        senAtCommand(cmd, QUERY_ANT_INDEX_BY_STATUS);
    }

    public void senAtCommand(String[] atCommand, int msg) {
        EmUtils.invokeOemRilRequestStringsEm(atCommand, mATCmdHander.obtainMessage(msg));
    }

    public void getTasSettingStates() {
        final SharedPreferences tasVersionSh = getSharedPreferences(SHREDPRE_NAME,
                MODE_PRIVATE);
        mForceType = tasVersionSh.getInt(TAS_TYPE, FORCE_TYPE_BY_RAT);
        mtasMode = tasVersionSh.getInt(TAS_MODE, 0);
        mtasRat = tasVersionSh.getInt(TAS_RAT, 1);
        mtasNvram = tasVersionSh.getString(TAS_NVRAM, "true");
        mtasBand = tasVersionSh.getString(TAS_BAND, "1");

        Elog.d(TAG, "\ngetTasSettingStates: ");
        Elog.d(TAG, "mForceType = " + mForceType);
        Elog.d(TAG, "mtasMode = " + mtasMode);
        Elog.d(TAG, "mtasRat = " + mtasRat);
        Elog.d(TAG, "mtasNvram = " + mtasNvram);
        Elog.d(TAG, "mtasBand = " + mtasBand);
    }

    public void writeTasSettingStates() {
        final SharedPreferences tasVersionSh = getSharedPreferences(SHREDPRE_NAME,
                MODE_PRIVATE);
        final SharedPreferences.Editor editor = tasVersionSh.edit();

        editor.putInt(TAS_TYPE, mForceType);
        editor.putInt(TAS_MODE, mtasMode);
        editor.putInt(TAS_RAT, mtasRat);
        editor.putString(TAS_NVRAM, mtasNvram);
        editor.putString(TAS_BAND, mtasBand);
        editor.commit();
    }

    void forceTypeUpdate() {
        if (mForceType == FORCE_TYPE_BY_RAT) {
            mForceRatRadioBtn.setChecked(true);
            mTasNvramEdit.setText("true");
            mtasNvram = "true";
            mTasBandEdit.setEnabled(false);
            mTasBandEdit.setText("all band");
        } else if (mForceType == FORCE_TYPE_BY_BAND) {
            mForceBandRadioBtn.setChecked(true);
            mTasNvramEdit.setText("false");
            mtasNvram = "false";
            mTasBandEdit.setEnabled(true);
            mTasBandEdit.setText(mtasBand);
        }

        if (mtasMode == FORCE_TX_QUERY_RAT || mtasMode == FORCE_TX_QUERY_BAND) {       //read
            mTasNvramEdit.setText("");
            mtasNvram = "false";
        }

        if (mForceType == FORCE_TYPE_BY_BAND &&
                (mtasMode == FORCE_TX_ENABLE_RAT || mtasMode == FORCE_TX_ENABLE_BAND)) {
            mQueryIndexOrStatesBtn.setEnabled(true);
        } else {
            mQueryIndexOrStatesBtn.setEnabled(false);
        }
    }


    @Override
    public void onCheckedChanged(RadioGroup arg0, int checkedId) {
        if (checkedId == R.id.force_rat) {
            mForceType = FORCE_TYPE_BY_RAT;
        } else if (checkedId == R.id.force_band) {
            mForceType = FORCE_TYPE_BY_BAND;
        }
        Elog.d(TAG, "forceTypeChanged, mForceType = " + mForceType + "," + mTypeArray[mForceType]);
        forceTypeUpdate();
    }

    @Override
    protected Dialog onCreateDialog(int id) {
        switch (id) {
            case DIG_WARING_WRITE_NVRAM:
                return new AlertDialog.Builder(this)
                        .setTitle("Warning")
                        .setMessage("Enable force state for all bands and" +
                                " write force state to NVRAM\n")
                        .setPositiveButton("Confirm", new DialogInterface.OnClickListener() {
                            @Override
                            public void onClick(DialogInterface dialog, int which) {
                                if (which == DialogInterface.BUTTON_POSITIVE) {
                                    execTheCmd();
                                }
                                dialog.dismiss();
                            }
                        })
                        .setNegativeButton("Cancel", null)
                        .create();

        }
        return super.onCreateDialog(id);
    }
}
