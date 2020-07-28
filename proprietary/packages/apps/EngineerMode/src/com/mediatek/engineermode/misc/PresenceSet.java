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

package com.mediatek.engineermode.misc;

import android.app.Activity;
import android.content.ContentValues;
import android.content.Context;
import android.content.Intent;
import android.database.Cursor;
import android.net.Uri;
import android.os.Bundle;
import android.os.SystemProperties;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.EditText;
import android.widget.Toast;

import com.android.ims.ImsConfig;
import com.android.ims.ImsManager;
import com.android.ims.ImsException;
import com.android.internal.telephony.Phone;
import com.mediatek.engineermode.Elog;
import com.mediatek.engineermode.R;
import com.mediatek.ims.internal.MtkImsConfig;
import java.util.HashMap;

//import com.mediatek.presence.provider.settings.RcsSettingsData;

public class PresenceSet extends Activity implements OnClickListener {
    public static final String ACTION_PRESENCE_RESET_ETAG = "android.intent.presence.RESET_ETAG";
    public static final String ACTION_PRESENCE_489_STATE =
            "android.intent.presence.RESET_489_STATE";
    private final String PRESENCE_PACKAGE_NAME = "com.mediatek.presence";
    public static final Uri PRESENCE_SLOT0_SETTINGS_URI = Uri
            .parse("content://com.mediatek.presence.settings/slot0settings");
    public static final Uri PRESENCE_SLOT1_SETTINGS_URI = Uri
            .parse("content://com.mediatek.presence.settings/slot1settings");
    private static String TAG = "[SLOT0] PresenceSet";
    private static HashMap<String, String> mPresenceSetDataMap = null;
    private int mSlotId = 0;

    private EditText mEdSlotId;
    private EditText mEd489TimerValue;
    private EditText mEdCapabilityPollingPeriod;
    private EditText mEdCapabilityExpiryTime;
    private EditText mEdPublishExpiryTime;
    private EditText mEdMaxSubscriptionList;
    private EditText mEnablePresence;
    private Button mBtResetETAG;
    private Button mBtReset489State;
    private Button mBt489TimerValue;
    private Button mBtCapabilityPollingPeriod;
    private Button mBtCapabilityExpiryTime;
    private Button mBtPublishExpiryTime;
    private Button mBtMaxSubscriptionList;
    private Button mBtEnablePresence;
    private Button mBtSlotId;

    //EAB
    private EditText mEdEabPublishTimer;
    private EditText mEdEabPublishTimerExtended;
    private EditText mEdEabCapCacheExpiry;
    private EditText mEdEabAvalCacheExpiry;
    private EditText mEdEabSrcThrotPub;
    private EditText mEdEabCapPollIntv;
    private EditText mEdEabMaxNoEntries;
    private EditText mEdEabCapPollExpiry;
    private EditText mEdEabCapDiscovery;
    private EditText mEdEabGzipEnabled;
    private EditText mEdEabPubErrRetryTimer;

    private Button mBtEabPublishTimer;
    private Button mBtEabPublishTimerExtended;
    private Button mBtEabCapCacheExpiry;
    private Button mBtEabAvalCacheExpiry;
    private Button mBtEabSrcThrotPub;
    private Button mBtEabCapPollIntv;
    private Button mBtEabMaxNoEntries;
    private Button mBtEabCapPollExpiry;
    private Button mBtEabCapDiscovery;
    private Button mBtEabGzipEnabled;
    private Button mBtEabPubErrRetryTimer;

    private String mPort = null;
    private String mRWType = null;
    private String mUsid = null;
    private String mAddress = null;
    private String mData = null;
    private String mMipiMode = null;
    private Toast mToast = null;

