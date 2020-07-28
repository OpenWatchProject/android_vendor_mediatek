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

package com.orangelabs.rcs.service;

import java.io.IOException;
import java.util.ArrayList;
import java.util.concurrent.atomic.AtomicBoolean;

import org.xmlpull.v1.XmlPullParser;
import org.xmlpull.v1.XmlPullParserException;

import android.app.Activity;
import android.app.Service;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.SharedPreferences;
import android.content.res.XmlResourceParser;
import android.net.ConnectivityManager;
import android.net.NetworkInfo;
import android.os.AsyncTask;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.IBinder;
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;

import com.mediatek.ims.rcsua.RcsUaService;

import com.orangelabs.rcs.R;
import com.orangelabs.rcs.platform.AndroidFactory;
import com.orangelabs.rcs.platform.registry.AndroidRegistryFactory;
import com.orangelabs.rcs.provider.settings.RcsSettings;
import com.orangelabs.rcs.provider.settings.RcsSettingsData;
import com.orangelabs.rcs.provisioning.ProvisioningInfo;
import com.orangelabs.rcs.provisioning.https.HttpsProvisioningService;
import com.orangelabs.rcs.utils.logger.Logger;
import com.orangelabs.rcs.provisioning.AcsEventCallbackImpl;
import com.orangelabs.rcs.provisioning.https.HttpsProvisioningUtils;
import com.orangelabs.rcs.provisioning.https.HttpsProvisioningSMS;
/**
 * RCS start service.
 *
 * @author hlxn7157
 */
public class StartService extends Service {
    /**
     * Service name
     */
    public static final String SERVICE_NAME = "com.orangelabs.rcs.service.START";

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
    public final static String CONFIGURATION_STATUS = "com.orangelabs.rcs.CONFIGURATION_STATUS";

    /**
     * Indicates the last User icci account
     */
    private ArrayList<String> lastUserIcci = null;

    /**
     * Indicates the user account index included in lastUserAccount
     */
    private final static int LAST_FIRST_USER_ACCOUNT_INDEX = 0;
    private final static int LAST_SECOND_USER_ACCOUNT_INDEX = 1;
    private final static int LAST_THIRD_USER_ACCOUNT_INDEX = 2;
    /**
     * @}
     */

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
     * Connection manager
     */
    private ConnectivityManager connMgr = null;

    /**
     * Network state listener
     */
    private BroadcastReceiver networkStateListener = null;

    /**
     * Last User account
     */
    private String lastUserAccount = null;

    /**
     * Current User account
     */
    private String currentUserAccount = null;


    /**
     * M:
     */
    /**
     * Indicate whether network state listener has been registered.
     */
    private final AtomicBoolean mIsRegisteredAtomicBoolean = new AtomicBoolean();

    //add for auto config service (ACS)
    private AcsEventCallbackImpl mAcsEventCallbackImpl = null;

    /**
     * Launch boot flag
     */
    private boolean boot = false;
    public boolean user = false;

    private static final String INTENT_KEY_BOOT = "boot";
    private static final String INTENT_KEY_USER = "user";

    private Handler startServiceHandler;

    /**
     * The logger
     */
    private static Logger logger = Logger.getLogger(StartService.class.getSimpleName());

    /**
     * Account changed broadcast receiver
     */
    private HttpsProvisioningSMS reconfSMSReceiver = null;


    @Override
    public void onCreate() {
        if (logger.isActivated()) {
            logger.debug("onCreate() called");
        }

        // Instantiate RcsSettings
        RcsSettings.createInstance(getApplicationContext());
    }

    private Handler allocateBgHandler(String threadName) {
        HandlerThread thread = new HandlerThread(threadName);
        thread.start();
        return new Handler(thread.getLooper());
    }

