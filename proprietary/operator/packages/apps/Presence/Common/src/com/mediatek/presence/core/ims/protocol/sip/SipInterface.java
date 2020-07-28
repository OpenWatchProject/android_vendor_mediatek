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

package com.mediatek.presence.core.ims.protocol.sip;

import java.io.File;
import java.util.ArrayList;
import java.util.ListIterator;
import java.util.Properties;
import java.util.Vector;

import javax2.sip.ClientTransaction;
import javax2.sip.DialogTerminatedEvent;
import javax2.sip.IOExceptionEvent;
import javax2.sip.ListeningPoint;
import javax2.sip.RequestEvent;
import javax2.sip.ResponseEvent;
import javax2.sip.ServerTransaction;
import javax2.sip.SipFactory;
import javax2.sip.SipListener;
import javax2.sip.SipProvider;
import javax2.sip.SipStack;
import javax2.sip.TimeoutEvent;
import javax2.sip.TransactionTerminatedEvent;
import javax2.sip.address.Address;
import javax2.sip.address.SipURI;
import javax2.sip.header.ContactHeader;
import javax2.sip.header.ExtensionHeader;
import javax2.sip.header.Header;
import javax2.sip.header.ViaHeader;

import android.net.ConnectivityManager;
import android.net.NetworkCapabilities;

import com.mediatek.presence.core.ims.network.ImsNetworkInterface;
import com.mediatek.presence.core.ims.network.sip.SipManager;
import com.mediatek.presence.core.ims.network.sip.SipMessageFactory;
import com.mediatek.presence.core.ims.network.sip.SipUtils;
import com.mediatek.presence.core.ims.security.cert.KeyStoreManager;
import com.mediatek.presence.provider.settings.RcsSettings;
import com.mediatek.presence.provider.settings.RcsSettingsManager;
import com.mediatek.presence.utils.IdGenerator;
import com.mediatek.presence.utils.IpAddressUtils;
import com.mediatek.presence.utils.NetworkRessourceManager;
import com.mediatek.presence.utils.logger.Logger;
import com.mediatek.presence.utils.SimUtils;
import com.mediatek.presence.core.Core;
import com.mediatek.presence.core.ims.ImsModule;

import com.mediatek.presence.core.ims.rcsua.RcsUaAdapter;
import com.mediatek.presence.core.ims.rcsua.RcsUaAdapterManager;

/**
 * SIP interface which manage the SIP stack. The NIST stack is used
 * statefully (i.e. messages are sent via a SIP transaction).
 *
 * NIST release is nist-sip-96f517a (2010-10-29)
 *
 * @author JM. Auffret
 */
public class SipInterface implements SipListener {
    /**
     * Trace separator
     */
    private final static String TRACE_SEPARATOR = "-----------------------------------------------------------------------------";

    /**
     * Default SIP port
     */
    public final static int DEFAULT_SIP_PORT = 5060;

    /**
     * Local IP address
     */
    private String localIpAddress;

    /**
     * Outbound proxy address
     */
    private String outboundProxyAddr;

    /**
     * Outbound proxy port
     */
    private int outboundProxyPort;

    /**
     * Default route path
     */
    private Vector<String> defaultRoutePath;

    /**
     * Service route path
     */
    private Vector<String> serviceRoutePath;

    /**
     * SIP listening port
     */
    private int listeningPort;

    /**
     * SIP default protocol
     */
    private String defaultProtocol;

    /**
     *  List of current SIP transactions
     */
    private SipTransactionList transactions = new SipTransactionList();

    /**
     * SIP interface listeners
     */
    private Vector<SipEventListener> listeners = new Vector<SipEventListener>();

    /**
     * SIP stack
     */
    private SipStack sipStack;

    /**
     * Default SIP stack provider
     */
    private SipProvider defaultSipProvider;

    /**
     * SIP stack providers
     */
    private Vector<SipProvider> sipProviders = new Vector<SipProvider>();

    /**
     * Keep-alive manager
     */
    private KeepAliveManager keepAliveManager = null;

    /**
     * Public GRUU
     */
    private String publicGruu = null;

    /**
     * Temporary GRUU
     */
    private String tempGruu = null;

    /**
     * Instance ID
     */
    private String instanceId = null;

    /**
     * Base timer T1 (in ms)
     */
    private int timerT1 = 500;

    /**
     * Base timer T2 (in ms)
     */
    private int timerT2 = 4000;

    /**
     * Base timer T4 (in ms)
     */
    private int timerT4 = 5000;

    private ImsModule mImsModule = null;
    private int mSlotId = 0;

