package com.orangelabs.rcs.core.ims.service;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.ListIterator;
import java.util.Vector;

import javax2.sip.header.ContactHeader;
import javax2.sip.header.ExpiresHeader;
import javax2.sip.header.Header;
import javax2.sip.header.SubscriptionStateHeader;

import com.orangelabs.rcs.core.Core;
import com.orangelabs.rcs.core.ims.ImsModule;
import com.orangelabs.rcs.core.ims.network.sip.SipMessageFactory;
import com.orangelabs.rcs.core.ims.network.sip.SipUtils;
import com.orangelabs.rcs.core.ims.protocol.sip.SipDialogPath;
import com.orangelabs.rcs.core.ims.protocol.sip.SipRequest;
import com.orangelabs.rcs.core.ims.protocol.sip.SipResponse;
import com.orangelabs.rcs.core.ims.protocol.sip.SipTransactionContext;
import com.orangelabs.rcs.core.ims.service.im.InstantMessagingService;
import com.orangelabs.rcs.core.ims.service.im.chat.ChatError;
import com.orangelabs.rcs.platform.registry.RegistryFactory;
import com.orangelabs.rcs.provider.settings.RcsSettings;
import com.orangelabs.rcs.utils.PeriodicRefresher;
import com.orangelabs.rcs.utils.logger.Logger;

public class SubscriptionManager {

    /**
     * Constructor
     *
     * @param imsModule
     */
    public SubscriptionManager() {
    }

    /**
     * Get single instance
     *
     * @return SubscriptionManager
     */
    public static SubscriptionManager getInstance() {
        if (mInstance == null)
            mInstance = new SubscriptionManager();

        return mInstance;
    }

    /**
     * Start subscribe request
     *
     * @param request Subscribe request
     * @param callback Event notify callback
     */
    public void startSubscribe(SubscribeRequest request, EventCallback callback) {
        if (logger.isActivated())
            logger.info("startSubscribe");
        Subscription subRequest = new Subscription(this, request, callback, false);

        subscribe(subRequest);
    }

    /**
     * Poll status once
     *
     * @param request Subscribe request
     * @param callback Event notify callback
     */
    public void pollStatus(SubscribeRequest request, EventCallback callback) {
        if (logger.isActivated())
            logger.info("pollStatus");
        Subscription subRequest = new Subscription(this, request, callback, true);

        subscribe(subRequest);
    }

    /**
     * Stop subscription
     *
     * @param identity Identity of subscription to be stopped
     */
    public void stopSubscribe(EventCallback callback) {
        if (logger.isActivated())
            logger.info("stopSubscribe");

        synchronized (processingRequests) {
            for (int i = 0; i < processingRequests.size(); i++) {
                Subscription subscription = processingRequests.get(i);
                if (callback == subscription.mCallback) {
                    subscription.cancelRequest();
                    return;
                }
            }
        }

        synchronized (subscriptions) {
            for (Subscription subscription:subscriptions.values()) {
                if (callback == subscription.mCallback) {
                    subscription.cancelRequest();
                    return;
                }
            }
        }
    }

    /**
     * Terminate manager
     */
    public void terminate() {
        if (logger.isActivated())
            logger.info("terminate");

        synchronized (processingRequests) {
            for (int i = 0; i < processingRequests.size(); i++) {
                Subscription subscription = processingRequests.get(i);
                subscription.cancelRequest();
            }
        }

        synchronized (subscriptions) {
            for (Subscription s:subscriptions.values()) {
                s.cancelRequest();
            }
        }
        mInstance = null;
    }

    public Subscription getSubscription(EventCallback callback) {
        if (callback == null)
            return null;

        synchronized (processingRequests) {
            for (int i = 0; i < processingRequests.size(); i++) {
                Subscription subscription = processingRequests.get(i);
                if (callback == subscription.mCallback) {
                    return subscription;
                }
            }
        }
        synchronized (subscriptions) {
            for (Subscription subscription:subscriptions.values()) {
                if (callback == subscription.mCallback) {
                    return subscription;
                }
            }
        }
        
        return null;
    }

