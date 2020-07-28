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

package com.mediatek.engineermode.rfdesense;

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
import android.telephony.PhoneStateListener;
import android.telephony.TelephonyManager;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.Button;
import android.widget.EditText;
import android.widget.ListAdapter;
import android.widget.ListView;
import android.widget.TextView;

import com.mediatek.engineermode.Elog;
import com.mediatek.engineermode.EmUtils;
import com.mediatek.engineermode.FeatureSupport;
import com.mediatek.engineermode.ModemCategory;
import com.mediatek.engineermode.R;
import com.mediatek.engineermode.RatConfiguration;

import java.util.ArrayList;


public class RfDesenseTxTest extends Activity implements OnClickListener, OnItemClickListener {
    public static final String TAG = "RfDesense/TxTest";
    public static final String PREF_FILE = "rfdesense_tx_test";
    public static final int STATE_NONE = 0;
    public static final int STATE_STARTED = 1;
    public static final int STATE_STOPPED = 2;
    public static final int CDMD_MODE_1X = 1;
    public static final int CDMD_MODE_EVDO = 2;

    public static final int HINT = 0;
    public static final int SIM_CARD_INSERT = 1;
    public static final int MSG_START_TX = 1;
    public static final int MSG_CONTINUE_TX = 2;
    public static final int MSG_STOP_ALL_TX = 3;
    public static final int MSG_NEXT_RAT = 4;
    public static final int MSG_UPDATE_BUTTON = 5;
    public static final int MSG_TURN_ON_RF = 6;
    public static final int MSG_FORCE_TX_POWER_READ_URC = 7;
    public static final int MSG_SWITCH_RAT = 8;
    public static final int MSG_TURN_OFF_RF = 9;
    public static final int MSG_READ_POWER = 10;
    public static final int MSG_EWMPOLICY_TDSCDMA = 12;
    public static final int MSG_EWMPOLICY_WCDMA = 13;
    public static final int MSG_ECSRA = 14;
    public static final int MSG_SWITCH_RAT_DONE = 15;
    public static final int MSG_START_TX_TEST = 16;
    public static final int MSG_REBOOT_MODEM = 17;
    public static final int MSG_ANT_SWITCH = 18;
    public static final int MSG_RADIO_STATE_CHANGED = 19;

    public static final String DEFAULT_GSM_ATCMD = "AT+ERFTX=2,1,190,4100,128,0,5,0";
    public static final String DEFAULT_TDSCDMA_ATCMD = "AT+ERFTX=0,0,1,10087,24";
    public static final String DEFAULT_WCDMA_ATCMD = "AT+ERFTX=0,0,1,9750,23";
    public static final String DEFAULT_LTE_FDD_ATCMD = "AT+ERFTX=6,0,1,3,3,17475,1,0,0,0,1,0,23";
    public static final String DEFAULT_LTE_TDD_ATCMD = "AT+ERFTX=6,0,1,38,3,25950,0,0,0,0,1,0,23";
    public static final String DEFAULT_CDMA_EVDO_ATCMD = "AT+ERFTX=13,4,384,0,83";
    public static final String DEFAULT_CDMA_1X_ATCMD = "AT+ECRFTX=1,384,0,83,0";
    public static final String DEFAULT_NR_ATCMD = "AT+EGMC=1,\"NrForcedTx\",2,1,20,2";
    public static final String DEFAULT_CDMA_EVDO_ATCMD_93before = "AT+ERFTX=1,384,0,83,1";

    public static final int DEFAULT_TEST_DURATION = 10;
    public static final int DEFAULT_TEST_COUNT = 1;
    public static final int DEFAULT_CHECK_LIMIT = 2;
    public static final int DEFAULT_READBACK_INTREVAL = 5;

    public static final String KEY_GSM_ATCMD = "gsm_at_cmd";
    public static final String KEY_TDSCDMA_ATCMD = "tdscdma_at_cmd";
    public static final String KEY_WCDMA_ATCMD = "wcdma_at_cmd";
    public static final String KEY_LTE_FDD_ATCMD = "lte_fdd_at_cmd";
    public static final String KEY_LTE_TDD_ATCMD = "lte_tdd_at_cmd";
    public static final String KEY_CDMA_1X_ATCMD = "cdma_at_cmd";
    public static final String KEY_CDMA_EVDO_ATCMD = "cdma_evdo_at_cmd";
    public static final String KEY_NR_ATCMD = "nr_at_cmd";

    public static final String KEY_TEST_DURATION = "test_duration";
    public static final String KEY_TEST_COUNT = "test_count";
    public static final String KEY_CHECK_LIMIT = "check_limit";
    public static final String KEY_READBACK_INTREVAL = "readback_interval";

    public static final String KEY_GSM_ATCMD_ANT_SWITCH = "gsm_at_cmd_ant_switch";
    public static final String KEY_TDSCDMA_ATCMD_ANT_SWITCH = "tdscdma_at_cmd_ant_switch";
    public static final String KEY_WCDMA_ATCMD_ANT_SWITCH = "wcdma_at_cmd_ant_switch";
    public static final String KEY_CDMA1X_ATCMD_ANT_SWITCH = "cdma1x_at_cmd_ant_switch";
    public static final String KEY_EVDO_ATCMD_ANT_SWITCH = "evdo_at_cmd_ant_switch";
    public static final String KEY_NR_ATCMD_ANT_SWITCH = "nr_at_cmd_ant_switch";

