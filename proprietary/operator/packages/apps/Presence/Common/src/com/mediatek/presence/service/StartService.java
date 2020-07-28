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

import java.io.IOException;
import java.util.ArrayList;
import java.util.concurrent.atomic.AtomicBoolean;

import org.xmlpull.v1.XmlPullParser;
import org.xmlpull.v1.XmlPullParserException;

import android.accounts.Account;
import android.app.Activity;
import android.app.Service;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.SharedPreferences;
import android.content.res.XmlResourceParser;
import android.net.NetworkInfo;
import android.os.AsyncTask;
import android.os.Build;
import android.os.IBinder;
import android.os.RemoteException;
import android.os.ServiceManager;
import android.telephony.SubscriptionManager;
import android.text.TextUtils;
import android.support.v4.content.LocalBroadcastManager;

import android.telephony.TelephonyManager;

import com.android.ims.ImsConfig;
import com.android.internal.telephony.ITelephony;
import com.android.internal.telephony.PhoneConstants;

import com.mediatek.ims.rcsua.RcsUaService;

import com.mediatek.presence.R;

import com.mediatek.presence.platform.AndroidFactory;
import com.mediatek.presence.platform.registry.AndroidRegistryFactory;
import com.mediatek.presence.provider.eab.ContactsManager;
import com.mediatek.presence.provider.settings.RcsSettings;
import com.mediatek.presence.provider.settings.RcsSettingsData;
import com.mediatek.presence.provider.settings.RcsSettingsManager;
import com.mediatek.presence.utils.PhoneUtils;
import com.mediatek.presence.utils.logger.Logger;
import com.mediatek.presence.provisioning.AcsProvisioningController;
import com.mediatek.internal.telephony.IMtkTelephonyEx;
import com.mediatek.presence.utils.SimUtils;

/**
 * RCS start service.
 *
 * @author hlxn7157
 */
public class StartService extends Service {
    /**
     * Service name
     */
    public static final String SERVICE_NAME = "com.mediatek.presence.service.START";

    /**
     * M:  Add for storing 3 SIM card user data info @{
     */
    /**
     * Indicates the last first user account used
     */
    public static final String REGISTRY_LAST_FIRST_USER_ACCOUNT = "LastFirstUserAccount";

    /**
     * Indicates the last second user account used
     */
    public static final String REGISTRY_LAST_SECOND_USER_ACCOUNT = "LastSecondUserAccount";

    /**
     * Indicates the last third user account used
     */
    public static final String REGISTRY_LAST_THIRD_USER_ACCOUNT = "LastThirdUserAccount";

    /**
     * Indicates that db changed for current account.
     */
    public static final String ACTION_DB_CHANGE = "com.mediatek.mms.ipmessage.dbChange";

    /**
     * Intent broadcasted when the RCS configuration status has changed (see constant attribute "status").
     *
     * <p>The intent will have the following extra values:
     * <ul>
     *   <li><em>status</em> - Configuration status.</li>
     * </ul>
     * </ul>
     */
    public final static String CONFIGURATION_STATUS = "com.mediatek.presence.CONFIGURATION_STATUS";

    /**
     * Indicates the last User icci account
     */
    private ArrayList<String> lastUserIcci = null;

    /**
     * Current user account used
     */
    public static final String REGISTRY_CURRENT_USER_ACCOUNT = "CurrentUserAccount";

    /**
     * RCS new user account
     */
    public static final String REGISTRY_NEW_USER_ACCOUNT = "NewUserAccount";

    /*
     * KEY FOR SHARED PREFERENCE FOR IMS SERVICES
     */
     //XDM SERVICE

    private static final String KEY_XDM_REQUEST_ENABLE = "XdmRequestEnable";

    private static final String KEY_FIRST_BOOT = "XdmFirstBoot";

    private static final String KEY_INITIALIZE_STATUS = "XdmInitializeStatus";
    /*
     * @:KEY FOR SHARED PREFERENCE FOR IMS SERVICES [ENDS]
     */

    /**
     * M:
     */
    /**
     * Indicate whether network state listener has been registered.
     */
    private final AtomicBoolean mIsRegisteredAtomicBoolean = new AtomicBoolean();


