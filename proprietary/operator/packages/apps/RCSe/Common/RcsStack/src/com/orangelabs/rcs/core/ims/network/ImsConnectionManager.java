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

package com.orangelabs.rcs.core.ims.network;

import android.text.TextUtils;
import com.mediatek.ims.rcsua.*;
import com.orangelabs.rcs.core.CoreException;
import com.orangelabs.rcs.core.ims.ImsModule;
import com.orangelabs.rcs.core.ims.network.ImsNetworkInterface.DnsResolvedFields;
import com.orangelabs.rcs.core.ims.network.registration.RegistrationUtils;
import com.orangelabs.rcs.core.ims.network.sip.SipManager;
import com.orangelabs.rcs.core.ims.protocol.sip.SipInterface;
import com.orangelabs.rcs.platform.AndroidFactory;
import com.orangelabs.rcs.platform.network.NetworkFactory;
import com.orangelabs.rcs.provider.settings.RcsSettings;
import com.orangelabs.rcs.provider.settings.RcsSettingsData;
import com.orangelabs.rcs.service.UaServiceManager;
import com.orangelabs.rcs.service.LauncherUtils;
import com.orangelabs.rcs.utils.logger.Logger;

import android.content.BroadcastReceiver;
import android.content.ContentResolver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.database.Cursor;
import android.net.ConnectivityManager;
import android.net.NetworkInfo;
import android.net.Uri;
import android.os.Build;
import android.os.RemoteCallbackList;
import android.os.RemoteException;
import android.os.SystemProperties;
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;

import com.orangelabs.rcs.core.CoreException;
import com.orangelabs.rcs.core.ims.ImsModule;
import com.orangelabs.rcs.platform.AndroidFactory;
import com.orangelabs.rcs.platform.network.NetworkFactory;
import com.orangelabs.rcs.provider.settings.RcsSettings;
import com.orangelabs.rcs.provider.settings.RcsSettingsData;
import com.orangelabs.rcs.utils.logger.Logger;
import com.orangelabs.rcs.utils.NetworkSwitchInfo;
import java.util.Random;

/**
 * IMS connection manager
 *
 * @author JM. Auffret
 * @author Deutsche Telekom
 */
public class ImsConnectionManager implements Runnable {

    private ImsModule imsModule;
    private static boolean hackyflag = false;
    private static int countNetworkInterface = 3;
    private ImsNetworkInterface[] networkInterfaces = new ImsNetworkInterface[countNetworkInterface];
    private static ImsNetworkInterface currentNetworkInterface;
    private Thread imsPollingThread = null;
    private NetworkSwitchInfo netSwitchInfo = new NetworkSwitchInfo();
    private boolean imsActivationFlag = false;
    private long imsPollingThreadID = -1;
    private ConnectivityManager connectivityMgr;
    private int network;
    private String operator;
    private String apn;
    private DnsResolvedFields mDnsResolvedFields = null;
    private boolean imsServicesStarted = false;
    private Logger logger = Logger.getLogger(this.getClass().getName());

    /**
     * M: Added to achieve the auto configuration related feature. @{
     */
    private static final String PREFER_APN_URI = "content://telephony/carriers/preferapn";
    private static final String GEMINI_PREFER_APN_URI = "content://telephony/carriers_gemini/preferapn";
    private static final String APN = "apn";
    /**
     * @}
     */

    /**
     * M: add for auto-rejoining group chat @{
     */
    /**
     * List of network connectivity listener.
     */
    private RemoteCallbackList<INetworkConnectivity> mListeners = new RemoteCallbackList<INetworkConnectivity>();
    /**
     * Lock used for synchronization
     */
    private Object mLock = new Object();
    /** @} */