    @Override
    public void onDestroy() {
        // Unregister network state listener
        if (networkStateListener != null && mIsRegisteredAtomicBoolean.compareAndSet(true, false)) {
            try {
                unregisterReceiver(networkStateListener);
                   networkStateListener = null;
            } catch (IllegalArgumentException e) {
                // Nothing to do
            }
        }

        //add for auto config service (ACS)
        if (mAcsEventCallbackImpl != null) {
            mAcsEventCallbackImpl.releaseAcsService();
            mAcsEventCallbackImpl = null;
        }

        UaServiceManager.getInstance().stopService();
    }

    @Override
    public IBinder onBind(Intent intent) {
        return null;
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        if (logger.isActivated()) {
            logger.debug("Start RCS service");
        }

        if (startServiceHandler == null) {
            startServiceHandler = allocateBgHandler("StartServiceOps");
        }

        // Check boot
        if (intent != null) {
            boot = intent.getBooleanExtra("boot", false);
            user = intent.getBooleanExtra(INTENT_KEY_USER, false);
        }

        UaServiceManager.getInstance().startService(this, LauncherUtils.getCurrentUserPhoneId());

        // Use a network listener to start RCS core when the data will be ON
        if (RcsSettings.getInstance().getAutoConfigMode() == RcsSettingsData.NO_AUTO_CONFIG) {
            // Get connectivity manager
            if (connMgr == null) {
                connMgr = (ConnectivityManager) getSystemService(Context.CONNECTIVITY_SERVICE);
            }

            // Instantiate the network listener
            networkStateListener = new BroadcastReceiver() {
                @Override
                public void onReceive(Context context, final Intent intent) {
                    startServiceHandler.post(new Runnable() {
                        @Override
                        public void run() {
                            connectionEvent(intent.getAction());
                        }
                    });
                }
            };

            // Register network state listener
            IntentFilter intentFilter = new IntentFilter();
            intentFilter.addAction(ConnectivityManager.CONNECTIVITY_ACTION);
            registerReceiver(networkStateListener, intentFilter);
            mIsRegisteredAtomicBoolean.set(true);
        }

        /**
         * M: Check account in a background thread, fix an ANR issue. @{
         */
        startServiceHandler.post(new Runnable() {
            @Override
            public void run() {
                boolean accountAvailable = checkAccount();
                if (logger.isActivated()) {
                    logger.debug("accountAvailable = " + accountAvailable);
                }
                if (accountAvailable) {
                    //add for auto config service (ACS)
                    if (RcsUaService.isAcsAvailable(StartService.this)) {
                        mAcsEventCallbackImpl = new AcsEventCallbackImpl(StartService.this);
                        mAcsEventCallbackImpl.requestAcsService();
                    }
                    launchRcsService(boot, user);
                } else {
                    // User account can't be initialized (no radio to read IMSI, .etc)
                    if (logger.isActivated()) {
                        logger.error("Can't create the user account");
                    }

                    // Exit service
                    StartService.this.stopSelf();
                }
            }
        });
        /**
         * @}
         */

        // We want this service to continue running until it is explicitly
        // stopped, so return sticky.
        return START_STICKY;
    }

    /**
     * Connection event
     *
     * @param action Connectivity action
     */
    private void connectionEvent(String action) {

        if (logger.isActivated()) {
            logger.debug("Connection event " + action);
        }

        // Try to start the service only if a data connectivity is available
        if (action.equals(ConnectivityManager.CONNECTIVITY_ACTION)) {
            NetworkInfo networkInfo = connMgr.getActiveNetworkInfo();

            if ((networkInfo != null) && networkInfo.isConnected()) {
                 logger.debug("Device connected - Launch RCS service");

                boolean accountAvailable = checkAccount();
                logger.debug("accountAvailable = " + accountAvailable);
                if (accountAvailable) {
                    launchRcseCoreServie();

                    // Stop Network listener
                    if (networkStateListener != null
                            && mIsRegisteredAtomicBoolean.compareAndSet(true, false)) {
                        try {
                            unregisterReceiver(networkStateListener);
                                networkStateListener = null;
                        } catch (IllegalArgumentException e) {
                        }
                    }
                 } else {
                    // Exit service
                    stopSelf();
                }
            }
        }
    }

