/*
 * Copyright (C) 2014 The Android Open Source Project
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

package com.mediatek.setting;

import android.app.ActionBar;
import android.app.AlertDialog;
import android.app.ListActivity;
import android.content.DialogInterface;
import android.content.DialogInterface.OnClickListener;
import android.content.Intent;
import android.content.SharedPreferences;
import android.os.Bundle;
import android.preference.PreferenceManager;
import android.telephony.SubscriptionInfo;
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;
import android.text.InputFilter;
import android.util.Log;
import android.view.MenuItem;
import android.view.View;
import android.view.inputmethod.EditorInfo;
import android.widget.CheckBox;
import android.widget.EditText;
import android.widget.ListView;
import android.widget.Toast;

import com.android.internal.telephony.PhoneConstants;
import com.android.mms.LogTag;
import com.android.mms.MmsApp;
import com.android.mms.R;
import com.android.mms.ui.MessageUtils;
import com.android.mms.util.FeatureOption;
import com.android.mms.util.MmsLog;

import com.mediatek.cb.cbsettings.CellBroadcastSettings;
import com.mediatek.mms.ext.IOpSubSelectActivityExt;
import com.mediatek.opmsg.util.OpMessageUtils;
import com.mediatek.setting.SimStateMonitor.SimStateListener;
import com.mediatek.simmessage.ManageSimMessages;

import java.util.ArrayList;
import java.util.List;

import mediatek.telephony.MtkSmsManager;

public class SubSelectActivity extends ListActivity implements SimStateListener {

    private static String TAG = "SubSelectActivity";

    // If intent has longArrayExtra with key EXTRA_APPOINTED_SUBS, activity
    // only show subs in the longArrayExtra. If intent doesn't has the extra
    // value,activity will show all active subs.
    public static final String EXTRA_APPOINTED_SUBS = "subsArray";
    private List<SubscriptionInfo> mSubInfoList = new ArrayList<SubscriptionInfo>();
    private String mPreferenceKey;
    private int mPreferenceTitleId;
    private SubSelectAdapter mAdapter;
    private int mOldSubCount = 0;
    private int[] mAppointedSubArray = null;
    private EditText mNumberText;
    private AlertDialog mNumberTextDialog;
    private int mCurrentSubId = -1;
    private IOpSubSelectActivityExt mOpSubSelectActivityExt;
    private SharedPreferences mSpref;
    private static final int MAX_EDITABLE_LENGTH = 20;
    public String SUB_TITLE_NAME = "sub_title_name";

    @Override
    protected void onCreate(Bundle icicle) {
        super.onCreate(icicle);
        mSpref = PreferenceManager.getDefaultSharedPreferences(this);
        getExtraValues(getIntent());
        if (Log.isLoggable(LogTag.TRANSACTION, Log.VERBOSE)) {
            Log.d(TAG, "onCreate preference key is: " + mPreferenceKey);
        }
        if (mPreferenceTitleId != 0) {
            setTitle(mPreferenceTitleId);
        }
        // add Plugin,set title
        mOpSubSelectActivityExt = OpMessageUtils.getOpMessagePlugin()
                .getOpSubSelectActivityExt();
        mOpSubSelectActivityExt.onCreate(this);
        //add action bar
        ActionBar actionBar = getActionBar();
        actionBar.setDisplayHomeAsUpEnabled(true);
        mOldSubCount = SubscriptionManager.from(MmsApp.getApplication())
                .getActiveSubscriptionInfoCount();
        setAdapter();
    }

    @Override
    protected void onStart() {
        super.onStart();
        SimStateMonitor.getInstance().addListener(this);
    }

    @Override
    protected void onStop() {
        super.onStop();
        SimStateMonitor.getInstance().removeListener(this);
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();

        if (mNumberTextDialog != null && mNumberTextDialog.isShowing()) {
            mNumberTextDialog.dismiss();
        }
    }

    @Override
    protected void onResume() {
        super.onResume();
        Log.d(TAG, "onResume");
        refreshAdapter();
    }

    @Override
    protected void onNewIntent(Intent intent) {
        super.onNewIntent(intent);
        getExtraValues(intent);
        setTitle(mPreferenceTitleId);
        mAppointedSubArray = intent.getIntArrayExtra(EXTRA_APPOINTED_SUBS);
        refreshAdapter();
    }

    private void getExtraValues(Intent intent) {
        mPreferenceKey = intent.getStringExtra(SmsPreferenceActivity.PREFERENCE_KEY);
        mPreferenceTitleId = intent
                .getIntExtra(SmsPreferenceActivity.PREFERENCE_TITLE_ID, 0);
        mAppointedSubArray = intent.getIntArrayExtra(EXTRA_APPOINTED_SUBS);
    }

    private void initialSubInfoList() {
        int simCount = TelephonyManager.getDefault().getSimCount();
        mSubInfoList.clear();
        for (int slotId = 0; slotId < simCount; slotId++) {
            SubscriptionInfo subInfoRecordInOneSim = SubscriptionManager.from(
                    MmsApp.getApplication())
                    .getActiveSubscriptionInfoForSimSlotIndex(slotId);
            if (subInfoRecordInOneSim == null) {
                continue;
            } else {
                    // mNeedShowSubArray == null means intent isn't specified
                if (mAppointedSubArray == null
                        || isSubIdInNeededShowArray(subInfoRecordInOneSim.getSubscriptionId())) {
                    mSubInfoList.add(subInfoRecordInOneSim);
                    }

            }
        }
        if (mSubInfoList == null || mSubInfoList.size() == 0) {
            finish();
            return;
        }
    }

    private void setAdapter() {
        initialSubInfoList();
        mAdapter = new SubSelectAdapter(this, mPreferenceKey, mSubInfoList);
        setListAdapter(mAdapter);
    }

    private void refreshAdapter() {
        initialSubInfoList();
        mAdapter.setPreferenceKey(mPreferenceKey);
        mAdapter.notifyDataSetChanged();
    }

    @Override
    protected void onListItemClick(ListView l, View v, int position, long id) {
        mCurrentSubId = mSubInfoList.get(position).getSubscriptionId();
        // start manage SIM message activity if preference is MANAGE_SIM_MESSAGE_MODE.
        // else change the preference data.
        //add OP01 Plugin,set select card info
        if (mOpSubSelectActivityExt.onListItemClick(this, mCurrentSubId)) {
            return;
        }
        if (MmsPreferenceActivity.RETRIEVAL_DURING_ROAMING.equals(mPreferenceKey)) {
            if (PreferenceManager.getDefaultSharedPreferences(getApplicationContext()).getBoolean(
                    Long.toString((mSubInfoList.get(position)).getSubscriptionId()) + "_"
                            + MmsPreferenceActivity.AUTO_RETRIEVAL, true) == false) {
                return;
            }
        }
        if (SmsPreferenceActivity.SMS_MANAGE_SIM_MESSAGES.equals(mPreferenceKey)) {
            /// change for ALPS01964512, don't allow go into SIM message before SIM ready. @{
            if (MessageUtils.isSimMessageAccessable(this, mCurrentSubId)) {
                startManageSimMessages(position);
            }
            /// @}
        } else if (SmsPreferenceActivity.SMS_SERVICE_CENTER.equals(mPreferenceKey)) {
            setServiceCenter(mCurrentSubId);
        } else if (SmsPreferenceActivity.SMS_SAVE_LOCATION.equals(mPreferenceKey)) {
            setSaveLocation(mCurrentSubId);
        } else if (GeneralPreferenceActivity.CELL_BROADCAST.equals(mPreferenceKey)) {
            startCellBroadcast(position);
        } else {
            boolean isChecked =  mAdapter.isChecked(position);
            SharedPreferences.Editor editor = PreferenceManager.getDefaultSharedPreferences(this)
                    .edit();
            editor.putBoolean(Long.toString((mSubInfoList.get(position)).getSubscriptionId()) + "_"
                    + mPreferenceKey, (!isChecked));
            editor.apply();
            CheckBox subCheckBox = (CheckBox) v.findViewById(R.id.subCheckBox);
            subCheckBox.setChecked(!isChecked);
        }
    }

    private void startCellBroadcast(int position) {
        Intent it = new Intent();
        MmsLog.i(TAG, "startCellBroadcast, currentSubId is: "
                + mSubInfoList.get(position).getSubscriptionId());
        it.setClass(this, CellBroadcastSettings.class);
        it.putExtra(PhoneConstants.SUBSCRIPTION_KEY, mSubInfoList.get(position)
                .getSubscriptionId());
        Log.d(TAG,
                "mSubInfoList.get(position).getSubscriptionId(): "
                + mSubInfoList.get(position).getSubscriptionId());
        it.putExtra(SUB_TITLE_NAME, mSubInfoList.get(position).getDisplayName().toString());
        startActivity(it);
    }

    private void setSaveLocation(int subId) {
        MmsLog.d(TAG, "setSaveLocation, subId is: " + subId);
        //the key value for each saveLocation
        final String [] saveLocation;
        //the diplayname for each saveLocation
        String [] saveLocationDisp;

        if (!getResources().getBoolean(R.bool.isTablet)) {
            saveLocation = getResources().getStringArray(R.array.pref_sms_save_location_values);
            saveLocationDisp = mOpSubSelectActivityExt.setSaveLocation();
            if (saveLocationDisp == null) {
                saveLocationDisp = getResources().getStringArray(
                        R.array.pref_sms_save_location_choices);
            }
        } else {
            saveLocation = getResources().getStringArray(
                    R.array.pref_tablet_sms_save_location_values);
            saveLocationDisp = getResources().getStringArray(
                    R.array.pref_tablet_sms_save_location_choices);
        }

           if (saveLocation == null || saveLocationDisp == null) {
               MmsLog.d(TAG, "setSaveLocation is null");
               return;
           }

        final String saveLocationKey
                = Long.toString(subId) + "_" + SmsPreferenceActivity.SMS_SAVE_LOCATION;
        int pos = getSelectedPosition(saveLocationKey, saveLocation);
        new AlertDialog.Builder(this)
            .setTitle(R.string.sms_save_location)
            .setNegativeButton(R.string.Cancel, new NegativeButtonListener())
            .setSingleChoiceItems(saveLocationDisp, pos, new DialogInterface.OnClickListener() {
                    public void onClick(DialogInterface dialog, int whichButton) {
                        SharedPreferences.Editor editor = mSpref.edit();
                        editor.putString(saveLocationKey, saveLocation[whichButton]);
                        editor.commit();
                        dialog.dismiss();
//                        mSim.setNotifyChange(context);
                    }
            }).show();
    }

    // get the position which is selected before
    private int getSelectedPosition(String inputmodeKey, String[] modes) {
        /// M: fix bug ALPS00455172, add tablet "device" support
        String res = "";
        if (!getResources().getBoolean(R.bool.isTablet)) {
            res = mSpref.getString(inputmodeKey, "Phone");
        } else {
            res = mSpref.getString(inputmodeKey, "Device");
        }
        MmsLog.d(TAG, "getSelectedPosition found the res = " + res);
        for (int i = 0; i < modes.length; i++) {
            if (res.equals(modes[i])) {
                MmsLog.d(TAG, "getSelectedPosition found the position = " + i);
                return i;
            }
        }
        MmsLog.d(TAG, "getSelectedPosition not found the position");

        return 0;
    }

    public void setServiceCenter(final int subId) {
        new Thread(new Runnable() {
            @Override
            public void run() {
                // TODO Auto-generated method stub
                MmsLog.d(TAG, "setServiceCenter slotId is : " + subId);
                final Bundle result = MtkSmsManager.getSmsManagerForSubscriptionId(subId)
                        .getScAddressWithErroCode();
                runOnUiThread(new Runnable() {
                    @Override
                    public void run() {
                        // TODO Auto-generated method stub
                        if (result != null
                                && result.getByte(MtkSmsManager.GET_SC_ADDRESS_KEY_RESULT)
                                        == MtkSmsManager.ERROR_CODE_NO_ERROR) {
                            AlertDialog.Builder dialog = new AlertDialog
                                    .Builder(SubSelectActivity.this);
                            String scNumber = (String) result.getCharSequence(
                                    MtkSmsManager.GET_SC_ADDRESS_KEY_ADDRESS);
                            MmsLog.d(TAG, "getScAddress is: " + scNumber);
                            mNumberText = new EditText(dialog.getContext());
                            mNumberText.setHint(R.string.type_to_compose_text_enter_to_send);
                            mNumberText.setFilters(new InputFilter[] { new InputFilter
                                    .LengthFilter(MAX_EDITABLE_LENGTH) });
                            mNumberText.setInputType(
                                    EditorInfo.TYPE_CLASS_TEXT | EditorInfo.TYPE_CLASS_PHONE);
                            mNumberText.computeScroll();
                            mNumberText.setText(scNumber);
                            if (!SubSelectActivity.this.isFinishing()) {
                                mNumberTextDialog = dialog.setIcon(
                                        R.drawable.ic_dialog_info_holo_light)
                                    .setTitle(R.string.sms_service_center)
                                    .setView(mNumberText)
                                    .setPositiveButton(R.string.OK, new PositiveButtonListener())
                                    .setNegativeButton(R.string.Cancel,
                                            new NegativeButtonListener())
                                    .show();
                            }
                        } else if (result != null
                                && result.getByte(MtkSmsManager.GET_SC_ADDRESS_KEY_RESULT)
                                == MtkSmsManager.ERROR_CODE_NO_SUPPORT_SC_ADDR) {
                            Toast.makeText(getApplicationContext(),
                                    getString(R.string.cdma_not_support),
                                    Toast.LENGTH_SHORT).show();
                            MmsLog.dpi(TAG, "getScAddress not support error: " + result);
                        } else {
                            MmsLog.dpi(TAG, "getScAddress error: " + result);
                            Toast.makeText(getApplicationContext(),
                                    getString(R.string.sms_not_ready),
                                    Toast.LENGTH_SHORT).show();
                        }
                    }
                });
            }
        }).start();
    }

    private class PositiveButtonListener implements OnClickListener {
        public void onClick(DialogInterface dialog, int which) {
            final String scNumber = mNumberText.getText().toString();
            MmsLog.dpi(TAG, "setScAddress is: " + scNumber
                    + " mCurrentSubId is: " + mCurrentSubId);
            new Thread(new Runnable() {
                public void run() {
                    MtkSmsManager.getSmsManagerForSubscriptionId(mCurrentSubId)
                        .setScAddress(scNumber);
                }
            }).start();
        }
    }

    private class NegativeButtonListener implements OnClickListener {
        public void onClick(DialogInterface dialog, int which) {
            // cancel
            dialog.dismiss();
        }
    }

    public void startManageSimMessages(int position) {
        Intent it = new Intent();
        it.setClass(this, ManageSimMessages.class);
        it.putExtra(PhoneConstants.SUBSCRIPTION_KEY,
                mSubInfoList.get(position).getSubscriptionId());
        startActivity(it);
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        switch (item.getItemId()) {
            case android.R.id.home:
                // The user clicked on the Messaging icon in the action bar. Take them back from
                // wherever they came from
                finish();
                return true;
        }
        return false;
    }

    private boolean isSubIdInNeededShowArray(int subId) {
        for (int id : mAppointedSubArray) {
            if (subId == id) {
                return true;
            }
        }
        return false;
    }

    @Override
    public void onSimStateChanged() {
        if (SimStateMonitor.getInstance().getSubCount() != mOldSubCount) {
            Log.d(TAG, "sub count changed");
            finish();
        } else {
            mAdapter.notifyDataSetChanged();
        }
    }


    @Override
    public void onActivityResult(int requestCode, int resultCode, Intent data) {
        MmsLog.d(TAG, "onActivityResult");
        mOpSubSelectActivityExt.onActivityResult(requestCode, resultCode, data);
    }
}