    /**
     * Create a SUBSCRIBE request
     *
     * @param dialog SIP dialog path
     * @return SIP request
     * @throws Exception
     */
    public boolean receiveNotification(SipRequest notify) {

        SubscriptionStateHeader stateHeader = (SubscriptionStateHeader)notify.getHeader(
                SubscriptionStateHeader.NAME);

        if (stateHeader != null) {
            String state = stateHeader.getState();

            synchronized (processingRequests) {
                for (int i = 0; i < processingRequests.size(); i++) {

                    Subscription subscription = processingRequests.get(i);
                    String identity = subscription.getIdentity();

                    if (notify.getCallId().equals(identity)) {
                        subscription.receiveNotification(notify);
                        if (state.equalsIgnoreCase(SubscriptionStateHeader.ACTIVE)) {
                            synchronized (subscriptions) {
                                subscriptions.put(identity, subscription);
                            }
                            subscription.mCallback.onActive(identity);
                        } else
                        if (state.equalsIgnoreCase(SubscriptionStateHeader.PENDING)) {
                            synchronized (subscriptions) {
                                subscriptions.put(identity, subscription);
                            }
                            subscription.mCallback.onPending(identity);
                        } else
                        if (state.equalsIgnoreCase(SubscriptionStateHeader.TERMINATED)) {
                            subscription.mCallback.onTerminated(stateHeader.getReasonCode(), stateHeader.getRetryAfter());
                        }
                        processingRequests.remove(i);
                        return true;
                    }
                }
            }
            synchronized (subscriptions) {
                Subscription subscription = subscriptions.get(notify.getCallId());
                if (subscription != null) {
                    subscription.receiveNotification(notify);

                    if (state.equalsIgnoreCase(SubscriptionStateHeader.TERMINATED)) {
                        subscription.mCallback.onTerminated(stateHeader.getReasonCode(), stateHeader.getRetryAfter());
                        subscriptions.remove(notify.getCallId());
                    }

                    return true;
                }
            }
        }

        return false;
    }

    private void subscribe(final Subscription subscription) {
        synchronized (processingRequests) {
            processingRequests.add(subscription);
        }

        Thread thread = new Thread() {
            @Override
            public void run() {
                subscription.subscribe();
            }
        };
        thread.start();
    }

    /**
     * Create a SUBSCRIBE request
     *
     * @param dialog SIP dialog path
     * @return SIP request
     * @throws Exception
     */
    private void removeSubscription(Subscription subscription) {

        synchronized (processingRequests) {
            for (int i = 0; i < processingRequests.size(); i++) {
                if (processingRequests.get(i) == subscription) {
                    processingRequests.remove(i);
                    return;
                }
            }
        }
        synchronized (subscriptions) {
            subscriptions.remove(subscription.getIdentity());
        }
    }

    /**
     * Create a SUBSCRIBE request
     *
     * @param dialog SIP dialog path
     * @return SIP request
     * @throws Exception
     */
    public static class EventCallback {
        /**
         * Called when event Notify recieved
         *
         * @param content Notify body
         */
        protected void handleEventNotify(byte[] content) {}

        /**
         * Called when subscription become Active
         *
         * @param dialog SIP dialog path
         * @return SIP request
         * @throws Exception
         */
        protected void onActive(String identity) {}

        /**
         * Called when subscription enter pending state
         *
         * @param dialog SIP dialog path
         * @return SIP request
         * @throws Exception
         */
        protected void onPending(String identity) {}

        /**
         * Called when subscription terminated
         *
         * @param dialog SIP dialog path
         * @return SIP request
         * @throws Exception
         */
        protected void onTerminated(String reason, int retryAfter) {}
    }

    private static class Subscription extends PeriodicRefresher {

        /**
         * Constructor
         *
         * @param manager Subscription manager
         * @param request Subscribe request
         * @param callback event Notify callback
         * @param pollOnce Subscribe or poll resource status
         */
        public Subscription(
                SubscriptionManager manager,
                SubscribeRequest request,
                EventCallback callback,
                boolean pollOnce) {
            this.mManager = manager;
            this.mRequest = request;
            this.mCallback = callback;

            ImsModule imsModule = Core.getInstance().getImsModule();
            this.authenticationAgent = new SessionAuthenticationAgent(imsModule);

            if (pollOnce || request.getExpirePeriod() == 0) {
                this.mExpirePeriod = 0;
            } else {
                int defaultExpire;
                int minExpire = RegistryFactory.getFactory().readInteger(MIN_EXPIRE_PERIOD, -1);

                if (request.getExpirePeriod() > 0)
                    defaultExpire = request.getExpirePeriod();
                else
                    defaultExpire = RcsSettings.getInstance().getSubscribeExpirePeriod();
                if ((minExpire != -1) && (defaultExpire < minExpire))
                    this.mExpirePeriod = minExpire;
                else
                    this.mExpirePeriod = defaultExpire;
            }

            this.mIdentity = imsModule.getSipManager().getSipStack().generateCallId();
        }