    /**
     * Set the country code
     *
     * @return Boolean
     */
    private boolean setCountryCode() {
        // Get country code
        int subId=0;
        subId = SubscriptionManager.getDefaultDataSubscriptionId();
        if (logger.isActivated()) {
            logger.error("setCountryCode subId" + subId);
        }
        if(subId == -1){
             subId = SubscriptionManager.getDefaultSubscriptionId();
        }
        if (logger.isActivated()) {
            logger.error("setCountryCode subId" + subId);
        }
        TelephonyManager mgr = (TelephonyManager)getSystemService(Context.TELEPHONY_SERVICE);
        String countryCodeIso = mgr.getSimCountryIso(subId);
        if (logger.isActivated()) {
                logger.error("countryCodeIso" + countryCodeIso);
            }

        if (countryCodeIso == null) {
            if (logger.isActivated()) {
                logger.error("Can't read country code from SIM");
            }
            return false;
        }


        // Parse country table to resolve the area code and country code
        try {
            XmlResourceParser parser = getResources().getXml(R.xml.country_table);
            parser.next();
            int eventType = parser.getEventType();
            while (eventType != XmlPullParser.END_DOCUMENT) {
                if (eventType == XmlPullParser.START_TAG) {
                    if (parser.getName().equals("Data")) {
                        if (parser.getAttributeValue(null, "code").equalsIgnoreCase(countryCodeIso)) {
                            String countryCode = parser.getAttributeValue(null, "cc");
                        //    String areaCode = parser.getAttributeValue(null, "tc");
                            if (countryCode != null) {
                                if (!countryCode.startsWith("+")) {
                                    countryCode = "+" + countryCode;
                                }
                                if (logger.isActivated()) {
                                    logger.info("Set country code to " + countryCode);
                                }
                               /**
                                 * M: Add for storing 3 SIM card user data info @{
                                 */
                                // Used to avoid ANR while do I/O operation
                                final String finalCountryCode = countryCode;
                                AsyncTask.execute(new Runnable(){
                                    public void run() {
                                        RcsSettings.getInstance().setCountryCode(finalCountryCode);
                                    }
                                });
                                /**
                                 * @}
                                 */
                            }

                            final String areaCode = parser.getAttributeValue(null, "tc");
                            if (areaCode != null) {
                                if (logger.isActivated()) {
                                    logger.info("Set area code to " + areaCode);
                                }
                                /**
                                 * M: Add for storing 3 SIM card user data info @{
                                 */
                                // Used to avoid ANR while do I/O operation
                                AsyncTask.execute(new Runnable(){
                                    public void run() {
                                RcsSettings.getInstance().setCountryAreaCode(areaCode);
                            }
                                });
                                /**
                                 * @}
                                 */
                            }
                                return true;
                            }
                        }
                    }
                eventType = parser.next();
            }

            if (logger.isActivated()) {
                logger.error("Country code not found");
            }
            return false;
        } catch (XmlPullParserException e) {
            if (logger.isActivated()) {
                logger.error("Can't parse country code from XML file", e);
            }
            return false;
        } catch (IOException e) {
            if (logger.isActivated()) {
                logger.error("Can't read country code from XML file", e);
            }
            return false;
        }
    }

