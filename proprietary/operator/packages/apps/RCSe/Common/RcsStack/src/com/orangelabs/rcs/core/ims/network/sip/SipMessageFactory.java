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

package com.orangelabs.rcs.core.ims.network.sip;

import gov2.nist.core.NameValue;
import gov2.nist.javax2.sip.Utils;
import gov2.nist.javax2.sip.header.Subject;
import gov2.nist.javax2.sip.header.ims.PAccessNetworkInfoHeader;


import static com.orangelabs.rcs.utils.StringUtils.UTF8;

import java.net.URLEncoder;
import java.util.ArrayList;
import java.util.HashSet;
import java.util.List;
import java.util.Vector;
import java.util.Arrays;

import android.content.Context;
import android.net.ConnectivityManager;
import android.net.NetworkInfo;
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;
import android.text.TextUtils;
import android.net.Uri;

import com.orangelabs.rcs.core.ims.protocol.sip.SipMessage;

import javax2.sip.ClientTransaction;
import javax2.sip.Dialog;
import javax2.sip.address.Address;
import javax2.sip.address.URI;
import javax2.sip.header.AcceptHeader;
import javax2.sip.header.CSeqHeader;
import javax2.sip.header.CallIdHeader;
import javax2.sip.header.ContactHeader;
import javax2.sip.header.ContentDispositionHeader;
import javax2.sip.header.ContentLengthHeader;
import javax2.sip.header.ContentTypeHeader;
import javax2.sip.header.EventHeader;
import javax2.sip.header.ExpiresHeader;
import javax2.sip.header.FromHeader;
import javax2.sip.header.Header;
import javax2.sip.header.ReasonHeader;
import javax2.sip.header.ReferToHeader;
import javax2.sip.header.ProxyRequireHeader;
import javax2.sip.header.RequireHeader;
import javax2.sip.header.RouteHeader;
import javax2.sip.header.SIPIfMatchHeader;
import javax2.sip.header.SupportedHeader;
import javax2.sip.header.ToHeader;
import javax2.sip.header.UserAgentHeader;
import javax2.sip.header.ViaHeader;
import javax2.sip.message.Request;
import javax2.sip.message.Response;

import com.orangelabs.rcs.core.ims.ImsModule;
import com.orangelabs.rcs.core.ims.network.registration.RegistrationManager;
import com.orangelabs.rcs.core.ims.protocol.sip.PLastAccessNetworkInfoHeader;
import com.orangelabs.rcs.core.ims.ImsModule;
import com.orangelabs.rcs.core.ims.protocol.sip.SipDialogPath;
import com.orangelabs.rcs.core.ims.protocol.sip.SipException;
import com.orangelabs.rcs.core.ims.protocol.sip.SipRequest;
import com.orangelabs.rcs.core.ims.protocol.sip.SipResponse;
import com.orangelabs.rcs.core.ims.service.SessionTimerManager;
import com.orangelabs.rcs.core.ims.service.im.InstantMessagingService;
import com.orangelabs.rcs.core.ims.service.im.chat.ChatUtils;
import com.orangelabs.rcs.platform.AndroidFactory;
import com.orangelabs.rcs.provider.settings.RcsSettings;
import com.orangelabs.rcs.utils.DeviceUtils;
import com.orangelabs.rcs.utils.IdGenerator;
import com.orangelabs.rcs.utils.PhoneUtils;
import com.orangelabs.rcs.utils.logger.Logger;
import com.orangelabs.rcs.core.ims.protocol.PayloadException;
/**
 * SIP message factory
 *
 * @author Jean-Marc AUFFRET
 */
public class SipMessageFactory {
    /**
     * The logger
     */
    private static Logger logger = Logger.getLogger(SipMessageFactory.class.getName());

    private static final String CRLF = "\r\n";

    private static final long SECONDS_TO_MILLISECONDS_CONVERSION_RATE = 1000;


    /**
     * Create a SIP REGISTER request
     *
     * @param dialog SIP dialog path
     * @param featureTags Feature tags
     * @param expirePeriod Expiration period
     * @param instanceId UA SIP instance ID
     * @return SIP request
     * @throws SipException
     */
    public static SipRequest createRegister(SipDialogPath dialog, List<String> featureTags,
            int expirePeriod, String instanceId) throws SipException {
        try {
            // Set request line header
            URI requestURI = SipUtils.ADDR_FACTORY.createURI(dialog.getTarget());

            // Set Call-Id header
            CallIdHeader callIdHeader = SipUtils.HEADER_FACTORY.createCallIdHeader(dialog
                    .getCallId());

            // Set the CSeq header
            CSeqHeader cseqHeader = SipUtils.HEADER_FACTORY.createCSeqHeader(dialog.getCseq(),
                    Request.REGISTER);

            // Set the From header
            Address fromAddress = SipUtils.ADDR_FACTORY.createAddress(dialog.getLocalParty());
            FromHeader fromHeader = SipUtils.HEADER_FACTORY.createFromHeader(fromAddress,
                    IdGenerator.getIdentifier());

            // Set the To header
            Address toAddress = SipUtils.ADDR_FACTORY.createAddress(dialog.getRemoteParty());
            ToHeader toHeader = SipUtils.HEADER_FACTORY.createToHeader(toAddress, null);

            // Insert "keep" flag to Via header (RFC6223
            // "Indication of Support for Keep-Alive")
            ArrayList<ViaHeader> viaHeaders = dialog.getSipStack().getViaHeaders();
            if (viaHeaders != null && !viaHeaders.isEmpty()) {
                ViaHeader viaHeader = viaHeaders.get(0);
                viaHeader.setParameter(new NameValue("keep", null, true));
                viaHeader.setParameter(new NameValue("transport=UDP", null, true));
            }

            // Create the request
            Request register = SipUtils.MSG_FACTORY.createRequest(requestURI, Request.REGISTER,
                    callIdHeader, cseqHeader, fromHeader, toHeader, viaHeaders,
                    SipUtils.buildMaxForwardsHeader());

            // Set Contact header
            ContactHeader contact = dialog.getSipStack().getLocalContact();
            if (instanceId != null) {
                contact.setParameter(SipUtils.SIP_INSTANCE_PARAM, instanceId);
            }
            register.addHeader(contact);

            // Set Supported header
            String supported;
            if (instanceId != null) {
                supported ="";
                supported = "path, gruu";
                supported += ", sec-agree";
            } else {
                supported = "path";
            }
            SupportedHeader supportedHeader = SipUtils.HEADER_FACTORY
                    .createSupportedHeader(supported);
            register.addHeader(supportedHeader);

            // Set feature tags
            SipUtils.setContactFeatureTags(register, featureTags);

            if (RcsSettings.getInstance().isCPMBurnAfterReadingSupported()) {
                String ContactheaderString = "";
                ContactheaderString += ""
                        + FeatureTags.FEATURE_CPM_BURNED_MSG;
                ContactHeader contactHeader = (ContactHeader) register.getHeader("Contact");
                contactHeader.setParameter(new NameValue(ContactheaderString, null, true));
            }

            // Set Allow header
            SipUtils.buildAllowHeader(register);

            // Set the Route header
            Vector<String> route = dialog.getSipStack().getDefaultRoutePath();
            for (int i = 0; i < route.size(); i++) {
                Header routeHeader = SipUtils.HEADER_FACTORY.createHeader(RouteHeader.NAME,
                        route.elementAt(i));
                register.addHeader(routeHeader);
            }

            // Set the Expires header
            ExpiresHeader expHeader = SipUtils.HEADER_FACTORY.createExpiresHeader(expirePeriod);
            register.addHeader(expHeader);

            if (RcsSettings.getInstance().supportOP01()) {
                Context context = AndroidFactory.getApplicationContext();
                ConnectivityManager connMgr = (ConnectivityManager)context.getSystemService(
                        Context.CONNECTIVITY_SERVICE);
                NetworkInfo info = connMgr.getActiveNetworkInfo();
                if (info != null && info.isConnected()) {
                    Header header = null;
                    if (ConnectivityManager.isNetworkTypeMobile(info.getType())) {
                        TelephonyManager telMgr = (TelephonyManager)context.getSystemService(
                                Context.TELEPHONY_SERVICE);
                        if (TelephonyManager.NETWORK_CLASS_2_G == TelephonyManager.getNetworkClass(
                                telMgr.getNetworkType(SubscriptionManager.getDefaultDataSubscriptionId()))) {
                            header = SipUtils.buildAccessNetworkInfo("3GPP-GERAN");
                        } else {
                            header = SipUtils.buildAccessNetworkInfo("3GPP-E-UTRAN-TDD");
                        }
                    } else {
                        header = SipUtils.buildAccessNetworkInfo("IEEE-802.11");
                    }
                    register.addHeader(header);
                }
            }

            // Set User-Agent header
            register.addHeader(SipUtils.buildUserAgentHeader());

            // Set "rport" (RFC3581)
            ViaHeader viaHeader = (ViaHeader) register.getHeader(ViaHeader.NAME);
            viaHeader.setRPort();

            return new SipRequest(register);
        } catch (Exception e) {
            if (logger.isActivated()) {
                logger.error("Can't create SIP message", e);
            }
            throw new SipException("Can't create SIP REGISTER message");
        }
    }

    /**
     * Create a SIP SUBSCRIBE request
     *
     * @param dialog SIP dialog path
     * @param expirePeriod Expiration period
     * @return SIP request
     * @throws SipException
     */
    public static SipRequest createSubscribe(SipDialogPath dialog, int expirePeriod)
            throws SipException {
        try {

            if (logger.isActivated()) {
                logger.error("createSubscribe :");
            }

            // Set request line header
            URI requestURI = SipUtils.ADDR_FACTORY.createURI(dialog.getTarget());


            if (logger.isActivated()) {
                logger.error("requestURI  :" + requestURI.toString());
            }

            if (logger.isActivated()) {
                logger.error("dialog.getLocalTag()  :" + dialog.getLocalTag());
                logger.error("dialog.getTarget()  :" + dialog.getTarget());
            }


            // Set Call-Id header
            CallIdHeader callIdHeader = SipUtils.HEADER_FACTORY.createCallIdHeader(
                    dialog.getCallId());

            // Set the CSeq header
            CSeqHeader cseqHeader = SipUtils.HEADER_FACTORY.createCSeqHeader(
                    dialog.getCseq(), Request.SUBSCRIBE);

            // Set the From header
            Address fromAddress = SipUtils.ADDR_FACTORY.createAddress(dialog.getLocalParty());
            FromHeader fromHeader = SipUtils.HEADER_FACTORY.createFromHeader(
                    fromAddress, dialog.getLocalTag());

            // Set the To header
            Address toAddress = SipUtils.ADDR_FACTORY.createAddress(dialog.getRemoteParty());
            ToHeader toHeader = SipUtils.HEADER_FACTORY.createToHeader(
                    toAddress, dialog.getRemoteTag());

            // Create the request
            Request subscribe = SipUtils.MSG_FACTORY.createRequest(
                    requestURI, Request.SUBSCRIBE,
                    callIdHeader, cseqHeader, fromHeader, toHeader,
                    dialog.getSipStack().getViaHeaders(), SipUtils.buildMaxForwardsHeader());

            // Set the Route header
            Vector<String> route = dialog.getRoute();
            for (int i = 0; i < route.size(); i++) {
                Header routeHeader = SipUtils.HEADER_FACTORY.createHeader(
                        RouteHeader.NAME, route.elementAt(i));
                subscribe.addHeader(routeHeader);
            }

            // Set the Expires header
            ExpiresHeader expHeader = SipUtils.HEADER_FACTORY.createExpiresHeader(expirePeriod);
            subscribe.addHeader(expHeader);

            // Set User-Agent header
            subscribe.addHeader(SipUtils.buildUserAgentHeader());

            // Set Contact header
            subscribe.addHeader(dialog.getSipStack().getContact());

            // Set Allow header
            SipUtils.buildAllowHeader(subscribe);

            // Set "rport" (RFC3581)
            ViaHeader viaHeader = (ViaHeader) subscribe.getHeader(ViaHeader.NAME);
            viaHeader.setRPort();

            return new SipRequest(subscribe);
        } catch (Exception e) {
            if (logger.isActivated()) {
                logger.error("Can't create SIP message", e);
            }
            throw new SipException("Can't create SIP SUBSCRIBE message");
        }
    }

    /**
     * Create a Individual SIP SUBSCRIBE request
     *
     * @param dialog SIP dialog path
     * @param expirePeriod Expiration period
     * @return SIP request
     * @throws SipException
     */
    public static SipRequest createIndividualPresenceSubscribe(SipDialogPath dialog, int expirePeriod , String contact) throws SipException {
        try {
            // Set request line header
            String contactUri = PhoneUtils.formatNumberToSipUri(contact);
            URI requestURI = SipUtils.ADDR_FACTORY.createURI(contactUri);

            // Set Call-Id header
            CallIdHeader callIdHeader = SipUtils.HEADER_FACTORY.createCallIdHeader(dialog.getCallId());

            // Set the CSeq header
            CSeqHeader cseqHeader = SipUtils.HEADER_FACTORY.createCSeqHeader(dialog.getCseq(), Request.SUBSCRIBE);

            // Set the From header
            String localParty = "sip:anonymous@anonymous.invalid";
            Address fromAddress = SipUtils.ADDR_FACTORY.createAddress(localParty);
            FromHeader fromHeader = SipUtils.HEADER_FACTORY.createFromHeader(fromAddress, dialog.getLocalTag());

            // Set the To header

            Address toAddress = SipUtils.ADDR_FACTORY.createAddress(contactUri);
            ToHeader toHeader = SipUtils.HEADER_FACTORY.createToHeader(toAddress, null);

            // Create the request
            Request subscribe = SipUtils.MSG_FACTORY.createRequest(requestURI,
                    Request.SUBSCRIBE,
                    callIdHeader,
                    cseqHeader,
                    fromHeader,
                    toHeader,
                    dialog.getSipStack().getViaHeaders(),
                    SipUtils.buildMaxForwardsHeader());

            // Set Allow header
            SipUtils.buildAllowHeader(subscribe);

            //subscribe.addHeader(SipUtils.buildNetworkAccessHeader());

            // Set User-Agent header
            subscribe.addHeader(SipUtils.buildUserAgentHeader());


            // Set the Expires header
            ExpiresHeader expHeader = SipUtils.HEADER_FACTORY.createExpiresHeader(expirePeriod);
            subscribe.addHeader(expHeader);

            Header encondingHeader = SipUtils.HEADER_FACTORY.createHeader("Accept-Encoding", "gzip");
            subscribe.addHeader(encondingHeader);
            // Set the Event header
            Header presenceHeader = SipUtils.HEADER_FACTORY.createHeader(EventHeader.NAME, "presence");
            subscribe.addHeader(presenceHeader);


            Header acceptContactHeader = SipUtils.HEADER_FACTORY.createHeader("Accept-Contact", "*;+g.oma.sip-im;explicit;require");
            subscribe.addHeader(acceptContactHeader);
            //Header acceptContactHeader = HEADER_FACTORY.createHeader();


            if (logger.isActivated()) {
                logger.debug("subscribe request until now " +subscribe);
            }
            //subscribe.addHeader(AcceptHeader.NAME, "application/pidf+xml, application/rlmi+xml, multipart/related");
            String acceptHeaderValue = "application/pidf+xml, application/pidf-diff+xml, multipart/related, application/rlmi+xml, application/simple-filter+xml";
            // Set the Accept header
            Header acceptdHeader = SipUtils.HEADER_FACTORY.createHeader(AcceptHeader.NAME, acceptHeaderValue);
            subscribe.addHeader(acceptdHeader);

            // Set the P-Preferred-Identity header
            if (ImsModule.IMS_USER_PROFILE.getPreferredUri() != null) {
                Header prefHeader = SipUtils.HEADER_FACTORY.createHeader(SipUtils.HEADER_P_PREFERRED_IDENTITY, ImsModule.IMS_USER_PROFILE.getPreferredUri());
                subscribe.addHeader(prefHeader);
            }

         // Set Contact header
            subscribe.addHeader(dialog.getSipStack().getContact());


         // Set Require header
            SipUtils.buildRequireHeader(subscribe);

            // Set the message content length
            int length = 0;
           /* if (xml != null) {
                length = xml.getBytes().length;
            }*/
            /*ContentLengthHeader contentLengthHeader = SipUtils.HEADER_FACTORY.createContentLengthHeader(length);
            subscribe.setContentLength(contentLengthHeader);*/

           // Set the Route header
            Vector<String> route = dialog.getRoute();
            for(int i=0; i < route.size(); i++) {
                Header routeHeader = SipUtils.HEADER_FACTORY.createHeader(RouteHeader.NAME, route.elementAt(i));
                subscribe.addHeader(routeHeader);
            }


            //+g.3gpp.icsi-ref="urn%3Aurn-7%3A3gpp-service.ims.icsi.oma.cpm.session,urn%3Aurn-7%3A3gpp-service.ims.icsi.oma.cpm.filetransfer"

            // Set "rport" (RFC3581)
            ViaHeader viaHeader = (ViaHeader)subscribe.getHeader(ViaHeader.NAME);
            viaHeader.setRPort();

            SipRequest sipRequest = new SipRequest(subscribe);
            sipRequest = (SipRequest) addAccessNetworkInfoHeader(sipRequest);
            return sipRequest;

        } catch(Exception e) {
            if (logger.isActivated()) {
                logger.error("Can't create SIP message", e);
            }
            throw new SipException("Can't create SIP SUBSCRIBE message");
        }
    }




/**
     * Create a SIP SUBSCRIBE request
     *
     * @param dialog SIP dialog path
     * @param expirePeriod Expiration period
     * @return SIP request
     * @throws SipException
     */
    public static SipRequest createPresenceSubscribe(SipDialogPath dialog, int expirePeriod , String xml) throws SipException {
        try {
            // Set request line header
            URI requestURI = SipUtils.ADDR_FACTORY.createURI(dialog.getTarget());

            // Set Call-Id header
            CallIdHeader callIdHeader = SipUtils.HEADER_FACTORY.createCallIdHeader(dialog.getCallId());

            // Set the CSeq header
            CSeqHeader cseqHeader = SipUtils.HEADER_FACTORY.createCSeqHeader(dialog.getCseq(), Request.SUBSCRIBE);

            // Set the From header
            Address fromAddress = SipUtils.ADDR_FACTORY.createAddress(dialog.getLocalParty());
            FromHeader fromHeader = SipUtils.HEADER_FACTORY.createFromHeader(fromAddress, dialog.getLocalTag());

            // Set the To header
            Address toAddress = SipUtils.ADDR_FACTORY.createAddress(dialog.getRemoteParty());
            ToHeader toHeader = SipUtils.HEADER_FACTORY.createToHeader(toAddress, dialog.getRemoteTag());

            // Create the request
            Request subscribe = SipUtils.MSG_FACTORY.createRequest(requestURI,
                    Request.SUBSCRIBE,
                    callIdHeader,
                    cseqHeader,
                    fromHeader,
                    toHeader,
                    dialog.getSipStack().getViaHeaders(),
                    SipUtils.buildMaxForwardsHeader());

            // Set Allow header
            SipUtils.buildAllowHeader(subscribe);

            //subscribe.addHeader(SipUtils.buildNetworkAccessHeader());

            // Set User-Agent header
            subscribe.addHeader(SipUtils.buildUserAgentHeader());

            Header encondingHeader = SipUtils.HEADER_FACTORY.createHeader("Accept-Encoding", "gzip");
            subscribe.addHeader(encondingHeader);

            // Set the Accept header
            Header dispositionHeader = SipUtils.HEADER_FACTORY.createHeader("Content-Disposition", "recipient-list");
            subscribe.addHeader(dispositionHeader);

            // Set the Accept header
            //Header acceptHeader = SipUtils.HEADER_FACTORY.createHeader(AcceptHeader.NAME, "application/pidf+xml");
            //subscribe.addHeader(acceptHeader);

            // Set the Expires header
            ExpiresHeader expHeader = SipUtils.HEADER_FACTORY.createExpiresHeader(expirePeriod);
            subscribe.addHeader(expHeader);

            // Set the Event header
            Header presenceHeader = SipUtils.HEADER_FACTORY.createHeader(EventHeader.NAME, "presence");
            subscribe.addHeader(presenceHeader);

            Header supportedHeader = SipUtils.HEADER_FACTORY.createHeader(SupportedHeader.NAME, "eventlist");
            subscribe.addHeader(supportedHeader);

            Header acceptContactHeader = SipUtils.HEADER_FACTORY.createHeader("Accept-Contact", "*;+g.oma.sip-im;explicit;require");
            subscribe.addHeader(acceptContactHeader);
            //Header acceptContactHeader = HEADER_FACTORY.createHeader();


            if (logger.isActivated()) {
                logger.debug("subscribe request until now " +subscribe);
            }
            //subscribe.addHeader(AcceptHeader.NAME, "application/pidf+xml, application/rlmi+xml, multipart/related");
            String acceptHeaderValue = "application/pidf+xml, application/pidf-diff+xml, multipart/related, application/rlmi+xml, application/simple-filter+xml";
            // Set the Accept header
            Header acceptdHeader = SipUtils.HEADER_FACTORY.createHeader(AcceptHeader.NAME, acceptHeaderValue);
            subscribe.addHeader(acceptdHeader);

            // Set the P-Preferred-Identity header
            if (ImsModule.IMS_USER_PROFILE.getPreferredUri() != null) {
                Header prefHeader = SipUtils.HEADER_FACTORY.createHeader(SipUtils.HEADER_P_PREFERRED_IDENTITY, ImsModule.IMS_USER_PROFILE.getPreferredUri());
                subscribe.addHeader(prefHeader);
            }

         // Set Contact header
            subscribe.addHeader(dialog.getSipStack().getContact());
            List<String> tags = new ArrayList<String>();
            String additionalRcseTags = "";
            additionalRcseTags += FeatureTags.FEATURE_CPM_SESSION + ",";
            additionalRcseTags += FeatureTags.FEATURE_CPM_FT + ",";

            if (additionalRcseTags.length() > 0) {
                if (additionalRcseTags.endsWith(",")) {
                    additionalRcseTags = additionalRcseTags.substring(0, additionalRcseTags.length()-1);
                }
                tags.add(FeatureTags.FEATURE_CPM_RCSE + "=\"" + additionalRcseTags + "\"");
            }
         // Update Contact header
            ContactHeader contact = (ContactHeader)subscribe.getHeader(ContactHeader.NAME);
            for(int i=0; i < tags.size(); i++) {
                if (contact != null) {
                    contact.setParameter(tags.get(i), null);
                }
            }


         // Set Require header
            SipUtils.buildPresenceRequireHeader(subscribe);



            if (logger.isActivated()) {
                logger.debug("Subscribe xml content : " + xml);
            }

         // Set the message content
            if (xml != null) {
                ContentTypeHeader contentTypeHeader = SipUtils.HEADER_FACTORY.createContentTypeHeader("application", "resource-lists+xml");
                subscribe.setContent(xml, contentTypeHeader);
            }

            // Set the message content length
            int length = 0;
            if (xml != null) {
                length = xml.getBytes().length;
            }
            ContentLengthHeader contentLengthHeader = SipUtils.HEADER_FACTORY.createContentLengthHeader(length);
            subscribe.setContentLength(contentLengthHeader);

           // Set the Route header
            Vector<String> route = dialog.getRoute();
            for(int i=0; i < route.size(); i++) {
                Header routeHeader = SipUtils.HEADER_FACTORY.createHeader(RouteHeader.NAME, route.elementAt(i));
                subscribe.addHeader(routeHeader);
            }


            //+g.3gpp.icsi-ref="urn%3Aurn-7%3A3gpp-service.ims.icsi.oma.cpm.session,urn%3Aurn-7%3A3gpp-service.ims.icsi.oma.cpm.filetransfer"

            // Set "rport" (RFC3581)
            ViaHeader viaHeader = (ViaHeader)subscribe.getHeader(ViaHeader.NAME);
            viaHeader.setRPort();


             /**
             * M:add P-Access-Network-Info and P-Last-Access-Network-Info
             * Headers. @{T-Mobile
             */
            SipRequest sipRequest = new SipRequest(subscribe);
            sipRequest = (SipRequest) addAccessNetworkInfoHeader(sipRequest);
            return sipRequest;
            /**
             * T-Mobile@}
             */
        } catch(Exception e) {
            if (logger.isActivated()) {
                logger.error("Can't create SIP message", e);
            }
            throw new SipException("Can't create SIP SUBSCRIBE message");
        }
    }

