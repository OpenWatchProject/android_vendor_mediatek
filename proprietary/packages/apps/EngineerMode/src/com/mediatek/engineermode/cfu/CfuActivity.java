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

package com.mediatek.engineermode.cfu;

import android.app.Activity;
import android.graphics.Color;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.telephony.SubscriptionManager;
import android.view.View;
import android.widget.Button;
import android.widget.TextView;

import com.android.internal.telephony.PhoneConstants;
import com.android.internal.telephony.uicc.IccRecords;
import com.android.internal.telephony.uicc.IccUtils;
import com.android.internal.telephony.uicc.UiccController;
import com.mediatek.engineermode.Elog;
import com.mediatek.engineermode.ModemCategory;
import com.mediatek.engineermode.R;
import com.mediatek.telephony.MtkTelephonyManagerEx;

import java.util.List;

/**
 * Description: To set CFU status.
 */
public class CfuActivity extends Activity {
    private static final String TAG = "CFU";

    private static final int EVENT_ICC_READY = 1;
    private static final int EVENT_EF_CFIS_SIM_INFO_UPDATE = 2;
    private static final int EVENT_EF_CFF_CPHS_SIM_INFO_UPDATE = 3;

    private static final int EF_INVALID = 0;
    private static final int EF_VALID = 1;

    private TextView mTvEfcfisStatus;
    private TextView mTvEfCffCphsStatus;
    private Button btRefresh;

    private int mSlot = PhoneConstants.SIM_ID_1;
    private int mSubId = SubscriptionManager.INVALID_SUBSCRIPTION_ID;
    private IccRecords mIccRecords = null;

    // SIM information
    private int mIccCardType = 0; // 0: SIM  1: USIM  2:other
    private String[] mIccCardTypes = {"SIM", "USIM", "UNSUPPORT"};

    // EF_CFIS information
    CfuSimInfoData mCfisSimInfoData = null;

    // EF_CFF_CPHS information
    CfuSimInfoData mCffCphsSimInfoData = null;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        this.setContentView(R.layout.cfu_activity);

        Elog.d(TAG, "onCreate");

        mTvEfcfisStatus = (TextView) findViewById(R.id.cfu_efcfis_status_text);
        mTvEfCffCphsStatus = (TextView) findViewById(R.id.cfu_efcff_cphs_status_text);

        btRefresh = (Button) findViewById(R.id.cfu_refresh_button);
        btRefresh.setOnClickListener(new View.OnClickListener() {
            public void onClick(View v) {
                Elog.d(TAG, "onClick: Refresh");
                refreshPage();
            }
        });

