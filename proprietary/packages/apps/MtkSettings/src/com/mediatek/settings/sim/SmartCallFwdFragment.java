/*
 * Copyright (C) 2015 The Android Open Source Project
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

package com.mediatek.settings.sim;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.ProgressDialog;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.DialogInterface;
import android.content.ServiceConnection;
import android.content.SharedPreferences;
import android.content.SharedPreferences.Editor;
import android.content.pm.PackageManager;
import android.content.pm.ResolveInfo;
import android.os.Bundle;
import android.os.IBinder;
import android.os.Handler;
import android.os.Message;
import android.os.Messenger;
import android.os.RemoteException;

import androidx.preference.EditTextPreference;
import androidx.preference.ListPreference;
import androidx.preference.Preference;
//import androidx.preference.PreferenceFragment;
import android.telephony.SubscriptionInfo;
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;
import android.telephony.PhoneNumberUtils;
import android.util.Log;
import android.widget.Switch;
import android.widget.TextView;
import android.widget.Toast;

//import com.android.internal.logging.MetricsLogger;
import com.android.internal.telephony.CommandsInterface;
import com.android.settings.widget.SwitchBar;
import com.android.settings.SettingsActivity;
import com.android.settings.SettingsPreferenceFragment;
import com.android.settings.R;
import com.mediatek.internal.telephony.MtkSubscriptionManager;
import com.mediatek.telephony.MtkTelephonyManagerEx;
import java.util.List;
/**
 * "Smart Call Forward settings" screen.  This preference screen lets you
 * enable/disable Smart call forward setting and change smart call forwarding mode.
 */