    /**
     * Create a SIP SUBSCRIBE request
     *
     * @param dialog SIP dialog path
     * @param expirePeriod Expiration period
     * @return SIP request
     * @throws SipException
     */
    public static SipRequest createCpimSubscribe(SipDialogPath dialog, int expirePeriod)
            throws SipException {

        return createCpimSubscribe(dialog, expirePeriod, null, null);
    }

    public static SipRequest createCpimSubscribe(
            SipDialogPath dialog, int expirePeriod, String contentType, byte[] content)
            throws SipException {
        try {
            if (logger.isActivated()) {
                logger.info("ABCH createCpimSubscribe: target" + dialog.getTarget());
            }

            // Set request line header
            URI requestURI = SipUtils.ADDR_FACTORY.createURI(dialog.getTarget());

            // Set Call-Id header
            CallIdHeader callIdHeader = SipUtils.HEADER_FACTORY.createCallIdHeader(
                    dialog.getCallId());

            // Set the CSeq header
            CSeqHeader cseqHeader = SipUtils.HEADER_FACTORY.createCSeqHeader(
                    dialog.getCseq(), Request.SUBSCRIBE);

            // Set the From header
            Address fromAddress = SipUtils.ADDR_FACTORY.createAddress(dialog.getLocalParty());
            FromHeader fromHeader = SipUtils.HEADER_FACTORY.createFromHeader(
                    fromAddress, dialog.getLocalTag());

            // Set the To header
            Address toAddress = SipUtils.ADDR_FACTORY.createAddress(dialog.getRemoteParty());
            ToHeader toHeader = SipUtils.HEADER_FACTORY.createToHeader(
                    toAddress, dialog.getRemoteTag());

            // Create the request
            Request subscribe = SipUtils.MSG_FACTORY.createRequest(
                    requestURI, Request.SUBSCRIBE,
                    callIdHeader, cseqHeader, fromHeader, toHeader,
                    dialog.getSipStack().getViaHeaders(), SipUtils.buildMaxForwardsHeader());

            // Set the Route header
            Vector<String> route = dialog.getRoute();
            for (int i = 0; i < route.size(); i++) {
                Header routeHeader = SipUtils.HEADER_FACTORY.createHeader(
                        RouteHeader.NAME, route.elementAt(i));
                subscribe.addHeader(routeHeader);
            }

            try {
                Header allowEvents;
                if (RcsSettings.getInstance().supportOP01())
                    allowEvents = SipUtils.HEADER_FACTORY
                        .createAllowEventsHeader("grouplist,presence,presence.winfo");
                else
                    allowEvents = SipUtils.HEADER_FACTORY
                    .createAllowEventsHeader("presence,presence.winfo");
                subscribe.addHeader(allowEvents);
            } catch (Exception e) {
                e.printStackTrace();
            }

            // Set the Expires header
            ExpiresHeader expHeader = SipUtils.HEADER_FACTORY.createExpiresHeader(expirePeriod);
            subscribe.addHeader(expHeader);

            // Set User-Agent header
            subscribe.addHeader(SipUtils.buildCpimUserAgentHeader());

            // Set Contact header
            subscribe.addHeader(dialog.getSipStack().getContact());

            // Set Allow header
            SipUtils.buildAllowHeader(subscribe);

            // Set "rport" (RFC3581)
            ViaHeader viaHeader = (ViaHeader) subscribe.getHeader(ViaHeader.NAME);
            viaHeader.setRPort();

            ContentLengthHeader header;
            if (content != null) {
                header = SipUtils.HEADER_FACTORY.createContentLengthHeader(content.length);
                String[] subTypes = contentType.split("/");
                ContentTypeHeader typeHeader = SipUtils.HEADER_FACTORY
                        .createContentTypeHeader(subTypes[0], subTypes[1]);
                subscribe.setContent(content, typeHeader);
            }
            else {
                header = SipUtils.HEADER_FACTORY.createContentLengthHeader(0);
            }
            subscribe.setContentLength(header);

            return new SipRequest(subscribe);
        } catch (Exception e) {
            if (logger.isActivated()) {
                logger.error("Can't create SIP message", e);
            }
            throw new SipException("Can't create SIP SUBSCRIBE message");
        }
    }

    /**
     * Create a SIP MESSAGE request with a feature tag
     *
     * @param dialog SIP dialog path
     * @param contentType Content type
     * @param content Content
     * @return SIP request
     * @throws SipException
     */
    public static SipRequest createMessage(SipDialogPath dialog, String contentType, String content)
            throws SipException {
        return createMessage(dialog, null, contentType, content.getBytes());
    }

    /**
     * Create a SIP MESSAGE request with a feature tag
     *
     * @param dialog SIP dialog path
     * @param contentType Content type
     * @param content Content
     * @return SIP request
     * @throws SipException
     */
    public static SipRequest createCpimMessage(SipDialogPath dialog, String contentType,
            String content) throws SipException {
        return createMessage(dialog, null, contentType, content.getBytes());
    }

    /**
     * Create a SIP MESSAGE request with a feature tag
     *
     * @param dialog SIP dialog path
     * @param featureTag Feature tag
     * @param contentType Content type
     * @param content Content
     * @return SIP request
     * @throws SipException
     */
    public static SipRequest createMessage(SipDialogPath dialog, String featureTag,
            String contentType, byte[] content) throws SipException {
        try {
            if (logger.isActivated()) {
                logger.info("ABC createMessage: target" + dialog.getTarget());
            }

            // Set request line header
            URI requestURI = SipUtils.ADDR_FACTORY.createURI(dialog.getTarget());

            // Set Call-Id header
            CallIdHeader callIdHeader = SipUtils.HEADER_FACTORY.createCallIdHeader(dialog
                    .getCallId());

            // Set the CSeq header
            CSeqHeader cseqHeader = SipUtils.HEADER_FACTORY.createCSeqHeader(dialog.getCseq(),
                    Request.MESSAGE);

            // Set the From header
            Address fromAddress = SipUtils.ADDR_FACTORY.createAddress(dialog.getLocalParty());
            FromHeader fromHeader = SipUtils.HEADER_FACTORY.createFromHeader(fromAddress,
                    dialog.getLocalTag());

            // Set the To header
            Address toAddress = SipUtils.ADDR_FACTORY.createAddress(dialog.getRemoteParty());
            ToHeader toHeader = SipUtils.HEADER_FACTORY.createToHeader(toAddress,
                    dialog.getRemoteTag());

            // Create the request
            Request message = SipUtils.MSG_FACTORY.createRequest(requestURI, Request.MESSAGE,
                    callIdHeader, cseqHeader, fromHeader, toHeader, dialog.getSipStack()
                            .getViaHeaders(), SipUtils.buildMaxForwardsHeader());

            // Set the Route header
            Vector<String> route = dialog.getRoute();
            for (int i = 0; i < route.size(); i++) {
                Header routeHeader = SipUtils.HEADER_FACTORY.createHeader(RouteHeader.NAME,
                        route.elementAt(i));
                message.addHeader(routeHeader);
            }

            // Set the P-Preferred-Identity header
            if (ImsModule.IMS_USER_PROFILE.getPreferredUri() != null) {
                Header prefHeader = SipUtils.HEADER_FACTORY.createHeader(
                        SipUtils.HEADER_P_PREFERRED_IDENTITY,
                        ImsModule.IMS_USER_PROFILE.getPreferredUri());
                message.addHeader(prefHeader);
            }

            // Set Contact header
            message.addHeader(dialog.getSipStack().getContact());

            // Set User-Agent header
            message.addHeader(SipUtils.buildUserAgentHeader());

            // Set feature tags
            if (featureTag != null) {
                SipUtils.setFeatureTags(message, new String[] { featureTag });
            }

            // Set the message content
            String[] type = contentType.split("/");
            ContentTypeHeader contentTypeHeader = SipUtils.HEADER_FACTORY.createContentTypeHeader(
                    type[0], type[1]);
            message.setContent(content, contentTypeHeader);

            // Set the message content length
            ContentLengthHeader contentLengthHeader = SipUtils.HEADER_FACTORY
                    .createContentLengthHeader(content.length);
            message.setContentLength(contentLengthHeader);

            // Set "rport" (RFC3581)
            ViaHeader viaHeader = (ViaHeader) message.getHeader(ViaHeader.NAME);
            viaHeader.setRPort();

            // Add remote SIP instance ID
            SipUtils.setRemoteInstanceID(message, dialog.getRemoteSipInstance());

            return new SipRequest(message);
        } catch (Exception e) {
            if (logger.isActivated()) {
                logger.error("Can't create SIP message", e);
            }
            throw new SipException("Can't create SIP MESSAGE message");
        }
    }

    /**
     * Create a SIP SUBSCRIBE request
     *
     * @param dialog SIP dialog path
     * @param expirePeriod Expiration period
     * @return SIP request
     * @throws SipException
     */
    public static SipRequest createCpimChatListSubscribe(SipDialogPath dialog, int expirePeriod,
            int version) throws SipException {
        try {
            if (logger.isActivated()) {
                logger.info("ABCH createCpimChatListSubscribe: target" + dialog.getTarget());
            }

            String[] featureTags = InstantMessagingService.CPM_CHAT_FEATURE_TAGS;

            // Set request line header
            URI requestURI = SipUtils.ADDR_FACTORY.createURI(dialog.getTarget());

            // Set Call-Id header
            CallIdHeader callIdHeader = SipUtils.HEADER_FACTORY.createCallIdHeader(
                    dialog.getCallId());

            // Set the CSeq header
            CSeqHeader cseqHeader = SipUtils.HEADER_FACTORY.createCSeqHeader(
                    dialog.getCseq(), Request.SUBSCRIBE);

            // Set the From header
            Address fromAddress = SipUtils.ADDR_FACTORY.createAddress(dialog.getLocalParty());
            FromHeader fromHeader = SipUtils.HEADER_FACTORY.createFromHeader(
                    fromAddress, dialog.getLocalTag());

            // Set the To header
            Address toAddress = SipUtils.ADDR_FACTORY.createAddress(dialog.getRemoteParty());
            ToHeader toHeader = SipUtils.HEADER_FACTORY.createToHeader(
                    toAddress, dialog.getRemoteTag());

            // Create the request
            Request subscribe = SipUtils.MSG_FACTORY.createRequest(
                    requestURI, Request.SUBSCRIBE,
                    callIdHeader, cseqHeader, fromHeader, toHeader,
                    dialog.getSipStack().getViaHeaders(),
                    SipUtils.buildMaxForwardsHeader());

            // Set the Route header
            Vector<String> route = dialog.getRoute();
            for (int i = 0; i < route.size(); i++) {
                Header routeHeader = SipUtils.HEADER_FACTORY.createHeader(
                        RouteHeader.NAME, route.elementAt(i));
                subscribe.addHeader(routeHeader);
            }

            // Set the Expires header
            ExpiresHeader expHeader = SipUtils.HEADER_FACTORY.createExpiresHeader(expirePeriod);
            subscribe.addHeader(expHeader);

            // Set User-Agent header
            subscribe.addHeader(SipUtils.buildCpimUserAgentHeader());

            // Set Contact header
            subscribe.addHeader(dialog.getSipStack().getContact());

            // Set feature tags
            SipUtils.setFeatureTags(subscribe, featureTags);

            // subscribe.addHeader(EventHeader.NAME, "grouplist");
            Header eventHeader = SipUtils.HEADER_FACTORY.createHeader(
                    EventHeader.NAME, "grouplist");
            subscribe.addHeader(eventHeader);

            // Set the Accept header
            // subscribe.addHeader(AcceptHeader.NAME, "application/conference-info+xml");
            Header acceptHeader = SipUtils.HEADER_FACTORY.createHeader(
                    AcceptHeader.NAME, "application/conference-info+xml");
            subscribe.addHeader(acceptHeader);

            // Set Allow header
            SipUtils.buildAllowHeader(subscribe);

            String document = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" + SipUtils.CRLF
                    + "<grouplist-ver version=\"" + version + "\">" + SipUtils.CRLF
                    + "</grouplist-ver>";

            // Set "rport" (RFC3581)
            ViaHeader viaHeader = (ViaHeader) subscribe.getHeader(ViaHeader.NAME);
            viaHeader.setRPort();

            // Set the message content length
            int length = 0;
            if (document != null) {
                length = document.getBytes().length;
            }
            ContentLengthHeader contentLengthHeader = SipUtils.HEADER_FACTORY
                    .createContentLengthHeader(length);
            subscribe.setContentLength(contentLengthHeader);

            // Set the message content
            if (document != null) {
                ContentTypeHeader contentTypeHeader = SipUtils.HEADER_FACTORY
                        .createContentTypeHeader("application", "grouplist-ver+xml");
                subscribe.setContent(document, contentTypeHeader);
            }

            return new SipRequest(subscribe);
        } catch (Exception e) {
            if (logger.isActivated()) {
                logger.error("Can't create SIP message", e);
            }
            throw new SipException("Can't create SIP SUBSCRIBE message");
        }
    }

    /**
     * Create a SIP MESSAGE request with a feature tag
     *
     * @param dialog SIP dialog path
     * @param featureTag Feature tag
     * @param contentType Content type
     * @param content Content
     * @return SIP request
     * @throws SipException
     */
    public static SipRequest createMessageCPM(
            SipDialogPath dialog,
            String[] featureTags,
            String preferService,
            String contentType,
            String content) throws SipException {
        try {
            // Set request line header
            URI requestURI = SipUtils.ADDR_FACTORY.createURI(dialog.getTarget());

            // Set Call-Id header
            CallIdHeader callIdHeader = SipUtils.HEADER_FACTORY.createCallIdHeader(
                    dialog.getCallId());

            // Set the CSeq header
            CSeqHeader cseqHeader = SipUtils.HEADER_FACTORY.createCSeqHeader(
                    dialog.getCseq(), Request.MESSAGE);

            // Set the From header
            Address fromAddress = SipUtils.ADDR_FACTORY.createAddress(dialog.getLocalParty());
            FromHeader fromHeader = SipUtils.HEADER_FACTORY.createFromHeader(
                    fromAddress, dialog.getLocalTag());

            // Set the To header
            Address toAddress = SipUtils.ADDR_FACTORY.createAddress(dialog.getRemoteParty());
            ToHeader toHeader = SipUtils.HEADER_FACTORY.createToHeader(
                    toAddress, dialog.getRemoteTag());

            // Create the request
            Request message = SipUtils.MSG_FACTORY.createRequest(
                    requestURI, Request.MESSAGE,
                    callIdHeader, cseqHeader, fromHeader, toHeader,
                    dialog.getSipStack().getViaHeaders(),
                    SipUtils.buildMaxForwardsHeader());

            // Set the Route header
            Vector<String> route = dialog.getRoute();
            for (int i = 0; i < route.size(); i++) {
                Header routeHeader = SipUtils.HEADER_FACTORY.createHeader(
                        RouteHeader.NAME, route.elementAt(i));
                message.addHeader(routeHeader);
            }

            // Set the P-Preferred-Identity header
            if (ImsModule.IMS_USER_PROFILE.getPreferredUri() != null) {
                Header prefHeader = SipUtils.HEADER_FACTORY.createHeader(
                        SipUtils.HEADER_P_PREFERRED_IDENTITY,
                        ImsModule.IMS_USER_PROFILE.getPreferredUri());
                message.addHeader(prefHeader);
            }

            // Set the preferred service header
            Header preferredServiceHeader;
            if (preferService == null)
                preferredServiceHeader = SipUtils.HEADER_FACTORY.createHeader(
                    SipUtils.HEADER_P_PREFERRED_SERVICE, FeatureTags.FEATURE_CPM_URN_MSG);
            else
                preferredServiceHeader = SipUtils.HEADER_FACTORY.createHeader(
                        SipUtils.HEADER_P_PREFERRED_SERVICE, preferService);
            message.addHeader(preferredServiceHeader);

            // Set Contact header
            message.addHeader(dialog.getSipStack().getContact());

            // Set User-Agent header
            message.addHeader(SipUtils.buildUserAgentHeader());

            // Set feature tags
            if (featureTags != null) {
                SipUtils.setFeatureTags(message, featureTags);
            } else {
                SipUtils.setFeatureTags(message, new String[] {FeatureTags.FEATURE_RCSE_PAGER_MSG});
            }

            // Set the message content
            String[] type = contentType.split("/");
            ContentTypeHeader contentTypeHeader = SipUtils.HEADER_FACTORY.createContentTypeHeader(
                    type[0], type[1]);
            message.setContent(content, contentTypeHeader);

            // Set the message content length
            ContentLengthHeader contentLengthHeader = SipUtils.HEADER_FACTORY
                    .createContentLengthHeader(content.getBytes().length);
            message.setContentLength(contentLengthHeader);

            // Set "rport" (RFC3581)
            ViaHeader viaHeader = (ViaHeader) message.getHeader(ViaHeader.NAME);
            viaHeader.setRPort();

            // Add remote SIP instance ID
            SipUtils.setRemoteInstanceID(message, dialog.getRemoteSipInstance());

            // Set PAccessNetworkInfo header
            SipRequest req  = new SipRequest(message);
            addAccessNetworkInfoHeader(req);

            return req;
        } catch (Exception e) {
            if (logger.isActivated()) {
                logger.error("Can't create SIP message", e);
            }
            throw new SipException("Can't create SIP MESSAGE message");
        }
    }

