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

package com.mediatek.simrecovery;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.Bundle;
import android.os.CountDownTimer;
import android.os.PowerManager;
import android.os.RemoteException;
import android.os.ServiceManager;
import android.os.SystemProperties;
import android.telephony.PhoneStateListener;
import android.telephony.RadioAccessFamily;
import android.telephony.Rlog;
import android.telephony.ServiceState;
import android.telephony.SignalStrength;
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.ProgressBar;
import android.widget.TextView;
import android.widget.Toast;
import android.os.Handler;

import com.android.internal.telephony.ITelephony;
import com.android.internal.telephony.Phone;
import com.android.internal.telephony.PhoneConstants;
import com.android.internal.telephony.PhoneFactory;
import com.android.internal.telephony.SubscriptionController;
import com.android.internal.telephony.TelephonyIntents;

import com.mediatek.internal.telephony.MtkGsmCdmaPhone;

import java.util.ArrayList;

public class SimRecoveryTestToolActivity extends Activity {
    private static final String LOG_TAG = "SimRecoveryTestToolActivity";

    private static final boolean MTK_SIM_HOT_SWAP_COMMON_SLOT = (SystemProperties.getInt("ro.vendor.mtk_sim_hot_swap_common_slot", 0) == 1);
    private static final int PROJECT_SIM_NUM = TelephonyManager.getDefault().getSimCount();
    private static final String AT_CMD_SIM_PLUG_OUT = "AT+ESIMTEST=17";
    private static final String AT_CMD_SIM_PLUG_IN  = "AT+ESIMTEST=18";
    private static final String AT_CMD_SIM_PLUG_IN_ALL = "AT+ESIMTEST=19";
    private static final String AT_CMD_SIM_MISSING  = "AT+ESIMTEST=65";
    private static final String AT_CMD_SIM_RECOVERY = "AT+ESIMTEST=66";

    private static Phone[] sPhones = null;

    private static Button sBtnTestButton;
    private static Button sBtnPlugOutAllSims;
    private static Button sBtnPlugInAllSims;
    private static Button[] sBtnPlugOutSim = new Button[PROJECT_SIM_NUM];
    private static Button[] sBtnPlugInSim = new Button[PROJECT_SIM_NUM];
    private static Button[] sBtnMissingSim = new Button[PROJECT_SIM_NUM];
    private static Button[] sBtnRecoverySim = new Button[PROJECT_SIM_NUM];

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        View contentView = getLayoutInflater().inflate(R.layout.main, null);
        setContentView(contentView);
        logd("[onCreate]+");
        logd("PROJECT_SIM_NUM: " + PROJECT_SIM_NUM);

        sPhones = PhoneFactory.getPhones();

        for (int i = 0; i < PROJECT_SIM_NUM; i++) {
            sBtnPlugOutSim[i] = (Button) contentView.findViewWithTag("btn_plug_out_sim" + (i + 1));
            sBtnPlugInSim[i] = (Button) contentView.findViewWithTag("btn_plug_in_sim" + (i + 1));
            sBtnMissingSim[i] = (Button) contentView.findViewWithTag("btn_missing_sim" + (i + 1));
            sBtnRecoverySim[i] = (Button) contentView.findViewWithTag("btn_recovery_sim" + (i + 1));
        }
        for (int i = PROJECT_SIM_NUM; i <= PhoneConstants.SIM_ID_4; i++) {
            contentView.findViewWithTag("btn_plug_out_sim" + (i + 1)).setVisibility(View.GONE);
            contentView.findViewWithTag("btn_plug_in_sim" + (i + 1)).setVisibility(View.GONE);
            contentView.findViewWithTag("btn_missing_sim" + (i + 1)).setVisibility(View.GONE);
            contentView.findViewWithTag("btn_recovery_sim" + (i + 1)).setVisibility(View.GONE);
        }