    private ImsConfig mImsConfig;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.presence_set);

        ImsManager imsManager = ImsManager.getInstance(this, 0);
        if (imsManager != null) {
            try {
                mImsConfig = imsManager.getConfigInterface();
            } catch (ImsException ex) {
            }
        }

        mEd489TimerValue = (EditText) findViewById(R.id.presence_set_slot_id);
        mEd489TimerValue = (EditText) findViewById(R.id.presence_set_489_timer_value);
        mEdCapabilityPollingPeriod = (EditText)
                findViewById(R.id.presence_set_capability_polling_period);
        mEdCapabilityExpiryTime = (EditText)
                findViewById(R.id.presence_set_capability_expiry_time);
        mEdPublishExpiryTime = (EditText) findViewById(R.id.presence_set_publish_expiry_time);
        mEdMaxSubscriptionList = (EditText) findViewById(R.id.presence_set_max_subscription_list);
        mEnablePresence = (EditText) findViewById(R.id.presence_enable);
        mEdEabPublishTimer = (EditText) findViewById(R.id.presence_set_eab_publish_timer_value);
        mEdEabPublishTimerExtended = (EditText) findViewById(
                                    R.id.presence_set_eab_publish_timer_extended_value);
        mEdEabCapCacheExpiry = (EditText) findViewById(
                                    R.id.presence_set_eab_capability_cache_exp_value);
        mEdEabAvalCacheExpiry = (EditText) findViewById(
                                    R.id.presence_set_eab_availability_cache_exp_value);
        mEdEabSrcThrotPub = (EditText) findViewById(
                                    R.id.presence_set_eab_source_throttle_publish_value);
        mEdEabCapPollIntv = (EditText) findViewById(
                                    R.id.presence_set_eab_capability_poll_interval_value);
        mEdEabMaxNoEntries = (EditText) findViewById(
                                    R.id.presence_set_eab_max_no_entries_value);
        mEdEabCapPollExpiry = (EditText) findViewById(
                                    R.id.presence_set_eab_cap_poll_exp_value);
        mEdEabCapDiscovery = (EditText) findViewById(
                                    R.id.presence_set_eab_capability_discovery_value);
        mEdEabGzipEnabled = (EditText) findViewById(
                                    R.id.presence_set_eab_gzip_enabled_value);
        mEdEabPubErrRetryTimer = (EditText) findViewById(
                                    R.id.presence_set_eab_publish_error_retry_timer_value);
        mEdSlotId = (EditText) findViewById(
                                    R.id.presence_set_slot_id);

        mBt489TimerValue = (Button) findViewById(R.id.presence_set_489_timer_value_button);
        mBtCapabilityPollingPeriod = (Button)
                findViewById(R.id.presence_set_capability_polling_period_button);
        mBtCapabilityExpiryTime = (Button)
                findViewById(R.id.presence_set_capability_expiry_time_button);
        mBtPublishExpiryTime = (Button) findViewById(R.id.presence_set_publish_expiry_time_button);
        mBtMaxSubscriptionList = (Button)
                findViewById(R.id.presence_set_max_subscription_list_button);

        mBtEnablePresence = (Button) findViewById(R.id.presence_enable_button);
        mBtResetETAG = (Button) findViewById(R.id.presence_set_reset_ETAG);
        mBtReset489State = (Button) findViewById(R.id.presence_set_reset_489_state);
        mBtSlotId = (Button) findViewById(R.id.presence_set_slot_id_button);

        mBtEabPublishTimer =
            (Button) findViewById(R.id.presence_set_eab_publish_timer_button);
        mBtEabPublishTimerExtended =
            (Button) findViewById(R.id.presence_set_eab_publish_timer_extended_button);
        mBtEabCapCacheExpiry =
            (Button) findViewById(R.id.presence_set_eab_capability_cache_exp_button);
        mBtEabAvalCacheExpiry =
            (Button) findViewById(R.id.presence_set_eab_availability_cache_exp_button);
        mBtEabSrcThrotPub =
            (Button) findViewById(R.id.presence_set_eab_source_throttle_publish_button);
        mBtEabCapPollIntv =
            (Button) findViewById(R.id.presence_set_eab_capability_poll_interval_button);
        mBtEabMaxNoEntries =
            (Button) findViewById(R.id.presence_set_eab_max_no_entries_button);
        mBtEabCapPollExpiry =
            (Button) findViewById(R.id.presence_set_eab_cap_poll_exp_button);
        mBtEabCapDiscovery =
            (Button) findViewById(R.id.presence_set_eab_capability_discovery_button);
        mBtEabGzipEnabled =
            (Button) findViewById(R.id.presence_set_eab_gzip_enabled_button);
        mBtEabPubErrRetryTimer =
            (Button) findViewById(R.id.presence_set_eab_publish_error_retry_timer_button);

        mBt489TimerValue.setOnClickListener(this);
        mBtCapabilityPollingPeriod.setOnClickListener(this);
        mBtCapabilityExpiryTime.setOnClickListener(this);
        mBtPublishExpiryTime.setOnClickListener(this);
        mBtMaxSubscriptionList.setOnClickListener(this);
        mBtEnablePresence.setOnClickListener(this);
        mBtResetETAG.setOnClickListener(this);
        mBtReset489State.setOnClickListener(this);
        mBtEabPublishTimer.setOnClickListener(this);
        mBtEabPublishTimerExtended.setOnClickListener(this);
        mBtEabCapCacheExpiry.setOnClickListener(this);
        mBtEabAvalCacheExpiry.setOnClickListener(this);
        mBtEabSrcThrotPub.setOnClickListener(this);
        mBtEabCapPollIntv.setOnClickListener(this);
        mBtEabMaxNoEntries.setOnClickListener(this);
        mBtEabCapPollExpiry.setOnClickListener(this);
        mBtEabCapDiscovery.setOnClickListener(this);
        mBtEabGzipEnabled.setOnClickListener(this);
        mBtEabPubErrRetryTimer.setOnClickListener(this);
        mBtSlotId.setOnClickListener(this);

        if (mPresenceSetDataMap == null) {
            mPresenceSetDataMap = new HashMap<String, String>();
            mPresenceSetDataMap.put("SipBadEventExpiredTime", "0");
            mPresenceSetDataMap.put("CapabilityPollingPeriod", "0");
            mPresenceSetDataMap.put("CapabilityExpiryTimeout", "0");
            mPresenceSetDataMap.put("PublishExpirePeriod", "0");
            mPresenceSetDataMap.put("MaxSubscriptionPresenceList", "0");
        }
        queryPresenceDB();

        mEdEabPublishTimer.setText(
            String.valueOf(queryEabConfiguration(ImsConfig.ConfigConstants.PUBLISH_TIMER)));
        mEdEabPublishTimerExtended.setText(
            String.valueOf(queryEabConfiguration(
            ImsConfig.ConfigConstants.PUBLISH_TIMER_EXTENDED)));
        mEdEabCapCacheExpiry.setText(
            String.valueOf(queryEabConfiguration(
            ImsConfig.ConfigConstants.CAPABILITIES_CACHE_EXPIRATION)));
        mEdEabAvalCacheExpiry.setText(
            String.valueOf(queryEabConfiguration(
            ImsConfig.ConfigConstants.AVAILABILITY_CACHE_EXPIRATION)));
        mEdEabSrcThrotPub.setText(
            String.valueOf(queryEabConfiguration(
            ImsConfig.ConfigConstants.SOURCE_THROTTLE_PUBLISH)));
        mEdEabCapPollIntv.setText(
            String.valueOf(queryEabConfiguration(
            ImsConfig.ConfigConstants.CAPABILITIES_POLL_INTERVAL)));
        mEdEabMaxNoEntries.setText(
            String.valueOf(queryEabConfiguration(ImsConfig.ConfigConstants.MAX_NUMENTRIES_IN_RCL)));
        mEdEabCapPollExpiry.setText(
            String.valueOf(queryEabConfiguration(
            ImsConfig.ConfigConstants.CAPAB_POLL_LIST_SUB_EXP)));
        mEdEabCapDiscovery.setText(
            String.valueOf(queryEabConfiguration(
            ImsConfig.ConfigConstants.CAPABILITY_DISCOVERY_ENABLED)));
        mEdEabGzipEnabled.setText(
            String.valueOf(queryEabConfiguration(ImsConfig.ConfigConstants.GZIP_FLAG)));
        mEdEabPubErrRetryTimer.setText(
            String.valueOf(queryEabConfiguration(
            MtkImsConfig.ConfigConstants.PUBLISH_ERROR_RETRY_TIMER)));

        mEdSlotId.setText("0");
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
    }

    private void queryPresenceDB() {
        Cursor cursor = null;
        if (mSlotId == 0) {
            cursor = this.getContentResolver().query(
                    PRESENCE_SLOT0_SETTINGS_URI, null, null, null, null);
        } else if (mSlotId == 1) {
            cursor = this.getContentResolver().query(
                    PRESENCE_SLOT1_SETTINGS_URI, null, null, null, null);
        }
        if (cursor != null) {
            cursor.moveToFirst();
            // get uri
            if (cursor.getCount() > 0) {
                do {
                    String key = cursor.getString(cursor.getColumnIndex("key"));
                    String value = cursor.getString(cursor.getColumnIndex("value"));

                    if (mPresenceSetDataMap.containsKey(key)) {
                        Elog.d(TAG, key + " = " + value);
                        mPresenceSetDataMap.put(key, value);
                    }
                } while (cursor.moveToNext());

            }
            cursor.close();
        } else
            Elog.d(TAG, "cursor is null!");

        mEdCapabilityPollingPeriod.setText(mPresenceSetDataMap.get("CapabilityPollingPeriod"));
        mEdCapabilityExpiryTime.setText(mPresenceSetDataMap.get("CapabilityExpiryTimeout"));
        mEdPublishExpiryTime.setText(mPresenceSetDataMap.get("PublishExpirePeriod"));
        mEdMaxSubscriptionList.setText(mPresenceSetDataMap.get("MaxSubscriptionPresenceList"));
        mEnablePresence.setText(SystemProperties.get("persist.vendor.mtk_uce_support"));
        mEd489TimerValue.setText(mPresenceSetDataMap.get("SipBadEventExpiredTime"));
    }

    private void setPresenceDB(int slotId, String settins, String value) {
        String where = "key = \"" + settins + "\"";
        if (!value.equals("")) {
            ContentValues values = new ContentValues();
            values.put("value", value);
            Elog.d(TAG, "where = " + where);
            if (slotId == 0) {
                this.getContentResolver().update(PRESENCE_SLOT0_SETTINGS_URI, values, where, null);
                showToast("set " + settins + " to " + value + " succeed");
                return;
            } else if (slotId == 1) {
                this.getContentResolver().update(PRESENCE_SLOT1_SETTINGS_URI, values, where, null);
                showToast("set " + settins + " to " + value + " succeed");
                return;
            }
        }
        showToast("The value input is error!");
    }

    private int checkValue(String value) {
        int msg = 0;
        try {
            msg = Integer.valueOf(value);
        } catch (Exception e) {
            Elog.d(TAG, "The value input is error!");
            showToast("The value input is error!");
        }
        return msg;
    }

    private void updateSlotId() {
        mSlotId = checkValue(mEdSlotId.getText().toString());
        TAG = "[SLOT" + mSlotId + "] PresenceSet";
        queryPresenceDB();
        ImsManager imsManager = ImsManager.getInstance(this, mSlotId);
        if (imsManager != null) {
            try {
                mImsConfig = imsManager.getConfigInterface();
            } catch (ImsException ex) {
            }
        }

        showToast("Slot Id is updated to : " + mSlotId);
    }

    public void onClick(View arg0) {
        switch (arg0.getId()) {
            case R.id.presence_set_slot_id_button:
                updateSlotId();
                break;
            case R.id.presence_set_reset_ETAG:
                SendBroadcast(this, ACTION_PRESENCE_RESET_ETAG, -1);
                Elog.d(TAG, "sendBroadcast = " + ACTION_PRESENCE_RESET_ETAG);
                showToast("sendBroadcast = " + ACTION_PRESENCE_RESET_ETAG + " succeed");
                break;
            case R.id.presence_set_reset_489_state:
                SendBroadcast(this, ACTION_PRESENCE_489_STATE, -1);
                Elog.d(TAG, "sendBroadcast = " + ACTION_PRESENCE_489_STATE);
                showToast("sendBroadcast = " + ACTION_PRESENCE_489_STATE + " succeed");
                break;
            case R.id.presence_set_489_timer_value_button:
                int msg = 0;
                msg = checkValue(mEd489TimerValue.getText().toString());
                SendBroadcast(this, ACTION_PRESENCE_489_STATE, msg);
                setPresenceDB(mSlotId, "SipBadEventExpiredTime", "" + msg);
                Elog.d(TAG, "sendBroadcast = " + ACTION_PRESENCE_489_STATE + ",489ExpiredTime = "
                        + msg);
                showToast("sendBroadcast = " + ACTION_PRESENCE_489_STATE + " , " + msg + " " +
                        "succeed");
                break;
            case R.id.presence_set_capability_polling_period_button:
                setPresenceDB(mSlotId, "CapabilityPollingPeriod",
                        mEdCapabilityPollingPeriod.getText().toString());
                break;
            case R.id.presence_set_capability_expiry_time_button:
                setPresenceDB(mSlotId, "CapabilityExpiryTimeout", mEdCapabilityExpiryTime.getText()
                        .toString());
                break;
            case R.id.presence_set_publish_expiry_time_button:
                setPresenceDB(mSlotId, "PublishExpirePeriod",
                        mEdPublishExpiryTime.getText().toString());
                break;
            case R.id.presence_set_max_subscription_list_button:
                setPresenceDB(mSlotId, "MaxSubscriptionPresenceList",
                        mEdMaxSubscriptionList.getText().toString());
                break;
            case R.id.presence_enable_button:
                if ("1".equals(mEnablePresence.getText().toString())) {
                    SystemProperties.set("persist.vendor.mtk_uce_support", "1");
                    showToast("persist.vendor.mtk_uce_support = 1");
                } else {
                    SystemProperties.set("persist.vendor.mtk_uce_support", "0");
                    showToast("persist.vendor.mtk_uce_support = 0");
                }
                break;
            case R.id.presence_set_eab_publish_timer_button:
                setEabConfiguration(ImsConfig.ConfigConstants.PUBLISH_TIMER,
                                        Integer.parseInt(mEdEabPublishTimer.getText().toString()));
                break;
            case R.id.presence_set_eab_publish_timer_extended_button:
                setEabConfiguration(ImsConfig.ConfigConstants.PUBLISH_TIMER_EXTENDED,
                                        Integer.parseInt(
                                        mEdEabPublishTimerExtended.getText().toString()));
                break;
            case R.id.presence_set_eab_capability_cache_exp_button:
                setEabConfiguration(ImsConfig.ConfigConstants.CAPABILITIES_CACHE_EXPIRATION,
                                        Integer.parseInt(
                                        mEdEabCapCacheExpiry.getText().toString()));
                break;
            case R.id.presence_set_eab_availability_cache_exp_button:
                setEabConfiguration(ImsConfig.ConfigConstants.AVAILABILITY_CACHE_EXPIRATION,
                                        Integer.parseInt(
                                        mEdEabAvalCacheExpiry.getText().toString()));
                break;
            case R.id.presence_set_eab_source_throttle_publish_button:
                setEabConfiguration(ImsConfig.ConfigConstants.SOURCE_THROTTLE_PUBLISH,
                                        Integer.parseInt(mEdEabSrcThrotPub.getText().toString()));
                break;
             case R.id.presence_set_eab_capability_poll_interval_button:
                 setEabConfiguration(ImsConfig.ConfigConstants.CAPABILITIES_POLL_INTERVAL,
                                        Integer.parseInt(mEdEabCapPollIntv.getText().toString()));
                break;
            case R.id.presence_set_eab_max_no_entries_button:
                setEabConfiguration(ImsConfig.ConfigConstants.MAX_NUMENTRIES_IN_RCL,
                                        Integer.parseInt(mEdEabMaxNoEntries.getText().toString()));
                break;
            case R.id.presence_set_eab_cap_poll_exp_button:
                setEabConfiguration(ImsConfig.ConfigConstants.CAPAB_POLL_LIST_SUB_EXP,
                                        Integer.parseInt(mEdEabCapPollExpiry.getText().toString()));
                break;
            case R.id.presence_set_eab_capability_discovery_button:
                setEabConfiguration(ImsConfig.ConfigConstants.CAPABILITY_DISCOVERY_ENABLED,
                                        Integer.parseInt(mEdEabCapDiscovery.getText().toString()));
                break;
            case R.id.presence_set_eab_gzip_enabled_button:
                setEabConfiguration(ImsConfig.ConfigConstants.GZIP_FLAG,
                                        Integer.parseInt(mEdEabGzipEnabled.getText().toString()));
                break;
            case R.id.presence_set_eab_publish_error_retry_timer_button:
                setEabConfiguration(MtkImsConfig.ConfigConstants.PUBLISH_ERROR_RETRY_TIMER,
                                        Integer.parseInt(
                                        mEdEabPubErrRetryTimer.getText().toString()));
                break;
        }
    }

    private int queryEabConfiguration(int key) {
        int result = -1;
        try {
            result = mImsConfig.getProvisionedValue(key);
            Elog.d(TAG, "getProvisionedValue, result="+result);
        } catch (ImsException ex) {
            Elog.d(TAG, "setEabConfiguration exception = " + ex);
        }
        return result;
    }

    private void setEabConfiguration(int key, int value) {
        try {
            mImsConfig.setProvisionedValue(key, value);
            showToast("config:"+key+", value = "+value);
        } catch (ImsException ex) {
            Elog.d(TAG, "setEabConfiguration exception = " + ex);
        }
    }

    private void SendBroadcast(Context mcontext, String intentID, int msg) {
        Intent intent = new Intent();
        intent.setAction(intentID);
        intent.setPackage(PRESENCE_PACKAGE_NAME);
        intent.putExtra("slotId", mSlotId);
        if (msg != -1) {
            intent.putExtra("489ExpiredTime", msg);
            Elog.d(TAG, "sendBroadcast result = " + msg);
        }
        if (mcontext != null) {
            mcontext.sendBroadcast(intent);
        }
    }

    private void showToast(String msg) {
        mToast = Toast.makeText(this, "[SLOT" + mSlotId + "] " + msg, Toast.LENGTH_SHORT);
        mToast.show();
    }

}