    /**
     * The logger
     */
    private Logger logger = null;

    /**
     * Constructor
     *
     * @param localIpAddress Local IP address
     * @param proxyAddr Outbound proxy address
     * @param proxyPort Outbound proxy port
     * @param defaultProtocol Default protocol
     * @param networkType Type of network
     * @throws SipException
     */
    public SipInterface(ImsModule imsModule, String localIpAddress, String proxyAddr,
            int proxyPort, String defaultProtocol, int networkType) throws SipException {


        this.mSlotId = imsModule.getSlotId();
        this.localIpAddress = localIpAddress;
        this.defaultProtocol = defaultProtocol;
        this.listeningPort = NetworkRessourceManager.generateLocalSipPort(mSlotId);
        this.outboundProxyAddr = proxyAddr;
        this.outboundProxyPort = proxyPort;
        this.mImsModule = imsModule;
        this.keepAliveManager = new KeepAliveManager(mSlotId, this);
        logger = Logger.getLogger(mSlotId, "SipInterface");

        if (logger.isActivated()) {
            logger.debug("SIPINTERFACE; localIpAddress =  " + localIpAddress
                    +" ;destination address : "+ proxyAddr + " ; destination default port : "+proxyPort+ " ; DEFAULT_PROTOCOL : " +defaultProtocol
                    + " ; network type = "+networkType
                    );
           logger.debug("listeningPort:" + listeningPort);
        }


        // Set timers value from provisioning for 3G or default for Wifi
        if (networkType == NetworkCapabilities.TRANSPORT_CELLULAR) {
            this.timerT1 = RcsSettingsManager.getRcsSettingsInstance(mSlotId).getSipTimerT1();
            this.timerT2 = RcsSettingsManager.getRcsSettingsInstance(mSlotId).getSipTimerT2();
            this.timerT4 = RcsSettingsManager.getRcsSettingsInstance(mSlotId).getSipTimerT4();
        }

        // Set the default route path
        defaultRoutePath = new Vector<String>();
        defaultRoutePath.addElement(getDefaultRoute());

        // Set the default service route path
        serviceRoutePath = new Vector<String>();
        serviceRoutePath.addElement(getDefaultRoute());

        try {
            // Init SIP factories
            SipFactory sipFactory = SipFactory.getInstance();
            SipUtils.HEADER_FACTORY = sipFactory.createHeaderFactory();
            SipUtils.ADDR_FACTORY = sipFactory.createAddressFactory();
            SipUtils.MSG_FACTORY = sipFactory.createMessageFactory();

            // Set SIP stack properties
            Properties properties = new Properties();
            properties.setProperty("javax2.sip.STACK_NAME", localIpAddress);
            properties.setProperty("gov2.nist.javax2.sip.THREAD_POOL_SIZE", "1");
            if (defaultProtocol.equals(ListeningPoint.TLS)) {
                // Set SSL properties
                properties.setProperty("gov2.nist.javax2.sip.TLS_CLIENT_PROTOCOLS", "SSLv3, TLSv1");
                properties.setProperty("javax2.net.ssl.keyStoreType", KeyStoreManager.getKeystoreType());
                properties.setProperty("javax2.net.ssl.keyStore", KeyStoreManager.getKeystorePath());
                properties.setProperty("javax2.net.ssl.keyStorePassword", KeyStoreManager.getKeystorePassword());
                properties.setProperty("javax2.net.ssl.trustStore", KeyStoreManager.getKeystorePath());
            }

            // Create the SIP stack
            sipStack = sipFactory.createSipStack(mSlotId, properties);

            if (RcsSettingsManager.getRcsSettingsInstance(mSlotId).isSingleRegistrationFeasible()) {
                ListeningPoint tcp = sipStack.createListeningPoint(localIpAddress, listeningPort, ListeningPoint.TCP);
                SipProvider tcpSipProvider = sipStack.createSipProvider(tcp);
                tcpSipProvider.addSipListener(this);
                sipProviders.addElement(tcpSipProvider);

                // TCP protocol used by default
                defaultSipProvider = tcpSipProvider;
                defaultProtocol = ListeningPoint.TCP;
                listeningPort = 5060;
                // change listen port for composeing the message. the prot should be 5060.
                // volte module will not expose the outgoing port and will replace the 5060 in the SIP
                // request with the actual ports
                // initialize the listening port
                if (logger.isActivated()) {
                    logger.debug("SIP stack started at " + localIpAddress + ":" + listeningPort);
                }
            } else {
                // Create UDP provider
                ListeningPoint udp = sipStack.createListeningPoint(localIpAddress, listeningPort, ListeningPoint.UDP);
                SipProvider udpSipProvider = sipStack.createSipProvider(udp);
                udpSipProvider.addSipListener(this);
                sipProviders.addElement(udpSipProvider);

                // Set the default SIP provider
                if (defaultProtocol.equals(ListeningPoint.TLS)) {
                    if (logger.isActivated()) {
                        logger.debug("SIPINTERFACE TLS");
                    }
                    // Create TLS provider
                    ListeningPoint tls = sipStack.createListeningPoint(localIpAddress, listeningPort, ListeningPoint.TLS);
                    SipProvider tlsSipProvider = sipStack.createSipProvider(tls);
                    tlsSipProvider.addSipListener(this);
                    sipProviders.addElement(tlsSipProvider);

                    // TLS protocol used by default
                    defaultSipProvider = tlsSipProvider;
                } else
                if (defaultProtocol.equals(ListeningPoint.TCP)) {
                    if (logger.isActivated()) {
                        logger.debug("SIPINTERFACE TCP");
                    }
                    // Create TCP provider
                    ListeningPoint tcp = sipStack.createListeningPoint(localIpAddress, listeningPort, ListeningPoint.TCP);
                    SipProvider tcpSipProvider = sipStack.createSipProvider(tcp);
                    tcpSipProvider.addSipListener(this);
                    sipProviders.addElement(tcpSipProvider);

                    // TCP protocol used by default
                    defaultSipProvider = tcpSipProvider;
                } else {
                    // Create TCP provider
                    ListeningPoint tcp = sipStack.createListeningPoint(localIpAddress, listeningPort, ListeningPoint.TCP);
                    SipProvider tcpSipProvider = sipStack.createSipProvider(tcp);
                    tcpSipProvider.addSipListener(this);
                    sipProviders.addElement(tcpSipProvider);

                    if (logger.isActivated()) {
                        logger.debug("SIPINTERFACE UDP");
                    }

                    // UDP protocol used by default
                    defaultSipProvider = udpSipProvider;
                }
            }

             /** M: add for SioTLS @{ */
            /*
            if (ListeningPoint.TLS.equals(defaultProtocol)) {
                logger.debug("initPrivateKeyAndSelfsignedCertificate ");
                KeyStoreManager.initPrivateKeyAndSelfsignedCertificate();
            }
            */
            /** @}*/

            // Start the stack
            sipStack.start();

        } catch(Exception e) {
            if (logger.isActivated()) {
                logger.error("SIP stack initialization has failed", e);
            }
            //throw new SipException("Can't create the SIP stack");
        }
    }