        sBtnTestButton = (Button) findViewById(R.id.btn_test_button);
        sBtnTestButton.setText(R.string.test_button);
        sBtnTestButton.setOnClickListener(mTestButtonOnClickListener);
        sBtnPlugOutAllSims = (Button) findViewById(R.id.btn_plug_out_all_sims);
        sBtnPlugInAllSims = (Button) findViewById(R.id.btn_plug_in_all_sims);


        if (MTK_SIM_HOT_SWAP_COMMON_SLOT) {
            sBtnPlugOutAllSims.setOnClickListener(mSimTestOnClickListener);
            sBtnPlugInAllSims.setOnClickListener(mSimTestOnClickListener);
            for (int i = 0; i < PROJECT_SIM_NUM; i++) {
                sBtnPlugOutSim[i].setVisibility(View.GONE);
            }
        } else {
            sBtnPlugOutAllSims.setVisibility(View.GONE);
            sBtnPlugInAllSims.setVisibility(View.GONE);
        }

        for (int i = 0; i < PROJECT_SIM_NUM; i++) {
            sBtnPlugOutSim[i].setOnClickListener(mSimTestOnClickListener);
            sBtnPlugInSim[i].setOnClickListener(mSimTestOnClickListener);
            sBtnMissingSim[i].setOnClickListener(mSimTestOnClickListener);
            sBtnRecoverySim[i].setOnClickListener(mSimTestOnClickListener);
        }

        logd("[onCreate]-");
    }

    private OnClickListener mTestButtonOnClickListener = new OnClickListener() {
        @Override
        public void onClick(View v) {
            Toast.makeText(getApplicationContext(), "RD use only", Toast.LENGTH_LONG).show();
        }
    };

    private OnClickListener mSimTestOnClickListener = new OnClickListener() {
        @Override
        public void onClick(View v) {
            for (int i = 0; i < PROJECT_SIM_NUM; i++) {
                if (v == sBtnPlugOutAllSims) {
                    logd("Plug out all SIMs");
                    String cmdStr[] = {AT_CMD_SIM_PLUG_OUT, ""};
                    invokeOemRilRequest(cmdStr, PhoneConstants.SIM_ID_1);
                    break;
                } else if (v == sBtnPlugInAllSims) {
                    logd("Plug in all SIMs");
                    String cmdStr[] = {AT_CMD_SIM_PLUG_IN_ALL, ""};
                    invokeOemRilRequest(cmdStr, PhoneConstants.SIM_ID_1);
                    break;
                }
                if (v == sBtnPlugOutSim[i]) {
                    logd("Plug out SIM" + (i + 1));
                    String cmdStr[] = {AT_CMD_SIM_PLUG_OUT, ""};
                    invokeOemRilRequest(cmdStr, i);
                    break;
                }
                if (v == sBtnPlugInSim[i]) {
                    logd("Plug in SIM" + (i + 1));
                    String cmdStr[] = {AT_CMD_SIM_PLUG_IN, ""};
                    invokeOemRilRequest(cmdStr, i);
                    break;
                }
                if (v == sBtnMissingSim[i]) {
                    logd("Missing SIM" + (i + 1));
                    String cmdStr[] = {AT_CMD_SIM_MISSING, ""};
                    invokeOemRilRequest(cmdStr, i);
                    break;
                }
                if (v == sBtnRecoverySim[i]) {
                    logd("Recover SIM" + (i + 1));
                    String cmdStr[] = {AT_CMD_SIM_RECOVERY, ""};
                    invokeOemRilRequest(cmdStr, i);
                    break;
                }
            }
            Toast.makeText(getApplicationContext(), "Please wait", Toast.LENGTH_LONG).show();
        }
    };

    private void invokeOemRilRequest(String[] cmdStr, int phoneId) {
        logd("[invokeOemRilRequest] " + cmdStr[0] + ", sPhones[phoneId] " +
                sPhones[phoneId]);
        if (sPhones[phoneId] != null) {
            ((MtkGsmCdmaPhone)sPhones[phoneId]).invokeOemRilRequestStrings(cmdStr, null);
        }
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
    }

    private static void logd(String msg) {
        Rlog.d(LOG_TAG, msg);
    }
}