        /**
         * Create a SUBSCRIBE request
         *
         * @param dialog SIP dialog path
         * @return SIP request
         * @throws Exception
         */
        public void receiveNotification(SipRequest notify) {
            if (mState == STATE_CANCEL) {
                handleSubscriptionTerminated("canceled");
                return;
            }

            SubscriptionStateHeader stateHeader = (SubscriptionStateHeader)notify.getHeader(
                    SubscriptionStateHeader.NAME);

            mCallback.handleEventNotify(notify.getContentBytes());

            int expire = stateHeader.getExpires();
            if (expire > 0 && expire < mExpirePeriod) {
                stopTimer();
                mExpirePeriod = expire;

                if (mExpirePeriod <= 1200 ) {
                    startTimer(mExpirePeriod, 0.5);
                } else {
                    startTimer(mExpirePeriod-600);
                }
            }

            String state = stateHeader.getState();

            switch (mState) {
            case STATE_WAIT:
            case STATE_REFRESH:
                if (state.equalsIgnoreCase(SubscriptionStateHeader.ACTIVE)) {
                    mState = STATE_ACTIVE;
                } else
                if (state.equalsIgnoreCase(SubscriptionStateHeader.PENDING)) {
                    mState = STATE_PENDING;
                }
                break;

            case STATE_PENDING:
                if (state.equalsIgnoreCase(SubscriptionStateHeader.ACTIVE)) {
                    mState = STATE_ACTIVE;
                }
                break;
            }
        }

        /**
         * Create a SUBSCRIBE request
         *
         * @param dialog SIP dialog path
         * @return SIP request
         * @throws Exception
         */
        public String getIdentity() {
            return this.mIdentity;
        }

        /**
         * Periodic processing
         */
        public void periodicProcessing() {
            switch (mState) {
            case STATE_PENDING:
            case STATE_ACTIVE:
                mState = STATE_REFRESH;
                subscribe();
                break;
            }
        }

        /**
         * Create a SUBSCRIBE request
         *
         * @param dialog SIP dialog path
         * @return SIP request
         * @throws Exception
         */
        private void subscribe() {
            if (logger.isActivated()) {
                logger.info("Subscribe to " + mRequest.getRequestUri());
            }
            ImsModule imsModule = Core.getInstance().getImsModule();

            // Create a dialog path if necessary
            if (dialogPath == null) {
                // Set Call-Id
                String callId = mIdentity;

                // Set target
                String target = mRequest.getRequestUri();

                // Set local party
                String localParty = ImsModule.IMS_USER_PROFILE.getPublicUri();

                // Set remote party
                String remoteParty = mRequest.getRequestUri();

                // Set the route path
                Vector<String> route = imsModule.getSipManager().getSipStack().getServiceRoutePath();

                // Create a dialog path
                dialogPath = new SipDialogPath(
                        imsModule.getSipManager().getSipStack(),
                        callId,
                        1,
                        target,
                        localParty,
                        remoteParty,
                        route);
            } else {
                // Increment the CSEQ number of the dialog path
                dialogPath.incrementCseq();
            }

            try {
                SipRequest subscribe;
                // Create a SUBSCRIBE request
                if (mState == STATE_UNSUB)
                    subscribe = createSubscribe(0);
                else
                    subscribe = createSubscribe(mExpirePeriod);

                // Send SUBSCRIBE request
                sendSubscribe(subscribe);

            } catch (Exception e) {
                if (logger.isActivated()) {
                    logger.error("Subscribe has failed", e);
                }
                handleSubscriptionTerminated(e.getMessage());
            }
        }

        /**
         * Create a SUBSCRIBE request
         *
         * @param dialog SIP dialog path
         * @return SIP request
         * @throws Exception
         */
        private void cancelRequest() {
            stopTimer();

            switch (mState) {
            case STATE_WAIT:
            case STATE_REFRESH:
                mState = STATE_CANCEL;
                break;

            case STATE_PENDING:
            case STATE_ACTIVE:
                mState = STATE_UNSUB;
                subscribe();
                break;
            }
        }

        /**
         * Create a SUBSCRIBE request
         *
         * @param dialog SIP dialog path
         * @param expirePeriod Expiration period
         * @return SIP request
         * @throws Exception
         */
        private SipRequest createSubscribe(int expirePeriod) throws Exception {
            SipRequest subscribe = null;

            subscribe = SipMessageFactory.createCpimSubscribe(
                    dialogPath, expirePeriod, mRequest.getContentType(), mRequest.getContent());
            SipUtils.setFeatureTags(subscribe, InstantMessagingService.CPM_CHAT_FEATURE_TAGS);

            // Set the Event header
            subscribe.addHeader("Event", mRequest.getSubscribeEvent());

            // Set the Accept header
            subscribe.addHeader("Accept", mRequest.getAcceptContent());

            return subscribe;
        }

