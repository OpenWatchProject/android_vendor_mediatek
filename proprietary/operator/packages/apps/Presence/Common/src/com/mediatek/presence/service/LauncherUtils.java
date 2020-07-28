/*******************************************************************************
 * Software Name : RCS IMS Stack
 *
 * Copyright (C) 2010 France Telecom S.A.
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
 ******************************************************************************/

package com.mediatek.presence.service;

import java.util.Date;
import java.util.List;

import android.app.Activity;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.SharedPreferences;
import com.android.internal.telephony.TelephonyIntents;
import android.net.ConnectivityManager;
import android.os.AsyncTask;
import android.os.Environment;
import android.os.RemoteException;
import android.os.SystemProperties;
import android.preference.PreferenceManager;
import android.telephony.TelephonyManager;
import android.widget.Toast;
import android.telephony.SubscriptionManager;
import android.support.v4.content.LocalBroadcastManager;

import com.android.internal.telephony.TelephonyProperties;
import com.android.internal.telephony.IccCardConstants;
import com.mediatek.common.carrierexpress.CarrierExpressManager;
import com.mediatek.presence.platform.AndroidFactory;
import com.mediatek.presence.platform.registry.AndroidRegistryFactory;
import com.mediatek.presence.provider.eab.ContactsBackup;
import com.mediatek.presence.provider.eab.ContactsBackupHelper;
import com.mediatek.presence.provider.eab.ContactsManager;
import com.mediatek.presence.provider.eab.RichAddressBookProvider;
import com.mediatek.presence.provider.settings.RcsSettings;
import com.mediatek.presence.utils.logger.Logger;
import com.mediatek.presence.provider.settings.RcsSettings;
import com.mediatek.presence.provider.settings.RcsSettingsData;
import com.mediatek.presence.provider.settings.RcsSettingsManager;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.ArrayList;

/**
 * Launcher utility functions
 *
 * @author hlxn7157
 */
public class LauncherUtils {
    /**
     * Logger
     */
    private static Logger logger = Logger.getLogger(LauncherUtils.class.getName());

    /**
     * Data SIM change listener
     */
    private static BroadcastReceiver defaultSimStateChangeListener = null;
    /**
     * Last user account used
     */
    public static final String REGISTRY_LAST_USER_ACCOUNT = "LastUserAccount";

    /** M: if no vodafone sim card, disable service @{ */
    private static final int[] OP01_NUMBERICS = {
            46000, 46002, 46007, 46008
    };

    /**
     * M: Debug mode flag.@{
     */
    //public static boolean sIsDebug = true;
    public static final String DEBUG_FORCEUSE_ONLYAPN_ACTION = "com.mediatek.rcse.service.ENABLE_ONLYAPN";
    public static final boolean DEBUG_ENABLE_ONLY_APN_FEATURE = false;

    /**
     * @}
     */
    /**
     * Key for storing the latest positive provisioning version
     */
    private static final String REGISTRY_PROVISIONING_VERSION = "ProvisioningVersion";
    public static final String CORE_CONFIGURATION_STATUS = "status";
    private static final String REGISTRY_CLIENT_VENDOR= "clientsvendor";
    private static final String REGISTRY_CLIENT_VERSION = "clientsversion";

    /**
     * Key for storing the latest positive provisioning validity
     */
    private static final String REGISTRY_PROVISIONING_VALIDITY = "ProvisioningValidity";

    /**
     * Key for storing the expiration date of the provisioning
     */
    private static final String REGISTRY_PROVISIONING_EXPIRATION = "ProvisioningExpiration";

    /**
     * secondary device mode is on
     */
    private final static boolean isSecondaryDevice = false;
    private static boolean WAITING_FOR_TELEPHONY = true;

    /**
     *  Subscription ID
     */
    private static Long mSubID = null;
    private final static boolean DBG = logger.isActivated();

    public static final String UCE_CORE_SERVICE_STATE_STARTSERVICE_ON
        = "com.mediatek.uce.service.startservice.on";
    public static final String UCE_CORE_SERVICE_STATE_STARTSERVICE_OFF
        = "com.mediatek.uce.service.startservice.off";
    public static final String UCE_CORE_SERVICE_STATE_RCSCORESERVICE_ON
        = "com.mediatek.uce.service.rcscoreservice.on";
    public static final String UCE_CORE_SERVICE_STATE_RCSCORESERVICE_OFF
        = "com.mediatek.uce.service.rcscoreservice.off";

    private static boolean mIsStartServiceAlive = false;
    private static boolean mIsRcsCoreServiceAlive = false;
    private static boolean mPendingStartRcs = false;


