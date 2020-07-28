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

package com.mediatek.engineermode.antenna;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.Dialog;
import android.os.AsyncResult;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemSelectedListener;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.CompoundButton;
import android.widget.LinearLayout;
import android.widget.RadioButton;
import android.widget.RadioGroup;
import android.widget.Spinner;
import android.widget.Toast;

import com.mediatek.engineermode.Elog;
import com.mediatek.engineermode.EmUtils;
import com.mediatek.engineermode.FeatureSupport;
import com.mediatek.engineermode.ModemCategory;
import com.mediatek.engineermode.R;

import java.util.Arrays;

public class AntennaActivity extends Activity {
    private static final String TAG = "AntennaActivity";

    private static final int MSG_QUERY_ANTENNA_MODE = 1;
    private static final int MSG_SET_ANTENNA_MODE = 2;
    private static final int MSG_QUERY_ANTENNA_MODE_C2K = 4;
    private static final int MSG_QUERY_ANTENNA_EGMC_4G = 5;
    private static final int MSG_SET_ANTENNA_EGMC_4G = 6;
    private static final int MSG_INIT_ANTENNA_EGMC_4G = 7;
    private static final int MODE_INDEX_BASE_3G = 10;
    private static final int MODE_INDEX_BASE_2G = 20;
    private static final int MODE_2G_COUNT = 4;
    private static final int MODE_EPCM_VALID = 0xFF;
    private static final int CELL_2RX_LENGTH = 2;
    private static final int CELL_4RX_LENGTH = 4;
    private static final String CMD_QUERY_EGMC_4G = "AT+EGMC=0,\"rx_path\"";
    private static final String CMD_SET_EGMC_4G = "AT+EGMC=1,\"rx_path\"";
    private static final String CMD_INIT_EGMC_4G = "AT+EGMC=1,\"rx_path\",1,0,3,15,3,15";
    private static final int DIALOG_EGMC_4G_SET_FAIL = 0;


    private LinearLayout antennaLayout4G;
    private LinearLayout antennaLayout2G;
    private LinearLayout antennaLayoutEgmc4G;
    private Spinner mSpinner4G = null;
    private Spinner mSpinner3G = null;
    private Spinner mSpinner2G = null;
    private Spinner mSpinnerC2kMode = null;

    private Button queryEgmcBtn;
    private Button setEgmcBtn;
    private RadioGroup egmcForceRxMode;
    private AntennaEgmc4GInfo antEgmc4GObj;
    private RadioGroup.OnCheckedChangeListener antEgmc4GModeChangeListener;
    private CompoundButton.OnCheckedChangeListener antEgmc4GStatusChangeListener;
    private OnClickListener mOnClickListener;

    private RadioGroup sccFollowPccMode;
    private CheckBox[] pCell2Rx = new CheckBox[CELL_2RX_LENGTH];
    private CheckBox[] pCell4Rx = new CheckBox[CELL_4RX_LENGTH];
    private CheckBox[] sCell2Rx = new CheckBox[CELL_2RX_LENGTH];
    private CheckBox[] sCell4Rx = new CheckBox[CELL_4RX_LENGTH];

    private boolean G95Valid = false;

    private Toast mToast = null;

    private int mCurrentPos = 0, mCurrent3GPos = 0, mCurrentPosCdma = 0, mCurrent2GPos = 0;

    private final OnItemSelectedListener mItemSelectedListener = new OnItemSelectedListener() {
        @Override
        public void onItemSelected(AdapterView<?> arg0, View arg1, int pos, long arg3) {
            if (arg0 == mSpinner4G) {
                if (mCurrentPos == arg0.getSelectedItemPosition()) {
                    return;
                }
                mCurrentPos = arg0.getSelectedItemPosition();
                setMode(pos);
            } else if (arg0 == mSpinner3G) {
                if (mCurrent3GPos == arg0.getSelectedItemPosition() || pos == 0) {
                    return;
                }
                mCurrent3GPos = arg0.getSelectedItemPosition();
                setMode(MODE_INDEX_BASE_3G + pos - 1);
            } else if (arg0 == mSpinnerC2kMode) {
                if (mCurrentPosCdma == arg0.getSelectedItemPosition()) {
                    return;
                }
                mCurrentPosCdma = arg0.getSelectedItemPosition();
                setCdmaMode(pos);
            } else if(arg0 == mSpinner2G) {
                if (mCurrent2GPos == arg0.getSelectedItemPosition() || pos == 0) {
                    return;
                }
                mCurrent2GPos = arg0.getSelectedItemPosition();
                setMode(MODE_INDEX_BASE_2G + pos - 1);
            }
        }

        @Override
        public void onNothingSelected(AdapterView<?> arg0) {
            // Do nothing
        }
    };

