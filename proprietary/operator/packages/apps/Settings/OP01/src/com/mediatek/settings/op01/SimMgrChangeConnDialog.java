/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2017. All rights reserved.
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

package com.mediatek.settings.op01;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.AlertDialog.Builder;
import android.app.Dialog;
import android.app.ProgressDialog;
import android.app.StatusBarManager;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.IntentFilter;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.os.RemoteException;
import android.os.ServiceManager;
import android.os.SystemProperties;
import android.provider.Settings;
import android.telecom.PhoneAccount;
import android.telecom.PhoneAccountHandle;
import android.telecom.TelecomManager;
import android.telephony.RadioAccessFamily;
import android.telephony.SubscriptionInfo;
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;
import android.text.Spannable;
import android.text.SpannableStringBuilder;
import android.text.style.BackgroundColorSpan;
import android.util.Log;
import android.view.KeyEvent;
import android.widget.Toast;

import com.android.internal.telephony.ITelephony;
import com.android.internal.telephony.PhoneConstants;
import com.android.internal.telephony.TelephonyIntents;
import com.android.internal.util.CharSequences;

import com.mediatek.widget.AccountViewAdapter.AccountElements;

import java.util.List;

public class SimMgrChangeConnDialog extends Activity {
    private static final String TAG = "SimMgrChangeConnDialog";

    private Context mContext;
    private IntentFilter mIntentFilter;
    private ProgressDialog mWaitDlg;
    private boolean mIsDataSwitchWaiting = false;
    private int mToCloseSlot = -1;
    private static final int DIALOG_ID_CHANGE_DATA_CONNECTION = 1000;
    private static final int DIALOG_ID_PROGRESS = 1001;
    
    private static final int DATA_SWITCH_TIME_OUT_MSG = 2000;
    private static final int DATA_SWITCH_TIME_OUT_TIME = 10000;
    
    private BroadcastReceiver mSubReceiver = new BroadcastReceiver() {
        
            @Override
            public void onReceive(Context context, Intent intent) {
                String action = intent.getAction();
                Log.d("@M_" + TAG, "mSubReceiver action = " + action);
                if (action.equals(TelephonyIntents.ACTION_SUBINFO_CONTENT_CHANGE)
                        || action.equals(TelephonyIntents.ACTION_SUBINFO_RECORD_UPDATED)) {
                        int[] subids = SubscriptionManager.from(mContext).getActiveSubscriptionIdList();
                        if (subids == null || subids.length <= 1) {
                                Log.d("@M_" + TAG, "onReceive dealWithDataConnChanged dismiss AlertDlg"); 
                                finish();
                        }
                } else if (TelephonyIntents.ACTION_DEFAULT_DATA_SUBSCRIPTION_CHANGED.equals(action)) {
                    int subId = intent.getIntExtra(PhoneConstants.SUBSCRIPTION_KEY,
                            SubscriptionManager.INVALID_SUBSCRIPTION_ID);
                    //dealDataConnectionChanged(subId);
                    Log.d("@M_" + TAG, "changed default data subId: " + subId);
                } else if (action.equals(Intent.ACTION_AIRPLANE_MODE_CHANGED)) {
                    finish();
                } else if (action.equals(TelephonyIntents.ACTION_SET_RADIO_CAPABILITY_DONE)) {
                    Log.d("@M_" + TAG, "Received Intent TelephonyIntents.ACTION_SET_PHONE_RAT_FAMILY_DONE");
                    int subId = SubscriptionManager.getDefaultDataSubscriptionId();
                    dealDataConnectionChanged(subId);
                } else if (action.equals(TelephonyIntents.ACTION_SET_RADIO_CAPABILITY_FAILED)) {
                    Log.d("@M_" + TAG, "Received Intent TelephonyIntents.ACTION_SET_PHONE_RAT_FAMILY_DONE");
                    int subId = SubscriptionManager.getDefaultDataSubscriptionId();
                    dealDataConnectionChanged(subId);
                }
            }
    };