    /**
     * Check account
     *
     * @return true if an account is available
     */
    private boolean checkAccount() {

        if (LauncherUtils.isSecondaryDevice()) {
            return true;
        }

        // Read the current and last end user account
        currentUserAccount = LauncherUtils.getCurrentUserAccount(getApplicationContext());
        lastUserAccount = LauncherUtils.getLastUserAccount(getApplicationContext());
        if (logger.isActivated()) {
            logger.info("Last user account is " + lastUserAccount);
            logger.info("Current user account is " + currentUserAccount);
        }

        // Check the current SIM
        if (currentUserAccount == null) {
            if (isFirstLaunch()) {
                // If it's a first launch the IMSI is necessary to initialize the service the first time
                return false;
            } else {
                // Set the user account ID from the last used IMSI
                currentUserAccount = lastUserAccount;
            }
        }

        // On the first launch and if SIM card has changed
        if (isFirstLaunch()) {
            // Set the country code
            boolean result = setCountryCode();
            if (!result) {
                // Can't set the country code
                return false;
            }

            // Set new user flag
            setNewUserAccount(true);
        } else if (hasChangedAccount()) {
            // Backup last account settings
            if (lastUserAccount != null) {
                if (logger.isActivated()) {
                    logger.info("Backup " + lastUserAccount);
                }
                RcsSettings.getInstance().backupAccountSettings(lastUserAccount);
                if (!RcsSettings.getInstance().supportOP01())
                LauncherUtils.backupMessage(getApplicationContext());
            }
            RcsSettings.getInstance().setFristLaunchState(true);
            // Set the country code
            boolean result = setCountryCode();
            if (!result) {
                // Can't set the country code
                return false;
            }

            // Reset RCS account
            LauncherUtils.resetRcsConfig(getApplicationContext());

            // Restore current account settings
            if (logger.isActivated()) {
                logger.info("Restore " + currentUserAccount);
            }
            RcsSettings.getInstance().restoreAccountSettings(currentUserAccount);
            RcsSettings.createInstance(AndroidFactory.getApplicationContext());

           //LauncherUtils.restoreMessages(getApplicationContext());
           // Intent intent = new Intent(ACTION_DB_CHANGE);
           // AndroidFactory.getApplicationContext().sendBroadcast(intent);

            // Activate service if new account
            RcsSettings.getInstance().setServiceActivationState(true);

            // Set new user flag
            setNewUserAccount(true);
        } else {
            // Set new user flag
            setNewUserAccount(false);
        }

        // Save the current end user account
        LauncherUtils.setLastUserAccount(getApplicationContext(), currentUserAccount);

        return true;
    }

    /**
     * Launch the RCS service.
     *
     * @param boot indicates if RCS is launched from the device boot
     * @param user indicates if RCS is launched from the user interface
     */
    private void launchRcsService(boolean boot, boolean user) {
        int mode = RcsSettings.getInstance().getAutoConfigMode();

        if (logger.isActivated())
            logger.debug("Launch RCS service: HTTPS=" + (mode == RcsSettingsData.HTTPS_AUTO_CONFIG) + ", boot=" + boot + ", user=" + user);

        if(boot){
            //set the boot flag initialization needed for some IMS service
            setIMSServicePropertyInSharedPrefs(boot);
        }

        if (mode == RcsSettingsData.HTTPS_AUTO_CONFIG) {
            // HTTPS auto config
            String version = RcsSettings.getInstance().getProvisioningVersion();

            if (logger.isActivated())
                logger.debug("launchRcsService : getProvisioningVersion := " + version);


            // Check the last provisioning version
            if (ProvisioningInfo.Version.RESETED_NOQUERY.equals(version)) {
                // (-1) : RCS service is permanently disabled. SIM change is required
                if (hasChangedAccount()) {
                    // Start provisioning as a first launch
                    HttpsProvisioningService.startHttpsProvisioningService(getApplicationContext(), true, user);
                } else {
                    if (logger.isActivated()) {
                        logger.debug("Provisioning is blocked with this account");
                    }

                    //register a listner for Provisioning SMS
                    if (reconfSMSReceiver == null) {
                        reconfSMSReceiver = new HttpsProvisioningSMS(this);
                        reconfSMSReceiver.registerSmsProvisioningReceiver(Integer.toString(HttpsProvisioningUtils.DEFAULT_SMS_PORT), null, null, null);
                    }

                }
            } else {
                if (isFirstLaunch() || hasChangedAccount()) {
                    // First launch: start the auto config service with special tag
                    HttpsProvisioningService.startHttpsProvisioningService(getApplicationContext(), true, user);
                } else {
                    if (ProvisioningInfo.Version.DISABLED_NOQUERY.equals(version)) {
                        // -2 : RCS client and configuration query is disabled
                        if (user) {
                            // Only start query if requested by user action
                            HttpsProvisioningService.startHttpsProvisioningService(getApplicationContext(), false, user);
                        }else{
                            //register a listner for Provisioning SMS
                            if (reconfSMSReceiver == null) {
                                reconfSMSReceiver = new HttpsProvisioningSMS(this);
                                reconfSMSReceiver.registerSmsProvisioningReceiver(Integer.toString(HttpsProvisioningUtils.DEFAULT_SMS_PORT), null, null, null);
                            }
                        }
                    } else {
                        // Start or restart the HTTP provisioning service
                        HttpsProvisioningService.startHttpsProvisioningService(getApplicationContext(), false, user);
                        if (ProvisioningInfo.Version.DISABLED_DORMANT.equals(version)) {
                            // -3 : RCS client is disabled but configuration query is not
                        } else {
                            // Start the RCS core service
                            /**M:
                             * core services will be started by the after provisining is validated
                             */
                            //LauncherUtils.launchRcsCoreService(getApplicationContext());
                        }
                    }
                }
            }
        } else {

            if(RcsSettings.getInstance().isSupportOP08()|| RcsSettings.getInstance().isSupportOP07()){
              updateSIMDetailsinDB();
            }

            RcsSettings.getInstance().setConfigurationState(true);
            // No auto config: directly start the RCS core service
            LauncherUtils.launchRcsCoreService(getApplicationContext());
        }
    }

