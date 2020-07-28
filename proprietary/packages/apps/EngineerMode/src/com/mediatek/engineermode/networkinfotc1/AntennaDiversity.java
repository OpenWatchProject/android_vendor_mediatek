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

package com.mediatek.engineermode.networkinfotc1;

import android.app.Activity;
import android.app.Dialog;
import android.app.ProgressDialog;
import android.os.AsyncResult;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.TextView;
import android.widget.ToggleButton;

import com.android.internal.telephony.PhoneConstants;
import com.mediatek.engineermode.Elog;
import com.mediatek.engineermode.EmUtils;
import com.mediatek.engineermode.ModemCategory;
import com.mediatek.engineermode.R;
import com.mediatek.mdml.Msg;

import java.util.ArrayList;
import java.util.List;

public class AntennaDiversity extends Activity implements OnClickListener,
        MDMCoreOperation.IMDMSeiviceInterface {
    private static final String TAG = "AntennaDiversity";

    private static final int MSG_NW_INFO_QUERY_3G_4G_ANT = 1;
    private static final int MSG_NW_INFO_QUERY_CDMA_ANT = 2;
    private static final int MSG_NW_INFO_SET_ANT = 3;
    private static final int MSG_NW_INFO_SET_ANT_CDMA = 4;
    private static final int MODE_INDEX_BASE_3G = 10;

    private static final int ANT_PRX = 1;
    private static final int ANT_DRX = 2;
    private static final int ANT_BOTH = 3;
    private static final String DEFAULT_VALUE = "---";
    private static final int DIALOG_WAIT_INIT = 0;
    private static final int DIALOG_WAIT_SUBSCRIB = 1;
    private static final int DIALOG_WAIT_UNSUBSCRIB = 2;
    private TextView mWcdmaRscpAnt0;
    private TextView mWcdmaRscpAnt1;
    private TextView mWcdmaRscpCombine;
    private TextView mWcdmaRssiAnt0;
    private TextView mWcdmaRssiAnt1;
    private TextView mWcdmaRssiCombine;
    private TextView mLteRsrpAnt0;
    private TextView mLteRsrpAnt1;
    private TextView mLteRsrpCombine;
    private TextView mLteRsrqAnt0;
    private TextView mLteRsrqAnt1;
    private TextView mLteRsrqCombine;
    private TextView mLteRssiAnt0;
    private TextView mLteRssiAnt1;
    private TextView mLteRssiCombine;
    private TextView mLteSinrAnt0;
    private TextView mLteSinrAnt1;
    private TextView mLteSinrCombine;
    private TextView m1xRssiAnt0;
    private TextView m1xRssiAnt1;
    private TextView m1xRssiCombine;
    private TextView mEvdoRssiAnt0;
    private TextView mEvdoRssiAnt1;
    private TextView mEvdoRssiCombine;
    private ToggleButton m3GPrx;
    private ToggleButton m3GDrx;
    private ToggleButton m3GBoth;
    private ToggleButton m4GPrx;
    private ToggleButton m4GDrx;
    private ToggleButton m4GBoth;
    private ToggleButton m1xPrx;
    private ToggleButton m1xDrx;
    private ToggleButton m1xBoth;
    private ToggleButton mEvdoPrx;
    private ToggleButton mEvdoDrx;
    private ToggleButton mEvdoBoth;
    private ProgressDialog mProgressDialog;
    private ProgressDialog mProgressDialog1;
    private ProgressDialog mProgressDialog2;
    private int m3gAnt = -1;
    private int m4gAnt = -1;
    private int mCdmaAnt = -1;

    private String mExtent = "+ERXTESTMODE:";

    private List<MdmBaseComponent> componentsArray = new ArrayList<MdmBaseComponent>();
    private int mSimTypeToShow = 0;
    private String SubscribeMsgIdNameCDMA[] = {"MSG_ID_EM_C2K_L4_RTT_RADIO_INFO_IND",
            "MSG_ID_EM_C2K_L4_EVDO_SERVING_INFO_IND"};
    private String SubscribeMsgIdNameLTE[] = {"MSG_ID_EM_EL1_STATUS_IND"};
    private String SubscribeMsgIdNameFDD[] = {"MSG_ID_FDD_EM_UL1_TAS_INFO_IND"};

    private Handler mAtCmdHandler = new Handler() {
        public void handleMessage(Message msg) {
            AsyncResult ar;
            switch (msg.what) {
                case MSG_NW_INFO_QUERY_3G_4G_ANT:
                    ar = (AsyncResult) msg.obj;
                    if (ar.exception == null) {
                        final String[] result = (String[]) ar.result;
                        for (int i = 0; i < result.length; i++) {
                            parseCurrentMode(result[i]);
                        }
                        updateButtons();
                    } else {
                        Elog.e(TAG, "Query 3G/4G antenna currect mode failed.");
                    }
                    break;
                case MSG_NW_INFO_QUERY_CDMA_ANT:
                    ar = (AsyncResult) msg.obj;
                    if (ar.exception == null) {
                        String[] data = (String[]) ar.result;
                        Elog.v(TAG, "cdma,data[0] = " + data[0]);
                        try {
                            String mode = data[0].substring(mExtent.length());
                            Elog.v(TAG, "data[0] = " + mode);
                            mCdmaAnt = Integer.parseInt(mode);
                            updateButtons();
                        } catch (NumberFormatException e) {
                            return;
                        }
                    } else {
                        Elog.e(TAG, "exception");
                    }
                    break;
                case MSG_NW_INFO_SET_ANT:
                    ar = (AsyncResult) msg.obj;
                    if (ar.exception == null) {
                        Elog.d(TAG, "set 3G/4G antenna mode succeed.");
                    } else {
                        Elog.e(TAG, "set 3G/4G antenna mode failed.");
                    }
                    break;
                case MSG_NW_INFO_SET_ANT_CDMA:
                    ar = (AsyncResult) msg.obj;
                    if (ar.exception == null) {
                        Elog.d(TAG, "set cdma antenna mode succeed.on/off RF");
                        EmUtils.setAirplaneModeEnabled(true);
                        EmUtils.setAirplaneModeEnabled(false);
                    } else {
                        Elog.e(TAG, "set cdma antenna mode failed.");
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
        setContentView(R.layout.antenna_diversity);
        mWcdmaRscpAnt0 = (TextView) findViewById(R.id.antenna_diversity_rscp_ant0);
        mWcdmaRscpAnt1 = (TextView) findViewById(R.id.antenna_diversity_rscp_ant1);
        mWcdmaRscpCombine = (TextView) findViewById(R.id.antenna_diversity_rscp_combined);
        mWcdmaRssiAnt0 = (TextView) findViewById(R.id.antenna_diversity_pssi_ant0);
        mWcdmaRssiAnt1 = (TextView) findViewById(R.id.antenna_diversity_pssi_ant1);
        mWcdmaRssiCombine = (TextView) findViewById(R.id.antenna_diversity_pssi_combined);
        mLteRsrpAnt0 = (TextView) findViewById(R.id.antenna_diversity_rsrp_ant0);
        mLteRsrpAnt1 = (TextView) findViewById(R.id.antenna_diversity_rsrp_ant1);
        mLteRsrpCombine = (TextView) findViewById(R.id.antenna_diversity_rsrp_combined);
        mLteRsrqAnt0 = (TextView) findViewById(R.id.antenna_diversity_rsrq_ant0);
        mLteRsrqAnt1 = (TextView) findViewById(R.id.antenna_diversity_rsrq_ant1);
        mLteRsrqCombine = (TextView) findViewById(R.id.antenna_diversity_rsrq_combined);
        mLteRssiAnt0 = (TextView) findViewById(R.id.antenna_diversity_rssi_ant0);
        mLteRssiAnt1 = (TextView) findViewById(R.id.antenna_diversity_rssi_ant1);
        mLteRssiCombine = (TextView) findViewById(R.id.antenna_diversity_rssi_combined);
        mLteSinrAnt0 = (TextView) findViewById(R.id.antenna_diversity_sinr_ant0);
        mLteSinrAnt1 = (TextView) findViewById(R.id.antenna_diversity_sinr_ant1);
        mLteSinrCombine = (TextView) findViewById(R.id.antenna_diversity_sinr_combined);
        m1xRssiAnt0 = (TextView) findViewById(R.id.antenna_diversity_1x_rssi_ant0);
        m1xRssiAnt1 = (TextView) findViewById(R.id.antenna_diversity_1x_rssi_ant1);
        m1xRssiCombine = (TextView) findViewById(R.id.antenna_diversity_1x_rssi_combined);
        mEvdoRssiAnt0 = (TextView) findViewById(R.id.antenna_diversity_evdo_rssi_ant0);
        mEvdoRssiAnt1 = (TextView) findViewById(R.id.antenna_diversity_evdo_rssi_ant1);
        mEvdoRssiCombine = (TextView) findViewById(R.id.antenna_diversity_evdo_rssi_combined);
        m3GPrx = (ToggleButton) findViewById(R.id.antenna_diversity_3g_prx);
        m3GDrx = (ToggleButton) findViewById(R.id.antenna_diversity_3g_drx);
        m3GBoth = (ToggleButton) findViewById(R.id.antenna_diversity_3g_prx_drx);
        m4GPrx = (ToggleButton) findViewById(R.id.antenna_diversity_4g_prx);
        m4GDrx = (ToggleButton) findViewById(R.id.antenna_diversity_4g_drx);
        m4GBoth = (ToggleButton) findViewById(R.id.antenna_diversity_4g_prx_drx);
        m1xPrx = (ToggleButton) findViewById(R.id.antenna_diversity_1x_prx);
        m1xDrx = (ToggleButton) findViewById(R.id.antenna_diversity_1x_drx);
        m1xBoth = (ToggleButton) findViewById(R.id.antenna_diversity_1x_prx_drx);
        mEvdoPrx = (ToggleButton) findViewById(R.id.antenna_diversity_evdo_prx);
        mEvdoDrx = (ToggleButton) findViewById(R.id.antenna_diversity_evdo_drx);
        mEvdoBoth = (ToggleButton) findViewById(R.id.antenna_diversity_evdo_prx_drx);
        m3GPrx.setOnClickListener(this);
        m3GDrx.setOnClickListener(this);
        m3GBoth.setOnClickListener(this);
        m4GPrx.setOnClickListener(this);
        m4GDrx.setOnClickListener(this);
        m4GBoth.setOnClickListener(this);
        m1xPrx.setOnClickListener(this);
        m1xDrx.setOnClickListener(this);
        m1xBoth.setOnClickListener(this);
        mEvdoPrx.setOnClickListener(this);
        mEvdoDrx.setOnClickListener(this);
        mEvdoBoth.setOnClickListener(this);

        mSimTypeToShow = PhoneConstants.SIM_ID_1;
        showDialog(DIALOG_WAIT_INIT);
        MDMCoreOperation.getInstance().mdmInitialize(this);
        MDMCoreOperation.getInstance().setOnMDMChangedListener(this);
        MDMCoreOperation.getInstance().mdmParametersSeting(componentsArray, mSimTypeToShow);
    }

    @Override
    protected void onResume() {
        super.onResume();
        QueryAntennaStatus();
    }

    @Override
    public void onPause() {
        super.onPause();
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
        Elog.d(TAG, "onDestroy");
    }

    public void onBackPressed() {
        showDialog(DIALOG_WAIT_UNSUBSCRIB);
        MDMCoreOperation.getInstance().mdmlUnSubscribe();
    }

    @Override
    public void onClick(View arg0) {
        String cmd = "";
        int old3gAnt = m3gAnt;
        int old4gAnt = m4gAnt;
        int oldCdmaAnt = mCdmaAnt;

        if (arg0.getId() == m3GPrx.getId()) {
            m3gAnt = ANT_PRX;
            cmd = "AT+ERXPATH=10";
        } else if (arg0.getId() == m3GDrx.getId()) {
            m3gAnt = ANT_DRX;
            cmd = "AT+ERXPATH=11";
        } else if (arg0.getId() == m3GBoth.getId()) {
            m3gAnt = ANT_BOTH;
            cmd = "AT+ERXPATH=12";
        } else if (arg0.getId() == m4GPrx.getId()) {
            m4gAnt = ANT_PRX;
            cmd = "AT+ERXPATH=1";
        } else if (arg0.getId() == m4GDrx.getId()) {
            m4gAnt = ANT_DRX;
            cmd = "AT+ERXPATH=2";
        } else if (arg0.getId() == m4GBoth.getId()) {
            m4gAnt = ANT_BOTH;
            cmd = "AT+ERXPATH=0";
        } else if (arg0.getId() == m1xPrx.getId()) {
            mCdmaAnt = ANT_PRX;
        } else if (arg0.getId() == m1xDrx.getId()) {
            mCdmaAnt = ANT_DRX;
        } else if (arg0.getId() == m1xBoth.getId()) {
            mCdmaAnt = ANT_BOTH;
        } else if (arg0.getId() == mEvdoPrx.getId()) {
            mCdmaAnt = ANT_PRX;
        } else if (arg0.getId() == mEvdoDrx.getId()) {
            mCdmaAnt = ANT_DRX;
        } else if (arg0.getId() == mEvdoBoth.getId()) {
            mCdmaAnt = ANT_BOTH;
        }
        if (old3gAnt != m3gAnt) {
            sendATCommand(new String[]{cmd, ""}, MSG_NW_INFO_SET_ANT);
        } else if (old4gAnt != m4gAnt) {
            sendATCommand(new String[]{cmd, ""}, MSG_NW_INFO_SET_ANT);
        } else if (oldCdmaAnt != mCdmaAnt) {
            String[] setMode = {"AT+ERXTESTMODE=" + mCdmaAnt, "", "DESTRILD:C2K"};
            String[] setMode_s = ModemCategory.getCdmaCmdArr(setMode);
            sendATCommand(setMode_s, MSG_NW_INFO_SET_ANT_CDMA);
        }
        setDefaultText();
        updateButtons();
    }

    private void QueryAntennaStatus() {
        String[] queryMode = {"AT+ERXTESTMODE?", "+ERXTESTMODE:", "DESTRILD:C2K"};
        String[] queryMode_s = ModemCategory.getCdmaCmdArr(queryMode);
        sendATCommand(queryMode_s, MSG_NW_INFO_QUERY_CDMA_ANT);
        sendATCommand(new String[]{"AT+ERXPATH?", "+ERXPATH:"}, MSG_NW_INFO_QUERY_3G_4G_ANT);
    }


    private void registerNetworkInfo() {
        MdmBaseComponent components = new MdmBaseComponent();
        components.setEmComponentName(SubscribeMsgIdNameCDMA);
        componentsArray.add(components);

        components = new MdmBaseComponent();
        components.setEmComponentName(SubscribeMsgIdNameLTE);
        componentsArray.add(components);

        components = new MdmBaseComponent();
        components.setEmComponentName(SubscribeMsgIdNameFDD);
        componentsArray.add(components);

        MDMCoreOperation.getInstance().mdmlSubscribe();
    }

    private void setDefaultText() {
        mWcdmaRscpAnt0.setText(DEFAULT_VALUE);
        mWcdmaRssiAnt0.setText(DEFAULT_VALUE);
        mWcdmaRscpAnt1.setText(DEFAULT_VALUE);
        mWcdmaRssiAnt1.setText(DEFAULT_VALUE);
        mWcdmaRscpCombine.setText(DEFAULT_VALUE);
        mWcdmaRssiCombine.setText(DEFAULT_VALUE);
        mLteRsrpAnt0.setText(DEFAULT_VALUE);
        mLteRsrqAnt0.setText(DEFAULT_VALUE);
        mLteRssiAnt0.setText(DEFAULT_VALUE);
        mLteSinrAnt0.setText(DEFAULT_VALUE);
        mLteRsrpAnt1.setText(DEFAULT_VALUE);
        mLteRsrqAnt1.setText(DEFAULT_VALUE);
        mLteRssiAnt1.setText(DEFAULT_VALUE);
        mLteSinrAnt1.setText(DEFAULT_VALUE);
        mLteRsrpCombine.setText(DEFAULT_VALUE);
        mLteRsrqCombine.setText(DEFAULT_VALUE);
        mLteRssiCombine.setText(DEFAULT_VALUE);
        mLteSinrCombine.setText(DEFAULT_VALUE);
        m1xRssiAnt0.setText(DEFAULT_VALUE);
        m1xRssiAnt1.setText(DEFAULT_VALUE);
        m1xRssiCombine.setText(DEFAULT_VALUE);
        mEvdoRssiAnt0.setText(DEFAULT_VALUE);
        mEvdoRssiAnt1.setText(DEFAULT_VALUE);
        mEvdoRssiCombine.setText(DEFAULT_VALUE);
    }

    private void updateButtons() {
        m3GPrx.setChecked(m3gAnt == ANT_PRX);
        m3GDrx.setChecked(m3gAnt == ANT_DRX);
        m3GBoth.setChecked(m3gAnt == ANT_BOTH);
        m4GPrx.setChecked(m4gAnt == ANT_PRX);
        m4GDrx.setChecked(m4gAnt == ANT_DRX);
        m4GBoth.setChecked(m4gAnt == ANT_BOTH);
        m1xPrx.setChecked(mCdmaAnt == ANT_PRX);
        m1xDrx.setChecked(mCdmaAnt == ANT_DRX);
        m1xBoth.setChecked(mCdmaAnt == ANT_BOTH);
        mEvdoPrx.setChecked(mCdmaAnt == ANT_PRX);
        mEvdoDrx.setChecked(mCdmaAnt == ANT_DRX);
        mEvdoBoth.setChecked(mCdmaAnt == ANT_BOTH);
    }

    private void parseCurrentMode(String data) {
        // query result is like: +ERXPATH: 1
        int mode = -1;
        Elog.d(TAG, "parseCurrentMode data= " + data);
        try {
            mode = Integer.valueOf(data.substring("+ERXPATH:".length()).trim());
        } catch (Exception e) {
            Elog.e(TAG, "Wrong current mode format: " + data);
            return;
        }

        if (mode >= MODE_INDEX_BASE_3G) {
            m3gAnt = mode - MODE_INDEX_BASE_3G + 1;
            Elog.d(TAG, "parseCurrent3GMode is: " + m3gAnt);
        } else {
            if (mode == 0) {
                m4gAnt = ANT_BOTH;
            } else if (mode == 1) {
                m4gAnt = ANT_PRX;
            } else if (mode == 2) {
                m4gAnt = ANT_DRX;
            } else {
                m4gAnt = -1;
            }
            Elog.d(TAG, "parseCurrentLteMode is: " + m4gAnt);
        }
    }

    private void sendATCommand(String[] atCommand, int msg) {
        Elog.v(TAG, "sendATCommand = " + atCommand[0]);
        EmUtils.invokeOemRilRequestStringsEm(atCommand, mAtCmdHandler
                .obtainMessage(msg));
    }

    @Override
    public void onUpdateMDMStatus(int msg_id) {
        switch (msg_id) {
            case MDMCoreOperation.MDM_SERVICE_INIT: {
                Elog.d(TAG, "MDM Service init done");
                registerNetworkInfo();
                removeDialog(DIALOG_WAIT_INIT);
                showDialog(DIALOG_WAIT_SUBSCRIB);
                break;
            }
            case MDMCoreOperation.SUBSCRIBE_DONE: {
                Elog.d(TAG, "Subscribe message id done");
                removeDialog(DIALOG_WAIT_SUBSCRIB);
                MDMCoreOperation.getInstance().mdmlEnableSubscribe();
                break;
            }
            case MDMCoreOperation.UNSUBSCRIBE_DONE:
                Elog.d(TAG, "UnSubscribe message id done");
                removeDialog(DIALOG_WAIT_UNSUBSCRIB);
                MDMCoreOperation.getInstance().mdmlClosing();
                finish();
                break;
            default:
                break;
        }
    }

    int getFieldValue(Msg data, String key, boolean signed) {
        return MDMCoreOperation.getInstance().getFieldValue(data, key, signed);
    }

    int getFieldValue(Msg data, String key) {
        return MDMCoreOperation.getInstance().getFieldValue(data, key, false);
    }

    void updateLTEInfo(Msg data) {
        int dl_rsrp0 = getFieldValue(data, "dl_info[0].dl_rsrp[0]", true);
        int dl_rsrq0 = getFieldValue(data, "dl_info[0].dl_rsrq[0]", true);
        int dl_rssi0 = getFieldValue(data, "dl_info[0].dl_rssi[0]", true);
        int dl_sinr0 = getFieldValue(data, "dl_info[0].dl_sinr[0]", true);
        int dl_rsrp1 = getFieldValue(data, "dl_info[0].dl_rsrp[1]", true);
        int dl_rsrq1 = getFieldValue(data, "dl_info[0].dl_rsrq[1]", true);
        int dl_rssi1 = getFieldValue(data, "dl_info[0].dl_rssi[1]", true);
        int dl_sinr1 = getFieldValue(data, "dl_info[0].dl_sinr[1]", true);
        int dl_rsrp = getFieldValue(data, "dl_info[0].rsrp", true);
        int dl_rsrq = getFieldValue(data, "dl_info[0].rsrq", true);
        int dl_rssi = (dl_rssi0 > dl_rssi1) ? dl_rssi0 : dl_rssi1;
        int dl_sinr = getFieldValue(data, "dl_info[0].sinr", true);
        Elog.d(TAG, "dl_rsrp0 = " + dl_rsrp0);
        Elog.d(TAG, "dl_rsrq0 = " + dl_rsrq0);
        Elog.d(TAG, "dl_rssi0 = " + dl_rssi0);
        Elog.d(TAG, "dl_sinr0 = " + dl_sinr0);
        Elog.d(TAG, "dl_rsrp1 = " + dl_rsrp1);
        Elog.d(TAG, "dl_rsrq1 = " + dl_rsrq1);
        Elog.d(TAG, "dl_rssi1 = " + dl_rssi1);
        Elog.d(TAG, "dl_sinr1 = " + dl_sinr1);
        Elog.d(TAG, "dl_rsrp = " + dl_rsrp);
        Elog.d(TAG, "dl_rsrq = " + dl_rsrq);
        Elog.d(TAG, "dl_rssi = " + dl_rssi);
        Elog.d(TAG, "dl_sinr = " + dl_sinr);
        switch (m4gAnt) {
            case ANT_PRX:
                mLteRsrpAnt0.setText(dl_rsrp0 + "");
                mLteRsrqAnt0.setText(dl_rsrq0 + "");
                mLteRssiAnt0.setText(dl_rssi0 + "");
                mLteSinrAnt0.setText(dl_sinr0 + "");
                break;
            case ANT_DRX:
                mLteRsrpAnt1.setText(dl_rsrp1 + "");
                mLteRsrqAnt1.setText(dl_rsrq1 + "");
                mLteRssiAnt1.setText(dl_rssi1 + "");
                mLteSinrAnt1.setText(dl_sinr1 + "");
                break;
            case ANT_BOTH:
                mLteRsrpCombine.setText(dl_rsrp + "");
                mLteRsrqCombine.setText(dl_rsrq + "");
                mLteRssiCombine.setText(dl_rssi + "");
                mLteSinrCombine.setText(dl_sinr + "");
                break;
            default:
                break;
        }
    }

    void updateWcdmaInfo(Msg data) {
        int main_ant_idx = getFieldValue(data, "EmUl1Tas.main_ant_idx");
        int rscp_0 = getFieldValue(data, "EmUl1Tas.rscp_0", true);
        int rscp_1 = getFieldValue(data, "EmUl1Tas.rscp_1", true);
        int rscp_2 = getFieldValue(data, "EmUl1Tas.rscp_2", true);

        int rssi_0 = getFieldValue(data, "EmUl1Tas.rssi_0", true);
        int rssi_1 = getFieldValue(data, "EmUl1Tas.rssi_1", true);
        int rssi_2 = getFieldValue(data, "EmUl1Tas.rssi_2", true);
        int rscp_max = getFieldValue(data, "EmUl1Tas.rscp_max", true);
        int rssi_max = getFieldValue(data, "EmUl1Tas.rssi_max", true);


        int rscp_main = 0;
        int rssi_main = 0;

        int rscp_div = 0;
        int rssi_div = 0;

        if (main_ant_idx == 0) {
            rscp_main = rscp_0;
            rssi_main = rssi_0;
            rscp_div = rscp_1;
            rssi_div = rssi_1;
        } else if (main_ant_idx == 1) {
            rscp_main = rscp_1;
            rssi_main = rssi_1;
            rscp_div = rscp_0;
            rssi_div = rssi_0;
        } else if (main_ant_idx == 2) {
            rscp_main = rscp_2;
            rssi_main = rssi_2;
            rscp_div = rscp_1;
            rssi_div = rssi_1;
        } else if (main_ant_idx == 3) {
            rscp_main = rscp_1;
            rssi_main = rssi_1;
            rscp_div = rscp_2;
            rssi_div = rssi_2;
        }
        Elog.d(TAG, "rscp_main = " + rscp_main);
        Elog.d(TAG, "rssi_main = " + rssi_main);

        Elog.d(TAG, "rscp_div = " + rscp_div);
        Elog.d(TAG, "rssi_div = " + rssi_div);
        switch (m3gAnt) {
            case ANT_PRX:
                mWcdmaRscpAnt0.setText(rscp_main + "");
                mWcdmaRssiAnt0.setText(rssi_main + "");
                break;
            case ANT_DRX:
                mWcdmaRscpAnt1.setText(rscp_div + "");
                mWcdmaRssiAnt1.setText(rssi_div + "");
                break;
            case ANT_BOTH:
                mWcdmaRscpCombine.setText(rscp_max + "");
                mWcdmaRssiCombine.setText(rssi_max + "");
                break;
            default:
                break;
        }
    }

    void updateCdma1xInfo(Msg data) {
        int prx_power = getFieldValue(data, "rx_power", true);
        int drx_power = getFieldValue(data, "div_rx_power", true);
        int rx_power = (prx_power > drx_power) ? prx_power : drx_power;
        Elog.d(TAG, "prx_power = " + prx_power);
        Elog.d(TAG, "drx_power = " + rx_power);
        Elog.d(TAG, "rx_power = " + rx_power);
        switch (mCdmaAnt) {
            case ANT_PRX:
                m1xRssiAnt0.setText(prx_power + "");
                break;
            case ANT_DRX:
                m1xRssiAnt1.setText(rx_power + "");
                break;
            case ANT_BOTH:
                m1xRssiCombine.setText(rx_power + "");
                break;
            default:
                break;
        }
    }

    void updateCdmaEvdoInfo(Msg data) {
        int prx_power = getFieldValue(data, "rssi_dbm", true) / 128;
        int drx_power = getFieldValue(data, "div_rssi", true) / 128;
        int rx_power = (prx_power > drx_power) ? prx_power : drx_power;
        Elog.d(TAG, "prx_power = " + prx_power);
        Elog.d(TAG, "drx_power = " + drx_power);
        Elog.d(TAG, "rx_power = " + rx_power);
        switch (mCdmaAnt) {
            case ANT_PRX:
                mEvdoRssiAnt0.setText(prx_power + "");
                break;
            case ANT_DRX:
                mEvdoRssiAnt1.setText(drx_power + "");
                break;
            case ANT_BOTH:
                mEvdoRssiCombine.setText(rx_power + "");
                break;
            default:
                break;
        }
    }

    @Override
    public void onUpdateMDMData(String name, Msg data) {
        Elog.d(TAG, "update = " + name);
        if (name.equals("MSG_ID_EM_C2K_L4_RTT_RADIO_INFO_IND")) {
            updateCdma1xInfo(data);
        } else if (name.equals("MSG_ID_EM_C2K_L4_EVDO_SERVING_INFO_IND")) {
            updateCdmaEvdoInfo(data);
        } else if (name.equals("MSG_ID_EM_EL1_STATUS_IND")) {
            updateLTEInfo(data);
        } else if (name.equals("MSG_ID_FDD_EM_UL1_TAS_INFO_IND")) {
            updateWcdmaInfo(data);
        }
    }

    @Override
    protected Dialog onCreateDialog(int id, Bundle args) {
        switch (id) {
            case DIALOG_WAIT_INIT:
                Elog.d(TAG, "Wait MDML init");
                mProgressDialog = new ProgressDialog(this);
                mProgressDialog.setTitle("Waiting");
                mProgressDialog.setMessage("Wait MDML init");
                mProgressDialog.setCancelable(false);
                mProgressDialog.setIndeterminate(true);
                return mProgressDialog;
            case DIALOG_WAIT_SUBSCRIB:
                Elog.d(TAG, "Before Wait subscribe message..");
                mProgressDialog1 = new ProgressDialog(this);
                mProgressDialog1.setTitle("Waiting");
                mProgressDialog1.setMessage("Wait subscribe message..");
                mProgressDialog1.setCancelable(false);
                mProgressDialog1.setIndeterminate(true);
                return mProgressDialog1;
            case DIALOG_WAIT_UNSUBSCRIB:
                Elog.d(TAG, "Before Wait Unsubscribe message..");
                mProgressDialog2 = new ProgressDialog(this);
                mProgressDialog2.setTitle("Waiting");
                mProgressDialog2.setMessage("Wait Unsubscribe message..");
                mProgressDialog2.setCancelable(false);
                mProgressDialog2.setIndeterminate(true);
                return mProgressDialog2;
            default:
                return super.onCreateDialog(id);
        }
    }
}