    /**
     * Close the SIP stack
     */
    public void close() {
        try {
            // Stop keep alive
            keepAliveManager.stop();

            // Remove all application listeners
            listeners.removeAllElements();

            // Delete SIP providers
            for(int i=0; i < sipProviders.size(); i++) {
                SipProvider sipProvider = (SipProvider)sipProviders.elementAt(i);
                sipProvider.removeSipListener(this);
                sipProvider.removeListeningPoints();
                sipStack.deleteSipProvider(sipProvider);
            }

            // Stop the stack
            sipStack.stop();
            SipFactory.getInstance().resetFactory();
        } catch(Exception e) {
            if (logger.isActivated()) {
                logger.error("Can't stop SIP stack correctly", e);
            }
        }
    }

    /**
     * Return the default SIP provider
     *
     * @return SIP provider
     */
    public SipProvider getDefaultSipProvider() {
        return defaultSipProvider;
    }

    /**
     * Returns the local IP address
     *
     * @return IP address
     */
    public String getLocalIpAddress() {
        return localIpAddress;
    }

    /**
     * Returns the outbound proxy address
     *
     * @return Outbound proxy address
     */
    public String getOutboundProxyAddr() {
        return outboundProxyAddr;
    }

    /**
     * Returns the outbound proxy port
     *
     * @return Outbound proxy port
     */
    public int getOutboundProxyPort() {
        return outboundProxyPort;
    }

    /**
     * Returns the proxy protocol
     *
     * @return Outbound proxy protocol
     */
    public String getProxyProtocol() {
        return defaultProtocol;
    }

    public int getListeningPort() {
        return listeningPort;
    }

    /**
     * Returns the keep-alive manager
     *
     * @return Keep-alive manager
     */
    public KeepAliveManager getKeepAliveManager() {
        return keepAliveManager;
    }

    /**
     * Get public GRUU
     *
     * @return GRUU
     */
    public String getPublicGruu() {
        return publicGruu;
    }

    /**
     * Set public GRUU
     *
     * @param gruu GRUU
     */
    public void setPublicGruu(String gruu) {
        this.publicGruu = gruu;
    }