    private final Handler mCommandHander = new Handler() {
        @Override
        public void handleMessage(final Message msg) {
            AsyncResult asyncResult;
            switch (msg.what) {
            case MSG_QUERY_ANTENNA_MODE:
                asyncResult = (AsyncResult) msg.obj;
                if (asyncResult != null && asyncResult.exception == null
                        && asyncResult.result != null) {
                    final String[] result = (String[]) asyncResult.result;
                    Elog.d(TAG, "Query Ant Mode return: " + Arrays.toString(result));
                    antennaLayout2G.setVisibility(View.GONE);
                    for (int i = 0; i < result.length; i++) {
                        int mode = parseCurrentMode(result[i]);
                        if (isMode2G(mode)) {
                            antennaLayout2G.setVisibility(View.VISIBLE);
                        }
                    }
                } else {
                    EmUtils.showToast("Query 3G/4G antenna currect mode failed.",
                            Toast.LENGTH_SHORT);
                    Elog.e(TAG, "Query 3G/4G antenna currect mode failed.");
                }
                break;

            case MSG_QUERY_ANTENNA_MODE_C2K:
                asyncResult = (AsyncResult) msg.obj;
                if (asyncResult != null && asyncResult.exception == null
                        && asyncResult.result != null) {
                    final String[] result = (String[]) asyncResult.result;
                    Elog.d(TAG, "Query Ant Mode C2K return: " + Arrays.toString(result));
                    for (int i = 0; i < result.length; i++) {
                        parseCurrentCdmaMode(result[i]);
                    }
                } else {
                    EmUtils.showToast("Query cdma antenna currect mode failed.",
                            Toast.LENGTH_SHORT);
                    Elog.e(TAG, "Query cdma antenna currect mode failed.");
                }
                break;
            case MSG_SET_ANTENNA_MODE:
                asyncResult = (AsyncResult) msg.obj;
                if (asyncResult.exception == null) {
                    EmUtils.showToast("Set successful.",Toast.LENGTH_SHORT);

                    Elog.d(TAG, "set antenna mode succeed.");
                } else {
                    EmUtils.showToast("Set failed.",Toast.LENGTH_SHORT);
                    Elog.e(TAG, "set antenna mode failed.");
                }
                break;
            case MSG_QUERY_ANTENNA_EGMC_4G:
                asyncResult = (AsyncResult) msg.obj;
                if (asyncResult != null && asyncResult.exception == null
                        && asyncResult.result != null) {
                    final String[] result = (String[]) asyncResult.result;
                    Elog.d(TAG, "Query Ant Mode EGMC 4G return: " + Arrays.toString(result));
                    for (int i = 0; i < result.length; i++) {
                        parseCurEgmc4GMode(result[i]);
                    }
                } else {
                    EmUtils.showToast("Query EGMC 4G currect mode failed.", Toast.LENGTH_SHORT);
                    Elog.e(TAG, "Query EGMC 4G antenna currect mode failed.");
                }
                break;
            case MSG_SET_ANTENNA_EGMC_4G:
                asyncResult = (AsyncResult) msg.obj;
                if (asyncResult.exception == null) {
                    EmUtils.showToast("Set EGMC 4G successful.", Toast.LENGTH_SHORT);
                    Elog.d(TAG, "set EGMC 4G antenna mode succeed.");
                } else {
                    EmUtils.showToast("Set EGMC 4G failed.", Toast.LENGTH_SHORT);
                    Elog.e(TAG, "set EGMC 4G antenna mode failed.");
                }
                break;
            case MSG_INIT_ANTENNA_EGMC_4G:
                asyncResult = (AsyncResult) msg.obj;
                if (asyncResult.exception == null) {
                    setEgmcAnt4GForUser();
                    Elog.d(TAG, "Init EGMC 4G antenna mode succeed.");
                } else {
                    EmUtils.showToast("Init EGMC 4G failed.", Toast.LENGTH_SHORT);
                    Elog.e(TAG, "Init EGMC 4G antenna mode failed.");
                }
                break;
            default:
                break;
            }
        }

    };

