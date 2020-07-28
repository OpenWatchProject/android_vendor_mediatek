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

package com.mediatek.presence.core;

import com.mediatek.presence.addressbook.AddressBookManager;
import com.mediatek.presence.core.ims.ImsModule;
import com.mediatek.presence.core.ims.service.capability.CapabilityService;
import com.mediatek.presence.core.ims.service.presence.PresenceService;
import com.mediatek.presence.core.ims.service.sip.SipService;
import com.mediatek.presence.core.ims.service.terms.TermsConditionsService;
import com.mediatek.presence.platform.AndroidFactory;
import com.mediatek.presence.utils.DeviceUtils;
import com.mediatek.presence.utils.PhoneUtils;
import com.mediatek.presence.utils.logger.Logger;
import com.mediatek.presence.service.DataSimChanged;

import android.content.Context;
import android.telephony.TelephonyManager;
import android.content.SharedPreferences;
import android.preference.PreferenceManager;

/**
 * Core (singleton pattern)
 *
 * @author JM. Auffret
 */
public class Core {
    /**
     * Singleton instance
     */
    private static Core instance = null;

    /**
     * Core listener
     */
    private CoreListener listener;

    /**
     * Core status
     */
    private boolean started = false;

    /**
     * IMS module
     */
    private ImsModule[] mImsModule;

    /**
     * Address book manager
     */
    private AddressBookManager addressBookManager;

    /**
     * The logger
     */
    private Logger logger = Logger.getLogger(this.getClass().getName());

    private Context mContext = null;

    private int mSimCount = 0;

    /**
     * Returns the singleton instance
     *
     * @return Core instance
     */
    public static Core getInstance() {
        return instance;
    }

    /**
     * Instanciate the core
     *
     * @param listener Listener
     * @return Core instance
     * @throws CoreException
     */
    public synchronized static Core createCore(CoreListener listener) throws CoreException {
        if (instance == null) {
            instance = new Core(listener);
        }
        return instance;
    }

    /**
     * Terminate the core
     */
    public synchronized static void terminateCore() {
        if (instance != null) {
            instance.stopCore();
        }
           instance = null;
    }

    /**
     * Constructor
     *
     * @param listener Listener
     * @throws CoreException
     */
    private Core(CoreListener listener) throws CoreException {
        if (logger.isActivated()) {
            logger.info("Terminal core initialization");
        }

        mContext = AndroidFactory.getApplicationContext();
        TelephonyManager tm = (TelephonyManager) mContext.getSystemService(
                Context.TELEPHONY_SERVICE);
        if (tm == null) {
            logger.error("Core init fail. TelephonyManager is null");
            return;
        }
        mSimCount = tm.getSimCount();

        // Set core event listener
        this.listener = listener;

        // Get UUID
        for (int slotId = 0; slotId < mSimCount; ++slotId) {
            if (logger.isActivated()) {
                logger.info("My device UUID of slot " + slotId + "is "
                        + DeviceUtils.getDeviceUUID(slotId, mContext));
            }
        }

        // Initialize the phone utils
        PhoneUtils.initialize(mContext);

        // Get country code
        if (logger.isActivated()) {
            logger.info("My country code of default data SIM is " + PhoneUtils.getCountryCode());
        }

        // Create the address book manager
        addressBookManager = new AddressBookManager();

        SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(mContext);

        // Create the IMS module
        mImsModule = new ImsModule[mSimCount];
        for (int slotId = 0; slotId < mSimCount; ++slotId) {
            prefs.edit().putBoolean(DataSimChanged.IS_SIM_PLUG_OUT + slotId, false).apply();
            prefs.edit().putBoolean(DataSimChanged.IS_SIM_PLUG_IN + slotId, false).apply();
            initImsModule(slotId);
            startImsModule(slotId);
        }

        if (logger.isActivated()) {
            logger.info("Terminal core is created with success");
        }
    }

    /**
     * Returns the event listener
     *
     * @return Listener
     */
    public CoreListener getListener() {
        return listener;
    }

    /**
     * Returns the IMS module
     *
     * @return IMS module
     */
    public ImsModule getImsModule(int slotId) {
        return mImsModule[slotId];
    }

    /**
     * Returns the address book manager
     */
    public AddressBookManager getAddressBookManager(){
        return addressBookManager;
    }

    /**
     * Is core started
     *
     * @return Boolean
     */
    public boolean isCoreStarted() {
        return started;
    }

    /**
     * Start the terminal core
     *
     * @throws CoreException
     */
    public synchronized void startCore() throws CoreException {
        if (started) {
            // Already started
            return;
        }

        // Start the address book monitoring
        addressBookManager.startAddressBookMonitoring();

        // Notify event listener
        listener.handleCoreLayerStarted();

        started = true;
        if (logger.isActivated()) {
            logger.info("Presence service has been started with success");
        }
    }

    /**
     * Stop the terminal core
     */
    public synchronized void stopCore() {
        if (!started) {
            // Already stopped
            return;
        }

        if (logger.isActivated()) {
            logger.info("Stop the Presence service");
        }

        // Stop the address book monitoring
        addressBookManager.stopAddressBookMonitoring();

        // Notify event listener
        listener.handleCoreLayerStopped();

        started = false;
        if (logger.isActivated()) {
            logger.info("RCS core service has been stopped with success");
        }
    }

    public synchronized void initImsModule(int slotId) throws CoreException {
        if (mImsModule == null) {
            return;
        }
        logger.debug("initImsModule for slot: " + slotId);
        mImsModule[slotId] = new ImsModule(this, slotId);
    }

    public synchronized void startImsModule(int slotId) {
        if (mImsModule == null ||
                mImsModule[slotId] == null) {
            return;
        }
        logger.debug("startImsModule for slot: " + slotId);
        mImsModule[slotId].start();
    }

    public synchronized void stopImsModule(int slotId) {
        if (mImsModule == null ||
                mImsModule[slotId] == null) {
            return;
        }
        logger.debug("stopImsModule for slot: " + slotId);
        mImsModule[slotId].stop();
    }
}