    /**
     * Get temporary GRUU
     *
     * @return GRUU
     */
    public String getTemporaryGruu() {
        return tempGruu;
    }

    /**
     * Set temporary GRUU
     *
     * @param gruu GRUU
     */
    public void setTemporaryGruu(String gruu) {
        this.tempGruu = gruu;
    }

    /**
     * Get instance ID
     *
     * @return ID
     */
    public String getInstanceId() {
        return instanceId;
    }

    /**
     * Set instance ID
     *
     * @param id Instance ID
     */
    public void setInstanceId(String id) {
        this.instanceId = id;
    }

    /**
     * Returns the local via path
     *
     * @return List of headers
     * @throws Exception
     */
    public ArrayList<ViaHeader> getViaHeaders() throws Exception {
        ArrayList<ViaHeader> viaHeaders = new ArrayList<ViaHeader>();
        ViaHeader via = SipUtils.HEADER_FACTORY.createViaHeader(localIpAddress,
                getListeningPort(),
                getProxyProtocol(),
                null);
        viaHeaders.add(via);
        return viaHeaders;
    }

    /**
     * Generate a unique call-ID
     *
     * @return Call-Id
     */
    public String generateCallId() {
        return IdGenerator.getIdentifier() + "@" + localIpAddress;
    }

    /**
     * Get local contact
     *
     * @return Header
     * @throws Exception
     */
    public ContactHeader getLocalContactWithAddress() throws Exception {

        /**
         * M: Modified to add the username in the contact header
         *
         * @{
         */
        // Set the contact with the terminal IP address and SIP port
        SipURI contactURI = (SipURI) SipUtils.ADDR_FACTORY.createSipURI(
                ImsModule.IMS_USER_PROFILE.getUsername(), localIpAddress);

        /**
         * @}
         */
        contactURI.setPort(listeningPort);
        contactURI.setParameter("transport", defaultProtocol);
        // Create the Contact header
        Address contactAddress = SipUtils.ADDR_FACTORY.createAddress(contactURI);
        ContactHeader contactHeader = SipUtils.HEADER_FACTORY.createContactHeader(contactAddress);

        return contactHeader;
    }

    /**
     * Get local contact
     *
     * @return Header
     * @throws Exception
     */
    public ContactHeader getLocalContact() throws Exception {
        // Set the contact with the terminal IP address, port and transport
        SipURI contactURI = null;

        /**
         * M: Modified to add the username in the contact header
         * @{
         */
        // Set the contact with the terminal IP address and SIP port
        contactURI = (SipURI)SipUtils.ADDR_FACTORY.createSipURI(
                ImsModule.IMS_USER_PROFILE.getUsername(), localIpAddress);

        contactURI.setPort(getListeningPort());
        contactURI.setParameter("transport", defaultProtocol);
        // do not must set sips:XXXX in TLS for Contact header.
        // rfc5630 3.1.3
        // contactURI.setSecure(defaultProtocol.equals(ListeningPoint.TLS));
        // Create the Contact header
        Address contactAddress = SipUtils.ADDR_FACTORY.createAddress(contactURI);
        ContactHeader contactHeader = SipUtils.HEADER_FACTORY.createContactHeader(contactAddress);

        return contactHeader;
    }



    /**
     * Get contact based on local contact info and multidevice
     * infos (GRUU, sip.instance)
     *
     * @return Header
     * @throws Exception
     */
    public ContactHeader getContact() throws Exception {
        ContactHeader contactHeader;
        if (publicGruu != null) {
            // Create a contact with GRUU
            SipURI contactURI = (SipURI)SipUtils.ADDR_FACTORY.createSipURI(publicGruu);
            Address contactAddress = SipUtils.ADDR_FACTORY.createAddress(contactURI);
            contactHeader = SipUtils.HEADER_FACTORY.createContactHeader(contactAddress);
        } else
        if (instanceId != null) {
            // Create a local contact with an instance ID
            contactHeader = getLocalContactWithAddress();
            contactHeader.setParameter(SipUtils.SIP_INSTANCE_PARAM, instanceId);
        } else {
            // Create a local contact
            contactHeader = getLocalContact();
        }

        // add +linesx if digits registers
        if (SimUtils.isTmoSimCard(mSlotId)) {
            int vlineCount = RcsUaAdapterManager.getRcsUaAdapter(mSlotId).getVirtualLineCount();
            if (vlineCount != 0) {
                contactHeader.setParameter(null, "+lines" + vlineCount);
            }
        }

        return contactHeader;
    }