public class SmartCallFwdFragment extends SettingsPreferenceFragment
        implements SwitchBar.OnSwitchChangeListener,
        Preference.OnPreferenceChangeListener {

    private static final String TAG = "SmartCallFwdFragment";

    //String keys for preference lookup
    private static final String BUTTON_SMART_CALL_FWD = "smart_call_fwd_modes";
    private static final String SIM1_PREF = "smart_sim1_pref"; //Give unigque name in Settings app
    private static final String SIM2_PREF = "smart_sim2_pref"; //Give unigque name in Settings app
    private static final String SMART_CALL_FWD_MODE_PREF = "smart_call_fwd_modes";
    private static final String VALUE_NOT_SET = "unknown";
    private static final String SMARTCALLMODE = "smartcallmode";
    //Shared preference to store the mSwitch state
    private static final String SHARED_PREFERENCES_NAME = "sim_id";
    private static final String SWITCH_STATE = "switch_state";
    private static final String SLOT_PREFIX = "sim_slot_";

    private final int MSG_TYPE_DATA = 0;
    private final int MSG_TYPE_REGISTER = 1;
    private final int MSG_GET_CF_REQ = 2;
    private final int MSG_SET_CF_REQ = 3;
    private final int MSG_GET_CF_RES = 4;
    private final int MSG_SET_CF_RES = 5;
    private final int MSG_TYPE_DEREGISTER = 6;

    private final int CF_REASON_NOT_REACHABLE = 3;
    private final int SIM1 = 0;
    private final int SIM2 = 1;
    private final int TOTAL_SIM = 2;
    private final int READ = 0;
    private final int WRITE = 1;

    private final int SIM1_TO_SIM2 = 1;
    private final int SIM2_TO_SIM1 = 2;
    private final int DUAL_SIM = 3;

    //UI objects
    private SwitchBar mSwitchBar;
    private boolean mSwitchState = false;
    private ListPreference mButtonSmartCallFwdMode;
    private TextView mEmptyView;
    private Context mContext;
    private boolean mValidListener = false;

    private EditTextPreference mSim1Pref;
    private EditTextPreference mSim2Pref;
    private ListPreference mSmartCallFwdModePref;
    private SharedPreferences mSharedPreferences = null;
    private Messenger mMessenger = new Messenger(new IncomingHandler());
    private CFInfo[] mCfInfoArr = new CFInfo[] {new CFInfo(), new CFInfo()};
    private IntentFilter mIntentFilter;
    private String[] mSummary = {
                                    "SIM1 TO SIM2",
                                    "SIM2 TO SIM1",
                                    "DUAL SIM"
                                };
    private String[] mActionString = {
                                        "Disabling",
                                        "Enabling"
                                      };
    private String[] simPrefValue = new String[TOTAL_SIM];

    private boolean mReadProgress = false;
    private boolean mWriteProgress = false;
    private boolean mFlag = false;
    private MtkTelephonyManagerEx mTelephonyManagerEx;
    private TelephonyManager mTelephonyManager;
    private ProgressDialog mProgressDialog;
    private AlertDialog mAlertDialog;
    private String mSim1num = null;
    private String mSim2num = null;
    private int mCurrSelectedMode = 3;
    private int mPrevSelectedMode = 3;
    private boolean mPrevSwitchState = false;
    private boolean mNewSim1Inserted = false;
    private boolean mNewSim2Inserted = false;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        addPreferencesFromResource(R.xml.smart_call_fwd_settings);
    }

    @Override
    public void onResume() {
        super.onResume();
        Log.d(TAG, "onResume:");
    }

    @Override
    public void onPause() {
        super.onPause();
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
        if (getActivity().isFinishing()) {
            stopService();
        }
    }


    @Override
    public void onAttach(Activity activity) {
        // TODO Auto-generated method stub
        super.onAttach(activity);
        Log.d(TAG, "onAttach:");
        mContext = activity.getApplicationContext();
        mTelephonyManagerEx = MtkTelephonyManagerEx.getDefault();
        mTelephonyManager = (TelephonyManager) mContext.getSystemService(Context.TELEPHONY_SERVICE);
        startService();
    }

    @Override
    public void onActivityCreated(Bundle savedInstanceState) {
        super.onActivityCreated(savedInstanceState);
        Log.d(TAG, "onActivityCreated:");
        mSharedPreferences = mContext.getSharedPreferences(SHARED_PREFERENCES_NAME,
                Context.MODE_PRIVATE);
        final SettingsActivity activity = (SettingsActivity) getActivity();
        if (activity == null) {
            return;
        }
        mSwitchBar = activity.getSwitchBar();
        mSwitchBar.addOnSwitchChangeListener(this);
        mSwitchBar.show();
        mSwitchBar.setEnabled(false);

        mSim1Pref = (EditTextPreference) findPreference(SIM1_PREF);
        mSim2Pref = (EditTextPreference) findPreference(SIM2_PREF);
        ///first check sim phone number..
        // if it is set then display it and don't allow user to change
        mSim1num = getLine1Number(SIM1);
        mSim2num = getLine1Number(SIM2);
        Log.d(TAG, "mSim1num: " + mSim1num + ", mSim2num: " + mSim2num);

        // Restore previous EditTextPreference values
        getPreviousPrefValue();
        if (mSim1num != null && mSim1num.length() > 0) {
            Log.d(TAG, "mSim1num.length():" + mSim1num.length());
            mSim1Pref.setText(mSim1num);
        } else {
            // Update Sim1 EditTextPreferences
            if (simPrefValue[SIM1] != null) {
                mSim1Pref.setText(simPrefValue[SIM1]);
            }
            mSim1num = null;
        }
        if (mSim2num != null && mSim2num.length() > 0) {
            mSim2Pref.setText(mSim2num);
            Log.d(TAG, "mSim2num.length():" + mSim2num.length());
        } else {
            // Update Sim2 EditTextPreferences
            if (simPrefValue[SIM2] != null) {
                mSim2Pref.setText(simPrefValue[SIM2]);
            }
            mSim2num = null;
        }
        detectSimChange();
        mSmartCallFwdModePref = (ListPreference) findPreference(SMART_CALL_FWD_MODE_PREF);
        mSim1Pref.setOnPreferenceChangeListener(this);
        mSim2Pref.setOnPreferenceChangeListener(this);
        mSmartCallFwdModePref.setOnPreferenceChangeListener(this);
        mSmartCallFwdModePref.setValue(Integer.toString(DUAL_SIM));
        mSmartCallFwdModePref.setSummary(mSummary[DUAL_SIM - 1]);
        showProgressDialog(getResources().getString(R.string.progress_dlg_reading));
    }

    @Override
    public int getMetricsCategory() {
        return 0;
    }


    private void showProgressDialog(String msg) {
        Log.d(TAG, "showProgressDialog");
        mProgressDialog = new ProgressDialog(getActivity());
        mProgressDialog.setIndeterminate(false);
        mProgressDialog.setProgressStyle(ProgressDialog.STYLE_SPINNER);
        mProgressDialog.setCancelable(false);
        mProgressDialog.setCanceledOnTouchOutside(false);
        mProgressDialog.setTitle(R.string.progress_dlg_title);
        mProgressDialog.setMessage(msg);
        mProgressDialog.show();
    }

    private void updatePreference(int mode) {
        Log.d(TAG, "updatePreference");
        mSwitchState = (mode != 0);
        if (mode > 0) {
            mSmartCallFwdModePref.setValue(Integer.toString(mode));
            mSmartCallFwdModePref.setSummary(mSummary[mode - 1]);
        } else {
            mSmartCallFwdModePref.setValue(Integer.toString(DUAL_SIM));
            mSmartCallFwdModePref.setSummary(mSummary[DUAL_SIM - 1]);
        }
        Log.d(TAG, "mSwitchState:" + mSwitchState);
        mSwitchBar.setEnabled(true);
        if (mSwitchState != mSwitchBar.isChecked()) {
            mFlag = true;
        }
        mSwitchBar.setChecked(mSwitchState);
        mSmartCallFwdModePref.setEnabled(true);
        if (mSim1num == null) {
            mSim1Pref.setEnabled(!mSwitchState);
        }
        if (mSim2num == null) {
            mSim2Pref.setEnabled(!mSwitchState);
        }
        if (mNewSim1Inserted || mSim1Pref.getText() == null ||
                mSim1Pref.getText() != null && mSim1Pref.getText().length() == 0) {
            mSim1Pref.setSummary(VALUE_NOT_SET);
            mSim1Pref.setText("");
        } else {
            mSim1Pref.setSummary(mSim1Pref.getText());
        }
        if (mNewSim2Inserted || mSim2Pref.getText() == null ||
                mSim2Pref.getText() != null && mSim2Pref.getText().length() == 0) {
            mSim2Pref.setSummary(VALUE_NOT_SET);
            mSim2Pref.setText("");
        } else {
            mSim2Pref.setSummary(mSim2Pref.getText());
        }
    }

    private void detectSimChange() {
        final int numSlots = mTelephonyManager.getSimCount();
        boolean newSimInserted = false;
        Log.d(TAG, "detectSimChange : numSlots = " + numSlots);
        for (int i = 0; i < numSlots; i++) {
            newSimInserted = false;
            final SubscriptionInfo sir = SubscriptionManager.from(mContext)
                .getActiveSubscriptionInfoForSimSlotIndex(i);
            Log.d(TAG, "sir = " + sir + "for slot" + i);
            final String key = SLOT_PREFIX + i;
            final String lastSimImsi = getLastSimImsi(key);
            if (sir != null) {
                final int currentSubId = sir.getSubscriptionId();
                final String currentSimImsi = mTelephonyManager.getSubscriberId(currentSubId);
                Log.d(TAG, "lastSimImsi = " + lastSimImsi + " currentSimImsi = " + currentSimImsi);
                if (lastSimImsi.length() != 0) {
                    if (!lastSimImsi.equals(currentSimImsi)) {
                        setLastSimImsi(key, currentSimImsi);
                        newSimInserted = true;
                    }
                } else {
                    setLastSimImsi(key, currentSimImsi);
                    newSimInserted = true;
                }
            }
            if (i == 0) {
                mNewSim1Inserted = newSimInserted;
                Log.d(TAG, "detectSimChange : mNewSim1Inserted = " + mNewSim1Inserted);
            } else {
                mNewSim2Inserted = newSimInserted;
                Log.d(TAG, "detectSimChange : mNewSim2Inserted = " + mNewSim2Inserted);
            }
        }
    }

    private String getLastSimImsi(String strSlotId) {
        String imsi = "";
        try {
            imsi = mSharedPreferences.getString(strSlotId, "");
        } catch (ClassCastException e) {
            e.printStackTrace();
            mSharedPreferences.edit().remove(strSlotId).commit();
        }
        Log.d(TAG, "getLastSubId strSlotId = " + strSlotId + ", imsi = " + imsi);
        return imsi;
    }

    private void setLastSimImsi(String strSlotId, String value) {
        Log.d(TAG, "setLastSubId: strSlotId = " + strSlotId + ", value = " + value);
        Editor editor = mSharedPreferences.edit();
        editor.putString(strSlotId, value);
        editor.commit();
    }

    @Override
    public void onDestroyView() {
        super.onDestroyView();
        mSwitchBar.hide();
    }

    @Override
    public boolean onPreferenceChange(Preference preference, Object newValue) {
        final Context context = getActivity();
        boolean needUpdate = true;
        //int action;
        Log.d(TAG, "onPreferenceChange:" + "preference =" + preference + ", newValue: " + newValue);
        String newVal = (String) newValue;
        if (newVal.length() == 0)
            newVal = VALUE_NOT_SET;
        if (preference == mSim1Pref) {
            mSim1Pref.setSummary((String) newVal);
            // Saving Simpref value to shared preferences
            if (VALUE_NOT_SET.equals((String) newVal)) {
                setSimPrefValue(SIM1_PREF, (String) "");
            }
            else {
                setSimPrefValue(SIM1_PREF, (String) newVal);
            }

            //mSim1num = (String) newVal;
        } else if (preference ==  mSim2Pref) {
            mSim2Pref.setSummary((String) newVal);
            // Saving Simpref value to shared preferences
            if (VALUE_NOT_SET.equals((String) newVal)) {
                setSimPrefValue(SIM2_PREF, (String) "");
            }
            else {
                setSimPrefValue(SIM2_PREF, (String) newVal);
            }
            //mSim2num = (String) newVal;
        } else if (preference == mSmartCallFwdModePref) {
            int mode = Integer.parseInt((String) newValue);
            Log.d(TAG, "selected mode:" + mode);
            mPrevSelectedMode = mCurrSelectedMode;
            mCurrSelectedMode = mode;
            if (!mSwitchBar.isChecked()) {
                Log.d(TAG, "switch is off: don't update");
                needUpdate = false;
                mSmartCallFwdModePref.setValue(Integer.toString(mode));
                mSmartCallFwdModePref.setSummary(mSummary[mCurrSelectedMode - 1]);
            }
            setSmartCallFwdMode(mode, needUpdate);
        }
        return true;
    }

    private void setSmartCallFwdMode(int mode, boolean needUpdate) {
        int simId = SIM1;
        Log.d(TAG, "setSmartCallFwdMode:" + mode);
        if (mPrevSelectedMode == mCurrSelectedMode) {
            showToast("Already Set");
        } else {
            if (mode == SIM1_TO_SIM2) {
                //simId = SIM1;
                mCfInfoArr[SIM1].action = CommandsInterface.CF_ACTION_ENABLE;
                mCfInfoArr[SIM2].action = CommandsInterface.CF_ACTION_DISABLE;
            } else if (mode == SIM2_TO_SIM1) {
                //simId = SIM2;
                mCfInfoArr[SIM1].action = CommandsInterface.CF_ACTION_DISABLE;
                mCfInfoArr[SIM2].action = CommandsInterface.CF_ACTION_ENABLE;
            } else {
                mCfInfoArr[SIM1].action = CommandsInterface.CF_ACTION_ENABLE;
                mCfInfoArr[SIM2].action = CommandsInterface.CF_ACTION_ENABLE;
            }
            //if (mPrevSelectedMode == DUAL_SIM) {
                //simId = mCurrSelectedMode == SIM1_TO_SIM2 ? SIM2 : SIM1;
            //}
            if (needUpdate) {
                setCallForwardStatus(simId, mCfInfoArr[simId].action);
                showProgressDialog(getResources().getString(R.string.progress_dlg_writing));
            }
        }
        //setSmartCallFwdProperty(true);
    }

    /**
     * Listens to the state change of the switch.
     */
    @Override
    public void onSwitchChanged(Switch switchView, boolean isChecked) {
        Log.d(TAG, "OnSwitchChanged: " + isChecked);
        if (!isPhoneNumberSet()) {
            showToast("Set phone numbers for both SIMs first");
            if (isChecked) {
                mSwitchBar.setChecked(false);
            }
            return;
        }
        setSmartCallFwdProperty(isChecked);
        mPrevSwitchState = !isChecked;
        if (mFlag) {
            mFlag = false;
            Log.d(TAG, "Not triggered from user");
            return;
        }

        mSwitchBar.setChecked(isChecked);
        if (mSim1num == null) {
            mSim1Pref.setEnabled(!isChecked);
        }
        if (mSim2num == null) {
            mSim2Pref.setEnabled(!isChecked);
        }
        mSmartCallFwdModePref.setEnabled(true);
        if (!isChecked) {
            disableSmartCallForward();
        } else {
            enableSmartCallForward();
        }
    }

    private void setSmartCallFwdProperty(boolean switchActive) {
        int prop = 0;
        if (switchActive) {
            prop = mCurrSelectedMode;
        }
        Log.d(TAG, "SetSystem property to " + prop);
        android.provider.Settings.System.putInt(mContext.getContentResolver(), SMARTCALLMODE, prop);
    }

    private boolean isPhoneNumberSet() {
        if (VALUE_NOT_SET.equals(mSim1Pref.getText()) ||
                    VALUE_NOT_SET.equals(mSim2Pref.getText()) ||
                    mSim1Pref.getText() == null ||
                    mSim2Pref.getText() == null ||
                    (mSim1Pref.getText() != null && mSim1Pref.getText().length() == 0) ||
                    (mSim2Pref.getText() != null && mSim2Pref.getText().length() == 0)) {
            Log.d(TAG, "Phone number/ numbers not present");
            return false;
        }
        Log.d(TAG, "Phone number/ numbers present");
        return true;
    }

    private void disableSmartCallForward() {
        int simId = SIM1;
        Log.d(TAG, "disableSmartCallForward for mCurrSelectedMode: " + mCurrSelectedMode);
        if (mCurrSelectedMode == SIM1_TO_SIM2) {
            mCfInfoArr[SIM1].action = CommandsInterface.CF_ACTION_DISABLE;
        } else if (mCurrSelectedMode == SIM2_TO_SIM1) {
            mCfInfoArr[SIM2].action = CommandsInterface.CF_ACTION_DISABLE;
            simId = SIM2;
        } else {
            mCfInfoArr[SIM1].action = CommandsInterface.CF_ACTION_DISABLE;
            mCfInfoArr[SIM2].action = CommandsInterface.CF_ACTION_DISABLE;
        }
        setCallForwardStatus(simId, mCfInfoArr[simId].action);
        showProgressDialog(getResources().getString(R.string.progress_dlg_writing));
    }

    private void enableSmartCallForward() {
        int simId = SIM1;
        Log.d(TAG, "enableSmartCallForward for mCurrSelectedMode: " + mCurrSelectedMode);
        if (mCurrSelectedMode == SIM1_TO_SIM2) {
            mCfInfoArr[SIM1].action = CommandsInterface.CF_ACTION_ENABLE;
        } else if (mCurrSelectedMode == SIM2_TO_SIM1) {
            mCfInfoArr[SIM2].action = CommandsInterface.CF_ACTION_ENABLE;
            simId = SIM2;
        } else {
            mCfInfoArr[SIM1].action = CommandsInterface.CF_ACTION_ENABLE;
            mCfInfoArr[SIM2].action = CommandsInterface.CF_ACTION_ENABLE;
        }

        setCallForwardStatus(simId, mCfInfoArr[simId].action);
        showProgressDialog(getResources().getString(R.string.progress_dlg_writing));
    }

    /** Messenger for communicating with the service. */
    Messenger mService = null;

    /** Flag indicating whether we have called bind on the service. */
    boolean mBound;

    /**
     * Class for interacting with the main interface of the service.
     */
    private ServiceConnection mConnection = new ServiceConnection() {
        public void onServiceConnected(ComponentName className, IBinder service) {
            // This is called when the connection with the service has been
            // established, giving us the object we can use to
            // interact with the service.  We are communicating with the
            // service using a Messenger, so here we get a client-side
            // representation of that from the raw IBinder object.
            mService = new Messenger(service);
            mBound = true;
            // Register our messenger also on Service side:
            Message msg = Message.obtain(null, MSG_TYPE_REGISTER);
            msg.replyTo = mMessenger;
            try {
                mService.send(msg);
            } catch (RemoteException re) {
                re.printStackTrace();
            }
        }

        public void onServiceDisconnected(ComponentName className) {
            // This is called when the connection with the service has been
            // unexpectedly disconnected -- that is, its process crashed.
            mService = null;
            mBound = false;
        }
    };

        private void getCallForwardStatus(int simId) {
            if (mService == null) {
                Log.d(TAG, "service not started yet");
                return;
            }
            Log.d(TAG, "getCallForwardStatus: " + simId);
            mReadProgress = true;
            Message msg = Message.obtain(null, MSG_GET_CF_REQ);
            Bundle b = new Bundle();
            b.putInt("simId", simId);
            b.putInt("act", READ);
            msg.setData(b);
            try {
                mService.send(msg);
            } catch (RemoteException re) {
                re.printStackTrace();
            }
        }

        private void setCallForwardStatus(int simId, int action) {
            String currPhnum = null;
            if (mService == null) {
                Log.d(TAG, "service not started yet");
                return;
            }
            currPhnum = (simId == SIM1) ? mSim2Pref.getText() : mSim1Pref.getText();
            Log.d(TAG, "setCallForward to " + currPhnum);
            mWriteProgress = true;
            Message msg = Message.obtain(null, MSG_SET_CF_REQ);
            Bundle b = new Bundle();
            b.putInt("simId", simId);
            b.putInt("act", WRITE);
            b.putString("phnum", currPhnum);
            b.putInt("action", action);
            msg.setData(b);
            try {
                mService.send(msg);
            } catch (RemoteException re) {
                re.printStackTrace();
            }
        }

    /**
     * Handler of incoming messages from service.
     */
    class IncomingHandler extends Handler {
        @Override
        public void handleMessage(Message msg) {
            Bundle b = new Bundle();
            b = msg.getData();
            switch (msg.what) {
                case MSG_TYPE_DATA:
                    break;
                case MSG_GET_CF_RES:
                    handleGetCfResp(b);
                    break;
                case MSG_SET_CF_RES:
                    handleSetCfResp(b);
                    break;
                case MSG_TYPE_REGISTER:
                    Log.d(TAG, "Service has started");
                    getCallForwardStatus(SIM1);
                    break;
                default:
                    super.handleMessage(msg);
            }
        }

        private void handleSetCfResp(Bundle b) {
            Log.d(TAG, "handleSetCfResp: " + b.getInt("simId"));
            Log.d(TAG, "status: " + b.getInt("status"));
            Log.d(TAG, "reason: " + b.getInt("reason"));
            Log.d(TAG, "phnum: " + b.getString("phnum"));
            Log.d(TAG, "error: " + b.getInt("err"));
            Log.d(TAG, "action: " + b.getInt("action"));
            Log.d(TAG, "callwait: " + b.getInt("callwait"));
            Log.d(TAG, "mCurrSelectedMode: " + mCurrSelectedMode);
            int err = b.getInt("err");
            int sim = b.getInt("simId");
            mCfInfoArr[sim].status = b.getInt("status");
            mCfInfoArr[sim].phnum = b.getString("phnum");
            mCfInfoArr[sim].callwait = b.getInt("callwait");
            mCfInfoArr[sim].error = err;
            if (err == -1) {
                Log.d(TAG, "Network error" + (sim + 1));
            } else if (err > 0) {
                Log.d(TAG, "set cf failed on sim" + (sim + 1));
            } else {
                Log.d(TAG, "set cf success on sim" + (sim + 1));
            }
            if (sim == SIM1) {
                setCallForwardStatus(SIM2, mCfInfoArr[SIM2].action);
            } else {
                updateSetCfStatus();
            }
        }

        private void updateSetCfStatus() {
            // if CF set success on both SIMs
            String statusMsg = "";
            //int failMode = 0;
            int successMode = 0;

            // Status of SIM1
            statusMsg += "[SIM1]:\n";
            if (mCfInfoArr[SIM1].error == -1) {
                statusMsg += getResources().getString(R.string.network_error);
            } else if (mCfInfoArr[SIM1].error > 0) {
                statusMsg += "\nCallForward: " + mActionString[mCfInfoArr[SIM1].action] +
                " failed.\n";
            } else {
                if (mCfInfoArr[SIM1].action == CommandsInterface.CF_ACTION_DISABLE) {
                    statusMsg += "CallForward: disabled.\n";
                } else {
                    statusMsg += "CallForward: enabled.\nIf SIM1 unreachable," +
                            "incoming calls will be forwarded to " + mSim2Pref.getText();
                }
                statusMsg += "\nCallWaiting: " +
                        (mCfInfoArr[SIM1].callwait == 1 ? "enabled" : "disabled") + " on SIM2";
                if (mCurrSelectedMode != SIM2_TO_SIM1) {
                    successMode |= SIM1_TO_SIM2;
                }
            }
            // Status of SIM2
            statusMsg += "\n\n[SIM2]:\n";
            if (mCfInfoArr[SIM2].error == -1) {
                statusMsg += getResources().getString(R.string.network_error);
            } else if (mCfInfoArr[SIM2].error > 0) {
                statusMsg += "CallForward: " + mActionString[mCfInfoArr[SIM2].action] +
                " failed.\n";
            } else {
                if (mCfInfoArr[SIM2].action == CommandsInterface.CF_ACTION_DISABLE) {
                    statusMsg += "CallForward: disabled.\n";
                } else {
                    statusMsg += "CallForward: enabled.\nIf SIM2 unreachable," +
                            "incoming calls will be forwarded to " + mSim1Pref.getText();
                }
                statusMsg += "\nCallWaiting: " +
                        (mCfInfoArr[SIM2].callwait == 1 ? "enabled" : "disabled") + " on SIM1";
                if (mCurrSelectedMode != SIM1_TO_SIM2) {
                    successMode |= SIM2_TO_SIM1;
                }
            }
            Log.d(TAG, "successMode:" + successMode);
            Log.d(TAG, "mPrevSelectedMode:" + mPrevSelectedMode);
            if (successMode > 0) {
                mSmartCallFwdModePref.setValue(Integer.toString(successMode));
                mSmartCallFwdModePref.setSummary(mSummary[successMode - 1]);
            } else {
                if (!mPrevSwitchState) {
                    mFlag = true;
                    if (mSim1num == null) {
                        mSim1Pref.setEnabled(true);
                    }
                    if (mSim2num == null) {
                        mSim2Pref.setEnabled(true);
                    }
                    mSwitchBar.setChecked(mPrevSwitchState);
                }
                mCurrSelectedMode = mPrevSelectedMode;
                mSmartCallFwdModePref.setValue(Integer.toString(mCurrSelectedMode));
                mSmartCallFwdModePref.setSummary(mSummary[mCurrSelectedMode - 1]);
            }
            if (mProgressDialog != null) {
                Log.d(TAG, "Updating complete:");
                mProgressDialog.dismiss();
                mProgressDialog = null;
            }
            Log.d(TAG, "statusMsg: " + statusMsg);
            String title = getResources().getString(R.string.progress_dlg_title);
            showAlertDialog(title, statusMsg);
        }

        private void handleGetCfResp(Bundle b) {
            Log.d(TAG, "handleGetCfResp: " + b.getInt("simId"));
            Log.d(TAG, "status" + b.getInt("status"));
            Log.d(TAG, "reason" + b.getInt("reason"));
            Log.d(TAG, "phnum" + b.getString("phnum"));

            int sim = b.getInt("simId");
            if (mCfInfoArr[sim] != null) {
                mCfInfoArr[sim].status = b.getInt("status");
                mCfInfoArr[sim].reason = b.getInt("reason");
                mCfInfoArr[sim].phnum = b.getString("phnum");
                mCfInfoArr[sim].callwait = b.getInt("callwait");
                mCfInfoArr[sim].error = b.getInt("err");
            }
            if (sim == SIM1) {
                getCallForwardStatus(SIM2);
            } else if (sim == SIM2) {
                updateGetCfStatus();
            }
        }
    }

    private void updateGetCfStatus() {
        String sim1Num = mSim1Pref.getText();
        String sim2Num = mSim2Pref.getText();
        String statusMsg = "";
        int cfStatus = 0;
        Log.d(TAG, "updateGetCfStatus");
        Log.d(TAG, "sim1Num: " + sim1Num + ", sim2Num: " + sim2Num);
        Log.d(TAG, "mCfInfoArr[SIM1] phnum:" + mCfInfoArr[SIM1].phnum);
        Log.d(TAG, "mCfInfoArr[SIM2] phnum:" + mCfInfoArr[SIM2].phnum);
        Log.d(TAG, "mCfInfoArr[SIM1] callwait:" + mCfInfoArr[SIM1].callwait);
        Log.d(TAG, "mCfInfoArr[SIM2] callwait:" + mCfInfoArr[SIM2].callwait);

        if (mCfInfoArr[SIM1].error == -1 || mCfInfoArr[SIM2].error == -1) {
            mSwitchBar.setEnabled(true);
            if (mProgressDialog != null) {
                mProgressDialog.dismiss();
                mProgressDialog = null;
            }
            statusMsg += getResources().getString(R.string.network_error);
            showToast(statusMsg);
        } else {
            if (mCfInfoArr[SIM1].status == 1 && (mCfInfoArr[SIM1].callwait == 1) &&
                        PhoneNumberUtils.compareLoosely(sim2Num, mCfInfoArr[SIM1].phnum)) {
                cfStatus |= SIM1_TO_SIM2;
            }
            if (mCfInfoArr[SIM2].status == 1 && (mCfInfoArr[SIM2].callwait == 1) &&
                        PhoneNumberUtils.compareLoosely(sim1Num, mCfInfoArr[SIM2].phnum)) {
                cfStatus |= SIM2_TO_SIM1;
            }

            if (cfStatus == SIM1_TO_SIM2) {
                mCfInfoArr[SIM2].action = CommandsInterface.CF_ACTION_DISABLE;
                mCurrSelectedMode = SIM1_TO_SIM2;
            } else if (cfStatus == SIM2_TO_SIM1) {
                mCfInfoArr[SIM1].action = CommandsInterface.CF_ACTION_DISABLE;
                mCurrSelectedMode = SIM2_TO_SIM1;
            } else {
                mCurrSelectedMode = DUAL_SIM;
            }
            statusMsg += "[SIM1]:\n CallForwarding: " +
            ((cfStatus & SIM1_TO_SIM2) == 1 ? "enabled" : "disabled");
            statusMsg += "\n CallWaiting: " +
            (mCfInfoArr[SIM1].callwait == 1 ? "enabled" : "disabled");
            statusMsg += "\n\n[SIM2]:\n CallForwarding: " +
            ((cfStatus & SIM2_TO_SIM1) == 2 ? "enabled" : "disabled");
            statusMsg += "\n CallWaiting: " +
            (mCfInfoArr[SIM2].callwait == 1 ? "enabled" : "disabled");
         }
        updatePreference(cfStatus);
        Log.d(TAG, "cfStatus:" + cfStatus);
        Log.d(TAG, "statusMsg: " + statusMsg);
        if (mProgressDialog != null) {
            Log.d(TAG, "Reading complete:");
            mProgressDialog.dismiss();
            mProgressDialog = null;
        }
        //String title = getResources().getString(R.string.progress_dlg_title);
        //showAlertDialog(title, statusMsg);
    }

    private void startService() {
        // Bind to the service
        Intent eIntent = new Intent();
        eIntent.setClassName("com.mediatek.op18.settings",
                             "com.mediatek.op18.settings.SmartCallFwdService");
        if (eIntent != null) {
            mContext.bindService(eIntent, mConnection, Context.BIND_AUTO_CREATE);
        } else {
            Log.d(TAG, "null explicit intent");
        }
    }

    private void stopService() {
        if (mBound) {
            Message msg = Message.obtain(null, MSG_TYPE_DEREGISTER);
            try {
                mService.send(msg);
            } catch (RemoteException re) {
                re.printStackTrace();
            }
            mMessenger = null;
            getActivity().getApplicationContext().unbindService(mConnection);
        }
    }

    class CFInfo {
        public int             simId;       /* 0 = SIM1, 1 = SIM2 */
        public int             status;      /*1 = active, 0 = not active */
        public int             reason;      /* from TS 27.007 7.11 "reason" */
        public String          phnum;      /* "number" from TS 27.007 7.11 */
        public int             error; /* for CF no reply only */
        public int             action; /* for CF action */
        public int             callwait;

        public CFInfo() {
            this.simId = 0;
            this.status = 0;
            this.reason = CommandsInterface.CF_REASON_NOT_REACHABLE;
            this.phnum = "unknown";
            this.error = -1;
            this.action = CommandsInterface.CF_ACTION_ENABLE;
            this.callwait = 0;
        }
    }

    /**
     * Show the alert dialog when get status failed
     */

    private void showAlertDialog(String title, String message) {
        Log.d(TAG, "showAlertDialog");
        AlertDialog.Builder bld = new AlertDialog.Builder(getActivity());
        bld.setTitle(title);
        bld.setMessage(message);
        bld.setPositiveButton(android.R.string.ok, new DialogInterface.OnClickListener() {
            @Override
            public void onClick(DialogInterface dialog, int which) {
                if (mProgressDialog != null) {
                    Log.d(TAG, "Clear unwanted progress dialog");
                    mProgressDialog.dismiss();
                    mProgressDialog = null;
                }
            }
        });
        //mAlertDialog = bld.create();
        bld.setCancelable(false);
        mAlertDialog = bld.show();
    }

    private void showToast(String s) {
    Toast toast = Toast.makeText(mContext, s, Toast.LENGTH_LONG);
    toast.show();
    }

    /**
    * Getting SimPref value from Shared preference
    * @param simPref: key for Preference
    * @return Preference value
    */
    private String getSimPrefValue(String simPref) {
        String value = "";
        try {
            value = mSharedPreferences.getString(simPref, "");
        } catch (ClassCastException e) {
            e.printStackTrace();
            mSharedPreferences.edit().remove(simPref).commit();
        }
        Log.d(TAG, "getSimPrefValue simPref = " + simPref + ", value = " + value);
        return value;
    }


    /**
    * Set SimPref value in Shared preference
    * @param simPref : key for Preference
       * @param value : Preference value to add
    */
    private void setSimPrefValue(String simPref, String value) {
        Log.d(TAG, "setSimPrefValue: simPref = " + simPref + ", value = " + value);
        Editor editor = mSharedPreferences.edit();
        editor.putString(simPref, value);
        editor.commit();
    }


    /**
    * get both SimPref values from Shared preference
    */
    private void getPreviousPrefValue() {
        Log.d(TAG, "getPreviousPrefValues");
        simPrefValue[SIM1] = getSimPrefValue(SIM1_PREF);
        simPrefValue[SIM2] = getSimPrefValue(SIM2_PREF);
    }

    public String getLine1Number(int simId) {
        int subId = MtkSubscriptionManager.getSubIdUsingPhoneId(simId);
        Log.d(TAG, "getLine1Number with simId " + simId + " ,subId " + subId);
        return mTelephonyManager.getLine1Number(subId);
    }
}