    public static String[] mRatName = {"GSM", "TDSCDMA", "WCDMA", "LTE(FDD)", "LTE(TDD)",
            "CDMA(EVDO)", "CDMA(1X)", "NR"};

    public static String[] mRatCmdAntSwitch = {"0", "0", "0", "0", "0", "0", "0", "0"};

    public static String[] mRatCmdStart = {DEFAULT_GSM_ATCMD, DEFAULT_TDSCDMA_ATCMD,
            DEFAULT_WCDMA_ATCMD,
            DEFAULT_LTE_FDD_ATCMD, DEFAULT_LTE_TDD_ATCMD, DEFAULT_CDMA_EVDO_ATCMD,
            DEFAULT_CDMA_1X_ATCMD, DEFAULT_NR_ATCMD};

    public static String[] mRatCmdStop = {"AT+ERFTX=2,0", "AT+ERFTX=0,1", "AT+ERFTX=0,1",
            "AT+ERFTX=6,0,0", "AT+ERFTX=6,0,0", "AT+ERFTX=13,5", "AT+ECRFTX=0",
            "AT+EGMC=1,\"NrForcedTx\",0"};

    public static String[] mRatCmdSwitch = {"AT+ERAT=0", "AT+ERAT=1", "AT+ERAT=1", "AT+ERAT=6,4",
            "AT+ERAT=6,4", "AT+ERAT=7,64", "AT+ERAT=7,32", "AT+ERAT=15"};

    public static String[] mRatCmdPowerRead = {"AT+ERFTX=2,6", "AT+ERFTX=0,3", "AT+ERFTX=0,3",
            "AT+ERFTX=6,1", "AT+ERFTX=6,1", "AT+ERFTX=13,3", "AT+ERFTX=13,3",
            "AT+EGMC=0,\"NrFetchTxPwr\""};
    public static RfDesenseRatInfo mCurrectRatInfo = null;
    public static int phoneid = 0;
    public static long mTestDuration = 0;
    public static long mTestCount = 0;
    public static long mTestDurationSended = 0;
    public static long mTestCountSended = 0;
    public static long mCheckLimit = 0;
    public static long mReadbackInterval = 0;
    private static int mKeyStates = STATE_NONE;
    public String[] mRatBand = {"", "", "", "", "", "", "", ""};
    public String[] mRatPowerSet = {"", "", "", "", "", "", "", ""};
    public RfDesenseRatAdapter mFileListAdapter = null;
    public ArrayList<RfDesenseRatInfo> mRatList = new ArrayList<RfDesenseRatInfo>();
    private int mRadioStatesLast = -1;
    private int band850_900[] = {0, 0, 0, 0, 0, 33, 31, 29, 27, 25,
            23, 21, 19, 17, 15, 13, 11, 9, 7, 5};
    private int band1800_1900[] = {30, 28, 26, 24, 22, 20, 18, 16,
            14, 12, 10, 8, 6, 4, 2, 0};
    private String mGSMBand[] = {"850", "P900", "E900", "R900", "1800", "1900"};