    /**
     * Constructor
     *
     * @param imsModule IMS module
     * @throws CoreException
     */
    public ImsConnectionManager(ImsModule imsModule) throws CoreException {
        this.imsModule = imsModule;

        // Get network access parameters
        network = RcsSettings.getInstance().getNetworkAccess();

        // Get network operator parameters
        operator = RcsSettings.getInstance().getNetworkOperator();
        apn = RcsSettings.getInstance().getNetworkApn();

        // Set the connectivity manager
        connectivityMgr = (ConnectivityManager)AndroidFactory.getApplicationContext().getSystemService(Context.CONNECTIVITY_SERVICE);

        // Instanciates the IMS network interfaces
        networkInterfaces[0] = new MobileNetworkInterface(imsModule);
        networkInterfaces[1] = new WifiNetworkInterface(imsModule);
        //for Single IMS registration
        if (RcsSettings.getInstance().singleRegistrationSupported()) {
            VolteNetworkInterface volteInterface = new VolteNetworkInterface(imsModule);
            networkInterfaces[2] = volteInterface;

            uaCallback = new UaImsEventCallback();
            uaCallback.setCallbackOnMain(true);
            UaServiceManager.getInstance().registerClient(uaCallback);
        }

        // Set the mobile network interface by default
        currentNetworkInterface = getMobileNetworkInterface();

        // Load the user profile
        loadUserProfile();

        // Register network state listener
        IntentFilter intentFilter = new IntentFilter();
        intentFilter.addAction(ConnectivityManager.CONNECTIVITY_ACTION);

        /* M: Modified to resolve the issue that RCS-e server can not be connected */
        if (null == networkStateListener) {
            synchronized (mListenerLock) {
                networkStateListener = new BroadcastReceiver() {
                        @Override
                        public void onReceive(Context context, final Intent intent) {
                            Thread t = new Thread() {
                                    public void run() {
                                    logger.debug("networkStateListener onReceive() intent = " + intent);
                                    //connectionEvent(intent);
                                }
                            };
                            t.start();
                        }
                    };
                AndroidFactory.getApplicationContext().registerReceiver(networkStateListener,
                            intentFilter);
            }
        } else {
            logger.error("Constructor() networkStateListener is not null");
        }
    }

    /**
     * Returns the current network interface
     *
     * @return Current network interface
     */
    public ImsNetworkInterface getCurrentNetworkInterface() {
        return currentNetworkInterface;
    }

    /**
     * Returns the mobile network interface
     *
     * @return Mobile network interface
     */
    public ImsNetworkInterface getMobileNetworkInterface() {
        return networkInterfaces[0];
    }

    /**
     * Returns the Volte network interface
     *
     * @return volte network interface
     */
    public ImsNetworkInterface getVolteNetworkInterface() {
        return networkInterfaces[2];
    }

    /**
     * Returns the Wi-Fi network interface
     *
     * @return Wi-Fi network interface
     */
    public ImsNetworkInterface getWifiNetworkInterface() {
        return networkInterfaces[1];
    }

    /** @} */

    /**
     * Is connected to Wi-Fi
     *
     * @return Boolean
     */
    public boolean isConnectedToWifi() {
        if (currentNetworkInterface == getWifiNetworkInterface()) {
            return true;
        } else {
            return false;
        }
    }

    /**
     * Is connected to mobile
     *
     * @return Boolean
     */
    public boolean isConnectedToMobile() {
        if (currentNetworkInterface == getMobileNetworkInterface()) {
            return true;
        } else {
            return false;
        }
    }

    /**
     * Load the user profile associated to the network interface
     */
    private void loadUserProfile() {
        ImsModule.IMS_USER_PROFILE = currentNetworkInterface.getUserProfile();
        if (logger.isActivated()) {
            logger.debug("User profile has been reloaded");
        }
    }