    public boolean updateSIMDetailsinDB() {
        boolean status = false;

        try {
            if (RcsSettings.getInstance().isSupportOP08() ||
                    RcsSettings.getInstance().isSupportOP07()) {

                // Set the connectivity manager
                boolean isWifinetwork = false;
                try {
                    ConnectivityManager connectivityMgr = (ConnectivityManager)AndroidFactory.
                            getApplicationContext().getSystemService(Context.CONNECTIVITY_SERVICE);

                    NetworkInfo networkInfo = connectivityMgr.getActiveNetworkInfo();
                    if ((networkInfo != null) && networkInfo.getType() == ConnectivityManager.TYPE_WIFI) {
                        isWifinetwork = true;
                    }
                } catch (Exception e1) {
                    e1.printStackTrace();
                }

                logger.debug("updateSIMDetailsinDB isWifinetwork: " + isWifinetwork);

                boolean akaFlag = true;
                if (logger.isActivated())
                    logger.debug("updateSIMDetailsinDB");
                if (RcsSettings.getInstance().getImsAuhtenticationProcedureForMobile() == RcsSettingsData.AKA_AUTHENT
                        || akaFlag) {

                        logger.debug("RcsSettingsData.AKA_AUTHENT : update RCS_setting fileds  ");

                        TelephonyManager tm = (TelephonyManager) getApplicationContext().getSystemService(
                                Context.TELEPHONY_SERVICE);

                        String impi = tm.getIsimImpi();
                        String[] impu = tm.getIsimImpu();
                        String domain = tm.getIsimDomain();
                        String MSISDN="";

                        try {
                            MSISDN = extractUserNamePart(impu[0]);
                        } catch (Exception e) {
                            MSISDN="";
                            if (logger.isActivated()) {
                                logger.error("updateSIMDetailsinDB is MSISDN null");
                            }
                            e.printStackTrace();
                        }

                    if(MSISDN == null) {
                        MSISDN="";
                    }
                    RcsSettings.getInstance().setUserProfileImsUserName_full(impu[0]);
                    RcsSettings.getInstance().setUserProfileImsPrivateId(impi);
                    RcsSettings.getInstance().setUserProfileImsDomain(domain);
                    RcsSettings.getInstance().setUserProfileImsDisplayName(MSISDN);
                    RcsSettings.getInstance().setUserProfileImsUserName(MSISDN);
                    if (RcsSettings.getInstance().isSupportOP08()) {
                        RcsSettings.getInstance().setImConferenceUri("sip:adhoc@msg.pc.t-mobile.com");
                        if (isWifinetwork) {
                            RcsSettings.getInstance().setSipTransactionTimeout("20");
                        } else {
                            RcsSettings.getInstance().setSipTransactionTimeout("10");
                        }
                    } else if (RcsSettings.getInstance().isSupportOP07()){
                        RcsSettings.getInstance().setImConferenceUri("n-way_messaging@one.att.net");
                        RcsSettings.getInstance().setSipTransactionTimeout("20");
                        RcsSettings.getInstance().setChatIdleDuration("400");
                    }

                    //XDM CHANGES
                    RcsSettings.getInstance().setXdmServer("xcap.msg.pc.t-mobile.com");
                    RcsSettings.getInstance().setXdmLogin(impu[0]);
                    RcsSettings.getInstance().writeParameter(RcsSettingsData.XDM_AUTH_TYPE,"GBA");

                    //SERVICE CHANGES
                    RcsSettings.getInstance().writeParameter(RcsSettingsData.CAPABILITY_SOCIAL_PRESENCE, Boolean.toString(true));
                    RcsSettings.getInstance().writeParameter(RcsSettingsData.CAPABILITY_PRESENCE_DISCOVERY, Boolean.toString(true));
                    RcsSettings.getInstance().writeParameter(RcsSettingsData.AUTO_ACCEPT_CHAT,RcsSettingsData.TRUE);
                    RcsSettings.getInstance().writeParameter(RcsSettingsData.AUTO_ACCEPT_GROUP_CHAT,RcsSettingsData.TRUE);
                    RcsSettings.getInstance().writeParameter(RcsSettingsData.PERMANENT_STATE_MODE,RcsSettingsData.FALSE);
                    RcsSettings.getInstance().setCPMSupported(true);
                }
            }
            status = true;
        } catch(Exception err) {
            logger.debug("Exception while updating RCSsettng from SIm details for AKA");
        }
        return status;
    }