    /**
     * Create a SIP MESSAGE request with a feature tag
     *
     * @param dialog SIP dialog path
     * @param featureTag Feature tag
     * @param contentType Content type
     * @param content Content
     * @return SIP request
     * @throws SipException
     */
    public static SipRequest createMessageMultiCPM(
            SipDialogPath dialog, String featureTag, String contentType, String content)
                    throws SipException {
        try {
            // Set request line header
            URI requestURI = SipUtils.ADDR_FACTORY.createURI(dialog.getTarget());

            // Set Call-Id header
            CallIdHeader callIdHeader = SipUtils.HEADER_FACTORY.createCallIdHeader(
                    dialog.getCallId());

            // Set the CSeq header
            CSeqHeader cseqHeader = SipUtils.HEADER_FACTORY.createCSeqHeader(
                    dialog.getCseq(), Request.MESSAGE);

            // Set the Require header
            RequireHeader requireHeader = SipUtils.HEADER_FACTORY
                    .createRequireHeader("recipient-list-message");

            // Set the From header
            Address fromAddress = SipUtils.ADDR_FACTORY.createAddress(dialog.getLocalParty());
            FromHeader fromHeader = SipUtils.HEADER_FACTORY.createFromHeader(
                    fromAddress, dialog.getLocalTag());

            // Set the To header
            Address toAddress = SipUtils.ADDR_FACTORY.createAddress(dialog.getRemoteParty());
            ToHeader toHeader = SipUtils.HEADER_FACTORY.createToHeader(
                    toAddress, dialog.getRemoteTag());

            // Create the request
            Request message = SipUtils.MSG_FACTORY.createRequest(
                    requestURI, Request.MESSAGE,
                    callIdHeader, cseqHeader, fromHeader, toHeader,
                    dialog.getSipStack().getViaHeaders(),
                    SipUtils.buildMaxForwardsHeader());

            // Set the Route header
            Vector<String> route = dialog.getRoute();
            for (int i = 0; i < route.size(); i++) {
                Header routeHeader = SipUtils.HEADER_FACTORY.createHeader(
                        RouteHeader.NAME, route.elementAt(i));
                message.addHeader(routeHeader);
            }

            message.addHeader(requireHeader);

            // Set the P-Preferred-Identity header
            if (ImsModule.IMS_USER_PROFILE.getPreferredUri() != null) {
                Header prefHeader = SipUtils.HEADER_FACTORY.createHeader(
                        SipUtils.HEADER_P_PREFERRED_IDENTITY,
                        ImsModule.IMS_USER_PROFILE.getPreferredUri());
                message.addHeader(prefHeader);
            }

            // Set the preferred service header
            Header preferredServiceHeader = SipUtils.HEADER_FACTORY.createHeader(
                    SipUtils.HEADER_P_PREFERRED_SERVICE, FeatureTags.FEATURE_CPM_URN_MSG);
            message.addHeader(preferredServiceHeader);

            // Set Contact header
            message.addHeader(dialog.getSipStack().getContact());

            // Set User-Agent header
            message.addHeader(SipUtils.buildUserAgentHeader());

            // Set feature tags
            if (featureTag != null) {
                SipUtils.setFeatureTags(message, new String[] { featureTag });
            }

            // Set the message content
            String[] type = contentType.split("/");
            ContentTypeHeader contentTypeHeader = SipUtils.HEADER_FACTORY.createContentTypeHeader(
                    type[0], type[1]);
            message.setContent(content, contentTypeHeader);

            // Set the message content length
            ContentLengthHeader contentLengthHeader = SipUtils.HEADER_FACTORY
                    .createContentLengthHeader(content.getBytes().length);
            message.setContentLength(contentLengthHeader);

            // Set "rport" (RFC3581)
            ViaHeader viaHeader = (ViaHeader) message.getHeader(ViaHeader.NAME);
            viaHeader.setRPort();

            // Add remote SIP instance ID
            SipUtils.setRemoteInstanceID(message, dialog.getRemoteSipInstance());

            return new SipRequest(message);
        } catch (Exception e) {
            if (logger.isActivated()) {
                logger.error("Can't create SIP message", e);
            }
            throw new SipException("Can't create SIP MESSAGE message");
        }
    }

    /**
     * Create a SIP MESSAGE request with a feature tag
     *
     * @param dialog SIP dialog path
     * @param featureTag Feature tag
     * @param contentType Content type
     * @param content Content
     * @return SIP request
     * @throws SipException
     */
    public static SipRequest createCpimMessage(SipDialogPath dialog, String featureTag,
            String contentType, byte[] content) throws SipException {
        try {
            if (logger.isActivated()) {
                logger.info("ABCH createCpimMessage: target" + dialog.getTarget());
            }

            // Set request line header
            URI requestURI = SipUtils.ADDR_FACTORY.createURI(dialog.getTarget());

            // Set Call-Id header
            CallIdHeader callIdHeader = SipUtils.HEADER_FACTORY.createCallIdHeader(dialog
                    .getCallId());

            // Set the CSeq header
            CSeqHeader cseqHeader = SipUtils.HEADER_FACTORY.createCSeqHeader(dialog.getCseq(),
                    Request.MESSAGE);

            // Set the From header
            Address fromAddress = SipUtils.ADDR_FACTORY.createAddress(dialog.getLocalParty());
            FromHeader fromHeader = SipUtils.HEADER_FACTORY.createFromHeader(fromAddress,
                    dialog.getLocalTag());

            // Set the To header
            Address toAddress = SipUtils.ADDR_FACTORY.createAddress(dialog.getRemoteParty());
            ToHeader toHeader = SipUtils.HEADER_FACTORY.createToHeader(toAddress,
                    dialog.getRemoteTag());

            // Create the request
            Request message = SipUtils.MSG_FACTORY.createRequest(requestURI, Request.MESSAGE,
                    callIdHeader, cseqHeader, fromHeader, toHeader, dialog.getSipStack()
                            .getViaHeaders(), SipUtils.buildMaxForwardsHeader());

            // Set the Route header
            Vector<String> route = dialog.getRoute();
            for (int i = 0; i < route.size(); i++) {
                Header routeHeader = SipUtils.HEADER_FACTORY.createHeader(RouteHeader.NAME,
                        route.elementAt(i));
                message.addHeader(routeHeader);
            }

            // Set the P-Preferred-Identity header
            if (ImsModule.IMS_USER_PROFILE.getPreferredUri() != null) {
                Header prefHeader = SipUtils.HEADER_FACTORY.createHeader(
                        SipUtils.HEADER_P_PREFERRED_IDENTITY,
                        ImsModule.IMS_USER_PROFILE.getPreferredUri());
                message.addHeader(prefHeader);
            } else{
                Header prefHeader = SipUtils.HEADER_FACTORY.createHeader(
                        SipUtils.HEADER_P_PREFERRED_IDENTITY,
                        dialog.getLocalParty());
                message.addHeader(prefHeader);
            }

            // Set Allow header
            SipUtils.buildAllowHeader(message);

            // Set Contact header
            message.addHeader(dialog.getSipStack().getContact());

            // Set User-Agent header
            message.addHeader(SipUtils.buildCpimUserAgentHeader());

            // Set feature tags
            if (featureTag != null) {
                SipUtils.setFeatureTags(message, new String[] { featureTag });
            }

            // Set the message content
            String[] type = contentType.split("/");
            ContentTypeHeader contentTypeHeader = SipUtils.HEADER_FACTORY.createContentTypeHeader(
                    type[0], type[1]);
            message.setContent(content, contentTypeHeader);

            // Set the message content length
            ContentLengthHeader contentLengthHeader = SipUtils.HEADER_FACTORY
                    .createContentLengthHeader(content.length);
            message.setContentLength(contentLengthHeader);

            // Set the preferred service header
            Header preferredServiceHeader = SipUtils.HEADER_FACTORY.createHeader(
                    SipUtils.HEADER_P_PREFERRED_SERVICE, FeatureTags.FEATURE_CPM_URN_MSG);
            message.addHeader(preferredServiceHeader);

            // Set "rport" (RFC3581)
            ViaHeader viaHeader = (ViaHeader) message.getHeader(ViaHeader.NAME);
            viaHeader.setRPort();

            // Add remote SIP instance ID
            SipUtils.setRemoteInstanceID(message, dialog.getRemoteSipInstance());

            return new SipRequest(message);
        } catch (Exception e) {
            if (logger.isActivated()) {
                logger.error("Can't create SIP message", e);
            }
            throw new SipException("Can't create SIP MESSAGE message");
        }
    }

    /**
     * Create a SIP PUBLISH request
     *
     * @param dialog SIP dialog path
     * @param expirePeriod Expiration period
     * @param entityTag Entity tag
     * @param sdp SDP part
     * @return SIP request
     * @throws SipException
     */
    public static SipRequest createPublish(SipDialogPath dialog, int expirePeriod,
            String entityTag, String sdp) throws SipException {
        try {
            // Set request line header
            URI requestURI = SipUtils.ADDR_FACTORY.createURI(dialog.getTarget());

            // Set Call-Id header
            CallIdHeader callIdHeader = SipUtils.HEADER_FACTORY.createCallIdHeader(
                    dialog.getCallId());

            // Set the CSeq header
            CSeqHeader cseqHeader = SipUtils.HEADER_FACTORY.createCSeqHeader(
                    dialog.getCseq(), Request.PUBLISH);

            // Set the From header
            Address fromAddress = SipUtils.ADDR_FACTORY.createAddress(dialog.getLocalParty());
            FromHeader fromHeader = SipUtils.HEADER_FACTORY.createFromHeader(
                    fromAddress, dialog.getLocalTag());

            // Set the To header
            Address toAddress = SipUtils.ADDR_FACTORY.createAddress(dialog.getRemoteParty());
            ToHeader toHeader = SipUtils.HEADER_FACTORY.createToHeader(
                    toAddress, dialog.getRemoteTag());

            // Create the request
            Request publish = SipUtils.MSG_FACTORY.createRequest(requestURI, Request.PUBLISH,
                    callIdHeader, cseqHeader, fromHeader, toHeader, dialog.getSipStack()
                            .getViaHeaders(), SipUtils.buildMaxForwardsHeader());

           String instanceId = DeviceUtils.getInstanceId(AndroidFactory.getApplicationContext());
            // Set the Route header
            Vector<String> route = dialog.getRoute();
            for (int i = 0; i < route.size(); i++) {
                Header routeHeader = SipUtils.HEADER_FACTORY.createHeader(RouteHeader.NAME,
                        route.elementAt(i));
                publish.addHeader(routeHeader);
            }

            // Set the Expires header
            ExpiresHeader expHeader = SipUtils.HEADER_FACTORY.createExpiresHeader(expirePeriod);
            publish.addHeader(expHeader);

            // Set the SIP-If-Match header
            if (entityTag != null) {
                Header sipIfMatchHeader = SipUtils.HEADER_FACTORY.createHeader(
                        SIPIfMatchHeader.NAME, entityTag);
                publish.addHeader(sipIfMatchHeader);
            }

            // Set User-Agent header
            publish.addHeader(SipUtils.buildUserAgentHeader());

            // Set the Event header
            publish.addHeader(SipUtils.HEADER_FACTORY.createHeader(EventHeader.NAME, "presence"));

            // Set the message content
            if (sdp != null) {
                ContentTypeHeader contentTypeHeader = SipUtils.HEADER_FACTORY
                        .createContentTypeHeader("application", "pidf+xml");
                publish.setContent(sdp, contentTypeHeader);
            }

            String acceptHeaderValue = "application/pidf+xml";
            Header acceptdHeader = SipUtils.HEADER_FACTORY.createHeader(AcceptHeader.NAME, acceptHeaderValue);
            publish.addHeader(acceptdHeader);


            /*
             *
             * CONTACT HEADER STARTS
             */
            // Set Contact header

            if (logger.isActivated()) {
                logger.info("dialog.getSipStack().getContact() : " +dialog.getSipStack().getContact());
            }
            publish.addHeader(dialog.getSipStack().getContact());
            List<String> tags = new ArrayList<String>();
            String additionalRcseTags = "";
            additionalRcseTags += FeatureTags.FEATURE_CPM_SESSION + ",";
            additionalRcseTags += FeatureTags.FEATURE_CPM_FT + ",";

            if (additionalRcseTags.length() > 0) {
                if (additionalRcseTags.endsWith(",")) {
                    additionalRcseTags = additionalRcseTags.substring(0, additionalRcseTags.length()-1);
                }
                tags.add(FeatureTags.FEATURE_CPM_RCSE + "=\"" + additionalRcseTags + "\"");
            }
         // Update Contact header
            /*
            ContactHeader contact = (ContactHeader)publish.getHeader(ContactHeader.NAME);
            for(int i=0; i < tags.size(); i++) {
                if (contact != null) {
                    //contact.setParameter(tags.get(i), null);
                }
            }*/

            /**
             * CONTACT HEADER ENDS
             */

            /*
             * PROXY-REQUIRE  / REQUIRE STARTS
             */

         // Set Require header
            SipUtils.buildRequireHeader(publish);
            /*
             * PROXY-REQUIRE  / REQUIRE ENDS
             */


            // Set the message content length
            int length = 0;
            if (sdp != null) {
                length = sdp.getBytes().length;
            }
            ContentLengthHeader contentLengthHeader = SipUtils.HEADER_FACTORY
                    .createContentLengthHeader(length);
            publish.setContentLength(contentLengthHeader);

            // Set "rport" (RFC3581)
            ViaHeader viaHeader = (ViaHeader) publish.getHeader(ViaHeader.NAME);
            viaHeader.setRPort();

            return new SipRequest(publish);
        } catch (Exception e) {
            if (logger.isActivated()) {
                logger.error("Can't create SIP message", e);
            }
            throw new SipException("Can't create SIP PUBLISH message");
        }
    }

    /**
     * Create a SIP INVITE request
     *
     * @param dialog SIP dialog path
     * @param featureTags Feature tags
     * @param sdp SDP part
     * @return SIP request
     * @throws SipException
     */
    public static SipRequest createCpmInvite(SipDialogPath dialog, String[] featureTags, String sdp,boolean isGroup)
            throws SipException {
        try {
            if (logger.isActivated()) {
                logger.info("CPM createCpmInvite target: " + dialog.getTarget());
            }
            // Create the content type
            ContentTypeHeader contentType = SipUtils.HEADER_FACTORY.createContentTypeHeader(
                    "application", "sdp");

            // Create the request
            return createCpimInvite(dialog, featureTags, sdp, contentType, isGroup);
        } catch (Exception e) {
            if (logger.isActivated()) {
                logger.error("Can't create SIP message", e);
            }
            throw new SipException("Can't create SIP INVITE message");
        }
    }

    /**
     * Create a SIP INVITE request
     *
     * @param dialog SIP dialog path
     * @param featureTags Feature tags
     * @param sdp SDP part
     * @return SIP request
     * @throws SipException
     */
    public static SipRequest createLargeCpmInvite(
                    SipDialogPath dialog,
                    String[] featureTags,
                    String preferService,
                    String content,
                    boolean multipart,
                    String boundary) throws SipException {
        try {
            if (logger.isActivated()) {
                logger.info("CPMS createLargeMessageCpmInvite: target" + dialog.getTarget());
            }
            ContentTypeHeader typeHeader;
            if (multipart) {
                typeHeader = SipUtils.HEADER_FACTORY.createContentTypeHeader("multipart", "mixed");
                typeHeader.setParameter("boundary", boundary);
            } else {
                typeHeader = SipUtils.HEADER_FACTORY.createContentTypeHeader("application", "sdp");
            }

            return createLargeCpimInvite(dialog, featureTags, preferService, content, typeHeader);
        } catch (Exception e) {
            if (logger.isActivated()) {
                logger.error("Can't create SIP message", e);
            }
            throw new SipException("Can't create SIP INVITE message");
        }
    }

    /**
     * Create a SIP INVITE request
     *
     * @param dialog SIP dialog path
     * @param featureTags Feature tags
     * @param sdp SDP part
     * @return SIP request
     * @throws SipException
     */
    public static SipRequest createFileCpmInvite(
                    SipDialogPath dialog,
                    String[] featureTags,
                    String preferService,
                    String content,
                    boolean multipart,
                    String boundary) throws SipException {
        try {
            if (logger.isActivated()) {
                logger.info("CPMS createLargeMessageCpmInvite: target" + dialog.getTarget());
            }
            ContentTypeHeader typeHeader;
            if (multipart) {
                typeHeader = SipUtils.HEADER_FACTORY.createContentTypeHeader("multipart", "mixed");
                typeHeader.setParameter("boundary", boundary);
            } else {
                typeHeader = SipUtils.HEADER_FACTORY.createContentTypeHeader("application", "sdp");
            }

            return createFileCpimInvite(dialog, featureTags, preferService, content, typeHeader);
        } catch (Exception e) {
            if (logger.isActivated()) {
                logger.error("Can't create SIP message", e);
            }
            throw new SipException("Can't create SIP INVITE message");
        }
    }

    /**
     * Create a SIP INVITE request
     *
     * @param dialog SIP dialog path
     * @param featureTags Feature tags
     * @param multipart Multipart
     * @param boudary Boundary tag
     * @return SIP request
     * @throws SipException
     */
    public static SipRequest createCpmMultipartInvite(SipDialogPath dialog, String[] featureTags,
            String multipart, String boundary,boolean isGroup) throws SipException {
        try {
            if (logger.isActivated()) {
                logger.info("ABCH createCpmMultipartInvite: target" + dialog.getTarget());
            }
            // Create the content type
            ContentTypeHeader contentType = SipUtils.HEADER_FACTORY.createContentTypeHeader(
                    "multipart", "mixed");
            contentType.setParameter("boundary", boundary);

            // Create the request
            return createCpimInvite(dialog, featureTags, multipart, contentType,isGroup);
        } catch (Exception e) {
            if (logger.isActivated()) {
                logger.error("Can't create SIP message", e);
            }
            throw new SipException("Can't create SIP INVITE message");
        }
    }

    /**
     * Create a SIP INVITE request
     *
     * @param dialog SIP dialog path
     * @param featureTags Feature tags
     * @param multipart Multipart
     * @param boudary Boundary tag
     * @return SIP request
     * @throws SipException
     */
    public static SipRequest createCpmFileMultipartInvite(SipDialogPath dialog, String[] featureTags,
            String multipart, String boundary,boolean isGroup) throws SipException {
        try {
            if (logger.isActivated()) {
                logger.info("ABCH createCpmMultipartInvite: target" + dialog.getTarget());
            }
            // Create the content type
            ContentTypeHeader contentType = SipUtils.HEADER_FACTORY.createContentTypeHeader(
                    "multipart", "mixed");
            contentType.setParameter("boundary", boundary);

            // Create the request
            return createCpimFileInvite(dialog, featureTags, multipart, contentType, isGroup);
        } catch (Exception e) {
            if (logger.isActivated()) {
                logger.error("Can't create SIP message", e);
            }
            throw new SipException("Can't create SIP INVITE message");
        }
    }

    /**
     * Create a SIP INVITE request
     *
     * @param dialog SIP dialog path
     * @param featureTags Feature tags
     * @param multipart Multipart
     * @param boudary Boundary tag
     * @return SIP request
     * @throws SipException
     */
    public static SipRequest createCpmSpamMultipartInvite(SipDialogPath dialog,
            String[] featureTags, String multipart, String boundary) throws SipException {
        try {
            if (logger.isActivated()) {
                logger.info("ABCH createCpmMultipartInvite: target" + dialog.getTarget());
            }
            // Create the content type
            ContentTypeHeader contentType = SipUtils.HEADER_FACTORY.createContentTypeHeader(
                    "multipart", "mixed");
            contentType.setParameter("boundary", boundary);

            // Create the request
            return createSpamCpimInvite(dialog, featureTags, multipart, contentType);
        } catch (Exception e) {
            if (logger.isActivated()) {
                logger.error("Can't create SIP message", e);
            }
            throw new SipException("Can't create SIP INVITE message");
        }
    }

    /**
     * Create a SIP INVITE request
     *
     * @param dialog SIP dialog path
     * @param featureTags Feature tags
     * @param content Content
     * @param contentType Content type
     * @return SIP request
     * @throws SipException
     */
    public static SipRequest createSpamCpimInvite(SipDialogPath dialog, String[] featureTags,
            String content, ContentTypeHeader contentType) throws SipException {
        try {

            if (logger.isActivated()) {
                logger.info("ABC createCpimInvite target: " + dialog.getTarget());
            }
            // Set request line header
            URI requestURI = SipUtils.ADDR_FACTORY.createURI(dialog.getTarget());

            // Set Call-Id header
            CallIdHeader callIdHeader = SipUtils.HEADER_FACTORY.createCallIdHeader(dialog
                    .getCallId());

            // Set the CSeq header
            CSeqHeader cseqHeader = SipUtils.HEADER_FACTORY.createCSeqHeader(dialog.getCseq(),
                    Request.INVITE);

            // Set the From header
            Address fromAddress = SipUtils.ADDR_FACTORY.createAddress(dialog.getLocalParty());
            FromHeader fromHeader = SipUtils.HEADER_FACTORY.createFromHeader(
                    fromAddress, dialog.getLocalTag());

            // Set the To header
            Address toAddress = SipUtils.ADDR_FACTORY.createAddress(dialog.getRemoteParty());
            ToHeader toHeader = SipUtils.HEADER_FACTORY.createToHeader(toAddress, null);

            // Insert "keep" flag to Via header (RFC6223
            // "Indication of Support for Keep-Alive")
            ArrayList<ViaHeader> viaHeaders = dialog.getSipStack().getViaHeaders();
            if (viaHeaders != null && !viaHeaders.isEmpty()) {
                ViaHeader viaHeader = viaHeaders.get(0);
                if(RcsSettings.getInstance().getRcsVolteSingleRegistrationType() != 1){
                    viaHeader.setParameter(new NameValue("keep", null, true));
                }
            }

            // Create the request
            Request invite = SipUtils.MSG_FACTORY.createRequest(requestURI, Request.INVITE,
                    callIdHeader, cseqHeader, fromHeader, toHeader, viaHeaders,
                    SipUtils.buildMaxForwardsHeader());

            // Set Contact header
            invite.addHeader(dialog.getSipStack().getContact());

            List<String> list = Arrays.asList(featureTags);
            SipUtils.setAcceptContactFeatureTags(invite, list);

            // Set Allow header
            SipUtils.buildAllowHeader(invite);

            // Set the Route header
            Vector<String> route = dialog.getRoute();
            for (int i = 0; i < route.size(); i++) {
                Header routeHeader = SipUtils.HEADER_FACTORY.createHeader(RouteHeader.NAME,
                        route.elementAt(i));
                invite.addHeader(routeHeader);
            }

            // Set the P-Preferred-Identity header
            if (ImsModule.IMS_USER_PROFILE.getPreferredUri() != null) {
                Header prefHeader = SipUtils.HEADER_FACTORY.createHeader(
                        SipUtils.HEADER_P_PREFERRED_IDENTITY,
                        ImsModule.IMS_USER_PROFILE.getPreferredUri());
                invite.addHeader(prefHeader);
            }

            // Set the preferred service header
            Header preferredServiceHeader = SipUtils.HEADER_FACTORY.createHeader(
                    SipUtils.HEADER_P_PREFERRED_SERVICE, FeatureTags.FEATURE_CPM_URN_MSG);
            invite.addHeader(preferredServiceHeader);

         // Set Require header
            SipUtils.buildRequireHeader(invite);

            // Set User-Agent header
            invite.addHeader(SipUtils.buildCpimUserAgentHeader());

            // Add session timer management
            if (dialog.getSessionExpireTime() >= SessionTimerManager.MIN_EXPIRE_PERIOD) {
                // Set the Supported header
                Header supportedHeader = SipUtils.HEADER_FACTORY.createHeader(
                        SupportedHeader.NAME, "timer");
                invite.addHeader(supportedHeader);

                // Set Session-Timer headers
                Header sessionExpiresHeader = SipUtils.HEADER_FACTORY.createHeader(
                        SipUtils.HEADER_SESSION_EXPIRES, "" + dialog.getSessionExpireTime());
                invite.addHeader(sessionExpiresHeader);
            }

            // Set the message content
            invite.setContent(content, contentType);

            // Set the content length
            ContentLengthHeader contentLengthHeader = SipUtils.HEADER_FACTORY
                    .createContentLengthHeader(content.getBytes().length);
            invite.setContentLength(contentLengthHeader);

            // Set "rport" (RFC3581)
            ViaHeader viaHeader = (ViaHeader) invite.getHeader(ViaHeader.NAME);
            viaHeader.setRPort();

            return new SipRequest(invite);
        } catch (Exception e) {
            if (logger.isActivated()) {
                logger.error("Can't create CPMS SIP message", e);
            }
            throw new SipException("Can't create SIP INVITE message");
        }
    }