    /**
     * Returns the default route
     *
     * @return Route
     */
    public String getDefaultRoute() {
        String defaultRoute;
        if (IpAddressUtils.isIPv6(outboundProxyAddr)) {
            defaultRoute = String.format("<sip:[%s]:%s;transport=%s;lr>",
                    outboundProxyAddr, outboundProxyPort, getProxyProtocol());
        } else {
            defaultRoute = String.format("<sip:%s:%s;transport=%s;lr>",
                    outboundProxyAddr, outboundProxyPort, getProxyProtocol());
        }
        return defaultRoute.toLowerCase();
    }

    /**
     * Returns the default route path
     *
     * @return Route path
     */
    public Vector<String> getDefaultRoutePath() {
        return defaultRoutePath;
    }

    /**
     * Returns the service route path
     *
     * @return Route path
     */
    public Vector<String> getServiceRoutePath() {
        return serviceRoutePath;
    }

    /**
     * Set the service route path
     *
     * @param routes List of routes
     */
    public void setServiceRoutePath(ListIterator<Header> routes) {
        serviceRoutePath.clear();

        // Always add the outbound proxy
        serviceRoutePath.addElement(getDefaultRoute());

        if (routes != null) {
            // Add the received service route path
            while(routes.hasNext()) {
                ExtensionHeader route = (ExtensionHeader)routes.next();
                String rt = route.getValue().toLowerCase();
                if (!serviceRoutePath.contains(rt)) {
                    serviceRoutePath.addElement(rt);
                }
            }
        }
    }

    /**
     * Add a SIP event listener
     *
     * @param listener Listener
     */
    public void addSipEventListener(SipEventListener listener) {
        if (logger.isActivated()) {
            logger.debug("Add a SIP listener:" + listener);
        }
        listeners.addElement(listener);
    }

    /**
     * Remove a SIP event listener
     *
     * @param listener Listener
     */
    public void removeSipEventListener(SipEventListener listener) {
        if (logger.isActivated()) {
            logger.debug("Remove a SIP listener");
        }
        listeners.removeElement(listener);
    }

    /**
     * Remove a transaction context from its ID
     *
     * @param id Transaction ID
     */
    public synchronized void removeTransactionContext(String id) {
        transactions.remove(id);
    }

    /**
     * Notify the transaction context that a message has been received (response or ACK)
     *
     * @param id Transaction ID
     * @param msg SIP message
     */
    public void notifyTransactionContext(String transactionId, SipMessage msg) {
        SipTransactionContext ctx = (SipTransactionContext)transactions.get(transactionId);
        if (ctx != null) {
            if (logger.isActivated()) {
                logger.debug("Callback object found for transaction " + transactionId);
            }
            removeTransactionContext(transactionId);
            ctx.responseReceived(msg);
        }
   }

    /**
     * Send a SIP message and create a context to wait a response
     *
     * @param message SIP message
     * @return Transaction context
     * @throws SipException
     * @throws RSTException
     */
    public SipTransactionContext sendSipMessageAndWait(SipMessage message) throws SipException {
        try {
            if (message instanceof SipRequest) {
                // Send a request
                SipRequest req = (SipRequest)message;

                // Get stack transaction
                ClientTransaction transaction = (ClientTransaction)req.getStackTransaction();
                if (transaction == null) {
                    // Create a new transaction
                    transaction = getDefaultSipProvider().getNewClientTransaction(req.getStackMessage());
                    transaction.setRetransmitTimers(timerT1, timerT2, timerT4);
                    req.setStackTransaction(transaction);
                }

                // Create a transaction context
                SipTransactionContext ctx = new SipTransactionContext(mSlotId, transaction);
                String id = SipTransactionContext.getTransactionContextId(req);
                transactions.put(id, ctx);
                if (logger.isActivated()) {
                    logger.debug("Create a transaction context " + id);
                }

                // Send the SIP message to the network
                if (logger.isActivated()) {
                    logger.debug(">>> Send SIP " + req.getMethod());
                }
                transaction.sendRequest();

                // Returns the created transaction to wait synchronously the response
                return ctx;
            } else {
                // Send a response
                SipResponse resp = (SipResponse)message;

                // Get stack transaction
                ServerTransaction transaction = (ServerTransaction)resp.getStackTransaction();
                if (transaction == null) {
                    // No transaction exist
                    if (logger.isActivated()) {
                        logger.warn("No transaction exist for " + resp.getCallId() + ": the response can't be sent");
                    }
                    return null;
                }

                // Create a transaction context
                SipTransactionContext ctx = new SipTransactionContext(mSlotId, transaction);
                String id = SipTransactionContext.getTransactionContextId(resp);
                transactions.put(id, ctx);
                if (logger.isActivated()) {
                    logger.debug("Create a transaction context " + id);
                }

                // Send the SIP message to the network
                if (logger.isActivated()) {
                    logger.debug(">>> Send SIP " + resp.getStatusCode() + " response");
                }
                transaction.sendResponse(resp.getStackMessage());

                // Returns the created transaction to wait synchronously the response
                return ctx;
            }
        }catch(javax2.sip.SipException e){
            throw new SipException("Can't send SIP message");

        } catch(Exception e) {
            if (logger.isActivated()) {
                logger.error("Can't send SIP message", e);
            }
            throw new SipException("Can't send SIP message");
        }
    }