    /**
     * Terminate the connection manager
     */
    public void terminate() {
        if (logger.isActivated()) {
            logger.info("Terminate the IMS connection manager");
        }

        if (RcsSettings.getInstance().singleRegistrationSupported()) {

        } else {
            // Unregister network state listener
            try {
                /**
                 * M: Modified to resolve the issue that RCS-e server can not be
                 * connected. @{
                 */
                if (null != networkStateListener) {
                    synchronized (mListenerLock) {
                        if (null != networkStateListener) {
                            logger.debug("terminate() networkStateListener is not null");
                            // Unregister network state listener
                            AndroidFactory.getApplicationContext().unregisterReceiver(networkStateListener);
                            /**
                             * M: Modified to resolve the issue that RCSe service need
                             * to start twice when installed. @{
                             */
                            networkStateListener = null;
                            /**
                             * @}
                            */
                        }
                    }
                }
            /**
             * @}
             */
            } catch (IllegalArgumentException e) {
                // Nothing to do
            }
        }

        stopImsConnection();

        /* send notification to notication bar */
        imsModule.getCore().getListener().handleTryDeregister();

        /* Unregister from the IMS */
        currentNetworkInterface.unregister();

        Client client = UaServiceManager.getInstance().getActiveClient();
        if (client != null) {
            UaServiceManager.getInstance().unregisterClient(client);
        }

        logger.info("IMS connection manager has been terminated");
    }

    private volatile BroadcastReceiver networkStateListener = null;
    private Object mListenerLock = new Object();

