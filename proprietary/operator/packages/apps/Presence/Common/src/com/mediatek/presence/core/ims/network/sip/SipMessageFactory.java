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
 /*
 * Copyright (C) 2018 MediaTek Inc., this file is modified on 07/05/2018
 * by MediaTek Inc. based on Apache License, Version 2.0.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may not
 * use this file except in compliance with the License. See NOTICE for more details.
 */

package com.mediatek.presence.core.ims.network.sip;

import gov2.nist.core.NameValue;
import gov2.nist.javax2.sip.Utils;
import gov2.nist.javax2.sip.header.Subject;

import java.net.URLEncoder;
import java.util.ArrayList;
import java.util.HashSet;
import java.util.List;
import java.util.Vector;
import java.util.Arrays;

import android.content.Context;
import android.net.ConnectivityManager;
import android.net.NetworkInfo;
import android.net.NetworkCapabilities;
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;
import android.text.TextUtils;
import com.mediatek.presence.core.ims.protocol.sip.SipMessage;

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
import gov2.nist.javax2.sip.address.GenericURI;
import gov2.nist.javax2.sip.header.ims.PrivacyHeader;
import gov2.nist.javax2.sip.header.RequestLine;
import gov2.nist.javax2.sip.header.ParameterNames;

import com.mediatek.presence.core.ims.ImsModule;
import com.mediatek.presence.core.ims.network.registration.RegistrationManager;
import com.mediatek.presence.core.ims.protocol.sip.PLastAccessNetworkInfoHeader;
import gov.nist.javax.sip.header.ims.PAccessNetworkInfoHeader;
import com.mediatek.presence.core.ims.protocol.sip.SipDialogPath;
import com.mediatek.presence.core.ims.protocol.sip.SipException;
import com.mediatek.presence.core.ims.protocol.sip.SipRequest;
import com.mediatek.presence.core.ims.protocol.sip.SipResponse;
import com.mediatek.presence.core.ims.service.capability.VzwCapabilityService;
import com.mediatek.presence.core.ims.service.SessionTimerManager;
import com.mediatek.presence.core.ims.service.im.chat.ChatUtils;
import com.mediatek.presence.platform.AndroidFactory;
import com.mediatek.presence.provider.settings.RcsSettings;
import com.mediatek.presence.provider.settings.RcsSettingsManager;
import com.mediatek.presence.core.ims.rcsua.RcsUaAdapter;
import com.mediatek.presence.core.ims.rcsua.RcsUaAdapterManager;
import com.mediatek.presence.utils.DeviceUtils;
import com.mediatek.presence.utils.IdGenerator;
import com.mediatek.presence.utils.PhoneUtils;
import com.mediatek.presence.utils.logger.Logger;
import com.mediatek.presence.utils.SimUtils;

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



    /**
     * Create a SIP REGISTER request
     *
     * @param slotId slot index
     * @param dialog SIP dialog path
     * @param featureTags Feature tags
     * @param expirePeriod Expiration period
     * @param instanceId UA SIP instance ID
     * @return SIP request
     * @throws SipException
     */
    public static SipRequest createRegister(int slotId, SipDialogPath dialog, List<String> featureTags,
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
                //viaHeader.setParameter(new NameValue("keep", null, true));
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

            if (RcsSettingsManager.getRcsSettingsInstance(slotId)
                    .isCPMBurnAfterReadingSupported()) {
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

            /*
            if (SimUtils.isCmccSimCard(slotId)) {
                Context context = AndroidFactory.getApplicationContext();
                ConnectivityManager connMgr = (ConnectivityManager)context.getSystemService(
                        Context.CONNECTIVITY_SERVICE);
                NetworkInfo info = connMgr.getActiveNetworkInfo();
                if (info != null && info.isConnected()) {
                    Header header = null;
                    if (ConnectivityManager.isNetworkTypeMobile(info.getType())) {
                        int[] subIds = SubscriptionManager.getSubId(slotId);
                        TelephonyManager telMgr = TelephonyManager.from(context).createForSubscriptionId(subIds[0]);
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
            */

            // Set User-Agent header
            register.addHeader(SipUtils.buildUserAgentHeader(slotId));

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
     * @param slotId slot index
     * @param dialog SIP dialog path
     * @param expirePeriod Expiration period
     * @return SIP request
     * @throws SipException
     */
    public static SipRequest createSubscribe(int slotId, SipDialogPath dialog, int expirePeriod)
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
            subscribe.addHeader(SipUtils.buildUserAgentHeader(slotId));

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
     * @param slotId slot index
     * @param dialog SIP dialog path
     * @param expirePeriod Expiration period
     * @return SIP request
     * @throws SipException
     */
    public static SipRequest createEABIndividualSubscribe(
            int slotId, int type, SipDialogPath dialog, int expirePeriod, String contact) throws SipException {
        try {
            // Set request line header
            String contactUri;
            if (type == VzwCapabilityService.SINGLE_SUBSCRIBE_TYPE_AVAILABILITY) {
                contactUri = PhoneUtils.formatNumberToSipUri(contact);
            } else {
                contactUri = PhoneUtils.formatNumberToTelUri(contact);
            }
            URI requestURI = SipUtils.ADDR_FACTORY.createURI(contactUri);

            // Set Call-Id header
            CallIdHeader callIdHeader = SipUtils.HEADER_FACTORY.createCallIdHeader(dialog.getCallId());

            // Set the CSeq header
            CSeqHeader cseqHeader = SipUtils.HEADER_FACTORY.createCSeqHeader(dialog.getCseq(), Request.SUBSCRIBE);

            // Set the From header
            String localParty = dialog.getLocalParty();
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

            // Set Contact header
            /*Address contactAddress = SipUtils.ADDR_FACTORY.createAddress(dialog.getTarget());
            ContactHeader contactHeader = SipUtils.HEADER_FACTORY.createContactHeader(contactAddress);
            subscribe.addHeader(contactHeader);*/
            subscribe.addHeader(dialog.getSipStack().getContact());

            // Set the Event header
            Header presenceHeader = SipUtils.HEADER_FACTORY.createHeader(EventHeader.NAME, "presence");
            subscribe.addHeader(presenceHeader);

            // Set the Accept header
            String acceptHeaderValue = "application/pidf+xml";
            Header acceptdHeader = SipUtils.HEADER_FACTORY.createHeader(AcceptHeader.NAME, acceptHeaderValue);
            subscribe.addHeader(acceptdHeader);

            ExpiresHeader expHeader = SipUtils.HEADER_FACTORY.createExpiresHeader(expirePeriod);
            subscribe.addHeader(expHeader);

            Header encondingHeader = SipUtils.HEADER_FACTORY.createHeader("Accept-Encoding", "gzip");
            subscribe.addHeader(encondingHeader);

            // Set Require header
            SipUtils.buildRequireHeader(subscribe);

            // Set Allow header
            SipUtils.buildAllowHeader(subscribe);

            //subscribe.addHeader(SipUtils.buildNetworkAccessHeader());

            // Set User-Agent header
            subscribe.addHeader(SipUtils.buildUserAgentHeader(slotId));

           // Set the Route header
            Vector<String> route = dialog.getRoute();
            for(int i=0; i < route.size(); i++) {
                Header routeHeader = SipUtils.HEADER_FACTORY.createHeader(RouteHeader.NAME, route.elementAt(i));
                subscribe.addHeader(routeHeader);
            }

            SipRequest sipRequest = new SipRequest(subscribe);
            return sipRequest;
        } catch(Exception e) {
            if (logger.isActivated()) {
                logger.error("Can't create SIP message", e);
            }
            throw new SipException("Can't create SIP SUBSCRIBE message");
        }
    }

    public static SipRequest createEABListSubscribe(
            int slotId, SipDialogPath dialog, int expirePeriod ,byte[] xml) throws SipException {
        String presList= "dummy-rfc5367";

        try {
            // Set request line header
            URI requestURI = SipUtils.ADDR_FACTORY.createURI(dialog.getTarget());
            RequestLine reqLine = new RequestLine();
            reqLine.setUri((GenericURI) requestURI);
            reqLine.setPresList(presList);

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
            toHeader.setParameter(new NameValue(ParameterNames.PRES_LIST, presList));

            // Create the request
            Request subscribe = SipUtils.MSG_FACTORY.createRequest(reqLine,
                    Request.SUBSCRIBE,
                    callIdHeader,
                    cseqHeader,
                    fromHeader,
                    toHeader,
                    dialog.getSipStack().getViaHeaders(),
                    SipUtils.buildMaxForwardsHeader());

            // Set Contact header
            /*Address contactAddress = SipUtils.ADDR_FACTORY.createAddress(dialog.getTarget());
            ContactHeader contactHeader = SipUtils.HEADER_FACTORY.createContactHeader(contactAddress);
            subscribe.addHeader(contactHeader);*/
            subscribe.addHeader(dialog.getSipStack().getContact());

            // Set the Event header
            Header presenceHeader = SipUtils.HEADER_FACTORY.createHeader(EventHeader.NAME, "presence");
            subscribe.addHeader(presenceHeader);

            String acceptHeaderValue = "application/pidf+xml, application/pidf-diff+xml, multipart/related, application/rlmi+xml, application/simple-filter+xml";

            // Set the Accept header
            Header acceptdHeader = SipUtils.HEADER_FACTORY.createHeader(AcceptHeader.NAME, acceptHeaderValue);
            subscribe.addHeader(acceptdHeader);

            // Set the Expires header
            ExpiresHeader expHeader = SipUtils.HEADER_FACTORY.createExpiresHeader(expirePeriod);
            subscribe.addHeader(expHeader);

            // Set Require header
            SipUtils.buildPresenceRequireHeader(subscribe);

            //Set Support header
            Header supportedHeader = SipUtils.HEADER_FACTORY.createHeader(SupportedHeader.NAME, "eventlist");
            subscribe.addHeader(supportedHeader);

            // Set Accpet-Encoding header
            Header encondingHeader = SipUtils.HEADER_FACTORY.createHeader("Accept-Encoding", "gzip");
            subscribe.addHeader(encondingHeader);

            // Set Accpet-Encoding header
            Header contentEncondingHeader = SipUtils.HEADER_FACTORY.createHeader("Content-Encoding", "gzip");
            subscribe.addHeader(contentEncondingHeader);

            // Set the message content
            if (xml != null) {
                ContentTypeHeader contentTypeHeader = SipUtils.HEADER_FACTORY.createContentTypeHeader("application", "resource-lists+xml");
                subscribe.setContent(xml, contentTypeHeader);
            }

            // Set Allow header
            SipUtils.buildAllowHeader(subscribe);

            // Set User-Agent header
            subscribe.addHeader(SipUtils.buildUserAgentHeader(slotId));

           // Set the Route header
            Vector<String> route = dialog.getRoute();
            for(int i=0; i < route.size(); i++) {
                Header routeHeader = SipUtils.HEADER_FACTORY.createHeader(RouteHeader.NAME, route.elementAt(i));
                subscribe.addHeader(routeHeader);
            }

            // Set the message content length
            int length = 0;
            if (xml != null) {
                length = xml.length;
            }
            ContentLengthHeader contentLengthHeader = SipUtils.HEADER_FACTORY.createContentLengthHeader(length);
            subscribe.setContentLength(contentLengthHeader);

             /**
             * M:add P-Access-Network-Info and P-Last-Access-Network-Info
             * Headers. @{T-Mobile
             */
            SipRequest sipRequest = new SipRequest(subscribe);
            //sipRequest = (SipRequest) addAccessNetworkInfoHeader(sipRequest);
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
     * Create a Individual SIP SUBSCRIBE request
     *
     * @param slotId slot index
     * @param dialog SIP dialog path
     * @param expirePeriod Expiration period
     * @return SIP request
     * @throws SipException
     */
    public static SipRequest createIndividualPresenceSubscribe(int slotId, SipDialogPath dialog,
            int expirePeriod , String contact) throws SipException {
        try {
            // Set request line header
            String contactUri;
            if (SimUtils.isAttSimCard(slotId)) {
                contactUri = PhoneUtils.formatNumberToTelUri(contact);
            } else {
                contactUri = PhoneUtils.formatNumberToSipUri(contact);
            }

            URI requestURI = SipUtils.ADDR_FACTORY.createURI(contactUri);

            // Set Call-Id header
            CallIdHeader callIdHeader = SipUtils.HEADER_FACTORY.createCallIdHeader(dialog.getCallId());

            // Set the CSeq header
            CSeqHeader cseqHeader = SipUtils.HEADER_FACTORY.createCSeqHeader(dialog.getCseq(), Request.SUBSCRIBE);

            // Set the From header
            String localParty = dialog.getLocalParty();
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
            subscribe.addHeader(SipUtils.buildUserAgentHeader(slotId));


            // Set the Expires header
            if (SimUtils.isAttSimCard(slotId)) {
                //CDR-UCE-1450 from AT&T requirement.
                expirePeriod = 0;
            }
            ExpiresHeader expHeader = SipUtils.HEADER_FACTORY.createExpiresHeader(expirePeriod);
            subscribe.addHeader(expHeader);

            // Set the Event header
            Header presenceHeader = SipUtils.HEADER_FACTORY.createHeader(EventHeader.NAME, "presence");
            subscribe.addHeader(presenceHeader);


            Header acceptContactHeader = SipUtils.HEADER_FACTORY.createHeader("Accept-Contact", "*;+g.oma.sip-im;explicit;require");
            subscribe.addHeader(acceptContactHeader);
            //Header acceptContactHeader = HEADER_FACTORY.createHeader();

            Header encondingHeader = SipUtils.HEADER_FACTORY.createHeader("Accept-Encoding", "gzip");
            subscribe.addHeader(encondingHeader);


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

            //check if video capab supported
            logger.debug("createIndividualPresenceSubscribe - isIR94VideoCallSupported() = "
                    + RcsSettingsManager.getRcsSettingsInstance(slotId).isIR94VideoCallSupported());
            if(RcsSettingsManager.getRcsSettingsInstance(slotId).isIR94VideoCallSupported()){
                try {
                    String vilteString  = "" +FeatureTags.FEATURE_RCSE_IP_VIDEO_CALL;
                    ContactHeader contactHeader = (ContactHeader)subscribe.getHeader(SipUtils.HEADER_CONTACT_);
                    contactHeader.setParameter(new NameValue(vilteString, null, true));
                    if (SimUtils.isAttSimCard(slotId)) {
                        contactHeader.setParameter(new NameValue("mobility=\"mobile\"", null, true));
                    }
                } catch (Exception e) {
                    // TODO Auto-generated catch block
                    e.printStackTrace();
                }
            }

            if (SimUtils.isAttSimCard(slotId)) {
                Header privacyHeader = SipUtils.HEADER_FACTORY.createHeader(PrivacyHeader.NAME, "id");
                subscribe.addHeader(privacyHeader);

                String PANI = SipUtils.getPAccessNetworkInfo(slotId,
                    RcsUaAdapterManager.getRcsUaAdapter(slotId).isIMSViaWFC() ? NetworkCapabilities.TRANSPORT_WIFI :
                    NetworkCapabilities.TRANSPORT_CELLULAR);
                if (PANI != null) {
                    if (logger.isActivated()) {
                        logger.debug("P-Access-Network-Info:" + PANI);
                    }
                    Header PANIHeader = SipUtils.buildAccessNetworkInfo(PANI);
                    subscribe.addHeader(PANIHeader);
                }
            }

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


             /**
             * M:add P-Access-Network-Info and P-Last-Access-Network-Info
             * Headers. @{T-Mobile
             */
            SipRequest sipRequest = new SipRequest(subscribe);
            //sipRequest = (SipRequest) addAccessNetworkInfoHeader(sipRequest);
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
     * @param slotId slot index
     * @param dialog SIP dialog path
     * @param expirePeriod Expiration period
     * @return SIP request
     * @throws SipException
     */
    public static SipRequest createPresenceSubscribe(int slotId, SipDialogPath dialog,
            int expirePeriod , String xml) throws SipException {
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
            subscribe.addHeader(SipUtils.buildUserAgentHeader(slotId));

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

            // Set P-Preferred-Association
            if (SimUtils.isTmoSimCard(slotId) && !TextUtils.isEmpty(RcsUaAdapterManager.getRcsUaAdapter(slotId).getPpa())) {
                Header ppaHeader = SipUtils.HEADER_FACTORY.createHeader(SipUtils.HEADER_P_PREFERRED_ASSOCIATION, RcsUaAdapterManager.getRcsUaAdapter(slotId).getPpa());
                subscribe.addHeader(ppaHeader);
            }

            //check if video capab supported
            logger.debug("createPresenceSubscribe - isIR94VideoCallSupported() = " + RcsSettingsManager.getRcsSettingsInstance(slotId)
                    .isIR94VideoCallSupported());
            if(RcsSettingsManager.getRcsSettingsInstance(slotId).isIR94VideoCallSupported()){
                try {
                    String vilteString  = "" +FeatureTags.FEATURE_RCSE_IP_VIDEO_CALL;
                    ContactHeader contactHeader = (ContactHeader)subscribe.getHeader(SipUtils.HEADER_CONTACT_);
                    contactHeader.setParameter(new NameValue(vilteString, null, true));
                    if (SimUtils.isAttSimCard(slotId)) {
                        contactHeader.setParameter(new NameValue("mobility=\"mobile\"", null, true));
                    }
                } catch (Exception e) {
                    // TODO Auto-generated catch block
                    e.printStackTrace();
                }
            }


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

            if (SimUtils.isAttSimCard(slotId)) {
                Header privacyHeader = SipUtils.HEADER_FACTORY.createHeader(PrivacyHeader.NAME, "id");
                subscribe.addHeader(privacyHeader);

                String PANI = SipUtils.getPAccessNetworkInfo(slotId,
                    RcsUaAdapterManager.getRcsUaAdapter(slotId).isIMSViaWFC() ? NetworkCapabilities.TRANSPORT_WIFI :
                    NetworkCapabilities.TRANSPORT_CELLULAR);
                if (PANI != null) {
                    if (logger.isActivated()) {
                        logger.debug("P-Access-Network-Info:" + PANI);
                    }
                    Header PANIHeader = SipUtils.buildAccessNetworkInfo(PANI);
                    subscribe.addHeader(PANIHeader);
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
            //sipRequest = (SipRequest) addAccessNetworkInfoHeader(sipRequest);
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
     * @param slotId slot index
     * @param dialog SIP dialog path
     * @param expirePeriod Expiration period
     * @return SIP request
     * @throws SipException
     */
    public static SipRequest createCpimSubscribe(int slotId, SipDialogPath dialog, int expirePeriod)
            throws SipException {

        return createCpimSubscribe(slotId, dialog, expirePeriod, null, null);
    }

    public static SipRequest createCpimSubscribe(
            int slotId, SipDialogPath dialog, int expirePeriod, String contentType, byte[] content)
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
                // Set the preferred service header
                Header allowEvents = SipUtils.HEADER_FACTORY
                        .createAllowEventsHeader("presence,presence.winfo");
                subscribe.addHeader(allowEvents);
            } catch (Exception e) {
                e.printStackTrace();
            }

            // Set the Expires header
            ExpiresHeader expHeader = SipUtils.HEADER_FACTORY.createExpiresHeader(expirePeriod);
            subscribe.addHeader(expHeader);

            // Set User-Agent header
            subscribe.addHeader(SipUtils.buildCpimUserAgentHeader(slotId));

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
     * @param slotId slot index
     * @param dialog SIP dialog path
     * @param contentType Content type
     * @param content Content
     * @return SIP request
     * @throws SipException
     */
    public static SipRequest createMessage(int slotId, SipDialogPath dialog, String contentType, String content)
            throws SipException {
        return createMessage(slotId, dialog, null, contentType, content.getBytes());
    }

    /**
     * Create a SIP MESSAGE request with a feature tag
     *
     * @param slotId slot index
     * @param dialog SIP dialog path
     * @param contentType Content type
     * @param content Content
     * @return SIP request
     * @throws SipException
     */
    public static SipRequest createCpimMessage(int slotId, SipDialogPath dialog, String contentType,
            String content) throws SipException {
        return createMessage(slotId, dialog, null, contentType, content.getBytes());
    }

    /**
     * Create a SIP MESSAGE request with a feature tag
     *
     * @param slotId slot index
     * @param dialog SIP dialog path
     * @param featureTag Feature tag
     * @param contentType Content type
     * @param content Content
     * @return SIP request
     * @throws SipException
     */
    public static SipRequest createMessage(int slotId, SipDialogPath dialog, String featureTag,
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
            message.addHeader(SipUtils.buildUserAgentHeader(slotId));

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
     * Create a SIP PUBLISH request
     *
     * @param slotId slot index
     * @param dialog SIP dialog path
     * @param expirePeriod Expiration period
     * @param entityTag Entity tag
     * @param sdp SDP part
     * @return SIP request
     * @throws SipException
     */
    public static SipRequest createPublish(int slotId, SipDialogPath dialog, int expirePeriod,
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

            // Set the Route header
            Vector<String> route = dialog.getRoute();
            for (int i = 0; i < route.size(); i++) {
                Header routeHeader = SipUtils.HEADER_FACTORY.createHeader(RouteHeader.NAME,
                        route.elementAt(i));
                publish.addHeader(routeHeader);
            }

            // Set the Expires header
            ExpiresHeader expHeader = SipUtils.HEADER_FACTORY.createExpiresHeader(expirePeriod);
            if (SimUtils.isAttSimCard(slotId) || SimUtils.isTmoSimCard(slotId)) {
                if (expirePeriod == 0)
                    publish.addHeader(expHeader);
            } else {
                publish.addHeader(expHeader);
            }

            // Set the SIP-If-Match header
            if (entityTag != null) {
                Header sipIfMatchHeader = SipUtils.HEADER_FACTORY.createHeader(
                        SIPIfMatchHeader.NAME, entityTag);
                publish.addHeader(sipIfMatchHeader);
            }

            // Set User-Agent header
            publish.addHeader(SipUtils.buildUserAgentHeader(slotId));

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

            // Set P-Preferred-Association
            if (SimUtils.isTmoSimCard(slotId) && !TextUtils.isEmpty(RcsUaAdapterManager.getRcsUaAdapter(slotId).getPpa())) {
                Header ppaHeader = SipUtils.HEADER_FACTORY.createHeader(SipUtils.HEADER_P_PREFERRED_ASSOCIATION, RcsUaAdapterManager.getRcsUaAdapter(slotId).getPpa());
                publish.addHeader(ppaHeader);
            }

            /*
             * PROXY-REQUIRE  / REQUIRE STARTS
             */
            try {
                ContactHeader contactHeader = (ContactHeader) publish.getHeader(ContactHeader.NAME);
                if (SimUtils.isAttSimCard(slotId)) {
                    contactHeader.setParameter(new NameValue("mobility=\"mobile\"", null, true));
                }
            } catch (Exception e) {
                // TODO Auto-generated catch block
                e.printStackTrace();
            }


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
            if (SimUtils.isAttSimCard(slotId) || SimUtils.isVzwSimCard(slotId)) {
                String PANI = SipUtils.getPAccessNetworkInfo(slotId,
                    RcsUaAdapterManager.getRcsUaAdapter(slotId).isIMSViaWFC() ? NetworkCapabilities.TRANSPORT_WIFI :
                    NetworkCapabilities.TRANSPORT_CELLULAR);
                if (PANI != null) {
                    if (logger.isActivated()) {
                        logger.debug("P-Access-Network-Info:" + PANI);
                    }
                    Header PANIHeader = SipUtils.buildAccessNetworkInfo(PANI);
                    publish.addHeader(PANIHeader);
                }
            }

            return new SipRequest(publish);
        } catch (Exception e) {
            if (logger.isActivated()) {
                logger.error("Can't create SIP message", e);
            }
            throw new SipException("Can't create SIP PUBLISH message");
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
     * @param slotId slot index
     * @param dialog SIP dialog path
     * @return SIP request
     * @throws SipException
     */
    public static SipRequest createAck(int slotId, SipDialogPath dialog) throws SipException {
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

            String[] featureTagList = null;//InstantMessagingService.CPM_CHAT_FEATURE_TAGS;
            List<String> tags = Arrays.asList(featureTagList);

            // Set feature tags
            SipUtils.setContactFeatureTags(ack, tags);

            if (RcsSettingsManager.getRcsSettingsInstance(slotId).isCPMSupported()) {
                // Set User-Agent header
                ack.addHeader(SipUtils.buildCpimUserAgentHeader(slotId));
            } else {
                // Set User-Agent header
                ack.addHeader(SipUtils.buildUserAgentHeader(slotId));
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
     * @param slotId slot index
     * @param dialog SIP dialog path
     * @return SIP request
     * @throws SipException
     */
    public static SipRequest createGroupBye(int slotId, SipDialogPath dialog) throws SipException {
        try {
            // Create the request
            Request bye = dialog.getStackDialog().createRequest(Request.BYE);

            /* OP01 spe defines that group related request should use group target as To */
            if (SimUtils.isCmccSimCard(slotId)) {
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
     * @param slotId slot index
     * @param dialog SIP dialog path
     * @param featureTags Feature tags
     * @return SIP request
     * @throws SipException
     */
    public static SipRequest createOptions(int slotId, SipDialogPath dialog, List<String> featureTags)
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
            options.addHeader(SipUtils.buildUserAgentHeader(slotId));

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
     * Create a response for OPTIONS request (AOSP procedure)
     *
     * @param options SIP options
     * @param contact Contact header
     * @param featureTags Feature tags
     * @param sdp SDP part
     * @param sipResponseCode SIP response code the UE needs to share to network
     * @param reasonPharse response phrase corresponding to the response code
     * @return SIP response
     * @throws SipException
     */
    public static SipResponse createOptionsResponse(SipRequest options, ContactHeader contact,
            List<String> featureTags, String sdp, int sipResponseCode,
            String reasonPharse) throws SipException {
        try {
            // Create the response
            Response response = SipUtils.MSG_FACTORY.createResponse(
                    sipResponseCode, (Request) options.getStackMessage());

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
            resp.getStackMessage().setReasonPhrase(reasonPharse);
            return resp;
        } catch (Exception e) {
            if (logger.isActivated()) {
                logger.error("Can't create SIP message", e);
            }
            throw new SipException("Can't create SIP response");
        }
    }

}