    //Timeout handler
    private Handler mTimerHandler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            if (DATA_SWITCH_TIME_OUT_MSG == msg.what) {

                Log.i("@M_" + TAG, "reveive time out msg...");
                if (mIsDataSwitchWaiting) {

                    mTimerHandler.removeMessages(DATA_SWITCH_TIME_OUT_MSG);
                    if (mWaitDlg !=null && mWaitDlg.isShowing()) {
                        mWaitDlg.dismiss();
                        finish();
                    }
                    mIsDataSwitchWaiting = false;
                }
            }
        }
    };


    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        super.setFinishOnTouchOutside(false);
        mContext = this;
        
        mToCloseSlot = getIntent().getIntExtra("slotId", -1);
        Log.d("@M_" + TAG, "Slot id =" + mToCloseSlot);
        mIntentFilter = new IntentFilter(TelephonyIntents.ACTION_SUBINFO_CONTENT_CHANGE);
        mIntentFilter.addAction(TelephonyIntents.ACTION_DEFAULT_DATA_SUBSCRIPTION_CHANGED);
        mIntentFilter.addAction(Intent.ACTION_AIRPLANE_MODE_CHANGED);
        
        // For SIM Switch
        mIntentFilter.addAction(TelephonyIntents.ACTION_SET_RADIO_CAPABILITY_DONE);
        mIntentFilter.addAction(TelephonyIntents.ACTION_SET_RADIO_CAPABILITY_FAILED);
        
        registerReceiver(mSubReceiver, mIntentFilter);
        boolean isAirPlaneModeOn = Settings.System.getInt(mContext.getContentResolver(), 
                    Settings.Global.AIRPLANE_MODE_ON, 0) != 0;
        if (!isAirPlaneModeOn) {
            showDialog(DIALOG_ID_CHANGE_DATA_CONNECTION);
        } else {
            finish();
        }
    }


    @Override
    public Dialog onCreateDialog(int id, Bundle data) {
        switch (id) {
            case DIALOG_ID_CHANGE_DATA_CONNECTION: {
                AlertDialog.Builder builder = new AlertDialog.Builder(this);
                AlertDialog dialog = getChangeDataConnDialog(builder);
                if (dialog == null) {
                    finish();
                    return null;
                }
                dialog.setCanceledOnTouchOutside(false);
                dialog.setOnKeyListener(new DialogInterface.OnKeyListener() {
                    @Override
                    public boolean onKey(DialogInterface dialog, int keyCode, KeyEvent event) {
                        if (keyCode == KeyEvent.KEYCODE_BACK && event.getAction() == KeyEvent.ACTION_DOWN) {
                            finish();
                            return true;
                        } else {
                            return false;
                        }
                    }
                });
                return dialog;
            }
                        
            default:
                return null;
        }
    }

    @Override
    protected void onDestroy() {
        Log.d("@M_" + TAG, "onDestroy()");
        unregisterReceiver(mSubReceiver);
        mTimerHandler.removeMessages(DATA_SWITCH_TIME_OUT_MSG);
        mToCloseSlot = -1;
        super.onDestroy();
    }

    private AlertDialog getChangeDataConnDialog(Builder builder) {

        Log.d("@M_" + TAG, "getChangeDataConnDialog()");
        final int toCloseSlot = mToCloseSlot;
        SubscriptionInfo currentSiminfo =
               SubscriptionManager.from(mContext).getActiveSubscriptionInfoForSimSlotIndex(toCloseSlot);
        SubscriptionInfo anotherSiminfo =
               SubscriptionManager.from(mContext).getActiveSubscriptionInfoForSimSlotIndex(1 - toCloseSlot);

        if (currentSiminfo == null || anotherSiminfo == null) {
            Log.d("@M_" + TAG, "getChangeDataConnDialog() can't get some sim info");
            return null;
        }

        CharSequence currentSimName = currentSiminfo.getDisplayName();
        CharSequence anotherSimName = anotherSiminfo.getDisplayName();

        if (currentSimName == null) {
            currentSimName = "SIM " + (currentSiminfo.getSimSlotIndex() + 1);
        }
        if (anotherSimName == null) {
            anotherSimName = "SIM " + (anotherSiminfo.getSimSlotIndex() + 1);
        }
        Log.d("@M_" + TAG, "currentSimName:" + currentSimName + "\n anotherSimName:" + anotherSimName);

        Bitmap currSimResId = currentSiminfo.createIconBitmap(mContext);
        Bitmap otherSimResId = anotherSiminfo.createIconBitmap(mContext);
        int currentSimColor = getDrawableColorValue(currSimResId);
        int anotherSimColor = getDrawableColorValue(otherSimResId);

        Log.d("@M_" + TAG, "mToClosedSimCard = " + toCloseSlot);

        String message = String.format(currentSimName +
               mContext.getString(R.string.change_data_conn_message) +
               anotherSimName +
               "?");

        int currentSimStartIdx = message.indexOf(currentSimName.toString());
        int currentSimEndIdx = currentSimStartIdx + currentSimName.length();

        int anotherSimStartIdx = currentSimEndIdx
                + (mContext.getString(R.string.change_data_conn_message)).length();
        int anotherSimEndIdx = anotherSimStartIdx + anotherSimName.length();

        SpannableStringBuilder style = new SpannableStringBuilder(message);

        style.setSpan(new BackgroundColorSpan(currentSimColor), currentSimStartIdx,
               currentSimEndIdx, Spannable.SPAN_EXCLUSIVE_EXCLUSIVE);

        style.setSpan(new BackgroundColorSpan(anotherSimColor), anotherSimStartIdx,
               anotherSimEndIdx, Spannable.SPAN_EXCLUSIVE_EXCLUSIVE);

        builder.setTitle(mContext.getString(R.string.change_data_conn_title));
        builder.setMessage(style);

        builder.setPositiveButton(android.R.string.yes,
               new DialogInterface.OnClickListener() {
                      public void onClick(DialogInterface dialog, int whichButton) {
                           Log.d("@M_" + TAG, "Perform On click ok");
                           removeDialog(DIALOG_ID_CHANGE_DATA_CONNECTION);
                           int subid = getSubIdBySlot(1 - toCloseSlot);
                           Log.d("@M_" + TAG, "Auto Switch GPRS Sim id = " + subid);
                           if (TelecomManager.from(mContext).isInCall()) {
                               Log.d("@M_" + TAG, "Perform On click ok,InCall");
                               Toast.makeText(mContext, R.string.default_data_switch_err_msg1,
                                       Toast.LENGTH_SHORT).show();
                               finish();
                           } else {
                           switchGprsDefautSIM(subid);
                       }

                       }
                       });

        builder.setNegativeButton(android.R.string.cancel,
               new DialogInterface.OnClickListener() {
                      public void onClick(DialogInterface dialog, int whichButton) {
                           Log.d("@M_" + TAG, "Perform On click cancel");
                           finish();
                       }
                       });
        

        return builder.create();

    }
    
    private int getSubIdBySlot(int slotId) {
            Log.d("@M_" + TAG, "SlotId = " + slotId);
            if (slotId < 0 || slotId > 1) {
                return -1;
            }
            int[] subids = SubscriptionManager.getSubId(slotId);
            int subid = -1;
            if (subids != null && subids.length >= 1) {
                subid = subids[0];
            }
            Log.d("@M_" + TAG, "GetSimIdBySlot: sub id = " + subid 
                    + "sim Slot = " + slotId);
            return subid;
        }

    private void switchGprsDefautSIM(int subid) {

        Log.d("@M_" + TAG, "switchGprsDefautSIM() with simid=" + subid);

        if (subid < 0) {
            finish();
            return;
        }
        int curConSubId = SubscriptionManager.getDefaultDataSubscriptionId();
        Log.d("@M_" + TAG,"curConSimId=" + curConSubId);

        if (subid == curConSubId) {
            finish();
            return;
        }
        showDataConnWaitDialog();
        SubscriptionManager.from(mContext).setDefaultDataSubId(subid);
        TelephonyManager tm = TelephonyManager.from(this);
        tm.setDataEnabled(curConSubId, false);
        tm.setDataEnabled(subid, true);
        //handleSimSwitch(subid);

    }

    private void showDataConnWaitDialog() {

        mTimerHandler.removeMessages(DATA_SWITCH_TIME_OUT_MSG);
        mTimerHandler.sendEmptyMessageDelayed(DATA_SWITCH_TIME_OUT_MSG,
                DATA_SWITCH_TIME_OUT_TIME);

        mWaitDlg = new ProgressDialog(this);
        mWaitDlg.setMessage(mContext.getString(R.string.change_data_conn_progress_message));
        mWaitDlg.setIndeterminate(true);
        mWaitDlg.setCancelable(false);
        mWaitDlg.show();

        mIsDataSwitchWaiting = true;
    }

    private int getDrawableColorValue(Bitmap bitmap) {
        Log.i("@M_" + TAG, "getDrawableColorValue");
        int width = bitmap.getWidth();
        int height = bitmap.getHeight();
        int color = bitmap.getPixel(width / 2, height / 2);
        return color;
    }

    private void setStatusBarEnableStatus(boolean enabled) {
        Log.i("@M_" + TAG, "setStatusBarEnableStatus(" + enabled + ")");
        StatusBarManager statusMgr = (StatusBarManager)getSystemService(Context.STATUS_BAR_SERVICE);
        if (statusMgr != null) {
            if (enabled) {
                statusMgr.disable(StatusBarManager.DISABLE_NONE);
            } else {
                statusMgr.disable(StatusBarManager.DISABLE_EXPAND |
                        StatusBarManager.DISABLE_RECENT |
                        StatusBarManager.DISABLE_HOME);
            }
        } else {
            Log.e("@M_" + TAG, "Fail to get status bar instance");
        }
    }

    
    private void dealDataConnectionChanged(int subId) {
        Log.i("@M_" + TAG, "dealDataConnectionChanged: mToCloseSlot is " + mToCloseSlot);
        if (mToCloseSlot >= 0) {
                
            int toCloseSubId = getSubIdBySlot(mToCloseSlot);
            Log.i("@M_" + TAG, "dealDataConnectionChanged: toCloseSimId is " + toCloseSubId);
        
            if (toCloseSubId != subId) {
                Log.d("@M_" + TAG, "dealWithDataConnChanged dismiss progressDialog");
                mTimerHandler.removeMessages(DATA_SWITCH_TIME_OUT_MSG);
                if (mWaitDlg != null && mWaitDlg.isShowing()) {
                    mWaitDlg.cancel();
                }
                finish();
            } 
        }
    }

}
