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
import android.database.Cursor;
import android.provider.ContactsContract.CommonDataKinds.Phone;
import android.os.Looper;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.Message;
import android.telephony.ims.ProvisioningManager;

import com.android.ims.ImsConfig;
import com.android.ims.ImsManager;
import com.android.internal.util.State;
import com.android.internal.util.StateMachine;

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

import com.mediatek.presence.core.ims.service.im.chat.ChatUtils;
import com.mediatek.presence.core.ims.service.presence.rlmi.ResourceInstance;

import com.mediatek.presence.service.api.PresenceServiceImpl;
import com.mediatek.presence.platform.AndroidFactory;
import com.mediatek.presence.provider.eab.ContactsManager;
import com.mediatek.presence.utils.ContactNumberUtils;
import com.mediatek.presence.utils.ImsConfigMonitorService;
import com.mediatek.presence.utils.logger.Logger;
import com.mediatek.presence.utils.PhoneUtils;
import com.mediatek.presence.utils.PstUtils;

import java.io.ByteArrayOutputStream;

import java.util.Arrays;
import java.util.ArrayList;
import java.util.List;
import java.util.Vector;
import java.util.zip.GZIPOutputStream;

import javax2.sip.header.SubscriptionStateHeader;

import gov2.nist.javax2.sip.header.ParameterNames;


public class CapabilityPollingManager extends StateMachine {

    //COMMANDS
    private final int EVENT_NOTIFY = 0;
    private final int EVENT_SUBSCRIBE = 1;
    private final int EVENT_SUBSCRIBING = 2;
    private final int EVENT_SUBSCRIBE_TIMEOUT = 3;
    private final int EVENT_SUBSCRIBE_PERIODIC = 4;
    private final int EVENT_SUBSCRIBE_ERROR = 5;
    private final int EVENT_SERVICE_START = 6;
    private final int EVENT_SERVICE_STOP = 7;
    private final int EVENT_PROVISION_VALUE_CHANGED = 8;
    private final int EVENT_CAPABILITIES_CACHE_EXPIRED = 9;

    private ImsModule mImsModule;
    private CoreListener mCoreListener;

    private String mCurrentState = "DefaultState";
    private DefaultState mDefaultState = new DefaultState();
    private IdleState mIdleState = new IdleState();
    private SubscribeState mSubscribeState = new SubscribeState();
    private NotifyState mNotifyState = new NotifyState();
    private RecoverState mRecoverState = new RecoverState();
    private Handler mHandler;

    private Context mContext;
    private ContactsManager mContactsManager;
    private Logger logger = null;

    //Contacts Changed
    private int mWaitForContactStable = 5000;
    private ArrayList<String> mLastToBeTreatedNumbers = new ArrayList<String>();

    //Retry capability polling
    public final int CAPABILITY_POLLING_USER_APPLICATION = 0;
    private final int CAPABILITY_POLLING_USER_SYSTEM = 1;
    private int mRetryCount = 0;

    // error cause
    private final int CAPABILITY_POLLING_403_FAIL = 403;
    private final int CAPABILITY_POLLING_408_FAIL = 408;
    private final int CAPABILITY_POLLING_423_FAIL = 423;
    private final int CAPABILITY_POLLING_500_FAIL = 500;
    private final int CAPABILITY_POLLING_503_FAIL = 503;
    private final int CAPABILITY_POLLING_603_FAIL = 603;

    //forbidden handling
    private ImsModule.EabActions mAction = ImsModule.EabActions.NONE;
    private boolean mIsForbidden = false;

    private ImsConfigMonitorService mImsConfigMonitorService;

    private int mSlotId = 0;

    /**
     * RequestId which shall be sent to client when receiveing SIP response (AOSP procedure)
     */
    private int mRequestId = -1;

    private class VzWSipRequest {
        int user;
        SipRequest sipRequest;
        List<String> contactList;
    }

    private List<VzWSipRequest> mSubscribeRequests = new ArrayList<VzWSipRequest>();