    /**
     * Launch boot flag
     */
    private boolean boot = false;

    public boolean user = false;

    private int mSimCount = 1;


    /**
     * The logger
     */
    private static Logger logger = Logger.getLogger(StartService.class.getSimpleName());

    private static final boolean DBG = logger.isActivated();

    //AT&T Provision Handler
    private Object mLock = new Object();
    private static AcsProvisioningController mAcsCtrl = null;

    @Override
    public void onCreate() {
        if (DBG) logger.debug("onCreate() called");

        TelephonyManager tm = (TelephonyManager) getApplicationContext().getSystemService(
                Context.TELEPHONY_SERVICE);
        if (tm == null) {
            logger.error("Get SIM count fail. TelephonyManager is null");
        } else {
            mSimCount = tm.getSimCount();
        }

        // AndroidFactory.setApplicationContext(getApplicationContext());
        // Instantiate RcsSettings
        RcsSettingsManager.createRcsSettings();

        synchronized(mLock) {
            if (mAcsCtrl == null) {
                mAcsCtrl = new AcsProvisioningController(getApplicationContext());
            }
        }

        Intent status = new Intent(LauncherUtils.UCE_CORE_SERVICE_STATE_STARTSERVICE_ON);
        LocalBroadcastManager.getInstance(getApplicationContext()).sendBroadcast(status);
    }

    @Override
    public void onDestroy() {
        synchronized(mLock) {
            if (mAcsCtrl != null) {
                mAcsCtrl = null;
            }
        }

        Intent status = new Intent(LauncherUtils.UCE_CORE_SERVICE_STATE_STARTSERVICE_OFF);
        LocalBroadcastManager.getInstance(getApplicationContext()).sendBroadcast(status);
    }

    @Override
    public IBinder onBind(Intent intent) {
        return null;
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        if (DBG) logger.debug("Start RCS service");

        // Check boot
        if (intent != null) {
            boot = intent.getBooleanExtra("boot", false);
            user = intent.getBooleanExtra("user", false);
        }

        AsyncTask.execute(new Runnable() {
            @Override
            public void run() {
                launchRcsService(boot, user);
            }
        });

        // We want this service to continue running until it is explicitly
        // stopped, so return sticky.
        return START_STICKY;
    }

    /**
     * Launch the RCS service.
     *
     * @param boot indicates if RCS is launched from the device boot
     * @param user indicates if RCS is launched from the user interface
     */
    private void launchRcsService(boolean boot, boolean user) {

        if (DBG) logger.debug("Launch RCS service: boot=" + boot + ", user=" + user);

        for (int slotId = 0; slotId < mSimCount; ++slotId) {
            if(boot){
                //set the boot flag initialization needed for some IMS service
                setIMSServicePropertyInSharedPrefs(slotId, boot);
            }

            setDefaultProvisioningValue(slotId);
            RcsSettingsManager.getRcsSettingsInstance(slotId)
                    .setConfigurationState(true);
        }
        // No auto config: directly start the RCS core service
        LauncherUtils.launchRcsCoreService(getApplicationContext());
    }

    private static void setDefaultProvisioningValue(int slotId) {
        if (!RcsSettingsManager.getRcsSettingsInstance(slotId)
                .isFirstLaunch()) {
            logger.debug("skip default value provisioning");
            return;
        }
        if (SimUtils.isTmoSimCard(slotId)){
            updateSIMDetailsinDB(slotId);
        } else if (SimUtils.isAttSimCard(slotId)) {
            mAcsCtrl.initDataBase(slotId);
        }
        RcsSettingsManager.getRcsSettingsInstance(slotId)
                .setFristLaunchState(false);

    }