    /**
     * Send a SIP response
     *
     * @param response SIP response
     * @throws SipException
     */
    public void sendSipResponse(SipResponse response) throws SipException {
        try {
            // Get stack transaction
            ServerTransaction transaction = (ServerTransaction)response.getStackTransaction();
            if (transaction == null) {
                // No transaction exist
                if (logger.isActivated()) {
                    logger.warn("No transaction exist for " + response.getCallId() + ": the response can't be sent");
                }
                throw new SipException("No transaction found");
            }

            // Send the SIP message to the network
            if (logger.isActivated()) {
                logger.debug(">>> Send SIP " + response.getStatusCode() + " response");
            }
            transaction.sendResponse(response.getStackMessage());
        } catch(Exception e) {
            if (logger.isActivated()) {
                logger.error("Can't send SIP message", e);
            }
            throw new SipException("Can't send SIP message");
        }
    }

    /**
     * Send a SIP ACK
     *
     * @param dialog Dialog path
     * @throws SipException
     */
    public void sendSipAck(SipDialogPath dialog) throws SipException {
        try {
            // Create the SIP request
            SipRequest ack = SipMessageFactory.createAck(mSlotId, dialog);

            // Send the SIP message to the network
            if (logger.isActivated()) {
                logger.debug(">>> Send SIP ACK");
            }

            // Re-use INVITE transaction
            dialog.getStackDialog().sendAck(ack.getStackMessage());
        } catch(Exception e) {
            if (logger.isActivated()) {
                logger.error("Can't send SIP message", e);
            }
            throw new SipException("Can't send SIP message");
        }
    }

    /**
     * Send a SIP CANCEL
     *
     * @param dialog Dialog path
     * @throws SipException
     */
    public void sendSipCancel(SipDialogPath dialog) throws SipException {
        try {
            if (dialog.getInvite().getStackTransaction() instanceof ServerTransaction) {
                // Server transaction can't send a cancel
                return;
            }

            // Create the SIP request
            SipRequest cancel = SipMessageFactory.createCancel(dialog);

            // Set the Proxy-Authorization header
            if (dialog.getAuthenticationAgent() != null) {
                dialog.getAuthenticationAgent().setProxyAuthorizationHeader(cancel);
            }

            // Create a new transaction
            ClientTransaction transaction = defaultSipProvider.getNewClientTransaction(cancel.getStackMessage());
            transaction.setRetransmitTimers(timerT1, timerT2, timerT4);

            // Send the SIP message to the network
            if (logger.isActivated()) {
                logger.debug(">>> Send SIP CANCEL");
            }
            transaction.sendRequest();
        } catch(Exception e) {
            if (logger.isActivated()) {
                logger.error("Can't send SIP message", e);
            }
            throw new SipException("Can't send SIP message");
        }
    }

    /**
     * Send a SIP BYE
     *
     * @param dialog Dialog path
     * @throws SipException
     */
    public void sendSipBye(SipDialogPath dialog) throws SipException {
        try {
            // Create the SIP request
            SipRequest bye = SipMessageFactory.createBye(dialog);

            // Set the Proxy-Authorization header
            if (dialog.getAuthenticationAgent() != null) {
                dialog.getAuthenticationAgent().setProxyAuthorizationHeader(bye);
            }

            // Create a new transaction
            ClientTransaction transaction = defaultSipProvider.getNewClientTransaction(bye.getStackMessage());
            transaction.setRetransmitTimers(timerT1, timerT2, timerT4);

            // Send the SIP message to the network
            if (logger.isActivated()) {
                logger.debug(">>> Send SIP BYE");
            }
            dialog.getStackDialog().sendRequest(transaction);
        } catch(Exception e) {
            if (logger.isActivated()) {
                logger.error("Can't send SIP message", e);
            }
            throw new SipException("Can't send SIP message");
        }
    }

