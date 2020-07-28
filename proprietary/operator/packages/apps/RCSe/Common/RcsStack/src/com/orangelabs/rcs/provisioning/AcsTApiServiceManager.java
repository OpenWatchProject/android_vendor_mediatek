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

package com.orangelabs.rcs.provisioning;

import android.content.Context;
import android.util.Log;

import com.gsma.services.rcs.RcsServiceException;
import com.gsma.services.rcs.RcsServiceListener;
import com.gsma.services.rcs.capability.Capabilities;
import com.gsma.services.rcs.capability.CapabilityService;

import com.orangelabs.rcs.utils.logger.Logger;

public class AcsTApiServiceManager {

    private Logger logger = Logger.getLogger(
            AcsTApiServiceManager.class.getSimpleName());
    private static AcsTApiServiceManager sInstance = null;

    private CapabilityService mCapabilitiesApi = null;
    private static final String TAG = "AcsTApiServiceManager";

    /**
     * initialize
     *
     *
     * @param context
     *            The Context of this application.
     * @return true If initialize successfully, otherwise false.
     */
    public static synchronized boolean initialize(Context context) {
        Log.d(TAG, "initialize() entry");
        if (null != sInstance) {
            Log.w(TAG, "initialize() sInstance has existed, " +
                    "is it really the first time you call this method?");
            return true;
        } else {
            if (null != context) {
                AcsTApiServiceManager apiManager = new AcsTApiServiceManager(context);
                sInstance = apiManager;
                return true;
            } else {
                Log.e(TAG, "initialize() the context is null");
                return false;
            }
        }
    }

    /**
     * Get the instance of AcsTApiServiceManager.
     *
     * @return The instance of AcsTApiServiceManager, or null if the instance has not been
     *         initialized.
     */
    public static AcsTApiServiceManager getInstance() {
        return sInstance;
    }

    /**
     * Get the connected CapabilityApi.
     *
     * @return The instance of CapabilityApi, or null if the instance has not
     *         connected.
     */
    public CapabilityService getCapabilityApi() {
        logger.debug("getCapabilityApi()");
        return mCapabilitiesApi;
    }

    public Capabilities getMyCapabilities() {
        Capabilities capabilities = null;
        if (mCapabilitiesApi != null) {
            try {
                capabilities = mCapabilitiesApi.getMyCapabilities();
            } catch (RcsServiceException e) {
            }
        }

        return capabilities;
    }

    public void publishMyCapabilities(boolean persistent, Capabilities capabilities) {
        if (mCapabilitiesApi != null && capabilities != null) {
            try {
                mCapabilitiesApi.publishMyCap(persistent, capabilities);
            } catch (RcsServiceException e) {
            }
        }
    }

    /**
     * Instantiates a new api manager.
     *
     * @param context the context
     */
    private AcsTApiServiceManager(Context context) {
        logger.debug("AcsTApiServiceManager() entry");
        try {
            mCapabilitiesApi = new CapabilityService(context,
                    new MyCapabilitiesServiceListener());
            mCapabilitiesApi.connect();
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    /**
     * The listener interface for receiving myCapabilitiesService events.
     * The class that is interested in processing a myCapabilitiesService
     * event implements this interface, and the object created
     * with that class is registered with a component using the
     * component's addMyCapabilitiesServiceListener method. When
     * the myCapabilitiesService event occurs, that object's appropriate
     * method is invoked.
     *
     * @see MyCapabilitiesServiceEvent
     */
    public class MyCapabilitiesServiceListener implements RcsServiceListener {

        /**
         * On service connected.
         */
        @Override
        public void onServiceConnected() {
            logger.debug("onServiceConnected()");
        }

        /**
         * On service disconnected.
         *
         * @param error the error
         */
        @Override
        public void onServiceDisconnected(ReasonCode reasonCode) {
            logger.debug("onServiceDisconnected()");
            //AcsTApiServiceManager.this.mCapabilitiesApi = null;
            mCapabilitiesApi.connect(); //connect again
        }

    }
}