        /**
         * Send SUBSCRIBE message
         *
         * @param subscribe SIP SUBSCRIBE
         * @throws Exception
         */
        private void sendSubscribe(SipRequest subscribe) throws Exception {
            if (logger.isActivated()) {
                logger.info("Send SUBSCRIBE, expire=" + subscribe.getExpires());
            }

            if (mState > STATE_WAIT) {
                // Set the Authorization header
                authenticationAgent.setProxyAuthorizationHeader(subscribe);
            }

            // Send SUBSCRIBE request
            ImsModule imsModule = Core.getInstance().getImsModule();
            SipTransactionContext ctx = imsModule.getSipManager().sendSipMessageAndWait(subscribe);

            // Analyze the received response
            if (ctx.isSipResponse()) {
                switch (mState) {
                case STATE_WAIT:
                    handleSubscribeResponse(ctx);
                    break;
                case STATE_REFRESH:
                    handleRefreshResponse(ctx);
                    break;
                case STATE_UNSUB:
                    handleUnSubscribeResponse(ctx);
                    break;
                }
            } else {
                if (logger.isActivated()) {
                    logger.debug("No response received for SUBSCRIBE");
                }
                handleSubscriptionTerminated("timeout");
            }
        }

        /**
         * Send SUBSCRIBE message
         *
         * @param subscribe SIP SUBSCRIBE
         * @throws Exception
         */
        private void handleSubscribeResponse(SipTransactionContext ctx) throws Exception {
            if (mState == STATE_CANCEL) {
                handleSubscriptionTerminated("canceled");
                return;
            }

            if (ctx.getStatusCode() >= 200 &&
                ctx.getStatusCode() < 300) {
                handle200OK(ctx);
            } else
            if (ctx.getStatusCode() == 407) {
                // 407 Proxy Authentication Required
                handle407Authentication(ctx);
            } else
            if (ctx.getStatusCode() == 423) {
                // 423 Interval Too Brief
                handle423IntervalTooBrief(ctx);
            } else {
                handleSubscriptionTerminated("failed");
            }
        }

        /**
         * Send SUBSCRIBE message
         *
         * @param subscribe SIP SUBSCRIBE
         * @throws Exception
         */
        private void handleRefreshResponse(SipTransactionContext ctx) throws Exception {
            if (ctx.getStatusCode() >= 200 &&
                ctx.getStatusCode() < 300) {
                mState = STATE_ACTIVE;
                handle200OK(ctx);
            } else
            if (ctx.getStatusCode() == 407) {
                // 407 Proxy Authentication Required
                handle407Authentication(ctx);
            } else
            if (ctx.getStatusCode() == 423) {
                // 423 Interval Too Brief
                handle423IntervalTooBrief(ctx);
            } else
                // RFC6665 4-1-2-2
            if (ctx.getStatusCode() == 404 ||
                ctx.getStatusCode() == 405 ||
                ctx.getStatusCode() == 410 ||
                ctx.getStatusCode() == 416 ||
                ctx.getStatusCode() == 480 ||
                ctx.getStatusCode() == 481 ||
                ctx.getStatusCode() == 482 ||
                ctx.getStatusCode() == 483 ||
                ctx.getStatusCode() == 484 ||
                ctx.getStatusCode() == 485 ||
                ctx.getStatusCode() == 489 ||
                ctx.getStatusCode() == 501 ||
                ctx.getStatusCode() == 604) {
                handleSubscriptionTerminated("failed");
            } else {
            }
        }

        /**
         * Handle 200 0K response
         *
         * @param ctx SIP transaction context
         */
        private void handleUnSubscribeResponse(SipTransactionContext ctx) {
            // Consider subscription terminated anyway
            handleSubscriptionTerminated("stopped");
        }