    public static boolean updateSIMDetailsinDB(int slotId){

        boolean status = false;

        try {
            if (DBG) logger.debug("updateSIMDetailsinDB, slotId: " + slotId);
            if ((RcsSettingsManager.getRcsSettingsInstance(slotId)
                    .getImsAuhtenticationProcedureForMobile()== RcsSettingsData.AKA_AUTHENT)) {
                if (DBG) logger.debug("RcsSettingsData.AKA_AUTHENT : update RCS_setting fileds  ");

                int[] subIds = SubscriptionManager.getSubId(slotId);
                if (subIds == null) {
                    logger.debug("updateSIMDetailsinDB fail. subIds is null");
                    return false;
                }
                TelephonyManager tm = TelephonyManager.from(
                        AndroidFactory.getApplicationContext()).createForSubscriptionId(subIds[0]);

                String impi = tm.getIsimImpi();
                String[] impu = tm.getIsimImpu();
                String domain = tm.getIsimDomain();
                String MSISDN="";
                try {
                    MSISDN = extractUserNamePart(impu[0]);
                } catch (Exception e) {
                    MSISDN="";
                    if (DBG) logger.error("updateSIMDetailsinDB is MSISDN null");
                    e.printStackTrace();
                }
                if(MSISDN == null) {
                    MSISDN="";
                }
                RcsSettingsManager.getRcsSettingsInstance(slotId)
                        .setUserProfileImsUserName_full(impu[0]);
                RcsSettingsManager.getRcsSettingsInstance(slotId)
                        .setUserProfileImsPrivateId(impi);
                RcsSettingsManager.getRcsSettingsInstance(slotId)
                        .setUserProfileImsDomain(domain);
                RcsSettingsManager.getRcsSettingsInstance(slotId)
                        .setUserProfileImsDisplayName(MSISDN);
                RcsSettingsManager.getRcsSettingsInstance(slotId)
                        .setUserProfileImsUserName(MSISDN);
                RcsSettingsManager.getRcsSettingsInstance(slotId)
                        .setImConferenceUri("sip:adhoc@msg.pc.t-mobile.com");

                //XDM CHANGES
                RcsSettingsManager.getRcsSettingsInstance(slotId)
                        .setXdmServer("xcap.msg.pc.t-mobile.com");
                RcsSettingsManager.getRcsSettingsInstance(slotId)
                        .setXdmLogin(impu[0]);
                RcsSettingsManager.getRcsSettingsInstance(slotId)
                        .writeParameter(RcsSettingsData.XDM_AUTH_TYPE,"GBA");

                //SERVICE CHANGES
                RcsSettingsManager.getRcsSettingsInstance(slotId)
                        .writeParameter(RcsSettingsData.CAPABILITY_SOCIAL_PRESENCE, Boolean.toString(true));
                RcsSettingsManager.getRcsSettingsInstance(slotId)
                        .writeParameter(RcsSettingsData.CAPABILITY_PRESENCE_DISCOVERY, Boolean.toString(true));
                RcsSettingsManager.getRcsSettingsInstance(slotId)
                        .writeParameter(RcsSettingsData.AUTO_ACCEPT_CHAT,RcsSettingsData.TRUE);
                RcsSettingsManager.getRcsSettingsInstance(slotId)
                        .writeParameter(RcsSettingsData.AUTO_ACCEPT_GROUP_CHAT,RcsSettingsData.TRUE);
                RcsSettingsManager.getRcsSettingsInstance(slotId)
                        .writeParameter(RcsSettingsData.PERMANENT_STATE_MODE,RcsSettingsData.FALSE);
                RcsSettingsManager.getRcsSettingsInstance(slotId)
                        .writeParameter(RcsSettingsData.IMS_AUTHENT_PROCEDURE_WIFI, RcsSettingsData.AKA_AUTHENT);
                RcsSettingsManager.getRcsSettingsInstance(slotId)
                        .writeParameter(RcsSettingsData.IMS_AUTHENT_PROCEDURE_MOBILE, RcsSettingsData.AKA_AUTHENT);
                RcsSettingsManager.getRcsSettingsInstance(slotId)
                        .writeParameter(RcsSettingsData.RCS_VOLTE_SINGLE_REGISTRATION, "1");
                RcsSettingsManager.getRcsSettingsInstance(slotId)
                        .setCPMSupported(true);
                RcsSettingsManager.getRcsSettingsInstance(slotId)
                        .setServicePermissionState(true);
            }
           status = true;
       } catch (Exception err) {
           if (DBG) logger.debug("Exception while updating RCSsettng from SIm details for AKA ");
       }
       return status;
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
     * Set true if new user account
     *
     * @param slotId slot index
     * @param value true if new user account
     */
    private void setNewUserAccount(int slotId, boolean value) {
        SharedPreferences preferences = getSharedPreferences(AndroidRegistryFactory.RCS_PREFS_NAME, Activity.MODE_PRIVATE);
        SharedPreferences.Editor editor = preferences.edit();
        editor.putBoolean(REGISTRY_NEW_USER_ACCOUNT + String.valueOf(slotId), value);
        editor.commit();
    }

    /**
     * Check if new user account
     *
     * @param slotId slot index
     * @param context Application context
     * @return true if new user account
     */
    public static boolean getNewUserAccount(int slotId, Context context) {
        SharedPreferences preferences = context.getSharedPreferences(AndroidRegistryFactory.RCS_PREFS_NAME, Activity.MODE_PRIVATE);
        return preferences.getBoolean(REGISTRY_NEW_USER_ACCOUNT + String.valueOf(slotId), false);
    }

    /*
     * Set device boot flag. its for achaiveing one request per boot for XDM manager
     */
    public static void setXDMBootFlag(int slotId, Context context, boolean value) {
        SharedPreferences preferences = context.getSharedPreferences(AndroidRegistryFactory.RCS_PREFS_IMS_SERVICES, Activity.MODE_PRIVATE);
        SharedPreferences.Editor editor = preferences.edit();
        editor.putBoolean(KEY_XDM_REQUEST_ENABLE + String.valueOf(slotId), value);
        editor.commit();
    }

    public static boolean getXDMBootFlag(int slotId, Context context) {
        SharedPreferences preferences = context.getSharedPreferences(AndroidRegistryFactory.RCS_PREFS_IMS_SERVICES, Activity.MODE_PRIVATE);
        return preferences.getBoolean(KEY_XDM_REQUEST_ENABLE + String.valueOf(slotId), false);
    }

    public static void setFirstBootFlag(int slotId, Context context,boolean value) {
        SharedPreferences preferences = context.getSharedPreferences(AndroidRegistryFactory.RCS_PREFS_IMS_SERVICES, Activity.MODE_PRIVATE);
        SharedPreferences.Editor editor = preferences.edit();
        editor.putBoolean(KEY_FIRST_BOOT + String.valueOf(slotId), value);
        editor.commit();
    }

    public static boolean getFirstBootFlag(int slotId, Context context) {
        SharedPreferences preferences = context.getSharedPreferences(AndroidRegistryFactory.RCS_PREFS_IMS_SERVICES, Activity.MODE_PRIVATE);
        return preferences.getBoolean(KEY_FIRST_BOOT + String.valueOf(slotId), true);
    }

    public static void setXdmInitializeFlag(int slotId, Context context, boolean value) {
        SharedPreferences preferences = context.getSharedPreferences(AndroidRegistryFactory.RCS_PREFS_IMS_SERVICES, Activity.MODE_PRIVATE);
        SharedPreferences.Editor editor = preferences.edit();
        editor.putBoolean(KEY_INITIALIZE_STATUS + String.valueOf(slotId), value);
        editor.commit();
    }

    public static boolean getXdmInitializeFlag(int slotId, Context context) {
        SharedPreferences preferences = context.getSharedPreferences(AndroidRegistryFactory.RCS_PREFS_IMS_SERVICES, Activity.MODE_PRIVATE);
        return preferences.getBoolean(KEY_INITIALIZE_STATUS + String.valueOf(slotId), false);
    }

    private void setIMSServicePropertyInSharedPrefs(int slotId, boolean isBoot){
        //for XDM set its RCS service initialization via boot
        if (!SimUtils.isAttSimCard(slotId)) {
            setXDMBootFlag(slotId, getApplicationContext(),isBoot);
        }
    }

    public static AcsProvisioningController getAcsProvisioningController() {
        return mAcsCtrl;
    }

    public static void updateProvisionValue(int slotId) {
        setDefaultProvisioningValue(slotId);
        RcsSettingsManager.getRcsSettingsInstance(slotId)
                .setConfigurationState(true);
        
    }
}