    private int parseCurrentMode(String data) {
        // query result is like: +ERXPATH: 1
        int mode = -1;
        Elog.i(TAG, "parseCurrentMode data= " + data);

        try {
            mode = Integer.valueOf(data.substring("+ERXPATH:".length()).trim());
        } catch (Exception e) {
            Elog.e(TAG, "Wrong current mode format: " + data);
        }


        if (mode < 0 || ((mode >= MODE_INDEX_BASE_2G + MODE_2G_COUNT ||
                (mode >= mSpinner4G.getCount() && mode < MODE_INDEX_BASE_3G) ||
                (mode >= MODE_INDEX_BASE_3G + mSpinner3G.getCount() &&
                mode < MODE_INDEX_BASE_2G)) &&
                mode != MODE_EPCM_VALID)) {
            EmUtils.showToast("Modem returned invalid mode: " + data,Toast.LENGTH_SHORT);
            return -1;
        } else {
            if (mode == MODE_EPCM_VALID){
                antennaLayout4G.setVisibility(View.GONE);
                antennaLayoutEgmc4G.setVisibility(View.VISIBLE);
                queryEgmcAnt4G();
            } else if(mode >= MODE_INDEX_BASE_2G) {
                mCurrent2GPos = mode - MODE_INDEX_BASE_2G + 1;
                Elog.d(TAG, "parseCurrent2GMode is: " + mCurrent2GPos);
                mSpinner2G.setSelection(mCurrent2GPos);
            } else if (mode >= MODE_INDEX_BASE_3G) {
                mCurrent3GPos = mode - MODE_INDEX_BASE_3G + 1;
                Elog.d(TAG, "parseCurrent3GMode is: " + mCurrent3GPos);
                mSpinner3G.setSelection(mCurrent3GPos);
            } else {
                antennaLayout4G.setVisibility(View.VISIBLE);
                antennaLayoutEgmc4G.setVisibility(View.GONE);
                Elog.d(TAG, "parseCurrentLteMode is: " + mode);
                mCurrentPos = mode;
                mSpinner4G.setSelection(mode);
                mSpinner4G.setEnabled(true);
            }
        }
        return mode;
    }

    private boolean isMode2G(int mode) {
        if(mode >= MODE_INDEX_BASE_2G &&
                mode < MODE_INDEX_BASE_2G + mSpinner2G.getCount() - 1)
            return true;
        return false;
    }

    private boolean isMode3G(int mode) {
        if(mode >= MODE_INDEX_BASE_2G &&
                mode < MODE_INDEX_BASE_2G + mSpinner2G.getCount())
            return true;
        return false;
    }
    private int parseCurrentCdmaMode(String data) {
        // query result is like: +ERXTESTMODE: 1
        int mode = -1;
        Elog.d(TAG, "parseCurrentCdmaMode data= " + data);
        try {
            mode = Integer.valueOf(data.substring("+ERXTESTMODE:".length()).trim());
        } catch (Exception e) {
            Elog.e(TAG, "Wrong current mode format: " + data);
        }

        if (mode < 0 ||
           (mode >= mSpinnerC2kMode.getCount())) {
            EmUtils.showToast("Modem returned invalid mode: " + data,Toast.LENGTH_SHORT);
            return -1;
        } else {
            mCurrentPosCdma = mode;
            Elog.d(TAG, "parseCurrentCDMAMode is: " + mCurrentPosCdma);
            mSpinnerC2kMode.setSelection(mode);
            mSpinnerC2kMode.setEnabled(true);
        }
        return mode;
    }


