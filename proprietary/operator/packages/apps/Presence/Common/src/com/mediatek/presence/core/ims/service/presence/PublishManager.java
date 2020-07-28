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

import java.util.Vector;

import javax2.sip.header.ExpiresHeader;
import javax2.sip.header.RetryAfterHeader;
import javax2.sip.header.SIPETagHeader;

import com.mediatek.presence.core.ims.ImsModule;
import com.mediatek.presence.core.ims.network.sip.SipMessageFactory;
import com.mediatek.presence.core.ims.network.sip.SipUtils;
import com.mediatek.presence.core.ims.protocol.sip.SipDialogPath;
import com.mediatek.presence.core.ims.protocol.sip.SipRequest;
import com.mediatek.presence.core.ims.protocol.sip.SipResponse;
import com.mediatek.presence.core.ims.protocol.sip.SipTransactionContext;
import com.mediatek.presence.core.ims.service.SessionAuthenticationAgent;
import com.mediatek.presence.core.ims.rcsua.RcsUaAdapter;
import com.mediatek.presence.core.ims.rcsua.RcsUaAdapterManager;
import com.mediatek.presence.platform.AndroidFactory;
import com.mediatek.presence.platform.registry.RegistryFactory;
import com.mediatek.presence.provider.settings.RcsSettings;
import com.mediatek.presence.provider.settings.RcsSettingsManager;
import com.mediatek.presence.utils.PeriodicRefresher;
import com.mediatek.presence.utils.logger.Logger;
import com.mediatek.presence.utils.SimUtils;
import com.mediatek.presence.service.api.PresenceServiceImpl;
import com.android.ims.internal.uce.presence.PresPublishTriggerType;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.support.v4.content.LocalBroadcastManager;

/**
 * Publish manager for sending current user presence status
 *
 * @author JM. Auffret
 */