    /**
     * Launch the RCS service
     *
     * @param context application context
     * @param boot indicates if RCS is launched from the device boot
     */
    public synchronized static void launchRcsService(final Context context, boolean boot, boolean user) {

        logger.debug("mIsStartServiceAlive: " + mIsStartServiceAlive +
                    " mIsRcsCoreServiceAlive: " + mIsRcsCoreServiceAlive);
        if (mIsStartServiceAlive != false || mIsRcsCoreServiceAlive != false) {
            logger.debug("pending launch rcs service");
            mPendingStartRcs = true;
            return;
        }

        // Instantiate the settings manager
        RcsSettingsManager.createRcsSettings();

        TelephonyManager tm = (TelephonyManager) context.getSystemService(
                Context.TELEPHONY_SERVICE);
        if (tm == null) {
            logger.error("LaunchRcsService fail. TelephonyManager is null");
            return;
        }
        int simCount = tm.getSimCount();
        for (int slotId = 0; slotId < simCount; ++slotId) {
            RcsSettingsManager.getRcsSettingsInstance(slotId).setDefaultValuesConfig();
        }

        if (SystemProperties.getInt("persist.vendor.mtk_uce_support", 0) == 1) {
            logger.info("launchRcsService");
        } else {
            logger.info("don't launchRcsService");
            return;
        }

         /**
         * M: Added to send broadcast whether the device has SIM card. @{
         */
        if (context == null) {
            if (DBG) {
                logger.info("launchRcsService()-context is null");
            }
            return;
        } else {
            logger.info("launch StartService, boot = " + boot + ", user = " + user);
            Intent intent = new Intent(context, StartService.class);
            intent.putExtra("boot", boot);
            intent.putExtra("user", user);
            context.startService(intent);
        }
    }

    public synchronized static void updateRcsSettingsValue(int slotId) {
        RcsSettingsManager.getRcsSettingsInstance(slotId).setServiceActivationState(true);
        StartService.updateProvisionValue(slotId);
    }

    public static boolean checkSimCard() {
        if (DBG) {
            logger.debug("checkSimCard() entry");
        }
        return true;
    }

    private static boolean simCardMatched(String numberic) {
        if (DBG) {
            logger.debug("simCardMatched() entry, numberic: " + numberic);
        }
        if ("-1".equals(numberic)) {
            return false;
        } else {
            return true;
        }
    }


    private static boolean isOP01SimCard(String numberic) {
        if (DBG) {
            logger.debug("isOP01SimCard entry, numberic: " + numberic);
        }
        if (!numberic.isEmpty()) {
            boolean result = binarySearch(OP01_NUMBERICS, Integer.valueOf(numberic));
            if (DBG) {
                logger.debug("isOP01SimCard exit, result: " + result);
            }
            return result;
        }
        return false;
    }

    private static boolean binarySearch(int[] list, int value) {
        int low = 0;
        int mid = 0;
        int high = list.length - 1;
        while (low <= high) {
            mid = (high + low) / 2;
            if (list[mid] == value) {
                return true;
            }
            if (list[mid] < value) {
                low = mid + 1;
            } else {
                high = mid - 1;
            }
        }
        return false;
    }

    /** @} */
    /**
     * Launch the RCS core service
     *
     * @param context Application context
     */
    public static void launchRcsCoreService(final Context context) {
        if (SystemProperties.getInt("persist.vendor.mtk_uce_support", 0) == 1) {
            logger.info("launchRcsCoreService");
        } else {
            logger.info("don't launchRcsCoreService");
            return;
        }
        //if (isServiceActivated) {
            Intent intent = new Intent(context, RcsCoreService.class);
            context.startService(intent);
        //} else {
        //    logger.debug("launchRcsCoreService fail");
        //}
    }

    private static String extractUserNamePart(String uri) {
        if ((uri == null) || (uri.trim().length() == 0)) {
            return "";
        }

        try {
            uri = uri.trim();
            int index1 = uri.indexOf("sip:");
            if (index1 != -1) {
                int index2 = uri.indexOf("@", index1);
                String result = uri.substring(index1+4, index2);
                return result;
            } else {
                return uri;
            }
        } catch(Exception e) {
            return "";
        }
    }

    /**
     * Stop the RCS service
     *
     * @param context Application context
     */
    public static void stopRcsService(Context context) {
        if (DBG) {
            logger.debug("Stop RCS service");
        }
        context.stopService(new Intent(context, StartService.class));
        context.stopService(new Intent(context, RcsCoreService.class));
    }
    /**
     * Stop the RCS core service (but keep provisioning)
     *
     * @param context Application context
     */
    public static void stopRcsCoreService( Context context) {
        if (DBG) {
            logger.debug("Stop RCS core service");
        }
        context.stopService(new Intent(context, StartService.class));
        context.stopService(new Intent(context, RcsCoreService.class));
    }

    /**
     * Get the last user account
     *
     * @param context Application context
     * @return last user account
     */
    /*
    public static String getLastUserAccount(Context context) {
        SharedPreferences preferences = context.getSharedPreferences(AndroidRegistryFactory.RCS_PREFS_NAME_ACC, Activity.MODE_PRIVATE);
        if (DBG) {
            logger.debug("Last User Account from preference:- "+ preferences.getString(REGISTRY_LAST_USER_ACCOUNT, null));
        }
        return preferences.getString(REGISTRY_LAST_USER_ACCOUNT, null);
    }*/