    private  String extractUserNamePart(String uri) {
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
     * Is the first RCs is launched ?
     *
     * @return true if it's the first time RCS is launched
     */
    private boolean isFirstLaunch() {

        if(LauncherUtils.isSecondaryDevice()){
            //if MSISDN is not entered
           if(RcsSettings.getInstance().getMsisdn()!=null){
               RcsSettings.getInstance().setFristLaunchState(false);
               return false;
           }
           else{
               RcsSettings.getInstance().setFristLaunchState(true);
               return true;
           }
        }
        if(lastUserAccount == null)
            RcsSettings.getInstance().setFristLaunchState(true);
        return (lastUserAccount == null);
    }

    /**
     * Check if RCS account has changed since the last time we started the service
     *
     * @return true if the active account was changed
     */
    private boolean hasChangedAccount() {

        if(LauncherUtils.isSecondaryDevice()){
          return false;
        }


        if (lastUserAccount == null) {
            return true;
        } else
        if (currentUserAccount == null) {
            return false;
        } else {
            return (!currentUserAccount.equalsIgnoreCase(lastUserAccount));
        }
    }

    /**
     * Set true if new user account
     *
     * @param value true if new user account
     */
    private void setNewUserAccount(boolean value) {
        SharedPreferences preferences = getSharedPreferences(AndroidRegistryFactory.RCS_PREFS_NAME, Activity.MODE_PRIVATE);
        SharedPreferences.Editor editor = preferences.edit();
        editor.putBoolean(REGISTRY_NEW_USER_ACCOUNT, value);
        editor.commit();
    }

    /**
     * Check if new user account
     *
     * @param context Application context
     * @return true if new user account
     */
    public static boolean getNewUserAccount(Context context) {
        SharedPreferences preferences = context.getSharedPreferences(AndroidRegistryFactory.RCS_PREFS_NAME, Activity.MODE_PRIVATE);
        return preferences.getBoolean(REGISTRY_NEW_USER_ACCOUNT, false);
    }

    /**
     * Launch the RCS start service
     *
     * @param context
     * @param boot
     *            start RCS service upon boot
     * @param user
     *            start RCS service upon user action
     */
    static void LaunchRcsStartService(Context context, boolean boot, boolean user) {
        if (logger.isActivated())
            logger.debug("Launch RCS service (boot=" + boot + ") (user="+user+")");
        //AndroidFactory.setApplicationContext(context);
        Intent intent = new Intent(context, StartService.class);
        intent.putExtra(INTENT_KEY_BOOT, boot);
        intent.putExtra(INTENT_KEY_USER, user);
        context.startService(intent);
    }
    /**
     * M: Added to indicate whether the receiver is registered. @{
     */

    /**
     * NetworkChangedReceiver instance
     */
  //  private final NetworkChangedReceiver mNetworkChangedReceiver = new NetworkChangedReceiver();

    private void launchRcseCoreServie() {
                    LauncherUtils.launchRcsCoreService(getApplicationContext());
                }

   /* private synchronized void registerNetworkReceiver() {
        IntentFilter filter = new IntentFilter();
        filter.addAction(ConnectivityManager.CONNECTIVITY_ACTION);
        this.registerReceiver(mNetworkChangedReceiver, filter);
        }

    private synchronized void unregisterNetworkReceiver() {
        this.unregisterReceiver(mNetworkChangedReceiver);
    }*/
    /**
     * @}
     */

    /*
     * Set device boot flag. its for achaiveing one request per boot for XDM manager
     */
    public static void setXDMBootFlag(Context context,boolean value) {
        SharedPreferences preferences = context.getSharedPreferences(AndroidRegistryFactory.RCS_PREFS_IMS_SERVICES, Activity.MODE_PRIVATE);
        SharedPreferences.Editor editor = preferences.edit();
        editor.putBoolean(KEY_XDM_REQUEST_ENABLE, value);
        editor.commit();
    }

    public static boolean getXDMBootFlag(Context context) {
        SharedPreferences preferences = context.getSharedPreferences(AndroidRegistryFactory.RCS_PREFS_IMS_SERVICES, Activity.MODE_PRIVATE);
        return preferences.getBoolean(KEY_XDM_REQUEST_ENABLE, false);
    }

    public static void setFirstBootFlag(Context context,boolean value) {
        SharedPreferences preferences = context.getSharedPreferences(AndroidRegistryFactory.RCS_PREFS_IMS_SERVICES, Activity.MODE_PRIVATE);
        SharedPreferences.Editor editor = preferences.edit();
        editor.putBoolean(KEY_FIRST_BOOT, value);
        editor.commit();
    }

    public static boolean getFirstBootFlag(Context context) {
        SharedPreferences preferences = context.getSharedPreferences(AndroidRegistryFactory.RCS_PREFS_IMS_SERVICES, Activity.MODE_PRIVATE);
        return preferences.getBoolean(KEY_FIRST_BOOT, true);
    }

    public static void setXdmInitializeFlag(Context context,boolean value) {
        SharedPreferences preferences = context.getSharedPreferences(AndroidRegistryFactory.RCS_PREFS_IMS_SERVICES, Activity.MODE_PRIVATE);
        SharedPreferences.Editor editor = preferences.edit();
        editor.putBoolean(KEY_INITIALIZE_STATUS, value);
        editor.commit();
    }

    public static boolean getXdmInitializeFlag(Context context) {
        SharedPreferences preferences = context.getSharedPreferences(AndroidRegistryFactory.RCS_PREFS_IMS_SERVICES, Activity.MODE_PRIVATE);
        return preferences.getBoolean(KEY_INITIALIZE_STATUS, false);
    }

    private void setIMSServicePropertyInSharedPrefs(boolean isBoot){

        //for XDM set its RCS service initialization via boot
        setXDMBootFlag(getApplicationContext(),isBoot);
    }

}
