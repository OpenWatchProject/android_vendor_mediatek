/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2012. All rights reserved.
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

package com.mediatek.mms.plugin;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.Context;
import android.content.ContextWrapper;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.SharedPreferences;
import android.database.Cursor;
import android.graphics.drawable.Drawable;
import android.os.Bundle;
import android.preference.CheckBoxPreference;
import android.preference.Preference;
import android.preference.PreferenceCategory;
import android.preference.PreferenceManager;
import android.provider.Telephony.Mms;
import android.telephony.SmsManager;
import android.telephony.SubscriptionInfo;
import android.telephony.SubscriptionManager;
import android.util.Log;
import android.view.View;
import android.widget.CheckBox;

import com.mediatek.mms.plugin.R;
//import com.mediatek.telephony.SimInfoManager;
import java.util.List;

/**
 * Op01MmsPreference.
 *
 */
public class Op01MmsPreference extends ContextWrapper {

    /**
     * Construction.
     * @param base Context
     */
    public Op01MmsPreference(Context base) {
        super(base);
    }

    private static final String TAG = "Mms/Op01MmsPreferenceExt";
    private static final String PREFERENCE_NAME = "preference";
    private static final String PREFERENCE_NAME_SMS_VALIDITY_PERIOD = "sms_validity_period";
    public static final String SMS_VALIDITY_PERIOD_PREFERENCE_KEY = "pref_key_sms_validity_period";
    private static final String MMS_ENABLE_TO_SEND_DELIVERY_REPORT_KEY =
                                            "pref_key_mms_enable_to_send_delivery_reports";
    public static final String SMS_FORWARD_WITH_SENDER = "pref_key_forward_with_sender";
    /**
     * configSmsPreference.
     * @param hostActivity Activity
     * @param pC PreferenceCategory
     * @param simCount int
     */
    /* q0 migration, phase out sms validate*/
    /*
    public void configSmsPreference(Activity hostActivity, PreferenceCategory pC, int simCount) {
          Log.d(TAG, "configSmsPreference");
        addSmsValidityPeriodPreference(hostActivity, pC, simCount);
        addForwardWithSenderPreference(hostActivity, pC);
    }
*/
    /**
     * configSmsPreferenceEditorWhenRestore.
     * @param hostActivity Activity
     * @param editor SharedPreferences.Editor
     */
    /* q0 migration, phase out sms validate*/
    /*
    public void configSmsPreferenceEditorWhenRestore(Activity hostActivity,
                                                        SharedPreferences.Editor editor) {
        Log.d(TAG, "configSmsPreferenceEditorWhenRestore");
        if (Op01MmsUtils.isMtkGemini()) {
            List<SubscriptionInfo> simList = SubscriptionManager.from(hostActivity)
                                                    .getActiveSubscriptionInfoList();
            if (simList != null) {
                int simCount = simList.size();
                for (int index = 0; index < simCount; index ++) {
                    int subId = simList.get(index).getSubscriptionId();
                    String key = getSmsValidityKeyBySubId(subId);
                    editor.remove(key);
                }
            }
        }

        Log.d(TAG, "set SMS_FORWARD_WITH_SENDER true");
        editor.putBoolean(SMS_FORWARD_WITH_SENDER, true);
    }
*/
    /**
     * configMmsPreferenceEditorWhenRestore.
     * @param hostActivity Activity
     * @param editor SharedPreferences
     */
    public void configMmsPreferenceEditorWhenRestore(Activity hostActivity,
                                                        SharedPreferences.Editor editor) {
        Log.d(TAG, "configMmsPreferenceEditorWhenRestore");
        if (Op01MmsUtils.isMtkGemini()) {
            List<SubscriptionInfo> simList =
                    SubscriptionManager.from(hostActivity).getActiveSubscriptionInfoList();
            if (simList != null) {
                int simCount = simList.size();
                for (int index = 0; index < simCount; index ++) {
                    Log.d(TAG, "set simId_MMS_ENABLE_TO_SEND_DELIVERY_REPORT_KEY false");
                    int simId = simList.get(index).getSubscriptionId();
                    editor.putBoolean(Integer.toString(simId) + "_" +
                                        MMS_ENABLE_TO_SEND_DELIVERY_REPORT_KEY, false);
                }
            }
        } else {
            Log.d(TAG, "set MMS_ENABLE_TO_SEND_DELIVERY_REPORT_KEY false");
            editor.putBoolean(MMS_ENABLE_TO_SEND_DELIVERY_REPORT_KEY, false);
        }
    }