    private synchronized void connectionEvent() {

        NetworkInfo networkInfo = connectivityMgr.getActiveNetworkInfo();

        if ((networkInfo == null) || (currentNetworkInterface == null)) {
            // Disconnect from IMS network interface
            logger.debug("Disconnect from IMS: no network (e.g. air plane mode)");
            logger.info("set_ims_off_by_network true");
            netSwitchInfo.set_ims_off_by_network();
            disconnectFromIms();
            return;
        }

        // Check if SIM account has changed (i.e. hot SIM swap)
        if (networkInfo.getType() == ConnectivityManager.TYPE_MOBILE) {
            String lastUserAccount = LauncherUtils.getLastUserAccount(AndroidFactory.getApplicationContext());
            if (logger.isActivated()) {
                logger.debug("LastUserAccount: "+lastUserAccount);
            }
            String currentUserAccount = LauncherUtils.getCurrentUserAccount(AndroidFactory.getApplicationContext());
            if (logger.isActivated()) {
                logger.debug("currentUserAccount: " +currentUserAccount);
            }
            if (lastUserAccount != null) {
                if ((currentUserAccount == null) || !currentUserAccount.equalsIgnoreCase(lastUserAccount)) {
                    imsModule.getCoreListener().handleSimHasChanged();
                    return;
                }
            }
        }

        // Get the current local IP address
        String localIpAddr = null;

        // Check if the network access type has changed
        if (networkInfo.getType() != currentNetworkInterface.getType()) {
            // Network interface changed
            if (logger.isActivated()) {
                logger.info("Data connection state: NETWORK ACCESS CHANGED");
            }

            // Disconnect from current IMS network interface
            if (logger.isActivated()) {
                logger.debug("Disconnect from IMS: network access has changed");
            }
            disconnectFromIms();

            // Change current network interface
            if (networkInfo.getType() == ConnectivityManager.TYPE_MOBILE) {
                if (logger.isActivated()) {
                    logger.debug("Change the network interface to mobile");
                }

                currentNetworkInterface = getMobileNetworkInterface();

            } else if (networkInfo.getType() == ConnectivityManager.TYPE_WIFI) {
                if (logger.isActivated()) {
                    logger.debug("Change the network interface to Wi-Fi");
                }
                currentNetworkInterface = getWifiNetworkInterface();
            }

            // Load the user profile for the new network interface
            loadUserProfile();

            // update DNS entry
            try {
                mDnsResolvedFields = currentNetworkInterface.getDnsResolvedFields();
            } catch (Exception e) {
                if (logger.isActivated()) {
                    logger.error(
                            "Resolving remote IP address to figure out initial local IP address failed!", e);
                }
            }

            // get latest local IP address
            localIpAddr = NetworkFactory.getFactory().getLocalIpAddress(mDnsResolvedFields, networkInfo.getType());

        } else {
            // Check if the IP address has changed
            try {
                if (mDnsResolvedFields == null) {
                    mDnsResolvedFields = currentNetworkInterface.getDnsResolvedFields();
                }
            } catch (Exception e) {
                if (logger.isActivated()) {
                    logger.error("Resolving remote IP address to figure out initial local IP address failed!", e);
                }
            }
            localIpAddr = NetworkFactory.getFactory().getLocalIpAddress(mDnsResolvedFields, networkInfo.getType());

            if (localIpAddr != null) {
                String lastIpAddr = currentNetworkInterface.getNetworkAccess().getIpAddress();
                if (!localIpAddr.equals(lastIpAddr)) {
                    // Changed by Deutsche Telekom
                    if (lastIpAddr != null) {
                // Disconnect from current IMS network interface
                if (logger.isActivated()) {
                    logger.debug("Disconnect from IMS: IP address has changed");
                }
                        disconnectFromIms();
                    } else {
                    if (logger.isActivated()) {
                            logger.debug("IP address available (again)");
                    }
                    }
                } else {
                    // Changed by Deutsche Telekom
                    if (logger.isActivated()) {
                        logger.debug("Neither interface nor IP address has changed; nothing to do.");
                    }
                    return;
                    }
            }
        }

        // Check if there is an IP connectivity
        if (networkInfo.isConnected() && (localIpAddr != null)) {
            String remoteAddress;
            if (mDnsResolvedFields != null) {
                remoteAddress = mDnsResolvedFields.ipAddress;
            } else {
                remoteAddress = new String("unresolved");
            }

            if (logger.isActivated()) {
                logger.info("Data connection state: CONNECTED to " + networkInfo.getTypeName() + " with local IP " + localIpAddr + " valid for " + remoteAddress);
            }

            // Test network access type
            if ((network != RcsSettingsData.ANY_ACCESS) && (network != networkInfo.getType())) {
                if (logger.isActivated()) {
                    logger.warn("Network access " + networkInfo.getTypeName() + " is not authorized");
                }
                return;
            }

            // Test the operator id
            int subId=0;
            subId=(int)SubscriptionManager.getDefaultDataSubscriptionId();
            TelephonyManager tm = (TelephonyManager)AndroidFactory.getApplicationContext().getSystemService(Context.TELEPHONY_SERVICE);
            String currentOpe = tm.getSimOperatorName(subId);
            if ((operator.length() > 0) && !currentOpe.equalsIgnoreCase(operator)) {
                if(RcsSettings.getInstance().getAutoConfigMode()==1) {
                    if (logger.isActivated()) {
                        logger.warn("Operator not authorized");
                    }
                    return;
                }
            }

            // From Android 4.2, the management of APN is only for system app
            /*if (Build.VERSION.SDK_INT < 17) {
                // Test the default APN configuration if mobile network
                *//**
                 * M: Added to achieve the auto configuration related feature. @{
                 *//*
                if (networkInfo.getType() == ConnectivityManager.TYPE_MOBILE) {
                    // Test the default APN configuration
                    ContentResolver cr = AndroidFactory.getApplicationContext()
                            .getContentResolver();
                    String currentApn = null;
                    Cursor cursor = null;
                    try {
                        if (logger.isActivated()) {
                            logger.debug("connectionEvent GEMINI_PHONE: " + GEMINI_PHONE);
                        }
                        if (GEMINI_PHONE) {
                            cursor = cr.query(Uri.parse(GEMINI_PREFER_APN_URI), new String[] {
                                APN
                            }, null, null, null);
                        } else {
                            cursor = cr.query(Uri.parse(PREFER_APN_URI), new String[] {
                                APN
                            }, null, null, null);
                        }
                        if (cursor != null) {
                            final int apnIndex = cursor.getColumnIndexOrThrow("apn");
                            if (cursor.moveToFirst()) {
                                currentApn = cursor.getString(apnIndex);
                        }
                        }
                    } finally {
                        if (cursor != null) {
                            cursor.close();
                        }
                    }
                    if (logger.isActivated()) {
                        logger.warn("connectionEvent apn = " + apn + ", currentApn = " + currentApn);
                    }
                    if ((apn.length() > 0) && !apn.equalsIgnoreCase(currentApn)) {
                        if (logger.isActivated()) {
                            logger.warn("APN not authorized");
                        }
//                        return;
                    }
                }
              *//**
                 * @}
                 *//*
            }*/
            // Test the configuration
            if (!currentNetworkInterface.isInterfaceConfigured()) {
                if (logger.isActivated()) {
                    logger.warn("IMS network interface not well configured");
                }
                return;
            }
            // Connect to IMS network interface
            if (logger.isActivated()) {
                logger.debug("Connect to IMS");
            }
            connectToIms(localIpAddr);
        } else {
            if (logger.isActivated()) {
                logger.info("Data connection state: DISCONNECTED from " + networkInfo.getTypeName());
            }

            // Disconnect from IMS network interface
            if (logger.isActivated()) {
                logger.debug("Disconnect from IMS: IP connection lost");
            }
            if (logger.isActivated()) {
                logger.info("set_ims_off_by_network true");
            }
            netSwitchInfo.set_ims_off_by_network();
            disconnectFromIms();

        }
    }

