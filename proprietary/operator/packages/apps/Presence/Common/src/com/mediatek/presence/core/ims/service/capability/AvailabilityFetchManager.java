/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2018. All rights reserved.
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

package com.mediatek.presence.core.ims.service.capability;

import android.content.Context;
import android.content.Intent;
import android.telephony.TelephonyManager;
import android.telephony.SubscriptionManager;

import com.mediatek.presence.core.CoreListener;
import com.mediatek.presence.core.ims.ImsModule;
import com.mediatek.presence.core.ims.network.sip.SipMessageFactory;
import com.mediatek.presence.core.ims.network.sip.SipUtils;
import com.mediatek.presence.core.ims.protocol.sip.SipDialogPath;
import com.mediatek.presence.core.ims.protocol.sip.SipException;
import com.mediatek.presence.core.ims.protocol.sip.SipRequest;
import com.mediatek.presence.core.ims.protocol.sip.SipResponse;
import com.mediatek.presence.core.ims.protocol.sip.SipTransactionContext;
import com.mediatek.presence.core.ims.rcsua.RcsUaAdapter;
import com.mediatek.presence.core.ims.service.capability.NotificationHelper.NotificationInfo;
import com.mediatek.presence.core.ims.service.capability.NotificationHelper.PresentifyInfo;
import com.mediatek.presence.core.ims.service.ContactInfo;
import com.mediatek.presence.core.ims.service.presence.rlmi.ResourceInstance;

import com.mediatek.presence.platform.AndroidFactory;
import com.mediatek.presence.provider.eab.ContactsManager;
import com.mediatek.presence.utils.ContactNumberUtils;
import com.mediatek.presence.utils.logger.Logger;
import com.mediatek.presence.utils.PstUtils;
import com.mediatek.presence.service.api.PresenceServiceImpl;

import java.util.ArrayList;
import java.util.List;
import java.util.Vector;
import java.lang.StringBuilder;


import javax2.sip.header.SubscriptionStateHeader;


public class AvailabilityFetchManager {

    private ImsModule mImsModule;
    private CoreListener mCoreListener;

    private Context mContext;
    private TelephonyManager mTelephonyManager;

    private ContactsManager mContactsManager;
    private Logger logger = null;

    private boolean mIsServiceStarted = false;

    private List<VzwSubscription> mSubscribeRequests = new ArrayList<VzwSubscription>();

    // error cause
    private final int AVAILABILITY_FETCH_403_FAIL = 403;
    private final int AVAILABILITY_FETCH_404_FAIL = 404;
    private final int AVAILABILITY_FETCH_408_FAIL = 408;
    private final int AVAILABILITY_FETCH_500_FAIL = 500;
    private final int AVAILABILITY_FETCH_503_FAIL = 503;
    private final int AVAILABILITY_FETCH_603_FAIL = 603;

    private boolean mRecoverImsRgistered = false;

    private int mSlotId = 0;

    /**
     * RequestId which shall be sent to client when receiveing SIP response (AOSP procedure)
     */
    private int mRequestId = -1;

   /**
     * Constructor
     *
     * @param parent
     *            IMS module
     * @throws CoreException
     */
    public AvailabilityFetchManager(ImsModule imsModule) {

        mSlotId = imsModule.getSlotId();
        logger = Logger.getLogger(mSlotId, "AvailabilityFetchManager");
        logger.debug("AvailabilityFetchManager.construct()");
        mImsModule = imsModule;
        mCoreListener = imsModule.getCoreListener();
        mContext = AndroidFactory.getApplicationContext();
        mContactsManager = ContactsManager.getInstance();
        mTelephonyManager = (TelephonyManager) mContext.getSystemService(Context.TELEPHONY_SERVICE);
    }

   public void startService() {
       logger.debug("startService");
       mIsServiceStarted = true;
       mRecoverImsRgistered = false;
       resumeAllSubscriptionTasks();
   }

    public void stopService() {
       logger.debug("stopService");
       mIsServiceStarted = false;
       abortAllSubscriptionTasks();
       if (mRecoverImsRgistered == false) {
           mSubscribeRequests.clear();
       }
    }

   public void onForbiddenReceived(ImsModule.EabActions action, String reason) {
        logger.debug("onForbiddenReceived reason: " + reason);

        if (reason.indexOf("User Not Registered") > -1) {
            mRecoverImsRgistered = true;
        } else {
            abortAllSubscriptionTasks();
            mSubscribeRequests.clear();
        }
   }