    private void parseCurEgmc4GMode(String data) {
        // TODO Auto-generated method stub
        int mode = -1;
        Elog.d(TAG, "parseCurrentMode data= " + data);
        try {
            String rxPath = data.substring("+EGMC:".length()).trim();
            String[] rxDataArray = rxPath.split(",");
            if(rxDataArray.length < 6 || rxDataArray.length > 7) return;
            int start = rxDataArray.length == 7 ? 1 : 0;
            if(antEgmc4GObj == null) {
                antEgmc4GObj = new AntennaEgmc4GInfo(Integer.valueOf(rxDataArray[start]),
                        Integer.valueOf(rxDataArray[start+1]), Integer.valueOf(rxDataArray[start+2]),
                        Integer.valueOf(rxDataArray[start+3]), Integer.valueOf(rxDataArray[start+4]),
                        Integer.valueOf(rxDataArray[start+5]));
            } else {
                antEgmc4GObj.updateAntennaEgmc4GInfo(Integer.valueOf(rxDataArray[start]),
                        Integer.valueOf(rxDataArray[start+1]), Integer.valueOf(rxDataArray[start+2]),
                        Integer.valueOf(rxDataArray[start+3]), Integer.valueOf(rxDataArray[start+4]),
                        Integer.valueOf(rxDataArray[start+5]));
            }
            updateAntEgmc4GView();
        } catch (Exception e) {
            Elog.e(TAG, "Wrong current mode format: " + data);
        }
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.antena_test);