    /**
     * Connect to IMS network interface
     *
     * @param ipAddr IP address
     */
    private void connectToIms(String ipAddr) {
        // Connected to the network access
        currentNetworkInterface.getNetworkAccess().connect(ipAddr);

        /**
         * M:update Access Network Info to database. @{T-Mobile
         */
        // currentNetworkInterface.setAccessNetworkInfo();
        /**
         * @}
         */
        // Start the IMS connection
        startImsConnection();
    }

    /**
     * Disconnect from IMS network interface
     */
    private void disconnectFromIms() {

        if (logger.isActivated()) {
            logger.debug("ready to notify prepareToDisconnect");
        }

        /**M : synchronized the code beginBroadcast
         * as only one thread is required to beging it at one time.
         */
        synchronized(mLock) {
            final int N = mListeners.beginBroadcast();
            for (int i = 0; i < N; i++) {
                try {
                    mListeners.getBroadcastItem(i).prepareToDisconnect();
                } catch (RemoteException e) {
                    if (logger.isActivated()) {
                        logger.error("Can't notify listener", e);
                    }
                }
            }
            mListeners.finishBroadcast();
        }

        // Stop the IMS connection
        stopImsConnection();

        // Registration terminated
        currentNetworkInterface.registrationTerminated();

        // Disconnect from the network access
        currentNetworkInterface.getNetworkAccess().disconnect();
    }

    /**
     * M: add for auto-rejoining group chat @{
     */
    /**
     * Add network connectivity listener
     *
     * @param listener Listener
     */
    public void addNetworkConnectivityListener(INetworkConnectivity listener) {
        if (logger.isActivated()) {
            logger.info("Add network connectivity listener");
        }

        synchronized (mLock) {
            mListeners.register(listener);
        }
    }

    /**
     * Remove network connectivity listener
     *
     * @param listener Listener
     */
    public void removeNetworkConnectivityListener(INetworkConnectivity listener) {
        if (logger.isActivated()) {
            logger.info("Remove network connectivity listener");
        }
        synchronized (mLock) {
            mListeners.unregister(listener);
        }
    }

    /** @} */

    /**
     * Start the IMS connection
     */
    private synchronized void startImsConnection() {
        if (imsActivationFlag) {
            // Already connected
            return;
        }

        if (imsPollingThreadID >= 0) {
            // Already connected
            return;
        }
        // Set the connection flag
        if (logger.isActivated()) {
            logger.info("Start the IMS connection manager");
        }
        imsActivationFlag = true;

        // Start background polling thread
        try {
            imsPollingThread = new Thread(this);
            imsPollingThreadID = imsPollingThread.getId();
            imsPollingThread.start();
        } catch(Exception e) {
            if (logger.isActivated()) {
                logger.error("Internal exception while starting IMS polling thread", e);
            }
        }
    }