        createCfuSimInfoData();
        refreshPage();
    }

    private void refreshPage() {
        if (ModemCategory.isSimReady(mSlot)) {
            loadInCardInfoBackground();
        } else {
            mTvEfcfisStatus.setTextColor(Color.parseColor("#000000"));
            mTvEfcfisStatus.setText("N/A");

            mTvEfCffCphsStatus.setTextColor(Color.parseColor("#000000"));
            mTvEfCffCphsStatus.setText("N/A");
        }
    }

    @Override
    protected void onDestroy() {
        Elog.d(TAG, "onDestroy()");
        super.onDestroy();
    }

    private void createCfuSimInfoData() {
        mCfisSimInfoData = new CfuSimInfoData(
            "EF_CFIS",                       // name
            0x6fcb,                          // EF_id
            new String[] {"7F20", "7FFF"},   // EF_Path - SIM: DF_GSM, USIM: DF_ADF
            UiccController.APP_FAM_3GPP,     // Famiily
            "linear fixed");                 // type

        mCffCphsSimInfoData = new CfuSimInfoData(
            "EF_CFF_CPHS",                   // name
            0x6f13,                          // EF_id
            new String[] {"7F20", "7F20"},   // EF_Path - SIM: DF_GSM, USIM: DF_GSM
            UiccController.APP_FAM_3GPP,     // Famiily
            "transparent");                  // type
    }

    private final Handler mHandler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            int efValid = msg.arg1;
            String info = msg.obj.toString();

            switch (msg.what) {
                case EVENT_EF_CFIS_SIM_INFO_UPDATE:
                    if (efValid == EF_INVALID) {
                        mTvEfcfisStatus.setTextColor(Color.parseColor("#a00000"));
                    } else {
                        mTvEfcfisStatus.setTextColor(Color.parseColor("#004cc6"));
                    }
                    mTvEfcfisStatus.setText(info);
                    break;

                case EVENT_EF_CFF_CPHS_SIM_INFO_UPDATE:
                    if (efValid == EF_INVALID) {
                        mTvEfCffCphsStatus.setTextColor(Color.parseColor("#a00000"));
                    } else {
                        mTvEfCffCphsStatus.setTextColor(Color.parseColor("#004cc6"));
                    }
                    mTvEfCffCphsStatus.setText(info);
                    break;

                default:
                    break;
            }
        }
    };

    private void loadInCardInfoBackground() {
        new Thread() {
            @Override
            public void run() {
                loadSubId();
                loadIccCardType();
                if (mIccCardType < 2) {
                    loadCfuSimInfoData();
                } else {
                    mHandler.sendMessage(mHandler.obtainMessage(EVENT_EF_CFIS_SIM_INFO_UPDATE,
                            EF_INVALID, 0, "Not Supported for this card type"));
                    mHandler.sendMessage(mHandler.obtainMessage(EVENT_EF_CFF_CPHS_SIM_INFO_UPDATE,
                            EF_INVALID, 0, "Not Supported for this card type"));
                }
            }
        }.start();
    }

    private void loadSubId() {
        int[] subId = SubscriptionManager.getSubId(mSlot);
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

    private void loadIccCardType() {
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

    private void loadCfuSimInfoData() {
        // EF_CFIS
        int cfisValid = 0;
        if (mCfisSimInfoData.EF_Path[mIccCardType].equals("x")) {
            mHandler.sendMessage(mHandler.obtainMessage(EVENT_EF_CFIS_SIM_INFO_UPDATE,
                    EF_INVALID, 0, "Not Valid"));
        } else {
            List<String> data = MtkTelephonyManagerEx.getDefault().loadEFLinearFixedAll(
                            mSlot, mCfisSimInfoData.Family, mCfisSimInfoData.EF_id,
                            mCfisSimInfoData.EF_Path[mIccCardType]);
            if (data != null) {
                for (int j = 0; data != null && j < data.size(); j++) {
                    Elog.d(TAG, mCfisSimInfoData.name + "[" + j + "]" + " : "
                            + data.get(j).toUpperCase());
                }

                // Check if EF_CFIS is valid
                byte[] cfisBytes = IccUtils.hexStringToBytes(data.get(0));
                if (cfisBytes != null && cfisBytes.length == 16) {
                    mHandler.sendMessage(mHandler.obtainMessage(EVENT_EF_CFIS_SIM_INFO_UPDATE,
                            EF_VALID, 0, "Valid (" + data.get(0) + ")"));
                } else {
                    mHandler.sendMessage(mHandler.obtainMessage(EVENT_EF_CFIS_SIM_INFO_UPDATE,
                            EF_INVALID, 0, "Not Valid"));
                }
            } else {
                Elog.d(TAG, mCfisSimInfoData.name + ":" + " is empty");
                mHandler.sendMessage(mHandler.obtainMessage(EVENT_EF_CFIS_SIM_INFO_UPDATE,
                        EF_INVALID, 0, "Not Valid"));
            }
        }

        // EF_CFF_CPHS
        String cffString = "";
        if (mCffCphsSimInfoData.EF_Path[mIccCardType].equals("x")) {
            mHandler.sendMessage(mHandler.obtainMessage(EVENT_EF_CFF_CPHS_SIM_INFO_UPDATE,
                    EF_INVALID, 0, "Not Valid"));
        } else {
            byte[] data = MtkTelephonyManagerEx.getDefault().loadEFTransparent(
                            mSlot, mCffCphsSimInfoData.Family, mCffCphsSimInfoData.EF_id,
                            mCffCphsSimInfoData.EF_Path[mIccCardType]);
            if (data != null) {
                cffString = IccUtils.bytesToHexString(data);
                Elog.d(TAG, mCffCphsSimInfoData.name + ":" + cffString.toUpperCase());
                mHandler.sendMessage(mHandler.obtainMessage(EVENT_EF_CFF_CPHS_SIM_INFO_UPDATE,
                        EF_VALID, 0, "Valid (" + cffString + ")"));
            } else {
                Elog.d(TAG, mCffCphsSimInfoData.name + ":" + " is empty");
                mHandler.sendMessage(mHandler.obtainMessage(EVENT_EF_CFF_CPHS_SIM_INFO_UPDATE,
                        EF_INVALID, 0, "Not Valid"));
            }
        }
    }
}