        initAnt4GView();
        initAnt3GView();
        initAntC2KView();
        initAnt2GView();
    }

    public void initAntEgmc4GView() {
        antEgmc4GModeChangeListener =
                new RadioGroup.OnCheckedChangeListener() {

            @Override
            public void onCheckedChanged(RadioGroup group, int checkedId) {
                if(antEgmc4GObj == null) return;
                switch (checkedId) {
                    case R.id.force_rx_enable:
                        antEgmc4GObj.setForceRx(true);
                        break;
                    case R.id.force_rx_disable:
                        antEgmc4GObj.setForceRx(false);
                        break;
                    case R.id.scc_follow_pcc_enable:
                        antEgmc4GObj.setCssFollowPcc(true);
                        break;
                    case R.id.scc_follow_pcc_disable:
                        antEgmc4GObj.setCssFollowPcc(false);
                        break;
                    default:
                        break;
                }
            }
        };
        antEgmc4GStatusChangeListener = new CompoundButton.OnCheckedChangeListener() {

            @Override
            public void onCheckedChanged(CompoundButton buttonView,
                    boolean isChecked) {
                if(antEgmc4GObj == null) {
                    antEgmc4GObj = new AntennaEgmc4GInfo();
                }
                switch(buttonView.getId()){
                case R.id.pcell_2rx_rx1:
                    antEgmc4GObj.updatePCell2Rx(isChecked, 0);
                    break;
                case R.id.pcell_2rx_rx2:
                    antEgmc4GObj.updatePCell2Rx(isChecked, 1);
                    break;
                case R.id.pcell_4rx_rx1:
                    antEgmc4GObj.updatePCell4Rx(isChecked, 0);
                    break;
                case R.id.pcell_4rx_rx2:
                    antEgmc4GObj.updatePCell4Rx(isChecked, 1);
                    break;
                case R.id.pcell_4rx_rx3:
                    antEgmc4GObj.updatePCell4Rx(isChecked, 2);
                    break;
                case R.id.pcell_4rx_rx4:
                    antEgmc4GObj.updatePCell4Rx(isChecked, 3);
                    break;
                case R.id.scell_2rx_rx1:
                    antEgmc4GObj.updateSCell2Rx(isChecked, 0);
                    break;
                case R.id.scell_2rx_rx2:
                    antEgmc4GObj.updateSCell2Rx(isChecked, 1);
                    break;
                case R.id.scell_4rx_rx1:
                    antEgmc4GObj.updateSCell4Rx(isChecked, 0);
                    break;
                case R.id.scell_4rx_rx2:
                    antEgmc4GObj.updateSCell4Rx(isChecked, 1);
                    break;
                case R.id.scell_4rx_rx3:
                    antEgmc4GObj.updateSCell4Rx(isChecked, 2);
                    break;
                case R.id.scell_4rx_rx4:
                    antEgmc4GObj.updateSCell4Rx(isChecked, 3);
                    break;
                default:
                    break;
                }
            }
        };
        mOnClickListener = new OnClickListener() {

            @Override
            public void onClick(View v) {

                switch(v.getId()) {
                case R.id.antenna_query_4g:
                    queryEgmcAnt4G();
                    break;
                case R.id.antenna_set_4g:
                    setEgmcAnt4G();
                    break;
                default:
                    Elog.d(TAG, "OnClickListener: " + v.getLabelFor());
                    break;
                }
            }
        };
        queryEgmcBtn = (Button) findViewById(R.id.antenna_query_4g);
        queryEgmcBtn.setOnClickListener(mOnClickListener);
        setEgmcBtn = (Button) findViewById(R.id.antenna_set_4g);
        setEgmcBtn.setOnClickListener(mOnClickListener);
        egmcForceRxMode = (RadioGroup) findViewById(R.id.force_rx_mode);
        sccFollowPccMode = (RadioGroup) findViewById(R.id.scc_follow_pcc_mode);
        egmcForceRxMode.setOnCheckedChangeListener(antEgmc4GModeChangeListener);
        sccFollowPccMode.setOnCheckedChangeListener(antEgmc4GModeChangeListener);
        pCell2Rx[0] = (CheckBox)findViewById(R.id.pcell_2rx_rx1);
        pCell2Rx[1] = (CheckBox)findViewById(R.id.pcell_2rx_rx2);
        pCell4Rx[0] = (CheckBox)findViewById(R.id.pcell_4rx_rx1);
        pCell4Rx[1] = (CheckBox)findViewById(R.id.pcell_4rx_rx2);
        pCell4Rx[2] = (CheckBox)findViewById(R.id.pcell_4rx_rx3);
        pCell4Rx[3] = (CheckBox)findViewById(R.id.pcell_4rx_rx4);
        sCell2Rx[0] = (CheckBox)findViewById(R.id.scell_2rx_rx1);
        sCell2Rx[1] = (CheckBox)findViewById(R.id.scell_2rx_rx2);
        sCell4Rx[0] = (CheckBox)findViewById(R.id.scell_4rx_rx1);
        sCell4Rx[1] = (CheckBox)findViewById(R.id.scell_4rx_rx2);
        sCell4Rx[2] = (CheckBox)findViewById(R.id.scell_4rx_rx3);
        sCell4Rx[3] = (CheckBox)findViewById(R.id.scell_4rx_rx4);
        for(int i=0; i<CELL_2RX_LENGTH; i++) {
            pCell2Rx[i].setOnCheckedChangeListener(antEgmc4GStatusChangeListener);
            sCell2Rx[i].setOnCheckedChangeListener(antEgmc4GStatusChangeListener);
        }
        for(int i=0; i<CELL_4RX_LENGTH; i++) {
            pCell4Rx[i].setOnCheckedChangeListener(antEgmc4GStatusChangeListener);
            sCell4Rx[i].setOnCheckedChangeListener(antEgmc4GStatusChangeListener);
        }
    }


    public void updateAntEgmc4GView(){
        antennaLayout4G.setVisibility(View.GONE);
        antennaLayoutEgmc4G.setVisibility(View.VISIBLE);
        if(antEgmc4GObj == null) return;
        ((RadioButton) findViewById(R.id.force_rx_enable)).setChecked(antEgmc4GObj.getForceRx());
        ((RadioButton) findViewById(R.id.force_rx_disable)).setChecked(!antEgmc4GObj.getForceRx());
        ((RadioButton) findViewById(R.id.scc_follow_pcc_enable)).setChecked(antEgmc4GObj.getCssFollowPcc());
        ((RadioButton) findViewById(R.id.scc_follow_pcc_disable)).setChecked(!antEgmc4GObj.getCssFollowPcc());
        for(int i=0; i<CELL_2RX_LENGTH; i++) {
            pCell2Rx[i].setChecked(antEgmc4GObj.getBoolArrayPCell2Rx()[i]);
            sCell2Rx[i].setChecked(antEgmc4GObj.getBoolArraySCell2Rx()[i]);
        }
        for(int i=0; i<CELL_4RX_LENGTH; i++) {
            pCell4Rx[i].setChecked(antEgmc4GObj.getBoolArrayPCell4Rx()[i]);
            sCell4Rx[i].setChecked(antEgmc4GObj.getBoolArraySCell4Rx()[i]);
        }
    }

    public void queryEgmcAnt4G() {
        String[] cmd = new String[2];
        cmd[0] = CMD_QUERY_EGMC_4G;
        cmd[1] = "+EGMC:";
        sendCommand(cmd,MSG_QUERY_ANTENNA_EGMC_4G);
    }

    public void setEgmcAnt4G() {
        if(antEgmc4GObj.getIntForceRx() == 0) {
            antEgmc4GObj.CloseForceRxFor4G();
            updateAntEgmc4GView();
        }
        initEgmcAnt4G();
    }

    public void setEgmcAnt4GForUser() {
        String[] cmd = new String[2];
        if (!antEgmc4GObj.getForceRx()) {
            cmd[0] = CMD_SET_EGMC_4G+"," + 0;
        } else if(!checkValidEgmc4GInfo()){
            return;
        } else {
            cmd[0] = CMD_SET_EGMC_4G+"," + antEgmc4GObj.getIntForceRx()+"," +
                antEgmc4GObj.getIntCssFollowPcc()+"," + antEgmc4GObj.getPCell2Rx()+"," +
                antEgmc4GObj.getPCell4Rx()+"," + antEgmc4GObj.getSCell2Rx()+"," +
                antEgmc4GObj.getSCell4Rx();
        }
        cmd[1] = "";
        sendCommand(cmd,MSG_SET_ANTENNA_EGMC_4G);
    }

    private void initEgmcAnt4G() {
        String[] cmd = new String[2];
        cmd[0] = CMD_INIT_EGMC_4G;
        cmd[1] = "";
        sendCommand(cmd, MSG_INIT_ANTENNA_EGMC_4G);
    }

    private boolean checkValidEgmc4GInfo() {
        if(antEgmc4GObj == null) return false;
        if(antEgmc4GObj.getPCell4Rx() == 0 || antEgmc4GObj.getPCell2Rx() == 0) {
            showDialog(DIALOG_EGMC_4G_SET_FAIL,
                    this.getResources().getString(R.string.antenna_title_4g),
                    this.getResources().getString(R.string.pcell_set_rx0_warn));
            return false;
        }
        if(antEgmc4GObj.getSCell4Rx() == 0 || antEgmc4GObj.getSCell2Rx() == 0) {
            showDialog(DIALOG_EGMC_4G_SET_FAIL,
                    this.getResources().getString(R.string.antenna_title_4g),
                    this.getResources().getString(R.string.scell_set_rx0_warn));
            return false;
        }
        if(antEgmc4GObj.getPCell4Rx() == 7 || antEgmc4GObj.getPCell4Rx() == 11 ||
                antEgmc4GObj.getPCell4Rx() == 13 || antEgmc4GObj.getPCell4Rx() == 14) {
            showDialog(DIALOG_EGMC_4G_SET_FAIL,
                    this.getResources().getString(R.string.antenna_title_4g),
                    this.getResources().getString(R.string.pcell_set_rx3_warn));
            return false;
        }
        if(antEgmc4GObj.getSCell4Rx() == 7 || antEgmc4GObj.getSCell4Rx() == 11 ||
                antEgmc4GObj.getSCell4Rx() == 13 || antEgmc4GObj.getSCell4Rx() == 14) {
            showDialog(DIALOG_EGMC_4G_SET_FAIL,
                    this.getResources().getString(R.string.antenna_title_4g),
                    this.getResources().getString(R.string.scell_set_rx3_warn));
            return false;
        }
        return true;
    }

    protected Dialog showDialog(int id, String title, String info) {
        switch (id) {
        case DIALOG_EGMC_4G_SET_FAIL: // Fail
            return new AlertDialog.Builder(AntennaActivity.this)
                    .setTitle(title)
                    .setMessage(info)
                    .setPositiveButton(android.R.string.ok, null).show();
        default:
            return null;
        }
    }
    public void initAnt4GView() {
        antennaLayout4G = (LinearLayout) findViewById(R.id.antenna_4g);
        antennaLayoutEgmc4G = (LinearLayout) findViewById(R.id.antanna_4g_egmc);
        mSpinner4G = (Spinner) findViewById(R.id.spinner_antenna_4g);
        mSpinner4G.setOnItemSelectedListener(mItemSelectedListener);
        mSpinner4G.setEnabled(false);

        initAntEgmc4GView();

        if (!ModemCategory.isLteSupport()) {
            findViewById(R.id.antenna_title_4g).setVisibility(View.GONE);
            findViewById(R.id.antenna_hint).setVisibility(View.GONE);
            antennaLayout4G.setVisibility(View.GONE);
            antennaLayoutEgmc4G.setVisibility(View.GONE);
        }
    }

    public void initAnt3GView() {
        mSpinner3G = (Spinner) findViewById(R.id.spinner_antenna_3g);
        mSpinner3G.setOnItemSelectedListener(mItemSelectedListener);

        if (ModemCategory.getModemType() == ModemCategory.MODEM_TD ) {
            findViewById(R.id.antenna_title_3g).setVisibility(View.GONE);
            mSpinner3G.setVisibility(View.GONE);
        }
    }

    public void initAnt2GView() {
        antennaLayout2G = (LinearLayout) findViewById(R.id.antenna_2g);
        mSpinner2G = (Spinner) findViewById(R.id.spinner_antenna_2g);
        mSpinner2G.setOnItemSelectedListener(mItemSelectedListener);
        antennaLayout2G.setVisibility(View.GONE);
    }

    public void initAntC2KView() {
        mSpinnerC2kMode = (Spinner) findViewById(R.id.spinner_antenna_c2k_mode);
        mSpinnerC2kMode.setOnItemSelectedListener(mItemSelectedListener);

        if (!ModemCategory.isCdma() || FeatureSupport.is90Modem()) {
            findViewById(R.id.antenna_title_c2k).setVisibility(View.GONE);
            mSpinnerC2kMode.setVisibility(View.GONE);
        }
    }
    @Override
    protected void onResume() {
        super.onResume();
        queryCurrentMode();
        if (ModemCategory.isCdma()
                && !FeatureSupport.is90Modem()) {
            queryCurrentCdmaMode();
        }
    }

    private void queryCurrentMode() {
        sendCommand(new String[] {"AT+ERXPATH?", "+ERXPATH:"}, MSG_QUERY_ANTENNA_MODE);
    }

    private void queryCurrentCdmaMode() {
        String[] cmd = new String[3];
        cmd[0] = "AT+ERXTESTMODE?";
        cmd[1] = "+ERXTESTMODE:";
        cmd[2] = "DESTRILD:C2K";
        String[] cmd_s = ModemCategory.getCdmaCmdArr(cmd);
        sendCdmaCommand(cmd_s,MSG_QUERY_ANTENNA_MODE_C2K);
    }

    private void setMode(int mode) {
        Elog.i(TAG, "Set mode " + mode);
        sendCommand(new String[] {"AT+ERXPATH=" + mode, ""}, MSG_SET_ANTENNA_MODE);
    }

    private void setCdmaMode(int mode) {
        String[] cmd = new String[3];
        cmd[0] = "AT+ERXTESTMODE=" + mode;
        cmd[1] = "+ERXTESTMODE:";
        cmd[2] = "DESTRILD:C2K";
        String[] cmd_s = ModemCategory.getCdmaCmdArr(cmd);
        sendCdmaCommand(cmd_s,MSG_SET_ANTENNA_MODE);
    }

    private void sendCommand(String[] command, int msg) {
        Elog.d(TAG, "sendCommand " + msg + "," + Arrays.toString(command));
        EmUtils.invokeOemRilRequestStringsEm(command, mCommandHander.obtainMessage(msg));
    }

    private void sendCdmaCommand(String[] command, int msg) {
        Elog.d(TAG, "send cdma cmd: " + msg + "," + Arrays.toString(command) +
                ",command.length = " + command.length);
        EmUtils.invokeOemRilRequestStringsEm(true, command, mCommandHander.obtainMessage(msg));
    }
}
