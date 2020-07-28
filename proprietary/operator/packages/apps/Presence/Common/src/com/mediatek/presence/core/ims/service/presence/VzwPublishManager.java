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
package com.mediatek.presence.core.ims.service.presence;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.telephony.TelephonyManager;
import android.os.Looper;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.Message;
import android.support.v4.content.LocalBroadcastManager;

import com.android.internal.util.State;
import com.android.internal.util.StateMachine;
import com.android.ims.ImsConfig;
import com.android.ims.ImsManager;

import com.mediatek.presence.core.ims.ImsModule;
import com.mediatek.presence.core.ims.network.sip.SipMessageFactory;
import com.mediatek.presence.core.ims.network.sip.SipUtils;
import com.mediatek.presence.core.ims.protocol.sip.SipDialogPath;
import com.mediatek.presence.core.ims.protocol.sip.SipException;
import com.mediatek.presence.core.ims.protocol.sip.SipRequest;
import com.mediatek.presence.core.ims.protocol.sip.SipResponse;
import com.mediatek.presence.core.ims.protocol.sip.SipTransactionContext;
import com.mediatek.presence.core.ims.rcsua.RcsUaAdapter;
import com.mediatek.presence.core.ims.rcsua.RcsUaAdapterManager;
import com.mediatek.presence.core.ims.service.capability.Capabilities;
import com.mediatek.presence.platform.AndroidFactory;
import com.mediatek.presence.provider.eab.ContactsManager;
import com.mediatek.presence.service.LauncherUtils;
import com.mediatek.presence.service.api.PresenceServiceImpl;
import com.mediatek.presence.utils.logger.Logger;
import com.mediatek.presence.utils.DateUtils;
import com.mediatek.presence.utils.PstUtils;
import com.mediatek.presence.utils.ImsConfigMonitorService;
import com.mediatek.presence.utils.ImsConfigMonitorService.FeatureValueListener;
import com.mediatek.presence.utils.PhoneStateMonitor;
import com.mediatek.presence.utils.PhoneStateMonitor.NetowrkStateListener;
import com.mediatek.presence.utils.PhoneStateMonitor.MobileNetowrkListener;
import com.mediatek.presence.provider.settings.RcsSettings;
import com.mediatek.presence.provider.settings.RcsSettingsManager;

import com.android.ims.internal.uce.presence.PresServiceInfo;
import com.android.ims.internal.uce.presence.PresPublishTriggerType;

import java.util.Vector;

import javax2.sip.header.ExpiresHeader;
import javax2.sip.header.SIPETagHeader;


/**
 * Publish manager for sending current user presence status on verizon network
 *
 */
