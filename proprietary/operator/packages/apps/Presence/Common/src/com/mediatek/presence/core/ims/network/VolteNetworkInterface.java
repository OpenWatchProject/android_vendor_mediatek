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

package com.mediatek.presence.core.ims.network;



import com.mediatek.presence.core.ims.rcsua.RcsUaAdapter;
import com.mediatek.presence.core.ims.rcsua.RcsUaAdapterManager;
import com.mediatek.presence.core.CoreException;
import com.mediatek.presence.core.access.VolteNetworkAccess;
import com.mediatek.presence.core.access.WifiNetworkAccess;
import com.mediatek.presence.core.ims.ImsModule;
import com.mediatek.presence.core.ims.network.sip.SipManager;
import com.mediatek.presence.platform.AndroidFactory;
import com.mediatek.presence.platform.network.AndroidEPDGNetwork;
import com.mediatek.presence.platform.network.NetworkException;
import com.mediatek.presence.provider.settings.RcsSettings;
import com.mediatek.presence.provider.settings.RcsSettingsData;
import com.mediatek.presence.provider.settings.RcsSettingsManager;
import com.mediatek.presence.utils.logger.Logger;

import android.content.Context;
import android.net.ConnectivityManager;
import android.net.Network;
import android.net.NetworkCapabilities;
import android.os.SystemProperties;
import android.telephony.SubscriptionManager;

/**
 * Volte Network interface for Single Registration
 *
 * @author
 */
public class VolteNetworkInterface extends ImsNetworkInterface {
    /**
     * The logger
     */
    private Logger logger = null;

    private boolean isRegistered = false;

    //if registering still going on
    private static boolean isRegistering = false;

    private int mSlotId = 0;
    /**
     * SIP manager
     */
 //   private SipManager sip;


    /**
     * EPDG connection
     */
    private AndroidEPDGNetwork mEPDGNetworkConnection = null;
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
        super(imsModule, NetworkCapabilities.TRANSPORT_CELLULAR,
                new VolteNetworkAccess(imsModule.getSlotId()),
                RcsUaAdapterManager.getRcsUaAdapter(imsModule.getSlotId()).getImsProxyAddrForVoLTE(),
                RcsUaAdapterManager.getRcsUaAdapter(imsModule.getSlotId()).getImsProxyPortForVoLTE(),
                RcsUaAdapterManager.getRcsUaAdapter(imsModule.getSlotId()).getSIPDefaultProtocolForVoLTE(),
                RcsSettingsData.VOLTE_AUTHENT);

        mSlotId = imsModule.getSlotId();
        logger = Logger.getLogger(mSlotId, "VolteNetworkInterface");
        //set this interface as used for single registartion
        this.setSingleRegistrationInterface(true);
        /**
         * M: add for MSRPoTLS
         */
        if(RcsSettingsManager.getRcsSettingsInstance(mSlotId)
                .isSecureMsrpOverMobile()){
            logger.info("MobileNetworkInterface initSecureTlsMsrp0");
            initSecureTlsMsrp(false);
        }
        else if(RcsSettingsManager.getRcsSettingsInstance(mSlotId)
                .getSipDefaultProtocolForMobile() == "TLS"){
            logger.info("MobileNetworkInterface initSecureTlsMsrp1");
            initSecureTlsMsrp(false);
        }
        else{
            logger.info("MobileNetworkInterface initSecureTlsMsrp2");
            initSecureTlsMsrp(false);
            }
        /**
         * @}
         */

        // Instantiates the SIP manager
       // sip = new SipManager(this);



        if (logger.isActivated()) {
            logger.info("RCS VOLTE network interface has been loaded");
        }
    }


    public void UpdateVolteDetails(){
        super.UpdateVolteDetails(RcsUaAdapterManager.getRcsUaAdapter(mSlotId).getImsProxyAddrForVoLTE(),
                RcsUaAdapterManager.getRcsUaAdapter(mSlotId).getImsProxyPortForVoLTE());
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
    public boolean register(int pcscaddress) {

        isRegistered = volteRegister(pcscaddress);

        if (RcsUaAdapterManager.getRcsUaAdapter(mSlotId).isRegisteredbyRoI()) {
            logger.info("skip ePDGConnection");
        } else if (isRegistered) {
            if (RcsSettingsManager.getRcsSettingsInstance(mSlotId)
                    .isEPDGConnectionRequired()) {
                //enable the IMS ePDG connection and make it default for the process
                establishIMSConnection();
            }
        }
        return isRegistered;
    }



    private void establishIMSConnection(){

        if (logger.isActivated()) {
            logger.info("establishIMSConnection for ePDG");
        }

        Network epdgNetwork = null;
        try {
                // Set the connectivity manager
                connectivityMgr = (ConnectivityManager)AndroidFactory.getApplicationContext().getSystemService(Context.CONNECTIVITY_SERVICE);

                mEPDGNetworkConnection = new AndroidEPDGNetwork(AndroidFactory.getApplicationContext());

                //long subID= (long)1 ;//LauncherUtils.getsubId();
                int[] subIds = SubscriptionManager.getSubId(mSlotId);
                if (subIds == null) {
                    logger.debug("establishIMSConnection fail. subIds is null");
                    return;
                }
                long subId = (long)subIds[0];
                mEPDGNetworkConnection.acquireNetwork(subId);

                epdgNetwork = mEPDGNetworkConnection.getNetwork();

        } catch (NetworkException e) {
            if (logger.isActivated()) {
                logger.info("establishIMSConnection : exception " + e.getMessage());
            }
            // for case: AirPlane mode with WiFi cinnected
            if (e.getMessage().contains("airplane")) {
                logger.info("getNetworkForType(TYPE_MOBILE_IMS) for Wi-Fi in AirPlane mode");
                epdgNetwork = connectivityMgr.getNetworkForType(
                        ConnectivityManager.TYPE_MOBILE_IMS);
            }

        } finally {
            if (epdgNetwork != null) {
                logger.debug("EPDG epdgNetwork : " + epdgNetwork);
                connectivityMgr.bindProcessToNetwork(epdgNetwork);
                //epdgNetwork.bindProcessForHostResolution();
                connectivityMgr.setProcessDefaultNetworkForHostResolution(epdgNetwork);

            }
        }
    }

    private void releaseIMSConnection(){
        if(mEPDGNetworkConnection!=null){
            if (logger.isActivated()) {
                logger.info("releaseIMSConnection");
            }

            //remove epdg as default nw
            connectivityMgr.bindProcessToNetwork(null);

            mEPDGNetworkConnection.releaseNetwork();
        }
    }
    /*
     * Unregister from the VOLTE IMS
     */
    public void unregister() {
        if (logger.isActivated()) {
            logger.debug("Unregister from IMS:" + isRegistered);
        }

        //clsose edg pdn connection
         if(RcsSettingsManager.getRcsSettingsInstance(mSlotId)
                .isEPDGConnectionRequired()){
             //release the IMS ePDG connection
             releaseIMSConnection();
            }

        volteUnregister();

        //close mEPDGNetworkConnection
        isRegistered = false;

    }

    public void registrationTerminated(){
        if(!isRegistered()){
            getSipManager().closeStack();
            return;
        }
        unregister();
    }

}
