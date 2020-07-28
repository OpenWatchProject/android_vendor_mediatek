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

package com.mediatek.engineermode.siminfo;

import android.app.Activity;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.telephony.SubscriptionManager;
import android.view.View;
import android.widget.FrameLayout;
import android.widget.TextView;
import android.widget.Toast;

import com.android.internal.telephony.PhoneConstants;
import com.android.internal.telephony.uicc.IccRecords;
import com.android.internal.telephony.uicc.IccUtils;
import com.android.internal.telephony.uicc.UiccController;
import com.mediatek.engineermode.Elog;
import com.mediatek.engineermode.ModemCategory;
import com.mediatek.engineermode.R;
import com.mediatek.internal.telephony.uicc.IccServiceInfo;
import com.mediatek.internal.telephony.uicc.MtkSIMRecords;
import com.mediatek.telephony.MtkTelephonyManagerEx;

import java.util.ArrayList;
import java.util.List;


public class SimInfo extends Activity {
    private static final String TAG = "SimInfo";
    private static final String VALUE_ICC_LOADED = "LOADED";
    private static final String VALUE_ICC_ABSENT = "ABSENT";
    private static final int INTENT_VALUE_ICC_READY = 1;
    private static final int INTENT_SIM_INFO_DPDATE = 2;
    private static final int INTENT_VALUE_ICC_ABSENT = 3;
    private static final String ACTION_SIM_STATE_CHANGED
            = "android.intent.action.SIM_STATE_CHANGED";
    private static final String INTENT_KEY_ICC_STATE = "ss";
    IccRecords mIccRecords = null;
    List<SimInfoData> mSimInfoData;
    NormalTableComponent siminfo = null;
    TextView siminfo_status = null;
    private Toast mToast = null;
    private FrameLayout mInfoFrameLayout;
    private int mSimType = PhoneConstants.SIM_ID_1;
    private int mSubId = 1;
    private boolean mFirstBroadcast = true;
    private int mIccCardType = 0; // 0: SIM 1 : USIM 2:other
    private String[] mIccCardTypes = {"SIM", "USIM", "UNSUPPORT"};
    private Handler mHandler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            switch (msg.what) {
                case INTENT_VALUE_ICC_READY:
                    Elog.d(TAG, "SIM card status ready ");
                    siminfo_status.setText("sim info status: Ready , " + "card type: "
                            + mIccCardTypes[mIccCardType]);
                    break;
                case INTENT_VALUE_ICC_ABSENT:
                    Elog.d(TAG, "SIM card status absent ");
                    siminfo_status.setText("sim info status: UNReady");
                    siminfo.update(0, "");
                    siminfo.update(2, "");
                    break;

                case INTENT_SIM_INFO_DPDATE:
                    String info = msg.obj.toString();
                    if (info.equals("clean"))
                        siminfo.update(0, "");
                    else if (info.equals("update"))
                        siminfo.update(2, "");
                    else
                        siminfo.update(1, info);
                    break;
                default:
                    break;
            }
        }
    };
    private final BroadcastReceiver SimCardChangedReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            Elog.d(TAG, "SIMChangedBroadcastReceiver " + action.toString());

            if (mFirstBroadcast == true) {
                Elog.d(TAG, "it is the  mFirstBroadcast");
                mFirstBroadcast = false;
                return;
            }

            if (action != null && action.equals(ACTION_SIM_STATE_CHANGED)) {
                String newState = intent.getStringExtra(INTENT_KEY_ICC_STATE);
                int changeSlot = intent.getIntExtra(PhoneConstants.SLOT_KEY, 0);
                Elog.d(TAG, "SIM state change" + " changeSlot=" + changeSlot + " mSimType="
                        + mSimType + " new state =" + newState);
                if (changeSlot == mSimType) {
                    if (newState.equals(VALUE_ICC_LOADED)) {
                        Elog.d(TAG, "SIM card status ready ");
                        loadInCardInfoBackground();
                    } else if (newState.equals(VALUE_ICC_ABSENT)) {
                        Elog.d(TAG, "SIM card status absent ");
                        mHandler.sendEmptyMessage(INTENT_VALUE_ICC_ABSENT);
                    }
                }

            }
        }
    };

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.siminfo);
        mSimType = getIntent().getIntExtra("mSimType", PhoneConstants.SIM_ID_1);
        Elog.d(TAG, " onCreate mSimType " + mSimType);
        mInfoFrameLayout = (FrameLayout) findViewById(R.id.siminfo_frame);
        mFirstBroadcast = true;

        siminfo = new SimInfoUpdate(this);
        View view = siminfo.getView();
        if (view == null) {
            Elog.e(TAG, "updateUI view is null");
            return;
        }
        mInfoFrameLayout.removeAllViews();
        mInfoFrameLayout.addView(view);

        siminfo_status = (TextView) this.findViewById(R.id.siminfo_status);
        siminfo_status.setText("sim" + mSimType + " info status: UNReady");

        mSimInfoData = new ArrayList<SimInfoData>();
        updateSimInfoData();

        boolean recordsLoaded = ModemCategory.isSimReady(mSimType);
        if (recordsLoaded) {
            Elog.d(TAG, "loadInCardInfoBackground before registSimReceiver");
            loadInCardInfoBackground();
        }

        registSimReceiver(this);

    }

    private void updateSimInfoData() {
        mSimInfoData.add(new SimInfoData("CDR-PER-512", "EF-ICCID", 0x2fe2, new String[]{
                "3F00", "3F00"}, UiccController.APP_FAM_3GPP, "transparent", false));

        mSimInfoData.add(new SimInfoData("CDR-PER-514", "EF-MSISDN", 0x6F40, new String[]{
                "7F10", "7FFF"}, UiccController.APP_FAM_3GPP, "linear fixed", false));

        mSimInfoData.add(new SimInfoData("CDR-PER-516", "EF-CPHS MAIL BOX NUMBER", 0x6F17,
                new String[]{"7F20", "x"}, UiccController.APP_FAM_3GPP, "linear fixed",
                false));

        mSimInfoData.add(new SimInfoData("CDR-PER-518", "EF-3GPP MAIL BOX DIALING NUMBER",
                0x6FC7, new String[]{"x", "7FFF"}, UiccController.APP_FAM_3GPP,
                "linear fixed", false));

        mSimInfoData.add(new SimInfoData("CDR-PER-520", "EF-HPPLMN SEARCH PERIOD", 0x6F31,
                new String[]{"x", "7FFF"}, UiccController.APP_FAM_3GPP, "transparent",
                false));

        mSimInfoData.add(new SimInfoData("CDR-PER-522", "EF-LOCI", 0x6F71, new String[]{
                "7F20", "7FFF"}, UiccController.APP_FAM_3GPP, "transparent", false));

        mSimInfoData.add(new SimInfoData("CDR-PER-524", "EF-GPRS/PS-LOCI", 0x6F73,
                new String[]{"x", "7FFF"}, UiccController.APP_FAM_3GPP, "transparent",
                false));

        mSimInfoData.add(new SimInfoData("CDR-PER-526", "EF-ACCESS CONTROL CLASS", 0x6F78,
                new String[]{"7F20", "7FFF"}, UiccController.APP_FAM_3GPP, "transparent",
                false));

        mSimInfoData.add(new SimInfoData("CDR-PER-528", "EF-ADMINISTRATIVE DATA", 0x6FAD,
                new String[]{"7F20", "7FFF"}, UiccController.APP_FAM_3GPP, "transparent",
                false));

        mSimInfoData.add(new SimInfoData("CDR-PER-530", "EF-FPLMN", 0x6F7B, new String[]{
                "7F20", "7FFF"}, UiccController.APP_FAM_3GPP, "transparent", false));

        // mSimInfoData.add(new SimInfoData("CDR-PER-532", "EF-ACTING HPLMN",
        // 0x4F34, new String[]{"",""},
        // UiccController.APP_FAM_3GPP, "transparent", false));

        // mSimInfoData.add(new SimInfoData("CDR-PER-534", "EF-RAT MODE",
        // 0x4F36, new String[]{"",""},
        // UiccController.APP_FAM_3GPP, "transparent", false));
        mSimInfoData.add(new SimInfoData("CDR-PER-536", "EF-PLMN NETWORK NAME", 0x6FC5,
                new String[]{"x", "7FFF"}, UiccController.APP_FAM_3GPP, "linear fixed",
                false));

        mSimInfoData.add(new SimInfoData("CDR-PER-538", "EF-OPERATOR PLMN LIST", 0x6FC6,
                new String[]{"x", "7FFF"}, UiccController.APP_FAM_3GPP, "transparent",
                false));

        mSimInfoData
                .add(new SimInfoData("CDR-PER-540", "EF-PLMN SELECTOR", 0x6F30, new String[]{
                        "7F20", "x"}, UiccController.APP_FAM_3GPP, "transparent", false));

        mSimInfoData.add(new SimInfoData("CDR-PER-542", "EF-OPLMNAcT", 0x6F61, new String[]{
                "x", "7FFF"}, UiccController.APP_FAM_3GPP, "transparent", false));

        mSimInfoData.add(new SimInfoData("CDR-PER-544", "EF-IMS PUBLIC USER IDENTITY", 0x6F04,
                new String[]{"x", "7FFF"}, UiccController.APP_FAM_IMS, "linear fixed",
                false));
    }

    private void loadInCardInfoBackground() {
        // TODO (ywen) - AsyncTask to avoid creating a new thread?
        new Thread() {
            @Override
            public void run() {
                loadSubId();
                loadIccCardType();
                mHandler.sendEmptyMessage(INTENT_VALUE_ICC_READY);
                if (mIccCardType < 2) {
                    loadSimInfoData();
                }
            }
        }.start();
    }

    void registSimReceiver(Context context) {
        IntentFilter intentFilter = new IntentFilter();
        intentFilter.addAction(ACTION_SIM_STATE_CHANGED);
        context.registerReceiver(SimCardChangedReceiver, intentFilter);
    }

    void sendMsg(int MsgID, String MsgInfo) {
        Message msg = new Message();
        msg.what = MsgID;
        msg.obj = MsgInfo;
        mHandler.sendMessage(msg);
    }

    void loadSimInfoData() {
        String SimInfoData = "";

        sendMsg(INTENT_SIM_INFO_DPDATE, "clean");

        for (int i = 0; mSimInfoData != null && i < mSimInfoData.size(); i++) {
            // the sim not support
            if (mSimInfoData.get(i).EF_Path[mIccCardType].equals("x")) {
                SimInfoData = mSimInfoData.get(i).name + "is not support";
                sendMsg(INTENT_SIM_INFO_DPDATE, SimInfoData);
                continue;
            } else {
                Elog.d(TAG, mSimInfoData.get(i).num + "," + mSimInfoData.get(i).name);

                if (mSimInfoData.get(i).need_to_check) {
                    if (mIccCardType == 1) {
                        Elog.d(TAG, "USIM need to check");
                        if (mIccRecords != null) {
                            IccServiceInfo.IccServiceStatus iccSerStatus
                                    = ((MtkSIMRecords) mIccRecords)
                                    .getSIMServiceStatus(IccServiceInfo.IccService.OPLMNwACT);
                            if (iccSerStatus != IccServiceInfo.IccServiceStatus.ACTIVATED) {
                                Elog.d(TAG, "USIM not support EF-OPLMNAcT");
                                SimInfoData = mSimInfoData.get(i).name + "is not support";
                                sendMsg(INTENT_SIM_INFO_DPDATE, SimInfoData);
                                continue;
                            }
                        }
                    } else if (mIccCardType == 0) {
                        Elog.d(TAG, "SIM need to check");
                        if (mIccRecords != null) {
                            IccServiceInfo.IccServiceStatus iccSerStatus
                                    = ((MtkSIMRecords) mIccRecords)
                                    .getSIMServiceStatus(IccServiceInfo.IccService.PLMNsel);
                            if (iccSerStatus != IccServiceInfo.IccServiceStatus.ACTIVATED) {
                                Elog.d(TAG, "SIM not support EF-PLMN SELECTOR");
                                SimInfoData = mSimInfoData.get(i).name + "is not support";
                                sendMsg(INTENT_SIM_INFO_DPDATE, SimInfoData);
                                continue;
                            }
                        }
                    }
                }

                if (mSimInfoData.get(i).type.equals("transparent")) {
                    Elog.d(TAG,
                            mSimInfoData.get(i).EF_id + ","
                                    + mSimInfoData.get(i).EF_Path[mIccCardType] + ","
                                    + mSimInfoData.get(i).Family + "," + "transparent");

                    byte[] data = MtkTelephonyManagerEx.getDefault().loadEFTransparent(mSimType,
                            mSimInfoData.get(i).Family, mSimInfoData.get(i).EF_id,
                            mSimInfoData.get(i).EF_Path[mIccCardType]);

                    if (data != null) {
                        SimInfoData = IccUtils.bytesToHexString(data);
                        Elog.d(TAG, mSimInfoData.get(i).name + ":" + SimInfoData.toUpperCase());
                        SimInfoData = mSimInfoData.get(i).name + ":"
                                + SimInfoData.toUpperCase();
                    } else {
                        Elog.d(TAG, SimInfoData);
                        SimInfoData = mSimInfoData.get(i).name + " is empty";
                    }

                    sendMsg(INTENT_SIM_INFO_DPDATE, SimInfoData);

                } else if (mSimInfoData.get(i).type.equals("linear fixed")) {
                    Elog.d(TAG,
                            mSimInfoData.get(i).EF_id + ","
                                    + mSimInfoData.get(i).EF_Path[mIccCardType] + ","
                                    + mSimInfoData.get(i).Family + "," + "linear fixed");

                    List<String> data = MtkTelephonyManagerEx.getDefault().loadEFLinearFixedAll(
                            mSimType, mSimInfoData.get(i).Family, mSimInfoData.get(i).EF_id,
                            mSimInfoData.get(i).EF_Path[mIccCardType]);

                    SimInfoData = "";
                    for (int j = 0; data != null && j < data.size(); j++) {
                        Elog.d(TAG, mSimInfoData.get(i).name + "[" + j + "]" + " : "
                                + data.get(j).toUpperCase());
                        SimInfoData += mSimInfoData.get(i).name + "[" + j + "]" + ":"
                                + data.get(j).toUpperCase() + "\n";
                    }
                    if (data == null) {
                        SimInfoData = mSimInfoData.get(i).name + " is empty";
                        Elog.d(TAG, SimInfoData);
                    }
                    sendMsg(INTENT_SIM_INFO_DPDATE, SimInfoData);
                }
            }
        }
        sendMsg(INTENT_SIM_INFO_DPDATE, "update");
    }

    void loadSubId() {
        int[] subId = SubscriptionManager.getSubId(mSimType);
        if (subId != null) {
            for (int i = 0; i < subId.length; i++) {
                Elog.d(TAG, "subId[" + i + "]: " + subId[i]);
            }
        }
        if (subId == null || subId.length == 0
                || !SubscriptionManager.isValidSubscriptionId(subId[0])) {
            Elog.e(TAG, "Invalid sub id");
        } else {
            mSubId = subId[0];
        }
    }

    void loadIccCardType() {
        String IccCardType = MtkTelephonyManagerEx.getDefault().getIccCardType(mSubId);
        if (IccCardType.equals("SIM")) {
            mIccCardType = 0;
        } else if (IccCardType.equals("USIM")) {
            mIccCardType = 1;
        } else {
            mIccCardType = 2;
        }
        Elog.d(TAG, "IccCardType: " + IccCardType + " = " + mIccCardType);
    }

    @Override
    protected void onResume() {
        super.onResume();
    }

    @Override
    public void onDestroy() {
        unregisterReceiver(SimCardChangedReceiver);
        mHandler.removeMessages(INTENT_VALUE_ICC_READY);
        mHandler.removeMessages(INTENT_SIM_INFO_DPDATE);
        Elog.d(TAG, "unregisterReceiver SimCardChangedReceiver");
        super.onDestroy();
    }


    private void showToast(String msg) {
        mToast = Toast.makeText(this, msg, Toast.LENGTH_SHORT);
        mToast.show();
    }
}