   public void onNotProvisionedReceived() {
   }

   public void putSubscription(String contact) {
        logger.debug("putAvailabilitySubscription");

        if (!mImsModule.isProvisioned()) {
                logger.debug("Device not be Provisioned");
                return;
            }
        if (!PstUtils.getCapabilityDiscoveryEnabled(mSlotId, mContext)) {
            logger.debug("capability discovery service is disabled");
            return;
        }

        if (!PstUtils.isLvcProvisioned(mSlotId, mContext)) {
            logger.debug("vilte disabled, ignore the request");
            return;
        }

        ContactInfo contactInfo = mContactsManager.getContactInfo(contact);
        if (contactInfo.getRcsStatus() != ContactInfo.RCS_CAPABLE) {
             logger.debug("contact(" + contact + ") rcs status: " + contactInfo.getRcsStatus());
            return;
        }

        //check network type
        int[] subIds = SubscriptionManager.getSubId(mSlotId);
        if (subIds == null) {
            logger.debug("putSubscription fail. subIds is null");
            return;
        }
        int network = mTelephonyManager.getNetworkType(subIds[0]);
        if (network != TelephonyManager.NETWORK_TYPE_LTE &&
            network != TelephonyManager.NETWORK_TYPE_EHRPD &&
            network != TelephonyManager.NETWORK_TYPE_IWLAN) {
                logger.debug("network" + network + "is not support availability");
                return;
            }

        if (network == TelephonyManager.NETWORK_TYPE_EHRPD &&
            contactInfo.getCapabilities().isImSessionSupported() == false) {
            logger.debug("RCS must support chat at eHRPD netowrk");
            return;
        }

        int cacheExpiration = PstUtils.getAvailabilityCacheExpiration(mSlotId, mContext);
        if (cacheExpiration < 0) {
            logger.error("AvailabilityCacheExpiration can not less 0");
            return;
        }
        long delta = (System.currentTimeMillis() - contactInfo.getCapabilities().getTimestamp()) / 1000;
        if (delta < cacheExpiration) {
            logger.debug("contact (" + contact + ") delta: " + delta + " cacheExpiration:" + cacheExpiration);
            mCoreListener.handleCapabilitiesNotification(contact, contactInfo.getCapabilities());
            return;
        }

        for (VzwSubscription info : mSubscribeRequests) {
            if (info.getContact().equalsIgnoreCase(contact)) {
                logger.debug("contact(" + contact + ") existed");
                return;
            }
        }

        VzwSubscription subscribe = new VzwSubscription(contact, mRequestId);
        mSubscribeRequests.add(subscribe);
        if (mIsServiceStarted) {
            subscribe.initialize();
            subscribe.start();
        }
   }

   public void putNotification(SipRequest notify) {

        if(mIsServiceStarted == false) {
            logger.debug("service offline, because ims not registered, drop the notification: " + notify.getCallId());
            return;
        }

        for (VzwSubscription subscribe : mSubscribeRequests) {
            if (notify.getCallId().equalsIgnoreCase(subscribe.getIdentity())) {
                logger.debug("Got notification for contact: " + subscribe.getContact());
                mSubscribeRequests.remove(subscribe);
                NotificationHelper notificationHelper = NotificationHelper.getInstance();
                notificationHelper.setImsModule(mImsModule);
                NotificationInfo info = notificationHelper.parsingNotification(notify);
                //handling presentify information
                List<PresentifyInfo> presentifyList = info.presentifyList;
                if (presentifyList != null && presentifyList.size() > 0) {
                    for (PresentifyInfo presentify : presentifyList) {
                        mContactsManager.setContactCapabilities(presentify.contact, presentify.capabilities, ContactInfo.RCS_CAPABLE, presentify.rcsStatus);
                         logger.debug("notify capabilities to ap");
                         logger.debug("Contact: " + presentify.contact + " Capabilities: " + presentify.capabilities);
                        mCoreListener.handleCapabilitiesNotification(presentify.contact, presentify.capabilities);
                    }
                }
            }
        }
    }

    private void abortAllSubscriptionTasks() {
        logger.debug("abortAllSubscriptionTasks");

        for(VzwSubscription subscribe: mSubscribeRequests) {
            if (subscribe.isAlive( )) {
                try {
                    logger.debug("contact: " + subscribe.getContact() + " still alive");
                    subscribe.join();
                    logger.debug("contact: " + subscribe.getContact() + " is death");
                } catch (InterruptedException e) {
                    logger.debug("exception: " + e);
                }
            }
       }
    }