    /**
     * Create a SIP INVITE request
     *
     * @param dialog SIP dialog path
     * @param featureTags Feature tags
     * @param content Content
     * @param contentType Content type
     * @return SIP request
     * @throws SipException
     */
    public static SipRequest createCpimInvite(SipDialogPath dialog, String[] featureTags,
            String content, ContentTypeHeader contentType,boolean isGroup) throws SipException {
        try {

            if (logger.isActivated()) {
                logger.info("createCpimInvite target: " + dialog.getTarget() + ", isGroup: " + isGroup);
            }
            // Set request line header
            URI requestURI = SipUtils.ADDR_FACTORY.createURI(dialog.getTarget());

            // Set Call-Id header
            CallIdHeader callIdHeader = SipUtils.HEADER_FACTORY.createCallIdHeader(
                    dialog.getCallId());

            // Set the CSeq header
            CSeqHeader cseqHeader = SipUtils.HEADER_FACTORY.createCSeqHeader(
                    dialog.getCseq(), Request.INVITE);

            // Set the From header
            Address fromAddress = SipUtils.ADDR_FACTORY.createAddress(dialog.getLocalParty());
            FromHeader fromHeader = SipUtils.HEADER_FACTORY.createFromHeader(
                    fromAddress, dialog.getLocalTag());

            // Set the To header
            Address toAddress = SipUtils.ADDR_FACTORY.createAddress(dialog.getRemoteParty());
            ToHeader toHeader = SipUtils.HEADER_FACTORY.createToHeader(toAddress, null);

            // Insert "keep" flag to Via header (RFC6223
            // "Indication of Support for Keep-Alive")
            ArrayList<ViaHeader> viaHeaders = dialog.getSipStack().getViaHeaders();
            if (viaHeaders != null && !viaHeaders.isEmpty()) {
                ViaHeader viaHeader = viaHeaders.get(0);
                if(RcsSettings.getInstance().getRcsVolteSingleRegistrationType() != 1){
                    viaHeader.setParameter(new NameValue("keep", null, true));
                }
            }

            // Create the request
            Request invite = SipUtils.MSG_FACTORY.createRequest(
                    requestURI, Request.INVITE,
                    callIdHeader, cseqHeader, fromHeader, toHeader, viaHeaders,
                    SipUtils.buildMaxForwardsHeader());

            // Set Contact header
            invite.addHeader(dialog.getSipStack().getContact());

            // Set feature tags
            SipUtils.setFeatureTags(invite, featureTags);

            // Set Allow header
            SipUtils.buildAllowHeader(invite);

            // Set the Route header
            Vector<String> route = dialog.getRoute();
            for (int i = 0; i < route.size(); i++) {
                Header routeHeader = SipUtils.HEADER_FACTORY.createHeader(
                        RouteHeader.NAME, route.elementAt(i));
                invite.addHeader(routeHeader);
            }

            // Set the P-Preferred-Identity header
            if (ImsModule.IMS_USER_PROFILE.getPreferredUri() != null) {
                Header prefHeader = SipUtils.HEADER_FACTORY.createHeader(
                        SipUtils.HEADER_P_PREFERRED_IDENTITY,
                        ImsModule.IMS_USER_PROFILE.getPreferredUri());
                invite.addHeader(prefHeader);
            }

            // Set the preferred service header
            Header preferredServiceHeader = null;
            boolean useLargeMode = false;
            String featureTagString = Arrays.toString(featureTags);
            useLargeMode = (RcsSettings.getInstance().isLargeModeSupported() &&
                    (RcsSettings.getInstance().isSupportOP08()
                    || RcsSettings.getInstance().isSupportOP07()))
                    || featureTagString.contains(FeatureTags.FEATURE_CPM_LARGE_MSG);

            if (logger.isActivated()) {
                logger.info("createCpimInvite featuretag: " + featureTagString
                        + " useLargeMode: " + useLargeMode);
            }
            if (useLargeMode) {
                preferredServiceHeader = SipUtils.HEADER_FACTORY.createHeader(
                        SipUtils.HEADER_P_PREFERRED_SERVICE, FeatureTags.FEATURE_CPM_URN_LARGE_MSG);
            } else {
                 if(!featureTagString.contains(FeatureTags.FEATURE_CPM_URN_SESSION)) {
                    if(isGroup){
                        preferredServiceHeader = SipUtils.HEADER_FACTORY.createHeader(
                                SipUtils.HEADER_P_PREFERRED_SERVICE, FeatureTags.FEATURE_CPM_URN_PREFERRED_GROUP_FT);
                    } else {
                        preferredServiceHeader = SipUtils.HEADER_FACTORY.createHeader(
                                SipUtils.HEADER_P_PREFERRED_SERVICE, FeatureTags.FEATURE_CPM_URN_PREFERRED_FT);
                    }
                } else {
                    if(isGroup){
                        preferredServiceHeader = SipUtils.HEADER_FACTORY.createHeader(
                                SipUtils.HEADER_P_PREFERRED_SERVICE, FeatureTags.FEATURE_CPM_URN_PREFERRED_GROUP_SESSION);
                    } else {
                        preferredServiceHeader = SipUtils.HEADER_FACTORY.createHeader(
                                SipUtils.HEADER_P_PREFERRED_SERVICE, FeatureTags.FEATURE_CPM_URN_PREFERRED_SESSION);
                    }
                }
            }

            invite.addHeader(preferredServiceHeader);

            // Set User-Agent header
            invite.addHeader(SipUtils.buildCpimUserAgentHeader());

            // Add session timer management
            if (dialog.getSessionExpireTime() >= SessionTimerManager.MIN_EXPIRE_PERIOD) {
                // Set the Supported header
                Header supportedHeader = SipUtils.HEADER_FACTORY.createHeader(
                        SupportedHeader.NAME, "timer");
                invite.addHeader(supportedHeader);

                // Set Session-Timer headers
                Header sessionExpiresHeader = SipUtils.HEADER_FACTORY.createHeader(
                        SipUtils.HEADER_SESSION_EXPIRES, "" + dialog.getSessionExpireTime());
                invite.addHeader(sessionExpiresHeader);
            }

            if(isGroup && (RcsSettings.getInstance().isSupportOP08() || RcsSettings.getInstance().isSupportOP07())){
                logger.info("ABC buildGroupRequireHeader");
                SipUtils.buildGroupRequireHeader(invite);
            }else {
                SipUtils.buildRequireHeader(invite);
            }

            // Set the message content
            invite.setContent(content, contentType);

            // Set the content length
            ContentLengthHeader contentLengthHeader = SipUtils.HEADER_FACTORY
                    .createContentLengthHeader(content.getBytes().length);
            invite.setContentLength(contentLengthHeader);

            // Set "rport" (RFC3581)
            ViaHeader viaHeader = (ViaHeader) invite.getHeader(ViaHeader.NAME);
            viaHeader.setRPort();

            SipRequest req =  new SipRequest(invite);
            // Set PAccessNetworkInfo header
            addAccessNetworkInfoHeader(req);

            //Add Private Header
            addPrivateHeader(req);

            //Add XATT service Header
            addATTServiceHeader(req);

            addEarlyMediaHeader(req);

            return req;
        } catch (Exception e) {
            if (logger.isActivated()) {
                logger.error("Can't create CPMS SIP message", e);
            }
            throw new SipException("Can't create SIP INVITE message");
        }
    }

    /**
     * Create a SIP INVITE request
     *
     * @param dialog SIP dialog path
     * @param featureTags Feature tags
     * @param content Content
     * @param contentType Content type
     * @return SIP request
     * @throws SipException
     */
    public static SipRequest createCpimFileInvite(SipDialogPath dialog, String[] featureTags,
            String content, ContentTypeHeader contentType, boolean isGroup) throws SipException {
        try {

            if (logger.isActivated()) {
                logger.info("ABC createCpimInvite target: " + dialog.getTarget());
            }
            // Set request line header
            URI requestURI = SipUtils.ADDR_FACTORY.createURI(dialog.getTarget());

            // Set Call-Id header
            CallIdHeader callIdHeader = SipUtils.HEADER_FACTORY.createCallIdHeader(
                    dialog.getCallId());

            // Set the CSeq header
            CSeqHeader cseqHeader = SipUtils.HEADER_FACTORY.createCSeqHeader(
                    dialog.getCseq(), Request.INVITE);

            // Set the From header
            Address fromAddress = SipUtils.ADDR_FACTORY.createAddress(dialog.getLocalParty());
            FromHeader fromHeader = SipUtils.HEADER_FACTORY.createFromHeader(
                    fromAddress, dialog.getLocalTag());

            // Set the To header
            Address toAddress = SipUtils.ADDR_FACTORY.createAddress(dialog.getRemoteParty());
            ToHeader toHeader = SipUtils.HEADER_FACTORY.createToHeader(toAddress, null);

            // Insert "keep" flag to Via header (RFC6223
            // "Indication of Support for Keep-Alive")
            ArrayList<ViaHeader> viaHeaders = dialog.getSipStack().getViaHeaders();
            if (viaHeaders != null && !viaHeaders.isEmpty()) {
                ViaHeader viaHeader = viaHeaders.get(0);
                if(RcsSettings.getInstance().getRcsVolteSingleRegistrationType() != 1){
                    viaHeader.setParameter(new NameValue("keep", null, true));
                }
            }

            // Create the request
            Request invite = SipUtils.MSG_FACTORY.createRequest(
                    requestURI, Request.INVITE,
                    callIdHeader, cseqHeader, fromHeader, toHeader, viaHeaders,
                    SipUtils.buildMaxForwardsHeader());

            // Set Contact header
            invite.addHeader(dialog.getSipStack().getContact());




            // Set feature tags
            SipUtils.setFeatureTags(invite, featureTags);

            // Set Allow header
            SipUtils.buildAllowHeader(invite);

            // Set the Route header
            Vector<String> route = dialog.getRoute();
            for (int i = 0; i < route.size(); i++) {
                Header routeHeader = SipUtils.HEADER_FACTORY.createHeader(
                        RouteHeader.NAME, route.elementAt(i));
                invite.addHeader(routeHeader);
            }

            // Set the P-Preferred-Identity header
            if (ImsModule.IMS_USER_PROFILE.getPreferredUri() != null) {
                Header prefHeader = SipUtils.HEADER_FACTORY.createHeader(
                        SipUtils.HEADER_P_PREFERRED_IDENTITY,
                        ImsModule.IMS_USER_PROFILE.getPreferredUri());
                invite.addHeader(prefHeader);
            }

            Header preferredServiceHeader = null;
            boolean useLargeMode = false;
            String featureTagString = Arrays.toString(featureTags);
            useLargeMode = (RcsSettings.getInstance().isLargeModeSupported() &&
                    (RcsSettings.getInstance().isSupportOP08()
                    || RcsSettings.getInstance().isSupportOP07()))
                    || featureTagString.contains(FeatureTags.FEATURE_CPM_LARGE_MSG);
            if (logger.isActivated()) {
                logger.info("createCpimInvite featuretagstring: "
                        + featureTagString + " useLargeMode: " + useLargeMode);
            }
            if(useLargeMode){
             // Set the preferred service header
                preferredServiceHeader = SipUtils.HEADER_FACTORY.createHeader(
                       SipUtils.HEADER_P_PREFERRED_SERVICE, FeatureTags.FEATURE_CPM_URN_LARGE_MSG);
            } else {
                if(isGroup){
                    preferredServiceHeader = SipUtils.HEADER_FACTORY.createHeader(
                            SipUtils.HEADER_P_PREFERRED_SERVICE, FeatureTags.FEATURE_CPM_URN_PREFERRED_GROUP_FT);
                } else {
                    preferredServiceHeader = SipUtils.HEADER_FACTORY.createHeader(
                            SipUtils.HEADER_P_PREFERRED_SERVICE, FeatureTags.FEATURE_CPM_URN_PREFERRED_FT);
                }
            }
            invite.addHeader(preferredServiceHeader);

            // Set User-Agent header
            invite.addHeader(SipUtils.buildCpimUserAgentHeader());

            // Add session timer management
            if (dialog.getSessionExpireTime() >= SessionTimerManager.MIN_EXPIRE_PERIOD) {
                // Set the Supported header
                Header supportedHeader = SipUtils.HEADER_FACTORY.createHeader(
                        SupportedHeader.NAME, "timer");
                invite.addHeader(supportedHeader);

                // Set Session-Timer headers
                Header sessionExpiresHeader = SipUtils.HEADER_FACTORY.createHeader(
                        SipUtils.HEADER_SESSION_EXPIRES, "" + dialog.getSessionExpireTime());
                invite.addHeader(sessionExpiresHeader);
            }

             SipUtils.buildRequireHeader(invite);

            // Set the message content
            invite.setContent(content, contentType);

            // Set the content length
            ContentLengthHeader contentLengthHeader = SipUtils.HEADER_FACTORY
                    .createContentLengthHeader(content.getBytes().length);
            invite.setContentLength(contentLengthHeader);

            // Set "rport" (RFC3581)
            ViaHeader viaHeader = (ViaHeader) invite.getHeader(ViaHeader.NAME);
            viaHeader.setRPort();

            SipRequest req = new SipRequest(invite);

            // Set PAccessNetworkInfo header
            addAccessNetworkInfoHeader(req);

            //Add Private Header
            addPrivateHeader(req);

            //Add XATT service Header
            addATTServiceHeader(req);

            addEarlyMediaHeader(req);

            return req;
        } catch (Exception e) {
            if (logger.isActivated()) {
                logger.error("Can't create CPMS SIP message", e);
            }
            throw new SipException("Can't create SIP INVITE message");
        }
    }

    /**
     * Create a SIP INVITE request
     *
     * @param dialog SIP dialog path
     * @param featureTags Feature tags
     * @param content Content
     * @param contentType Content type
     * @return SIP request
     * @throws SipException
     */
    public static SipRequest createLargeCpimInvite(
            SipDialogPath dialog, String[] featureTags, String preferService,
            String content, ContentTypeHeader contentType) throws SipException {
        try {

            if (logger.isActivated()) {
                logger.info("ABC createLargeCpimInvite target: " + dialog.getTarget());
            }
            // Set request line header
            URI requestURI = SipUtils.ADDR_FACTORY.createURI(dialog.getTarget());

            // Set Call-Id header
            CallIdHeader callIdHeader = SipUtils.HEADER_FACTORY.createCallIdHeader(
                    dialog.getCallId());

            // Set the CSeq header
            CSeqHeader cseqHeader = SipUtils.HEADER_FACTORY.createCSeqHeader(
                    dialog.getCseq(), Request.INVITE);

            // Set the From header
            Address fromAddress = SipUtils.ADDR_FACTORY.createAddress(dialog.getLocalParty());
            FromHeader fromHeader = SipUtils.HEADER_FACTORY.createFromHeader(
                    fromAddress, dialog.getLocalTag());

            // Set the To header
            Address toAddress = SipUtils.ADDR_FACTORY.createAddress(dialog.getRemoteParty());
            ToHeader toHeader = SipUtils.HEADER_FACTORY.createToHeader(toAddress, null);

            // Insert "keep" flag to Via header (RFC6223
            // "Indication of Support for Keep-Alive")
            ArrayList<ViaHeader> viaHeaders = dialog.getSipStack().getViaHeaders();
            if (viaHeaders != null && !viaHeaders.isEmpty()) {
                ViaHeader viaHeader = viaHeaders.get(0);
                if(RcsSettings.getInstance().getRcsVolteSingleRegistrationType() != 1){
                    viaHeader.setParameter(new NameValue("keep", null, true));
                }
            }

            // Create the request
            Request invite = SipUtils.MSG_FACTORY.createRequest(
                    requestURI, Request.INVITE,
                    callIdHeader, cseqHeader, fromHeader, toHeader, viaHeaders,
                    SipUtils.buildMaxForwardsHeader());

            // Set Contact header
            invite.addHeader(dialog.getSipStack().getContact());




            // Set feature tags
            SipUtils.setFeatureTags(invite, featureTags);

            // Set Allow header
            SipUtils.buildAllowHeader(invite);

            // Set the Route header
            Vector<String> route = dialog.getRoute();
            for (int i = 0; i < route.size(); i++) {
                Header routeHeader = SipUtils.HEADER_FACTORY.createHeader(
                        RouteHeader.NAME, route.elementAt(i));
                invite.addHeader(routeHeader);
            }

            // Set the P-Preferred-Identity header
            if (ImsModule.IMS_USER_PROFILE.getPreferredUri() != null) {
                Header prefHeader = SipUtils.HEADER_FACTORY.createHeader(
                        SipUtils.HEADER_P_PREFERRED_IDENTITY,
                        ImsModule.IMS_USER_PROFILE.getPreferredUri());
                invite.addHeader(prefHeader);
            }

            // Set the preferred service header
            Header preferredServiceHeader;
            if (preferService == null) {
                preferredServiceHeader = SipUtils.HEADER_FACTORY.createHeader(
                    SipUtils.HEADER_P_PREFERRED_SERVICE, FeatureTags.FEATURE_CPM_URN_LARGE_MSG);
            } else {
                preferredServiceHeader = SipUtils.HEADER_FACTORY.createHeader(
                        SipUtils.HEADER_P_PREFERRED_SERVICE, preferService);
            }
            invite.addHeader(preferredServiceHeader);

            // Set User-Agent header
            invite.addHeader(SipUtils.buildCpimUserAgentHeader());

            // Add session timer management
            if (dialog.getSessionExpireTime() >= SessionTimerManager.MIN_EXPIRE_PERIOD) {
                // Set the Supported header
                Header supportedHeader = SipUtils.HEADER_FACTORY.createHeader(
                        SupportedHeader.NAME, "timer");
                invite.addHeader(supportedHeader);

                // Set Session-Timer headers
                Header sessionExpiresHeader = SipUtils.HEADER_FACTORY.createHeader(
                        SipUtils.HEADER_SESSION_EXPIRES, "" + dialog.getSessionExpireTime());
                invite.addHeader(sessionExpiresHeader);
            }
            // Set Require header
            SipUtils.buildRequireHeader(invite);

            // Set the message content
            invite.setContent(content, contentType);

            // Set the content length
            ContentLengthHeader contentLengthHeader = SipUtils.HEADER_FACTORY
                    .createContentLengthHeader(content.getBytes().length);
            invite.setContentLength(contentLengthHeader);

            // Set "rport" (RFC3581)
            ViaHeader viaHeader = (ViaHeader) invite.getHeader(ViaHeader.NAME);
            viaHeader.setRPort();

            SipRequest req = new SipRequest(invite);

            // Set PAccessNetworkInfo header
            addAccessNetworkInfoHeader(req);

            //Add Private Header
            addPrivateHeader(req);

            //Add XATT service Header
            addATTServiceHeader(req);

            addEarlyMediaHeader(req);

            return req;
        } catch (Exception e) {
            if (logger.isActivated()) {
                logger.error("Can't create CPMS SIP message", e);
            }
            throw new SipException("Can't create SIP INVITE message");
        }
    }