    public CapabilityPollingManager(String name, Handler handler, ImsModule imsModule) {
        super(name, handler);

        mImsModule = imsModule;
        mSlotId = imsModule.getSlotId();
        mCoreListener = imsModule.getCoreListener();
        mContext = AndroidFactory.getApplicationContext();
        mContactsManager = ContactsManager.getInstance();
        mHandler = handler;
        mImsConfigMonitorService = ImsConfigMonitorService.getInstance(mContext);
        logger = Logger.getLogger(mSlotId, "CapabilityPollingManager");

        addState(mDefaultState);
        addState(mIdleState, mDefaultState);
        addState(mSubscribeState, mDefaultState);
        addState(mNotifyState, mDefaultState);
        addState(mRecoverState, mDefaultState);
        setInitialState(mIdleState);
        start();
    }

    public void putSubscription(List<String> contactList, int user) {

        if (!PstUtils.getCapabilityDiscoveryEnabled(mSlotId, mContext)) {
            logd("capability discovery service is disabled");
            return;
        }

        int maxCount = PstUtils.getMaxNumbersInRCL(mSlotId, mContext);
        if (maxCount < 0) {
            logger.error("MaxNumbersInRCL can not less 0");
            return;
        }
        logd("contactList size: " + contactList.size());
        while (contactList.size() > 0) {
            VzWSipRequest sipRequest = new VzWSipRequest();
            sipRequest.user = user;
            if (contactList.size() > maxCount) {
                logd("contacts's count: " + contactList.size() + " big than MaxNumber: " + maxCount);
                List<String> subList = contactList.subList(0, maxCount);
                sipRequest.contactList = new ArrayList(subList);
                sipRequest.sipRequest = buildListSubscribeSipRequest(subList);
                subList.clear();
            } else {
                if (contactList.size() == 1) {
                    sipRequest.sipRequest = buildSingleSubscribeSipRequest(contactList.get(0));
                } else {
                    sipRequest.sipRequest = buildListSubscribeSipRequest(contactList);
                }

                sipRequest.contactList = new ArrayList(contactList);
                contactList.clear();
            }
            if (sipRequest == null) {
                logger.error("request can't be null");
                return;
            }
            mSubscribeRequests.add(sipRequest);
        }

        Message msg = obtainMessage(EVENT_SUBSCRIBE);
        sendMessage(msg);
    }

    public void putNotification(SipRequest notify) {
        Message msg = obtainMessage(EVENT_NOTIFY, notify);
        sendMessage(msg);
    }

    public void startService() {
        logd("startService");

        Message msg = obtainMessage(EVENT_SERVICE_START);
        sendMessage(msg);
    }

    public void stopService() {
        logd("stopService");

        clearRetryStatus();
        Message msg = obtainMessage(EVENT_SERVICE_STOP);
        sendMessage(msg);
    }

    public void onForbiddenReceived(ImsModule.EabActions action, String reason) {
        logd("onForbiddenReceived");
        if (reason.indexOf("Not authorized for Presence") > -1) {
            // Periodic Processing finish, go to next run.
            if (mSubscribeRequests.size() > 0) {
                if (mSubscribeRequests.get(0).user == CAPABILITY_POLLING_USER_SYSTEM) {
                    handleCapabilityPollingInterval();
                }
                mSubscribeRequests.remove(0);
            }
            transitionTo(mIdleState);
            Message next = obtainMessage(EVENT_SUBSCRIBE);
            sendMessage(next);
        } else {
            // reason: User Not Registered
            mAction = action;
            mIsForbidden = true;
        }
    }

    public void onNotProvisionedReceived() {
    }

