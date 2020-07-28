package com.mediatek.op08.settings.mulitine;

import android.app.ActionBar;
import android.app.AlertDialog;
import android.app.ProgressDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.os.SystemProperties;
import android.preference.Preference;
import android.preference.PreferenceActivity;
import android.preference.PreferenceGroup;
import android.telephony.PhoneStateListener;
import android.telephony.ServiceState;
import android.telephony.TelephonyManager;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.view.View;
import android.widget.CompoundButton;
import android.widget.CompoundButton.OnCheckedChangeListener;
import android.widget.EditText;
import android.widget.Switch;
import android.widget.Toast;

import com.mediatek.digits.DigitsConst;
import com.mediatek.digits.DigitsDevice;
import com.mediatek.digits.DigitsLine;
import com.mediatek.digits.DigitsManager;
import com.mediatek.digits.DigitsProfile;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;

/**
 * Activity to show settings for multi line.
 */
public class MultiLineSettingsActivity extends PreferenceActivity {

    private static final String TAG = "MultiLineSettingsActivity";
    private static final int GET_NUMBER = 1;
    private static final String LIST_NUMBERS_KEY = "list_numbers_key";
    public static final String EXTRA_MESSAGE = "com.multilinesettings.mtk33170.url";
    private static final boolean DBG =
        SystemProperties.get("ro.build.type").equals("eng") ? true : false;

    //Toast message token id
    private static final int TOAST_ACTIVATE_LINE_SUCCESS = 1;
    private static final int TOAST_ACTIVATE_LINE_FAIL = 2;
    private static final int TOAST_DEACTIVATE_LINE_SUCCESS = 3;
    private static final int TOAST_DEACTIVATE_LINE_FAIL = 4;
    private static final int TOAST_LOGOUT_SUCCESS = 5;
    private static final int TOAST_LOGOUT_FAIL = 6;
    private static final int TOAST_ACTIVATE_ALL_LINE_SUCCESS = 7;
    private static final int TOAST_ACTIVATE_ALL_LINE_FAIL = 8;
    private static final int TOAST_NUMBER_ADDED_SUCCESS = 9;
    private static final int TOAST_NUMBER_ADDED_FAIL = 10;
    private static final int TOAST_DEVICE_NAME_CHANGE_SUCCESS = 11;
    private static final int TOAST_DEVICE_NAME_CHANGE_FAIL = 12;
    private static final int TOAST_LINE_NAME_CHANGE_SUCCESS = 13;
    private static final int TOAST_LINE_NAME_CHANGE_FAIL = 14;
    private static final int TOAST_NO_LINE_TOACTIVATE = 15;
    private static final int TOAST_MYNUMBER_NOT_ALLOWED_AS_VIRTUAL = 16;
    private static final int TOAST_NUMBER_ALREADY_ADDED = 17;
    private static final int TOAST_DEACTIVATE_ALL_LINE_SUCCESS = 18;
    private static final int TOAST_DEACTIVATE_ALL_LINE_FAIL = 19;
    //map of network controls to the network data.
    private HashMap<RadioPowerPreference, String> mNumbersMap;
    //preference objects
    private PreferenceGroup mNumbersList;
    private static Context sContext;
    private  Context mContext;
    private DigitsManager mManager;
    private DigitsProfile mProfile;
    private DigitsLine[] mLine;
    private DigitsDevice[] mDevice;
    private String mDeviceName;
    private String mMyLineName;
    private String mMyMsisdn;
    private int mState;
    private ProgressDialog mPd = null;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        sContext = getApplicationContext();
        //kgetWindow().requestFeature(Window.FEATURE_ACTION_BAR);
        //setContentView(R.layout.activity_multi_line_settings);
        addPreferencesFromResource(R.xml.multiline_settings_preference);

