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

package com.mediatek.rcse.fragments;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.Dialog;
import android.app.DialogFragment;
import android.app.FragmentManager;
import android.app.FragmentTransaction;
import android.app.ProgressDialog;
import android.content.ActivityNotFoundException;
import android.content.BroadcastReceiver;
import android.content.ContentProviderOperation;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.SharedPreferences;
import android.content.SharedPreferences.Editor;
import android.content.pm.ApplicationInfo;
import android.content.pm.PackageManager;
import android.content.pm.ResolveInfo;
import android.content.res.Resources;
import android.os.AsyncTask;
import android.os.Bundle;
import android.os.Message;
import android.preference.CheckBoxPreference;
import android.preference.ListPreference;
import android.preference.Preference;
import android.preference.PreferenceManager;
import android.preference.Preference.OnPreferenceChangeListener;
import android.preference.Preference.OnPreferenceClickListener;
import android.preference.PreferenceFragment;
import android.preference.PreferenceScreen;
import android.preference.SwitchPreference;
import android.provider.ContactsContract;
import android.provider.ContactsContract.Contacts;
import android.provider.ContactsContract.CommonDataKinds.Phone;
import android.provider.ContactsContract.CommonDataKinds.StructuredName;
import android.provider.ContactsContract.Data;
import android.provider.ContactsContract.RawContacts;
import android.telephony.TelephonyManager;
import android.util.Log;
import android.util.Xml;
import android.view.LayoutInflater;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.ViewGroup;
import android.widget.Button;
import android.widget.Toast;

import com.mediatek.rcs.R;
import com.mediatek.rcse.interfaces.ChatController;
import com.mediatek.rcse.mvc.ControllerImpl;
import com.mediatek.rcse.plugin.message.IpMessageConsts;
import com.mediatek.rcse.activities.DirectoryPicker;
import com.mediatek.rcse.api.ContactsApiIntents;
import com.mediatek.rcse.api.Logger;
import com.mediatek.rcse.api.RegistrationApi;
import com.mediatek.rcse.plugin.message.PluginUtils;
import com.mediatek.rcse.service.ApiManager;
import com.mediatek.rcse.service.CoreApplication;
import com.mediatek.rcse.service.RcsOsUtils;
import com.mediatek.rcse.service.ApiManager.RcseComponentController;
import com.mediatek.rcse.service.MediatekFactory;
import com.mediatek.rcse.settings.AppSettings;
import com.mediatek.rcse.settings.AppSettingsData;
import com.mediatek.rcse.settings.RcsSettings;
import com.orangelabs.rcs.provider.settings.RcsSettingsData;
import com.orangelabs.rcs.service.LauncherUtils;



import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.InputStreamReader;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Calendar;
import java.util.List;


import com.gsma.services.rcs.Intents;
import com.gsma.services.rcs.JoynServiceConfiguration;
import org.xmlpull.v1.XmlPullParser;
import org.xmlpull.v1.XmlPullParserException;

/*
 * Displays RCS-e settings UI. It will be set up to a activity.
 */
/**
 * The Class RcseSystemSettingsFragment.
 */
public class RcseSystemSettingsFragment extends PreferenceFragment {

    public static final String SETTINGS_PREFERENCES = "UserPreferences";
    private static final String TAG = "RcseSystemSettingsFragment";
    private static final int REQUIRED_PERMISSIONS_REQUEST_CODE = 1;
    private CheckBoxPreference mRcseServiceActivation;
    private CheckBoxPreference mRcseRoming;
    private CheckBoxPreference mIntegratedMode;
    private CheckBoxPreference mDebugMode;
    private ListPreference mConfigPreference;
    private SwitchPreference mRCSeClientSwitch;
    private SwitchPreference mRCSeMasterSwitch;
    private Preference mBackupMessage;
    private Preference mRestoreMessage;
    private Preference mOp07Button;
    private static final String RCS_OP07_TEST = "op07_test";
    private static final String RCS_ACTIVITATION = "rcs_activation";
    private static final String RCSE_MASTER_SWITCH = "masterSwitch";
    private static final String RCS_ROMING = "rcs_roaming";
    private static final String BACKUP_MESSAGE = "backup_message";
    private static final String RESTORE_MESSAGE = "restore_message";
    private static final String INTEGRATED_MODE = "integrated_mode";
    private static final String DEBUG_MODE = "debug_mode";
    private static final String PREF_CONFIG = "prefConfig";
    private static final String CLIENT = "clientSwitch";
    private static final String MESSAGE_RCS_ID = "messageResId";
    private static final String ACTION_PROVISION_PROFILE =
            "com.mediatek.rcse.action.PROVISION_PROFILE_SETTING";
    private static final String RCS_DISABLE_JOYN = "rcse_disable_joyn";
    private static final String RCS_MASTER_SWITCH = "rcse_master_switch";
    private static final String RCS_CLIENT_SWITCH = "rcse_client_switch";
    private static final String RCS_DEACTIVATE_JOYN = "rcse_deactivate_joyn";
    private static final String RCS_DISABLE_TIME = "";
    private static final String RCS_BACKUP_TIME = "";
    private static final int MODE_PRIVATE = 0;
    private boolean mCancelledDisabling = false;
    private static SharedPreferences sSettings = null;
    public static boolean sMasterSwitch = true;
    public static boolean sClientSwitch = true;
    private static final int ENABLE_JOYN = 0;
    private static final int DEACTIVATE_JOYN_TEMP = 1;
    private static final int DEACTIVATE_JOYN_PERMANENTLY = 2;
    private static final String DISABLE_JOYN_VERSION = "-1";
    private static final String VERSION_RESETED = "0";
    public static String sDisableTime;
    public static String sBackupTime;
    IntentListener mIntentListener = new IntentListener();
    private static final String CONNECTIVITY_CHANGE =
            "android.net.conn.CONNECTIVITY_CHANGE";
    private static boolean sIsRoaming = false;
    private final static int RCS_CORE_LOADED = 0;
    private final static int RCS_CORE_FAILED = 1;
    private final static int RCS_CORE_STARTED = 2;
    private final static int RCS_CORE_STOPPED = 3;
    private final static int RCS_CORE_IMS_CONNECTED = 4;
    private final static int RCS_CORE_IMS_TRY_CONNECTION = 5;
    private final static int RCS_CORE_IMS_CONNECTION_FAILED = 6;
    private final static int RCS_CORE_IMS_TRY_DISCONNECT = 7;
    private final static int RCS_CORE_IMS_BATTERY_DISCONNECTED = 8;
    private final static int RCS_CORE_IMS_DISCONNECTED = 9;
    private final static int RCS_CORE_NOT_LOADED = 10;
    private static int sRcsState = RCS_CORE_NOT_LOADED;

    private static String OP07_TEST_CONTACT_LIST[] = {"Addision",
        "Adrian", "Alex", "Alexis", "Andy", "Angel", "Ariel", "Angel"};

