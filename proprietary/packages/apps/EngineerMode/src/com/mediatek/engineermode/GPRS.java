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

package com.mediatek.engineermode;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.content.SharedPreferences;
import android.os.AsyncResult;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.os.SystemProperties;
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;
import android.telephony.TelephonyManager.MultiSimVariants;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemSelectedListener;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.EditText;
import android.widget.LinearLayout;
import android.widget.RadioGroup;
import android.widget.Spinner;
import android.widget.TextView;

import com.android.internal.telephony.PhoneConstants;

public class GPRS extends Activity implements OnClickListener {

    String mContextCmdStringArray[] = {
            "3,128,128,0,0,1,1500,\"1e3\",\"4e3\",1,0,0",
            "3,128,128,0,0,1,1500,\"1e4\",\"1e5\",0,0,0",
            "3,128,128,0,0,1,1500,\"1e3\",\"4e3\",1,0,0",
            "3,256,256,0,0,1,1500,\"1e4\",\"1e5\",0,0,0",
            "3,128,128,0,0,1,1500,\"1e4\",\"1e5\",0,0,0",
            "3,256,256,0,0,1,1500,\"1e3\",\"4e3\",1,0,0",
            "3,256,256,0,0,1,1500,\"1e3\",\"4e3\",1,0,0",
            "3,128,128,0,0,1,1500,\"1e4\",\"1e5\",0,0,0",
            "3,128,128,0,0,1,1500,\"1e4\",\"1e5\",0,0,0",
            "3,128,128,0,0,1,1500,\"1e3\",\"4e3\",1,0,0",
            "3,128,128,0,0,1,1500,\"1e6\",\"1e5\",0,0,0",
            "3,128,128,0,0,1,1500,\"1e6\",\"1e5\",0,0,0",
            "3,128,128,0,0,1,1500,\"1e6\",\"1e5\",0,0,0",
            "3,128,128,0,0,1,1500,\"1e4\",\"1e5\",0,0,0",
            "3,256,256,0,0,1,1500,\"1e3\",\"4e3\",1,0,0",
            "3,512,512,0,0,1,1500,\"1e4\",\"1e5\",0,0,0" };

    private static final int EVENT_GPRS_ATTACHED = 1;
    private static final int EVENT_GPRS_DETACHED = 2;
    private static final int EVENT_PDP_ACTIVATE = 3;
    private static final int EVENT_PDP_DEACTIVATE = 4;
    private static final int EVENT_SEND_DATA = 5;
    private static final int EVENT_GPRS_INTERNAL_AT = 6;
    private static final int EVENT_GPRS_FD = 7;
    private static final int EVENT_GPRS_ATTACH_TYPE = 8;

    private static final String PROPERTY_PDN_REUSE = "vendor.ril.pdn.reuse";
    private boolean mIsDsds = false;
    private boolean mFlag = true;

    static final String TAG = "GPRS";

    private Button mBtnSim1;
    private Button mBtnSim2;
    private Button mBtnAttached;
    private Button mBtnDetached;
    private Button mBtnAttachedContinue;
    private Button mBtnDetachedContinue;
    private Button mBtnNotSpecify;
    private RadioGroup mRaGpPDPSelect;
    private RadioGroup mRaGpUsageSelect;
    private Spinner mSPinnerPDPContext;
    private Button mBtnActivate;
    private Button mBtnDeactivate;
    private EditText mEditDataLen;
    private Button mBtnSendData;
    private RadioGroup mGprstAttachSelect;
    private Button mBtnPdnReuseOn;
    private Button mBtnPdnReuseOff;
    private TextView mAttachApn;

    private int mPDPSelect = 0;
    private int mUsageSelect = 0;
    private int mPDPContextIndex = 0;

    private ArrayAdapter<String> mSpinnerAdapter;

    private boolean mAlive = false;

    private AlertDialog mAlertDialog;

    public static final String PREFERENCE_GPRS = "com.mtk.GPRS";
    public static final String PREF_ATTACH_MODE = "ATTACH_MODE";
    public static final String PREF_ATTACH_MODE_SIM = "ATTACH_MODE_SIM";
    public static final int ATTACH_MODE_ALWAYS = 1;
    public static final int ATTACH_MODE_WHEN_NEEDED = 0;
    public static final int ATTACH_MODE_NOT_SPECIFY = -1;

    private static final int PDP_CONTEXT_MAX = 15;

