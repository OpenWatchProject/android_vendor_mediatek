/*
 * Copyright (C) 2011 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.mediatek.cellbroadcastreceiver;

import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.SharedPreferences;
import android.os.Handler;
import android.os.Message;
import android.preference.CheckBoxPreference;
import android.telephony.SmsManager;
import android.telephony.SubscriptionManager;
import android.text.TextUtils;
import android.util.AttributeSet;
import android.util.Log;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.CheckBox;
import android.widget.ImageView;
import android.widget.TextView;

import mediatek.telephony.MtkSmsManager;

public class CheckBoxAndSettingsPreference extends CheckBoxPreference {
    private final String TAG = "[CMAS]CheckBoxAndSettingsPreference";

    public static final String KEY_ENABLE_CMAS_PRESIDENTIAL_ALERTS = "enable_cmas_presidential_threat_alerts";
    public static final String KEY_ENABLE_CMAS_IMMINENT_ALERTS = "enable_cmas_imminent_threat_alerts";
    public static final String KEY_ENABLE_CMAS_AMBER_ALERTS = "enable_cmas_amber_threat_alerts";
    public static final String KEY_CMAS_PREVIEW_ALERTS_TONE = "cmas_preview_alert_tone";
    public static final String KEY_ENABLE_CMAS_EXTREME_ALERTS = "enable_cmas_extreme_threat_alerts";
    public static final String KEY_ENABLE_CMAS_SEVERE_ALERTS = "enable_cmas_severe_threat_alerts";
    public static final String KEY_ENABLE_ALERT_SPEECH = "enable_cmas_speech_threat_alerts";
    public static final String KEY_ENABLE_ALL_ALERT = "enable_cmas_all_alerts";
    public static final String KEY_ENABLE_ALL_IMMINENT_ALERT = "enable_cmas_all_imminent_alerts";
    public static final String KEY_ENABLE_ALERT_MESSAGE = "enable_cmas_alerts_messages";

    ///M: Preference key for whether to enable repeat alert.
    public static final String KEY_ENABLE_REPEAT_ALERT = "enable_cmas_repeat_alert";

    public static final String KEY_ENABLE_CELLBROADCAST = "enable_cell_broadcast";
    private static final float DISABLED_ALPHA = 0.4f;

    private static final int MESSAGE_ENABLE_CB_END = 101;
    public static final int MESSAGE_ENABLE_CB_END_ERROR = 102;

    private CBSettingHandler mHandler = new CBSettingHandler();
    private Context mContext;
    private boolean mEnableCB;
    private String mChannelsInfo;
    private int mSlotId;
    private String mSlotKey = "";

    private TextView mTitleText;
    private TextView mSummaryText;
    private ImageView mSettingsButton;
    private View mLine;
    private CheckBox mCheckBox;
    private Intent mSettingsIntent;
    private OnSettingChangedListener mListener;

    public CheckBoxAndSettingsPreference(Context context, AttributeSet attrs) {
        super(context, attrs);
        setLayoutResource(R.layout.preference_item);
        setWidgetLayoutResource(R.layout.preference_item_checkbox);

        mContext = context;
    }

    @Override
    protected void onBindView(View view) {
        super.onBindView(view);
        View textLayout = view.findViewById(R.id.alert_pref);
        textLayout.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View arg0) {
                String key = getKey();
                if (TextUtils.isEmpty(key)) {
                    return;
                }
                if (key.equals(KEY_ENABLE_CELLBROADCAST)) {
                    Log.d(TAG, "KEY_ENABLE_CELLBROADCAST,isChecked() = " + isChecked());
                    if (isChecked()) {
                        AlertDialog.Builder builder = new AlertDialog.Builder(getContext());
                        builder.setIconAttribute(android.R.attr.alertDialogIcon)
                                .setCancelable(true).setPositiveButton(R.string.button_dismiss,
                                        new DialogInterface.OnClickListener() {
                                            @Override
                                            public void onClick(DialogInterface arg0, int arg1) {
                                                onCheckBoxClicked();
                                            }
                                        }).setNegativeButton(R.string.button_cancel, null)
                                .setMessage(R.string.cmas_disable_cmas).show();
                    } else {
                        onCheckBoxClicked();
                    }
                } else {
                    onCheckBoxClicked();
                }
            }
        });

        mCheckBox = (CheckBox) view.findViewById(android.R.id.checkbox);
        mSettingsButton = (ImageView) view.findViewById(R.id.imminent_btn);
        mTitleText = (TextView) view.findViewById(android.R.id.title);
        mSummaryText = (TextView) view.findViewById(android.R.id.summary);
        mLine = (View) view.findViewById(R.id.line);
        mSettingsButton.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View clickedView) {
                mSettingsIntent = new Intent(getContext(), CellBroadcastSubSettings.class);
                mSettingsIntent.putExtra("slotid", mSlotId);
                getContext().startActivity(mSettingsIntent);
            }
        });
        //Log.d(TAG, "key = " + getKey());

        if (getKey().equals(KEY_ENABLE_CMAS_IMMINENT_ALERTS)) {
            if (mSettingsButton != null) {
                mSettingsButton.setVisibility(View.VISIBLE);
            }
            if (mLine != null) {
                mLine.setVisibility(View.VISIBLE);
            }
        }

        enableSettingsButton();
    }

    protected void onCheckBoxClicked() {
        String key = getKey();
        if (TextUtils.isEmpty(key)) {
            return;
        }
        if (key.equals(KEY_ENABLE_CELLBROADCAST)) {
            SharedPreferences.Editor cbEditor = getContext()
                    .getSharedPreferences(
                            "com.mediatek.cellbroadcastreceiver_preferences",
                            getContext().MODE_PRIVATE).edit();
            if (!isChecked()) {
                mEnableCB = true;
                cbEditor.putBoolean(mSlotKey + KEY_ENABLE_CELLBROADCAST, true);
            } else {
                mEnableCB = false;
                cbEditor.putBoolean(mSlotKey + KEY_ENABLE_CELLBROADCAST, false);
            }
            cbEditor.commit();
            enableCB();
        }
        if (key.equals(KEY_ENABLE_CMAS_IMMINENT_ALERTS) || key.equals(KEY_ENABLE_CMAS_AMBER_ALERTS)
                || key.equals(KEY_ENABLE_CMAS_EXTREME_ALERTS)
                || key.equals(KEY_ENABLE_CMAS_SEVERE_ALERTS)
                || key.equals(KEY_ENABLE_ALL_ALERT)
                || key.equals(KEY_ENABLE_ALL_IMMINENT_ALERT)) {
            AlertDialog.Builder builder = new AlertDialog.Builder(getContext());
            builder.setIconAttribute(android.R.attr.alertDialogIcon).setCancelable(true)
                    .setPositiveButton(R.string.button_dismiss,
                            new DialogInterface.OnClickListener() {
                                @Override
                                public void onClick(DialogInterface arg0, int arg1) {
                                    SharedPreferences.Editor editor = getContext()
                                    .getSharedPreferences(
                                            "com.mediatek.cellbroadcastreceiver_preferences",
                                            getContext().MODE_PRIVATE)
                                    .edit();
                                    if (getKey().equals(KEY_ENABLE_ALL_ALERT)
                                            || getKey().equals(KEY_ENABLE_ALL_IMMINENT_ALERT)) {
                                        editor.putBoolean(mSlotKey + getKey(), !isChecked());
                                        mListener.onEnableAllChanged();
                                    }

                                    if (isChecked()) {
                                        setChecked(false);
                                        if (getKey().equals(KEY_ENABLE_CMAS_IMMINENT_ALERTS)) {
                                            editor.putBoolean(mSlotKey +
                                                KEY_ENABLE_ALL_IMMINENT_ALERT, false);
                                            editor.putBoolean(mSlotKey +
                                                KEY_ENABLE_CMAS_EXTREME_ALERTS, false);
                                            editor.putBoolean(mSlotKey +
                                                KEY_ENABLE_CMAS_SEVERE_ALERTS, false);
                                            editor.putBoolean(mSlotKey +
                                                KEY_ENABLE_CMAS_IMMINENT_ALERTS, false);
                                        } else if (getKey().equals(KEY_ENABLE_CMAS_AMBER_ALERTS)) {
                                            editor.putBoolean(mSlotKey +
                                                KEY_ENABLE_CMAS_AMBER_ALERTS, false);
                                        } else if (getKey().equals(KEY_ENABLE_CMAS_SEVERE_ALERTS)) {
                                            editor.putBoolean(mSlotKey +
                                                KEY_ENABLE_CMAS_SEVERE_ALERTS, false);
                                        }
                                    } else {
                                        setChecked(true);
                                        if (getKey().equals(KEY_ENABLE_CMAS_IMMINENT_ALERTS)) {
                                            editor.putBoolean(mSlotKey +
                                                KEY_ENABLE_CMAS_EXTREME_ALERTS, true);
                                            editor.putBoolean(mSlotKey +
                                                KEY_ENABLE_CMAS_IMMINENT_ALERTS, true);
                                        } else if (getKey().equals(KEY_ENABLE_CMAS_AMBER_ALERTS)) {
                                            editor.putBoolean(mSlotKey +
                                                KEY_ENABLE_CMAS_AMBER_ALERTS, true);
                                        } else if (getKey().equals(KEY_ENABLE_CMAS_SEVERE_ALERTS)) {
                                            editor.putBoolean(mSlotKey +
                                                KEY_ENABLE_CMAS_SEVERE_ALERTS, true);
                                        } else if (getKey().equals(KEY_ENABLE_ALL_ALERT)) {
                                            editor.putBoolean(mSlotKey +
                                                KEY_ENABLE_CMAS_EXTREME_ALERTS, true);
                                            editor.putBoolean(mSlotKey +
                                                KEY_ENABLE_CMAS_SEVERE_ALERTS, true);
                                            editor.putBoolean(mSlotKey +
                                                KEY_ENABLE_CMAS_IMMINENT_ALERTS, true);
                                            editor.putBoolean(mSlotKey +
                                                KEY_ENABLE_CMAS_AMBER_ALERTS, true);
                                        } else if (getKey().equals(KEY_ENABLE_ALL_IMMINENT_ALERT)) {
                                            editor.putBoolean(mSlotKey +
                                                KEY_ENABLE_CMAS_SEVERE_ALERTS, true);
                                        }
                                    }
                                    editor.commit();
                                    if (CmasConfigManager.getGeminiSupport()) {
                                        Intent serviceIntent = new Intent(
                                            CellBroadcastConfigService.ACTION_ENABLE_CHANNELS,
                                            null, getContext(), CellBroadcastConfigService.class);
                                        serviceIntent.putExtra("SLOTKEY", mSlotId);
                                        getContext().startService(serviceIntent);
                                    } else {
                                        CellBroadcastReceiver.startConfigService(getContext());
                                    }
                                }
                            }).setNegativeButton(R.string.button_cancel, null).setMessage(
                            R.string.cmas_change_settings).show();
        } else {
            if (isChecked()) {
                setChecked(false);
            } else {
                setChecked(true);
            }
        }
    }

    @Override
    public void setEnabled(boolean enabled) {
        super.setEnabled(enabled);
        enableSettingsButton();
    }

    private void enableSettingsButton() {
        //Log.e(TAG, "enableSettingsButton " + getKey());
        if (getKey().equals(KEY_ENABLE_CMAS_IMMINENT_ALERTS)) {
            //Log.e(TAG, "KEY_ENABLE_CMAS_IMMINENT_ALERTS");
            if (mSettingsButton != null) {
                final boolean enable = isEnabled();
                mSettingsButton.setEnabled(enable);
                mSettingsButton.setClickable(enable);
                mSettingsButton.setFocusable(enable);
                if (!enable) {
                    mSettingsButton.setAlpha(DISABLED_ALPHA);
                }
            }
        }
    }

    private void enableCB() {
        Log.d(TAG, "enableCB inThread");
        mHandler.mCheckBox = this;
        setEnabled(false);
        CBEnableThread enableThread = new CBEnableThread();
        enableThread.start();
    }

    private class CBEnableThread extends Thread {
        Message mMsg;

        @Override
        public void run() {
            final int[] subIds = SubscriptionManager.getSubId(mSlotId);
            MtkSmsManager mtkSmsManager = MtkSmsManager.getDefault();
            if (subIds != null && subIds.length > 0) {
                Log.d(TAG, "invalid subIds");
                int subId = subIds[0];
                mtkSmsManager = MtkSmsManager.getSmsManagerForSubscriptionId(subId);
            }

            Log.d(TAG, "CBEnableThread run enableCB " + mEnableCB);

            if (mtkSmsManager.activateCellBroadcastSms(mEnableCB) ||
                    (CmasConfigManager.isTwProfile())) {
                mMsg = mHandler.obtainMessage(MESSAGE_ENABLE_CB_END, 0,
                                 MESSAGE_ENABLE_CB_END, null);

                if (mEnableCB || (CmasConfigManager.isTwProfile())) {
                    CellBroadcastReceiver.sEnableCbMsg = mMsg;
                    CellBroadcastReceiver.startConfigService(mContext);
                } else {
                    Log.d(TAG, mEnableCB + " mMsg.sendToTarget() ");
                    mMsg.sendToTarget();
                }
                Log.d(TAG, "activateCellBroadcastSms success! ");
            } else {
                mMsg = mHandler.obtainMessage(MESSAGE_ENABLE_CB_END_ERROR, 0,
                           MESSAGE_ENABLE_CB_END_ERROR, null);
                mMsg.sendToTarget();
                Log.d(TAG, "activateCellBroadcastSms error! ");
            }

            Log.d(TAG, "CBEnableThread exit ");

        }

    }

    public interface OnSettingChangedListener {
        void onEnableCBChanged();
        void onEnableAllChanged();
    }

    public void setOnSettingChangedListener(OnSettingChangedListener listener) {
        mListener = listener;
    }

    public void setSlotId(int slotId) {
        mSlotId = slotId;
        mSlotKey = CMASUtils.getSlotKey(mSlotId);
    }

    private class CBSettingHandler extends Handler {
        CheckBoxAndSettingsPreference mCheckBox;

        @Override
        public void handleMessage(Message msg) {
            switch (msg.what) {
            case MESSAGE_ENABLE_CB_END_ERROR:
                Log.d(TAG, "handleMessage MESSAGE_ENABLE_CB_END_ERROR ");
                // TODO: add a toast message to user for error.
                break;
            case MESSAGE_ENABLE_CB_END:
                Log.d(TAG, "handleMessage MESSAGE_ENABLE_CB_END ");
                break;
            default:
                break;
            }
            mCheckBox.setEnabled(true);
            if (mListener != null) {
                mListener.onEnableCBChanged();
            }

        }
    }
}