    /**
     * Stop the IMS connection
     */
    private synchronized void stopImsConnection() {
        if (!imsActivationFlag) {
            // Already disconnected
            return;
        }

        if (imsPollingThreadID == -1) {
            // Already disconnected
            return;
        }
        // Set the connection flag
        if (logger.isActivated()) {
            logger.info("Stop the IMS connection manager");
        }
        imsPollingThreadID = -1;
        imsActivationFlag = false;

        // Stop background polling thread
        try {
            imsPollingThread.interrupt();
            imsPollingThread = null;
        } catch(Exception e) {
            if (logger.isActivated()) {
                logger.error("Internal exception while stopping IMS polling thread", e);
            }
        }

        // Stop IMS services
        if (imsServicesStarted) {
            imsModule.stopImsServices();
            imsServicesStarted = false;
        }
    }

    /**
     * Background processing
     */
    public void run() {
        if (logger.isActivated()) {
            logger.debug("Start polling of the IMS connection");
        }

        int servicePollingPeriod = RcsSettings.getInstance().getImsServicePollingPeriod();
        int regBaseTime = RcsSettings.getInstance().getRegisterRetryBaseTime();
        int regMaxTime = RcsSettings.getInstance().getRegisterRetryMaxTime();
        Random random = new Random();
        int nbFailures = 0;

        /*
         * For P-CSCF multiple address retry
         *
         */
        int nbAddress=RcsSettings.getInstance().getNumberOfPCssfAddresses();
        /*
         * currentCscfAddress is the index of P CSCF address
         * nbFailure is count of failed trials.
         * on every failed trial we chage the PCSCF address Cyclically
         */
        int currentCscfAddress=nbFailures%nbAddress;

        while (imsActivationFlag && (imsPollingThreadID == Thread.currentThread().getId())) {

            // Connection management
            try {
                // Test IMS registration
                if (!currentNetworkInterface.isRegistered()) {

                    logger.debug("Not yet registered to IMS: try registration");

                    /**M
                     * added to how notification of connecting and disconnecting states during registration
                     */
                    //send notification
                    imsModule.getCore().getListener().handleTryRegister();
                    /**@*/

                    // Try to register to IMS
                    if (currentNetworkInterface.register(currentCscfAddress)) {
                        // InterruptedException thrown by stopImsConnection() may be caught by one
                        // of the methods used in currentNetworkInterface.register() above
                        if (imsPollingThreadID != Thread.currentThread().getId()) {
                            logger.debug("IMS connection polling thread race condition");
                            break;
                        } else {
                            if (logger.isActivated()) {
                                logger.debug("Registered to the IMS with success: start IMS services");
                            }
                            if (imsModule.isReady() && !imsServicesStarted) {
                                imsModule.startImsServices();
                                imsServicesStarted = true;
                                /**
                                 * M: add for auto-rejoining group chat @{
                                 */
                                if (logger.isActivated()) {
                                    logger.debug("ready to notify connect");
                                }
                                final int N = mListeners.beginBroadcast();
                                for (int i = 0; i < N; i++) {
                                    try {
                                        mListeners.getBroadcastItem(i).connect();
                                    } catch (RemoteException e) {
                                        if (logger.isActivated()) {
                                            logger.error("Can't notify listener", e);
                                        }
                                    }
                                }
                                mListeners.finishBroadcast();

                                /** @} */
                            }
                            RcsSettings.getInstance().setCurrentAddressCounter(currentCscfAddress);
                            // Reset number of failures
                            nbFailures = 0;
                            currentCscfAddress=nbFailures%nbAddress;

                        }
                    } else {
                        if (logger.isActivated()) {
                            logger.debug("Can't register to the IMS");
                        }

                        // Increment number of failures
                        nbFailures++;
                        currentCscfAddress=nbFailures%nbAddress;
                    }
                } else {
                    if (imsModule.isReady()) {
                        if (!imsServicesStarted) {
                            if (logger.isActivated()) {
                                logger.debug("Already registered to IMS: start IMS services");
                            }
                            imsModule.startImsServices();
                            imsServicesStarted = true;
                        } else {
                            if (logger.isActivated()) {
                                logger.debug("Already registered to IMS: check IMS services");
                            }
                            imsModule.checkImsServices();
                        }
                    } else {
                        if (logger.isActivated()) {
                            logger.debug("Already registered to IMS: IMS services not yet started");
                        }
                    }
                }
            } catch (Exception e) {
                if (logger.isActivated()) {
                    logger.error("Internal exception", e);
                }
            }

            // InterruptedException thrown by stopImsConnection() may be caught by one
            // of the methods used in currentNetworkInterface.register() above
            if (imsPollingThreadID != Thread.currentThread().getId()) {
                logger.debug("IMS connection polling thread race condition");
                break;
            }

            // Make a pause before the next polling
            try {
                if (!currentNetworkInterface.isRegistered()) {
                    // Pause before the next register attempt
                    double w = Math.min(regMaxTime, (regBaseTime * Math.pow(2, nbFailures)));
                    double coeff = (random.nextInt(51) + 50) / 100.0; // Coeff between 50% and 100%
                    int retryPeriod = (int)(coeff * w);
                    if (logger.isActivated()) {
                        logger.debug("Wait " + retryPeriod + "s before retry registration (failures=" + nbFailures + ", coeff="+ coeff + ")");
                    }
                    /**
                    * M:Add for closing open port when idle
                    */
                    //close the sipstack if until the next regsiter request
                    currentNetworkInterface.closeSipStack();
                    /**
                    * M @}
                    */

                    Thread.sleep(retryPeriod * 1000);
                } else if (!imsServicesStarted) {
                    int retryPeriod = 5;
                    if (logger.isActivated()) {
                        logger.debug("Wait " + retryPeriod + "s before retry to start services");
                    }
                    Thread.sleep(retryPeriod * 1000);
                } else {
                    // Pause before the next service check
                    Thread.sleep(servicePollingPeriod * 1000);
                }
            } catch(InterruptedException e) {
                break;
            }
        }

        if (logger.isActivated()) {
            logger.debug("IMS connection polling is terminated");
        }
    }