    /**
     * Create a SIP INVITE request
     *
     * @param dialog SIP dialog path
     * @param featureTags Feature tags
     * @param content Content
     * @param contentType Content type
     * @return SIP request
     * @throws SipException
     */
    public static SipRequest createFileCpimInvite(
                SipDialogPath dialog,
                String[] featureTags,
                String preferService,
                String content,
                ContentTypeHeader contentType) throws SipException {
        try {

            if (logger.isActivated()) {
                logger.info("ABC createCpimInvite target: " + dialog.getTarget());
            }
            // Set request line header
            URI requestURI = SipUtils.ADDR_FACTORY.createURI(dialog.getTarget());

            // Set Call-Id header
            CallIdHeader callIdHeader = SipUtils.HEADER_FACTORY.createCallIdHeader(
                    dialog.getCallId());

            // Set the CSeq header
            CSeqHeader cseqHeader = SipUtils.HEADER_FACTORY.createCSeqHeader(
                    dialog.getCseq(), Request.INVITE);

            // Set the From header
            Address fromAddress = SipUtils.ADDR_FACTORY.createAddress(dialog.getLocalParty());
            FromHeader fromHeader = SipUtils.HEADER_FACTORY.createFromHeader(
                    fromAddress, dialog.getLocalTag());

            // Set the To header
            Address toAddress = SipUtils.ADDR_FACTORY.createAddress(dialog.getRemoteParty());
            ToHeader toHeader = SipUtils.HEADER_FACTORY.createToHeader(toAddress, null);

            // Insert "keep" flag to Via header (RFC6223
            // "Indication of Support for Keep-Alive")
            ArrayList<ViaHeader> viaHeaders = dialog.getSipStack().getViaHeaders();
            if (viaHeaders != null && !viaHeaders.isEmpty()) {
                ViaHeader viaHeader = viaHeaders.get(0);
                if(RcsSettings.getInstance().getRcsVolteSingleRegistrationType() != 1){
                    viaHeader.setParameter(new NameValue("keep", null, true));
                }
            }

            // Create the request
            Request invite = SipUtils.MSG_FACTORY.createRequest(
                    requestURI, Request.INVITE,
                    callIdHeader, cseqHeader, fromHeader, toHeader, viaHeaders,
                    SipUtils.buildMaxForwardsHeader());

            // Set Contact header
            invite.addHeader(dialog.getSipStack().getContact());




            // Set feature tags
            SipUtils.setFeatureTags(invite, featureTags);

            // Set Allow header
            SipUtils.buildAllowHeader(invite);

            // Set the Route header
            Vector<String> route = dialog.getRoute();
            for (int i = 0; i < route.size(); i++) {
                Header routeHeader = SipUtils.HEADER_FACTORY.createHeader(
                        RouteHeader.NAME, route.elementAt(i));
                invite.addHeader(routeHeader);
            }

            // Set the P-Preferred-Identity header
            if (ImsModule.IMS_USER_PROFILE.getPreferredUri() != null) {
                Header prefHeader = SipUtils.HEADER_FACTORY.createHeader(
                        SipUtils.HEADER_P_PREFERRED_IDENTITY,
                        ImsModule.IMS_USER_PROFILE.getPreferredUri());
                invite.addHeader(prefHeader);
            }

            // Set the preferred service header
            Header preferredServiceHeader;
            boolean useLargeMode = false;
            String featureTagString = Arrays.toString(featureTags);
            useLargeMode = featureTagString.contains(FeatureTags.FEATURE_CPM_LARGE_MSG);
            if (logger.isActivated()) {
                logger.info("createCpimInvite featuretagstring: " + featureTagString
                        + " useLargeMode: " + useLargeMode);
            }
            if(useLargeMode){
                preferredServiceHeader = SipUtils.HEADER_FACTORY.createHeader(
                        SipUtils.HEADER_P_PREFERRED_SERVICE, FeatureTags.FEATURE_CPM_URN_LARGE_MSG);
            } else {
                if (preferService == null)
                    preferredServiceHeader = SipUtils.HEADER_FACTORY.createHeader(
                        SipUtils.HEADER_P_PREFERRED_SERVICE, FeatureTags.FEATURE_CPM_URN_PREFERRED_FT);
                else
                    preferredServiceHeader = SipUtils.HEADER_FACTORY.createHeader(
                            SipUtils.HEADER_P_PREFERRED_SERVICE, preferService);
            }
            invite.addHeader(preferredServiceHeader);

            // Set User-Agent header
            invite.addHeader(SipUtils.buildCpimUserAgentHeader());

            // Add session timer management
            if (dialog.getSessionExpireTime() >= SessionTimerManager.MIN_EXPIRE_PERIOD) {
                // Set the Supported header
                Header supportedHeader = SipUtils.HEADER_FACTORY.createHeader(
                        SupportedHeader.NAME, "timer");
                invite.addHeader(supportedHeader);

                // Set Session-Timer headers
                Header sessionExpiresHeader = SipUtils.HEADER_FACTORY.createHeader(
                        SipUtils.HEADER_SESSION_EXPIRES, "" + dialog.getSessionExpireTime());
                invite.addHeader(sessionExpiresHeader);
            }

             SipUtils.buildRequireHeader(invite);

            // Set the message content
            invite.setContent(content, contentType);

            // Set the content length
            ContentLengthHeader contentLengthHeader = SipUtils.HEADER_FACTORY
                    .createContentLengthHeader(content.getBytes().length);
            invite.setContentLength(contentLengthHeader);

            // Set "rport" (RFC3581)
            ViaHeader viaHeader = (ViaHeader) invite.getHeader(ViaHeader.NAME);
            viaHeader.setRPort();


            SipRequest req = new SipRequest(invite);
            // Set PAccessNetworkInfo header
            addAccessNetworkInfoHeader(req);

            //Add Private Header
            addPrivateHeader(req);

            //Add XATT service Header
            addATTServiceHeader(req);

            addEarlyMediaHeader(req);

            return req;
        } catch (Exception e) {
            if (logger.isActivated()) {
                logger.error("Can't create CPMS SIP message", e);
            }
            throw new SipException("Can't create SIP INVITE message");
        }
    }

    /**
     * Create a SIP INVITE request
     *
     * @param dialog SIP dialog path
     * @param featureTags Feature tags
     * @param sdp SDP part
     * @return SIP request
     * @throws SipException
     */
    public static SipRequest createInvite(SipDialogPath dialog, String[] featureTags, String sdp)
            throws SipException {
        try {
            // Create the content type
            ContentTypeHeader contentType = SipUtils.HEADER_FACTORY.createContentTypeHeader(
                    "application", "sdp");

            // Create the request
            return createInvite(dialog, featureTags, sdp, contentType);
        } catch (Exception e) {
            if (logger.isActivated()) {
                logger.error("Can't create SIP message", e);
            }
            throw new SipException("Can't create SIP INVITE message");
        }
    }

    /**
     * Create a SIP INVITE request
     *
     * @param dialog SIP dialog path
     * @param featureTags Feature tags
     * @param multipart Multipart
     * @param boudary Boundary tag
     * @return SIP request
     * @throws SipException
     */
    public static SipRequest createMultipartInvite(SipDialogPath dialog, String[] featureTags,
            String multipart, String boundary) throws SipException {
        try {
            // Create the content type
            ContentTypeHeader contentType = SipUtils.HEADER_FACTORY.createContentTypeHeader(
                    "multipart", "mixed");
            contentType.setParameter("boundary", boundary);

            // Create the request
            return createInvite(dialog, featureTags, multipart, contentType);
        } catch (Exception e) {
            if (logger.isActivated()) {
                logger.error("Can't create SIP message", e);
            }
            throw new SipException("Can't create SIP INVITE message");
        }
    }



    private static void setPPreferedIdentityHeader(Request request) throws Exception {
        String preferedUri = ImsModule.IMS_USER_PROFILE.getPreferredUri();
        if (preferedUri != null) {
            Header prefHeader = SipUtils.HEADER_FACTORY.createHeader(
                    SipUtils.HEADER_P_PREFERRED_IDENTITY, preferedUri);
            request.addHeader(prefHeader);
        }
    }


    /**
     * Create a SIP INVITE request
     *
     * @param dialog SIP dialog path
     * @param featureTags Feature tags
     * @param acceptTags Feature tags
     * @param content Content
     * @param contentType Content type
     * @return SIP request
     * @throws PayloadException
     */
    public static SipRequest createInvite(SipDialogPath dialog, String[] featureTags,
            String[] acceptTags, String content, ContentTypeHeader contentType)
            throws PayloadException {
        try {
            // Set request line header
            URI requestURI = SipUtils.ADDR_FACTORY.createURI(dialog.getTarget());

            // Set Call-Id header
            CallIdHeader callIdHeader = SipUtils.HEADER_FACTORY.createCallIdHeader(dialog
                    .getCallId());

            // Set the CSeq header
            CSeqHeader cseqHeader = SipUtils.HEADER_FACTORY.createCSeqHeader(dialog.getCseq(),
                    Request.INVITE);

            // Set the From header
            Address fromAddress = SipUtils.ADDR_FACTORY.createAddress(dialog.getLocalParty());
            FromHeader fromHeader = SipUtils.HEADER_FACTORY.createFromHeader(fromAddress,
                    dialog.getLocalTag());

            // Set the To header
            Address toAddress = SipUtils.ADDR_FACTORY.createAddress(dialog.getRemoteParty());
            ToHeader toHeader = SipUtils.HEADER_FACTORY.createToHeader(toAddress, null);

            // Insert "keep" flag to Via header (RFC6223 "Indication of Support for Keep-Alive")
            List<ViaHeader> viaHeaders = dialog.getSipStack().getViaHeaders();
            if (viaHeaders != null && !viaHeaders.isEmpty()) {
                ViaHeader viaHeader = viaHeaders.get(0);
                viaHeader.setParameter(new NameValue("keep", null, true));
            }

            // Create the request
            Request invite = SipUtils.MSG_FACTORY.createRequest(requestURI, Request.INVITE,
                    callIdHeader, cseqHeader, fromHeader, toHeader, viaHeaders,
                    SipUtils.buildMaxForwardsHeader());

            // Set Contact header
            invite.addHeader(dialog.getSipStack().getContact());

            // Set feature tags
            SipUtils.setFeatureTags(invite, featureTags, acceptTags);

            // Set Allow header
            SipUtils.buildAllowHeader(invite);

            // Set the Route header
            Vector<String> route = dialog.getRoute();
            for (int i = 0; i < route.size(); i++) {
                Header routeHeader = SipUtils.HEADER_FACTORY.createHeader(RouteHeader.NAME,
                        route.elementAt(i));
                invite.addHeader(routeHeader);
            }

            setPPreferedIdentityHeader(invite);

            // Set User-Agent header
            invite.addHeader(SipUtils.buildUserAgentHeader());

            // Add session timer management
            if (dialog.getSessionExpireTime() >= SessionTimerManager.MIN_EXPIRE_PERIOD) {
                // Set the Supported header
                Header supportedHeader = SipUtils.HEADER_FACTORY.createHeader(SupportedHeader.NAME,
                        "timer");
                invite.addHeader(supportedHeader);

                // Set Session-Timer headers
                Header sessionExpiresHeader = SipUtils.HEADER_FACTORY.createHeader(
                        SipUtils.HEADER_SESSION_EXPIRES, "" + dialog.getSessionExpireTime());
                invite.addHeader(sessionExpiresHeader);
            }

            // Set the message content
            invite.setContent(content, contentType);

            // Set the content length
            invite.setContentLength(SipUtils.HEADER_FACTORY.createContentLengthHeader(content
                    .getBytes(UTF8).length));

            // Set "rport" (RFC3581)
            ViaHeader viaHeader = (ViaHeader) invite.getHeader(ViaHeader.NAME);
            viaHeader.setRPort();

            return new SipRequest(invite);

        } catch (Exception e) {
            throw new PayloadException("Can't create SIP message with content : " + content, e);
        }
    }


    /**
     * Create a SIP INVITE request
     *
     * @param dialog SIP dialog path
     * @param featureTags Feature tags
     * @param content Content
     * @param contentType Content type
     * @return SIP request
     * @throws SipException
     */
    public static SipRequest createInvite(SipDialogPath dialog, String[] featureTags,
            String content, ContentTypeHeader contentType) throws SipException {
        try {
            // Set request line header
            URI requestURI = SipUtils.ADDR_FACTORY.createURI(dialog.getTarget());

            // Set Call-Id header
            CallIdHeader callIdHeader = SipUtils.HEADER_FACTORY.createCallIdHeader(
                    dialog.getCallId());

            // Set the CSeq header
            CSeqHeader cseqHeader = SipUtils.HEADER_FACTORY.createCSeqHeader(
                    dialog.getCseq(), Request.INVITE);

            // Set the From header
            Address fromAddress = SipUtils.ADDR_FACTORY.createAddress(dialog.getLocalParty());
            FromHeader fromHeader = SipUtils.HEADER_FACTORY.createFromHeader(
                    fromAddress, dialog.getLocalTag());

            // Set the To header
            Address toAddress = SipUtils.ADDR_FACTORY.createAddress(dialog.getRemoteParty());
            ToHeader toHeader = SipUtils.HEADER_FACTORY.createToHeader(toAddress, null);

            // Insert "keep" flag to Via header (RFC6223
            // "Indication of Support for Keep-Alive")
            ArrayList<ViaHeader> viaHeaders = dialog.getSipStack().getViaHeaders();
            if (viaHeaders != null && !viaHeaders.isEmpty()) {
                ViaHeader viaHeader = viaHeaders.get(0);
                if(RcsSettings.getInstance().getRcsVolteSingleRegistrationType() != 1){
                    viaHeader.setParameter(new NameValue("keep", null, true));
                }
            }

            // Create the request
            Request invite = SipUtils.MSG_FACTORY.createRequest(
                    requestURI, Request.INVITE,
                    callIdHeader, cseqHeader, fromHeader, toHeader, viaHeaders,
                    SipUtils.buildMaxForwardsHeader());

            // Set Contact header
            invite.addHeader(dialog.getSipStack().getContact());

            // Set feature tags
            SipUtils.setFeatureTags(invite, featureTags);

            // Set Allow header
            SipUtils.buildAllowHeader(invite);

            // Set the Route header
            Vector<String> route = dialog.getRoute();
            for (int i = 0; i < route.size(); i++) {
                Header routeHeader = SipUtils.HEADER_FACTORY.createHeader(
                        RouteHeader.NAME, route.elementAt(i));
                invite.addHeader(routeHeader);
            }

            // Set the P-Preferred-Identity header
            if (ImsModule.IMS_USER_PROFILE.getPreferredUri() != null) {
                Header prefHeader = SipUtils.HEADER_FACTORY.createHeader(
                        SipUtils.HEADER_P_PREFERRED_IDENTITY,
                        ImsModule.IMS_USER_PROFILE.getPreferredUri());
                invite.addHeader(prefHeader);
            }

            // Set User-Agent header
            invite.addHeader(SipUtils.buildUserAgentHeader());

            SipUtils.buildRequireHeader(invite);

            if (logger.isActivated()) {
                logger.info("ABC sesion timer CREATE INVITE " + dialog.getSessionExpireTime());
            }

            // Add session timer management
            if (dialog.getSessionExpireTime() >= SessionTimerManager.MIN_EXPIRE_PERIOD) {
                // Set the Supported header
                Header supportedHeader = SipUtils.HEADER_FACTORY.createHeader(
                        SupportedHeader.NAME, "timer");
                invite.addHeader(supportedHeader);

                // Set Session-Timer headers
                Header sessionExpiresHeader = SipUtils.HEADER_FACTORY.createHeader(
                        SipUtils.HEADER_SESSION_EXPIRES, "" + dialog.getSessionExpireTime());
                invite.addHeader(sessionExpiresHeader);
                // + ";refresher=" + SessionTimerManager.UAC_ROLE

                Header min_se = SipUtils.HEADER_FACTORY.createHeader(SipUtils.HEADER_MIN_SE, " "
                        + SessionTimerManager.MIN_EXPIRE_PERIOD);
                invite.addHeader(min_se);
            }

            // Set the message content
            invite.setContent(content, contentType);

            // Set the content length
            ContentLengthHeader contentLengthHeader = SipUtils.HEADER_FACTORY
                    .createContentLengthHeader(content.getBytes().length);
            invite.setContentLength(contentLengthHeader);

            // Set "rport" (RFC3581)
            ViaHeader viaHeader = (ViaHeader) invite.getHeader(ViaHeader.NAME);
            viaHeader.setRPort();

            return new SipRequest(invite);
        } catch (Exception e) {
            if (logger.isActivated()) {
                logger.error("Can't create SIP message", e);
            }
            throw new SipException("Can't create SIP INVITE message");
        }
    }

    /**
     * Create a 200 OK response for INVITE request
     *
     * @param dialog SIP dialog path
     * @param featureTags Feature tags
     * @param sdp SDP part
     * @return SIP response
     * @throws SipException
     */
    public static SipResponse create200OkInviteResponse(SipDialogPath dialog, String[] featureTags,
            String sdp) throws SipException {
        try {
            // Create the response
            Response response = SipUtils.MSG_FACTORY.createResponse(
                    200, (Request)dialog.getInvite().getStackMessage());

            // Set the local tag
            ToHeader to = (ToHeader) response.getHeader(ToHeader.NAME);
            to.setTag(dialog.getLocalTag());

            // Set Contact header
            response.addHeader(dialog.getSipStack().getContact());

            // Set feature tags
            SipUtils.setFeatureTags(response, featureTags);

            // Set Allow header
            SipUtils.buildAllowHeader(response);



            // Set Require header
            SipUtils.buildRequireHeader(response);

            // Set the Server header
            response.addHeader(SipUtils.buildServerHeader());

            if (logger.isActivated()) {
                logger.info("ABC sesion timer CREATE INVITE response "
                        + dialog.getSessionExpireTime());
            }

            // Add session timer management
            if (dialog.getSessionExpireTime() >= SessionTimerManager.MIN_EXPIRE_PERIOD) {
                // Set the Require header
                Header requireHeader = SipUtils.HEADER_FACTORY.createHeader(
                        RequireHeader.NAME, "timer");
                response.addHeader(requireHeader);

                // Set Session-Timer header
                Header sessionExpiresHeader = SipUtils.HEADER_FACTORY.createHeader(
                        SipUtils.HEADER_SESSION_EXPIRES, dialog.getSessionExpireTime()
                                + ";refresher=" + dialog.getInvite().getSessionTimerRefresher());
                response.addHeader(sessionExpiresHeader);
            }

            // Set the message content
            ContentTypeHeader contentTypeHeader = SipUtils.HEADER_FACTORY.createContentTypeHeader(
                    "application", "sdp");
            response.setContent(sdp, contentTypeHeader);

            // Set the message content length
            ContentLengthHeader contentLengthHeader = SipUtils.HEADER_FACTORY
                    .createContentLengthHeader(sdp.getBytes().length);
            response.setContentLength(contentLengthHeader);

            SipResponse resp = new SipResponse(response);
            resp.setStackTransaction(dialog.getInvite().getStackTransaction());
            return resp;
        } catch (Exception e) {
            if (logger.isActivated()) {
                logger.error("Can't create SIP message", e);
            }
            throw new SipException("Can't create SIP response");
        }
    }


    /**
     * Create a SIP INVITE request
     *
     * @param dialog SIP dialog path
     * @param featureTags Feature tags
     * @param acceptTags Feature tags
     * @param sdp SDP part
     * @return SIP request
     * @throws PayloadException
     */
    public static SipRequest createInvite(SipDialogPath dialog, String[] featureTags,
            String[] acceptTags, String sdp) throws PayloadException {
        try {
            // Create the content type
            ContentTypeHeader contentType = SipUtils.HEADER_FACTORY.createContentTypeHeader(
                    "application", "sdp");

            // Create the request
            return createInvite(dialog, featureTags, acceptTags, sdp, contentType);

        } catch (Exception e) {
            throw new PayloadException("Can't create SIP message with SDP : " + sdp, e);
        }
    }

    public static SipResponse create200OkInviteResponse(SipDialogPath dialog, String[] featureTags,
            String[] acceptContactTags, String sdp) throws PayloadException {
        try {
            // Create the response
            Response response = SipUtils.MSG_FACTORY.createResponse(200, dialog.getInvite()
                    .getStackMessage());

            // Set the local tag
            ToHeader to = (ToHeader) response.getHeader(ToHeader.NAME);
            to.setTag(dialog.getLocalTag());

            // Set Contact header
            response.addHeader(dialog.getSipStack().getContact());

            // Set feature tags
            SipUtils.setFeatureTags(response, featureTags, acceptContactTags);

            // Set Allow header
            SipUtils.buildAllowHeader(response);

            // Set the Server header
            response.addHeader(SipUtils.buildServerHeader());

            // Add session timer management
            if (dialog.getSessionExpireTime() >= SessionTimerManager.MIN_EXPIRE_PERIOD) {
                // Set the Require header
                Header requireHeader = SipUtils.HEADER_FACTORY.createHeader(RequireHeader.NAME,
                        "timer");
                response.addHeader(requireHeader);

                // Set Session-Timer header
                Header sessionExpiresHeader = SipUtils.HEADER_FACTORY.createHeader(
                        SipUtils.HEADER_SESSION_EXPIRES,
                        String.valueOf(dialog.getSessionExpireTime()
                                / SECONDS_TO_MILLISECONDS_CONVERSION_RATE)
                                + ";refresher=" + dialog.getInvite().getSessionTimerRefresher());
                response.addHeader(sessionExpiresHeader);
            }

            // Set the message content
            response.setContent(sdp,
                    SipUtils.HEADER_FACTORY.createContentTypeHeader("application", "sdp"));

            // Set the message content length
            response.setContentLength(SipUtils.HEADER_FACTORY.createContentLengthHeader(sdp
                    .getBytes(UTF8).length));

            SipResponse resp = new SipResponse(response);
            resp.setStackTransaction(dialog.getInvite().getStackTransaction());
            return resp;

        } catch (Exception e) {
            throw new PayloadException("Can't create SIP response with SDP : " + sdp, e);
        }
    }

    /**
     * Create a 200 OK response for INVITE request
     *
     * @param dialog SIP dialog path
     * @param featureTags Feature tags
     * @param sdp SDP part
     * @return SIP response
     * @throws SipException
     */
    public static SipResponse createCpm200OkInviteResponse(SipDialogPath dialog,
            String[] featureTags, String sdp) throws SipException {
        try {
            if (logger.isActivated()) {
                logger.info("ABCH createCpm200OkInviteResponse: target"
                        + dialog.getSipStack().getContact());
            }
            // Create the response
            Response response = SipUtils.MSG_FACTORY.createResponse(
                    200, (Request) dialog.getInvite().getStackMessage());

            // Set the local tag
            ToHeader to = (ToHeader) response.getHeader(ToHeader.NAME);
            to.setTag(dialog.getLocalTag());

            // Set Contact header
            response.addHeader(dialog.getSipStack().getContact());

            // Set feature tags
            SipUtils.setFeatureTags(response, featureTags);

            // Set Allow header
            SipUtils.buildAllowHeader(response);

            // Set the Server header
            response.addHeader(SipUtils.buildCpimServerHeader());

            // Add session timer management
            if (dialog.getSessionExpireTime() >= SessionTimerManager.MIN_EXPIRE_PERIOD) {
                // Set the Require header
                Header requireHeader = SipUtils.HEADER_FACTORY.createHeader(
                        RequireHeader.NAME, "timer");
                response.addHeader(requireHeader);

                // Set Session-Timer header
                Header sessionExpiresHeader = SipUtils.HEADER_FACTORY.createHeader(
                        SipUtils.HEADER_SESSION_EXPIRES, dialog.getSessionExpireTime()
                                + ";refresher=" + dialog.getInvite().getSessionTimerRefresher());
                response.addHeader(sessionExpiresHeader);
            }

             SipUtils.buildRequireHeader(response);

            // Set the message content
            ContentTypeHeader contentTypeHeader = SipUtils.HEADER_FACTORY.createContentTypeHeader(
                    "application", "sdp");
            response.setContent(sdp, contentTypeHeader);

            // Set the message content length
            ContentLengthHeader contentLengthHeader = SipUtils.HEADER_FACTORY
                    .createContentLengthHeader(sdp.getBytes().length);
            response.setContentLength(contentLengthHeader);

            SipResponse resp = new SipResponse(response);

            // Set PAccessNetworkInfo header
            addAccessNetworkInfoHeader(resp);

            //Add Private Header
            addPrivateHeader(resp);

            //Add XATT service Header
            addATTServiceHeader(resp);

            addEarlyMediaHeader(resp);

            resp.setStackTransaction(dialog.getInvite().getStackTransaction());
            return resp;
        } catch (Exception e) {
            if (logger.isActivated()) {
                logger.error("Can't create SIP message", e);
            }
            throw new SipException("Can't create SIP response");
        }
    }

