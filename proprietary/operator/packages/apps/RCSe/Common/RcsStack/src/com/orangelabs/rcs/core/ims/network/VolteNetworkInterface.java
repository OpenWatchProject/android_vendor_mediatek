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

import android.net.ConnectivityManager;

import com.mediatek.ims.rcsua.*;
import com.orangelabs.rcs.core.ims.network.registration.RegistrationUtils;
import com.orangelabs.rcs.core.ims.protocol.sip.SipInterface;
import com.orangelabs.rcs.core.CoreException;
import com.orangelabs.rcs.core.access.VolteNetworkAccess;
import com.orangelabs.rcs.core.ims.ImsModule;
import com.orangelabs.rcs.platform.AndroidFactory;
import com.orangelabs.rcs.platform.network.AndroidImsNetwork;
import com.orangelabs.rcs.platform.network.NetworkException;
import com.orangelabs.rcs.provider.settings.RcsSettings;
import com.orangelabs.rcs.provider.settings.RcsSettingsData;
import com.orangelabs.rcs.service.UaServiceManager;
import com.orangelabs.rcs.utils.logger.Logger;
import android.content.Context;
import android.net.Network;
import android.telephony.SubscriptionManager;
import javax2.sip.ListeningPoint;

/**
 * Volte Network interface for Single Registration
 *
 * @author
 */
public class VolteNetworkInterface extends ImsNetworkInterface {
    /**
     * The logger
     */
    private Logger logger = Logger.getLogger(this.getClass().getName());

    private boolean isRegistered = false;

    /**
     * EPDG connection
     */
    private AndroidImsNetwork mImsNetworkConnection = null;
    /**
     * Connectivity manager
     */
    private ConnectivityManager connectivityMgr;

    /**
     * Constructor
     *
     * @param imsModule IMS module
     * @throws CoreException
     */
    public VolteNetworkInterface(ImsModule imsModule) throws CoreException {
        super(imsModule, ConnectivityManager.TYPE_MOBILE,
                new VolteNetworkAccess(),
                new String[] {""},
                new int[] {0},
                ListeningPoint.TCP,
                RcsSettingsData.VOLTE_AUTHENT);

        //set this interface as used for single registartion
        this.setSingleRegistrationInterface(true);
        /**
         * M: add for MSRPoTLS
         */
        if(RcsSettings.getInstance().isSecureMsrpOverMobile()){
            logger.info("MobileNetworkInterface initSecureTlsMsrp0");
            initSecureTlsMsrp(false);
        }
        else if(RcsSettings.getInstance().getSipDefaultProtocolForMobile() == "TLS"){
            logger.info("MobileNetworkInterface initSecureTlsMsrp1");
            initSecureTlsMsrp(false);
        }
        else {
            logger.info("MobileNetworkInterface initSecureTlsMsrp2");
            initSecureTlsMsrp(false);
        }
        /**
         * @}
         */

        if (logger.isActivated()) {
            logger.info("RCS VOLTE network interface has been loaded");
        }
    }

    /**
     * Is registered
     *
     * @return Return True if the terminal is registered, else return False
     */
    public boolean isRegistered() {
        return isRegistered;
    }


    //send register request to the volte_rcs_proxy
    @Override
    public boolean register(int pcscaddress) {

        isRegistered = volteRegister(pcscaddress);

         if (isRegistered) {
             if (!imsOverInternet) {
                //enable the IMS connection and make it default for the process
                establishIMSConnection();
            }
         }

         return isRegistered;
    }

    public void reRegistration() {
        UaServiceManager.getInstance().triggerReregistration();
    }

    private void establishIMSConnection() {
        if (logger.isActivated()) {
            logger.info("establishIMSConnection for ePDG");
        }

        try {
            // Set the connectivity manager
            connectivityMgr = (ConnectivityManager)AndroidFactory.getApplicationContext().getSystemService(Context.CONNECTIVITY_SERVICE);

            mImsNetworkConnection = new AndroidImsNetwork(AndroidFactory.getApplicationContext());

            //long subID= (long)1 ;//LauncherUtils.getsubId();
            long subID = (long)SubscriptionManager.getDefaultDataSubscriptionId();
            mImsNetworkConnection.acquireNetwork(subID);

            Network imsNetwork = mImsNetworkConnection.getNetwork();
            logger.debug("IMS network : "+ imsNetwork);
            connectivityMgr.setProcessDefaultNetwork(imsNetwork);
            //epdgNetwork.bindProcessForHostResolution();
            connectivityMgr.setProcessDefaultNetworkForHostResolution(imsNetwork);

        } catch(NetworkException e) {
            if (logger.isActivated()) {
                logger.info("establishIMSConnection : exception " + e.getMessage());
            }
        }
    }

    private void releaseIMSConnection(){
        if (mImsNetworkConnection != null) {
            if (logger.isActivated()) {
                logger.info("releaseIMSConnection");
            }

            //remove epdg as default nw
            connectivityMgr.setProcessDefaultNetwork(null);

            mImsNetworkConnection.releaseNetwork();
        }
    }
    /*
     * Unregister from the VOLTE IMS
     */
    @Override
    public void unregister() {
        if (logger.isActivated()) {
            logger.debug("Unregister from IMS: " + isRegistered);
        }

        //clsose edg pdn connection
        if (RcsSettings.getInstance().isEPDGConnectionRequired()){
             //release the IMS ePDG connection
             releaseIMSConnection();
        }

        volteUnregister();

        //close mEPDGNetworkConnection
        isRegistered = false;

    }

    public void registrationTerminated() {
        if(!isRegistered()){
            getSipManager().closeStack();
            return;
        }
        unregister();
    }

    void setImsOverInternet(boolean imsOverInternet) {
        this.imsOverInternet = imsOverInternet;
    }

    /*@*/

    private boolean imsOverInternet = false;
}