public class PublishManager extends PeriodicRefresher
            implements RcsUaAdapter.ImsStatusListener {
    /**
     * Last min expire period (in seconds)
     */
    private static final String REGISTRY_MIN_EXPIRE_PERIOD = "MinPublishExpirePeriod";

    /**
     * Last SIP Etag
     */
    protected static final String REGISTRY_SIP_ETAG = "SipEntityTag";

    /**
     * SIP Etag expiration (in milliseconds)
     */
    protected static final String REGISTRY_SIP_ETAG_EXPIRATION = "SipETagExpiration";

    final static public String ACTION_RESET_ETAG = "android.intent.presence.RESET_ETAG";

    private Context mContext = AndroidFactory.getApplicationContext();

    /**
     * IMS module
     */
    protected ImsModule imsModule;

    /**
     * Expire period
     */
    private int expirePeriod;

    /**
     * Dialog path
     */
    private SipDialogPath dialogPath = null;

    /**
     * Entity tag
     */
    private String entityTag = null;

    /**
     * Published flag
     */
    protected boolean published = false;

    /**
     * IMS dereg trigger unpublish flag
     */
    private boolean isImsDeregUnpublish = false;

    /**
     * Authentication agent
     */
    private SessionAuthenticationAgent authenticationAgent;

    /**
     * The logger
     */
    private Logger logger = null;

    /**
     * Retry timer for temprary publication failure (second)
     */
    private int mRetryTimer = 60;

    /**
     * Retry timer for temprary publication failure (second)
     */
    private int mRetryCounter = 0;

    /**
     * Keep the PIDF from the caller
     */
    private String mMessageBody = null;

    private PublishManagerReceiver mPublishManagerReceiver = new PublishManagerReceiver();

    private static final int INITIAL_AND_MODIFY_PUBLISH_TIMEOUT = 15;
    private static final int DELETE_PUBLISH_TIMEOUT = 3;

    private int mSlotId = 0;

    /**
     * RequestId which shall be sent to client when receiveing SIP response (AOSP procedure)
     */
    private int mRequestId = -1;

    /**
     * Constructor
     *
     * @param parent IMS module
     */
    public PublishManager(ImsModule parent) {
        super("Publish");
        mSlotId = parent.getSlotId();
        logger = Logger.getLogger(mSlotId, "PublishManager");
        if (SimUtils.isVzwSimCard(mSlotId)) {
            logger.debug("no need to initial at vzw");
            return;
        }
        this.imsModule = parent;
        this.authenticationAgent = new SessionAuthenticationAgent(imsModule);

        int defaultExpirePeriod = RcsSettingsManager.getRcsSettingsInstance(mSlotId)
                .getPublishExpirePeriod();
        int minExpireValue = RegistryFactory.getFactory().readInteger(REGISTRY_MIN_EXPIRE_PERIOD, -1);
        if ((minExpireValue != -1) && (defaultExpirePeriod < minExpireValue)) {
            this.expirePeriod = minExpireValue;
        } else {
            this.expirePeriod = defaultExpirePeriod;
        }
    }

    /**
     * Is published
     *
     * @return Return True if the terminal has published, else return False
     */
    public boolean isPublished() {
        return published;
    }

    /**
     * Start the timer
     *
     * @param expirePeriod Expiration period in seconds
     */
    @Override
    public void startTimer(int expirePeriod) {
        //for TMO-US
        startTimer(expirePeriod, 0.85);
        return;
    }

    public void init() {
        logger.debug("PublishManager init()");

        IntentFilter filter = new IntentFilter();
        filter.addAction(ACTION_RESET_ETAG);
        mPublishManagerReceiver.register(mContext, filter);
        RcsUaAdapterManager.getRcsUaAdapter(mSlotId).registerImsStatusListener(this);
    }

    /**
     * Terminate manager
     */
    public void terminate() {
        if (logger.isActivated()) {
            logger.info("Terminate the publish manager");
        }

        mPublishManagerReceiver.unregister(mContext);
        RcsUaAdapterManager.getRcsUaAdapter(mSlotId).unRegisterImsStatusListener(this);

        // Do not unpublish for RCS, just stop timer
        if (published) {

            unPublish();//????
            // Stop timer
            published = false;
        }

        // Stop timer and reset retry counter
        stopTimer();
        mRetryCounter = 0;

        if (logger.isActivated()) {
            logger.info("Publish manager is terminated");
        }
    }


    /**
     * rfc3903
    +-----------+-------+---------------+---------------+
    | Operation | Body? | SIP-If-Match? | Expires Value |
    +-----------+-------+---------------+---------------+
    | Initial   | yes   | no            | > 0           |
    | Refresh   | no    | yes           | > 0           |
    | Modify    | yes   | yes           | > 0           |
    | Remove    | no    | yes           | 0             |
    +-----------+-------+---------------+---------------+
    */

    /**
     * Publish refresh processing
     */
    @Override
    public void periodicProcessing() {
        // Make a publish
        if (logger.isActivated()) {
            logger.info("Execute re-publish");
        }

        //AOSP procedure
        PresenceServiceImpl.publishTriggering(
                PresPublishTriggerType.UCE_PRES_PUBLISH_TRIGGER_ETAG_EXPIRED);

        try {
            // Create a new dialog path for each publish
            dialogPath = createDialogPath();

            // Create PUBLISH request with no SDP and expire period
            SipRequest publish = SipMessageFactory.createPublish(mSlotId,
                                 dialogPath,
                                 expirePeriod,
                                 entityTag,
                                 null);

            // Send PUBLISH request
            sendPublish(publish);
        } catch (Exception e) {
            if (logger.isActivated()) {
                logger.error("Publish has failed", e);
            }
            handleError(new PresenceError(PresenceError.UNEXPECTED_EXCEPTION, e.getMessage()));
        }
    }

    /**
     * Publish presence status
     *
     * @param info Presence info
     * @return Boolean
     */
    public synchronized boolean publish(String info) {
        try {
            if (logger.isActivated()) {
                logger.debug("Publish");
            }

            // Create a new dialog path for each publish
            dialogPath = createDialogPath();

            // Set the local SDP part in the dialog path
            dialogPath.setLocalContent(info);
            mMessageBody = info;

            // Create PUBLISH request
            SipRequest publish = SipMessageFactory.createPublish(mSlotId,
                                 dialogPath,
                                 expirePeriod,
                                 entityTag,
                                 info);

            // Send PUBLISH request
            sendPublish(publish);
        } catch (Exception e) {
            if (logger.isActivated()) {
                logger.error("Publish has failed", e);
            }
            handleError(new PresenceError(PresenceError.UNEXPECTED_EXCEPTION, e.getMessage()));
        }
        return published;
    }

    /**
     * Unpublish
     */
    public synchronized void unPublish() {
        if (!published) {
            // Already unpublished
            return;
        }

        try {
            // Stop periodic publish
            stopTimer();

            // Create a new dialog path for each publish
            dialogPath = createDialogPath();

            // Create PUBLISH request with no SDP and expire period
            SipRequest publish = SipMessageFactory.createPublish(mSlotId,
                                 dialogPath,
                                 0,
                                 entityTag,
                                 null);

            // Send PUBLISH request
            sendPublish(publish);

            // Force publish flag to false
            published = false;
        } catch (Exception e) {
            if (logger.isActivated()) {
                logger.error("Publish has failed", e);
            }
            handleError(new PresenceError(PresenceError.UNEXPECTED_EXCEPTION, e.getMessage()));
        }
    }

    /**
     * Send PUBLISH message
     *
     * @param publish SIP PUBLISH
     * @throws Exception
     */
    private void sendPublish(SipRequest publish) throws Exception {
        if (logger.isActivated()) {
            logger.info("Send PUBLISH, expire=" + publish.getExpires());
        }

        if (published) {
            // Set the Authorization header
            authenticationAgent.setProxyAuthorizationHeader(publish);
        }
        // Set the security header
        if (authenticationAgent != null) {
            authenticationAgent.setAuthorizationHeader(publish);
        }
        /*@*/

        // Send PUBLISH request
        SipTransactionContext ctx;
        if (publish.getExpires() == 0) {
            // Set timeout to 3s for unpublish
            ctx = imsModule.getSipManager().sendSipMessageAndWait(
                    publish, DELETE_PUBLISH_TIMEOUT);
        } else {
            ctx = imsModule.getSipManager().sendSipMessageAndWait(
                    publish, INITIAL_AND_MODIFY_PUBLISH_TIMEOUT);
        }

        if (mRequestId != -1) {
            logger.info("Notify the SipResponse with requestId = " + String.valueOf(mRequestId));
            PresenceServiceImpl.receiveSipResponse(mRequestId, ctx.getSipResponse());
        }

        // Analyze the received response
        if (ctx.isSipResponse()) {
            // A response has been received
            if (ctx.getStatusCode() == 200) {
                // 200 OK
                if (publish.getExpires() != 0) {
                    handle200OK(ctx);
                } else {
                    handle200OkUnpublish(ctx);
                }
            } else if (publish.getExpires() == 0) {
                // Un-Publish failed, no retry
                handleError(new PresenceError(PresenceError.PUBLISH_FAILED,
                                              ctx.getStatusCode() + " " + ctx.getReasonPhrase()));

            } else {
                // Initial/Refresh/Modify Publish failed, error recovery
                switch (ctx.getStatusCode()) {
                case 400:
                case 415:
                    handle400BadRequest(ctx, publish);
                    break;

                case 407:
                    // 407 Proxy Authentication Required
                    handle407Authentication(ctx);
                    break;

                case 412:
                    // 412 Error
                    handle412ConditionalRequestFailed(ctx);
                    break;

                case 423:
                    // 423 Interval Too Brief
                    handle423IntervalTooBrief(ctx);
                    break;

                case 489:
                    handle489BadEvent(ctx);
                    break;

                case 503:
                    handle503SeviceUnavailable(ctx);
                    break;

                case 674:
                    // 674 CNTIS-Timer F Expiry
                    handle674CntisTimerFExpiry(ctx);
                    break;

                default:
                    // Other error response
                    handleError(new PresenceError(PresenceError.PUBLISH_FAILED,
                                                  ctx.getStatusCode() + " " + ctx.getReasonPhrase()));
                    break;
                }
            }
        } else {
            if (logger.isActivated()) {
                logger.debug("No response received for PUBLISH");
            }

            // No response received: timeout
            handleError(new PresenceError(PresenceError.PUBLISH_FAILED));
        }
    }

    public void onImsDeregisterInd() {
         if (logger.isActivated()) {
            logger.debug("onImsDeregisterInd, published:" + published);
        }

        if (published) {
            isImsDeregUnpublish = true;

            Thread thread = new Thread() {
                @Override
                public void run() {
                    unPublish();
                }
            };
            thread.start();
        } else {
            Intent sendIntent = new Intent(RcsUaAdapter.ACTION_IMS_DEREG_UNPUBLISH_DONE);
            sendIntent.putExtra(RcsUaAdapter.EXTRA_SLOT_ID, mSlotId);

            LocalBroadcastManager.getInstance(mContext).sendBroadcast(sendIntent);
        }
    }

    /**
     * Handle 200 0K response
     *
     * @param ctx SIP transaction context
     */
    protected void handle200OK(SipTransactionContext ctx) {
        // 200 OK response received
        if (logger.isActivated()) {
            logger.info("200 OK response received");
        }
        published = true;

        SipResponse resp = ctx.getSipResponse();

        // Set the Proxy-Authorization header
        authenticationAgent.readProxyAuthenticateHeader(resp);

        // Retrieve the expire value in the response
        retrieveExpirePeriod(resp);

        // Retrieve the entity tag in the response
        saveEntityTag((SIPETagHeader)resp.getHeader(SIPETagHeader.NAME));

        // Start the periodic publish
        startTimer(expirePeriod);

        // Publication success, reset retry counter
        mRetryCounter = 0;
    }

    /**
     * Handle 200 0K response of UNPUBLISH
     *
     * @param ctx SIP transaction context
     */
    protected void handle200OkUnpublish(SipTransactionContext ctx) {
        // 200 OK response received
        if (logger.isActivated()) {
            logger.info("200 OK response received");
            logger.debug("IMS dereg unpublish done, isImsDeregUnpublish: " + isImsDeregUnpublish);
        }

        SipResponse resp = ctx.getSipResponse();

        // Retrieve the entity tag in the response
        //saveEntityTag((SIPETagHeader)resp.getHeader(SIPETagHeader.NAME));
        saveEntityTag(null);

        if (isImsDeregUnpublish == true) {
            Intent intent = new Intent(RcsUaAdapter.ACTION_IMS_DEREG_UNPUBLISH_DONE);
            isImsDeregUnpublish = false;

            LocalBroadcastManager.getInstance(mContext).sendBroadcast(intent);
            PresenceServiceImpl.unPublishMessageSent();
        }
    }

    private void handle400BadRequest(SipTransactionContext ctx,
                                     SipRequest publishOld) throws Exception {
        String messageBody = mMessageBody;

        // Increment the Cseq number of the dialog path
        dialogPath.incrementCseq();

        // Implement as TMO requirement spec
        // Initial PUBLISH
        if (isPublished() == false) {
            // Reset Sip-Etag
            saveEntityTag(null);
        } else {
            // Refresh PUBLISH
            if (publishOld.getStackMessage().getContentLength().getContentLength() == 0) {
                messageBody = null;
            }
        }

        SipRequest publish = SipMessageFactory.createPublish(mSlotId,
                             dialogPath,
                             expirePeriod,
                             entityTag,
                             messageBody);

        // Send PUBLISH request
        //M: to avoid StackOverflowError, use retry timer and counter
        //Maxinum retry 5 times, 60 sec
        // sendPublish(publish);
        if (mRetryCounter < 5) {
            logger.info("400 response received, start retry in 60 sec");
            startTimer(60, 1);
            mRetryCounter++;
        } else {
            logger.info("400 response received, don't retry");
        }
    }

    /**
     * Handle 407 response
     *
     * @param ctx SIP transaction context
     * @throws Exception
     */
    private void handle407Authentication(SipTransactionContext ctx) throws Exception {
        // 407 response received
        if (logger.isActivated()) {
            logger.info("407 response received");
        }

        SipResponse resp = ctx.getSipResponse();

        // Set the Proxy-Authorization header
        authenticationAgent.readProxyAuthenticateHeader(resp);

        // Increment the Cseq number of the dialog path
        dialogPath.incrementCseq();

        // Create a second PUBLISH request with the right token
        if (logger.isActivated()) {
            logger.info("Send second PUBLISH");
        }
        SipRequest publish = SipMessageFactory.createPublish(mSlotId,
                             dialogPath,
                             ctx.getTransaction().getRequest().getExpires().getExpires(),
                             entityTag,
                             dialogPath.getLocalContent());

        // Set the Authorization header
        authenticationAgent.setProxyAuthorizationHeader(publish);

        // Send PUBLISH request
        sendPublish(publish);
    }

    /**
     * Handle 412 response
     *
     * @param ctx SIP transaction context
     */
    private void handle412ConditionalRequestFailed(SipTransactionContext ctx) throws Exception {
        // 412 response received
        if (logger.isActivated()) {
            logger.info("412 conditional response received");
        }

        // Increment the Cseq number of the dialog path
        dialogPath.incrementCseq();

        // Reset Sip-Etag
        saveEntityTag(null);

        // Create a PUBLISH request without ETag
        SipRequest publish = SipMessageFactory.createPublish(mSlotId,
                             dialogPath,
                             expirePeriod,
                             entityTag,
                             mMessageBody);

        // Send PUBLISH request
        sendPublish(publish);
    }

    /**
     * Handle 423 response
     *
     * @param ctx SIP transaction context
     * @throws Exception
     */
    private void handle423IntervalTooBrief(SipTransactionContext ctx) throws Exception {
        // 423 response received
        if (logger.isActivated()) {
            logger.info("423 interval too brief response received");
        }

        SipResponse resp = ctx.getSipResponse();

        // Increment the Cseq number of the dialog path
        dialogPath.incrementCseq();

        // Extract the Min-Expire value
        int minExpire = SipUtils.getMinExpiresPeriod(resp);
        if (minExpire == -1) {
            if (logger.isActivated()) {
                logger.error("Can't read the Min-Expires value");
            }
            handleError(new PresenceError(PresenceError.PUBLISH_FAILED, "No Min-Expires value found"));
            return;
        }

        // Save the min expire value in the terminal registry
        RegistryFactory.getFactory().writeInteger(REGISTRY_MIN_EXPIRE_PERIOD, minExpire);

        // Set the default expire value
        expirePeriod = minExpire;

        // Create a new PUBLISH request with the right expire period
        SipRequest publish = SipMessageFactory.createPublish(mSlotId,
                             dialogPath,
                             expirePeriod,
                             entityTag,
                             dialogPath.getLocalContent());

        // Send a PUBLISH request
        sendPublish(publish);
    }

    protected void handle489BadEvent(SipTransactionContext ctx) throws Exception {
        // TMO 489 handling
        // 489 Bad Event received
        if (logger.isActivated()) {
            logger.info("400 Bad Reqeust response received");
        }

        // Increment the Cseq number of the dialog path
        dialogPath.incrementCseq();

        SipRequest publish = SipMessageFactory.createPublish(mSlotId,
                             dialogPath,
                             expirePeriod,
                             entityTag,
                             dialogPath.getLocalContent());

        // Send PUBLISH request
        sendPublish(publish);
    }

    /**
     * Handle 503 response
     *
     * @param ctx SIP transaction context
     * @throws Exception
     */
    private void handle503SeviceUnavailable(SipTransactionContext ctx) throws Exception {
        // 503 response received
        if (logger.isActivated()) {
            logger.info("503 service unavailable");
        }

        SipResponse resp = ctx.getSipResponse();

        RetryAfterHeader retryHeader = (RetryAfterHeader)resp.getHeader(RetryAfterHeader.NAME);

        if(retryHeader!=null) {
            int retryDuration = retryHeader.getDuration();
            //start timer to send publish in time : retry duration
            startTimer(retryDuration, retryDuration);
        } else {
            //default retry
            startTimer(expirePeriod, 0.5);
        }
    }

    /**
     * Handle 674 response
     *
     * @param ctx SIP transaction context
     * @throws Exception
     */
    private void handle674CntisTimerFExpiry(SipTransactionContext ctx) throws Exception {
        // 674 CNTIS-Timer F Expiry
        if (logger.isActivated()) {
            logger.info("mRetryCounter: " + mRetryCounter);
        }

        SipResponse resp = ctx.getSipResponse();

        /* Retry timer
         *  1st: 1 min
         *  2nd: 2 min
         *  3nd: 4 min
         *  4th: 6 min
         *  ...
         */
        if (mRetryCounter > 0) {
            int retryTimer =  mRetryCounter * 2 * 60;

            if (retryTimer > expirePeriod) {
                mRetryTimer = expirePeriod;
            } else {
                mRetryTimer = retryTimer;
            }
        } else {
            mRetryTimer = 60;
        }

        startTimer(mRetryTimer, 1);
        mRetryCounter++;
    }
    /**
     * Handle error response
     *
     * @param error Error
     */
    private void handleError(PresenceError error) {
        // Error
        if (logger.isActivated()) {
            logger.info("Publish has failed: " + error.getErrorCode() + ", reason=" + error.getMessage());
        }
        published = false;

        // Publish has failed, stop the periodic publish
        stopTimer();

        // Error
        if (logger.isActivated()) {
            logger.info("Publish has failed");
        }
    }

    /**
     * Retrieve the expire period
     *
     * @param response SIP response
     */
    private void retrieveExpirePeriod(SipResponse response) {
        // Extract expire value from Expires header
        ExpiresHeader expiresHeader = (ExpiresHeader)response.getHeader(ExpiresHeader.NAME);
        if (expiresHeader != null) {
            int expires = expiresHeader.getExpires();
            if (expires != -1) {
                expirePeriod = expires;
            }
        }
    }

    /**
     * Save the SIP entity tag
     *
     * @param etagHeader Header tag
     */
    protected void saveEntityTag(SIPETagHeader etagHeader) {
        if (etagHeader == null) {
            entityTag = null;
        } else {
            entityTag = etagHeader.getETag();
        }
        if (entityTag != null) {
            RegistryFactory.getFactory().writeString(REGISTRY_SIP_ETAG, entityTag);
            long etagExpiration = System.currentTimeMillis() + (expirePeriod * 1000);
            RegistryFactory.getFactory().writeLong(REGISTRY_SIP_ETAG_EXPIRATION, etagExpiration);
            if (logger.isActivated()) {
                logger.debug("New entity tag: " + entityTag + ", expire at=" + etagExpiration);
            }
        } else {
            RegistryFactory.getFactory().removeParameter(REGISTRY_SIP_ETAG);
            RegistryFactory.getFactory().removeParameter(REGISTRY_SIP_ETAG_EXPIRATION);
            if (logger.isActivated()) {
                logger.debug("Entity tag has been reset");
            }
        }
    }

    /**
     * Create a new dialog path
     *
     * @return Dialog path
     */
    protected SipDialogPath createDialogPath() {
        // Set Call-Id
        String callId = imsModule.getSipManager().getSipStack().generateCallId();

        if (logger.isActivated()) {
            logger.debug("imsModule.IMS_USER_PROFILE : " +imsModule.IMS_USER_PROFILE.toString());
        }

        // Set target
        String target = ImsModule.IMS_USER_PROFILE.getPublicUri();

        // Set local party
        String localParty = ImsModule.IMS_USER_PROFILE.getPublicUri();

        // Set remote party
        String remoteParty = ImsModule.IMS_USER_PROFILE.getPublicUri();

        // Set the route path
        Vector<String> route = imsModule.getSipManager().getSipStack().getServiceRoutePath();

        // Create a dialog path
        SipDialogPath dialog = new SipDialogPath(
            imsModule.getSipManager().getSipStack(),
            callId,
            1,
            target,
            localParty,
            remoteParty,
            route);
        return dialog;
    }

    private class PublishManagerReceiver extends BroadcastReceiver {
        private boolean isRegistered = false;

        /**
         * register receiver
         * @param context - Context
         * @param filter - Intent Filter
         * @return see Context.registerReceiver(BroadcastReceiver,IntentFilter)
         */
        public void register(Context context, IntentFilter filter) {
            if (!isRegistered) {
                logger.debug("PublishManagerReceiver register");
                context.registerReceiver(this, filter);
                isRegistered = true;
            }
        }

        /**
         * unregister received
         * @param context - context
         * @return true if was registered else false
         */
        public boolean unregister(Context context) {
            logger.debug("PublishManagerReceiver unregister:" + isRegistered);
            return isRegistered
                   && unregisterInternal(context);
        }

        private boolean unregisterInternal(Context context) {
            context.unregisterReceiver(this);
            isRegistered = false;
            return true;
        }

        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            int slotId = intent.getIntExtra("slotId", 0);
            logger.debug("onReceive, intent action is " + action + " and slotId is " + slotId);
            if (action.equalsIgnoreCase(ACTION_RESET_ETAG)
                    && slotId == mSlotId) {
                if (logger.isActivated()) {
                    logger.debug("Receive RESET ETAG intent, reset ETAG");
                }
                saveEntityTag(null);
            }
        }
    }

    public void setRequestId(int requestId) {
        mRequestId = requestId;
    }
}