    /**
     * Create a SIP ACK request
     *
     * @param dialog SIP dialog path
     * @return SIP request
     * @throws SipException
     */
    public static SipRequest createAck(SipDialogPath dialog) throws SipException {
        try {
            Request ack = null;

            // Set request line header
            URI requestURI = SipUtils.ADDR_FACTORY.createURI(dialog.getTarget());

            // Set Call-Id header
            CallIdHeader callIdHeader = SipUtils.HEADER_FACTORY.createCallIdHeader(
                    dialog.getCallId());

            // Set the CSeq header
            CSeqHeader cseqHeader = SipUtils.HEADER_FACTORY.createCSeqHeader(
                    dialog.getCseq(), Request.ACK);

            // Set the From header
            Address fromAddress = SipUtils.ADDR_FACTORY.createAddress(dialog.getLocalParty());
            FromHeader fromHeader = SipUtils.HEADER_FACTORY.createFromHeader(
                    fromAddress, dialog.getLocalTag());

            // Set the To header
            Address toAddress = SipUtils.ADDR_FACTORY.createAddress(dialog.getRemoteParty());
            ToHeader toHeader = SipUtils.HEADER_FACTORY.createToHeader(
                    toAddress, dialog.getRemoteTag());

            // Set the Via branch
            ArrayList<ViaHeader> vias = dialog.getSipStack().getViaHeaders();
            vias.get(0).setBranch(Utils.getInstance().generateBranchId());

            // Create the ACK request
            ack = SipUtils.MSG_FACTORY.createRequest(requestURI, Request.ACK, callIdHeader,
                    cseqHeader, fromHeader, toHeader, vias, SipUtils.buildMaxForwardsHeader());

            // Set the Route header
            Vector<String> route = dialog.getRoute();
            for (int i = 0; i < route.size(); i++) {
                Header routeHeader = SipUtils.HEADER_FACTORY.createHeader(RouteHeader.NAME,
                        route.elementAt(i));
                ack.addHeader(routeHeader);
            }


            // Set Require header
            SipUtils.buildRequireHeader(ack);

         // Set the P-Preferred-Identity header
            if (ImsModule.IMS_USER_PROFILE.getPreferredUri() != null) {
                Header prefHeader = SipUtils.HEADER_FACTORY.createHeader(SipUtils.HEADER_P_PREFERRED_IDENTITY, ImsModule.IMS_USER_PROFILE.getPreferredUri());
                ack.addHeader(prefHeader);
            }

            // Set Contact header
            ack.addHeader(dialog.getSipStack().getContact());

            String[] featureTagList = InstantMessagingService.CPM_CHAT_FEATURE_TAGS;
            List<String> tags = Arrays.asList(featureTagList);

         // Set feature tags
            SipUtils.setContactFeatureTags(ack, tags);

            if (RcsSettings.getInstance().isCPMSupported()) {
                // Set User-Agent header
                ack.addHeader(SipUtils.buildCpimUserAgentHeader());
            } else {
                // Set User-Agent header
                ack.addHeader(SipUtils.buildUserAgentHeader());
            }

            // Set Allow header
            SipUtils.buildAllowHeader(ack);

            // Set "rport" (RFC3581)
            ViaHeader viaHeader = (ViaHeader) ack.getHeader(ViaHeader.NAME);
            viaHeader.setRPort();

            return new SipRequest(ack);
        } catch (Exception e) {
            if (logger.isActivated()) {
                logger.error("Can't create SIP message", e);
            }
            throw new SipException("Can't create SIP ACK message");
        }
    }

    /**
     * Create a SIP response
     *
     * @param request SIP request
     * @param code Response code
     * @return SIP response
     * @throws SipException
     */
    public static SipResponse createResponse(SipRequest request, int code) throws SipException {
        try {
            // Create the response
            Response response = SipUtils.MSG_FACTORY.createResponse(code,
                    (Request) request.getStackMessage());
            SipResponse resp = new SipResponse(response);
            resp.setStackTransaction(request.getStackTransaction());
            return resp;
        } catch (Exception e) {
            if (logger.isActivated()) {
                logger.error("Can't create SIP message", e);
            }
            throw new SipException("Can't create SIP response");
        }
    }

    /**
     * Create a SIP response with a specific local tag
     *
     * @param request SIP request
     * @param localTag Local tag
     * @param code Response code
     * @return SIP response
     */
    public static SipResponse createResponse(SipRequest request, String localTag, int code)
            throws SipException {
        try {
            // Create the response
            Response response = SipUtils.MSG_FACTORY.createResponse(
                    code, (Request) request.getStackMessage());

            // Set the local tag
            if (localTag != null) {
                ToHeader to = (ToHeader) response.getHeader(ToHeader.NAME);
                to.setTag(localTag);
            }

            SipResponse resp = new SipResponse(response);
            resp.setStackTransaction(request.getStackTransaction());
            return resp;
        } catch (Exception e) {
            if (logger.isActivated()) {
                logger.error("Can't create SIP message: ", e);
            }
            throw new SipException("Can't create SIP response");
        }
    }

    /**
     * Create a SIP response with a specific local tag
     *
     * @param request SIP request
     * @param localTag Local tag
     * @param code Response code
     * @return SIP response
     */
    public static SipResponse create180Response(SipDialogPath dialog,SipRequest request, String localTag, int code)
            throws SipException {
        try {
            // Create the response
            Response response = SipUtils.MSG_FACTORY.createResponse(
                    code, (Request) request.getStackMessage());

            // Set the local tag
            if (localTag != null) {
                ToHeader to = (ToHeader) response.getHeader(ToHeader.NAME);
                to.setTag(localTag);
            }

            // Set Contact header
            response.addHeader(dialog.getSipStack().getContact());
            // Set Allow header
            SipUtils.buildAllowHeader(response);

            // Set User-Agent header
            response.addHeader(SipUtils.buildUserAgentHeader());

            SipResponse resp = new SipResponse(response);

            resp.setStackTransaction(request.getStackTransaction());
            return resp;
        } catch (Exception e) {
            if (logger.isActivated()) {
                logger.error("Can't create SIP message: ", e);
            }
            throw new SipException("Can't create SIP response");
        }
    }

    /**
     * Create a SIP response with a specific local tag
     *
     * @param request SIP request
     * @param localTag Local tag
     * @param code Response code
     * @return SIP response
     */
    public static SipResponse createCMCCResponse(SipDialogPath dialog, SipRequest request,
            String localTag, int code) throws SipException {
        try {
            // Create the response
            Response response = SipUtils.MSG_FACTORY.createResponse(
                    code, (Request)request.getStackMessage());

            // Set the local tag
            if (localTag != null) {
                ToHeader to = (ToHeader)response.getHeader(ToHeader.NAME);
                to.setTag(localTag);
            }

            // Set Contact header
            response.addHeader(dialog.getSipStack().getContact());

            SipResponse resp = new SipResponse(response);
            resp.setStackTransaction(request.getStackTransaction());
            return resp;
        } catch (Exception e) {
            if (logger.isActivated()) {
                logger.error("Can't create SIP message: ", e);
            }
            throw new SipException("Can't create SIP response");
        }
    }

    /**
     * Create a SIP BYE request
     *
     * @param dialog SIP dialog path
     * @return SIP request
     * @throws SipException
     */
    public static SipRequest createBye(SipDialogPath dialog) throws SipException {
        try {
            // Create the request
            Request bye = dialog.getStackDialog().createRequest(Request.BYE);

            // Set termination reason
            int reasonCode = dialog.getSessionTerminationReasonCode();
            if (reasonCode != -1) {
                ReasonHeader reasonHeader = SipUtils.HEADER_FACTORY.createReasonHeader(
                        "SIP", reasonCode, dialog.getSessionTerminationReasonPhrase());
                bye.addHeader(reasonHeader);
            }

            // Set "rport" (RFC3581)
            ViaHeader viaHeader = (ViaHeader) bye.getHeader(ViaHeader.NAME);
            viaHeader.setRPort();

            return new SipRequest(bye);
        } catch (Exception e) {
            if (logger.isActivated()) {
                logger.error("Can't create SIP message", e);
            }
            throw new SipException("Can't create SIP BYE message");
        }
    }

    /**
     * Create a SIP Group BYE request
     *
     * @param dialog SIP dialog path
     * @return SIP request
     * @throws SipException
     */
    public static SipRequest createGroupBye(SipDialogPath dialog) throws SipException {
        try {
            // Create the request
            Request bye = dialog.getStackDialog().createRequest(Request.BYE);

            /* OP01 spe defines that group related request should use group target as To */
            if (RcsSettings.getInstance().supportOP01()) {
                Dialog stackDialog = dialog.getStackDialog();
                if (!stackDialog.getRemoteParty().equals(stackDialog.getRemoteTarget())) {
                    Address address = SipUtils.ADDR_FACTORY.createAddress(dialog.getTarget());
                    ToHeader header = SipUtils.HEADER_FACTORY.createToHeader(address, dialog.getRemoteTag());
                    bye.setHeader(header);
                }
            }

            // Set termination reason
            int reasonCode = dialog.getSessionTerminationReasonCode();
            if (reasonCode != -1) {
                ReasonHeader reasonHeader = SipUtils.HEADER_FACTORY.createReasonHeader(
                        "SIP", reasonCode, dialog.getSessionTerminationReasonPhrase());
                bye.addHeader(reasonHeader);
            } else {
                ReasonHeader reasonHeader = SipUtils.HEADER_FACTORY.createReasonHeader(
                        "SIP", 200, "Call completed");
                bye.addHeader(reasonHeader);
            }

            // Set "rport" (RFC3581)
            String contact = "<sip:" + dialog.getSipStack().getLocalIpAddress() + ":"
                    + dialog.getSipStack().getListeningPort() + ";transport="
                    + dialog.getSipStack().getProxyProtocol() + ">;" + "*;+group.policy=\"gclose\"";
            // Header contactHeader =
            // SipUtils.HEADER_FACTORY.createHeader(ContactHeader.NAME,
            // "*;+group.policy=\"gclose\"");
            // bye.addHeader(contactHeader);
            bye.addUnparsed("Contact:" + contact);

            // Set "rport" (RFC3581)
            ViaHeader viaHeader = (ViaHeader) bye.getHeader(ViaHeader.NAME);
            viaHeader.setRPort();

            return new SipRequest(bye);
        } catch (Exception e) {
            if (logger.isActivated()) {
                logger.error("Can't create SIP message", e);
            }
            throw new SipException("Can't create SIP BYE message");
        }
    }

    /**
     * Create a SIP CANCEL request
     *
     * @param dialog SIP dialog path
     * @return SIP request
     * @throws SipException
     */
    public static SipRequest createCancel(SipDialogPath dialog) throws SipException {
        try {
            // Create the request
            ClientTransaction transaction = (ClientTransaction) dialog.getInvite()
                    .getStackTransaction();
            Request cancel = transaction.createCancel();

            // Set termination reason
            int reasonCode = dialog.getSessionTerminationReasonCode();
            if (reasonCode != -1) {
                ReasonHeader reasonHeader = SipUtils.HEADER_FACTORY.createReasonHeader(
                        "SIP", reasonCode, dialog.getSessionTerminationReasonPhrase());
                cancel.addHeader(reasonHeader);
            }

            // Set "rport" (RFC3581)
            ViaHeader viaHeader = (ViaHeader) cancel.getHeader(ViaHeader.NAME);
            viaHeader.setRPort();

            return new SipRequest(cancel);
        } catch (Exception e) {
            if (logger.isActivated()) {
                logger.error("Can't create SIP message", e);
            }
            throw new SipException("Can't create SIP BYE message");
        }
    }

    /**
     * Create a SIP OPTIONS request
     *
     * @param dialog SIP dialog path
     * @param featureTags Feature tags
     * @return SIP request
     * @throws SipException
     */
    public static SipRequest createOptions(SipDialogPath dialog, List<String> featureTags)
            throws SipException {
        try {
            // Set request line header
            URI requestURI = SipUtils.ADDR_FACTORY.createURI(dialog.getTarget());

            // Set Call-Id header
            CallIdHeader callIdHeader = SipUtils.HEADER_FACTORY.createCallIdHeader(
                    dialog.getCallId());

            // Set the CSeq header
            CSeqHeader cseqHeader = SipUtils.HEADER_FACTORY.createCSeqHeader(
                    dialog.getCseq(), Request.OPTIONS);

            // Set the From header
            Address fromAddress = SipUtils.ADDR_FACTORY.createAddress(dialog.getLocalParty());
            FromHeader fromHeader = SipUtils.HEADER_FACTORY.createFromHeader(
                    fromAddress, dialog.getLocalTag());

            // Set the To header
            Address toAddress = SipUtils.ADDR_FACTORY.createAddress(dialog.getRemoteParty());
            ToHeader toHeader = SipUtils.HEADER_FACTORY.createToHeader(toAddress, null);

            // Create the request
            Request options = SipUtils.MSG_FACTORY.createRequest(requestURI, Request.OPTIONS,
                    callIdHeader, cseqHeader, fromHeader, toHeader, dialog.getSipStack()
                            .getViaHeaders(), SipUtils.buildMaxForwardsHeader());

            // Set Contact header
            options.addHeader(dialog.getSipStack().getContact());

            // Set Accept header
            Header acceptHeader = SipUtils.HEADER_FACTORY.createHeader(AcceptHeader.NAME,
                    "application/sdp");
            options.addHeader(acceptHeader);

            /*
             * BURN MESSAGE
             */
            boolean isburnSupported = false;
            isburnSupported = featureTags
                    .contains(FeatureTags.FEATURE_CPM_BURNED_MSG);
            if (isburnSupported) {
                SipUtils.setContactFeatureTags(options, featureTags);

                // remove the burn tag
                featureTags.remove(FeatureTags.FEATURE_CPM_BURNED_MSG);
                SipUtils.setAcceptContactFeatureTags(options, featureTags);
            } else {
                // Set feature tags
                SipUtils.setFeatureTags(options, featureTags);

            }
            /*
             * @:end BURN MESSAGE
             */

            // Set Allow header
            SipUtils.buildAllowHeader(options);

            // Set the Route header
            Vector<String> route = dialog.getRoute();
            for (int i = 0; i < route.size(); i++) {
                Header routeHeader = SipUtils.HEADER_FACTORY.createHeader(
                        RouteHeader.NAME, route.elementAt(i));
                options.addHeader(routeHeader);
            }

            // Set the P-Preferred-Identity header
            if (ImsModule.IMS_USER_PROFILE.getPreferredUri() != null) {
                Header prefHeader = SipUtils.HEADER_FACTORY.createHeader(
                        SipUtils.HEADER_P_PREFERRED_IDENTITY,
                        ImsModule.IMS_USER_PROFILE.getPreferredUri());
                options.addHeader(prefHeader);
            }

            // Set User-Agent header
            options.addHeader(SipUtils.buildUserAgentHeader());

            // Set "rport" (RFC3581)
            ViaHeader viaHeader = (ViaHeader) options.getHeader(ViaHeader.NAME);
            viaHeader.setRPort();

            return new SipRequest(options);
        } catch (Exception e) {
            if (logger.isActivated()) {
                logger.error("Can't create SIP message", e);
            }
            throw new SipException("Can't create SIP OPTIONS message");
        }
    }

    /**
     * Create a 200 OK response for OPTIONS request
     *
     * @param options SIP options
     * @param contact Contact header
     * @param featureTags Feature tags
     * @param sdp SDP part
     * @return SIP response
     * @throws SipException
     */
    public static SipResponse create200OkOptionsResponse(SipRequest options, ContactHeader contact,
            List<String> featureTags, String sdp) throws SipException {
        try {
            // Create the response
            Response response = SipUtils.MSG_FACTORY.createResponse(
                    200, (Request) options.getStackMessage());

            // Set the local tag
            ToHeader to = (ToHeader) response.getHeader(ToHeader.NAME);
            to.setTag(IdGenerator.getIdentifier());

            // Set Contact header
            response.addHeader(contact);

            // Set feature tags
            SipUtils.setFeatureTags(response, featureTags);

            // Set Allow header
            SipUtils.buildAllowHeader(response);

            // Set the Server header
            response.addHeader(SipUtils.buildServerHeader());

            // Set the content part if available
            if (sdp != null) {
                // Set the content type header
                ContentTypeHeader contentTypeHeader = SipUtils.HEADER_FACTORY
                        .createContentTypeHeader("application", "sdp");
                response.setContent(sdp, contentTypeHeader);

                // Set the content length header
                ContentLengthHeader contentLengthHeader = SipUtils.HEADER_FACTORY
                        .createContentLengthHeader(sdp.getBytes().length);
                response.setContentLength(contentLengthHeader);
            }

            SipResponse resp = new SipResponse(response);
            resp.setStackTransaction(options.getStackTransaction());
            return resp;
        } catch (Exception e) {
            if (logger.isActivated()) {
                logger.error("Can't create SIP message", e);
            }
            throw new SipException("Can't create SIP response");
        }
    }

    /**
     * Create a SIP REFER request
     *
     * @param dialog SIP dialog path
     * @param toContact Refer to contact
     * @param subject Subject
     * @param contributionId Contribution ID
     * @return SIP request
     * @throws SipException
     */
    public static SipRequest createCpimRefer(SipDialogPath dialog, String toContact,
            String subject, String contributionId) throws SipException {
        try {
            // Create the request
            Request refer = dialog.getStackDialog().createRequest(Request.REFER);

            /* OP01 spe defines that group related request should use group target as To */
            if (RcsSettings.getInstance().supportOP01()) {
                Dialog stackDialog = dialog.getStackDialog();
                if (!stackDialog.getRemoteParty().equals(stackDialog.getRemoteTarget())) {
                    Address address = SipUtils.ADDR_FACTORY.createAddress(dialog.getTarget());
                    ToHeader header = SipUtils.HEADER_FACTORY.createToHeader(address, dialog.getRemoteTag());
                    refer.setHeader(header);
                }
            } else {
                // Set the To header
                Address toAddress = SipUtils.ADDR_FACTORY.createAddress(dialog.getRemoteParty());
                ToHeader toHeader = SipUtils.HEADER_FACTORY.createToHeader(
                        toAddress, dialog.getRemoteTag());
                refer.setHeader(toHeader);
            }

            // Set feature tags
            // String[] tags ;
            if (!RcsSettings.getInstance().isCPMSupported()) {
                String[] tags = { FeatureTags.FEATURE_OMA_IM };
                SipUtils.setFeatureTags(refer, tags);
            } else {
                String[] tags = { FeatureTags.FEATURE_RCSE_CPM_SESSION };
                SipUtils.setFeatureTags(refer, tags);
            }

            // Set Refer-To header
            String newToContact = "<" + toContact + ";method=INVITE" + ">";
            Header referTo = null;
            if (!RcsSettings.getInstance().supportOP01()) {
                referTo = SipUtils.HEADER_FACTORY.createHeader(ReferToHeader.NAME, toContact);
            } else {
                referTo = SipUtils.HEADER_FACTORY.createHeader(ReferToHeader.NAME, newToContact);
            }
            refer.addHeader(referTo);

            // Set Refer-Sub header
            Header referSub = SipUtils.HEADER_FACTORY.createHeader(
                    SipUtils.HEADER_REFER_SUB, "false");
            refer.addHeader(referSub);

            // Set the P-Preferred-Identity header
            if (ImsModule.IMS_USER_PROFILE.getPreferredUri() != null) {
                Header prefHeader = SipUtils.HEADER_FACTORY.createHeader(
                        SipUtils.HEADER_P_PREFERRED_IDENTITY,
                        ImsModule.IMS_USER_PROFILE.getPreferredUri());
                refer.addHeader(prefHeader);
            }

            // Set the preferred service header
            Header preferredServiceHeader = SipUtils.HEADER_FACTORY.createHeader(
                    SipUtils.HEADER_P_PREFERRED_SERVICE, FeatureTags.FEATURE_CPM_URN_SESSION);
            refer.addHeader(preferredServiceHeader);

            // Set Subject header
            if (subject != null) {
                if (RcsSettings.getInstance().supportOP01()) {
                    subject = URLEncoder.encode(subject, "UTF-8");
                }
                Header sub = SipUtils.HEADER_FACTORY.createHeader(Subject.NAME, subject);
                refer.addHeader(sub);
            }

            // Set Contribution-ID header
            Header cid = SipUtils.HEADER_FACTORY.createHeader(
                    ChatUtils.HEADER_CONTRIBUTION_ID, contributionId);
            refer.addHeader(cid);

            // Set User-Agent header
            refer.addHeader(SipUtils.buildCpimUserAgentHeader());

            // Set "rport" (RFC3581)
            ViaHeader viaHeader = (ViaHeader) refer.getHeader(ViaHeader.NAME);
            viaHeader.setRPort();

            // Add remote SIP instance ID
            SipUtils.setRemoteInstanceID(refer, dialog.getRemoteSipInstance());

            return new SipRequest(refer);
        } catch (Exception e) {
            if (logger.isActivated()) {
                logger.error("Can't create SIP message", e);
            }
            throw new SipException("Can't create SIP REFER message");
        }
    }