    // add sms validity period feature
    /* q0 migration, phase out sms validate*/
    /*
    private void addSmsValidityPeriodPreference(final Activity hostActivity, PreferenceCategory pC,
            int simCount) {
        Log.d(TAG, "addSmsValidityPeriodPreference: simCount = " + simCount);
        if (Op01MmsUtils.isMtkGemini()) {
            final List<SubscriptionInfo> simList = SubscriptionManager.from(hostActivity)
                                    .getActiveSubscriptionInfoList();

            Preference p = new Preference(hostActivity);
            p.setTitle(getString(R.string.sms_validity_period));
            p.setSummary(getString(R.string.sms_validity_period));
            int smsValidityOrder = 4;
            p.setOrder(smsValidityOrder);
            int count = pC.getPreferenceCount();
            //reset order
            for (int index = 0; index < count; index ++) {
                Preference preference = pC.getPreference(index);
                if (preference != null) {
                    int pOrder = preference.getOrder();
                    if (pOrder >= smsValidityOrder) {
                        preference.setOrder(pOrder + 1);
                    }
                }
            }
            if (simCount > 1) {
                p.setOnPreferenceClickListener(new Preference.OnPreferenceClickListener() {
                    @Override
                    public boolean onPreferenceClick(Preference preference) {
                        Log.d(TAG, "addSmsValidityPeriodPreference:" +
                                "onPreferenceClick, preference = " + preference);
                        Intent intent = new Intent();
                        intent.setClassName(hostActivity, "com.mediatek.setting.SubSelectActivity");
                        intent.putExtra("PREFERENCE_KEY", SMS_VALIDITY_PERIOD_PREFERENCE_KEY);
                        intent.putExtra(PREFERENCE_NAME, PREFERENCE_NAME_SMS_VALIDITY_PERIOD);
                        hostActivity.startActivity(intent);
                        return true;
                    }
                });
            } else {
                if (simCount == 0) {
                    p.setEnabled(false);
                } else {
                    p.setOnPreferenceClickListener(new Preference.OnPreferenceClickListener() {
                        @Override
                        public boolean onPreferenceClick(Preference preference) {
                            Log.d(TAG, "addSmsValidityPeriodPreference: " +
                                    "onPreferenceClick, preference = " + preference);
                            int subId = simList.get(0).getSubscriptionId();
                            showSmsValidityPeriodDialog(hostActivity, subId);
                            return true;
                        }
                    });
                }
            }
            pC.addPreference(p);
        }
    }
*/
    /**
     * configSelectCardPreferenceTitle.
     * @param hostActivity Activity
     */
    public void configSelectCardPreferenceTitle(Activity hostActivity) {
        Intent intent = hostActivity.getIntent();
        String preferenceName = intent.getStringExtra(PREFERENCE_NAME);
        /* q0 migration, phase out sms validate*/
        /*if (preferenceName != null && preferenceName.equals(PREFERENCE_NAME_SMS_VALIDITY_PERIOD)) {
            hostActivity.setTitle(getString(R.string.sms_validity_period));
        } else */if (preferenceName != null &&
                            preferenceName.equals(MMS_ENABLE_TO_SEND_DELIVERY_REPORT_KEY)) {
            hostActivity.setTitle(
                        getString(R.string.pref_title_mms_enable_to_send_delivery_reports));
        }
    }