    /**
     * Send a SIP Group BYE
     *
     * @param dialog Dialog path
     * @throws SipException
     */
    public void sendSipGroupBye(SipDialogPath dialog) throws SipException {
        try {
            // Create the SIP request
            SipRequest bye = SipMessageFactory.createGroupBye(mSlotId, dialog);

            // Set the Proxy-Authorization header
            if (dialog.getAuthenticationAgent() != null) {
                dialog.getAuthenticationAgent().setProxyAuthorizationHeader(bye);
            }

            // Create a new transaction
            ClientTransaction transaction = defaultSipProvider.getNewClientTransaction(bye.getStackMessage());
            transaction.setRetransmitTimers(timerT1, timerT2, timerT4);

            // Send the SIP message to the network
            if (logger.isActivated()) {
                logger.debug(">>> Send SIP BYE");
            }
            dialog.getStackDialog().sendRequest(transaction);
        } catch(Exception e) {
            if (logger.isActivated()) {
                logger.error("Can't send SIP message", e);
            }
            throw new SipException("Can't send SIP message");
        }
    }

    /**
     * Send a SIP UPDATE
     *
     * @param dialog Dialog path
     * @return Transaction context
     * @throws SipException
     */
    public SipTransactionContext sendSipUpdate(SipDialogPath dialog) throws SipException {
        try {
            // Create the SIP request
            SipRequest update = SipMessageFactory.createUpdate(dialog);

            // Set the Proxy-Authorization header
            if (dialog.getAuthenticationAgent() != null) {
                dialog.getAuthenticationAgent().setProxyAuthorizationHeader(update);
            }

            // Get stack transaction
            ClientTransaction transaction = defaultSipProvider.getNewClientTransaction(update.getStackMessage());
            transaction.setRetransmitTimers(timerT1, timerT2, timerT4);

            // Create a transaction context
            SipTransactionContext ctx = new SipTransactionContext(mSlotId, transaction);
            String id = SipTransactionContext.getTransactionContextId(update);
            transactions.put(id, ctx);
            if (logger.isActivated()) {
                logger.debug("Create a transaction context " + id);
            }

            // Send the SIP message to the network
            if (logger.isActivated()) {
                logger.debug(">>> Send SIP UPDATE");
            }
            transaction.sendRequest();

            // Returns the created transaction to wait synchronously the response
            return ctx;
        } catch(Exception e) {
            if (logger.isActivated()) {
                logger.error("Can't send SIP message", e);
            }
            throw new SipException("Can't send SIP message");
        }
    }

    /**
     * Send a subsequent SIP request and create a context to wait a response
     *
     * @param dialog Dialog path
     * @param request Request
     * @throws SipException
     */
    public SipTransactionContext sendSubsequentRequest(SipDialogPath dialog, SipRequest request) throws SipException {
        try {
            // Set the Proxy-Authorization header
            if (dialog.getAuthenticationAgent() != null) {
                dialog.getAuthenticationAgent().setProxyAuthorizationHeader(request);
            }

            // Get stack transaction
            ClientTransaction transaction = defaultSipProvider.getNewClientTransaction(request.getStackMessage());
            transaction.setRetransmitTimers(timerT1, timerT2, timerT4);

            // Send the SIP message to the network
            if (logger.isActivated()) {
                logger.debug(">>> Send SIP " + request.getMethod().toUpperCase());
            }
            dialog.getStackDialog().sendRequest(transaction);

            // Create a transaction context
            SipTransactionContext ctx = new SipTransactionContext(mSlotId, transaction);
            String id = SipTransactionContext.getTransactionContextId(request);
            transactions.put(id, ctx);

            // Returns the created transaction to wait synchronously the response
            return ctx;
        } catch(Exception e) {
            if (logger.isActivated()) {
                logger.error("Can't send SIP message", e);
            }
            throw new SipException("Can't send SIP message");
        }
    }

    /**
     * Process an asynchronously reported DialogTerminatedEvent
     *
     * @param dialogTerminatedEvent Event
     */
    public void processDialogTerminated(DialogTerminatedEvent dialogTerminatedEvent) {
        if (logger.isActivated()) {
            logger.debug("Dialog terminated");
        }
    }