    /* M : added for volte single regsitration */
    public class UaImsEventCallback extends ImsEventCallback {
        @Override
        public void onRegistering(int mode) {
            logger.debug("onRegistering");
        }

        @Override
        public void onRegistered(int mode) {
            logger.debug("onRegistered");
            if (!UaServiceManager.getInstance().serviceConnected())
                return;

            //int intRcsSrvTag = SystemProperties.getInt("persist.vendor.service.tag.rcs", 0);
            // error handle
            //if (intRcsSrvTag == 0x00) {
            //    logger.debug("onRegistered, return directly because of no rcs feature tag");
            //    return;
            //}

            Client client = UaServiceManager.getInstance().getActiveClient();
            RegistrationInfo info = client.getRegistrationInfo();
            Configuration config = info.readImsConfiguration();

            if (!info.isRegistered()) {
                //fix timing issue
                return;
            }
            disconnectFromIms();

            logger.debug("Change the network interface to volte network");

            currentNetworkInterface = getVolteNetworkInterface();
            ((VolteNetworkInterface)currentNetworkInterface)
                    .setImsOverInternet(mode == RcsUaService.REG_MODE_INTERNET);

            // update volte details
            currentNetworkInterface.updateVolteDetails(config);
            RcsSettings.getInstance().setUserProfileImsDomain(config.getHomeDomain());

            loadUserProfile();

            // update user profile as per IMS details provided by volte
            ImsModule.IMS_USER_PROFILE.setUserAgent(config.getUserAgent());

            //connect to IMS
            connectToIms(config.getLocalAddress());
        }

        @Override
        public void onDeregistering(int mode) {
            logger.debug("onDeregistering");
        }

        @Override
        public void onDeregistered(int mode) {
            logger.debug("onDeregistered");
            disconnectFromIms();
        }
    }

    private UaImsEventCallback uaCallback;
}