    private SipRequest buildListSubscribeSipRequest(List<String> contactList) {

        SipRequest request = null;

        try {
               String contactUri = ImsModule.IMS_USER_PROFILE.getPublicUri();
               String callId = mImsModule.getSipManager().getSipStack().generateCallId();
               String target = contactUri;
               String localParty = "\"Anonymous\" <sip:anonymous@anonymous.invalid>";
               String remoteParty = contactUri;
               Vector<String> route = mImsModule.getSipManager().getSipStack().getServiceRoutePath();
               SipDialogPath dialog = new SipDialogPath(mImsModule.getSipManager().getSipStack(),
                       callId, 1, target, localParty, remoteParty, route);
               int expiresd = PstUtils.getCapabilityPollListSubscriptionExpiration(mSlotId, mContext);
               String resList = ChatUtils.generateSubscribeResourceList(contactList, "dummy-rfc5367");
               ByteArrayOutputStream out = new ByteArrayOutputStream();
               GZIPOutputStream gzip = new GZIPOutputStream(out);
               gzip.write(resList.getBytes("UTF-8"));
               gzip.close();
               int slotId = mImsModule.getSlotId();
               request = SipMessageFactory.createEABListSubscribe(slotId, dialog, expiresd, out.toByteArray());
               //request = SipMessageFactory.createEABListSubscribe(dialog, expiresd, resList);
           } catch (Exception e) {
               if (logger.isActivated()) {
                   logger.error("buildSubscribeSipRequest has failed", e);
               }
           }
        return request;
    }

    private SipRequest buildSingleSubscribeSipRequest(String contact) {
        SipRequest request = null;

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
            request = SipMessageFactory.createEABIndividualSubscribe(
                    slotId, VzwCapabilityService.SINGLE_SUBSCRIBE_TYPE_CAPABILITY,
                    dialog, 0, contact);
        } catch (Exception e) {
            if (logger.isActivated()) {
                logger.error("buildSubscribeSipRequest has failed", e);
            }
        }