public class VzwPublishManager  extends StateMachine
        implements NetowrkStateListener, MobileNetowrkListener,
                          FeatureValueListener,
                          RcsUaAdapter.ImsStatusListener,
                          RcsUaAdapter.VopsStateListener {

    private Logger logger = null;
    private Context mContext;
    private ImsModule mImsModule;
    private ContactsManager mContactsManager;
    private Capabilities mSelfCapabilities;

    private ImsConfigMonitorService mImsConfigMonitorService;
    private PhoneStateMonitor mPhoneStateMonitor;
    private int mCurrentNetwork;
    private int mVopsState = 1;

    private String entityTag = null;
    private String mNewFeatureTag = null;
    private PresServiceInfo mNewFeatureTagDetailInfo = null;

    private String mCurrentState = "DefaultState";
    private DefaultState mDefaultState = new DefaultState();
    private InacviceState mInacviceState = new InacviceState();
    private ActiveState mActiveState = new ActiveState();
    private ThrottleState mThrottleState = new ThrottleState();
    private RecoverState mRecoverState = new RecoverState();

    //COMMANDS
    private final int EVENT_PUBLISH_INITIAL = 2;
    private final int EVENT_PUBLISH_MODIFY = 3;
    private final int EVENT_PUBLISH_REFRESH = 4;
    private final int EVENT_PUBLISH_TERMINAL = 5;
    private final int EVENT_PUBLISH_THROTTLING_START = 6;
    private final int EVENT_PUBLISH_THROTTLING_STOP = 7;
    private final int EVENT_PUBLISH_ERROR = 8;

    // error cause
    private final int PUBLISH_403_FAIL = 403;
    private final int PUBLISH_404_FAIL = 404;
    private final int PUBLISH_408_FAIL = 408;
    private final int PUBLISH_412_FAIL = 412;
    private final int PUBLISH_413_FAIL = 413;
    private final int PUBLISH_423_FAIL = 423;
    private final int PUBLISH_500_FAIL = 500;
    private final int PUBLISH_503_FAIL = 503;
    private final int PUBLISH_603_FAIL = 603;
    private final int PUBLISH_674_FAIL = 674;

    // Retry handling
    private boolean mIsForbiddenRetry = false;
    private int mRetryExponentialCount = 0;
    private final int MAX_RETRY_COUNT = 4;

    // Throttle handling
    private boolean mHasPendingPublishRequest = false;

    /**
     * RequestId which shall be sent to client when receiveing SIP response (AOSP procedure)
     */
    private int mRequestId = -1;
    private int mSlotId = 0;

    public VzwPublishManager(String name, Handler mHandler, ImsModule imsModule) {
        super(name, mHandler);

        mSlotId = imsModule.getSlotId();
        mImsModule = imsModule;
        mContext = AndroidFactory.getApplicationContext();
        mContactsManager = ContactsManager.getInstance();
        PstUtils.createInstance(mContext);

        //mPhoneStateMonitor = new PhoneStateMonitor(mSlotId);
        mImsConfigMonitorService = ImsConfigMonitorService.getInstance(mContext);
        logger = Logger.getLogger(mSlotId, "VzwPublishManager");

        addState(mDefaultState);
        addState(mInacviceState, mDefaultState);
        addState(mActiveState, mDefaultState);
        addState(mThrottleState, mDefaultState);
        addState(mRecoverState, mDefaultState);
        setInitialState(mInacviceState);
        start();
    }

    public void startService() {
        logger.debug("startService");

        //mPhoneStateMonitor.addMobileNetworkListener(this);
        //mPhoneStateMonitor.addNetworkStateListener(this);
        mImsConfigMonitorService
            .registerFeatureValueListener(
            ImsConfig.FeatureConstants.FEATURE_TYPE_VIDEO_OVER_LTE, this);

        RcsUaAdapterManager.getRcsUaAdapter(mSlotId).registerImsStatusListener(this);
        RcsUaAdapterManager.getRcsUaAdapter(mSlotId).registerVopsStateListener(this);

        Message msg = obtainMessage(EVENT_PUBLISH_INITIAL);
        sendMessage(msg);
    }

    public void stopService() {
        logger.debug("stopService");

        if (mIsForbiddenRetry == true) {
            logger.debug("no need to send unPublish during forbidden retry");
        } else {
            if (mCurrentState.equalsIgnoreCase("AcviceState")
                ||mCurrentState.equalsIgnoreCase("ThrottleState")
                ||mCurrentState.equalsIgnoreCase("RecoverState")) {
                logger.debug("Send unPublish on " + mCurrentState);
                sendUnPublish(); //1: default value
            }
        }

        //mPhoneStateMonitor.removeMobileNetworkListener(this);
        //mPhoneStateMonitor.removeNetworkStateListener(this);
        mImsConfigMonitorService.unRegisterFeatureValueListener(this);

        RcsUaAdapterManager.getRcsUaAdapter(mSlotId).unRegisterImsStatusListener(this);

        // clear status
        clearRetryConfig();
    }

    public void onForbiddenReceived(ImsModule.EabActions action, String reason) {
        logd("onForbiddenReceived reason: " + reason);

        if (reason.indexOf("User Not Registered") > -1) {
            if (mIsForbiddenRetry == true) {
                logger.debug("already run once, so do nothing");
                return;
            }
            mIsForbiddenRetry = true;
            transitionTo(mInacviceState);
        } else {
            handleExponentialBackOff();
        }
    }

    public void onNotProvisionedReceived() {
        logd("onNotProvisionedReceived");
        transitionTo(mInacviceState);
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

    private class InacviceState extends State {
        @Override
        public void enter() {
            mCurrentState = "InacviceState";
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
            switch(msg.what) {
                case EVENT_PUBLISH_INITIAL:
                    sendInitialOrModifyPublish();
                    break;
                default:
                    retVal = NOT_HANDLED;
                    break;
            }
            return retVal;
        }
    }

    private class ActiveState extends State {
        @Override
        public void enter() {
            mCurrentState = "AcviceState";
            logd("enter");
            if (mHasPendingPublishRequest == true) {
                logd("resume pending modification request");
                mHasPendingPublishRequest = false;
                Message msg = obtainMessage(EVENT_PUBLISH_MODIFY);
                sendMessage(msg);
            }
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
            switch(msg.what) {
                case EVENT_PUBLISH_REFRESH:
                    //AOSP procedure
                    PresenceServiceImpl.publishTriggering(
                            PresPublishTriggerType.UCE_PRES_PUBLISH_TRIGGER_ETAG_EXPIRED);

                    try {
                        SipRequest refresh = buildPublishRequest(PstUtils.getPublishTimer(mSlotId, mContext), null);
                        SipTransactionContext refreshTrans;
                        refreshTrans = mImsModule.getSipManager().sendSipMessageAndWait(refresh);
                        if (refreshTrans.isSipResponse()) {
                            if(refreshTrans.getStatusCode() == 200) {
                                handle200OK(refreshTrans);
                            } else {
                                transitionTo(mRecoverState);
                                next = obtainMessage(EVENT_PUBLISH_ERROR);
                                next.obj = refreshTrans;
                                sendMessage(next);
                            }
                        }
                    }catch (Exception e) {
                       logger.error("send refresh publish has failed", e);
                   }
                    break;
                case EVENT_PUBLISH_MODIFY:
                    sendInitialOrModifyPublish();
                    break;
                case EVENT_PUBLISH_TERMINAL:
                    sendUnPublish();
                    break;
                default:
                    retVal = NOT_HANDLED;
                    break;
            }
            return retVal;

        }
    }

    private class ThrottleState extends State {
        @Override
        public void enter() {
            mCurrentState = "ThrottleState";
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
            switch(msg.what) {
                case EVENT_PUBLISH_THROTTLING_START:
                    next = obtainMessage(EVENT_PUBLISH_THROTTLING_STOP);
                    sendMessageDelayed(next, PstUtils.getPublishThrottle(mSlotId, mContext) * 1000);
                    break;
                case EVENT_PUBLISH_THROTTLING_STOP:
                    transitionTo(mActiveState);
                    break;
                case EVENT_PUBLISH_REFRESH:
                    removeMessages(EVENT_PUBLISH_THROTTLING_STOP);
                    deferMessage(msg);
                    transitionTo(mActiveState);
                    break;
                case EVENT_PUBLISH_MODIFY:
                    logd("Pending modification request at throttle status");
                    mHasPendingPublishRequest = true;
                break;
                case EVENT_PUBLISH_TERMINAL:
                    removeMessages(EVENT_PUBLISH_THROTTLING_STOP);
                    deferMessage(msg);
                    transitionTo(mActiveState);
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
            boolean retVal = HANDLED;
            logd("msg=" + msgToString(msg.what));
            if (!mImsModule.isProvisioned()) {
                logd("Device not be Provisioned");
                return retVal;
            }

            Message next;
            SipTransactionContext trans;
            switch(msg.what) {
                case EVENT_PUBLISH_ERROR:
                    trans = (SipTransactionContext)msg.obj;
                    logd("status: " + trans.getStatusCode() + " reason: " + trans.getReasonPhrase());
                    switch(trans.getStatusCode()) {
                        case PUBLISH_403_FAIL:
                            handle403Forbidden(trans.getReasonPhrase());
                            break;
                        case PUBLISH_404_FAIL:
                            handle404NotProvisioned();
                            break;
                        case PUBLISH_412_FAIL:
                            handle412ConditionalFailed();
                            break;
                        case PUBLISH_423_FAIL:
                            handle423IntervalTooShort(trans);
                            break;
                        case PUBLISH_408_FAIL:
                        case PUBLISH_500_FAIL:
                        case PUBLISH_503_FAIL:
                        case PUBLISH_603_FAIL:
                            handleExponentialBackOff();
                            break;
                        case PUBLISH_674_FAIL:
                            handle674NoResponse();
                            break;
                        default:
                            logd("failcase:"+ trans.getStatusCode() + ", do noting");
                            if (entityTag == null) {
                                logd("entityTag is null, back to InactiveState");
                                transitionTo(mInacviceState);
                            } else {
                                logd("entityTag: "+entityTag+" back to ActiveState");
                                transitionTo(mActiveState);
                            }
                            break;
                    }
                    break;
                case EVENT_PUBLISH_TERMINAL:
                    //clear selfCapabilities
                    entityTag = null;
                    mSelfCapabilities = null;
                    break;
                default:
                    retVal = NOT_HANDLED;
                    break;
            }
            return retVal;

        }
    }

    private String buildSelfCapabilities() {
        PresenceInfo info = new PresenceInfo();
        PresentityBuilder builder = new PresentityBuilder();
        if (mSelfCapabilities == null) {
            mSelfCapabilities = RcsSettingsManager.getRcsSettingsInstance(mSlotId)
                    .getMyCapabilities();
        }

        if(mSelfCapabilities.isPresenceDiscoverySupported()){
            builder.addRCSCapability(PresentityBuilder.RCS_TUPLE_ID_DISCOVERY_PRES, "open",
                    PresenceUtils.FEATURE_RCS2_DISCOVERY_VIA_PRESENCE, "1.0",
                    "Presence Discovery", ImsModule.IMS_USER_PROFILE.getPublicUri());
        }

        if(mSelfCapabilities.isIR94_VoiceCallSupported() && mVopsState == 1) {
            builder.addIMSCapability(PresentityBuilder.RCS_TUPLE_ID_VOLTE, "open", mSelfCapabilities.isIR94_VoiceCallSupported(),
                    PstUtils.getInstance().isLvcEnabled(mContext), mSelfCapabilities.isIR94_VoiceCallSupported(),
                    PresenceUtils.FEATURE_RCS2_IP_VOICE_CALL, "1.0", "IPVideoCall",
                    ImsModule.IMS_USER_PROFILE.getPublicUri());
        }

        //AOSP procedure
        if (mNewFeatureTag != null && mNewFeatureTagDetailInfo != null) {
            String status = null;
            boolean audioSupport = false;
            boolean videoSupport = false;
            switch (mNewFeatureTagDetailInfo.getMediaType()) {
                case PresServiceInfo.UCE_PRES_MEDIA_CAP_FULL_AUDIO_ONLY:
                    status = "open";
                    audioSupport = true;
                    videoSupport = false;
                    break;
                case PresServiceInfo.UCE_PRES_MEDIA_CAP_FULL_AUDIO_AND_VIDEO:
                    status = "open";
                    audioSupport = true;
                    videoSupport = true;
                    break;
                default:
                    status = "close";
                    audioSupport = false;
                    videoSupport = false;
                    break;
            }
            //TODO: Need to confirm the tagId
            String tupleId = "";
            builder.addMediaCapability(tupleId, status, audioSupport, videoSupport,
                    mNewFeatureTagDetailInfo.getServiceId(),
                    mNewFeatureTagDetailInfo.getServiceVer(),
                    mNewFeatureTagDetailInfo.getServiceDesc(),
                    ImsModule.IMS_USER_PROFILE.getPublicUri());
            mNewFeatureTag = null;
            mNewFeatureTagDetailInfo = null;
        }

        builder.addPersonInfo(info);
        String selfCapabilities = builder.build();
        return selfCapabilities;
    }

    private SipRequest buildPublishRequest(int expireTime, String selfCapabilities) {
        SipRequest publish = null;

        try{
            // Set Call-Id
            String callId = mImsModule.getSipManager().getSipStack().generateCallId();
            // Set target
            String target = ImsModule.IMS_USER_PROFILE.getPublicUri();
            // Set local party
            String localParty = ImsModule.IMS_USER_PROFILE.getPublicUri();
            // Set remote party
            String remoteParty = ImsModule.IMS_USER_PROFILE.getPublicUri();
            // Set the route path
            Vector<String> route = mImsModule.getSipManager().getSipStack().getServiceRoutePath();
            // Create a dialog path
            SipDialogPath dialog = new SipDialogPath(mImsModule.getSipManager().getSipStack(),
                    callId, 1, target, localParty, remoteParty, route);
            // Set the local SDP part in the dialog path
            dialog.setLocalContent(selfCapabilities);
            // Create PUBLISH request
            publish = SipMessageFactory.createPublish(mSlotId, dialog,
                 expireTime, entityTag, selfCapabilities);
        }catch (Exception e) {
           logger.error("buildPublishRequest has failed", e);
       }
          return publish;
    }

    private SipDialogPath buildSipDialogPath() {
        // Set Call-Id
        String callId = mImsModule.getSipManager().getSipStack().generateCallId();
        // Set target
        String target = ImsModule.IMS_USER_PROFILE.getPublicUri();
        // Set local party
        String localParty = ImsModule.IMS_USER_PROFILE.getPublicUri();
        // Set remote party
        String remoteParty = ImsModule.IMS_USER_PROFILE.getPublicUri();
        // Set the route path
        Vector<String> route = mImsModule.getSipManager().getSipStack().getServiceRoutePath();
        // Create a dialog path
        SipDialogPath dialog = new SipDialogPath(mImsModule.getSipManager().getSipStack(),
                callId, 1, target, localParty, remoteParty, route);
        return dialog;
    }

    public void sendInitialOrModifyPublish() {
         try {
            String capabilities = buildSelfCapabilities();
            SipRequest publish;
            if (PstUtils.getInstance().isLvcEnabled(mContext)) {
                publish = buildPublishRequest(PstUtils.getPublishTimer(mSlotId, mContext), capabilities);
            } else {
                publish = buildPublishRequest(PstUtils.getPublishTimerExtended(mSlotId, mContext), capabilities);
            }
            SipTransactionContext trans;
            trans = mImsModule.getSipManager().sendSipMessageAndWait(publish);
            if (mRequestId != -1) {
                logger.info("Notify the SipResponse with requestId = " + String.valueOf(mRequestId));
                PresenceServiceImpl.receiveSipResponse(mRequestId, trans.getSipResponse());
            }
            if (trans.isSipResponse()) {
                if(trans.getStatusCode() == 200) {
                    handle200OK(trans);
                } else {
                    transitionTo(mRecoverState);
                    Message next = obtainMessage(EVENT_PUBLISH_ERROR);
                    next.obj = trans;
                    sendMessage(next);
                }
            }
        }catch (Exception e) {
           logger.error("send initial publish has failed", e);
       }
    }

    private void sendUnPublish() {
        try {
            SipRequest terminal = buildPublishRequest(0, null);
            SipTransactionContext terminalTrans;
            terminalTrans = mImsModule.getSipManager().sendSipMessageAndWait(terminal, 10);
            if (terminalTrans.isSipResponse()) {
                Intent intent = new Intent(RcsUaAdapter.ACTION_IMS_DEREG_UNPUBLISH_DONE);
                intent.putExtra(RcsUaAdapter.EXTRA_SLOT_ID, mSlotId);
                LocalBroadcastManager.getInstance(mContext).sendBroadcast(intent);
                //clear publish status
                entityTag = null;
                clearRetryConfig();
                transitionTo(mInacviceState);
                PresenceServiceImpl.unPublishMessageSent();
            }
        }catch (Exception e) {
           logger.error("send un publish has failed", e);
       }
    }

    /**
    * Device shall republish no sooner than after 90% of the expiration duration and no later than 120s prior
    * to expiration (to allow for lost packets and retransmissions). In a case when the 10% of the expiration
    * duration is equal or less than 120s
    */
    private int getRefershSuggestionTime(int expire) {
        int result = 0;

        if ((expire * 0.1) < 120) {
            result = expire - 120;
        } else {
            result = (int)(expire * 0.9);
        }

        logd("suggestion time: " + result + " sec");
        return result;
    }

    public void onNetworkStateChanged(int networkType) {
        logger.debug("onNetworkStateChanged, old: " + mCurrentNetwork + " new: " + networkType);
        mCurrentNetwork = networkType;

        if (mSelfCapabilities == null) {
            logger.debug("mSelfCapabilities is null");
            return;
        }

        switch(networkType) {
            case TelephonyManager.NETWORK_TYPE_EHRPD:
                mSelfCapabilities.setIR94_VideoCall(false);
                break;
            case TelephonyManager.NETWORK_TYPE_LTE:
                mSelfCapabilities.setIR94_VideoCall(true);
                break;
            default:
                logger.debug("no need to handling on network: " + networkType);
        }

        if (mCurrentNetwork == TelephonyManager.NETWORK_TYPE_EHRPD
            || mCurrentNetwork == TelephonyManager.NETWORK_TYPE_LTE) {

            Message msg = obtainMessage(EVENT_PUBLISH_MODIFY);
            sendMessage(msg);
        }
    }

    public void onVopsStateChanged(int vops) {
        logger.debug("onVopsStateChanged, old vops: " + mVopsState + ", new vops: " + vops);
        if (mVopsState != vops) {
            mVopsState = vops;
            Message message = obtainMessage(EVENT_PUBLISH_MODIFY);
            sendMessage(message);
        }
    }

    public void onMobileNetworkChanged(int state) {
        logger.debug("onMobileNetworkChanged: state " + state);
        if (state == MobileNetowrkListener.MOBILE_NETWORK_ON) {
            mSelfCapabilities.setIR94_VideoCall(true);
            mSelfCapabilities.setImSessionSupport(true);
            mSelfCapabilities.setFileTransferSupport(true);
        } else {
            mSelfCapabilities.setIR94_VideoCall(false);
            mSelfCapabilities.setImSessionSupport(false);
            mSelfCapabilities.setFileTransferSupport(false);
        }
        Message message = obtainMessage(EVENT_PUBLISH_MODIFY);
        sendMessage(message);
    }

    public void onFeatureValueChanged(int slotId, int feature, int value) {
        logd("onFeatureValueChanged,slotId: " + slotId + " feature: " + feature + " value: " + value);

        if (slotId != mSlotId) {
            return;
        }

        if (mSelfCapabilities == null) {
            logd("mSelfCapabilities is null");
            return;
        }

        boolean config = (value==ImsConfig.FeatureValueConstants.ON)?true:false;
        if (mSelfCapabilities.isIR94_VideoCallSupported() == config) {
            logd("capabilities no changed, ignore the redundant notification");
            return;
        }
        mSelfCapabilities.setIR94_VideoCall(config);
        mSelfCapabilities.setIR94_DuplexMode(config);
        Message msg = obtainMessage(EVENT_PUBLISH_MODIFY);
        sendMessage(msg);
    }

   public void onImsDeregisterInd() {
        //clear pending status
        mHasPendingPublishRequest = false;
        Message msg = obtainMessage(EVENT_PUBLISH_TERMINAL);
        sendMessage(msg);
    }

    private void handle200OK(SipTransactionContext trans) {
        logd("200 OK response received");

        SipResponse resp = trans.getSipResponse();
        // Retrieve the expire value in the response
        ExpiresHeader expiresHeader = (ExpiresHeader)resp.getHeader(ExpiresHeader.NAME);
        if (expiresHeader != null) {
            int expires = expiresHeader.getExpires();
            if (expires != -1) {
                removeMessages(EVENT_PUBLISH_REFRESH);
                Message refresh = obtainMessage(EVENT_PUBLISH_REFRESH);
                int refreshDelay = getRefershSuggestionTime(expires) * 1000;
                sendMessageDelayed(refresh, refreshDelay);
            }
            updatePublishExpire(expires);
        }
        // Retrieve the entity tag in the response
        SIPETagHeader etagHeader = (SIPETagHeader)resp.getHeader(SIPETagHeader.NAME);
        if (etagHeader != null) {
            entityTag = etagHeader.getETag();
            logd("etag: " + entityTag);
        }

        //clear retry state
        clearRetryConfig();
        //clear error state
        clearErrorConfig();

        //translation life state
        transitionTo(mThrottleState);
        Message next = obtainMessage(EVENT_PUBLISH_THROTTLING_START);
        sendMessage(next);
    }

    private void handle403Forbidden(String reason) {
        logd("handle403Forbidden");
        mImsModule.onForbiddenReceived(ImsModule.EabActions.PUBLISH, reason);
    }

    private void handle404NotProvisioned() {
        logd("handle404NotProvisioned");
        mImsModule.onNotProvisionedReceived();
    }

    private void handle412ConditionalFailed() {
        logd("handle412ConditionalFailed");
        //clear etag
        entityTag = null;

        transitionTo(mInacviceState);
        Message msg = obtainMessage(EVENT_PUBLISH_INITIAL);
        sendMessage(msg);
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

        updatePublishExpire(minExpire);
        if (entityTag == null) {
            logd("entityTag is null, back to InactiveState");
            transitionTo(mInacviceState);
            Message msg = obtainMessage(EVENT_PUBLISH_INITIAL);
            sendMessage(msg);
        } else {
            logd("entityTag: "+entityTag+" back to ActiveState");
            transitionTo(mActiveState);
            Message msg = obtainMessage(EVENT_PUBLISH_MODIFY);
            sendMessage(msg);
        }
    }

    private void handle674NoResponse() {
        logd("handle674NoResponse");
        if (entityTag == null) {
            logd("handle initial publish timeout");
            handleExponentialBackOff();
        } else {
            int valid;
            if (PstUtils.getInstance().isLvcEnabled(mContext)) {
                valid = PstUtils.getPublishTimer(mSlotId, mContext);
            } else {
                valid = PstUtils.getPublishTimerExtended(mSlotId, mContext);
            }

            entityTag = null;
            transitionTo(mInacviceState);
            //calculate delay time
            //delay = vaild time - RefershSuggestionTime
            int delay = valid - getRefershSuggestionTime(valid) + 20;
            logd("valid:" + valid + " delay time: " + delay);
            Message msg = obtainMessage(EVENT_PUBLISH_INITIAL);
            sendMessageDelayed(msg, delay * 1000);
        }
    }

    private void handleExponentialBackOff() {
        logd("handlEexponentialBackOff");
        int backOffTime = getNextRetryTimer();
        if (entityTag == null) {
            logd("entityTag is null, back to InactiveState");
            transitionTo(mInacviceState);
            Message msg = obtainMessage(EVENT_PUBLISH_INITIAL);
            sendMessageDelayed(msg, backOffTime);
        } else {
            logd("entityTag: "+entityTag+" back to ActiveState");
            transitionTo(mActiveState);
            Message msg = obtainMessage(EVENT_PUBLISH_MODIFY);
            sendMessageDelayed(msg, backOffTime);
        }
    }

    private int getNextRetryTimer() {
        if (mRetryExponentialCount == MAX_RETRY_COUNT) {
            logd("more than the maximum number of retry times");
            logd("try to recover after: " + PstUtils.getPublishErrorRetryTimer(mSlotId, mContext) * 1000 + " ms");
            return PstUtils.getPublishErrorRetryTimer(mSlotId, mContext) * 1000;
        }
        int result = (int)Math.pow(2, mRetryExponentialCount) * 60 * 1000;
        mRetryExponentialCount = mRetryExponentialCount + 1;
        logd("count: " + mRetryExponentialCount + " retry time: " + result);
        return result;
    }

    private void updatePublishExpire(int expire) {
        ImsManager imsManager = ImsManager.getInstance(mContext, 0);
        if (imsManager != null) {
            try {
                ImsConfig imsConfig = imsManager.getConfigInterface();
                if (imsConfig != null) {
                    if (PstUtils.getInstance().isLvcEnabled(mContext)) {
                        imsConfig.setProvisionedValue(
                            ImsConfig.ConfigConstants.PUBLISH_TIMER, expire);
                        logd("PUBLISH_TIMER: " + expire);
                    } else {
                        imsConfig.setProvisionedValue(
                            ImsConfig.ConfigConstants.PUBLISH_TIMER_EXTENDED, expire);
                        logd("PUBLISH_TIMER_EXTENDED: " + expire);
                    }
                }
            } catch (Exception ex) {
                logger.debug("setConfig fail, " + ex);
            }
        }
    }


    private void clearRetryConfig() {
        logd("clearRetryConfig");
        mRetryExponentialCount = 0;
    }

    private void clearErrorConfig() {
        logd("clearErrorConfig");
        mIsForbiddenRetry = false;
    }

    private String msgToString(int msg) {
        switch(msg) {
            case EVENT_PUBLISH_INITIAL:
                return "EVENT_PUBLISH_INITIAL";
            case EVENT_PUBLISH_MODIFY:
                return "EVENT_PUBLISH_MODIFY";
            case EVENT_PUBLISH_REFRESH:
                return "EVENT_PUBLISH_REFRESH";
            case EVENT_PUBLISH_TERMINAL:
                return "EVENT_PUBLISH_TERMINAL";
            case EVENT_PUBLISH_THROTTLING_START:
                return "EVENT_PUBLISH_THROTTLING_START";
            case EVENT_PUBLISH_THROTTLING_STOP:
                return "EVENT_PUBLISH_THROTTLING_STOP";
            case EVENT_PUBLISH_ERROR:
                return "EVENT_PUBLISH_ERROR";
            default:
                return "unknown request";
        }
    }

    public void logd(String s) {
        logger.debug(mCurrentState +": " + s);
    }

    public void setNewFeatureTagInfo(String featureTag, PresServiceInfo serviceInfo) {
        mNewFeatureTag = featureTag;
        mNewFeatureTagDetailInfo = serviceInfo;
        Message msg = obtainMessage(EVENT_PUBLISH_MODIFY);
        sendMessage(msg);
    }

    public void setRequestId(int requestId) {
        mRequestId = requestId;
    }
}