    @Override
    public void onDestroy() {
        mAlive = false;
        super.onDestroy();
        if (mAlertDialog != null && mAlertDialog.isShowing()) {
            mAlertDialog.dismiss();
        }
    }

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.gprs);

        Elog.v(TAG, "onCreate");

        MultiSimVariants config = TelephonyManager.getDefault().getMultiSimConfiguration();
        mIsDsds = (config == MultiSimVariants.DSDS || config == MultiSimVariants.DSDA);
        Elog.v(TAG, "onCreate config = " + config + " mIsDsds = " + mIsDsds);

        mAlive = true;

        // create ArrayAdapter for Spinner
        mSpinnerAdapter = new ArrayAdapter<String>(this,
                android.R.layout.simple_spinner_item);
        mSpinnerAdapter
                .setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        for (int i = 1; i < PDP_CONTEXT_MAX; i++) {
            mSpinnerAdapter.add("PDP Context " + String.valueOf(i));
        }
        mSpinnerAdapter.add("PDP Context 30");
        mSpinnerAdapter.add("PDP Context 31");

        // get the object of the controls
        mBtnSim1 = (Button) findViewById(R.id.Sim1);
        mBtnSim2 = (Button) findViewById(R.id.Sim2);
        mBtnSim1.setOnClickListener(this);
        mBtnSim2.setOnClickListener(this);
        if (!mIsDsds) {
            mBtnSim1.setVisibility(View.GONE);
            mBtnSim2.setVisibility(View.GONE);
        }

        mBtnAttached = (Button) findViewById(R.id.Attached);
        mBtnDetached = (Button) findViewById(R.id.Detached);
        mBtnAttachedContinue = (Button) findViewById(R.id.always_mode_continue);
        mBtnDetachedContinue = (Button) findViewById(R.id.when_needed_continue);
        mBtnNotSpecify = (Button) findViewById(R.id.not_specify);

        mRaGpPDPSelect = (RadioGroup) findViewById(R.id.PDPSelect);
        mRaGpUsageSelect = (RadioGroup) findViewById(R.id.UsageSelect);
        mSPinnerPDPContext = (Spinner) findViewById(R.id.ContextNumber);
        mBtnActivate = (Button) findViewById(R.id.Activate);
        mBtnDeactivate = (Button) findViewById(R.id.Deactivate);
        mEditDataLen = (EditText) findViewById(R.id.DataLength);
        mBtnSendData = (Button) findViewById(R.id.SendData);

        mGprstAttachSelect = (RadioGroup) findViewById(R.id.GprsAttachType);

        mBtnPdnReuseOn = (Button) findViewById(R.id.pdn_reuse_on);
        mBtnPdnReuseOff = (Button) findViewById(R.id.pdn_reuse_off);

        mAttachApn = (TextView)  findViewById(R.id.attachApn);

        // setOnClickListener for the controls
        mBtnAttached.setOnClickListener(this);
        mBtnDetached.setOnClickListener(this);
        mBtnAttachedContinue.setOnClickListener(this);
        mBtnDetachedContinue.setOnClickListener(this);
        mBtnNotSpecify.setOnClickListener(this);
        mSPinnerPDPContext.setAdapter(mSpinnerAdapter);
        mBtnActivate.setOnClickListener(this);
        mBtnDeactivate.setOnClickListener(this);
        mBtnSendData.setOnClickListener(this);
        mBtnPdnReuseOn.setOnClickListener(this);
        mBtnPdnReuseOff.setOnClickListener(this);

        mSPinnerPDPContext
                .setOnItemSelectedListener(new OnItemSelectedListener() {

                    public void onItemSelected(AdapterView<?> arg0, View arg1,
                            int arg2, long arg3) {
                        // TODO Auto-generated method stub
                        mPDPContextIndex = arg2;
                    }

                    public void onNothingSelected(AdapterView<?> arg0) {
                        // TODO Auto-generated method stub

                    }

                });

        mGprstAttachSelect
                .setOnCheckedChangeListener(new RadioGroup.OnCheckedChangeListener() {
                    public void onCheckedChanged(RadioGroup arg0, int arg1) {
                        Elog.v(TAG, "onCheckedChanged:GprsAlwaysAttach");
                        // TODO Auto-generated method stub
                        if (arg0.getCheckedRadioButtonId() == R.id.GprsAlwaysAttach) {
                            String cmdStr[] = { "AT+EGTYPE=1", "" };
                            sendAtCommand(cmdStr,EVENT_GPRS_ATTACH_TYPE);
                        }
                        if (arg0.getCheckedRadioButtonId() == R.id.GprsWhenNeeded) {
                            Elog.v(TAG, "onCheckedChanged:GprsWhenNeeded");
                            String cmdStr[] = { "AT+EGTYPE=0", "" };
                            sendAtCommand(cmdStr,EVENT_GPRS_ATTACH_TYPE);
                        }
                    }
                });

        mRaGpPDPSelect
                .setOnCheckedChangeListener(new RadioGroup.OnCheckedChangeListener() {
                    public void onCheckedChanged(RadioGroup arg0, int arg1) {
                        if (arg0.getCheckedRadioButtonId() == R.id.FirstPDP) {
                            mPDPSelect = 0;
                            mRaGpUsageSelect.clearCheck();
                            mRaGpUsageSelect.getChildAt(0).setEnabled(false);
                            mRaGpUsageSelect.getChildAt(1).setEnabled(false);
                        }
                        if (arg0.getCheckedRadioButtonId() == R.id.SecondPDP) {
                            mPDPSelect = 1;
                            mRaGpUsageSelect.check(R.id.Primary);
                            mRaGpUsageSelect.getChildAt(0).setEnabled(true);
                            mRaGpUsageSelect.getChildAt(1).setEnabled(true);
                        }
                    }
                });

        mRaGpUsageSelect
                .setOnCheckedChangeListener(new RadioGroup.OnCheckedChangeListener() {
                    public void onCheckedChanged(RadioGroup arg0, int arg1) {
                        if (arg0.getCheckedRadioButtonId() == R.id.Primary) {
                            mUsageSelect = 0;
                        }
                        if (arg0.getCheckedRadioButtonId() == R.id.Secondary) {
                            mUsageSelect = 1;
                        }
                    }
                });

        // set initial value for PDP select
        mRaGpPDPSelect.check(R.id.FirstPDP);

        int gprsAttachType = SystemProperties.getInt(
                "persist.vendor.radio.gprs.attach.type", 1);

        if (gprsAttachType == 1) {
            mGprstAttachSelect.check(R.id.GprsAlwaysAttach);
        } else {
            mGprstAttachSelect.check(R.id.GprsWhenNeeded);
        }

        showDefaultSim();

        AlertDialog.Builder builder = new AlertDialog.Builder(GPRS.this);
        mAlertDialog = builder.create();
        mAlertDialog.setButton(DialogInterface.BUTTON_POSITIVE, "OK" ,
                                            (DialogInterface.OnClickListener)null);

        // Disable following UI after 93 MD
        // 1. PS attach/detach
        // 2. PDN related
        // 3. IA APN label
        LinearLayout layoutPsAttach = (LinearLayout) findViewById(R.id.layout_ps_attach);
        LinearLayout layoutPdn = (LinearLayout) findViewById(R.id.layout_pdn);
        LinearLayout layoutIaApn = (LinearLayout) findViewById(R.id.layout_ia_apn);
        LinearLayout layoutReusePdn = (LinearLayout) findViewById(R.id.layout_reuse_pdn);
        if (SystemProperties.get("ro.vendor.mtk_ril_mode").equals("c6m_1rild")) {
            layoutPsAttach.setVisibility(View.GONE);
            layoutIaApn.setVisibility(View.GONE);
            layoutReusePdn.setVisibility(View.GONE);
        } else {
            layoutPsAttach.setVisibility(View.VISIBLE);
            layoutIaApn.setVisibility(View.VISIBLE);
            layoutReusePdn.setVisibility(View.VISIBLE);
        }
    }

    @Override
    public void onResume() {
        super.onResume();
        updateAttachModeMMI();

        if ("0".equals(SystemProperties.get(PROPERTY_PDN_REUSE, "1"))) {
            mBtnPdnReuseOn.setEnabled(true);
            mBtnPdnReuseOff.setEnabled(false);
        } else {
            mBtnPdnReuseOn.setEnabled(false);
            mBtnPdnReuseOff.setEnabled(true);
        }
    }

    @Override
    public void onClick(View arg0) {
        Elog.v(TAG, "onClick:" + arg0.getId());

        if (arg0 == mBtnSim1) {
            if (mIsDsds) {
                int[] subIds = SubscriptionManager.getSubId(PhoneConstants.SIM_ID_1);
                if (subIds != null) {
                    Elog.v(TAG, "onClick:Phone1 subId = " + subIds[0]);
                    SubscriptionManager.from(GPRS.this).setDefaultDataSubId(subIds[0]);
                }
            }

            Elog.v(TAG, "onClick:SIM 1");
            showDefaultSim();
        }

        if (arg0 == mBtnSim2) {
            int[] subIds = SubscriptionManager.getSubId(PhoneConstants.SIM_ID_2);
            if (subIds != null) {
                Elog.v(TAG, "onClick:Phone2 subId = " + subIds[0]);
                SubscriptionManager.from(GPRS.this).setDefaultDataSubId(subIds[0]);
            }

            Elog.v(TAG, "onClick:SIM 2");
            showDefaultSim();
        }

        if (arg0.getId() == mBtnAttached.getId()) {
            String attachedAT[] = { "AT+CGATT=1", "" };
            sendAtCommand(attachedAT,EVENT_GPRS_ATTACHED);
        }

        if (arg0.getId() == mBtnDetached.getId()) {
            String detachedAT[] = { "AT+CGATT=0", "" };
            sendAtCommand(detachedAT,EVENT_GPRS_DETACHED);
        }

        SharedPreferences preference = getSharedPreferences(PREFERENCE_GPRS, 0);
        SharedPreferences.Editor editor = preference.edit();

        if (arg0 == mBtnAttachedContinue) {
            Elog.v(TAG, "onClick:mBtnAttachedContinue");
            String cmdStr[] = { "AT+EGTYPE=1,1", "" };
            sendAtCommand(cmdStr,EVENT_GPRS_ATTACH_TYPE);
            editor.putInt(PREF_ATTACH_MODE, ATTACH_MODE_ALWAYS);
        } else if (arg0 == mBtnDetachedContinue) {
            Elog.v(TAG, "onClick:mBtnDetachedContinue");
            String cmdStr[] = { "AT+EGTYPE=0,1", "" };
            sendAtCommand(cmdStr,EVENT_GPRS_ATTACH_TYPE);
            editor.putInt(PREF_ATTACH_MODE, ATTACH_MODE_WHEN_NEEDED);
        } else if (arg0 == mBtnNotSpecify) {
            editor.putInt(PREF_ATTACH_MODE, ATTACH_MODE_NOT_SPECIFY);
        }

        editor.putInt(PREF_ATTACH_MODE_SIM, getDefaultDataPhoneId());
        editor.commit();
        updateAttachModeMMI();

        if (arg0.getId() == mBtnActivate.getId()) {
            mFlag = true;

            if (0 == mPDPSelect) {
                String activateAT[] = { "AT+CGQMIN=1", "" };
                sendAtCommand(activateAT,EVENT_GPRS_INTERNAL_AT);
                activateAT[0] = "AT+CGQREQ=1";
                activateAT[1] = "";
                sendAtCommand(activateAT,EVENT_GPRS_INTERNAL_AT);
                activateAT[0] = "AT+CGDCONT=1,\"IP\",\"internet\",\"192.168.1.1\",0,0";
                activateAT[1] = "";
                sendAtCommand(activateAT,EVENT_GPRS_INTERNAL_AT);
                activateAT[0] = "AT+CGEQREQ=1,"
                        + mContextCmdStringArray[mPDPContextIndex];
                activateAT[1] = "";
                sendAtCommand(activateAT,EVENT_GPRS_INTERNAL_AT);

                activateAT[0] = "AT+ACTTEST=1,1";
                activateAT[1] = "";
                sendAtCommand(activateAT,EVENT_PDP_ACTIVATE);
            }
            if (1 == mPDPSelect) {
                String activateAT[] = { "AT+CGQMIN=2", "" };
                sendAtCommand(activateAT,EVENT_GPRS_INTERNAL_AT);
                activateAT[0] = "AT+CGQREQ=2";
                activateAT[1] = "";
                sendAtCommand(activateAT,EVENT_GPRS_INTERNAL_AT);
                if (0 == mUsageSelect) {
                    activateAT[0] = "AT+CGDCONT=2,\"IP\",\"internet\",\"192.168.1.1\",0,0";
                }
                if (1 == mUsageSelect) {
                    activateAT[0] = "AT+CGDSCONT=2,1,0,0";
                }
                activateAT[1] = "";
                sendAtCommand(activateAT,EVENT_GPRS_INTERNAL_AT);
                activateAT[0] = "AT+CGEQREQ=2,"
                        + mContextCmdStringArray[mPDPContextIndex];
                activateAT[1] = "";
                sendAtCommand(activateAT,EVENT_GPRS_INTERNAL_AT);
                activateAT[0] = "AT+ACTTEST=1,2";
                activateAT[1] = "";
                sendAtCommand(activateAT,EVENT_PDP_ACTIVATE);
            }
        }

        if (arg0.getId() == mBtnDeactivate.getId()) {
            mFlag = true;

            String deactivateAT[] = new String[2];
            if (0 == mPDPSelect) {
                deactivateAT[0] = "AT+ACTTEST=0,1";
            }
            if (1 == mPDPSelect) {
                deactivateAT[0] = "AT+ACTTEST=0,2";
            }
            deactivateAT[1] = "";
            sendAtCommand(deactivateAT,EVENT_PDP_DEACTIVATE);
        }

        if (arg0.getId() == mBtnSendData.getId()) {
            String strDataLength = mEditDataLen.getText().toString();
            String sendDataAT[] = new String[2];
            if (0 == mPDPSelect) {
                sendDataAT[0] = "AT+CGSDATA=" + strDataLength + ",1";
            }
            if (1 == mPDPSelect) {
                sendDataAT[0] = "AT+CGSDATA=" + strDataLength + ",2";
            }
            sendDataAT[1] = "";
            sendAtCommand(sendDataAT,EVENT_SEND_DATA);
        }

        if (arg0.getId() == mBtnPdnReuseOn.getId()) {
            setSystemProperty("vendor.ril.pdn.reuse", "1");
            mBtnPdnReuseOn.setEnabled(false);
            mBtnPdnReuseOff.setEnabled(true);
        } else if (arg0.getId() == mBtnPdnReuseOff.getId()) {
            setSystemProperty("vendor.ril.pdn.reuse", "0");
            mBtnPdnReuseOn.setEnabled(true);
            mBtnPdnReuseOff.setEnabled(false);
        }
    }

    private Handler mResponseHander = new Handler() {
        @Override
        public void handleMessage(Message msg) {

            if (!mAlive) {
                return;
            }

            if (mAlertDialog.isShowing()) {
                mAlertDialog.dismiss();
            }

            AsyncResult ar;
            switch (msg.what) {
                case EVENT_GPRS_ATTACHED:
                    ar = (AsyncResult) msg.obj;
                    if (ar.exception == null) {
                        mAlertDialog.setTitle("GPRS Attached");
                        mAlertDialog.setMessage("GPRS Attached succeeded.");
                        mAlertDialog.show();
                    } else {
                        mAlertDialog.setTitle("GPRS Attached");
                        mAlertDialog.setMessage("GPRS Attache failed.");
                        mAlertDialog.show();
                    }
                    break;
                case EVENT_GPRS_DETACHED:
                    ar = (AsyncResult) msg.obj;
                    if (ar.exception == null) {
                        mAlertDialog.setTitle("GPRS Detached");
                        mAlertDialog.setMessage("GPRS Detached succeeded.");
                        mAlertDialog.show();
                    } else {
                        mAlertDialog.setTitle("GPRS Detached");
                        mAlertDialog.setMessage("GPRS Detached failed.");
                        mAlertDialog.show();
                    }
                    break;
                case EVENT_GPRS_FD:
                    ar = (AsyncResult) msg.obj;
                    if (ar.exception == null) {
                        mAlertDialog.setTitle("GPRS Fast Dormancy");
                        mAlertDialog.setMessage("GPRS Fast Dormancy command succeeded.");
                        mAlertDialog.show();
                    } else {
                        mAlertDialog.setTitle("GPRS Fast Dormancy");
                        mAlertDialog.setMessage("GPRS Fast Dormancy command failed.");
                        mAlertDialog.show();
                    }
                    break;
                case EVENT_PDP_ACTIVATE:
                    ar = (AsyncResult) msg.obj;
                    if (ar.exception == null && mFlag) {
                        mAlertDialog.setTitle("PDP Activate");
                        mAlertDialog.setMessage("PDP Activate succeeded.");
                        mAlertDialog.show();
                    } else {
                        mAlertDialog.setTitle("PDP Activate");
                        mAlertDialog.setMessage("PDP Activate failed.");
                        mAlertDialog.show();
                    }
                    break;
                case EVENT_PDP_DEACTIVATE:
                    ar = (AsyncResult) msg.obj;
                    if (ar.exception == null) {
                        mAlertDialog.setTitle("PDP Deactivate");
                        mAlertDialog.setMessage("PDP Deactivate succeeded.");
                        mAlertDialog.show();
                    } else {
                        mAlertDialog.setTitle("PDP Deactivate");
                        mAlertDialog.setMessage("PDP Deactivate failed.");
                        mAlertDialog.show();
                    }
                    break;
                case EVENT_SEND_DATA:
                    ar = (AsyncResult) msg.obj;
                    if (ar.exception == null) {
                        mAlertDialog.setTitle("Send Data");
                        mAlertDialog.setMessage("Send Data succeeded.");
                        mAlertDialog.show();
                    } else {
                        mAlertDialog.setTitle("Send Data");
                        mAlertDialog.setMessage("Send Data failed.");
                        mAlertDialog.show();
                    }
                    break;
                case EVENT_GPRS_INTERNAL_AT:
                    ar = (AsyncResult) msg.obj;
                    if (ar.exception != null) {
                        mFlag = false;
                    }
                    break;
                default:
                    break;
            }
        }
    };

    private void showDefaultSim() {
        int simId = getDefaultDataPhoneId();
        Elog.v(TAG, "showDefaultSim: simId = " + simId);

        if (mIsDsds) {
            if (simId == PhoneConstants.SIM_ID_1) {
                mBtnSim1.setEnabled(false);
                mBtnSim2.setEnabled(true);
            } else if (simId == PhoneConstants.SIM_ID_2) {
                mBtnSim1.setEnabled(true);
                mBtnSim2.setEnabled(false);
            }
        }

    }

    private void updateAttachModeMMI() {
        SharedPreferences preference = getSharedPreferences(PREFERENCE_GPRS, 0);
        int attachMode = preference.getInt(PREF_ATTACH_MODE,
                ATTACH_MODE_NOT_SPECIFY);

        switch (attachMode) {
        case ATTACH_MODE_ALWAYS:
            mBtnAttachedContinue.setEnabled(false);
            mBtnDetachedContinue.setEnabled(true);
            mBtnNotSpecify.setEnabled(true);
            break;
        case ATTACH_MODE_WHEN_NEEDED:
            mBtnAttachedContinue.setEnabled(true);
            mBtnDetachedContinue.setEnabled(false);
            mBtnNotSpecify.setEnabled(true);
            break;
        case ATTACH_MODE_NOT_SPECIFY:
            mBtnAttachedContinue.setEnabled(true);
            mBtnDetachedContinue.setEnabled(true);
            mBtnNotSpecify.setEnabled(false);
            break;
        default:
            break;
        }

        //[ICCID],[Protocol],[Auth Type],[Username],[APN]
        String attachApnCache = SystemProperties.get("persist.vendor.radio.ia");
        String networkAssignedApn = SystemProperties.get("vendor.ril.ia.network");
        if (attachApnCache == null || attachApnCache.length() == 0) {
            mAttachApn.setText("Attach APN: no attach apn is set");
        } else {
            String[] splitedString = attachApnCache.split(",");
            if (splitedString.length == 5) {
                mAttachApn.setText("Attach APN: " + splitedString[4] +
                        "\nProtocol: " + splitedString[1] +
                    "\nAuth Type: " + splitedString[2] + "\nUsername: " + splitedString[3] +
                    "\nNetwork assigned APN: " + networkAssignedApn);
            } else {
                mAttachApn.setText("Attach APN: invalid value (error)");
            }
        }
    }

    private int getDefaultDataPhoneId() {
        int phoneId = SubscriptionManager.from(GPRS.this).getDefaultDataPhoneId();
        if (phoneId < PhoneConstants.SIM_ID_1 || phoneId > PhoneConstants.SIM_ID_2) {
            Elog.v(TAG, "invalid Phone id, set to sim 1");
            phoneId = PhoneConstants.SIM_ID_1;
        }
        return phoneId;
    }

    private void sendAtCommand(String[] command, int msg) {
        Elog.i(TAG, "sendAtCommand() " + command[0]);
        EmUtils.invokeOemRilRequestStringsEm(command, mResponseHander.obtainMessage(msg));
    }

    private void setSystemProperty(String key, String value) {
        Elog.i(TAG, "setSystemProperty() key = " + key + " value = " + value);
        try {
            EmUtils.getEmHidlService().setEmConfigure(key, value);
        } catch (Exception e) {
            Elog.e(TAG, "setSystemProperty() failed");
        }
    }
}