        return request;
    }

    private class DefaultState extends State {
        @Override
        public void enter() {
            mCurrentState = "DefaultState";
            logd("enter");
        }
        @Override
        public void exit() {
            logd("exit");
        }
        @Override
        public boolean processMessage(Message msg) {
            boolean retVal = HANDLED;
            return retVal;
        }
    }

    private class IdleState extends State {
        @Override
        public void enter() {
            mCurrentState = "IdleState";
            logd("enter");
            removeMessages(EVENT_SUBSCRIBE);
            Message msg = obtainMessage(EVENT_SUBSCRIBE);
            sendMessage(msg);
        }
        @Override
        public void exit() {
            logd("exit");
        }
        @Override
        public boolean processMessage(Message msg) {
            boolean retVal = HANDLED;
            logd("msg=" + msgToString(msg.what));
            if (!mImsModule.isProvisioned()) {
                logd("Device not be Provisioned");
                return retVal;
            }

            Message next;
            switch (msg.what) {
                case EVENT_SERVICE_START:
                    mImsConfigMonitorService.registerProvisionedValueCallback(
                            mSlotId, mProvisioningCallback);
                    checkCacheExpired();
                    if (mIsForbidden == false ||
                        (mIsForbidden == true &&
                        mAction == ImsModule.EabActions.CAPABILITY)) {
                        periodicProcessing();
                    } else {
                        handleCapabilityPollingInterval();
                        mIsForbidden = false;
                    }
                    break;
                case EVENT_SERVICE_STOP:
                    mImsConfigMonitorService.unRegisterProvisionedValueCallback(
                            mSlotId, mProvisioningCallback);
                    mHandler.removeCallbacksAndMessages(null);
                    mSubscribeRequests.clear();
                    break;
                case EVENT_SUBSCRIBE:
                    if (mSubscribeRequests.size() > 0) {
                        //starting subscribe requestion
                        transitionTo(mSubscribeState);
                        next = obtainMessage(EVENT_SUBSCRIBING);
                        sendMessage(next);
                    } else {
                        logd("no request in queue");
                    }
                    break;
                case EVENT_SUBSCRIBE_PERIODIC:
                    periodicProcessing();
                    break;
                case EVENT_PROVISION_VALUE_CHANGED:
                    handleCapabilityPollingInterval();
                    break;
                case EVENT_CAPABILITIES_CACHE_EXPIRED:
                    checkCacheExpired();
                    break;
                default:
                    retVal = NOT_HANDLED;
                    break;
            }
            return retVal;
        }
    }

    private class SubscribeState extends State {
        @Override
        public void enter() {
            mCurrentState = "SubscribeState";
            logd("enter");
        }
        @Override
        public void exit() {
            logd("exit");
        }
        @Override
        public boolean processMessage(Message msg) {
            boolean retVal = HANDLED;
            logd("msg=" + msgToString(msg.what));
            if (!mImsModule.isProvisioned()) {
                logd("Device not be Provisioned");
                return retVal;
            }

            Message next;
            switch (msg.what) {
                case EVENT_SERVICE_START:
                case EVENT_SERVICE_STOP:
                    deferMessage(msg);
                    transitionTo(mIdleState);
                    break;
                case EVENT_SUBSCRIBING:
                    SipTransactionContext trans = sendSubscribe(mSubscribeRequests.get(0).sipRequest);
                    if (trans == null) {
                        loge("sip transaction is null, back to idleState");
                        transitionTo(mIdleState);
                        return retVal;
                    }
                    if (trans.getStatusCode() >= 200 && trans.getStatusCode() < 300) {
                        transitionTo(mNotifyState);
                        next = obtainMessage(EVENT_SUBSCRIBE_TIMEOUT);
                        //Subscription Expiration(Vzw provisioning) + T1: 3sec
                        int delayTime = (PstUtils.getCapabilityPollListSubExp(mSlotId, mContext) + 3) * 1000;
                        sendMessageDelayed(next, delayTime);
                        if (mSubscribeRequests.get(0).user
                                == CAPABILITY_POLLING_USER_SYSTEM) {
                            handleCapabilityPollingInterval();
                        }
                    } else {
                        transitionTo(mRecoverState);
                        next = obtainMessage(EVENT_SUBSCRIBE_ERROR);
                        next.obj = trans;
                        sendMessage(next);
                    }
                    break;
                case EVENT_SUBSCRIBE_PERIODIC:
                    periodicProcessing();
                    break;
                case EVENT_PROVISION_VALUE_CHANGED:
                    handleCapabilityPollingInterval();
                    break;
                case EVENT_CAPABILITIES_CACHE_EXPIRED:
                    checkCacheExpired();
                    break;
                default:
                    retVal = NOT_HANDLED;
                    break;
            }
            return retVal;
        }
    }

    private class NotifyState extends State {
        @Override
        public void enter() {
            mCurrentState = "NotifyState";
            logd("enter");
        }
        @Override
        public void exit() {
            logd("exit");
        }
        @Override
        public boolean processMessage(Message msg) {
            boolean retVal = HANDLED;
            logd("msg=" + msgToString(msg.what));
            if (!mImsModule.isProvisioned()) {
                logd("Device not be Provisioned");
                return retVal;
            }

            Message next;
            switch (msg.what) {
                case EVENT_SERVICE_START:
                case EVENT_SERVICE_STOP:
                    deferMessage(msg);
                    transitionTo(mIdleState);
                    break;
                case EVENT_NOTIFY:
                    SipRequest notify = (SipRequest) msg.obj;
                    SipRequest request = mSubscribeRequests.get(0).sipRequest;
                    if (!notify.getCallId().equalsIgnoreCase(request.getCallId())) {
                        logd("Notify Identity: " + notify.getCallId() + " Subscribe Identity: " + request.getCallId());
                        return retVal;
                    }
                    NotificationHelper notificationHelper = NotificationHelper.getInstance();
                    notificationHelper.setImsModule(mImsModule);
                    NotificationInfo info = notificationHelper.parsingNotification((SipRequest) msg.obj);
                    //Server terminated this subscribe
                    logd("subscriptionState: " + info.subscriptionState);
                    if (SubscriptionStateHeader.TERMINATED.equalsIgnoreCase(info.subscriptionState)) {
                        if (mSubscribeRequests.get(0).user
                                == CAPABILITY_POLLING_USER_SYSTEM) {
                            handleCapabilityPollingInterval();
                        }
                        mSubscribeRequests.remove(0);
                        removeMessages(EVENT_SUBSCRIBE_TIMEOUT);
                        transitionTo(mIdleState);
                        next = obtainMessage(EVENT_SUBSCRIBE);
                        sendMessage(next);
                        return retVal;
                    }

                    //handling rlmi document
                    Vector<ResourceInstance> resList = info.resourceList;
                    if (resList != null && resList.size() > 0) {
                        for(int i = 0; i < resList.size(); i++) {
                            ResourceInstance res = (ResourceInstance)resList.elementAt(i);
                            String contact = res.getUri();
                            String state = res.getState();
                            String reason = res.getReason();
                            if (ResourceInstance.STATUS_TERMINATED.equalsIgnoreCase(state)) {
                                switch(reason) {
                                    case ResourceInstance.REASON_REJECTED:
                                        logd("contact : "+contact + " ; rejected the presence sharing request ");
                                        mContactsManager.setContactCapabilities(contact, new Capabilities(), ContactInfo.RCS_REVOKED,
                                                ContactInfo.REGISTRATION_STATUS_OFFLINE);
                                        break;
                                    case ResourceInstance.REASON_NORESOURCE:
                                        logd("contact : "+contact + " ; not rcs contact ");
                                        mContactsManager.setContactCapabilities(contact, new Capabilities(), ContactInfo.NO_RESOURCE,
                                                ContactInfo.REGISTRATION_STATUS_UNKNOWN);
                                        break;
                                    case ResourceInstance.REASON_EXPIRED:
                                        logd("the subscription already expired");
                                        // Periodic Processing finish, go to next run.
                                        if (mSubscribeRequests.get(0).user
                                                == CAPABILITY_POLLING_USER_SYSTEM) {
                                            handleCapabilityPollingInterval();
                                        }
                                        mSubscribeRequests.remove(0);
                                        removeMessages(EVENT_SUBSCRIBE_TIMEOUT);
                                        transitionTo(mIdleState);
                                        next = obtainMessage(EVENT_SUBSCRIBE);
                                        sendMessage(next);
                                        break;
                                    default:
                                        logd("contact : "+contact + " ; offline now ");
                                        Capabilities capabilities = mContactsManager.getContactCapabilities(contact);
                                        mContactsManager.setContactCapabilities(contact, capabilities, ContactInfo.RCS_CAPABLE,
                                                ContactInfo.REGISTRATION_STATUS_OFFLINE);
                                        break;
                                }
                            }
                        }
                    }

                    //handling presentify information
                    List<PresentifyInfo> presentifyList = info.presentifyList;

                    if (presentifyList != null && presentifyList.size() > 0) {
                        for (PresentifyInfo presentify : presentifyList) {
                            mContactsManager.setContactCapabilities(presentify.contact, presentify.capabilities, ContactInfo.RCS_CAPABLE, presentify.rcsStatus);
                             logd("notify capabilities to ap");
                            mCoreListener.handleCapabilitiesNotification(presentify.contact, presentify.capabilities);
                        }
                    }
                    break;
                    case EVENT_SUBSCRIBE_TIMEOUT:
                        logd("Subscribe already expiry");
                        // Periodic Processing finish, go to next run.
                        //fix 3.26
                        /*
                        if (mSubscribeRequests.get(0).user
                                == CAPABILITY_POLLING_USER_SYSTEM) {
                            handleCapabilityPollingInterval();
                        }*/
                        mSubscribeRequests.remove(0);
                        transitionTo(mIdleState);
                        next = obtainMessage(EVENT_SUBSCRIBE);
                        sendMessage(next);
                        break;
                case EVENT_SUBSCRIBE_PERIODIC:
                    periodicProcessing();
                    break;
                case EVENT_PROVISION_VALUE_CHANGED:
                    handleCapabilityPollingInterval();
                    break;
                case EVENT_CAPABILITIES_CACHE_EXPIRED:
                    checkCacheExpired();
                    break;
                default:
                    retVal = NOT_HANDLED;
                    break;
            }
            return retVal;
        }
    }

  private class RecoverState extends State {
        @Override
        public void enter() {
            mCurrentState = "RecoverState";
            logd("enter");
        }
        @Override
        public void exit() {
            logd("exit");
        }
        @Override
        public boolean processMessage(Message msg) {
            SipTransactionContext trans;
            boolean retVal = HANDLED;
            logd("msg=" + msgToString(msg.what));
            if (!mImsModule.isProvisioned()) {
                logd("Device not be Provisioned");
                return retVal;
            }

            Message next;
            switch (msg.what) {
                case EVENT_SERVICE_START:
                case EVENT_SERVICE_STOP:
                    deferMessage(msg);
                    transitionTo(mIdleState);
                    break;
                case EVENT_SUBSCRIBE_PERIODIC:
                    if (mRetryCount > 0 && mRetryCount < 4) {
                        logd("stop scheduled Subscribe retry");
                        clearRetryStatus();
                        removeMessages(EVENT_SUBSCRIBE);
                        mSubscribeRequests.remove(0);
                    }
                    transitionTo(mIdleState);
                    periodicProcessing();
                   break;
                case EVENT_SUBSCRIBING:
                        trans = sendSubscribe(mSubscribeRequests.get(0).sipRequest);
                        if (trans != null) {
                            if (trans.getStatusCode() >= 200 && trans.getStatusCode() < 300) {
                                clearRetryStatus();
                                transitionTo(mNotifyState);
                                next = obtainMessage(EVENT_SUBSCRIBE_TIMEOUT);
                                //Subscription Expiration(Vzw provisioning) + T1: 3sec
                                int delayTime = (PstUtils.getCapabilityPollListSubExp(mSlotId, mContext) + 3) * 1000;
                                sendMessageDelayed(next, delayTime);
                                if (mSubscribeRequests.get(0).user
                                        == CAPABILITY_POLLING_USER_SYSTEM) {
                                    handleCapabilityPollingInterval();
                                }
                            } else {
                                next = obtainMessage(EVENT_SUBSCRIBE_ERROR);
                                next.obj = trans;
                                sendMessage(next);
                            }
                        }
                    break;
                case EVENT_SUBSCRIBE_ERROR:
                        // handle error code
                        trans = (SipTransactionContext)msg.obj;
                        logd(" status: " + trans.getStatusCode() + " reason: " + trans.getReasonPhrase());
                        switch(trans.getStatusCode()) {
                            case CAPABILITY_POLLING_403_FAIL:
                                mImsModule.onForbiddenReceived(ImsModule.EabActions.CAPABILITY, trans.getReasonPhrase());
                                break;
                            case CAPABILITY_POLLING_408_FAIL:
                            case CAPABILITY_POLLING_500_FAIL:
                            case CAPABILITY_POLLING_503_FAIL:
                            case CAPABILITY_POLLING_603_FAIL:
                                int retryTime = getNextRetryTime(mSubscribeRequests.get(0));
                                if (retryTime > 0) {
                                    VzWSipRequest vzwSipRequest = mSubscribeRequests.get(0);
                                    if (vzwSipRequest.contactList.size() > 1) {
                                        vzwSipRequest.sipRequest = buildListSubscribeSipRequest(vzwSipRequest.contactList);
                                    } else {
                                        vzwSipRequest.sipRequest = buildSingleSubscribeSipRequest(vzwSipRequest.contactList.get(0));
                                    }
                                    next = obtainMessage(EVENT_SUBSCRIBING);
                                    sendMessageDelayed(next, retryTime * 1000);
                                } else {
                                    logd("retry fail, remove the request from queue");
                                    clearRetryStatus();
                                    mSubscribeRequests.remove(0);
                                }
                                break;
                            case CAPABILITY_POLLING_423_FAIL:
                                handle423IntervalTooShort(trans);
                                break;
                            default:
                                // Periodic Processing finish, go to next run.
                                if (mSubscribeRequests.size() > 0) {
                                    if (mSubscribeRequests.get(0).user
                                        == CAPABILITY_POLLING_USER_SYSTEM) {
                                        handleCapabilityPollingInterval();
                                    }
                                    mSubscribeRequests.remove(0);
                                }
                                transitionTo(mIdleState);
                                next = obtainMessage(EVENT_SUBSCRIBE);
                                sendMessage(next);
                        }
                    break;
                case EVENT_PROVISION_VALUE_CHANGED:
                    handleCapabilityPollingInterval();
                    break;
                case EVENT_CAPABILITIES_CACHE_EXPIRED:
                    checkCacheExpired();
                    break;
                default:
                    retVal = NOT_HANDLED;
                    break;
            }
            return retVal;
        }
    }

    private void periodicProcessing() {
        logger.info("periodicProcessing");
        List<String> contactList = mContactsManager.getAllContacts();
        if (contactList.size() < 1) {
            logd("no contact exist  on device, stop capability polling");
            handleCapabilityPollingInterval();
            return;
        }
        putSubscription(contactList, CAPABILITY_POLLING_USER_SYSTEM);
     }

    private SipTransactionContext sendSubscribe(SipRequest request) {
        SipTransactionContext trans = null;

        try {
            trans = mImsModule.getSipManager().sendSipMessageAndWait(request);
            if (mRequestId != -1) {
                logger.info("Notify the SipResponse with requestId = " + String.valueOf(mRequestId));
                PresenceServiceImpl.receiveSipResponse(mRequestId, trans.getSipResponse());
            }
            // Analyze the received response
            if (trans.isSipResponse()) {
                // A response has been received
                logd("sendSubscribe success");
            }
        } catch (Exception e) {
            if (logger.isActivated()) {
                logger.error("sendSubscribe has failed", e);
            }
        }
        return trans;
    }

    private int getNextPollingTime() {
         int interval = PstUtils.getCapabilityPollInterval(mSlotId, mContext);
         double randon = Math.random()*0.2 + 0.9;
         Double result = new Double(interval * randon);
         logd("getNextPollingTime, interval: " + result.intValue());
         return result.intValue();
    }

    private int getNextRetryTime(VzWSipRequest request) {
        int nextTime = -1;

        switch(request.user) {
            case CAPABILITY_POLLING_USER_SYSTEM: {
                if (mRetryCount < 4) {
                    nextTime = 30 * 60 * ((int)Math.pow(2, mRetryCount));
                    mRetryCount++;
                }
                break;
            }
            case CAPABILITY_POLLING_USER_APPLICATION: {
                if (mRetryCount < 4) {
                    nextTime = 60 * ((int)Math.pow(2, mRetryCount));
                    mRetryCount++;
                }
                break;
            }
            default: {
                logd("Not support the user: " + request.user);
            }
        }
        logd("user: " + request.user + " next retry time: " + nextTime);

        return nextTime;
    }

    private void clearRetryStatus() {
        mRetryCount = 0;
    }

    private ProvisioningManager.Callback mProvisioningCallback =
            new ProvisioningManager.Callback() {
        @Override
        public void onProvisioningIntChanged(int item, int value) {
            if (item == ImsConfig.ConfigConstants.CAPABILITIES_POLL_INTERVAL) {
                logd("onProvisionedIntChanged, item: " + item + " value: " + value);
                Message msg = obtainMessage(EVENT_PROVISION_VALUE_CHANGED);
                msg.arg1 = value;
                sendMessage(msg);
            }
        }
    };

    private void handleCapabilityPollingInterval() {
        logd("handleCapabilityPollingInterval");
        int periodicTimer = getNextPollingTime() * 1000;
        removeMessages(EVENT_SUBSCRIBE_PERIODIC);
        Message msg = obtainMessage(EVENT_SUBSCRIBE_PERIODIC);
        sendMessageDelayed(msg, periodicTimer);
    }

    private void handle423IntervalTooShort(SipTransactionContext trans) {
        logd("handle423IntervalTooShort");
        SipResponse resp = trans.getSipResponse();
        // Extract the Min-Expire value
        int minExpire = SipUtils.getMinExpiresPeriod(resp);
        if (minExpire == -1) {
            logger.error("Can't read the Min-Expires value");
            return;
        }

        ImsManager imsManager = ImsManager.getInstance(mContext, 0);
        if (imsManager != null) {
            try {
                ImsConfig imsConfig = imsManager.getConfigInterface();
                if (imsConfig != null) {
                    imsConfig.setProvisionedValue(
                            ImsConfig.ConfigConstants.CAPAB_POLL_LIST_SUB_EXP, minExpire);
                    logd("CPLS expiration=: " + minExpire);
                    VzWSipRequest vzwSipRequest = mSubscribeRequests.get(0);
                    if (vzwSipRequest.contactList.size() > 1) {
                        vzwSipRequest.sipRequest = buildListSubscribeSipRequest(vzwSipRequest.contactList);
                    } else {
                        vzwSipRequest.sipRequest = buildSingleSubscribeSipRequest(vzwSipRequest.contactList.get(0));
                    }
                    transitionTo(mSubscribeState);
                    Message msg = obtainMessage(EVENT_SUBSCRIBING);
                    sendMessage(msg);
                }
            } catch (Exception ex) {
                logger.debug("setConfig fail, " + ex);
            }
        }
    }

    private void checkCacheExpired() {
        long cacheExpiration = PstUtils.getCapabilitiesCacheExpiration(mSlotId, mContext);
        List<String> contactList = mContactsManager.getAllContacts();
        for (String contact : contactList) {
            logd("verify contact: " + contact + "cache timeliness");
            Capabilities capabilities = mContactsManager.getContactCapabilities(contact);
            long delta = (System.currentTimeMillis() - capabilities.getTimestamp()) / 1000;
            logd("delta: " + delta + " cacheExpiration: " + cacheExpiration);
            if (delta > cacheExpiration) {
                logd("reset " + contact + " cache information");
                mContactsManager.setContactCapabilities(
                    contact, new Capabilities(), ContactInfo.NO_RESOURCE,
                    ContactInfo.REGISTRATION_STATUS_UNKNOWN);
            }
        }
        removeMessages(EVENT_CAPABILITIES_CACHE_EXPIRED);
        Message msg = obtainMessage(EVENT_CAPABILITIES_CACHE_EXPIRED);
        logd("delay time: " + cacheExpiration * 1000);
        sendMessageDelayed(msg, cacheExpiration * 1000);
    }

    private String msgToString(int msg) {
        switch(msg) {
            case EVENT_NOTIFY:
                return "EVENT_NOTIFY";
            case EVENT_SERVICE_START:
                return "EVENT_SERVICE_START";
            case EVENT_SERVICE_STOP:
                return "EVENT_SERVICE_STOP";
            case EVENT_SUBSCRIBE:
                return "EVENT_SUBSCRIBE";
            case EVENT_SUBSCRIBE_ERROR:
                return "EVENT_SUBSCRIBE_ERROR";
            case EVENT_SUBSCRIBE_PERIODIC:
                return "EVENT_SUBSCRIBE_PERIODIC";
            case EVENT_SUBSCRIBE_TIMEOUT:
                return "EVENT_SUBSCRIBE_TIMEOUT";
            case EVENT_SUBSCRIBING:
                return "EVENT_SUBSCRIBING";
            case EVENT_CAPABILITIES_CACHE_EXPIRED:
                return "EVENT_CAPABILITIES_CACHE_EXPIRED";
            case EVENT_PROVISION_VALUE_CHANGED:
                return "EVENT_PROVISION_VALUE_CHANGED";
            default:
                return "unknown request";
        }
    }

    public void logd(String s) {
        logger.debug(mCurrentState +": " + s);
    }
    public void logi(String s) {
        logger.info(mCurrentState +": " + s);
    }
    public void loge(String s) {
        logger.error(mCurrentState +": " + s);
    }

    public void setRequestId(int requestId) {
        mRequestId = requestId;
    }
}