    /**
     * Process an asynchronously reported IO Exception
     *
     * @param exceptionEvent Event
     */
    public void processIOException(IOExceptionEvent exceptionEvent) {
        if (logger.isActivated()) {
            logger.debug("IO Exception on " + exceptionEvent.getTransport() + " transport");
        }

        if (!SimUtils.isCmccSimCard(mSlotId))
            return;

        new Thread() {
            public void run() {
                if (logger.isActivated()) {
                    logger.debug("RST exception handling ");
                }
                if (Core.getInstance() != null) {
                    if (mImsModule != null) {
                        SipManager manager = mImsModule.getSipManager();
                        if (manager != null) {
                            ImsNetworkInterface networkInterface = manager.getNetworkInterface();
                            networkInterface.getRegistrationManager().doPreReRegistrationProcess();
                            networkInterface.getRegistrationManager().registration();
                        }
                    }
                }
            }
        }.start();
    }

    /**
     * Processes a Request received on a SipProvider upon which this SipListener
     * is registered.
     *
     * @param requestEvent Event
     */
    public void processRequest(RequestEvent requestEvent) {
        if (logger.isActivated()) {
            logger.debug("<<< Receive SIP " + requestEvent.getRequest().getMethod());
        }
        // Get transaction
        ServerTransaction transaction = requestEvent.getServerTransaction();
        if (transaction == null) {
            try {
                // Create a transaction for this new incoming request
                SipProvider srcSipProvider = (SipProvider)requestEvent.getSource();
                transaction = srcSipProvider.getNewServerTransaction(requestEvent.getRequest());
            } catch(Exception e) {
                e.printStackTrace();
                if (logger.isActivated()) {
                    logger.error("Unable to create a new server transaction for an incoming request");
                }
                return;
            }
        }

        // Create received request with its associated transaction
        SipRequest req = new SipRequest(requestEvent.getRequest());
        req.setStackTransaction(transaction);

        if (req.getMethod().equals("ACK")) {
            // Search the context associated to the received ACK and notify it
            String transactionId = SipTransactionContext.getTransactionContextId(req);
            notifyTransactionContext(transactionId, req);
            return;
        }

        // Notify event listeners
        for(int i=0; i < listeners.size(); i++) {
            if (logger.isActivated()) {
                logger.debug("Notify a SIP listener: " + (SipEventListener)listeners.elementAt(i));
            }
            SipEventListener listener = (SipEventListener)listeners.elementAt(i);
            listener.receiveSipRequest(req);
        }
    }

    /**
     * Processes a Response received on a SipProvider upon which this SipListener
     * is registered
     *
     * @param responseEvent Event
     */
    public void processResponse(ResponseEvent responseEvent) {
        if (logger.isActivated()) {
            logger.debug("<<< Receive SIP " + responseEvent.getResponse().getStatusCode() + " response");
        }
        // Search transaction
        ClientTransaction transaction = responseEvent.getClientTransaction();
        if (transaction == null) {
            if (logger.isActivated()) {
                logger.debug("No transaction exist for this response: by-pass it");
            }
            return;
        }

        if (responseEvent.getResponse().getStatusCode() >= 200) {
            // Create received response with its associated transaction
            SipResponse resp = new SipResponse(responseEvent.getResponse());
            resp.setStackTransaction(transaction);

            // Search the context associated to the received response and notify it
            String transactionId = SipTransactionContext.getTransactionContextId(resp);
             if (logger.isActivated()) {
                logger.debug("notify to transaction");
            }
            notifyTransactionContext(transactionId, resp);
        } else {
            if (logger.isActivated()) {
                logger.debug("By pass provisional response");
            }
        }
    }

    /**
     * Processes a retransmit or expiration Timeout of an underlying
     * Transaction handled by this SipListener
     *
     * @param timeoutEvent Event
     */
    public void processTimeout(TimeoutEvent timeoutEvent) {
        if (logger.isActivated()) {
            logger.debug("Transaction timeout " + timeoutEvent.getTimeout().toString());
        }

        if (timeoutEvent.isServerTransaction()) {
            if (logger.isActivated()) {
                logger.warn("Unexpected timeout for a server transaction: should never arrives");
            }
            return;
        }

        ClientTransaction transaction = (ClientTransaction)timeoutEvent.getClientTransaction();
        if (transaction == null) {
            if (logger.isActivated()) {
                logger.debug("No transaction exist for this transaction: by-pass it");
            }
            return;
        }

        // Search the context associated to the received timeout and notify it
        String transactionId = SipTransactionContext.getTransactionContextId(transaction.getRequest());
        notifyTransactionContext(transactionId, null);
    }

    /**
     *  Process an asynchronously reported TransactionTerminatedEvent
     *
     * @param transactionTerminatedEvent Event
     */
    public void processTransactionTerminated(TransactionTerminatedEvent transactionTerminatedEvent) {
        if (logger.isActivated()) {
            logger.debug("Transaction terminated");
        }
    }

    public ImsModule getImsModule() {
        return mImsModule;
    }
}