    public class IntentListener extends BroadcastReceiver {
        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            Logger.d(TAG, "onReceive intent action:" + action);
            if(mRCSeMasterSwitch == null){
                return;
            }
            if (Intents.Client.ACTION_VIEW_SETTINGS.equalsIgnoreCase(action)) {
                sRcsState = intent.getIntExtra("label_enum", RCS_CORE_NOT_LOADED);
                Logger.d(TAG, "sRcsState value:" + sRcsState);
                switch (sRcsState) {
                    case RCS_CORE_IMS_TRY_CONNECTION:
                    case RCS_CORE_IMS_TRY_DISCONNECT:
                    case RCS_CORE_IMS_DISCONNECTED:
                       mRCSeMasterSwitch.setEnabled(false);
                       break;
                    default:
                       if(!mRCSeMasterSwitch.isEnabled() && isMasterSwitchActive()){
                           mRCSeMasterSwitch.setEnabled(true);
                       }
                       break;
                }
            } else if (CONNECTIVITY_CHANGE.equalsIgnoreCase(action)) {
                TelephonyManager telephony =
                        (TelephonyManager) context.getSystemService(Context.TELEPHONY_SERVICE);
                if (telephony.isNetworkRoaming()){
                    sIsRoaming = true;
                } else {
                    sIsRoaming = false;
                }
                Logger.d(TAG, "mIsRoaming value:" + sIsRoaming);
            }

        }
    }
    /**
     * Gets the disable time.
     *
     * @return the disable time
     */
    public static String getDisableTime() {
        return sDisableTime;
    }

    /**
     * Gets the backup time.
     *
     * @return the backup time
     */
    public static String getBackupTime() {
        return sBackupTime;
    }

    /**
     * Sets the disable time.
     *
     * @param disableTime the new disable time
     */
    public static void setDisableTime(String disableTime) {
        RcseSystemSettingsFragment.sDisableTime = disableTime;
        sSettings.edit().putString(RCS_DISABLE_TIME, disableTime).apply();
    }

    /**
     * Sets the backup time.
     *
     * @param backupTime the new backup time
     */
    public static void setBackupTime(String backupTime) {
        RcseSystemSettingsFragment.sBackupTime = backupTime;
        sSettings.edit().putString(RCS_BACKUP_TIME, backupTime).apply();
    }

    static boolean sDisableJoyn = false;

    /**
     * Checks if is disable joyn.
     *
     * @return true, if is disable joyn
     */
    public boolean isDisableJoyn() {
        return sDisableJoyn;
    }

    /**
     * Sets the disable joyn.
     *
     * @param disableJoyn the new disable joyn
     */
    public void setDisableJoyn(boolean disableJoyn) {
        sDisableJoyn = disableJoyn;
        sSettings.edit().putBoolean(RCS_DISABLE_JOYN, disableJoyn).apply();
    }

    static boolean sDeactivateJoyn = false;

    /**
     * Checks if is deactivate joyn.
     *
     * @return true, if is deactivate joyn
     */
    public boolean isDeactivateJoyn() {
        return sDeactivateJoyn;
    }

    /**
     * Sets the deactivate joyn.
     *
     * @param deactivateJoyn the new deactivate joyn
     */
    public void setDeativateJoyn(boolean deactivateJoyn) {
        sDeactivateJoyn = deactivateJoyn;
        sSettings.edit().putBoolean(RCS_DEACTIVATE_JOYN, deactivateJoyn)
                .apply();
    }

    /**
     * Checks if is master switch.
     *
     * @return true, if is master switch
     */
    public static boolean isMasterSwitch() {
        return RcseSystemSettingsFragment.sMasterSwitch;
    }

    /**
     * Checks if is client switch.
     *
     * @return true, if is client switch
     */
    public static boolean isClientSwitch() {
        return RcseSystemSettingsFragment.sClientSwitch;
    }

    /**
     * Sets the master switch.
     *
     * @param masterSwitch the new master switch
     */
    public static void setMasterSwitch(boolean masterSwitch) {
        RcseSystemSettingsFragment.sMasterSwitch = masterSwitch;
        sSettings.edit().putBoolean(RCS_MASTER_SWITCH, masterSwitch).apply();
    }

    /**
     * Sets the client switch.
     *
     * @param clientSwitch the new client switch
     */
    public static void setClientSwitch(boolean clientSwitch) {
        RcseSystemSettingsFragment.sClientSwitch = clientSwitch;
        sSettings.edit().putBoolean(RCS_CLIENT_SWITCH, clientSwitch).apply();
    }

    /**
     * M: added to resolve multiple RCSe client case @(.
     * */
    private ProgressDialog mProgressDialog = null;
    private boolean mIsOtherJoynAppEnabled;

    private void checkContactPermissions() {
        SharedPreferences sPrefer = PreferenceManager
                .getDefaultSharedPreferences(getActivity());
          boolean alwaysDeny = sPrefer.getBoolean("alwaysdenypermission", false);
          Logger.v(TAG, "checkContactPermissions ()alwaysdeny:" + alwaysDeny);
          if (!RcsOsUtils.hasRequiredPermissions()) {
              Logger.v(TAG, "checkContactPermissions not there");
          } else {
              if(!JoynServiceConfiguration.isServicePermission(MediatekFactory.getApplicationContext())) {
                  JoynServiceConfiguration.setServicePermissionState(true, MediatekFactory.getApplicationContext());
              }
              if(alwaysDeny) {
                  Editor alwaysdeny = sPrefer.edit();
                  alwaysdeny.putBoolean("alwaysdenypermission", false);
                  alwaysdeny.commit();
              }
          }
    }

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container,
            Bundle savedInstanceState) {
        Logger.d(TAG, "onCreateView() entry");
        checkContactPermissions();
        sSettings = getActivity().getSharedPreferences(SETTINGS_PREFERENCES,
                Activity.MODE_APPEND);
        // mSettings =
        boolean defValue = RegistrationApi.isServiceActivated();

        RcseSystemSettingsFragment.sMasterSwitch = sSettings.getBoolean(
                RCS_MASTER_SWITCH, defValue);
        RcseSystemSettingsFragment.sClientSwitch = sSettings.getBoolean(
                RCS_CLIENT_SWITCH, true);
        sDeactivateJoyn = sSettings.getBoolean(RCS_DEACTIVATE_JOYN, !defValue);
        sDisableJoyn = sSettings.getBoolean(RCS_DISABLE_JOYN, false);
        sDisableTime = sSettings.getString(RCS_DISABLE_TIME, "");
        sBackupTime = sSettings.getString(RCS_BACKUP_TIME, "");
        mIsOtherJoynAppEnabled = false;
        addPreferencesFromResource(R.xml.rcse_system_settings);
        setupDebugItems(Logger.IS_DEBUG);
        mRcseServiceActivation = (CheckBoxPreference) findPreference(RCS_ACTIVITATION);
        setupMasterSwitch();
        setupClientSwitch();
        setupBackupButton();
        setupRestoreButton();
        IntentFilter filter = new IntentFilter();
        filter.addAction(CONNECTIVITY_CHANGE);
        filter.addAction(Intents.Client.ACTION_VIEW_SETTINGS);
        getActivity().getApplicationContext().registerReceiver(mIntentListener, filter);

        mRcseServiceActivation
                .setTitle(R.string.rcs_settings_label_rcs_service);
        mRcseRoming = (CheckBoxPreference) findPreference(RCS_ROMING);
        mIntegratedMode = (CheckBoxPreference) findPreference(INTEGRATED_MODE);
        mDebugMode = (CheckBoxPreference) findPreference(DEBUG_MODE);
        mConfigPreference = (ListPreference) findPreference(PREF_CONFIG);

        setupOp07Button();

        Logger.d(TAG, "sMasterSwitch:" + sMasterSwitch);

        if (false) {
            Logger.d(TAG, "onCreateView() sIsDebug true");
            mDebugMode.setSummary(R.string.debug_mode_dummy);
            mDebugMode.setChecked(true);
        } else {
            Logger.d(TAG, "onCreateView() sIsDebug false");
            mDebugMode.setSummary(R.string.debug_mode_live);
            mDebugMode.setChecked(false);
        }
        if (RcsSettings.getInstance().getMessagingUx() == 0) {
            mIntegratedMode.setSummary(R.string.converged_integrated_mode);
            mIntegratedMode.setChecked(true);
        } else {
            mIntegratedMode.setSummary(R.string.full_integrated_mode);
            mIntegratedMode.setChecked(false);
        }
        // Remove rcse activation & roaming preference. Instead added a joyn
        // master switch
        getPreferenceScreen().removePreference(mRcseRoming);
        getPreferenceScreen().removePreference(mRcseServiceActivation);
        if (false) {
            getPreferenceScreen().removePreference(mConfigPreference);

        }
        return super.onCreateView(inflater, container, savedInstanceState);
    }

    /**
     * Setup backup button.
     */
    public void setupBackupButton() {

        mBackupMessage = (Preference) findPreference(BACKUP_MESSAGE);
        mBackupMessage.setSummary("Message Backup at " + sBackupTime);
        mBackupMessage
                .setOnPreferenceClickListener(new OnPreferenceClickListener() {

                    @Override
                    public boolean onPreferenceClick(Preference arg0) {
                        // TODO Auto-generated method stub
                        /*
                         * DirectoryChooserDialog directoryChooserDialog = new
                         * DirectoryChooserDialog(getActivity(), new
                         * DirectoryChooserDialog.ChosenDirectoryListener() {
                         *
                         * @Override public void onChosenDir(String chosenDir) {
                         * m_chosenDir = chosenDir; Toast.makeText(
                         * getActivity(), "Chosen context: " + getActivity(),
                         * Toast.LENGTH_LONG).show(); } }); // Toggle new folder
                         * button enabling
                         * directoryChooserDialog.setNewFolderEnabled
                         * (m_newFolderEnabled); // Load directory chooser
                         * dialog for initial 'm_chosenDir' directory. // The
                         * registered callback will be called upon final
                         * directory selection.
                         * directoryChooserDialog.chooseDirectory(m_chosenDir);
                         * m_newFolderEnabled = ! m_newFolderEnabled;
                         */

                        // Create DirectoryChooserDialog and register a callback

                        Intent intentGetMessage = new Intent(getActivity(),
                                DirectoryPicker.class);
                        startActivityForResult(intentGetMessage, 2);

                        // LauncherUtils.backupMessage(getActivity().getApplicationContext());
                        /*
                         * Calendar c = Calendar.getInstance(); SimpleDateFormat
                         * df = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss");
                         * setBackupTime(df.format(c.getTime()));
                         * mBackupMessage.setSummary("Message Backup at " +
                         * backupTime);
                         */
                        return false;
                    }
                });
        // LauncherUtils.backupMessage();

    }

    /**
     * Setup restore button.
     */
    public void setupRestoreButton() {

        mRestoreMessage = (Preference) findPreference(RESTORE_MESSAGE);
        mRestoreMessage.setSummary("Message Restored from "
        /* + backupTime */);
        mRestoreMessage
                .setOnPreferenceClickListener(new OnPreferenceClickListener() {

                    @Override
                    public boolean onPreferenceClick(Preference arg0) {
                        // TODO Auto-generated method stub
                        /*
                         * DirectoryChooserDialog directoryChooserDialog = new
                         * DirectoryChooserDialog(getActivity(), new
                         * DirectoryChooserDialog.ChosenDirectoryListener() {
                         *
                         * @Override public void onChosenDir(String chosenDir) {
                         * m_chosenDir = chosenDir; Toast.makeText(
                         * getActivity(), "Chosen context: " + getActivity(),
                         * Toast.LENGTH_LONG).show(); } }); // Toggle new folder
                         * button enabling
                         * directoryChooserDialog.setNewFolderEnabled
                         * (m_newFolderEnabled); // Load directory chooser
                         * dialog for initial 'm_chosenDir' directory. // The
                         * registered callback will be called upon final
                         * directory selection.
                         * directoryChooserDialog.chooseDirectory(m_chosenDir);
                         * m_newFolderEnabled = ! m_newFolderEnabled;
                         */

                        // Create DirectoryChooserDialog and register a callback

                        /*
                         * AlertDialog.Builder alertDialogBuilder = new
                         * AlertDialog.Builder(getActivity());
                         * alertDialogBuilder.setMessage(R.string.decision);
                         * alertDialogBuilder
                         * .setPositiveButton(R.string.positive_button, new
                         * DialogInterface.OnClickListener() {
                         *
                         * @Override public void onClick(DialogInterface arg0,
                         * int arg1) { Intent intentGetMessage=new
                         * Intent(getActivity(),DirectoryPicker.class);
                         * startActivityForResult(intentGetMessage, 3);
                         *
                         * } }); alertDialogBuilder.setNegativeButton(R.string.
                         * negative_button, new
                         * DialogInterface.OnClickListener() {
                         *
                         * @Override public void onClick(DialogInterface dialog,
                         * int which) {
                         *
                         * } });
                         *
                         * AlertDialog alertDialog =
                         * alertDialogBuilder.create(); alertDialog.show();
                         */

                        Intent intentGetMessage = new Intent(getActivity(),
                                DirectoryPicker.class);
                        startActivityForResult(intentGetMessage, 3);

                        // LauncherUtils.backupMessage(getActivity().getApplicationContext());

                        return false;
                    }
                });
        // LauncherUtils.backupMessage();

    }

    @Override
    public void onActivityResult(int requestCode, int resultCode, Intent data) {
        try {
            super.onActivityResult(requestCode, resultCode, data);

            Logger.d(TAG, "Request code for Activity Result" + requestCode);

            // check if the request code is same as what is passed here it is 2
            if (requestCode == 2) {
                Logger.d(TAG, "on Activity Result for backup");
                Bundle extras = data.getExtras();
                String path = (String) extras
                        .get(DirectoryPicker.CHOSEN_DIRECTORY);
                /*LauncherUtils.backupMessage(getActivity()
                        .getApplicationContext(), path);*/
                Calendar c = Calendar.getInstance();
                SimpleDateFormat df = new SimpleDateFormat(
                        "yyyy-MM-dd HH:mm:ss");
                setBackupTime(df.format(c.getTime()));
                mBackupMessage.setSummary("Message Backup at " + sBackupTime);
            }
            if (requestCode == 3) {
                Logger.d(TAG, "on Activity Result for restore status code 3");
                Bundle extras = data.getExtras();
                String path = (String) extras
                        .get(DirectoryPicker.CHOSEN_DIRECTORY);
                // RichMessagingDataProvider.destroyInstance();
                /*LauncherUtils.restoreMessages(getActivity()
                        .getApplicationContext(), path);*/
                Intent intent = new Intent(PluginUtils.ACTION_DB_CHANGE);
                MediatekFactory.getApplicationContext().sendBroadcast(intent, "com.mediatek.rcse.permission.RCS_ACCESS_MESSAGE");
                mRestoreMessage.setSummary("Message Restored from " + path
                /* + backupTime */);
                Logger.d(TAG, "on Activity Result now before exit 3");
                // System.exit(2);
            }
        } catch (Exception e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
        }

    }

    /**
     * Setup client switch.
     */
    public void setupClientSwitch() {
        mRCSeClientSwitch = (SwitchPreference) findPreference(CLIENT);
        mRCSeClientSwitch.setChecked(isClientSwitch());
        mRCSeClientSwitch.setSwitchTextOff("MTI");
        mRCSeClientSwitch.setSwitchTextOn("MTI");
        if (isClientSwitch()) {
            mRCSeClientSwitch.setSummary("MTI Client");
        } else {
            mRCSeClientSwitch.setSummary("MTI Client");
        }

        mRCSeClientSwitch
                .setOnPreferenceChangeListener(new OnPreferenceChangeListener() {
                    @Override
                    public boolean onPreferenceChange(Preference preference,
                            Object newValue) {
                        // TODO Auto-generated method stub
                        boolean switched = ((SwitchPreference) preference)
                                .isChecked();
                        if (switched) {
                            /*LauncherUtils.saveClient(getActivity()
                                    .getApplicationContext(), "MTI");*/
                            setClientSwitch(false);
                            mRCSeClientSwitch.setChecked(isClientSwitch());
                            mRCSeClientSwitch.setSummary("MTI Client");
                        } else {

                            setClientSwitch(true);
                            mRCSeClientSwitch.setChecked(isClientSwitch());
                            mRCSeClientSwitch.setSummary("MTI Client");
                        }
                        return switched;
                    }
                });
    }

    /**
     * Is master switch needs to be enabled as per configuration value.
     */
    private boolean isMasterSwitchActive() {
        return true;
    }

    /**
     * Setup master switch.
     */
    public void setupMasterSwitch() {
        Logger.d(TAG, "setup master switch");
        mRCSeMasterSwitch = (SwitchPreference) findPreference(RCSE_MASTER_SWITCH);
        boolean active = isMasterSwitchActive();
        mRCSeMasterSwitch.setEnabled(active);
        mRCSeMasterSwitch.setChecked(isMasterSwitch());
        mRCSeMasterSwitch.setSwitchTextOff("OFF");
        mRCSeMasterSwitch.setSwitchTextOn("ON");
        if (isMasterSwitch()) {
            mRCSeMasterSwitch.setSummary("Joyn is activated");
        } else {
            if (isDisableJoyn()) {
                mRCSeMasterSwitch.setSummary("Joyn disabled permanently at "
                        + sDisableTime);
            } else if (isDeactivateJoyn()) {
                mRCSeMasterSwitch.setSummary("Joyn Deactiavted at "
                        + sDisableTime);
            }
        }

        mRCSeMasterSwitch
                .setOnPreferenceChangeListener(new OnPreferenceChangeListener() {
                    @Override
                    public boolean onPreferenceChange(Preference preference,
                            Object newValue) {
                        // TODO Auto-generated method stub
                        boolean switched = ((SwitchPreference) preference)
                                .isChecked();
                        if (isMasterSwitch()) {
                            if (!isDeactivateJoyn() || !isDisableJoyn()) {
                                Logger.d(TAG,
                                        "setupMasterSwitch() switch already true, disabl;e = "
                                                + isDisableJoyn()
                                                + "deativate ="
                                                + isDeactivateJoyn());
                                createMasterSwitchDialog();
                            } else {
                                Logger.d(TAG,
                                        "setupMasterSwitch() true but disable is false");
                            }

                        } else {
                            Logger.d(TAG,
                                    "setupMasterSwitch() switch already false");
                            searchOtherRCSeClient();
                        }
                        return switched;
                    }
                });
    }

    @Override
    public void onResume() {
        Log.d(TAG, "onResume()");
        // new GetServiceStatusTask().execute();
        super.onResume();
    }

    @Override
    public boolean onPreferenceTreeClick(PreferenceScreen preferenceScreen,
            Preference preference) {
        if (preference == mRcseServiceActivation) {

            /**
             * M: added to resolve multiple RCSe client case @(
             * */

            // process the rcse service request
            if (mRcseServiceActivation != null
                    && !mRcseServiceActivation.isChecked()) {
                showAttentionDialog(R.string.rcse_settings_disable_rcse_service);
            } else if (mRcseServiceActivation != null
                    && mRcseServiceActivation.isChecked()) {
                // search if any rcs client available
                // it will check the values and according displya the dialogs if
                // no extra rcs cleint are present
                searchOtherRCSeClient();
            }

            /**
             * @}
             */

        } else if (preference == mRcseRoming) {
            if (mRcseRoming != null && !mRcseRoming.isChecked()) {
                showAttentionDialog(R.string.rcse_settings_disable_rcse_service_roaming);
            } else if (mRcseRoming != null && mRcseRoming.isChecked()) {
                showAttentionDialog(R.string.rcse_settings_enable_rcse_service_roaming);
            }
        } else if (preference == mBackupMessage) {
            // LauncherUtils.backupMessage(getActivity().getApplicationContext());
            Calendar c = Calendar.getInstance();
            SimpleDateFormat df = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss");
            setBackupTime(df.format(c.getTime()));
            Logger.d(TAG, "Backuping messages ");

        } else if (preference == mRestoreMessage) {
            // LauncherUtils.backupMessage(getActivity().getApplicationContext());
            /*
             * Calendar c = Calendar.getInstance(); SimpleDateFormat df = new
             * SimpleDateFormat("yyyy-MM-dd HH:mm:ss");
             * setBackupTime(df.format(c.getTime()));
             */
            Logger.d(TAG, "Restoring messages ");

        } else if (preference == mIntegratedMode) {
            if (mIntegratedMode != null && !mIntegratedMode.isChecked()) {
                mIntegratedMode.setSummary(R.string.full_integrated_mode);
                RcsSettings.getInstance().setMessagingUx(1);
            } else if (mIntegratedMode != null && mIntegratedMode.isChecked()) {
                mIntegratedMode.setSummary(R.string.converged_integrated_mode);
                RcsSettings.getInstance().setMessagingUx(0);
            }
            Intent intent = new Intent();
            intent.setAction(PluginUtils.ACTION_MODE_CHANGE);
            intent.putExtra("mode", RcsSettings.getInstance().getMessagingUx());
            getActivity().getApplicationContext().sendBroadcast(intent);
            Logger.d(TAG, "Changing the mode now is " + RcsSettings.getInstance().getMessagingUx());
        } else if (preference == mDebugMode) {
            if (mDebugMode != null && !mDebugMode.isChecked()) {
                mDebugMode.setSummary(R.string.debug_mode_live);
                //LauncherUtils.setDebugMode(getActivity(), false);
                // LauncherUtils.sIsDebug = false;
            } else if (mDebugMode != null && mDebugMode.isChecked()) {
                mDebugMode.setSummary(R.string.debug_mode_dummy);
                // LauncherUtils.sIsDebug = true;
                //LauncherUtils.setDebugMode(getActivity(), true);
            }
        } else {
            Logger.w(TAG, "Unknown check preference clicked");
        }
        return super.onPreferenceTreeClick(preferenceScreen, preference);
    }

    /**
     * Creates the master switch dialog.
     */
    private void createMasterSwitchDialog() {
        AlertDialog levelDialog;

        // Strings to Show In Dialog with Radio Buttons
        final CharSequence[] items = { " Deactivate Joyn ",
                " Disable Joyn Permanently " };
        setDeativateJoyn(true);
        // Creating and Building the Dialog
        AlertDialog.Builder builder = new AlertDialog.Builder(getActivity());
        builder.setTitle("Disable options");
        builder.setSingleChoiceItems(items, 0,
                new DialogInterface.OnClickListener() {
                    public void onClick(DialogInterface dialog, int item) {

                        switch (item) {
                        case 0:
                            setDeativateJoyn(true);
                            setDisableJoyn(false);
                            break;
                        case 1:
                            setDisableJoyn(true);
                            setDeativateJoyn(false);
                            break;
                        default:
                            break;

                        }
                    }
                });
        builder.setPositiveButton("Ok", new DialogInterface.OnClickListener() {
            public void onClick(DialogInterface dialog, int whichButton) {
                if (isDeactivateJoyn()) {
                    Logger.v(TAG, "DEACTIVATE_JOYN task");
                    mRCSeMasterSwitch.setChecked(false);
                    Calendar c = Calendar.getInstance();
                    SimpleDateFormat df = new SimpleDateFormat(
                            "yyyy-MM-dd HH:mm:ss");
                    setDisableTime(df.format(c.getTime()));
                    Intent it = new Intent();
                    it.setAction(IpMessageConsts.DisableServiceStatus.
                            ACTION_DISABLE_SERVICE_STATUS);
                    it.putExtra("status", DEACTIVATE_JOYN_TEMP);
                    MediatekFactory.getApplicationContext().sendBroadcast(it);
                    RcsSettings.getInstance().setDisableServiceStatus(
                            DEACTIVATE_JOYN_TEMP);
                    mRCSeMasterSwitch.setSummary("Joyn deactivated at "
                            + sDisableTime);
                    new StopServiceTask().execute();
                } else if (isDisableJoyn()) {
                    showAttentionDialog(R.string.rcse_settings_disable_rcse_masterswitch);
                }
                setMasterSwitch(false);
            }
        });
        builder.setNegativeButton("Cancel",
                new DialogInterface.OnClickListener() {
                    public void onClick(DialogInterface dialog, int whichButton) {
                        setDisableJoyn(false);
                        setDeativateJoyn(false);
                        mCancelledDisabling = true;
                        mRCSeMasterSwitch.setChecked(true);
                        mRCSeMasterSwitch.setSummary("Joyn is activated");
                        dialog.cancel();
                    }
                });
        levelDialog = builder.create();
        levelDialog.setCancelable(false);
        levelDialog.setCanceledOnTouchOutside(false);
        levelDialog.show();
    }

    /**
     * Show attention dialog.
     *
     * @param messageResId the message res id
     */
    private void showAttentionDialog(int messageResId) {

        ConfirmDialog confirmDialog = new ConfirmDialog(this, new StartServiceTask(), new StopServiceTask());
        Bundle bundle = new Bundle();
        bundle.putInt(MESSAGE_RCS_ID, messageResId);
        confirmDialog.setArguments(bundle);

        final FragmentManager fragmentManager = getFragmentManager();
        final FragmentTransaction transaction = fragmentManager
                .beginTransaction();
        transaction.add(confirmDialog, ConfirmDialog.TAG);
        transaction.commitAllowingStateLoss();
    }

    /**
     * Enable rcs service.
     */
    private void enableRcsService() {
        Logger.v(TAG, "enableRcsService()");
        Activity activity = getActivity();
        if (activity != null) {
            /*LauncherUtils.launchRcsService(activity.getApplicationContext(),
                    false, true);*/
            if (RcsOsUtils.hasRequiredPermissions()) {
                if(!JoynServiceConfiguration.isServicePermission(MediatekFactory.getApplicationContext())) {
                    Logger.v(TAG, "critical permission is there, set permission state true");
                    JoynServiceConfiguration.setServicePermissionState(true, MediatekFactory.getApplicationContext());
                }
            }
            Intent intent = new Intent();
            intent.setAction(CoreApplication.LAUNCH_SERVICE);
            intent.setClassName("com.orangelabs.rcs",
                    "com.orangelabs.rcs.service.LaunchServiceUser");
            //intent.putExtra(CORE_CONFIGURATION_STATUS, false);
            getActivity().getApplicationContext().sendBroadcast(intent);

        } else {
            Logger.w(TAG, "activity is null, dialog become null.");
        }
    }

    /**
     * Disable rcs service.
     */
    private void disableRcsService() {
        Logger.v(TAG, "disableRcsService()");
        Activity activity = getActivity();
        if (activity != null) {
            /*LauncherUtils.stopRcsService(getActivity().getApplicationContext());*/
            Intent intent = new Intent();
            intent.setAction(CoreApplication.STOP_SERVICE);
            intent.setClassName("com.orangelabs.rcs",
                    "com.orangelabs.rcs.service.StopServiceUser");
            //intent.putExtra(CORE_CONFIGURATION_STATUS, false);
            getActivity().getApplicationContext().sendBroadcast(intent);
            ControllerImpl controller = ControllerImpl.getInstance();
            Message controllerMessage = controller.obtainMessage(
                            ChatController.EVENT_REGISTRATION_DISCONNECTED,
                            null, null);
            controllerMessage.sendToTarget();
        } else {
            Logger.w(TAG, "activity is null, dialog become null.");
        }
    }

    /**
     * Start service thread.
     */
    private class StartServiceTask extends AsyncTask<Void, Void, Void> {
        @Override
        protected void onPreExecute() {
            setServiceActivatedEnabled(false);
        }

        @Override
        protected Void doInBackground(Void... params) {
            Logger.d(TAG,
                    "StartServiceTask doInBackground() enable ChatMainActivity");

            RcseComponentController rcseComponentController = ApiManager
                    .getInstance().getRcseComponentController();
            if (rcseComponentController != null) {
                rcseComponentController.onServiceActiveStatusChanged(true);
            } else {
                Logger.e(
                        TAG,
                        "StartServiceTask doInBackground()"
                                + " ApiManager.getInstance().getRcseComponentController() is null");
            }

            Activity activity = getActivity();
            if (activity != null) {
                Logger.v(TAG, "Enable rcse service.");
                RegistrationApi.setServiceActivationState(true);
                enableRcsService();
            } else {
                Logger.w(TAG, "activity is null, dialog become null.");
            }
            return null;
        }

        @Override
        protected void onPostExecute(Void result) {
            setServiceActivatedEnabled(true);
            setServiceActivatedStatus(true);
            setRoamingAuthorizedEnabled(true);
            setMasterSwitch(true);
            setDisableJoyn(false);
            setDeativateJoyn(false);
        }
    }

    /**
     * Stop service thread.
     */
    private class StopServiceTask extends AsyncTask<Void, Void, Void> {

        private static final String TAG = "StopServiceTask";

        @Override
        protected void onPreExecute() {
            setServiceActivatedEnabled(false);
        }

        @Override
        protected Void doInBackground(Void... params) {
            Activity activity = getActivity();
            if (activity != null) {
                RegistrationApi.setServiceActivationState(false);
                disableRcsService();
                boolean isRoamingAuthorized = RcsSettings.getInstance()
                        .isRoamingAuthorized();

                Logger.d(TAG,
                        "StopServiceTask doInBackground() disable ChatMainActivity");

                RcseComponentController rcseComponentController = ApiManager
                        .getInstance().getRcseComponentController();
                if (rcseComponentController != null) {
                    rcseComponentController.onServiceActiveStatusChanged(false);
                } else {
                    Logger.e(
                            TAG,
                            "StopServiceTask doInBackground()"
                                    + "ApiManager.getInstance()." +
                                    "getRcseComponentController() is null");
                }
            } else {
                Logger.w(TAG, "activity is null, dialog become null.");
            }
            return null;
        }

        @Override
        protected void onPostExecute(Void result) {
            setServiceActivatedEnabled(true);
            setServiceActivatedStatus(false);
            setRoamingAuthorizedEnabled(false);
            setRoamingAuthorizedStatus(false);
        }
    }

    /**
     * This task is used to set up the value of roaming authorized status.
     *
     * @param result the new service activated status
     */
    /*
     * private class RoamingAuthorizeTask extends AsyncTask<Void, Void, Void> {
     *
     * private static final String TAG = "RoamingAuthorizeTask";
     *
     * private boolean mIsAuthorized = false;
     *
     * private RoamingAuthorizeTask(boolean isAuthorized) { mIsAuthorized =
     * isAuthorized; }
     *
     * @Override protected void onPreExecute() { if (mRcseRoming != null) {
     * mRcseRoming.setEnabled(false); } else { Logger.w(TAG,
     * "onPreExecute() mRcseRoming is null"); } }
     *
     * @Override protected Void doInBackground(Void... params) { Activity
     * activity = getActivity(); if (activity != null) {
     * RcsSettings.getInstance().setRoamingAuthorizationState(mIsAuthorized); }
     * else { Logger.w(TAG, "activity is null, dialog become null."); } return
     * null; }
     *
     * @Override protected void onPostExecute(Void result) { if (mRcseRoming !=
     * null) { setRoamingAuthorizedStatus(mIsAuthorized);
     * mRcseRoming.setEnabled(true); } else { Logger.w(TAG,
     * "mRcseServiceActivation is null"); } } }
     */

    /*
     * private class GetServiceStatusTask extends AsyncTask<Void, Void,
     * boolean[]> {
     *
     * private static final String TAG = "GetServiceStatusTask"; private static
     * final int INDEX_SERVICE_ACTIVATED = 0; private static final int
     * INDEX_ROAMING_AUTHORIZED = 1; private static final int RESULTS_COUNT = 2;
     *
     * @Override protected boolean[] doInBackground(Void... params) { boolean
     * isServiceActivated = RegistrationApi.isServiceActivated(); boolean
     * isRoamingAuthorized = RcsSettings.getInstance().isRoamingAuthorized();
     * boolean[] results = new boolean[RESULTS_COUNT];
     *
     * results[INDEX_SERVICE_ACTIVATED] = isServiceActivated;
     * results[INDEX_ROAMING_AUTHORIZED] = isRoamingAuthorized;
     *
     * RcseComponentController rcseComponentController =
     * ApiManager.getInstance().getRcseComponentController(); if
     * (rcseComponentController != null) {
     * rcseComponentController.onServiceActiveStatusChanged(isServiceActivated);
     * } else { Logger.e(TAG, "StopServiceTask doInBackground()" +
     * " ApiManager.getInstance().getRcseComponentController() is null"); }
     * return results; }
     *
     * @Override protected void onPostExecute(boolean[] results) { boolean
     * isServiceActivated = results[INDEX_SERVICE_ACTIVATED];
     * setServiceActivatedStatus(isServiceActivated);
     *
     * // Only when RCS-e service is activated can user modify the roaming //
     * authorization status setRoamingAuthorizedEnabled(isServiceActivated);
     *
     * setRoamingAuthorizedStatus(results[INDEX_ROAMING_AUTHORIZED]); } }
     */

    private void setServiceActivatedStatus(boolean result) {
        Logger.d(TAG, "setServiceActivatedStatus() result is " + result);
        if (mRcseServiceActivation != null) {
            mRcseServiceActivation.setChecked(result);
            if (result) {
                mRcseServiceActivation
                        .setSummary(R.string.rcs_settings_summary_rcs_service_on);
            } else {
                mRcseServiceActivation
                        .setSummary(R.string.rcs_settings_summary_rcs_service_off);
            }
        } else {
            Logger.w(TAG,
                    "setServiceActivatedStatus() mRcseServiceActivation is null");
        }
    }

    /**
     * Sets the service activated enabled.
     *
     * @param isEnabled the new service activated enabled
     */
    private void setServiceActivatedEnabled(boolean isEnabled) {
        Logger.d(TAG, "setServiceActivatedStatus() result is " + isEnabled);
        if (mRcseServiceActivation != null) {
            mRcseServiceActivation.setEnabled(isEnabled);
        } else {
            Logger.w(TAG,
                    "setServiceActivatedStatus() mRcseServiceActivation is null");
        }
    }

    /**
     * Sets the roaming authorized status.
     *
     * @param result the new roaming authorized status
     */
    private void setRoamingAuthorizedStatus(boolean result) {
        Logger.d(TAG, "setRoamingAuthorizedStatus() result is " + result);
        if (mRcseRoming != null) {
            mRcseRoming.setChecked(result);
        } else {
            Logger.w(TAG, "setRoamingAuthorizedStatus() mRcseRoming is null");
        }
    }

    /**
     * Sets the roaming authorized enabled.
     *
     * @param isEnabled the new roaming authorized enabled
     */
    private void setRoamingAuthorizedEnabled(boolean isEnabled) {
        Logger.d(TAG, "setRoamingAuthorizedEnabled() result is " + isEnabled);
        if (mRcseRoming != null) {
            mRcseRoming.setEnabled(isEnabled);
        } else {
            Logger.w(TAG, "setRoamingAuthorizedEnabled() mRcseRoming is null");
        }
    }

    /**
     * The Class ConfirmDialog.
     */
    public static class ConfirmDialog extends DialogFragment implements
            DialogInterface.OnClickListener {
        private static final String TAG = "ConfirmDialog";
        private int mMessageResId;
        private RcseSystemSettingsFragment mFragment = null;
        StartServiceTask mStartServiceTask = null;
        StopServiceTask mStopServiceTask = null;

        public ConfirmDialog(RcseSystemSettingsFragment fragment,
                StartServiceTask startServiceTask,
                StopServiceTask stopServiceTask) {
            mFragment = fragment;
            mStartServiceTask = startServiceTask;
            mStopServiceTask = stopServiceTask;
        }

        @Override
        public void onCancel(DialogInterface dialog) {
            super.onCancel(dialog);
            switch (mMessageResId) {
            case R.string.rcse_settings_enable_rcse_service:
            case R.string.rcse_settings_disable_rcse_service:
                if (mFragment != null && mFragment.mRcseServiceActivation != null) {
                    mFragment.mRcseServiceActivation.setChecked(!mFragment.mRcseServiceActivation
                            .isChecked());
                } else {
                    Log.e(TAG, "onCancel() mRcseServiceActivation is null");
                }
                break;
            case R.string.rcse_settings_enable_rcse_service_roaming:
            case R.string.rcse_settings_disable_rcse_service_roaming:
                if (mFragment != null && mFragment.mRcseRoming != null) {
                    mFragment.mRcseRoming.setChecked(!mFragment.mRcseRoming.isChecked());
                } else {
                    Log.e(TAG, "onCancel() mRcseRoming is null");
                }
                break;
            default:
                break;
            }

        }

        @Override
        public Dialog onCreateDialog(Bundle savedInstanceState) {
            Activity activity = getActivity();
            mMessageResId = getArguments().getInt(MESSAGE_RCS_ID);
            if (activity != null) {
                return new AlertDialog.Builder(activity,
                        AlertDialog.THEME_HOLO_LIGHT)
                        .setIconAttribute(android.R.attr.alertDialogIcon)
                        .setTitle(R.string.attention_title)
                        .setMessage(getString(mMessageResId))
                        .setPositiveButton(
                                getString(R.string.rcs_dialog_positive_button),
                                this)
                        .setNegativeButton(
                                getString(R.string.rcs_dialog_negative_button),
                                this).create();
            } else {
                Logger.w(TAG, "activity is null, dialog become null.");
                return null;
            }
        }

        @Override
        public void onClick(DialogInterface dialog, int which) {
            if(mFragment != null) {
                if (which == DialogInterface.BUTTON_POSITIVE) {
                    switch (mMessageResId) {
                    case R.string.rcse_settings_enable_rcse_service:
                        Logger.v(TAG, "enableRcsService task");
                        if (mStartServiceTask != null) mStartServiceTask.execute();
                        break;
                    case R.string.rcse_settings_disable_rcse_masterswitch:
                        Logger.v(TAG, "enableRcsService task");
                        mFragment.mRCSeMasterSwitch.setChecked(false);
                        Calendar c = Calendar.getInstance();
                        SimpleDateFormat df = new SimpleDateFormat(
                                "yyyy-MM-dd HH:mm:ss");
                        mFragment.setDisableTime(df.format(c.getTime()));
                        Intent it = new Intent();
                        it.setAction(IpMessageConsts.DisableServiceStatus.
                                ACTION_DISABLE_SERVICE_STATUS);
                        it.putExtra("status", DEACTIVATE_JOYN_PERMANENTLY);
                        MediatekFactory.getApplicationContext().sendBroadcast(it);
                        AppSettings.getInstance().setDisableServiceStatus(
                                DEACTIVATE_JOYN_PERMANENTLY);
                        RcsSettings.getInstance().setProvisioningVersion(
                                DISABLE_JOYN_VERSION);
                        mFragment.mRCSeMasterSwitch
                                .setSummary("Joyn disabled permanently at "
                                        + sDisableTime);
                        if (mStopServiceTask != null) mStopServiceTask.execute();
                        break;
                    case R.string.rcse_settings_disable_rcse_service:
                        Logger.v(TAG, "disbleRcsService task");
                        if (mStopServiceTask != null) mStopServiceTask.execute();
                        break;
                    /*
                     * case R.string.rcse_settings_enable_rcse_service_roaming:
                     * Logger.v(TAG, "enbleRcsService roaming"); new
                     * RoamingAuthorizeTask(true).execute(); break; case
                     * R.string.rcse_settings_disable_rcse_service_roaming:
                     * Logger.v(TAG, "disbleRcsService roaming"); new
                     * RoamingAuthorizeTask(false).execute(); break;
                     */
                    default:
                        Logger.w(TAG, "Unknown op code");
                        break;
                    }
                } else {
                    if ((mMessageResId == R.string.rcse_settings_enable_rcse_service ||
                            mMessageResId == R.string.rcse_settings_disable_rcse_service)
                            && mFragment.mRcseServiceActivation != null) {
                        // Cancel enable or disable rcse service
                        mFragment.mRcseServiceActivation.setChecked(!mFragment.mRcseServiceActivation
                                .isChecked());
                    } else if ((mMessageResId == R.string.rcse_settings_enable_rcse_service_roaming ||
                            mMessageResId == R.string.rcse_settings_disable_rcse_service_roaming)
                            && mFragment.mRcseRoming != null) {
                        // Cancel enable or disable rcse service roaming
                        mFragment.mRcseRoming.setChecked(!mFragment.mRcseRoming.isChecked());
                    } else {
                        Logger.w(TAG, "Some error occur when cancel.");
                        mFragment.mRCSeMasterSwitch.setChecked(true);
                        mFragment.mRCSeMasterSwitch.setSummary("Joyn is activated");
                    }
                }
            }
            dismiss();
        }
    }

    /**
     * Sets the up debug items.
     *
     * @param isDebug the new up debug items
     */
    private void setupDebugItems(final boolean isDebug) {
        Logger.d(TAG, "setupDebugItems() entry with isDebug " + isDebug);
        Resources resources = getResources();
        String profileKey = resources
                .getString(R.string.rcs_settings_title_provision_settings);
        Preference profilePreference = findPreference(profileKey);
        if (isDebug) {
            Logger.d(TAG, "setupDebugItems() this is a debug version");
            profilePreference
                    .setOnPreferenceClickListener(new OnPreferenceClickListener() {

                        @Override
                        public boolean onPreferenceClick(Preference preference) {
                            Intent intent = new Intent(ACTION_PROVISION_PROFILE);
                            try {
                                startActivity(intent);
                            } catch (ActivityNotFoundException e) {
                                Toast.makeText(
                                        RcseSystemSettingsFragment.this
                                                .getActivity(),
                                        R.string.rcse_toast_profile_not_exist,
                                        Toast.LENGTH_LONG).show();
                                return false;
                            }
                            return true;
                        }
                    });
        } else {
            Logger.d(TAG,
                    "setupDebugItems() this is a user version, remove some debug items");
            String mapKey = resources
                    .getString(R.string.number_to_account_settings);
            Preference mapPreference = findPreference(mapKey);
            getPreferenceScreen().removePreference(profilePreference);
            getPreferenceScreen().removePreference(mapPreference);
        }
    }

    /**
     * M: added to resolve multiple RCSe client case @(.
     */

    // start a Async task to search any other RCS client
    private void searchOtherRCSeClient() {

        // execute Async task
        SearchRCSeClientAsynTask searchTask = new SearchRCSeClientAsynTask(
                getActivity());
        searchTask.execute();

    }

    // async task for searching any async client
    /**
     * The Class SearchRCSeClientAsynTask.
     */
    private class SearchRCSeClientAsynTask extends
            AsyncTask<Void, Void, ApplicationInfo> {

        private Context mAppContext = null;

        // strings for shared pref and key pair
        private static final String GSM_JOYN_SETTINGS_ACTIVITY_NAME = "gsma.joyn.settings.activity";
        private static final String GSM_JOYN_CLIENT_NAME = "gsma.joyn.client";
        private static final String GSM_JOYN_ENABLED = "gsma.joyn.enabled";
        private static final String SHARED_PREF_DIR = "shared_prefs";
        private static final String GSM_JOYN_SHARED_PREF_FILE_PATH = "gsma.joyn.preferences.xml";

        /**
         * Instantiates a new search rc se client asyn task.
         *
         * @param context the context
         */
        public SearchRCSeClientAsynTask(Context context) {
            // TODO Auto-generated constructor stub
            mAppContext = context;

        }

        @Override
        protected void onPostExecute(ApplicationInfo joynEnabledAppInfo) {
            // TODO Auto-generated method stub
            super.onPostExecute(joynEnabledAppInfo);

            if (mProgressDialog != null) {
                mProgressDialog.dismiss();
            }

            // if some joyn app exits where rcs service is enabled
            if (joynEnabledAppInfo != null) {

                Logger.d("RcseSystemSettingsActivity",
                        "Joyyn service is enabled in : "
                                + joynEnabledAppInfo.packageName);

                // get the settings activity of the client from meta data
                String settingActivityMetadata = joynEnabledAppInfo.metaData
                        .getString(GSM_JOYN_SETTINGS_ACTIVITY_NAME, "");

                // if setting activity exists for the joyn client
                if (!settingActivityMetadata.equals("")) {

                    // create intent
                    Intent intent = new Intent();
                    intent.setAction(settingActivityMetadata);

                    // check if activity at third party exists for handling the
                    // intent
                    PackageManager packageManager = mAppContext
                            .getPackageManager();
                    List<ResolveInfo> activities = packageManager
                            .queryIntentActivities(intent, 0);

                    if (activities != null) {
                        boolean isIntentSafe = activities.size() > 0;
                        // show dialog notification to the user
                        showRcsRunningDialog(mAppContext, intent);
                    } else { // if no activity of third party are present to
                             // handle the intent
                             // show the dialog notification to close the
                             // third party because there is no activity of
                             // third party to be launched
                             // from our application
                        showCloseOtherRCSDialog(mAppContext);
                    }
                }
            } else {
                // process the rcse service request NORMAL BEHAVIOUR

                mRCSeMasterSwitch.setChecked(true);
                mRCSeMasterSwitch.setSummary("Joyn is activated");
                Intent it = new Intent();
                it.setAction(IpMessageConsts.DisableServiceStatus.ACTION_DISABLE_SERVICE_STATUS);
                it.putExtra("status", ENABLE_JOYN);
                MediatekFactory.getApplicationContext().sendBroadcast(it);

                /*
                 * if(RcsSettings.getInstance().getDisableServiceStatus()==
                 * DEACTIVATE_JOYN_PERMANENTLY) {
                 * RcsSettings.getInstance().setProvisioningVersion
                 * (VERSION_RESETED); }
                 */

                final String[] missingPermissions = RcsOsUtils.getApkMissingRequiredPermissions();
                if (missingPermissions.length > 0) {
                    requestPermissions(missingPermissions,
                        REQUIRED_PERMISSIONS_REQUEST_CODE);
                }

                AppSettings.getInstance().setDisableServiceStatus(ENABLE_JOYN);
                new StartServiceTask().execute();
            }
        }

        @Override
        protected void onPreExecute() {
            // TODO Auto-generated method stub
            super.onPreExecute();

            mProgressDialog = new ProgressDialog(mAppContext);
            mProgressDialog.setMessage("Searching other Joyn apps...");
            mProgressDialog.show();

        }

        protected ApplicationInfo doInBackground(Void... params) {
            // TODO Auto-generated method stub
            PackageManager packageManager = mAppContext.getPackageManager();
            List<ApplicationInfo> packages = packageManager
                    .getInstalledApplications(PackageManager.GET_META_DATA);

            // information about the JOYN app in which rcse is enable currently
            ApplicationInfo joynEnabledAppInfo = null;

            for (ApplicationInfo appInfo : packages) {

                Logger.d("RcseSystemSettingsActivity", "appInfo : "
                        + appInfo.packageName + " ; metadata : "
                        + appInfo.metaData);

                // if own application or app is not enabled , skip it
                if ((!appInfo.enabled)
                        || (appInfo.packageName.equals(getActivity()
                                .getApplication().getPackageName()))) {
                    Logger.d("RcseSystemSettingsActivity", "my app ");
                    continue;
                }

                if (appInfo.metaData != null) {
                    boolean isAppJoynClient = appInfo.metaData.getBoolean(
                            GSM_JOYN_CLIENT_NAME, false);

                    if (isAppJoynClient) {
                        Logger.d("RcseSystemSettingsActivity",
                                "isAppJoynClient : enabled");

                        try {

                            // read from the preference file the if the client
                            // is enabled or not
                            String otherAppPath = appInfo.dataDir.toString();
                            otherAppPath += "/" + SHARED_PREF_DIR + "/"
                                    + GSM_JOYN_SHARED_PREF_FILE_PATH;

                            // is joyn enabled on third party client
                            boolean isOtherJoynAppEnabled = isJoynEnabled(otherAppPath);

                            // if joyn is enabled then set the app info
                            if (isOtherJoynAppEnabled) {
                                joynEnabledAppInfo = appInfo;
                                break;
                            }
                        } catch (Exception e) {
                            // TODO: handle exception
                            Logger.d("RcseSystemSettingsActivity", "exception:"
                                    + e.toString());
                            // TODO Auto-generated catch block
                            e.printStackTrace();
                        } // try-catch block ends

                    }
                }
            } // for loop ends

            return joynEnabledAppInfo;
        }

        // this function checks if the joyn is enabled for the tghird party app.
        // the path referes to the path of the shared preference file
        /**
         * Checks if is joyn enabled.
         *
         * @param path the path
         * @return true, if is joyn enabled
         */
        public boolean isJoynEnabled(String path) {

            boolean isJoynEnabled = false;

            XmlPullParser parser = Xml.newPullParser();
            File sharedPrefFile = null;
            FileInputStream fileInputStream = null;
            InputStreamReader inputStreamReader = null;
            try {

                sharedPrefFile = new File(path);
                fileInputStream = new FileInputStream(sharedPrefFile);
                inputStreamReader = new InputStreamReader(fileInputStream);

                // auto-detect the encoding from the stream
                parser.setInput(inputStreamReader);
                int eventType = parser.getEventType();
                boolean done = false;
                while (eventType != XmlPullParser.END_DOCUMENT && !done) {
                    String name = null;
                    String value = null;
                    String booleanname = null;

                    // switch case
                    switch (eventType) {
                    case XmlPullParser.START_DOCUMENT:
                        break;
                    case XmlPullParser.START_TAG:
                        name = parser.getName();

                        if (name != null && name.equals("boolean")) {
                            booleanname = parser
                                    .getAttributeValue(null, "name");

                            // read the boolean value of gsma.joyn.enable
                            if (booleanname != null
                                    && booleanname.equals(GSM_JOYN_ENABLED)) {
                                value = parser.getAttributeValue(null, "value");
                                if (value != null) {
                                    isJoynEnabled = Boolean.parseBoolean(value);
                                    done = true;
                                    break;
                                }
                            }
                        }
                        break;
                    case XmlPullParser.END_TAG:
                        name = parser.getName();
                        break;
                    default:
                        break;
                    }
                    eventType = parser.next();
                }

            } catch (FileNotFoundException e) {
                // TODO Auto-generated catch block
                e.printStackTrace();
            } catch (XmlPullParserException e) {
                // TODO Auto-generated catch block
                e.printStackTrace();
            } catch (IOException e) {
                // TODO Auto-generated catch block
                e.printStackTrace();
            } catch (Exception e) {
                e.printStackTrace();
            } finally {
                parser = null;

            }

            try {
                fileInputStream.close();
                inputStreamReader.close();
                sharedPrefFile = null;

            } catch (IOException e) {
                // TODO Auto-generated catch block
                e.printStackTrace();
            }

            return isJoynEnabled;

        }
    }

    // show dialog message if other RCS Client is running
    /**
     * Show rcs running dialog.
     *
     * @param appContext the app context
     * @param intent the intent
     */
    private void showRcsRunningDialog(final Context appContext,
            final Intent intent) {
        AlertDialog.Builder alertDialog = new AlertDialog.Builder(appContext);

        // Setting Dialog Title
        alertDialog.setTitle(R.string.third_party_detected_dialog);

        // Setting Dialog Message
        alertDialog.setMessage(R.string.third_party_detected_message);

        alertDialog.setCancelable(false);

        alertDialog.setPositiveButton(
                R.string.third_party_detected_message_yes,
                new DialogInterface.OnClickListener() {
                    public void onClick(DialogInterface dialog, int which) {
                        // go to the setting activity

                        try {
                            appContext.startActivity(intent);
                        } catch (Exception e) {
                            // TODO Auto-generated catch block
                            e.printStackTrace();
                        }
                    }
                });

        // Setting Negative "NO" Button
        alertDialog.setNegativeButton(R.string.third_party_detected_message_no,
                new DialogInterface.OnClickListener() {
                    public void onClick(DialogInterface dialog, int which) {
                        getActivity().finish();
                    }
                });

        // Showing Alert Message
        alertDialog.show();
    }

    // show dialog to close the other joyn client and then start our own client
    /**
     * Show close other rcs dialog.
     *
     * @param appContext the app context
     */
    private void showCloseOtherRCSDialog(final Context appContext) {
        AlertDialog.Builder alertDialog = new AlertDialog.Builder(appContext);

        // Setting Dialog Title
        alertDialog.setTitle(R.string.third_party_detected_dialog);

        // Setting Dialog Message
        alertDialog.setMessage(R.string.third_party_close_first);

        alertDialog.setCancelable(true);
        alertDialog.setPositiveButton(R.string.third_party_detected_message_no,
                new DialogInterface.OnClickListener() {
                    public void onClick(DialogInterface dialog, int which) {
                        getActivity().finish();
                    }
                });
    }
    /**
     * @}
     */

    private void setupOp07Button() {
        mOp07Button = (Preference) findPreference(RCS_OP07_TEST);
        mOp07Button.setOnPreferenceClickListener(new OnPreferenceClickListener() {
                @Override
                public boolean onPreferenceClick(Preference arg0) {
                    Logger.d(TAG, "mOp07ButtonHandler");

                    final String[] missingPermissions =
                            RcsOsUtils.getApkMissingRequiredPermissions();
                    if (missingPermissions.length > 0) {
                        requestPermissions(missingPermissions,
                                REQUIRED_PERMISSIONS_REQUEST_CODE);
                    }

                    // Step 1. Update file transfer size for 10MB video
                    // Configure 80 * 1024 Kb
                    RcsSettings.getInstance().writeParameter(
                            RcsSettingsData.MAX_FILE_TRANSFER_SIZE, "81920");
                    RcsSettings.getInstance().writeParameter(
                            RcsSettingsData.FT_CAPABILITY_ALWAYS_ON, Boolean.toString(true));
                    AppSettings.getInstance().writeParameter(
                            AppSettingsData.RCSE_COMPRESSING_VIDEO, Boolean.toString(false));
                    AppSettings.getInstance().writeParameter(
                            AppSettingsData.RCSE_COMPRESSING_IMAGE, Boolean.toString(false));

                    // Clean old contaacts
                    new Thread(new Runnable() {
                        @Override
                        public void run() {
                            try {
                                deleteContact();

                                // Step Inserts test contacts
                                String name = "";
                                String number = "";
                                for (int i = 0; i < OP07_TEST_CONTACT_LIST.length; i++) {
                                    name = OP07_TEST_CONTACT_LIST[i] + "_" + i;
                                    number = "+1302634012" + (i + 1);
                                    Logger.d(TAG, i + ":" + name + ":" + number);
                                    insertContact(name, number);
                                }
                            } catch (Exception e) {
                                e.printStackTrace();
                            }
                        }
                    }, RCS_OP07_TEST).start();
                    return true;
                }
        });
    }

    private boolean insertContact(String firstName, String mobileNumber) {
        ArrayList<ContentProviderOperation> ops = new ArrayList<ContentProviderOperation>();
        ops.add(ContentProviderOperation.newInsert(ContactsContract.RawContacts.CONTENT_URI)
                .withValue(ContactsContract.RawContacts.ACCOUNT_TYPE, null)
                .withValue(ContactsContract.RawContacts.ACCOUNT_NAME, null).build());
        ops.add(ContentProviderOperation.newInsert(ContactsContract.Data.CONTENT_URI)
                .withValueBackReference(ContactsContract.Data.RAW_CONTACT_ID, 0)
                .withValue(ContactsContract.Data.MIMETYPE,ContactsContract.CommonDataKinds.StructuredName.CONTENT_ITEM_TYPE)
                .withValue(ContactsContract.CommonDataKinds.StructuredName.GIVEN_NAME, firstName).build());
        ops.add(ContentProviderOperation.newInsert(ContactsContract.Data.CONTENT_URI)
                .withValueBackReference(ContactsContract.Data.RAW_CONTACT_ID, 0)
                .withValue(ContactsContract.Data.MIMETYPE,ContactsContract.CommonDataKinds.Phone.CONTENT_ITEM_TYPE)
                .withValue(ContactsContract.CommonDataKinds.Phone.NUMBER, mobileNumber)
                .withValue(ContactsContract.CommonDataKinds.Phone.TYPE,Phone.TYPE_MOBILE).build());
        try {
            getActivity().getApplicationContext().
                getContentResolver().applyBatch(ContactsContract.AUTHORITY, ops);
        } catch (Exception e) {
            return false;
        }
        return true;
    }

    private void deleteContact() {

        ArrayList<ContentProviderOperation> ops = new ArrayList<ContentProviderOperation>();
        ops.add(ContentProviderOperation.newDelete(RawContacts.CONTENT_URI).build());
        try {
            getActivity().getApplicationContext().
                    getContentResolver().applyBatch(ContactsContract.AUTHORITY, ops);
        } catch (Exception e) {
            e.printStackTrace();
        }
    }
}