        mContext = this;
        mManager = new DigitsManager(this);
        mManager.registerEventListener(mListener);
        ((TelephonyManager) mContext.getSystemService(Context.TELEPHONY_SERVICE)).listen(
                        mPhoneStateListener, PhoneStateListener.LISTEN_CALL_STATE |
                        PhoneStateListener.LISTEN_SERVICE_STATE);
        showProgressDialog();

    }


    @Override
    protected void onResume() {
         super.onResume();
         TelephonyManager tm =
                (TelephonyManager) mContext.getSystemService(Context.TELEPHONY_SERVICE);
        int state = tm.getSimState();
        if (state == TelephonyManager.SIM_STATE_ABSENT ||
            state == TelephonyManager.SIM_STATE_NOT_READY) {
            if ((mPd != null) && mPd.isShowing()) {
                mPd.dismiss();
            }
            finish();
        }
    }

    private PhoneStateListener mPhoneStateListener = new PhoneStateListener() {
        @Override
        public void onCallStateChanged(int state, String incomingNumber) {
            Log.d(TAG, "Phone state:" + state);
            switch (state) {
            case TelephonyManager.CALL_STATE_IDLE:
                Log.d(TAG, "No call switch enable");
                getPreferenceScreen().setEnabled(true);
                break;
            case TelephonyManager.CALL_STATE_OFFHOOK:
            case TelephonyManager.CALL_STATE_RINGING:
                Log.d(TAG, "call Exist switch disable");
                getPreferenceScreen().setEnabled(false);
                break;
            default:
                Log.d(TAG, "default");
                getPreferenceScreen().setEnabled(false);
                break;
            }
        }

        @Override
        public void onServiceStateChanged(ServiceState serviceState) {
            final TelephonyManager tm =
                (TelephonyManager) mContext.getSystemService(Context.TELEPHONY_SERVICE);
            int state = tm.getSimState();
            if (state == TelephonyManager.SIM_STATE_ABSENT ||
                state == TelephonyManager.SIM_STATE_NOT_READY) {
                if ((mPd != null) && mPd.isShowing()) {
                    mPd.dismiss();
                }
                finish();
            }
        }
    };


    /*Preference.OnPreferenceChangeListener mMultiLineChangeListener =
                    new Preference.OnPreferenceChangeListener() {
        public boolean onPreferenceChange(Preference preference, Object objValue) {
            // Code goes here
            if (((LinePreferenceSwitch) preference).isChecked() == (Boolean) objValue) {
            // State got changed
                Log.i(TAG, preference.getKey() + " : " + String.valueOf(objValue) +
                        " not changed");
                return false;
            // return false if you don't want to save the preference change.
            }
            String msisdn = preference.getSummary().toString();
            showProgressDialog();
            boolean checked = (Boolean) objValue;
            Log.d(TAG, "msisdn: " + msisdn + "checked: " + checked);
            if (checked) {
                mManager.activateLine(msisdn, mManager.new ActionListener() {
                    @Override
                    public void onSuccess(Bundle extras) {
                        runOnUiThread(new Runnable() {
                            public void run() {
                                if ((mPd != null) && mPd.isShowing()) {
                                    mPd.dismiss();
                                }
                                mHandler.sendMessage(mHandler.obtainMessage(
                                        TOAST_ACTIVATE_LINE_SUCCESS, 0));
                            }
                        });
                        return;
                    }
                    @Override
                    public void onFailure(final int reason, Bundle extras) {
                        runOnUiThread(new Runnable() {
                            public void run() {
                                if ((mPd != null) && mPd.isShowing()) {
                                    mPd.dismiss();
                                }
                                ((LinePreferenceSwitch) preference).setChecked(!checked);
                                mHandler.sendMessage(mHandler.obtainMessage(
                                        TOAST_ACTIVATE_LINE_FAIL, 0));
                            }
                        });
                        return;
                    }
                });
            } else {
                mManager.deactivateLine(msisdn, mManager.new ActionListener() {
                    @Override
                    public void onSuccess(Bundle extras) {
                        runOnUiThread(new Runnable() {
                            public void run() {
                                if ((mPd != null) && mPd.isShowing()) {
                                    mPd.dismiss();
                                }
                                mHandler.sendMessage(mHandler.obtainMessage(
                                        TOAST_DEACTIVATE_LINE_SUCCESS, 0));
                            }
                        });
                        return;
                    }
                    @Override
                    public void onFailure(final int reason, Bundle extras) {
                        runOnUiThread(new Runnable() {
                            public void run() {
                                if ((mPd != null) && mPd.isShowing()) {
                                    mPd.dismiss();
                                }
                                ((LinePreferenceSwitch) preference).setChecked(!checked);
                                mHandler.sendMessage(mHandler.obtainMessage(
                                        TOAST_DEACTIVATE_LINE_FAIL, 0));
                            }
                        });
                        return;
                    }
                });
            }
            return true;
        }
    };*/

    private void showProgressDialog() {
        // TODO Auto-generated method stub
        Log.d(TAG, "showProgressDialog");
        if (((PreferenceActivity) mContext).isFinishing()) {
            //show dialog
            return;
        }
        mPd = new ProgressDialog(this);
        mPd.setMessage(mContext.getText(R.string.wait_message));
        mPd.setCancelable(true);
        mPd.setCanceledOnTouchOutside(false);
        mPd.setOnDismissListener(new DialogInterface.OnDismissListener() {
            @Override
            public void onDismiss(DialogInterface dialog) {
                if (mPd != null) {
                    mPd = null;
                }
            }
        });
        mPd.setIndeterminate(true);
        mPd.show();
    }

    private DigitsManager.EventListener mListener = new DigitsManager.EventListener() {
        @Override
        public void onEvent(int event, int result, Bundle extras) {
            Log.i(TAG, "mListener onEvent(), event:" + DigitsConst.eventToString(event) +
                    ", result:" + result + ", extras:" + extras);
            int state = mManager.getServiceState();
            switch (event) {
                case DigitsConst.EVENT_SERVICE_STATE_CHANGED:
                    mState = mManager.getServiceState();
                    break;
                case DigitsConst.EVENT_SERVICE_CONNECTION_RESULT:
                    if (result == DigitsConst.RESULT_SUCCEED) {
                        if (state == DigitsConst.STATE_SUBSCRIBED) {
                            updateSettingsActivity();
                        }
                    }
                    break;
                case DigitsConst.EVENT_REGISTERED_DEVICES_CHANGED:
                case DigitsConst.EVENT_REGISTERED_MSISDN_CHANGED:
                    if (result == DigitsConst.RESULT_SUCCEED) {
                        refereshLines();
                    }
                    break;
                default:
                    Log.i(TAG, "Event not listed");
            }
        }
    };

    /**
     * Update the Settings only once the activity is connected to Subscribed DigitsManager.
     */
    public void updateSettingsActivity() {
        if ((mPd != null) && mPd.isShowing()) {
            mPd.dismiss();
        }
        mProfile = mManager.getDigitsProfile();
        mState = mManager.getServiceState();
        mLine = mManager.getRegisteredLine();
        mDevice = mManager.getRegisteredDevice();
        mNumbersList = (PreferenceGroup) getPreferenceScreen().findPreference("other_lines");
        mNumbersList.setTitle(R.string.other_lines);
        ActionBar actionBar = getActionBar();
        actionBar.setSubtitle(mProfile.getEmail());
        if (actionBar != null) {
            // android.R.id.home will be triggered in
            // onOptionsItemSelected()
            actionBar.setDisplayHomeAsUpEnabled(true);
        }
        if (mLine == null) {
            Log.d(TAG, "No line available currently");
            return;
        }
        for (DigitsLine currLine: mLine) {
            if (currLine.getIsVirtual()) {
                RadioPowerPreference numberPref = new RadioPowerPreference(this);
                numberPref.setTitle(currLine.getLineName());
                numberPref.setSummary(currLine.getMsisdn());
                boolean isChecked = false;

                if (currLine.getLineStatus() == DigitsLine.LINE_STATUS_REGISTERED ||
                    currLine.getLineStatus() == DigitsLine.LINE_STATUS_ACTIVATED) {
                    isChecked =  true;
                }
                numberPref.setPowerState(isChecked);
                numberPref.setPersistent(false);
                mNumbersList.addPreference(numberPref);
                if (isChecked) {
                    numberPref.setOnPreferenceClickListener(mPreferenceClickListener);
                }
                Log.i(TAG, "MSISDN: " + currLine.getMsisdn() + "Status: " + currLine.getLineStatus()
                        + "isChecked: " + isChecked);
            } else {
                Preference pref = getPreferenceScreen().findPreference("primary_profile");
                mMyLineName = currLine.getLineName();
                mMyMsisdn = currLine.getMsisdn();
                pref.setTitle(currLine.getLineName());
                pref.setSummary(currLine.getMsisdn());
                pref.setOnPreferenceClickListener(new Preference.OnPreferenceClickListener() {
                    @Override
                    public boolean onPreferenceClick(Preference preference) {
                        LayoutInflater editlineLi = LayoutInflater.from(
                                MultiLineSettingsActivity.this);
                        View editLineView = editlineLi.inflate(R.layout.edit_line_layout, null);

                        AlertDialog.Builder editLineAlertDialogBuilder = new AlertDialog.Builder(
                                MultiLineSettingsActivity.this);

                        // set prompts.xml to alertdialog builder
                        editLineAlertDialogBuilder.setView(editLineView);

                        final EditText lineInput = (EditText) editLineView
                                .findViewById(R.id.editTextEditLine);
                        lineInput.setText(mMyLineName);

                        // set dialog message
                        editLineAlertDialogBuilder
                                .setCancelable(false)
                                .setPositiveButton("OK",
                                        new DialogInterface.OnClickListener() {
                                            public void onClick(DialogInterface dialog, int id) {
                                                // get user input and set it to result
                                                // edit text
                                                String lineInputString =
                                                        lineInput.getText().toString();
                                                updatedLineName(lineInputString);
                                            }
                                        })
                                .setNegativeButton("Cancel",
                                        new DialogInterface.OnClickListener() {
                                            public void onClick(DialogInterface dialog, int id) {
                                                dialog.cancel();
                                            }
                                        });

                        // create alert dialog
                        AlertDialog editLinealertDialog = editLineAlertDialogBuilder.create();

                        // show it
                        editLinealertDialog.show();
                        return true;
                    }
                });
            }
        }
        for (DigitsDevice device : mDevice) {
            if (device.getIsMy()) {
                mDeviceName = device.getDeviceName();
            }
        }
    }

    Preference.OnPreferenceClickListener mPreferenceClickListener  =
            new Preference.OnPreferenceClickListener() {
        @Override
        public boolean onPreferenceClick(Preference preference) {
            LayoutInflater editlineLayout = LayoutInflater.from(MultiLineSettingsActivity.this);
            View promptsView = editlineLayout.inflate(R.layout.rename_device_dialog, null);

            AlertDialog.Builder alertDialogBuilder = new AlertDialog.Builder(
                    MultiLineSettingsActivity.this);

            // set prompts.xml to alertdialog builder
            alertDialogBuilder.setView(promptsView);
            String str = preference.getTitle().toString();

            final EditText userInput = (EditText) promptsView
                    .findViewById(R.id.editTextRenameDevice);
            userInput.setText(str);

            // set dialog message
            alertDialogBuilder
                    .setCancelable(false)
                    .setPositiveButton("OK",
                            new DialogInterface.OnClickListener() {
                                public void onClick(DialogInterface dialog, int id) {
                                    String name = userInput.getText().toString();
                                    String msisdn = preference.getSummary().toString();
                                    showProgressDialog();
                                    updatedLineNameForId(((RadioPowerPreference) preference),
                                            msisdn, name);
                                }
                            })
                    .setNegativeButton("Cancel",
                            new DialogInterface.OnClickListener() {
                                public void onClick(DialogInterface dialog, int id) {
                                    dialog.cancel();
                                }
                            });

            // create alert dialog
            AlertDialog alertDialog = alertDialogBuilder.create();

            // show it
            alertDialog.show();
            return true;
        }
    };

    @Override
    public void onDestroy() {
        mPd = null;
        ((TelephonyManager) mContext.getSystemService(Context.TELEPHONY_SERVICE)).listen(
                mPhoneStateListener, PhoneStateListener.LISTEN_NONE);
        mManager.unregisterEventListener(mListener);

        mManager.shutdown();
        if ((mPd != null) && mPd.isShowing()) {
            mPd.dismiss();
            mPd = null;
        }
        super.onDestroy();
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        MenuInflater inflater = getMenuInflater();
        inflater.inflate(R.menu.multiline_settings_menu, menu);

        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        // Handle item selection
        switch (item.getItemId()) {
            case R.id.rename_device:

                LayoutInflater li = LayoutInflater.from(MultiLineSettingsActivity.this);
                View promptsView = li.inflate(R.layout.rename_device_dialog, null);

                AlertDialog.Builder alertDialogBuilder = new AlertDialog.Builder(
                        MultiLineSettingsActivity.this);

                // set prompts.xml to alertdialog builder
                alertDialogBuilder.setView(promptsView);
                String str = mDeviceName;

                final EditText userInput = (EditText) promptsView
                        .findViewById(R.id.editTextRenameDevice);
                userInput.setText(str);

                // set dialog message
                alertDialogBuilder
                        .setCancelable(false)
                        .setPositiveButton("OK",
                                new DialogInterface.OnClickListener() {
                                    public void onClick(DialogInterface dialog, int id) {
                                        // get user input and set it to result
                                        // edit text
                                        String updatedName = userInput.getText().toString();
                                        setDeviceName(updatedName);
                                    }
                                })
                        .setNegativeButton("Cancel",
                                new DialogInterface.OnClickListener() {
                                    public void onClick(DialogInterface dialog, int id) {
                                        dialog.cancel();
                                    }
                                });

                // create alert dialog
                AlertDialog alertDialog = alertDialogBuilder.create();

                // show it
                alertDialog.show();
                return true;
            /*case R.id.edit_line:

                return true;*/
            case R.id.activate_new:
                Intent activateIntent = new Intent(this, ActivateNew.class);
                startActivityForResult(activateIntent, GET_NUMBER);
                return true;
            case R.id.logout_menu:
                showProgressDialog();
                if (DBG) {
                    Log.i(TAG, "logout");
                }
                mManager.logout(mManager.new ActionListener() {
                    @Override
                    public void onSuccess(Bundle extras) {
                        if (DBG) {
                            Log.i(TAG, "logout onSuccess");
                        }
                        runOnUiThread(new Runnable() {
                            public void run() {
                                if ((mPd != null) && mPd.isShowing()) {
                                    mPd.dismiss();
                                }
                                mHandler.sendMessage(mHandler.obtainMessage(TOAST_LOGOUT_SUCCESS,
                                        0));
                                launchMultiLineActvity();
                            }
                        });
                        return;
                    }
                    @Override
                    public void onFailure(final int reason, Bundle extras) {
                        if (DBG) {
                            Log.i(TAG, "logout onFailure");
                        }
                        runOnUiThread(new Runnable() {
                            public void run() {
                                if ((mPd != null) && mPd.isShowing()) {
                                    mPd.dismiss();
                                }
                                mHandler.sendMessage(mHandler.obtainMessage(TOAST_LOGOUT_FAIL, 0));
                            }
                        });
                        return;
                    }
                });
                return true;
            case R.id.activate_all_menu:
                activateAllLine();
                return true;
            case R.id.deactivate_all_menu:
                deactivateAllLine();
                return true;
            /*case android.R.id.home:
                finish();
                return true;*/
            default:
                return super.onOptionsItemSelected(item);
        }
    }

    /**
     * Activate all line.
     */
    private void activateAllLine() {
        DigitsLine[] line = mManager.getRegisteredLine();
        ArrayList<String> msisdnList = new ArrayList<String>();
        if (line.length <= 1) {
            mHandler.sendMessage(mHandler.obtainMessage(TOAST_NO_LINE_TOACTIVATE, 0));
            return;
        }
        for (DigitsLine currLine: line) {
            if (currLine.getIsVirtual()) {

                String msisdn = currLine.getMsisdn();

                msisdnList.add(msisdn);
                Log.i(TAG, "MSISDN: " + currLine.getMsisdn() + " Status: " +
                        currLine.getLineStatus());
            }
        }
        String [] msisdnArray =  msisdnList.toArray(new String[msisdnList.size()]);
        if (msisdnArray.length == 0) {
            mHandler.sendMessage(mHandler.obtainMessage(TOAST_NO_LINE_TOACTIVATE, 0));
            return;
        }
        showProgressDialog();
        if (DBG) {
            Log.i(TAG, "activateAllLines");
        }
        mManager.activateMultiLines(msisdnArray, mManager.new ActionListener() {
            @Override
            public void onSuccess(Bundle extras) {
                if (DBG) {
                    Log.i(TAG, "activateAllLines success");
                }
                runOnUiThread(new Runnable() {
                    public void run() {
                        if ((mPd != null) && mPd.isShowing()) {
                            mPd.dismiss();
                        }
                        refereshLines();
                        mHandler.sendMessage(mHandler.obtainMessage(TOAST_ACTIVATE_ALL_LINE_SUCCESS,
                                0));
                    }
                });
                return;
            }
            @Override
            public void onFailure(final int reason, Bundle extras) {
                if (DBG) {
                    Log.i(TAG, "activateAllLines failed");
                }
                runOnUiThread(new Runnable() {
                    public void run() {
                        if ((mPd != null) && mPd.isShowing()) {
                            mPd.dismiss();
                        }
                        mHandler.sendMessage(mHandler.obtainMessage(TOAST_ACTIVATE_ALL_LINE_FAIL,
                                0));
                    }
                });
                return;
            }
        });
    }

    /**
     * Activate all line.
     */
    private void deactivateAllLine() {
        DigitsLine[] line = mManager.getRegisteredLine();
        ArrayList<String> msisdnList = new ArrayList<String>();
        if (line.length <= 1) {
            mHandler.sendMessage(mHandler.obtainMessage(TOAST_NO_LINE_TOACTIVATE, 0));
            return;
        }
        for (DigitsLine currLine: line) {
            if (currLine.getIsVirtual()) {

                String msisdn = currLine.getMsisdn();

                msisdnList.add(msisdn);
                Log.i(TAG, "MSISDN: " + currLine.getMsisdn() + " Status: " +
                        currLine.getLineStatus());
            }
        }
        String [] msisdnArray =  msisdnList.toArray(new String[msisdnList.size()]);
        if (msisdnArray.length == 0) {
            mHandler.sendMessage(mHandler.obtainMessage(TOAST_NO_LINE_TOACTIVATE, 0));
            return;
        }
        showProgressDialog();
        if (DBG) {
            Log.i(TAG, "deactivateAllLines");
        }
        mManager.deactivateMultiLines(msisdnArray, mManager.new ActionListener() {
            @Override
            public void onSuccess(Bundle extras) {
                if (DBG) {
                    Log.i(TAG, "deactivateAllLines success");
                }
                runOnUiThread(new Runnable() {
                    public void run() {
                        if ((mPd != null) && mPd.isShowing()) {
                            mPd.dismiss();
                        }
                        refereshLines();
                        mHandler.sendMessage(mHandler.obtainMessage(
                                TOAST_DEACTIVATE_ALL_LINE_SUCCESS, 0));
                    }
                });
                return;
            }
            @Override
            public void onFailure(final int reason, Bundle extras) {
                if (DBG) {
                    Log.i(TAG, "deactivateAllLines failed");
                }
                runOnUiThread(new Runnable() {
                    public void run() {
                        if ((mPd != null) && mPd.isShowing()) {
                            mPd.dismiss();
                        }
                        mHandler.sendMessage(mHandler.obtainMessage(
                                TOAST_DEACTIVATE_ALL_LINE_FAIL, 0));
                    }
                });
                return;
            }
        });
    }

    /**
     * After logout launch multi line activtity.
     */
    public void launchMultiLineActvity() {
        Intent requestUse = new Intent(this, MultiLineActivity.class);
        startActivity(requestUse);
    }

    @Override
    public void onActivityResult(int requestCode, int resultCode, Intent data) {
        // Check which request we're responding to
        if (requestCode == GET_NUMBER) {
            // Make sure the request was successful
            if (resultCode == RESULT_OK) {
                String msisdn = data.getStringExtra("MESSAGE");
                if (DBG) {
                    Log.i(TAG, "addNewLine " + msisdn);
                }
                if (msisdn.equals(mMyMsisdn)) {
                    mHandler.sendMessage(mHandler.obtainMessage(
                            TOAST_MYNUMBER_NOT_ALLOWED_AS_VIRTUAL, 0));
                    return;
                }
                DigitsLine[] line = mManager.getRegisteredLine();
                for (DigitsLine currLine: line) {
                    if (currLine.getIsVirtual()) {
                        String msisdnCurr = currLine.getMsisdn();
                        if (isLineAlreadyAdded(msisdnCurr, msisdn)) {
                            mHandler.sendMessage(mHandler.obtainMessage(
                                    TOAST_NUMBER_ALREADY_ADDED, 0));
                            return;
                        }
                    }
                }
                showProgressDialog();
                mManager.addNewLine(msisdn, mManager.new ActionListener() {
                    @Override
                    public void onSuccess(Bundle extras) {
                        if (DBG) {
                            Log.i(TAG, "addNewLine onSuccess");
                        }

                        runOnUiThread(new Runnable() {
                            public void run() {
                                if ((mPd != null) && mPd.isShowing()) {
                                    mPd.dismiss();
                                }
                                refereshLines();
                                mHandler.sendMessage(mHandler.obtainMessage(
                                        TOAST_NUMBER_ADDED_SUCCESS, 0));
                            }
                        });
                        return;
                    }
                    @Override
                    public void onFailure(final int reason, Bundle extras) {
                        if (DBG) {
                            Log.i(TAG, "addNewLine onSuccess");
                        }
                        runOnUiThread(new Runnable() {
                            public void run() {
                                if ((mPd != null) && mPd.isShowing()) {
                                    mPd.dismiss();
                                }
                                mHandler.sendMessage(mHandler.obtainMessage(
                                        TOAST_NUMBER_ADDED_FAIL, 0));
                            }
                        });
                        return;
                    }
                });
            }
        }
    }

    /**
     * Utility function to if line is available or not.
     * @param msisdnCurr msisdn already
     * @param msisdn msisdn which is newly added
     * @return true if already added previously
     */
    private boolean isLineAlreadyAdded(String msisdnCurr, String msisdn) {
        if (msisdnCurr.equals(msisdn)) {
            return true;
        }
        int currLen = msisdnCurr.length();
        if (currLen > 10) {
            msisdnCurr = msisdnCurr.substring(currLen - 10);
        }
        int msisdnLen = msisdn.length();
        if (msisdnLen > 10) {
            msisdn = msisdn.substring(msisdnLen - 10);
        }
        if (msisdnCurr.equals(msisdn)) {
            return true;
        }
        return false;
    }

    private void refereshLines() {
        if (DBG) {
            Log.i(TAG, "refereshLines");
        }
        mNumbersList.removeAll();
        mLine = mManager.getRegisteredLine();
        for (DigitsLine currLine: mLine) {
            if (currLine.getIsVirtual()) {
                RadioPowerPreference numberPref = new RadioPowerPreference(this);
                numberPref.setTitle(currLine.getLineName());
                numberPref.setSummary(currLine.getMsisdn());
                boolean isChecked = false;
                if (currLine.getLineStatus() == DigitsLine.LINE_STATUS_REGISTERED ||
                    currLine.getLineStatus() == DigitsLine.LINE_STATUS_ACTIVATED) {
                    isChecked =  true;
                }
                numberPref.setPowerState(isChecked);
                numberPref.setPersistent(false);
                mNumbersList.addPreference(numberPref);
                if (isChecked) {
                    numberPref.setOnPreferenceClickListener(mPreferenceClickListener);
                }
                //numberPref.setOnPreferenceChangeListener(mMultiLineChangeListener);
            }
        }
    }

    private boolean allLinesActivated() {
        if (DBG) {
            Log.i(TAG, "allLinesActivated");
        }
        mLine = mManager.getRegisteredLine();
        for (DigitsLine currLine: mLine) {
            if (currLine.getIsVirtual()) {
                if (!(currLine.getLineStatus() == DigitsLine.LINE_STATUS_REGISTERED ||
                        currLine.getLineStatus() == DigitsLine.LINE_STATUS_ACTIVATED)) {
                    if (DBG) {
                        Log.i(TAG, "allLinesActivated false");
                    }
                    return false;
                }
                //numberPref.setOnPreferenceChangeListener(mMultiLineChangeListener);
            }
        }
        if (DBG) {
            Log.i(TAG, "allLinesActivated true");
        }
        return true;
    }

    /**
     * codePointsToString converts the utf-8 format utf-16 format string.
     * @param name input String
     * @return format string
     */
    private String codePointsToString(String name) {
        StringBuilder sb = new StringBuilder();
        int length = name.length();
        for (int i = 0 ; i < length; i++) {
            char codeChar = name.charAt(i);
            String temp = Integer.toHexString(codeChar);
            int val = Integer.valueOf(temp, 16);
            sb.append(Character.toChars(val));
        }
        return sb.toString();
    }

    private void setDeviceName(String name) {
        String newName = codePointsToString(name);
        if (DBG) {
            Log.i(TAG, "setDeviceName " + newName);
        }
        mManager.setDeviceName(newName, mManager.new ActionListener() {
            @Override
            public void onSuccess(Bundle extras) {
                if (DBG) {
                    Log.i(TAG, "setDeviceName onSuccess");
                }
                mDeviceName = newName;
                mHandler.sendMessage(mHandler.obtainMessage(TOAST_DEVICE_NAME_CHANGE_SUCCESS, 0));
            }
            @Override
            public void onFailure(final int reason, Bundle extras) {
                if (DBG) {
                    Log.i(TAG, "setDeviceName onFailure");
                }
                mHandler.sendMessage(mHandler.obtainMessage(TOAST_DEVICE_NAME_CHANGE_FAIL, 0));
            }
        });
    }

    private void updatedLineName(String name) {
        if (DBG) {
            Log.i(TAG, "updatedLineName " + name);
        }
        mManager.setLineName(mMyMsisdn, name, mManager.new ActionListener() {
            @Override
            public void onSuccess(Bundle extras) {
                if (DBG) {
                    Log.i(TAG, "updatedLineName onSuccess");
                }
                runOnUiThread(new Runnable() {
                    public void run() {
                        mMyLineName = name;
                        Preference pref = getPreferenceScreen().findPreference("primary_profile");
                        pref.setTitle(mMyLineName);
                    }
                });
                mHandler.sendMessage(mHandler.obtainMessage(TOAST_LINE_NAME_CHANGE_SUCCESS, 0));
            }
            @Override
            public void onFailure(final int reason, Bundle extras) {
                if (DBG) {
                    Log.i(TAG, "updatedLineName onFailure");
                }
                mHandler.sendMessage(mHandler.obtainMessage(TOAST_LINE_NAME_CHANGE_FAIL, 0));
            }
        });
    }

    private void updatedLineNameForId(RadioPowerPreference numberPref, String msisdn, String name) {
        mManager.setLineName(msisdn, name, mManager.new ActionListener() {
            @Override
            public void onSuccess(Bundle extras) {
                runOnUiThread(new Runnable() {
                    public void run() {
                        if ((mPd != null) && mPd.isShowing()) {
                            mPd.dismiss();
                        }
                        numberPref.setTitle(name);
                    }
                });
                mHandler.sendMessage(mHandler.obtainMessage(TOAST_LINE_NAME_CHANGE_SUCCESS, 0));
            }
            @Override
            public void onFailure(final int reason, Bundle extras) {
                mHandler.sendMessage(mHandler.obtainMessage(TOAST_LINE_NAME_CHANGE_FAIL, 0));
            }
        });
    }

    private final Handler mHandler = new Handler() {
        public void handleMessage(Message msg) {
            String toastText = "none";
            switch (msg.what) {
                case TOAST_ACTIVATE_LINE_SUCCESS:
                    toastText = "Line activated successfully";
                    break;
                case TOAST_ACTIVATE_LINE_FAIL:
                    toastText = "Line activated failed";
                    break;
                case TOAST_DEACTIVATE_LINE_SUCCESS:
                    toastText = "Line deactivated successfully";
                    break;
                case TOAST_DEACTIVATE_LINE_FAIL:
                    toastText = "Line deactivated failed";
                    break;
                case TOAST_LOGOUT_SUCCESS:
                    toastText = "Logout successfully";
                    break;
                case TOAST_LOGOUT_FAIL:
                    toastText = "Logout failed";
                    break;
                case TOAST_ACTIVATE_ALL_LINE_SUCCESS:
                    toastText = "Activated all lines Successfully";
                    break;
                case TOAST_ACTIVATE_ALL_LINE_FAIL:
                    toastText = "Activating all lines failed";
                    break;
                case TOAST_DEACTIVATE_ALL_LINE_SUCCESS:
                    toastText = "Deactivated all lines Successfully";
                    break;
                case TOAST_DEACTIVATE_ALL_LINE_FAIL:
                    toastText = "Deactivating all lines failed";
                    break;
                case TOAST_NUMBER_ADDED_SUCCESS:
                    toastText = "Number added successfully";
                    break;
                case TOAST_NUMBER_ADDED_FAIL:
                    toastText = "Number addition failed";
                    break;
                case TOAST_DEVICE_NAME_CHANGE_SUCCESS:
                    toastText = "Device name changed successfully";
                    break;
                case TOAST_DEVICE_NAME_CHANGE_FAIL:
                    toastText = "Device name change failed";
                    break;
                case TOAST_LINE_NAME_CHANGE_SUCCESS:
                    toastText = "Line name changed successfully";
                    break;
                case TOAST_LINE_NAME_CHANGE_FAIL:
                    toastText = "Line name change failed";
                    break;
                case TOAST_NO_LINE_TOACTIVATE:
                    toastText = "No Line to activate";
                    break;
                case TOAST_MYNUMBER_NOT_ALLOWED_AS_VIRTUAL:
                    toastText = "Can't add primary line as virtual";
                    break;
                case TOAST_NUMBER_ALREADY_ADDED:
                    toastText = "Already added as virtual line";
                    break;
                default:
                    return;
            }
            if (!toastText.equals("none")) {
                try {
                  Toast.makeText(getApplicationContext(), toastText, Toast.LENGTH_LONG).show();
                } catch (Exception e) {
                  Log.e(TAG, "Exception while showing toast: " + toastText);
                }
            }
        }
    };

    /**
     * RadioPowerPreference accumulates UI to show Line settings prefernce.
     */
    public class RadioPowerPreference  extends Preference {
        private static final String TAG = "RadioPowerPreference";
        private boolean mCheckState;
        private boolean mPowerEnabled = true;
        private Switch mRadioSwitch = null;
        private Context mRadioContext = null;

        /**
         * Construct of RadioPowerPreference.
         * @param context Context.
         */
        public RadioPowerPreference(Context context) {
            super(context);
            mRadioContext = context;
            setWidgetLayoutResource(R.layout.radio_power_switch);
        }




        @Override
        public void onBindView(View view) {
            super.onBindView(view);
            mRadioSwitch = (Switch) view.findViewById(R.id.radio_state);
            if (mRadioSwitch != null) {
                mRadioSwitch.setChecked(mCheckState);
                mRadioSwitch.setEnabled(mPowerEnabled);
                mRadioSwitch.setOnCheckedChangeListener(new OnCheckedChangeListener() {
                    @Override
                    public void onCheckedChanged(CompoundButton buttonView, boolean checked) {
                        String str = "Switch: " + checked;
                        // Code goes here
                        String msisdn = getSummary().toString();
                        showProgressDialog();
                        Log.d(TAG, "msisdn: " + msisdn + "checked: " + checked);
                        if (checked) {
                            mManager.activateLine(msisdn, mManager.new ActionListener() {
                                @Override
                                public void onSuccess(Bundle extras) {
                                    Log.d(TAG, "activateLine onSuccess");
                                    runOnUiThread(new Runnable() {
                                        public void run() {
                                            if ((mPd != null) && mPd.isShowing()) {
                                                mPd.dismiss();
                                            }
                                            mHandler.sendMessage(mHandler.obtainMessage(
                                                    TOAST_ACTIVATE_LINE_SUCCESS, 0));
                                        }
                                    });
                                    return;
                                }
                                @Override
                                public void onFailure(final int reason, Bundle extras) {
                                    Log.d(TAG, "activateLine onFailure");
                                    runOnUiThread(new Runnable() {
                                        public void run() {
                                            if ((mPd != null) && mPd.isShowing()) {
                                                mPd.dismiss();
                                            }
                                            mRadioSwitch.setChecked(false);
                                            setPowerState(!checked);
                                            mHandler.sendMessage(mHandler.obtainMessage(
                                                    TOAST_ACTIVATE_LINE_FAIL, 0));
                                        }
                                    });
                                    return;
                                }
                            });
                        } else {
                            mManager.deactivateLine(msisdn, mManager.new ActionListener() {
                                @Override
                                public void onSuccess(Bundle extras) {
                                    Log.d(TAG, "deactivateLine onSuccess");
                                    runOnUiThread(new Runnable() {
                                        public void run() {
                                            if ((mPd != null) && mPd.isShowing()) {
                                                mPd.dismiss();
                                            }
                                            mHandler.sendMessage(mHandler.obtainMessage(
                                                    TOAST_DEACTIVATE_LINE_SUCCESS, 0));
                                        }
                                    });
                                    return;
                                }
                                @Override
                                public void onFailure(final int reason, Bundle extras) {
                                    Log.d(TAG, "deactivateLine onFailure");
                                    runOnUiThread(new Runnable() {
                                        public void run() {
                                            if ((mPd != null) && mPd.isShowing()) {
                                                mPd.dismiss();
                                            }
                                            mRadioSwitch.setChecked(true);
                                            setPowerState(!checked);
                                            mHandler.sendMessage(mHandler.obtainMessage(
                                                    TOAST_DEACTIVATE_LINE_FAIL, 0));
                                        }
                                    });
                                    return;
                                }
                            });
                        }
                    }
                });
                // ensure setOnCheckedChangeListener before setChecked state, or the
                // expired OnCheckedChangeListener will be called, due to the view is RecyclerView
            }
        }

        public void setPowerState(boolean checkState) {
            mCheckState = checkState;
        }
    }
}