    /**
     * configSelectCardPreference.
     * @param hostActivity Activity
     * @param pC PreferenceCategory
     * @param intent Intent
     */
    /* q0 migration, phase out sms validate*/
    /*
    public void configSelectCardPreference(Activity hostActivity, PreferenceCategory pC,
                                        Intent intent) {
        if (intent == null) {
            throw new RuntimeException("configSelectCardPreference: intent cannot be null");
        }
        String preferenceName = intent.getStringExtra(PREFERENCE_NAME);
        if (preferenceName != null && preferenceName.equals(PREFERENCE_NAME_SMS_VALIDITY_PERIOD)) {
            hostActivity.setTitle(getString(R.string.sms_validity_period));
        }
    }
*/
    /**
     * handleSelectCardPreferenceTreeClick.
     * @param hostActivity Activity
     * @param subId int
     * @return boolean
     */
    /* q0 migration, phase out sms validate*/
    /*
    public boolean handleSelectCardPreferenceTreeClick(Activity hostActivity, final int subId) {
        Intent intent = hostActivity.getIntent();
        String intentKey = intent.getStringExtra(PREFERENCE_NAME);
        //int slotid = SubscriptionManager.getSlotId(subId);
        if (intentKey != null && intentKey.equals(PREFERENCE_NAME_SMS_VALIDITY_PERIOD)) {
            showSmsValidityPeriodDialog(hostActivity, subId);
            return true;
        }
        return false;
    }

    private String getSmsValidityKeyBySubId(int subId) {
        return Integer.toString(subId) + "_" + SMS_VALIDITY_PERIOD_PREFERENCE_KEY;
    }
    private void showSmsValidityPeriodDialog(Context context, int subId) {
        final CharSequence[] entries = getResources()
                                    .getTextArray(R.array.sms_validity_peroid_entries);
        final String validityKey = getSmsValidityKeyBySubId(subId);
        final SharedPreferences pref = PreferenceManager.getDefaultSharedPreferences(context);
        int index = pref.getInt(validityKey, 0);
        AlertDialog.Builder builder = new AlertDialog.Builder(context)
            .setTitle(getResources().getText(R.string.sms_validity_period))
            .setNegativeButton(android.R.string.cancel, null)
            .setSingleChoiceItems(entries, index, new DialogInterface.OnClickListener() {
             public void onClick(DialogInterface dialog, int item) {
                 Log.d(TAG, "showSmsValidityPeriodDialog->onClick: item =  " + item);
                SharedPreferences.Editor editor = pref.edit();
                editor.putInt(validityKey, item);
                editor.commit();
                dialog.dismiss();
            }
        });
        builder.show();
        builder.create();
    }
*/

    // Add for forward sms with append sender
    /* q0 migration, phase out forward sms with info*/
    /*
    private void addForwardWithSenderPreference(Context context, PreferenceCategory smsCategory) {
        Log.d(TAG, "Call addForwardWithSenderPref 1");
        CheckBoxPreference sp = new CheckBoxPreference(context);
        sp.setKey(SMS_FORWARD_WITH_SENDER);
        sp.setTitle(getString(R.string.sms_forward_setting));
        sp.setSummary(getString(R.string.sms_forward_setting_summary));
        SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(context);
        boolean smsForwardWithSender = prefs.getBoolean(SMS_FORWARD_WITH_SENDER, true);
        sp.setChecked(smsForwardWithSender);
        smsCategory.addPreference(sp);
    }
*/
    /**
     * formatSmsBody.
     * @param context Context
     * @param smsBody String
     * @param nameAndNumber String
     * @param boxId int
     * @return String
     */
    /* q0 migration, phase out forward sms with info*/
    /*
    public String formatSmsBody(Context context, String smsBody,
            String nameAndNumber, int boxId) {
        Log.d(TAG, "Call formatSmsBody 1");
        SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(context);
        boolean smsForwardWithSender = prefs.getBoolean(SMS_FORWARD_WITH_SENDER, true);
        Log.d(TAG, "forwardMessage(): SMS Forward With Sender ?= " + smsForwardWithSender);
        if (smsForwardWithSender) {
//            if (boxId == Mms.MESSAGE_BOX_INBOX) {
//                smsBody += "\n" + getString(R.string.forward_from);
//                smsBody += nameAndNumber;
//            }
              if (boxId == Mms.MESSAGE_BOX_INBOX) {
                  nameAndNumber = nameAndNumber + ":\n";
                  nameAndNumber += smsBody;
              } else {
                  nameAndNumber = getString(R.string.forward_byself) + ":\n";
                  nameAndNumber += smsBody;
              }
              return nameAndNumber;
        }
        return smsBody;
    }
*/
    /**
     * formatSmsBody.
     * @param context Context
     * @param smsBody String
     * @param nameAndNumber String
     * @param cursor Cursor
     * @return String
     */
    /* q0 migration, phase out forward sms with info*/
    /*
    public String formatSmsBody(Context context, String smsBody,
            String nameAndNumber, Cursor cursor) {
        Log.d(TAG, "Call formatSmsBody 2");
        SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(context);
        boolean smsForwardWithSender = prefs.getBoolean(SMS_FORWARD_WITH_SENDER, true);
        Log.d(TAG, "forwardMessage(): SMS Forward With Sender ?= " + smsForwardWithSender);

        if (smsForwardWithSender) {
            long mStatus = cursor.getLong(cursor.getColumnIndexOrThrow("status"));
            if (mStatus == SmsManager.STATUS_ON_ICC_READ
                            || mStatus == SmsManager.STATUS_ON_ICC_UNREAD) {
                   // it is a inbox sms
                   Log.d(TAG, "It is a inbox sms");
                   smsBody += "\n" + getString(R.string.forward_from);
                   smsBody += nameAndNumber;
            }
        }
        return smsBody;
    }
*/
    /**
     * configGeneralPreference.
     * @param hostActivity Activity
     * @param pC PreferenceCategory
     */
    public void configGeneralPreference(Activity hostActivity, PreferenceCategory pC) {
        Log.d(TAG, "configGeneralPreference");
        addStorageStatusPreference(hostActivity, pC);
    }

