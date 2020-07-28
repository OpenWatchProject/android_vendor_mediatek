/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2018. All rights reserved.
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

package com.mediatek.acs;

import android.app.Activity;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.pm.ActivityInfo;
import android.graphics.Color;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.support.v4.content.LocalBroadcastManager;
import android.telephony.TelephonyManager;
import android.util.Log;
import android.view.Gravity;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.CompoundButton;
import android.widget.CompoundButton.OnCheckedChangeListener;
import android.widget.Button;
import android.widget.EditText;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.TextView;
import android.widget.ToggleButton;
import android.widget.Toast;

import com.mediatek.ims.rcsua.AcsConfiguration;
import com.mediatek.ims.rcsua.RcsUaService;

import java.text.SimpleDateFormat;
import java.util.Locale;

import org.xml.sax.SAXException;

public class AcsConfigParamActivity extends Activity {
    private Context mContext;
    private AcsUtils mAcsUtils = null;

    private EditText mVersionTV = null;
    private EditText mAcsUrlSelectionTV = null;
    private EditText mIpmeUrlSelectionTV = null;
    private EditText mConfigExpirationTV = null;
    private EditText mUsernamePW = null;
    private ToggleButton mSwitchBtn = null;
    private Button mSaveUrlBtn = null;
    private int mRequestType = -1;
    private static final String TAG = "[ACSTOOL] AcsConfigParamActivity";
    private static final String ACTION_RETRY = "com.mediatek.rcs.provisioning.https.HttpsProvisioningService.ACTION_RETRY";

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        Log.i(TAG, "onCreate");
        mContext = this;
        mAcsUtils = AcsUtils.getInstance(mContext);
        // Set layout
        setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_PORTRAIT);
        setContentView(R.layout.acs_profile_provisioning);
        registerReceiver();

        mSwitchBtn = (ToggleButton)findViewById(R.id.AcsSwitchButton);
        mSwitchBtn.setOnCheckedChangeListener(new OnCheckedChangeListener() {
                @Override
                public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
                    boolean result = false;
                    try {
                        RcsUaService uaService = mAcsUtils.getRcsUaService();
                        if (uaService == null) {
                            return;
                        }
                        result = uaService.setAcsSwitchState(isChecked);
                    } catch(Exception e) {
                        e.printStackTrace();
                    }
                }
            });
        RcsUaService uaService = mAcsUtils.getRcsUaService();
        if (uaService != null) {
            try {
                mSwitchBtn.setChecked(uaService.getAcsSwitchState());
            } catch(Exception e) {
                e.printStackTrace();
            }
        }

        Button btn = (Button)findViewById(R.id.reset_btn);
        btn.setOnClickListener(new OnClickListener() {
                @Override
                public void onClick(View v) {
                    boolean result = false;
                    try {
                        RcsUaService uaService = mAcsUtils.getRcsUaService();
                        if (uaService == null) {
                            Log.d(TAG, "uaService is null, service still not connected, return.");
                            showToast("Ua service not connected!\nPlease wait!", Color.RED);
                            return;
                        }
                        result = uaService.clearAcsConfiguration();
                    } catch(Exception e) {
                        e.printStackTrace();
                    }

                    showToast("Send reset ACS request " + (result ? "success!" : "fail!"),
                              result ? Color.GREEN : Color.RED);
                }
            });

        mAcsUrlSelectionTV = (EditText)this.findViewById(R.id.AcsUrlSelection);
        clearFocus();
        mSaveUrlBtn = (Button)findViewById(R.id.SaveUrlBtn);
        mSaveUrlBtn.setOnClickListener(new OnClickListener() {
                @Override
                public void onClick(View v) {
                    boolean result = false;
                    RcsUaService uaService = mAcsUtils.getRcsUaService();
                    if (uaService == null) {
                        return;
                    }

                    String uri = mAcsUrlSelectionTV.getText().toString();
                    try {
                        result = uaService.setAcsProvisioningAddress(uri);
                    } catch(Exception e) {
                        e.printStackTrace();
                    }
                    if (!result || uri.isEmpty()) {
                        mAcsUrlSelectionTV.setText(uaService.getAcsConfigString("ACS_URL"));
                    }
                    showToast("Set ACS URL " + (result ? "success!" : "fail!"),
                              result ? Color.GREEN : Color.RED);
                    clearFocus();
                }
            });

        mVersionTV = (EditText)this.findViewById(R.id.Config_Ver);
        mConfigExpirationTV = (EditText)this.findViewById(R.id.Config_Doc_Expiration);
        mIpmeUrlSelectionTV = (EditText)this.findViewById(R.id.IpMeUrl);
        mUsernamePW = (EditText)this.findViewById(R.id.Username_Password);
    }

    @Override
    protected void onResume() {
        super.onResume();
        Log.i(TAG, "onResume");
        clearFocus();
        update(AcsUtils.SCONNECTION_CHGNAGE);
    }

    /**
     * Describe <code>onDestroy</code> method here.
     *
     */
    public final void onDestroy() {
        super.onDestroy();
        Log.i(TAG, "onDestroy");
        unregisterReceiver();
    }

    private LocalBroadcastManager bm;
    private void registerReceiver() {
        bm = LocalBroadcastManager.getInstance(mContext);
        IntentFilter intentFilter = new IntentFilter();
        intentFilter.addAction(AcsUtils.LOCAL_ACTION);
        bm.registerReceiver(mReceiver, intentFilter);
    }

    private void unregisterReceiver() {
        if (bm != null) {
            bm.unregisterReceiver(mReceiver);
        }
    }

    private static final int MESSAGE_CONFIG_CHANGE = 1;
    private BroadcastReceiver mReceiver = new BroadcastReceiver() {
            @Override
            public void onReceive(Context context, Intent intent) {
                Log.d(TAG, "BroadcastReceiver:" + intent);
                String change = intent.getStringExtra("change");
                int type = intent.getIntExtra("type", AcsUtils.SSERVICE_CONNECTED);
                if ("yes".equals(change)) {
                    mHandler.obtainMessage(MESSAGE_CONFIG_CHANGE, type).sendToTarget();
                }
            };
        };

    private Handler mHandler = new Handler() {
            @Override
            public void handleMessage(Message msg) {
                int type = (int)msg.obj;
                Log.d(TAG, "receive msg:" + type);
                switch(type) {
                case AcsUtils.SSERVICE_CONNECTED:
                case AcsUtils.SCONFIG_CHGNAGE:
                case AcsUtils.SCONNECTION_CHGNAGE:
                    update(type);
                    break;

                default:
                    break;
                }
            }
        };

    private void update(int type) {
        RcsUaService uaService = mAcsUtils.getRcsUaService();
        if (uaService == null) {
            Log.d(TAG, "uaService is null, service still not connected, return.");
            return;
        }

        AcsConfiguration configinfo;
        long expiration = 0;
        try {
            mSwitchBtn.setChecked(uaService.getAcsSwitchState());

            String uri = uaService.getAcsConfigString("ACS_URL");
            mAcsUrlSelectionTV.setText(uri);
            String ftServer = uaService.getAcsConfigString("FtHttpServerAddr");
            mIpmeUrlSelectionTV.setText(ftServer);

            String userName = uaService.getAcsConfigString("FtHttpServerLogin");
            String passWD = uaService.getAcsConfigString("FtHttpServerPassword");
            mUsernamePW.setText(userName + "/" + passWD);

            //validity = Long.parseLong(uaService.getAcsConfigString("ACS_VALIDITY"));
            expiration = Long.parseLong(uaService.getAcsConfigString("ACS_EXPIRATION"));
            configinfo = uaService.getAcsConfiguration();
        }  catch (Exception e) {
            e.printStackTrace();
            return;
        }

        int version = configinfo.getVersion();
        mVersionTV.setText(String.valueOf(version));

        if (version != 0) {
            java.util.Date date = new java.util.Date(expiration);
            SimpleDateFormat format = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss");
            mConfigExpirationTV.setText(format.format(date));
        } else {
            mConfigExpirationTV.setText("invalid value due to version is 0");
        }



        if (configinfo == null) {
            Log.d(TAG, "configinfo is null, ignore.");
            return;
        }


        switch (type) {
        case AcsUtils.SSERVICE_CONNECTED:
        case AcsUtils.SCONFIG_CHGNAGE:
        case AcsUtils.SCONNECTION_CHGNAGE:
            Log.d(TAG, "update done, type:" + type);
            break;

        default:
            break;
        }
    }

    private void showToast(String msg, int color) {
        Toast toast = Toast.makeText(mContext,
                                     msg,
                                     Toast.LENGTH_SHORT);
        toast.setGravity(Gravity.CENTER, 0, 0);
        TextView tv = (TextView)toast.getView().findViewById(android.R.id.message);
        tv.setTextColor(color);
        tv.setTextSize(30);
        toast.show();
    }

    private void clearFocus() {
        mAcsUrlSelectionTV.post(new Runnable() {
                @Override
                public void run() {
                    mAcsUrlSelectionTV.clearFocus();
                    mAcsUrlSelectionTV.setFocusable(false);
                    mAcsUrlSelectionTV.setFocusableInTouchMode(true);
                }
            });
    }
}