        /**
         * Handle 200 0K response
         *
         * @param ctx SIP transaction context
         */
        private void handle200OK(SipTransactionContext ctx) {
            // 200 OK response received
            if (logger.isActivated()) {
                logger.info("200 OK response received");
            }

            SipResponse resp = ctx.getSipResponse();

            // Set the route path with the Record-Route header
            Vector<String> newRoute = SipUtils.routeProcessing(resp, true);
            dialogPath.setRoute(newRoute);

            // Set the remote tag
            dialogPath.setRemoteTag(resp.getToTag());

            // Set the target
            dialogPath.setTarget(resp.getContactURI());

            // Set the Proxy-Authorization header
            authenticationAgent.readProxyAuthenticateHeader(resp);

            // Retrieve the expire value in the response
            int expire = retrieveExpirePeriod(resp);
            if (expire > 0 && expire < mExpirePeriod) {
                mExpirePeriod = expire;
                stopTimer();
            }

            // Start the periodic subscribe
            if (mExpirePeriod <= 1200 ) {
                startTimer(mExpirePeriod, 0.5);
            } else {
                startTimer(mExpirePeriod-600);
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

            // Create a second SUBSCRIBE request with the right token
            if (logger.isActivated()) {
                logger.info("Send second SUBSCRIBE");
            }

            int expire = retrieveExpirePeriod(resp);
            SipRequest subscribe;
            if (expire > 0)
                subscribe = createSubscribe(expire);
            else
                subscribe = createSubscribe(mExpirePeriod);

            // Set the Authorization header
            authenticationAgent.setProxyAuthorizationHeader(subscribe);

            // Send SUBSCRIBE request
            sendSubscribe(subscribe);
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
                handleSubscriptionTerminated("No Min-Expires value found");
                return;
            }

            // Save the min expire value in the terminal registry
            RegistryFactory.getFactory().writeInteger(MIN_EXPIRE_PERIOD, minExpire);

            // Set the default expire value
            mExpirePeriod = minExpire;

            // Create a new SUBSCRIBE request with the right expire period
            SipRequest subscribe = createSubscribe(mExpirePeriod);

            // Set the Authorization header
            authenticationAgent.setProxyAuthorizationHeader(subscribe);

            // Send SUBSCRIBE request
            sendSubscribe(subscribe);
        }

        private void handleSubscriptionTerminated(String reason) {
            if (logger.isActivated())
                logger.debug("handleSubscriptionTerminated->reason[" + reason + "]");

            dialogPath = null;

            stopTimer();

            mManager.removeSubscription(this);

            mManager = null;

            mCallback.onTerminated(reason, 0);
        }

        /**
         * Retrieve the expire period
         *
         * @param resp SIP response
         */
        private int retrieveExpirePeriod(SipResponse response) {
            // Extract expire value from Contact header
            ListIterator<Header> contacts = response.getHeaders(ContactHeader.NAME);
            if (contacts != null) {
                while(contacts.hasNext()) {
                    ContactHeader contact = (ContactHeader)contacts.next();
                    int expires = contact.getExpires();
                    if (expires != -1) {
                        if (logger.isActivated()) {
                            logger.info("Expire period set from Contact header" +expires);
                        }
                        return expires;
                    }
                }
            }

            // Extract expire value from Expires header
            ExpiresHeader expiresHeader = (ExpiresHeader)response.getHeader(ExpiresHeader.NAME);
            if (expiresHeader != null) {
                int expires = expiresHeader.getExpires();
                if (expires != -1) {
                    return expires;
                }
            }

            return 0;
        }

        /**
         * Wait notify state
         */
        private static final int STATE_WAIT     = 0;

        /**
         * Pending state
         */
        private static final int STATE_PENDING  = 1;

        /**
         * Active state
         */
        private static final int STATE_ACTIVE   = 2;

        /**
         * Wait refresh done
         */
        private static final int STATE_REFRESH  = 3;

        /**
         * User cancel subscription
         */
        private static final int STATE_CANCEL   = 4;

        /**
         * User un-subscribe
         */
        private static final int STATE_UNSUB    = 5;

        /**
         * Minimal expired period
         */
        private static final String MIN_EXPIRE_PERIOD = "MinSubscribeExpirePeriod";

        /**
         * Subscribe manager
         */
        private SubscriptionManager mManager;

        /**
         * Subscribe request
         */
        private SubscribeRequest mRequest;

        /**
         * Event notify callback
         */
        private EventCallback mCallback;

        /**
         * Identity of subscription
         */
        private String mIdentity;

        /**
         * Current expired period
         */
        private int mExpirePeriod;

        /**
         * Dialog path
         */
        private SipDialogPath dialogPath;

        /**
         * Authentication agent
         */
        private SessionAuthenticationAgent authenticationAgent;

        /**
         * State of subscription
         */
        private int mState = STATE_WAIT;

        /**
         * Logger
         */
        private Logger logger = Logger.getLogger(this.getClass().getName());
    }

    private static SubscriptionManager mInstance = null;
    private HashMap<String,Subscription> subscriptions = new HashMap<String, Subscription>();
    private ArrayList<Subscription> processingRequests =
            new ArrayList<Subscription>();

    private Logger logger = Logger.getLogger(this.getClass().getName());
}