    private void resumeAllSubscriptionTasks() {
        for(VzwSubscription lastSubscribe: mSubscribeRequests) {
            VzwSubscription vzwSubscription = new VzwSubscription(
                    lastSubscribe.getContact(), lastSubscribe.getRequestId());
            vzwSubscription.initialize();
            vzwSubscription.start();
            mSubscribeRequests.add(vzwSubscription);
            mSubscribeRequests.remove(0);
        }
    }

    public void setRequestId(int requestId) {
        mRequestId = requestId;
    }

    private class VzwSubscription extends Thread {

        private String mContact;
        private SipRequest mRequest;
        private int mRequestId;

        public VzwSubscription(String contact, int requestId) {
            mContact = contact;
            mRequestId = requestId;
        }

        public void initialize() {
            try {
                   String contactUri = ImsModule.IMS_USER_PROFILE.getPublicUri();
                   String callId = mImsModule.getSipManager().getSipStack().generateCallId();
                   String target = contactUri;
                   String localParty =  "\"Anonymous\" <sip:anonymous@anonymous.invalid>";
                   String remoteParty = contactUri;
                   Vector<String> route = mImsModule.getSipManager().getSipStack().getServiceRoutePath();
                   logger.debug("contactUri: " + contactUri + " callId: " + callId);
                   SipDialogPath dialog = new SipDialogPath(mImsModule.getSipManager().getSipStack(),
                           callId, 1, target, localParty, remoteParty, route);
                   int slotId = mImsModule.getSlotId();
                   mRequest = SipMessageFactory.createEABIndividualSubscribe(
                           slotId, VzwCapabilityService.SINGLE_SUBSCRIBE_TYPE_AVAILABILITY, dialog, 0, mContact);
               } catch (Exception e) {
                   if (logger.isActivated()) {
                       logger.error("buildSubscribeSipRequest has failed", e);
                   }
               }
        }

        public void run(){
            int result = 0;
            SipTransactionContext trans;
            try {
                if (!mIsServiceStarted) {
                    logger.debug("service offline, because ims not registered");
                    return;
                }
                trans = mImsModule.getSipManager().sendSipMessageAndWait(mRequest);

                if (mRequestId != -1) {
                    logger.info("Notify the SipResponse with requestId = "
                            + String.valueOf(mRequestId));
                    PresenceServiceImpl.receiveSipResponse(mRequestId, trans.getSipResponse());
                }

                // Analyze the received response
                if (trans.isSipResponse()) {
                    // A response has been received
                    result = trans.getStatusCode();
                    if (result >= 200 && result < 300) {
                        logger.debug("200OK");
                        return;
                    }
                    logger.debug("status: " + trans.getStatusCode() + " reason: " + trans.getReasonPhrase());
                    switch(result) {
                        case AVAILABILITY_FETCH_403_FAIL:
                            handle403Forbidden(trans);
                            break;
                        case AVAILABILITY_FETCH_404_FAIL:
                            handle404UserNotFound(trans);
                            break;
                        default:
                            logger.debug("Do not retry for  (" + trans.getStatusCode() + " error)");
                            mSubscribeRequests.remove(this);
                            break;
                    }
                }
            } catch (Exception e) {
                if (logger.isActivated()) {
                    logger.error("sendSubscribe has failed", e);
                }
            }
        }

        public String getIdentity() {
            if (mRequest != null) {
                return mRequest.getCallId();
            }
            return null;
        }

        public String getContact() {
            return mContact;
        }

        public int getRequestId() {
            return mRequestId;
        }

        /**
         * Handle user not found
         *
         * @param ctx SIP transaction context
         */
        private void handle404UserNotFound(SipTransactionContext ctx) {
            logger.debug("handle404UserNotFound");
            mSubscribeRequests.remove(0);
            // We update the database with empty capabilities
            Capabilities capabilities = new Capabilities();
            ContactsManager.getInstance().setContactCapabilities(mContact, capabilities, ContactInfo.NOT_RCS, ContactInfo.REGISTRATION_STATUS_UNKNOWN);
        }

        private void handle403Forbidden(SipTransactionContext ctx) {
           logger.debug("handle403Forbidden");
           mImsModule.onForbiddenReceived(ImsModule.EabActions.AVAILABILITY, ctx.getReasonPhrase());
        }
    }
}