    private EditText mEtTestDuration;
    private EditText mEtTestCount;
    private EditText mEtCheckLimit;
    private EditText mEtReadbackInterval;
    private TextView mTvCheckLimit;
    private TextView mTvReadbackInterval;
    private Button mStopButton;
    private Button mExitButton;
    private Button mStartButton;
    private TextView mTvTxStatus;
    private final Handler mHandler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            AsyncResult ar = (AsyncResult) msg.obj;
            switch (msg.what) {
                case MSG_FORCE_TX_POWER_READ_URC:
                    int[] data = (int[]) ar.result;
                    String result = "";
                    String rat = "";
                    String ratPowerSet = "PCL";
                    String ratband_s = "";
                    float getPower = data[1] / 8.0f;
                    int getRatPowerSet = Integer.valueOf(mCurrectRatInfo.getRatPowerSet());
                    int ratband = Integer.parseInt(mCurrectRatInfo.getRatband());
                    if (data[0] == 0) {
                        ratband_s = mGSMBand[ratband];
                        ratPowerSet += getRatPowerSet + "(";
                        if (ratband == 0 || ratband == 1 || ratband == 2 || ratband == 3) {
                            ratPowerSet += band850_900[getRatPowerSet] + ")";
                            getRatPowerSet = band850_900[getRatPowerSet];
                        } else {
                            ratPowerSet += band1800_1900[getRatPowerSet] + ")";
                            getRatPowerSet = band1800_1900[getRatPowerSet];
                        }
                    } else {
                        ratband_s = mCurrectRatInfo.getRatband();
                        ratPowerSet = mCurrectRatInfo.getRatPowerSet();
                    }

                    if (Math.abs(getRatPowerSet - getPower) > mCheckLimit) {
                        result = "failed\n";
                    } else {
                        result = "succeed\n";
                    }

                    rat = String.format("%-20s %-15s %-10s", mCurrectRatInfo.getRatName()
                            + "(b" + ratband_s + ")", ratPowerSet, getPower);

                    showTxStatusUI(rat);

                    if (result.equals("failed\n")) {
                        showTxStatusUI(String.format("%10s", result));
                    } else {
                        showTxStatusUI(String.format("%10s", result));
                    }
                    break;
                case MSG_ANT_SWITCH:
                    if (ar.exception == null) {
                        Elog.d(TAG, "switch ant status succeed");
                        mCurrectRatInfo.setRatSendState(true);
                        sendAtCommand(mCurrectRatInfo.getRatCmdStart(), MSG_START_TX);
                        Elog.d(TAG, "send: " + mCurrectRatInfo.getRatName() + " "
                                + mCurrectRatInfo.getRatCmdStart());
                    } else {
                        Elog.e(TAG, "switch ant status failed");
                    }
                    break;
                case MSG_START_TX_TEST:
                    Elog.d(TAG, "turn off rf succeed...");
                    if (mCurrectRatInfo != null) {
                        if (mCurrectRatInfo.getRatCmdAntSwitch().equals("0")
                                || "mt6580".equals(EmUtils.systemPropertyGet(
                                        FeatureSupport.FK_BOARD_PLATFORM, "0")) ) {
                            //lte ant switch inlcluded at at cmd
                            mCurrectRatInfo.setRatSendState(true);
                            sendAtCommand(mCurrectRatInfo.getRatCmdStart(), MSG_START_TX);
                            Elog.d(TAG, "send: " + mCurrectRatInfo.getRatName() + " "
                                    + mCurrectRatInfo.getRatCmdStart());
                        } else {
                            sendAtCommand(mCurrectRatInfo.getRatCmdAntSwitch(), MSG_ANT_SWITCH);
                            Elog.d(TAG, "switch ant statrus: " + mCurrectRatInfo
                                    .getRatCmdAntSwitch());
                        }
                    } else {
                        Elog.w(TAG, "mCurrectRatInfo == null");
                    }
                    break;
                case MSG_TURN_OFF_RF:
                    mHandler.sendMessageDelayed(Message.obtain(mHandler, MSG_START_TX_TEST), 2000);
                    break;
                case MSG_EWMPOLICY_WCDMA:
                    Elog.d(TAG, "AT+EWMPOLICY=0 send succeed");
                    Elog.d(TAG, "send AT+ECSRA=2,1,0,1,1,0 ...");
                    sendAtCommand("AT+ECSRA=2,1,0,1,1,0", MSG_ECSRA);
                    break;
                case MSG_EWMPOLICY_TDSCDMA:
                    Elog.d(TAG, "AT+EWMPOLICY=0 send succeed");
                    Elog.d(TAG, "send AT+ECSRA=2,0,1,0,1,0 ...");
                    sendAtCommand("AT+ECSRA=2,0,1,0,1,0", MSG_ECSRA);
                    break;
                case MSG_ECSRA:
                    Elog.d(TAG, "AT+ECSRA send succeed");
                    turnOffRf();
                    break;
                case MSG_SWITCH_RAT_DONE:
                    if (ar.exception == null) {
                        Elog.d(TAG, "switch rat succeed");
                        if (mCurrectRatInfo.getRatName().equals(mRatName[1])) { // tdscdma
                            Elog.d(TAG, "send AT+EWMPOLICY=0");
                            sendAtCommand("AT+EWMPOLICY=0", MSG_EWMPOLICY_TDSCDMA);
                        } else if (mCurrectRatInfo.getRatName().equals(mRatName[2])) { // wcdma
                            Elog.d(TAG, "send AT+EWMPOLICY=0");
                            sendAtCommand("AT+EWMPOLICY=0", MSG_EWMPOLICY_WCDMA);
                        } else { // other rat
                            turnOffRf();
                        }
                    } else {
                        Elog.e(TAG, "switch rat failed");
                        mHandler.sendMessageDelayed(Message.obtain(mHandler, MSG_SWITCH_RAT), 1000);
                    }
                    break;
                case MSG_SWITCH_RAT:
                    if (mKeyStates == STATE_STARTED) {
                        mCurrectRatInfo = getCurrectRatInfo();
                        if (mCurrectRatInfo != null
                                && !mCurrectRatInfo.getRatCmdSwitch().equals("")) {
                            Elog.d(TAG, "switch rat(" + mCurrectRatInfo.getRatCmdSwitch() + ")");
                            sendAtCommand(mCurrectRatInfo.getRatCmdSwitch(), MSG_SWITCH_RAT_DONE);
                        } else {
                            Elog.d(TAG, "no need switch rat");
                            turnOffRf();
                        }
                    }
                    break;
                case MSG_TURN_ON_RF:
                    Elog.d(TAG, "turn on rf succeed");
                    mHandler.sendMessageDelayed(Message.obtain(mHandler, MSG_SWITCH_RAT), 1000);
                    break;
                case MSG_UPDATE_BUTTON:
                    updateButtons();
                    break;
                case MSG_STOP_ALL_TX:
                    if (ar.exception == null) {
                        Elog.d(TAG, "Stop all succeed");
                    } else {
                        Elog.e(TAG, "Stop all failed");
                    }
                    //if (FeatureSupport.is93Modem()) {
                    //     RfDesenseFileSave.saveRatTestResult
                    //             (RfDesenseTxTest.this, mTvTxStatus.getText().toString());
                    //  }
                    updateUIView();
                    break;
                case MSG_CONTINUE_TX:
                    if (mKeyStates == STATE_STOPPED) {
                        mHandler.removeMessages(MSG_CONTINUE_TX);
                        txTestStop(MSG_STOP_ALL_TX);
                        return;
                    }
                    mTestDurationSended += mReadbackInterval;
                    if (mTestDurationSended > mTestDuration) {
                        mTestDurationSended = 0;
                        txTestStop(MSG_NEXT_RAT);
                    } else {
                        mHandler.sendMessageDelayed(Message.obtain(mHandler, MSG_CONTINUE_TX),
                                mReadbackInterval * 1000);
                        if (FeatureSupport.is93Modem()) {
                            if (mCurrectRatInfo != null
                                    && !mCurrectRatInfo.getRatCmdPowerRead().equals("")) {
                                Elog.d(TAG, "send read tx power:" +
                                        mCurrectRatInfo.getRatCmdPowerRead());
                                sendAtCommand(mCurrectRatInfo.getRatCmdPowerRead(), MSG_READ_POWER);
                            }
                        }
                    }
                    break;
                case MSG_NEXT_RAT:
                    if (ar.exception == null) {
                        Elog.d(TAG, "stop cmd ok");
                        showTxStatusUI(mCurrectRatInfo.getRatName() + " stop cmd ok \n");
                        mCurrectRatInfo = getCurrectRatInfo();
                        if (mCurrectRatInfo != null) {
                            turnOnRf();
                        } else {
                            mTestCountSended++;
                            Elog.d(TAG, "send done,mTestCountSended = " + mTestCountSended);
                            if (mTestCountSended < mTestCount) {
                                for (int i = 0; i < mRatList.size(); i++) {
                                    mRatList.get(i).setRatSendState(false);
                                }
                                turnOnRf();
                            } else {
                                showTxStatusUI("send all rat done\n");
                                updateUIView();
                            }
                        }
                    } else {
                        Elog.e(TAG, "stop cmd failed");
                        showTxStatusUI(mCurrectRatInfo.getRatName() + " stop cmd failed \n");
                        EmUtils.showToast("stop cmd failed");
                        updateUIView();
                    }
                    break;
                case MSG_START_TX:
                    if (ar.exception == null) {
                        Elog.d(TAG, "start cmd ok");
                        showTxStatusUI(mCurrectRatInfo.getRatName() + " start cmd ok\n");
                    } else {
                        Elog.e(TAG, "start cmd failed");
                        showTxStatusUI(mCurrectRatInfo.getRatName() + " start cmd failed\n");
                        EmUtils.showToast("start cmd failed");
                    }
                    mHandler.sendMessageDelayed(Message.obtain(mHandler, MSG_CONTINUE_TX),
                            1000);
                    break;
                case MSG_READ_POWER:
                    if (ar.exception == null) {
                        Elog.d(TAG, "read tx power succeed");
                    } else {
                        Elog.d(TAG, "read tx power failed");
                    }
                    break;
                case MSG_REBOOT_MODEM:
                    if (ar.exception == null) {
                        Elog.d(TAG, "AT+CFUN=1,1 send succeed");
                    } else {
                        Elog.d(TAG, "AT+CFUN=1,1 send failed");
                    }
                    break;
                case MSG_RADIO_STATE_CHANGED:
                    int[] state = (int[]) ar.result;
                    if (state[0] == TelephonyManager.RADIO_POWER_ON) {
                        Elog.v(TAG, "RADIO_POWER_ON");
                        mHandler.sendEmptyMessage(MSG_TURN_ON_RF);
                        mRadioStatesLast = state[0];
                    } else if (state[0] == TelephonyManager.RADIO_POWER_OFF) {
                        Elog.v(TAG, "RADIO_POWER_OFF");
                        if (mRadioStatesLast == TelephonyManager.RADIO_POWER_ON)
                            mHandler.sendEmptyMessage(MSG_TURN_OFF_RF);
                        mRadioStatesLast = state[0];
                    } else if (state[0] == TelephonyManager.RADIO_POWER_UNAVAILABLE) {
                        mRadioStatesLast = state[0];
                        Elog.v(TAG, "RADIO_POWER_UNAVAILABLE");
                    }
                    break;
                default:
                    break;
            }
        }
    };

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.rf_desense_tx_test);
        mStopButton = (Button) findViewById(R.id.button_stop_total);
        mExitButton = (Button) findViewById(R.id.button_exit_total);
        mStartButton = (Button) findViewById(R.id.button_start_total);
        mStopButton.setOnClickListener(this);
        mExitButton.setOnClickListener(this);
        mStartButton.setOnClickListener(this);
        mEtTestDuration = (EditText) findViewById(R.id.test_duration);
        mEtTestCount = (EditText) findViewById(R.id.test_count);
        mTvTxStatus = (TextView) findViewById(R.id.test_result);
        mEtCheckLimit = (EditText) findViewById(R.id.check_limit);
        mEtReadbackInterval = (EditText) findViewById(R.id.readback_interval);
        mTvCheckLimit = (TextView) findViewById(R.id.check_limit_view);
        mTvReadbackInterval = (TextView) findViewById(R.id.readback_interval_view);
        ListView simTypeListView = (ListView) findViewById(R.id.list);
        if (ModemCategory.isSimReady(-1)) {
            Elog.d(TAG, "some card insert");
            showDialog(SIM_CARD_INSERT);
        }
        if (!FeatureSupport.is93Modem()) {
            mEtCheckLimit.setVisibility(View.GONE);
            mEtReadbackInterval.setVisibility(View.GONE);
            mTvCheckLimit.setVisibility(View.GONE);
            mTvReadbackInterval.setVisibility(View.GONE);
            mRatCmdStart[5] = DEFAULT_CDMA_EVDO_ATCMD_93before;
            mRatCmdStop[5] = "AT+ECRFTX=0";
        }
        if (FeatureSupport.is90Modem()) {
            mRatCmdSwitch[5] = "AT^PREFMODE=4";
            mRatCmdSwitch[6] = "AT^EIRATMODE=2";
        }
        restoreAtCmdState();
        restoreConfigureState();
        for (int i = 0; i < mRatName.length; i++) {
            RfDesenseRatInfo mRatInfo = new RfDesenseRatInfo();
            mRatInfo.setRatName(mRatName[i]);
            mRatInfo.setRatCmdStart(mRatCmdStart[i]);
            mRatInfo.setRatCmdStop(mRatCmdStop[i]);
            mRatInfo.setRatCmdSwitch(mRatCmdSwitch[i]);
            mRatInfo.setRatPowerRead(mRatCmdPowerRead[i]);
            mRatInfo.setRatband(mRatBand[i]);
            mRatInfo.setRatPowerSet(mRatPowerSet[i]);
            mRatInfo.setRatCmdAntSwitch(mRatCmdAntSwitch[i]);
            mRatInfo.setRatCheckState(false);
            mRatInfo.setRatSendState(false);
            if (!ModemCategory.isCdma()) {
                if (i == 5 || i == 6)
                    continue;
            }
            if (!RatConfiguration.isNrSupported()) {
                if (i == 7)
                   continue;
            }
            mRatList.add(mRatInfo);
        }
        mFileListAdapter = new RfDesenseRatAdapter(this, mRatList);
        simTypeListView.setAdapter(mFileListAdapter);
        simTypeListView.setOnItemClickListener(this);
        setListViewItemsHeight(simTypeListView);

        EmUtils.registerForradioStateChanged(0, mHandler, MSG_RADIO_STATE_CHANGED);
        if (FeatureSupport.is93Modem()) {
            Elog.d(TAG, "registerForTxPower");
            EmUtils.registerForTxpowerInfo(mHandler, MSG_FORCE_TX_POWER_READ_URC);
        }
        mTvTxStatus.setText("");
        mKeyStates = STATE_NONE;
        mHandler.sendMessage(Message.obtain(mHandler, MSG_UPDATE_BUTTON));
        EmUtils.initPoweroffmdMode(true, true);
        if (!EmUtils.ifAirplaneModeEnabled()) {
            Elog.d(TAG, "turn off rf");
            EmUtils.setAirplaneModeEnabled(true);
        }
    }

    @Override
    protected void onResume() {
        super.onResume();
        Elog.d(TAG, "onResume");
        restoreAtCmdState();
        updateRatInfo();
    }

    @Override
    public void onDestroy() {
        EmUtils.unregisterradioStateChanged(0);
        EmUtils.unregisterForTxpowerInfo();
        EmUtils.initPoweroffmdMode(false, true);
        Elog.d(TAG, "onDestroy");
        super.onDestroy();
    }

    @Override
    public void onBackPressed() {
        if (mKeyStates == STATE_STARTED) {
            showDialog(HINT);
        } else {
            EmUtils.setAirplaneModeEnabled(false);
            RfDesenseTxTest.this.finish();
        }
    }

    @Override
    public void onClick(final View arg0) {
        if (arg0 == mExitButton) {
            if (mKeyStates == STATE_STARTED) {
                showDialog(HINT);
            } else if (mKeyStates == STATE_STOPPED || mKeyStates == STATE_NONE) {
                EmUtils.setAirplaneModeEnabled(false);
                RfDesenseTxTest.this.finish();
            }
        } else if (arg0 == mStartButton) {
            mTvTxStatus.setText("");
            if (isRatsChecked() == false) {
                Elog.d(TAG, "you must select at least one rat");
                showTxStatusUI("you must select at least one rat\n");
                return;
            }
            setTestParameter();
            mKeyStates = STATE_STARTED;
            mHandler.sendMessage(Message.obtain(mHandler, MSG_UPDATE_BUTTON));
            showTxStatusUI("Start TX: \n");
            if (FeatureSupport.is93Modem()) {
                showTxStatusUI("Rat(band)          " + "Power_Set   " + "Power_Get    "
                        + "Result\n");
                //RfDesenseFileSave.setRfdesenseFiletName();
            }
            turnOnRf();
        } else if (arg0 == mStopButton) {
            Elog.d(TAG, "Stop all");
            mKeyStates = STATE_STOPPED;
            mHandler.sendMessageDelayed(Message.obtain(mHandler, MSG_CONTINUE_TX), 1000);
        }
    }

    boolean isRatsChecked() {
        boolean checked = false;
        int index;
        for (index = 0; index < mRatList.size(); index++) {
            if (mRatList.get(index).getRatCheckState()) {
                if (mRatList.get(index).getRatCheckState()) {
                    checked = true;
                    break;
                }
            }
        }
        return checked;
    }

    private RfDesenseRatInfo getCurrectRatInfo() {
        int index;
        mCurrectRatInfo = null;
        for (index = 0; index < mRatList.size(); index++) {
            if (mRatList.get(index).getRatCheckState()) {
                if (mRatList.get(index).getRatSendState()) {
                    continue;
                }
                mCurrectRatInfo = mRatList.get(index);
                break;
            }
        }
        return mCurrectRatInfo;
    }

    private void updateRatInfo() {
        for (int i = 0; i < mRatList.size(); i++) {
            mRatList.get(i).setRatCmdStart(mRatCmdStart[i]);
            mRatList.get(i).setRatband(mRatBand[i]);
            mRatList.get(i).setRatPowerSet(mRatPowerSet[i]);
            mRatList.get(i).setRatCmdAntSwitch(mRatCmdAntSwitch[i]);
        }
        mFileListAdapter.notifyDataSetInvalidated();
    }

    private void txTestStop(int what) {
        if (mCurrectRatInfo != null) {
            sendAtCommand(mCurrectRatInfo.getRatCmdStop(), what);
            Elog.d(TAG, "stop: " + mCurrectRatInfo.getRatName() + " "
                    + mCurrectRatInfo.getRatCmdStop());
        } else {
            Elog.d(TAG, "mCurrectRatInfo is null");
            updateUIView();
        }
    }

    private void setTestParameter() {
        String TestDuration = mEtTestDuration.getText().toString();
        String TestCount = mEtTestCount.getText().toString();

        String CheckLimit = mEtCheckLimit.getText().toString();
        String ReadbackInterval = mEtReadbackInterval.getText().toString();

        if ("".equals(TestDuration)) {
            mEtTestDuration.setText(DEFAULT_TEST_DURATION + "");
            mTestDuration = DEFAULT_TEST_DURATION;
        } else {
            mTestDuration = Integer.valueOf(TestDuration);
        }

        if ("".equals(TestCount)) {
            mEtTestCount.setText(DEFAULT_TEST_COUNT + "");
            mTestCount = DEFAULT_TEST_COUNT;
        } else {
            mTestCount = Integer.valueOf(TestCount);
        }

        if ("".equals(CheckLimit)) {
            mEtCheckLimit.setText(DEFAULT_CHECK_LIMIT + "");
            mCheckLimit = DEFAULT_CHECK_LIMIT;
        } else {
            mCheckLimit = Integer.valueOf(CheckLimit);
        }
        if ("".equals(ReadbackInterval)) {
            mEtReadbackInterval.setText(String.valueOf(DEFAULT_READBACK_INTREVAL));
            mReadbackInterval = DEFAULT_READBACK_INTREVAL;
        } else {
            mReadbackInterval = Integer.valueOf(ReadbackInterval);
            if (mReadbackInterval < 5) {
                mReadbackInterval = 5;
                EmUtils.showToast("mReadbackInterval at least 5s");
                mEtReadbackInterval.setText(String.valueOf(mReadbackInterval));
            }
        }

        if (mReadbackInterval > mTestDuration) {
            mReadbackInterval = mTestDuration;
        }

        mTestCountSended = 0;
        mTestDurationSended = 0;
        saveConfigureState();
    }

    void turnOffRf() {
        Elog.d(TAG, "turn off rf....");
        EmUtils.setAirplaneModeEnabled(true);
    }

    private void turnOnRf() {
        Elog.d(TAG, "turn on rf...");
        EmUtils.setAirplaneModeEnabled(false);
        if (FeatureSupport.is93Modem() && !FeatureSupport.is95Modem()) {
            EmUtils.rebootModem();        //for some 93 modem branch.wcdma must
        } else if (FeatureSupport.is95Modem()) {
            Elog.d(TAG, "95 modem not need reboot");
        } else {
            sendAtCommand("AT+CFUN=1,1", MSG_REBOOT_MODEM);
        }
    }

    boolean isSendToCdmaCmd(String str) {
        if (mCurrectRatInfo != null) {
            if (str.equals("AT+EFUN=0")) {
                return false;
            } else if (str.equals("AT+CPOF")) {
                Elog.d(TAG, "send to cdma rat:");
                return true;
            } else if (str.equals("AT+CFUN=1,1")) {
                return false;
            } else if (mCurrectRatInfo.getRatName().equals(mRatName[5])
                    || mCurrectRatInfo.getRatName().equals(mRatName[6])) {
                Elog.d(TAG, "send to cdma rat:");
                return true;
            }
        }
        return false;
    }

    private void sendAtCommand(String str, int what) {
        if (FeatureSupport.is93Modem() || !isSendToCdmaCmd(str)) {
            String cmd[] = new String[]{str, ""};
            //Elog.d(TAG, "send md1: " + cmd[0]);
            EmUtils.invokeOemRilRequestStringsEm(cmd, mHandler.obtainMessage(what));
        } else {
            String cmd[] = new String[]{str, "", "DESTRILD:C2K"};
            //Elog.d(TAG, "send cdma: " + cmd[0]);
            EmUtils.invokeOemRilRequestStringsEm(true, cmd, mHandler.obtainMessage(what));
        }
    }

    private void showTxStatusUI(String msg) {
        mTvTxStatus.append(msg);
    }

    private void setListViewItemsHeight(ListView listview) {
        if (listview == null) {
            return;
        }
        ListAdapter adapter = listview.getAdapter();
        int totalHeight = 0;
        for (int i = 0; i < adapter.getCount(); i++) {
            View itemView = adapter.getView(i, null, listview);
            itemView.measure(0, 0);
            totalHeight += itemView.getMeasuredHeight();
        }
        totalHeight += (adapter.getCount() - 1) * listview.getDividerHeight();
        ViewGroup.LayoutParams params = listview.getLayoutParams();
        params.height = totalHeight;
        listview.setLayoutParams(params);
    }

    private void updateButtons() {
        mStartButton.setEnabled(mKeyStates == STATE_NONE || mKeyStates == STATE_STOPPED);
        mStopButton.setEnabled(mKeyStates == STATE_STARTED);
        mExitButton.setEnabled(mKeyStates == STATE_NONE || mKeyStates == STATE_STOPPED);
    }

    private void updateUIView() {
        mKeyStates = STATE_STOPPED;
        mHandler.sendMessageDelayed(Message.obtain(mHandler, MSG_UPDATE_BUTTON), 500);
        for (int i = 0; i < mRatList.size(); i++) {
            mRatList.get(i).setRatCmdStart(mRatCmdStart[i]);
            mRatList.get(i).setRatSendState(false);
        }
        mFileListAdapter.notifyDataSetInvalidated();
    }

    private void restoreAtCmdState() {
        SharedPreferences pref = getSharedPreferences(PREF_FILE, MODE_PRIVATE);
        mRatCmdStart[0] = pref.getString(KEY_GSM_ATCMD, mRatCmdStart[0]);
        mRatCmdStart[1] = pref.getString(KEY_TDSCDMA_ATCMD, mRatCmdStart[1]);
        mRatCmdStart[2] = pref.getString(KEY_WCDMA_ATCMD, mRatCmdStart[2]);
        mRatCmdStart[3] = pref.getString(KEY_LTE_FDD_ATCMD, mRatCmdStart[3]);
        mRatCmdStart[4] = pref.getString(KEY_LTE_TDD_ATCMD, mRatCmdStart[4]);
        mRatCmdStart[5] = pref.getString(KEY_CDMA_EVDO_ATCMD, mRatCmdStart[5]);
        mRatCmdStart[6] = pref.getString(KEY_CDMA_1X_ATCMD, mRatCmdStart[6]);
        mRatCmdStart[7] = pref.getString(KEY_NR_ATCMD, mRatCmdStart[7]);

        mRatCmdAntSwitch[0] = pref.getString(KEY_GSM_ATCMD_ANT_SWITCH, mRatCmdAntSwitch[0]);
        mRatCmdAntSwitch[1] = pref.getString(KEY_TDSCDMA_ATCMD_ANT_SWITCH, mRatCmdAntSwitch[1]);
        mRatCmdAntSwitch[2] = pref.getString(KEY_WCDMA_ATCMD_ANT_SWITCH, mRatCmdAntSwitch[2]);
        mRatCmdAntSwitch[5] = pref.getString(KEY_EVDO_ATCMD_ANT_SWITCH, mRatCmdAntSwitch[5]);
        mRatCmdAntSwitch[6] = pref.getString(KEY_CDMA1X_ATCMD_ANT_SWITCH, mRatCmdAntSwitch[6]);
        mRatCmdAntSwitch[7] = pref.getString(KEY_NR_ATCMD_ANT_SWITCH, mRatCmdAntSwitch[7]);

        mRatBand[0] = pref.getInt(RfDesenseTxTestGsm.KEY_BAND, 0) + "";

        String[] mBandValues = getResources().getStringArray(
                R.array.rf_desense_tx_test_td_band_values);

        mRatBand[1] = mBandValues[pref.getInt(RfDesenseTxTestTd.KEY_TDD_BAND, 0)];

        mRatBand[2] = pref.getInt(RfDesenseTxTestTd.KEY_FDD_BAND, 0) + 1 + "";
        mRatBand[3] = pref.getInt(RfDesenseTxTestLte.KEY_FDD_BAND,
                RfDesenseTxTestLte.DEFAULT_BAND_FDD) + 1 + "";
        mRatBand[4] = pref.getInt(RfDesenseTxTestLte.KEY_TDD_BAND,
                RfDesenseTxTestLte.DEFAULT_BAND_TDD) + 33 + "";
        mRatBand[5] = pref.getInt(RfDesenseTxTestCdma.KEY_EVDO_BAND,
                RfDesenseTxTestCdma.DEFAULT_BAND_VALUE) + "";
        mRatBand[6] = pref.getInt(RfDesenseTxTestCdma.KEY_1X_BAND,
                RfDesenseTxTestCdma.DEFAULT_BAND_VALUE) + "";
        mRatBand[7] = RfDesenseTxTestNR.mBandMapping[pref.getInt(RfDesenseTxTestNR.KEY_NR_BAND,
                RfDesenseTxTestNR.DEFAULT_BAND_NR)] + "";

        mRatPowerSet[0] = pref.getString(RfDesenseTxTestGsm.KEY_POWER, "5");
        mRatPowerSet[1] = pref.getString(RfDesenseTxTestTd.KEY_TDD_POWER, "24");
        mRatPowerSet[2] = pref.getString(RfDesenseTxTestTd.KEY_FDD_POWER, "23");
        mRatPowerSet[3] = pref.getString(RfDesenseTxTestLte.KEY_FDD_POWER,
                RfDesenseTxTestLte.DEFAULT_POWER);
        mRatPowerSet[4] = pref.getString(RfDesenseTxTestLte.KEY_TDD_POWER,
                RfDesenseTxTestLte.DEFAULT_POWER);
        mRatPowerSet[5] = pref.getString(RfDesenseTxTestCdma.KEY_EVDO_POWER,
                RfDesenseTxTestCdma.DEFAULT_POWER_VALUE + "");
        mRatPowerSet[6] = pref.getString(RfDesenseTxTestCdma.KEY_1X_POWER,
                RfDesenseTxTestCdma.DEFAULT_POWER_VALUE + "");
        mRatPowerSet[7] = pref.getString(RfDesenseTxTestNR.KEY_NR_POWER,
                RfDesenseTxTestNR.DEFAULT_POWER + "");
    }

    private void restoreConfigureState() {
        SharedPreferences pref = getSharedPreferences(PREF_FILE, MODE_PRIVATE);
        mTestDuration = pref.getLong(KEY_TEST_DURATION, DEFAULT_TEST_DURATION);
        mTestCount = pref.getLong(KEY_TEST_COUNT, DEFAULT_TEST_COUNT);
        mEtTestDuration.setText(String.valueOf(mTestDuration));
        mEtTestCount.setText(String.valueOf(mTestCount));
        mCheckLimit = pref.getLong(KEY_CHECK_LIMIT, DEFAULT_CHECK_LIMIT);
        mReadbackInterval = pref.getLong(KEY_READBACK_INTREVAL, DEFAULT_READBACK_INTREVAL);
        mEtCheckLimit.setText(String.valueOf(mCheckLimit));
        mEtReadbackInterval.setText(String.valueOf(mReadbackInterval));
    }

    private void saveConfigureState() {
        SharedPreferences.Editor editor = getSharedPreferences(PREF_FILE, MODE_PRIVATE).edit();
        editor.putLong(KEY_TEST_DURATION, mTestDuration);
        editor.putLong(KEY_TEST_COUNT, mTestCount);
        editor.putLong(KEY_CHECK_LIMIT, mCheckLimit);
        editor.putLong(KEY_READBACK_INTREVAL, mReadbackInterval);
        editor.apply();
    }

    @Override
    public void onItemClick(AdapterView<?> parent, View view, int position, long id) {
        Intent intent = new Intent();
        String item = mRatList.get(position).getRatName();
        if (item.equals(mRatName[0])) {
            intent.setClass(this, RfDesenseTxTestGsm.class);
        } else if (item.equals(mRatName[1])) {
            intent.putExtra("mModemType", ModemCategory.MODEM_TD);
            intent.setClass(this, RfDesenseTxTestTd.class);
        } else if (item.equals(mRatName[2])) {
            intent.putExtra("mModemType", ModemCategory.MODEM_FDD);
            intent.setClass(this, RfDesenseTxTestTd.class);
        } else if (item.equals(mRatName[3])) {
            intent.putExtra("mModemType", ModemCategory.MODEM_FDD);
            intent.setClass(this, RfDesenseTxTestLte.class);
        } else if (item.equals(mRatName[4])) {
            intent.putExtra("mModemType", ModemCategory.MODEM_TD);
            intent.setClass(this, RfDesenseTxTestLte.class);
        } else if (item.equals(mRatName[5])) {
            intent.putExtra("mModemType", CDMD_MODE_EVDO);
            intent.setClass(this, RfDesenseTxTestCdma.class);
        } else if (item.equals(mRatName[6])) {
            intent.putExtra("mModemType", CDMD_MODE_1X);
            intent.setClass(this, RfDesenseTxTestCdma.class);
        } else if (item.equals(mRatName[7])) {
            intent.setClass(this, RfDesenseTxTestNR.class);
        }
        this.startActivity(intent);
    }

    @Override
    protected Dialog onCreateDialog(int id) {
        if (id == HINT) {
            AlertDialog.Builder builder = new AlertDialog.Builder(this);
            return builder.setTitle("Hint")
                    .setMessage("Please stop the test first!")
                    .setPositiveButton("Confirm", null)
                    .create();
        } else if (id == SIM_CARD_INSERT) {
            AlertDialog.Builder builder = new AlertDialog.Builder(this);
            return builder.setTitle("Notice")
                    .setMessage("Please pull out the sim card before test")
                    .setPositiveButton("Confirm", new AlertDialog.OnClickListener() {
                        @Override
                        public void onClick(DialogInterface dialog, int which) {
                            finish();
                        }
                    })
                    .create();
        }
        return super.onCreateDialog(id);
    }

}