    /**
     * Create a SIP REFER request
     *
     * @param dialog SIP dialog path
     * @param participants List of participants
     * @param subject Subject
     * @param contributionId Contribution ID
     * @return SIP request
     * @throws SipException
     */
    public static SipRequest createCpimRefer(SipDialogPath dialog, List<String> participants,
            String subject, String contributionId) throws SipException {
        try {
            // Create the request
            Request refer = dialog.getStackDialog().createRequest(Request.REFER);

            /* OP01 spe defines that group related request should use group target as To */
            if (RcsSettings.getInstance().supportOP01()) {
                Dialog stackDialog = dialog.getStackDialog();
                if (!stackDialog.getRemoteParty().equals(stackDialog.getRemoteTarget())) {
                    Address address = SipUtils.ADDR_FACTORY.createAddress(dialog.getTarget());
                    ToHeader header = SipUtils.HEADER_FACTORY.createToHeader(address, dialog.getRemoteTag());
                    refer.setHeader(header);
                }
            }

            // Generate a list URI
            String listID = "Id_" + System.currentTimeMillis();

            String[] tags = { FeatureTags.FEATURE_RCSE_CPM_SESSION };
            SipUtils.setFeatureTags(refer, tags);

            // Set Require header
            Header require = SipUtils.HEADER_FACTORY.createHeader(RequireHeader.NAME,
                    "multiple-refer");
            refer.addHeader(require);
            require = SipUtils.HEADER_FACTORY.createHeader(RequireHeader.NAME, "norefersub");
            refer.addHeader(require);

            // Set Refer-To header
            String referToString = "<cid:" + listID + "@"
                    + ImsModule.IMS_USER_PROFILE.getHomeDomain() + ";method=INVITE" + ">";
            Header referTo = null;
            if (!RcsSettings.getInstance().supportOP01()) {
                referTo = SipUtils.HEADER_FACTORY.createHeader(ReferToHeader.NAME, "<cid:" + listID
                        + "@" + ImsModule.IMS_USER_PROFILE.getHomeDomain() + ">");
            } else {
                referTo = SipUtils.HEADER_FACTORY.createHeader(ReferToHeader.NAME, referToString);
            }
            refer.addHeader(referTo);

            // Set Refer-Sub header
            Header referSub = SipUtils.HEADER_FACTORY.createHeader(
                    SipUtils.HEADER_REFER_SUB, "false");
            refer.addHeader(referSub);

            // Set the P-Preferred-Identity header
            if (ImsModule.IMS_USER_PROFILE.getPreferredUri() != null) {
                Header prefHeader = SipUtils.HEADER_FACTORY.createHeader(
                        SipUtils.HEADER_P_PREFERRED_IDENTITY,
                        ImsModule.IMS_USER_PROFILE.getPreferredUri());
                refer.addHeader(prefHeader);
            }

            // Set the preferred service header
            Header preferredServiceHeader = SipUtils.HEADER_FACTORY.createHeader(
                    SipUtils.HEADER_P_PREFERRED_SERVICE, FeatureTags.FEATURE_CPM_URN_SESSION);
            refer.addHeader(preferredServiceHeader);

            // Set Subject header
            if (RcsSettings.getInstance().supportOP01()) {
                subject = URLEncoder.encode(subject, "UTF-8");
            }
            Header s = SipUtils.HEADER_FACTORY.createHeader(Subject.NAME, subject);
            refer.addHeader(s);

            // Set Contribution-ID header
            Header cid = SipUtils.HEADER_FACTORY.createHeader(
                    ChatUtils.HEADER_CONTRIBUTION_ID, contributionId);
            refer.addHeader(cid);

            // Set User-Agent header
            refer.addHeader(SipUtils.buildCpimUserAgentHeader());

            // Set the Content-ID header
            Header contentIdHeader = SipUtils.HEADER_FACTORY.createHeader(
                    SipUtils.HEADER_CONTENT_ID,
                    "<" + listID + "@" + ImsModule.IMS_USER_PROFILE.getHomeDomain() + ">");
            refer.addHeader(contentIdHeader);

            // Generate the resource list for given participants
            String resourceList = ChatUtils.generateChatResourceList(participants);

            // Set the message content
            ContentTypeHeader contentTypeHeader = SipUtils.HEADER_FACTORY.createContentTypeHeader(
                    "application", "resource-lists+xml");
            refer.setContent(resourceList, contentTypeHeader);

            // Set the message content length
            ContentLengthHeader contentLengthHeader = SipUtils.HEADER_FACTORY
                    .createContentLengthHeader(resourceList.getBytes().length);
            refer.setContentLength(contentLengthHeader);

            // Set the Content-Disposition header
            Header contentDispoHeader = SipUtils.HEADER_FACTORY.createHeader(
                    ContentDispositionHeader.NAME, "recipient-list");
            refer.addHeader(contentDispoHeader);

            // Set "rport" (RFC3581)
            ViaHeader viaHeader = (ViaHeader) refer.getHeader(ViaHeader.NAME);
            viaHeader.setRPort();

            // Add remote SIP instance ID
            SipUtils.setRemoteInstanceID(refer, dialog.getRemoteSipInstance());

            return new SipRequest(refer);
        } catch (Exception e) {
            if (logger.isActivated()) {
                logger.error("Can't create SIP message", e);
            }
            throw new SipException("Can't create SIP REFER message");
        }
    }

    /**
     * Create a SIP REFER request
     *
     * @param dialog SIP dialog path
     * @param toContact Refer to contact
     * @param subject Subject
     * @param contributionId Contribution ID
     * @return SIP request
     * @throws SipException
     */
    public static SipRequest createRefer(SipDialogPath dialog, String toContact, String subject,
            String contributionId) throws SipException {
        try {
            // Create the request
            Request refer = dialog.getStackDialog().createRequest(Request.REFER);

            /* OP01 spec defines that group related request should use group target as To */
            if (RcsSettings.getInstance().supportOP01()) {
                Dialog stackDialog = dialog.getStackDialog();
                if (!stackDialog.getRemoteParty().equals(stackDialog.getRemoteTarget())) {
                    Address address = SipUtils.ADDR_FACTORY.createAddress(dialog.getTarget());
                    ToHeader header = SipUtils.HEADER_FACTORY.createToHeader(address, dialog.getRemoteTag());
                    refer.setHeader(header);
                }
            }

            // Set feature tags
            String[] tags = { FeatureTags.FEATURE_OMA_IM };
            SipUtils.setFeatureTags(refer, tags);

            // Set Refer-To header
            Header referTo = SipUtils.HEADER_FACTORY.createHeader(ReferToHeader.NAME, toContact);
            refer.addHeader(referTo);

            // Set Refer-Sub header
            Header referSub = SipUtils.HEADER_FACTORY.createHeader(SipUtils.HEADER_REFER_SUB,
                    "false");
            refer.addHeader(referSub);

            // Set the P-Preferred-Identity header
            if (ImsModule.IMS_USER_PROFILE.getPreferredUri() != null) {
                Header prefHeader = SipUtils.HEADER_FACTORY.createHeader(
                        SipUtils.HEADER_P_PREFERRED_IDENTITY,
                        ImsModule.IMS_USER_PROFILE.getPreferredUri());
                refer.addHeader(prefHeader);
            }

            // Set Subject header
            if (subject != null) {
                Header sub = SipUtils.HEADER_FACTORY.createHeader(Subject.NAME, subject);
                refer.addHeader(sub);
            }

            // Set Contribution-ID header
            Header cid = SipUtils.HEADER_FACTORY.createHeader(ChatUtils.HEADER_CONTRIBUTION_ID,
                    contributionId);
            refer.addHeader(cid);

            // Set User-Agent header
            refer.addHeader(SipUtils.buildUserAgentHeader());

            // Set "rport" (RFC3581)
            ViaHeader viaHeader = (ViaHeader) refer.getHeader(ViaHeader.NAME);
            viaHeader.setRPort();

            // Add remote SIP instance ID
            SipUtils.setRemoteInstanceID(refer, dialog.getRemoteSipInstance());

            return new SipRequest(refer);
        } catch (Exception e) {
            if (logger.isActivated()) {
                logger.error("Can't create SIP message", e);
            }
            throw new SipException("Can't create SIP REFER message");
        }
    }

    /**
     * Create a SIP REFER request for Modify Subject
     *
     * @param dialog SIP dialog path
     * @param toContact Refer to contact
     * @param newSubject new Subject to modify
     * @param contributionId Contribution ID
     * @return SIP request
     * @throws SipException
     */
    public static SipRequest createReferModifySubject(SipDialogPath dialog, String toContact,
            String newSubject, String contributionId) throws SipException {
        try {
            // Create the request
            Request refer = dialog.getStackDialog().createRequest(Request.REFER);

            /* OP01 spec defines that group related request should use group target as To */
            if (RcsSettings.getInstance().supportOP01()) {
                Dialog stackDialog = dialog.getStackDialog();
                if (!stackDialog.getRemoteParty().equals(stackDialog.getRemoteTarget())) {
                    Address address = SipUtils.ADDR_FACTORY.createAddress(dialog.getTarget());
                    ToHeader header = SipUtils.HEADER_FACTORY.createToHeader(address, dialog.getRemoteTag());
                    refer.setHeader(header);
                }
            }

            if (RcsSettings.getInstance().supportOP01()) {
                SipUtils.setFeatureTags(refer, ChatUtils.getCpimSupportedFeatureTagsForGroupChat(true));
            } else {
                // Set feature tags
                String[] tags = { FeatureTags.FEATURE_RCSE_CPM_SESSION };
                SipUtils.setFeatureTags(refer, tags);
            }

            String newToContact = "<" + toContact + ";method=SUBJECT" + ">";

            // Set Refer-To header
            Header referTo = SipUtils.HEADER_FACTORY.createHeader(ReferToHeader.NAME, newToContact);
            refer.addHeader(referTo);

            // Set Refer-Sub header
            Header referSub = SipUtils.HEADER_FACTORY.createHeader(
                    SipUtils.HEADER_REFER_SUB, "false");
            refer.addHeader(referSub);

            // Set the P-Preferred-Identity header
            if (ImsModule.IMS_USER_PROFILE.getPreferredUri() != null) {
                Header prefHeader = SipUtils.HEADER_FACTORY.createHeader(
                        SipUtils.HEADER_P_PREFERRED_IDENTITY,
                        ImsModule.IMS_USER_PROFILE.getPreferredUri());
                refer.addHeader(prefHeader);
            }

            // Set Subject header
            if (newSubject != null) {
                if (RcsSettings.getInstance().supportOP01()) {
                    newSubject = URLEncoder.encode(newSubject, "UTF-8");
                }
                Header sub = SipUtils.HEADER_FACTORY.createHeader(Subject.NAME, newSubject);
                refer.addHeader(sub);
            }

            // Set Contribution-ID header
            Header cid = SipUtils.HEADER_FACTORY.createHeader(
                    ChatUtils.HEADER_CONTRIBUTION_ID, contributionId);
            refer.addHeader(cid);

            // Set User-Agent header
            refer.addHeader(SipUtils.buildUserAgentHeader());

            // Set "rport" (RFC3581)
            ViaHeader viaHeader = (ViaHeader) refer.getHeader(ViaHeader.NAME);
            viaHeader.setRPort();

            // Add remote SIP instance ID
            SipUtils.setRemoteInstanceID(refer, dialog.getRemoteSipInstance());

            return new SipRequest(refer);
        } catch (Exception e) {
            if (logger.isActivated()) {
                logger.error("Can't create SIP message", e);
            }
            throw new SipException("Can't create SIP REFER message");
        }
    }

    /**
     * Create a SIP REFER request for Modify NickName
     *
     * @param dialog SIP dialog path
     * @param toContact Refer to contact
     * @param newNickname new Nickname to modify
     * @param contributionId Contribution ID
     * @return SIP request
     * @throws SipException
     */
    public static SipRequest createReferModifyNickname(SipDialogPath dialog, String toContact,
            String newNickname, String contributionId) throws SipException {
        try {
            // Create the request
            Request refer = dialog.getStackDialog().createRequest(Request.REFER);

            /* OP01 spec defines that group related request should use group target as To */
            if (RcsSettings.getInstance().supportOP01()) {
                Dialog stackDialog = dialog.getStackDialog();
                if (!stackDialog.getRemoteParty().equals(stackDialog.getRemoteTarget())) {
                    Address address = SipUtils.ADDR_FACTORY.createAddress(dialog.getTarget());
                    ToHeader header = SipUtils.HEADER_FACTORY.createToHeader(address, dialog.getRemoteTag());
                    refer.setHeader(header);
                }
            }

            // Set feature tags
            String[] tags = { FeatureTags.FEATURE_RCSE_CPM_SESSION };
            SipUtils.setFeatureTags(refer, tags);

            // Set Refer-To header
            newNickname = URLEncoder.encode(newNickname, "UTF-8");
            String newToContact = "<" + dialog.getTarget() + ";method=MUNN" + ">" + ";nn=" + newNickname;

            Header referTo = SipUtils.HEADER_FACTORY.createHeader(ReferToHeader.NAME, newToContact);
            refer.addHeader(referTo);

            /* Special characters might contains in newNickName, just bypass stack parser */
//          refer.addUnparsed(ReferToHeader.NAME + ": " + newToContact);

            // Set Refer-Sub header
            Header referSub = SipUtils.HEADER_FACTORY.createHeader(
                    SipUtils.HEADER_REFER_SUB, "false");
            refer.addHeader(referSub);

            // Set the P-Preferred-Identity header
            if (ImsModule.IMS_USER_PROFILE.getPreferredUri() != null) {
                Header prefHeader = SipUtils.HEADER_FACTORY.createHeader(
                        SipUtils.HEADER_P_PREFERRED_IDENTITY,
                        ImsModule.IMS_USER_PROFILE.getPreferredUri());
                refer.addHeader(prefHeader);
            }

            // Set Subject header
            /*
             * if (subject != null) { Header sub =
             * SipUtils.HEADER_FACTORY.createHeader(Subject.NAME, subject);
             * refer.addHeader(sub); }
             */

            // Set Contribution-ID header
            Header cid = SipUtils.HEADER_FACTORY.createHeader(
                    ChatUtils.HEADER_CONTRIBUTION_ID, contributionId);
            refer.addHeader(cid);

            // Set User-Agent header
            refer.addHeader(SipUtils.buildUserAgentHeader());

            // Set "rport" (RFC3581)
            ViaHeader viaHeader = (ViaHeader) refer.getHeader(ViaHeader.NAME);
            viaHeader.setRPort();

            // Add remote SIP instance ID
            SipUtils.setRemoteInstanceID(refer, dialog.getRemoteSipInstance());

            return new SipRequest(refer);
        } catch (Exception e) {
            if (logger.isActivated()) {
                logger.error("Can't create SIP message", e);
            }
            throw new SipException("Can't create SIP REFER message");
        }
    }

    /**
     * Create a SIP REFER request for transfer Chairman
     *
     * @param dialog SIP dialog path
     * @param oldChairman old chairman
     * @param newChairman new chairman
     * @param contributionId Contribution ID
     * @return SIP request
     * @throws SipException
     */
    public static SipRequest createReferRemoveParticipant(SipDialogPath dialog, String chairman,
            String participantToRemove, String contributionId) throws SipException {
        try {
            // Create the request
            Request refer = dialog.getStackDialog().createRequest(Request.REFER);

            /* OP01 spec defines that group related request should use group target as To */
            if (RcsSettings.getInstance().supportOP01()) {
                Dialog stackDialog = dialog.getStackDialog();
                if (!stackDialog.getRemoteParty().equals(stackDialog.getRemoteTarget())) {
                    Address address = SipUtils.ADDR_FACTORY.createAddress(dialog.getTarget());
                    ToHeader header = SipUtils.HEADER_FACTORY.createToHeader(address, dialog.getRemoteTag());
                    refer.setHeader(header);
                }
            }

            // Set feature tags
            String[] tags = { FeatureTags.FEATURE_RCSE_CPM_SESSION };
            SipUtils.setFeatureTags(refer, tags);

            String newToContact = "<" + participantToRemove + ";method=BYE" + ">";

            // Set Refer-To header
            Header referTo = SipUtils.HEADER_FACTORY.createHeader(ReferToHeader.NAME, newToContact);
            refer.addHeader(referTo);

            // Set Refer-Sub header
            Header referSub = SipUtils.HEADER_FACTORY.createHeader(
                    SipUtils.HEADER_REFER_SUB, "false");
            refer.addHeader(referSub);

            // Set the P-Preferred-Identity header
            if (ImsModule.IMS_USER_PROFILE.getPreferredUri() != null) {
                Header prefHeader = SipUtils.HEADER_FACTORY.createHeader(
                        SipUtils.HEADER_P_PREFERRED_IDENTITY,
                        ImsModule.IMS_USER_PROFILE.getPreferredUri());
                refer.addHeader(prefHeader);
            }

            /*
             * // Set Subject header if (newSubject != null) { Header sub =
             * SipUtils.HEADER_FACTORY.createHeader(Subject.NAME, newSubject);
             * refer.addHeader(sub); }
             */

            // Set Contribution-ID header
            Header cid = SipUtils.HEADER_FACTORY.createHeader(
                    ChatUtils.HEADER_CONTRIBUTION_ID, contributionId);
            refer.addHeader(cid);

            // Set User-Agent header
            refer.addHeader(SipUtils.buildUserAgentHeader());

            // Set "rport" (RFC3581)
            ViaHeader viaHeader = (ViaHeader) refer.getHeader(ViaHeader.NAME);
            viaHeader.setRPort();

            // Add remote SIP instance ID
            SipUtils.setRemoteInstanceID(refer, dialog.getRemoteSipInstance());

            return new SipRequest(refer);
        } catch (Exception e) {
            if (logger.isActivated()) {
                logger.error("Can't create SIP message", e);
            }
            throw new SipException("Can't create SIP REFER message");
        }
    }

    /**
     * Create a SIP REFER request for transfer Chairman
     *
     * @param dialog SIP dialog path
     * @param oldChairman old chairman
     * @param newChairman new chairman
     * @param contributionId Contribution ID
     * @return SIP request
     * @throws SipException
     */
    public static SipRequest createReferTransferChairman(SipDialogPath dialog, String oldChairman,
            String newChairman, String contributionId) throws SipException {
        try {
            // Create the request
            Request refer = dialog.getStackDialog().createRequest(Request.REFER);

            /* OP01 spec defines that group related request should use group target as To */
            if (RcsSettings.getInstance().supportOP01()) {
                Dialog stackDialog = dialog.getStackDialog();
                if (!stackDialog.getRemoteParty().equals(stackDialog.getRemoteTarget())) {
                    Address address = SipUtils.ADDR_FACTORY.createAddress(dialog.getTarget());
                    ToHeader header = SipUtils.HEADER_FACTORY.createToHeader(address, dialog.getRemoteTag());
                    refer.setHeader(header);
                }
            }

            // Set feature tags
            /*String[] tags = {FeatureTags.FEATURE_RCSE_CPM_SESSION};
            SipUtils.setFeatureTags(refer, tags);*/
            newChairman = "\"" + newChairman + "\"";
            String newToContact = "<" +dialog.getTarget() + ";method=SETPLC" + ">" + ";plccmd=gaccede;plcvalue=" + newChairman;

            // Set Refer-To header
            Header referTo = SipUtils.HEADER_FACTORY.createHeader(ReferToHeader.NAME, newToContact);
            refer.addHeader(referTo);

            /* Special characters might contained in newChairman, just bypass stack parser */
//            refer.addUnparsed(ReferToHeader.NAME + ": " + newToContact);

            // Set Refer-Sub header
            Header referSub = SipUtils.HEADER_FACTORY.createHeader(
                    SipUtils.HEADER_REFER_SUB, "false");
            refer.addHeader(referSub);

            // Set the P-Preferred-Identity header
            if (ImsModule.IMS_USER_PROFILE.getPreferredUri() != null) {
                Header prefHeader = SipUtils.HEADER_FACTORY.createHeader(
                        SipUtils.HEADER_P_PREFERRED_IDENTITY,
                        ImsModule.IMS_USER_PROFILE.getPreferredUri());
                refer.addHeader(prefHeader);
            }

            /*
             * // Set Subject header if (newSubject != null) { Header sub =
             * SipUtils.HEADER_FACTORY.createHeader(Subject.NAME, newSubject);
             * refer.addHeader(sub); }
             */

            // Set Contribution-ID header
            Header cid = SipUtils.HEADER_FACTORY.createHeader(
                    ChatUtils.HEADER_CONTRIBUTION_ID, contributionId);
            refer.addHeader(cid);

            String headerString = "*;" + FeatureTags.FEATURE_RCSE_CPM_SESSION + ";"
                    + FeatureTags.FEATURE_CMCC_IARI_GROUP_MANAGE;
            Header acceptContactHeader = SipUtils.HEADER_FACTORY.createHeader(
                    SipUtils.HEADER_ACCEPT_CONTACT, headerString);
            refer.addHeader(acceptContactHeader);
            // Set User-Agent header
            refer.addHeader(SipUtils.buildUserAgentHeader());

            // Set "rport" (RFC3581)
            ViaHeader viaHeader = (ViaHeader) refer.getHeader(ViaHeader.NAME);
            viaHeader.setRPort();

            // Add remote SIP instance ID
            SipUtils.setRemoteInstanceID(refer, dialog.getRemoteSipInstance());

            return new SipRequest(refer);
        } catch (Exception e) {
            if (logger.isActivated()) {
                logger.error("Can't create SIP message", e);
            }
            throw new SipException("Can't create SIP REFER message");
        }
    }