    /**
     * Set the last user account
     *
     * @param context Application context
     * @param value last user account
     */
    /*
    public static void setLastUserAccount(Context context, String value) {
        SharedPreferences preferences = context.getSharedPreferences(AndroidRegistryFactory.RCS_PREFS_NAME_ACC, Activity.MODE_PRIVATE);
        SharedPreferences.Editor editor = preferences.edit();
        editor.putString(REGISTRY_LAST_USER_ACCOUNT, value);
        editor.commit();
        if (DBG) {
            logger.debug("Setting Last User Account on preference:- "+ preferences.getString(REGISTRY_LAST_USER_ACCOUNT, null));
        }
    }*/

    /**
     * Get current user account
     *
     * @param slotId slot index
     * @param context Application context
     * @return current user account
     */
    /*
    public static String getCurrentUserAccount(int slotId, Context context) {
        int[] subIds = SubscriptionManager.getSubId(slotId);
        TelephonyManager mgr = TelephonyManager.from(context).createForSubscriptionId(subIds[0]);
        String currentUserAccount = mgr.getSubscriberId(subIds[0]);
        return currentUserAccount;
    }*/

    /** @} */

    public static void setDebugMode(Context context, boolean value ){
        if(DBG){
            SharedPreferences preferences = context.getSharedPreferences(AndroidRegistryFactory.RCS_PREFS_NAME, Activity.MODE_PRIVATE);
            SharedPreferences.Editor editor = preferences.edit();
            editor.putBoolean("DEBUG", value);
            editor.commit();
        }
    }

    public static boolean getDebugMode(Context context){
        Boolean debugState = false ;
        if (DBG) {
             SharedPreferences preferences = context.getSharedPreferences(AndroidRegistryFactory.RCS_PREFS_NAME, Activity.MODE_PRIVATE);
             debugState = preferences.getBoolean("DEBUG",false);
        }
        return debugState;
    }

    public static boolean isSecondaryDevice() {
        return isSecondaryDevice;
    }

    public static boolean isLoggerActivated(){
        boolean isLogger= false;
        isLogger= DBG;
        return isLogger;
    }

    private static BroadcastReceiver rcsServiceListener;
    public static void registerRcsServciceListener(Context context) {
        if (rcsServiceListener == null) {
            rcsServiceListener = new BroadcastReceiver() {
                @Override
                public void onReceive(Context context, Intent intent) {
                    logger.debug("onReceive: " + intent.getAction());
                    switch(intent.getAction()) {
                        case UCE_CORE_SERVICE_STATE_STARTSERVICE_ON:
                            mIsStartServiceAlive = true;
                            logger.debug("mIsStartServiceAlive: " + mIsStartServiceAlive);
                            break;
                        case UCE_CORE_SERVICE_STATE_RCSCORESERVICE_ON:
                            mIsRcsCoreServiceAlive = true;
                            logger.debug("mIsRcsCoreServiceAlive: " + mIsRcsCoreServiceAlive);
                            break;
                        case UCE_CORE_SERVICE_STATE_STARTSERVICE_OFF:
                            mIsStartServiceAlive = false;
                            logger.debug("mIsStartServiceAlive: " + mIsStartServiceAlive);
                            if (mPendingStartRcs == true) {
                                mPendingStartRcs = false;
                                launchRcsService(AndroidFactory.getApplicationContext(), true, false);
                            }
                            break;
                        case UCE_CORE_SERVICE_STATE_RCSCORESERVICE_OFF:
                            mIsRcsCoreServiceAlive = false;
                            logger.debug("mIsRcsCoreServiceAlive: " + mIsRcsCoreServiceAlive);
                            if (mPendingStartRcs == true) {
                                mPendingStartRcs = false;
                                launchRcsService(AndroidFactory.getApplicationContext(), true, false);
                            }
                            break;
                    }
                }
            };
        }

        IntentFilter filters = new IntentFilter();
        filters.addAction(UCE_CORE_SERVICE_STATE_STARTSERVICE_ON);
        filters.addAction(UCE_CORE_SERVICE_STATE_STARTSERVICE_OFF);
        filters.addAction(UCE_CORE_SERVICE_STATE_RCSCORESERVICE_ON);
        filters.addAction(UCE_CORE_SERVICE_STATE_RCSCORESERVICE_OFF);
        LocalBroadcastManager.getInstance(AndroidFactory.getApplicationContext())
                .registerReceiver(rcsServiceListener, filters);
    }

    public static void unregisterRcsServiceListener(Context context) {
        try {
            LocalBroadcastManager.getInstance(AndroidFactory.getApplicationContext())
                    .unregisterReceiver(rcsServiceListener);
        }catch(Exception e){}
    }

}