    private void addStorageStatusPreference(final Activity hostActivity, PreferenceCategory pC) {
        Log.d(TAG, "addStorageStatusPreference");
        /* if (!EncapsulationConstant.USE_MTK_PLATFORM) {
         // if not mtk platform, no this feature
            return;
        } */

        Preference p = new Preference(hostActivity);
        p.setTitle(getString(R.string.pref_title_storage_status));
        p.setSummary(getString(R.string.pref_title_storage_status));
        p.setOrder(0);
        int count = pC.getPreferenceCount();
        //reset order
        for (int index = 0; index < count; index ++) {
            Preference preference = pC.getPreference(index);
            if (preference != null) {
                int pOrder = preference.getOrder();
                if (pOrder >= 0) {
                    preference.setOrder(pOrder + 1);
                }
            }
        }
        p.setOnPreferenceClickListener(new Preference.OnPreferenceClickListener() {
                @Override
                public boolean onPreferenceClick(Preference preference) {
                    Log.d(TAG, "addStorageStatusPreference: onPreferenceClick, preference = " +
                                    preference);
                    showStorageStatusDialog(hostActivity);
                    return true;
                }
            });
        pC.addPreference(p);
    }

    private void showStorageStatusDialog(Activity hostActivity) {
            final String memoryStatus = Op01MessagePluginExt
                    .sMessageUtilsCallback.getStorageStatus();
            String status = getResources().getString(R.string.pref_title_storage_status);
            Drawable icon = getResources().getDrawable(R.drawable.ic_dialog_info_holo_light);
            new AlertDialog.Builder(hostActivity)
                    .setTitle(status)
                    .setIcon(icon)
                    .setMessage(memoryStatus)
                    .setPositiveButton(android.R.string.ok, null)
                    .setCancelable(true)
                    .show();
    }

    /**
     * configMmsPreference.
     * @param hostActivity Activity
     * @param pC PreferenceCategory
     * @param simCount int
     */
    public void configMmsPreference(Activity hostActivity, PreferenceCategory pC, int simCount) {
        Log.d(TAG, "configMmsPreference");
        addMmsDeliveryReportPreference(hostActivity, pC, simCount);
    }

    private void addMmsDeliveryReportPreference(final Activity hostActivity, PreferenceCategory pC,
            int simCount) {
        Log.d(TAG, "addMmsDeliveryReportPreference: simCount = " + simCount);
        CheckBoxPreference cp = new CheckBoxPreference(hostActivity);
        cp.setTitle(getString(R.string.pref_title_mms_enable_to_send_delivery_reports));
        cp.setSummary(getString(R.string.pref_summary_mms_enable_to_send_delivery_reports));
        cp.setOrder(2);

        Preference p = new Preference(hostActivity);
        p.setKey(MMS_ENABLE_TO_SEND_DELIVERY_REPORT_KEY);
        p.setTitle(getString(R.string.pref_title_mms_enable_to_send_delivery_reports));
        p.setSummary(getString(R.string.pref_summary_mms_enable_to_send_delivery_reports));
        p.setOrder(2);

        if (Op01MmsUtils.isMtkGemini()) {
            final List<SubscriptionInfo> simList =
                    SubscriptionManager.from(hostActivity).getActiveSubscriptionInfoList();
            final SharedPreferences pref =
                    PreferenceManager.getDefaultSharedPreferences(hostActivity);

            if (simCount > 1) {
                p.setOnPreferenceClickListener(new Preference.OnPreferenceClickListener() {
                    @Override
                    public boolean onPreferenceClick(Preference preference) {
                        Log.d(TAG, "addMmsDeliveryReportPreference: onPreferenceClick, " +
                                "preference = " + preference);
                        Intent intent = new Intent();
                        intent.setClassName(hostActivity, "com.mediatek.setting.SubSelectActivity");
                        intent.putExtra("PREFERENCE_KEY", MMS_ENABLE_TO_SEND_DELIVERY_REPORT_KEY);
                        intent.putExtra(PREFERENCE_NAME, MMS_ENABLE_TO_SEND_DELIVERY_REPORT_KEY);
                        hostActivity.startActivity(intent);
                        return true;
                    }
                });
                pC.addPreference(p);
            } else {
                if (simCount == 0) {
                    cp.setEnabled(false);
                } else {
                    int simId = simList.get(0).getSubscriptionId();
                    cp.setKey(Integer.toString(simId) + "_" +
                                            MMS_ENABLE_TO_SEND_DELIVERY_REPORT_KEY);
                    // get the stored value
                    SharedPreferences sp =
                          getSharedPreferences("com.android.mms_preferences", MODE_PRIVATE);
                    cp.setChecked(sp.getBoolean(cp.getKey(), false));
                }
                pC.addPreference(cp);
            }
        } else {
            cp.setKey(MMS_ENABLE_TO_SEND_DELIVERY_REPORT_KEY);
            pC.addPreference(cp);
        }
        int count = pC.getPreferenceCount();
        //reset order
        for (int index = 0; index < count; index ++) {
            Preference preference = pC.getPreference(index);
            if (preference != null) {
                int pOrder = preference.getOrder();
                if (pOrder >= 2) {
                    preference.setOrder(pOrder + 1);
                }
            }
        }
    }