    /**
     * Create a SIP REFER request
     *
     * @param dialog SIP dialog path
     * @param participants List of participants
     * @param subject Subject
     * @param contributionId Contribution ID
     * @return SIP request
     * @throws SipException
     */
    public static SipRequest createRefer(SipDialogPath dialog, List<String> participants,
            String subject, String contributionId) throws SipException {
        try {
            // Create the request
            Request refer = dialog.getStackDialog().createRequest(Request.REFER);

            /* OP01 spe defines that group related request should use group target as To */
            if (RcsSettings.getInstance().supportOP01()) {
                Dialog stackDialog = dialog.getStackDialog();
                if (!stackDialog.getRemoteParty().equals(stackDialog.getRemoteTarget())) {
                    Address address = SipUtils.ADDR_FACTORY.createAddress(dialog.getTarget());
                    ToHeader header = SipUtils.HEADER_FACTORY.createToHeader(address, dialog.getRemoteTag());
                    refer.setHeader(header);
                }
            }

            // Generate a list URI
            String listID = "Id_" + System.currentTimeMillis();

            // Set feature tags
            String[] tags = { FeatureTags.FEATURE_OMA_IM };
            SipUtils.setFeatureTags(refer, tags);

            // Set Require header
            Header require = SipUtils.HEADER_FACTORY.createHeader(
                    RequireHeader.NAME, "multiple-refer");
            refer.addHeader(require);
            require = SipUtils.HEADER_FACTORY.createHeader(RequireHeader.NAME, "norefersub");
            refer.addHeader(require);

            // Set Refer-To header
            Header referTo = SipUtils.HEADER_FACTORY.createHeader(ReferToHeader.NAME, "<cid:"
                    + listID + "@" + ImsModule.IMS_USER_PROFILE.getHomeDomain() + ">");
            refer.addHeader(referTo);

            // Set Refer-Sub header
            Header referSub = SipUtils.HEADER_FACTORY.createHeader(
                    SipUtils.HEADER_REFER_SUB, "false");
            refer.addHeader(referSub);

            // Set the P-Preferred-Identity header
            if (ImsModule.IMS_USER_PROFILE.getPreferredUri() != null) {
                Header prefHeader = SipUtils.HEADER_FACTORY.createHeader(
                        SipUtils.HEADER_P_PREFERRED_IDENTITY,
                        ImsModule.IMS_USER_PROFILE.getPreferredUri());
                refer.addHeader(prefHeader);
            }

            // Set Subject header
            Header s = SipUtils.HEADER_FACTORY.createHeader(Subject.NAME, subject);
            refer.addHeader(s);

            // Set Contribution-ID header
            Header cid = SipUtils.HEADER_FACTORY.createHeader(
                    ChatUtils.HEADER_CONTRIBUTION_ID, contributionId);
            refer.addHeader(cid);

            // Set User-Agent header
            refer.addHeader(SipUtils.buildUserAgentHeader());

            // Set the Content-ID header
            Header contentIdHeader = SipUtils.HEADER_FACTORY.createHeader(
                    SipUtils.HEADER_CONTENT_ID,
                    "<" + listID + "@" + ImsModule.IMS_USER_PROFILE.getHomeDomain() + ">");
            refer.addHeader(contentIdHeader);

            // Generate the resource list for given participants
            String resourceList = ChatUtils.generateChatResourceList(participants);

            // Set the message content
            ContentTypeHeader contentTypeHeader = SipUtils.HEADER_FACTORY.createContentTypeHeader(
                    "application", "resource-lists+xml");
            refer.setContent(resourceList, contentTypeHeader);

            // Set the message content length
            ContentLengthHeader contentLengthHeader = SipUtils.HEADER_FACTORY
                    .createContentLengthHeader(resourceList.getBytes().length);
            refer.setContentLength(contentLengthHeader);

            // Set the Content-Disposition header
            Header contentDispoHeader = SipUtils.HEADER_FACTORY.createHeader(
                    ContentDispositionHeader.NAME, "recipient-list");
            refer.addHeader(contentDispoHeader);

            // Set "rport" (RFC3581)
            ViaHeader viaHeader = (ViaHeader) refer.getHeader(ViaHeader.NAME);
            viaHeader.setRPort();

            // Add remote SIP instance ID
            SipUtils.setRemoteInstanceID(refer, dialog.getRemoteSipInstance());

            return new SipRequest(refer);
        } catch (Exception e) {
            if (logger.isActivated()) {
                logger.error("Can't create SIP message", e);
            }
            throw new SipException("Can't create SIP REFER message");
        }
    }

    /**
     * Create a SIP RE-INVITE request
     *
     * @param dialog SIP dialog path
     * @return SIP request
     * @throws SipException
     */
    public static SipRequest createReInvite(SipDialogPath dialog) throws SipException {
        try {
            // Build the request
            Request reInvite = dialog.getStackDialog().createRequest(Request.INVITE);
            SipRequest firstInvite = dialog.getInvite();

            // Set feature tags
            reInvite.removeHeader(ContactHeader.NAME);
            reInvite.addHeader(firstInvite.getHeader(ContactHeader.NAME));
            reInvite.removeHeader(SipUtils.HEADER_ACCEPT_CONTACT);
            reInvite.addHeader(firstInvite.getHeader(SipUtils.HEADER_ACCEPT_CONTACT));

            // Set Allow header
            SipUtils.buildAllowHeader(reInvite);

            // Set the Route header
            reInvite.addHeader(firstInvite.getHeader(RouteHeader.NAME));

            // Set the P-Preferred-Identity header
            reInvite.addHeader(firstInvite.getHeader(SipUtils.HEADER_P_PREFERRED_IDENTITY));

            // Set User-Agent header
            reInvite.addHeader(firstInvite.getHeader(UserAgentHeader.NAME));

            // Add session timer management
            if (dialog.getSessionExpireTime() >= SessionTimerManager.MIN_EXPIRE_PERIOD) {
                // Set the Supported header
                Header supportedHeader = SipUtils.HEADER_FACTORY.createHeader(SupportedHeader.NAME,
                        "timer");
                reInvite.addHeader(supportedHeader);

                // Set Session-Timer headers
                Header sessionExpiresHeader = SipUtils.HEADER_FACTORY.createHeader(
                        SipUtils.HEADER_SESSION_EXPIRES, "" + dialog.getSessionExpireTime());
                reInvite.addHeader(sessionExpiresHeader);
            }

            // Set "rport" (RFC3581)
            ViaHeader viaHeader = (ViaHeader) reInvite.getHeader(ViaHeader.NAME);
            viaHeader.setRPort();

            // Add remote SIP instance ID
            SipUtils.setRemoteInstanceID(firstInvite.getStackMessage(),
                    dialog.getRemoteSipInstance());

            return new SipRequest(reInvite);
        } catch (Exception e) {
            if (logger.isActivated()) {
                logger.error("Can't create SIP message", e);
            }
            throw new SipException("Can't create SIP RE-INVITE message");
        }
    }

    /**
     * Create a SIP RE-INVITE request with content using initial Invite request
     *
     * @param dialog Dialog path SIP request
     * @param featureTags featureTags to set in request
     * @param content sdp content
     * @return SIP request
     * @throws SipException
     */
    public static SipRequest createReInvite(SipDialogPath dialog, String[] featureTags,
            String content) throws SipException {
        try {
            // Build the request
            Request reInvite = dialog.getStackDialog().createRequest(Request.INVITE);
            SipRequest firstInvite = dialog.getInvite();

            // Set the CSeq header
            CSeqHeader cseqHeader = SipUtils.HEADER_FACTORY.createCSeqHeader(dialog.getCseq(),
                    Request.INVITE);
            reInvite.removeHeader(CSeqHeader.NAME);
            reInvite.addHeader(cseqHeader);

            // Set Contact header
            reInvite.removeHeader(ContactHeader.NAME);
            reInvite.removeHeader(SipUtils.HEADER_ACCEPT_CONTACT);
            reInvite.addHeader(dialog.getSipStack().getContact());

            // Set feature tags
            SipUtils.setFeatureTags(reInvite, featureTags);

            // Add remote SIP instance ID
            SipUtils.setRemoteInstanceID(firstInvite.getStackMessage(),
                    dialog.getRemoteSipInstance());

            // Set Allow header
            SipUtils.buildAllowHeader(reInvite);

            // Set the Route header
            if (reInvite.getHeader(RouteHeader.NAME) == null
                    && firstInvite.getHeader(RouteHeader.NAME) != null) {
                reInvite.addHeader(firstInvite.getHeader(RouteHeader.NAME));
            }

            // Set the P-Preferred-Identity header
            if (firstInvite.getHeader(SipUtils.HEADER_P_PREFERRED_IDENTITY) != null) {
                reInvite.addHeader(firstInvite.getHeader(SipUtils.HEADER_P_PREFERRED_IDENTITY));
            } else if (ImsModule.IMS_USER_PROFILE.getPreferredUri() != null) {
                Header prefHeader = SipUtils.HEADER_FACTORY.createHeader(
                        SipUtils.HEADER_P_PREFERRED_IDENTITY,
                        ImsModule.IMS_USER_PROFILE.getPreferredUri());
                reInvite.addHeader(prefHeader);
            }

            // Set User-Agent header
            reInvite.addHeader(firstInvite.getHeader(UserAgentHeader.NAME));

            // Add session timer management
            if (dialog.getSessionExpireTime() >= SessionTimerManager.MIN_EXPIRE_PERIOD) {
                // Set the Supported header
                Header supportedHeader = SipUtils.HEADER_FACTORY.createHeader(SupportedHeader.NAME,
                        "timer");
                reInvite.addHeader(supportedHeader);

                // Set Session-Timer headers
                Header sessionExpiresHeader = SipUtils.HEADER_FACTORY.createHeader(
                        SipUtils.HEADER_SESSION_EXPIRES, "" + dialog.getSessionExpireTime());
                reInvite.addHeader(sessionExpiresHeader);
            }

            // Set "rport" (RFC3581)
            ViaHeader viaHeader = (ViaHeader) reInvite.getHeader(ViaHeader.NAME);
            viaHeader.setRPort();

            // Create the content type and set content
            ContentTypeHeader contentType = SipUtils.HEADER_FACTORY.createContentTypeHeader(
                    "application", "sdp");
            reInvite.setContent(content, contentType);

            // Set the content length
            ContentLengthHeader contentLengthHeader = SipUtils.HEADER_FACTORY
                    .createContentLengthHeader(content.getBytes().length);
            reInvite.setContentLength(contentLengthHeader);
            return new SipRequest(reInvite);
        } catch (Exception e) {
            if (logger.isActivated()) {
                logger.error("Can't create SIP message", e);
            }
            throw new SipException("Can't create SIP RE-INVITE message");
        }

    }

    /**
     * Create a SIP response for RE-INVITE request
     *
     * @param dialog Dialog path SIP request
     * @param request SIP request
     * @return SIP response
     * @throws SipException
     */
    public static SipResponse create200OkReInviteResponse(SipDialogPath dialog, SipRequest request)
            throws SipException {
        try {
            // Create the response
            Response response = SipUtils.MSG_FACTORY.createResponse(200,
                    (Request) request.getStackMessage());

            // Set Contact header
            response.addHeader(dialog.getSipStack().getContact());

            // Set the Server header
            response.addHeader(SipUtils.buildServerHeader());

            // Set the Require header
            Header requireHeader = SipUtils.HEADER_FACTORY
                    .createHeader(RequireHeader.NAME, "timer");
            response.addHeader(requireHeader);

            // Add Session-Timer header
            Header sessionExpiresHeader = request.getHeader(SipUtils.HEADER_SESSION_EXPIRES);
            if (sessionExpiresHeader != null) {
                response.addHeader(sessionExpiresHeader);
            }

            SipResponse resp = new SipResponse(response);
            resp.setStackTransaction(request.getStackTransaction());
            return resp;
        } catch (Exception e) {
            if (logger.isActivated()) {
                logger.error("Can't create SIP message", e);
            }
            throw new SipException("Can't create SIP response");
        }
    }

    /**
     * Create a SIP response for RE-INVITE request
     *
     * @param dialog Dialog path SIP request
     * @param request SIP request
     * @param featureTags featureTags to set in request
     * @param content SDP content
     * @return SIP response
     * @throws SipException
     */
    public static SipResponse create200OkReInviteResponse(SipDialogPath dialog, SipRequest request,
            String[] featureTags, String content) throws SipException {
        try {
            // Create the response
            Response response = SipUtils.MSG_FACTORY.createResponse(200,
                    (Request) request.getStackMessage());

            // Set the local tag
            ToHeader to = (ToHeader) response.getHeader(ToHeader.NAME);
            to.setTag(dialog.getLocalTag());

            // Set Contact header
            response.addHeader(dialog.getSipStack().getContact());

            // Set feature tags
            SipUtils.setFeatureTags(response, featureTags);

            // Set Allow header
            SipUtils.buildAllowHeader(response);

            // Set the Server header
            response.addHeader(SipUtils.buildServerHeader());

            // Add session timer management
            if (dialog.getSessionExpireTime() >= SessionTimerManager.MIN_EXPIRE_PERIOD) {
                // Set the Require header
                Header requireHeader = SipUtils.HEADER_FACTORY.createHeader(RequireHeader.NAME,
                        "timer");
                response.addHeader(requireHeader);

                // Set Session-Timer header
                Header sessionExpiresHeader = SipUtils.HEADER_FACTORY.createHeader(
                        SipUtils.HEADER_SESSION_EXPIRES, dialog.getSessionExpireTime()
                                + ";refresher=" + dialog.getInvite().getSessionTimerRefresher());
                response.addHeader(sessionExpiresHeader);
            }

            // Set the message content
            ContentTypeHeader contentTypeHeader = SipUtils.HEADER_FACTORY.createContentTypeHeader(
                    "application", "sdp");
            response.setContent(content, contentTypeHeader);

            // Set the message content length
            ContentLengthHeader contentLengthHeader = SipUtils.HEADER_FACTORY
                    .createContentLengthHeader(content.getBytes().length);
            response.setContentLength(contentLengthHeader);

            SipResponse resp = new SipResponse(response);
            resp.setStackTransaction(request.getStackTransaction());
            return resp;
        } catch (Exception e) {
            if (logger.isActivated()) {
                logger.error("Can't create SIP message", e);
            }
            throw new SipException("Can't create SIP response");
        }
    }

    /**
     * Create a SIP UPDATE request
     *
     * @param dialog SIP dialog path
     * @return SIP request
     * @throws SipException
     */
    public static SipRequest createUpdate(SipDialogPath dialog) throws SipException {
        try {
            // Create the request
            Request update = dialog.getStackDialog().createRequest(Request.UPDATE);

            // Set the Supported header
            Header supportedHeader = SipUtils.HEADER_FACTORY.createHeader(SupportedHeader.NAME,
                    "timer");
            update.addHeader(supportedHeader);

            // Add Session-Timer header
            Header sessionExpiresHeader = SipUtils.HEADER_FACTORY.createHeader(
                    SipUtils.HEADER_SESSION_EXPIRES, "" + dialog.getSessionExpireTime());
            update.addHeader(sessionExpiresHeader);

            // Set "rport" (RFC3581)
            ViaHeader viaHeader = (ViaHeader) update.getHeader(ViaHeader.NAME);
            viaHeader.setRPort();

            return new SipRequest(update);
        } catch (Exception e) {
            if (logger.isActivated()) {
                logger.error("Can't create SIP message", e);
            }
            throw new SipException("Can't create SIP UPDATE message");
        }
    }

    /**
     * Create a SIP response for UPDATE request
     *
     * @param dialog Dialog path SIP request
     * @param request SIP request
     * @return SIP response
     * @throws SipException
     */
    public static SipResponse create200OkUpdateResponse(SipDialogPath dialog, SipRequest request)
            throws SipException {
        try {
            // Create the response
            Response response = SipUtils.MSG_FACTORY.createResponse(200,
                    (Request) request.getStackMessage());

            // Set Contact header
            response.addHeader(dialog.getSipStack().getContact());

            // Set the Server header
            response.addHeader(SipUtils.buildServerHeader());

            // Set the Require header
            Header requireHeader = SipUtils.HEADER_FACTORY
                    .createHeader(RequireHeader.NAME, "timer");
            response.addHeader(requireHeader);

            // Add Session-Timer header
            Header sessionExpiresHeader = request.getHeader(SipUtils.HEADER_SESSION_EXPIRES);
            if (sessionExpiresHeader != null) {
                response.addHeader(sessionExpiresHeader);
            }

            SipResponse resp = new SipResponse(response);
            resp.setStackTransaction(request.getStackTransaction());
            return resp;
        } catch (Exception e) {
            if (logger.isActivated()) {
                logger.error("Can't create SIP message", e);
            }
            throw new SipException("Can't create SIP response");
        }
    }

/**
	 * Add P-Access-Network-Info and P-Last-Access-Network-Info Headers
	 *
	 * @param sipMessage
	 *            The target sip message who will be added headers.
	 * @return The sip message that have be added access information headers,if
	 *         the message is CANCEL response it will be returned without access
	 *         headers.
	 * @throws SipException
	 */
	public static SipMessage addAccessNetworkInfoHeader(SipMessage sipMessage)
			throws SipException {
		if (sipMessage == null) {
			logger.info("The sipMessage is null.");
			return null;
		} else if (sipMessage instanceof SipResponse) {
			SipResponse sipResponse = (SipResponse) sipMessage;
			String cSeqMethodName = ((CSeqHeader) sipResponse
					.getHeader(CSeqHeader.NAME)).getMethod();
			if (Request.CANCEL.equals(cSeqMethodName)) {
				return sipResponse;
			}
		}
       try {

			// if the current access network information is empty,no need to add
			String accessNetworkInfo = ImsModule.IMS_USER_PROFILE.getAccessNetworkInfo();
			if (!TextUtils.isEmpty(accessNetworkInfo)) {
				sipMessage.addHeader(SipUtils.HEADER_P_ACCESS_NETWORK_INFO,accessNetworkInfo);
			} else {
				if (logger.isActivated()) {
					logger.info("There is no current access network infomation.");
				}
			}

			// if the last access network information is empty,no need to add
			String lastAccessNetworkInfo = ImsModule.IMS_USER_PROFILE.getLastAccessNetworkInfo();
			if (!TextUtils.isEmpty(lastAccessNetworkInfo)) {
				sipMessage.addHeader(
						SipUtils.HEADER_P_LAST_ACCESS_NETWORK_INFO,
						lastAccessNetworkInfo);
			} else {
				if (logger.isActivated()) {
					logger.info("There is no last access network infomation.");
               }
           }

    } catch (Exception e) {
        if (logger.isActivated()) {
				logger.error("Can't add the SIP header", e);
        }
			throw new SipException("Can't add the SIP header");
    }
		return sipMessage;
   }

	public static SipMessage addPrivateHeader(SipMessage sipMessage) throws SipException {
	    if(RcsSettings.getInstance().isSupportOP07()){
            try {
                if (logger.isActivated()) {
                    logger.info("addPrivateHeader");
                }
                if (sipMessage == null) {
                    logger.info("The sipMessage is null.");
                    return null;
                } else if (sipMessage instanceof SipResponse) {
                    SipResponse sipResponse = (SipResponse) sipMessage;
                    String cSeqMethodName = ((CSeqHeader) sipResponse.getHeader(CSeqHeader.NAME)).getMethod();
                    if (Request.CANCEL.equals(cSeqMethodName)) {
                        return sipResponse;
                    }
                }
                if (logger.isActivated()) {
                    logger.info("addPrivateHeader");
                }
                sipMessage.addHeader(SipUtils.HEADER_PRIVATE,"none");
            } catch (Exception e) {
                e.printStackTrace();
            }
	    }
        return sipMessage;
   }

	public static SipMessage addEarlyMediaHeader(SipMessage sipMessage) throws SipException {
        if(RcsSettings.getInstance().isSupportOP07()){
            try {
                if (logger.isActivated()) {
                    logger.info("addEarlyMediaHeader");
                }
                if (sipMessage == null) {
                    logger.info("The sipMessage is null.");
                    return null;
                } else if (sipMessage instanceof SipResponse) {
                    SipResponse sipResponse = (SipResponse) sipMessage;
                    String cSeqMethodName = ((CSeqHeader) sipResponse.getHeader(CSeqHeader.NAME)).getMethod();
                    if (Request.CANCEL.equals(cSeqMethodName)) {
                        return sipResponse;
                    }
                }
                if (logger.isActivated()) {
                    logger.info("addEarlyMediaHeader");
                }
                sipMessage.addHeader(SipUtils.HEADER_P_Early_Media,"supported ");
            } catch (Exception e) {
                e.printStackTrace();
            }
        }
        return sipMessage;
   }

	public static SipMessage addATTServiceHeader(SipMessage sipMessage) throws SipException {
        if(RcsSettings.getInstance().isSupportOP07()){
            try {
                if (logger.isActivated()) {
                    logger.info("addATTServiceHeader");
                }
                if (sipMessage == null) {
                    logger.info("The sipMessage is null.");
                    return null;
                } else if (sipMessage instanceof SipResponse) {
                    SipResponse sipResponse = (SipResponse) sipMessage;
                    String cSeqMethodName = ((CSeqHeader) sipResponse.getHeader(CSeqHeader.NAME)).getMethod();
                    if (Request.CANCEL.equals(cSeqMethodName)) {
                        return sipResponse;
                    }
                }
                if (logger.isActivated()) {
                    logger.info("addATTServiceHeader");
                }
                sipMessage.addHeader(SipUtils.HEADER_X_ATT_SERVICE,"messaging");
            } catch (Exception e) {
                e.printStackTrace();
            }
        }
        return sipMessage;
   }

}