    /**
     * configMmsPreferenceState.
     * @param hostActivity Activity
     * @param preference String
     * @param subId int
     * @param cp CheckBoxPreference
     * @return boolean
     */
    public boolean configMmsPreferenceState(Activity hostActivity, String preference,
                                                        int subId, CheckBoxPreference cp) {
        Log.d(TAG, "configMmsPreferenceState[hostActivity]" + hostActivity);
        Log.d(TAG, "configMmsPreferenceState[preference]" + preference);
        Log.d(TAG, "configMmsPreferenceState[simId]" + subId);
        SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(hostActivity);
        if (preference != null && preference.equals(MMS_ENABLE_TO_SEND_DELIVERY_REPORT_KEY)) {
            boolean checked = prefs.getBoolean(Integer.toString(subId) + "_" +
                        MMS_ENABLE_TO_SEND_DELIVERY_REPORT_KEY, false);
            cp.setChecked(checked);
            return true;
        }
        return false;
    }

    /**
     * configMultiSimPreferenceTitle.
     * @param hostActivity Activity
     */
    public void configMultiSimPreferenceTitle(Activity hostActivity) {
        Intent intent = hostActivity.getIntent();
        String preferenceName = intent.getStringExtra(PREFERENCE_NAME);
        if (preferenceName != null &&
                            preferenceName.equals(MMS_ENABLE_TO_SEND_DELIVERY_REPORT_KEY)) {
            hostActivity.setTitle(
                            getString(R.string.pref_title_mms_enable_to_send_delivery_reports));
        }
    }

    /**
     * setMmsPreferenceState.
     * @param hostActivity Activity
     * @param preference String
     * @param subId int
     * @param checked boolean
     * @return boolean
     */
    public boolean setMmsPreferenceState(Activity hostActivity, String preference,
                                                    int subId, boolean checked) {
        Log.d(TAG, "setMmsPreferenceState[hostActivity]" + hostActivity);
        Log.d(TAG, "setMmsPreferenceState[preference]" + preference);
        Log.d(TAG, "setMmsPreferenceState[simId]" + subId);
        Log.d(TAG, "setMmsPreferenceState[checked]" + checked);
        SharedPreferences.Editor editor =
                PreferenceManager.getDefaultSharedPreferences(hostActivity).edit();
        if (preference.equals(MMS_ENABLE_TO_SEND_DELIVERY_REPORT_KEY)) {
            editor.putBoolean(Integer.toString(subId) + "_" +
                        MMS_ENABLE_TO_SEND_DELIVERY_REPORT_KEY, checked);
            return true;
        }
        return false;
    }

    public View getView(String preferenceKey, View view) {
        if (SMS_VALIDITY_PERIOD_PREFERENCE_KEY.equals(preferenceKey)) {
            CheckBox subCheckBox = (CheckBox) view.findViewWithTag("selectSubCheckBox");
            subCheckBox.setVisibility(View.GONE);
            return view;
        }
        return null;
    }

    public String[] getSaveLocationString() {
        Log.d(TAG, "getSaveLocationString");
        return getResources().getStringArray(R.array.cmcc_sms_save_location_array);
    }
}

