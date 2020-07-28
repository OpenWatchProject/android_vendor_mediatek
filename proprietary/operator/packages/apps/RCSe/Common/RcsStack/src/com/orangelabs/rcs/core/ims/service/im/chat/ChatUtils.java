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
package com.orangelabs.rcs.core.ims.service.im.chat;

import gov2.nist.javax2.sip.header.ContentType;
import java.text.CharacterIterator;
import java.text.StringCharacterIterator;

import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.UnsupportedEncodingException;
import java.nio.charset.Charset;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Date;
import java.util.HashMap;
import java.util.List;
import java.util.Set;
import java.util.Map;
import java.util.TimeZone;


import android.graphics.Bitmap;
import android.graphics.Bitmap.CompressFormat;
import android.graphics.BitmapFactory;
import android.graphics.Matrix;
import android.media.ThumbnailUtils;
import android.provider.MediaStore.Video.Thumbnails;
import android.util.Base64;


import javax2.sip.header.ContactHeader;
import javax2.sip.header.ContentLengthHeader;
import javax2.sip.header.ContentTypeHeader;
import javax2.sip.header.ExtensionHeader;

import org.xml.sax.InputSource;
import com.gsma.services.rcs.chat.GroupChat.ParticipantStatus;
import com.gsma.services.rcs.chat.ChatLog.Message.MimeType;
import com.gsma.services.rcs.contact.ContactId;
import com.gsma.services.rcs.chat.ChatLog.Message.Content.ReasonCode;

import com.gsma.services.rcs.Geoloc;
import com.orangelabs.rcs.core.ims.ImsModule;
import com.orangelabs.rcs.core.ims.service.im.chat.ChatMessage;
import com.orangelabs.rcs.core.ims.network.sip.FeatureTags;
import com.orangelabs.rcs.core.ims.network.sip.Multipart;
import com.orangelabs.rcs.core.ims.network.sip.SipUtils;
import com.orangelabs.rcs.core.ims.protocol.sip.SipRequest;
import com.orangelabs.rcs.core.ims.service.im.chat.cpim.CpimMessage;
import com.orangelabs.rcs.core.ims.service.im.chat.cpim.CpimParser;
import com.orangelabs.rcs.core.ims.service.im.chat.geoloc.GeolocInfoDocument;
import com.orangelabs.rcs.core.ims.service.im.chat.geoloc.GeolocInfoParser;
import com.orangelabs.rcs.core.ims.service.im.chat.imdn.ImdnDocument;
import com.orangelabs.rcs.core.ims.service.im.chat.imdn.ImdnParser;
import com.orangelabs.rcs.core.ims.service.im.chat.imdn.ImdnUtils;
import com.orangelabs.rcs.core.ims.service.im.chat.iscomposing.IsComposingInfo;
import com.orangelabs.rcs.core.ims.service.im.filetransfer.http.FileTransferHttpInfoDocument;
import com.orangelabs.rcs.core.ims.service.im.filetransfer.http.FileTransferHttpInfoParser;
import com.orangelabs.rcs.core.ims.service.im.filetransfer.http.FileTransferHttpResumeInfo;
import com.orangelabs.rcs.core.ims.service.im.filetransfer.http.FileTransferHttpResumeInfoParser;
import com.orangelabs.rcs.platform.AndroidFactory;
import com.orangelabs.rcs.platform.file.FileFactory;
import com.orangelabs.rcs.provider.settings.RcsSettings;
//import com.orangelabs.rcs.utils.Base64;
import com.orangelabs.rcs.utils.DateUtils;
import com.orangelabs.rcs.utils.IdGenerator;
import com.orangelabs.rcs.utils.PhoneUtils;
import com.orangelabs.rcs.utils.StringUtils;
import com.orangelabs.rcs.utils.logger.Logger;

/**
 * Chat utility functions
 * 
 * @author jexa7410
 */
public class ChatUtils {
    /**
     * Anonymous URI
     */
    public final static String ANOMYNOUS_URI = "sip:anonymous@anonymous.invalid";
        
    /**
     * Contribution ID header
     */
    public static final String HEADER_CONTRIBUTION_ID = "Contribution-ID";
    
    /**
     * Conversation ID header
     */
    public static final String HEADER_CONVERSATION_ID = "Conversation-ID";

    /**
     * In reply to Conversation ID header
     */
    public static final String HEADER_INREPLY_TO_CONTRIBUTION_ID = "InReplyTo-Contribution-ID";
    
    /**
     * CRLF constant
     */
    private static final String CRLF = "\r\n";
    
    /**
     * Max size thumbnail in Bytes
     * Enlarge this limitation might lead to payload of SIP Request oversize issue. 
     * Currently OP01 and Vodafone commercial network has limitation of 10K payload(including BASE64 encoding)
     */
    private static final int MAX_THUMBNAIL_SIZE_OP01 = 7*1024;
    private static final int MAX_THUMBNAIL_SIZE = 4*1024;
    
    public static final String ANONYMITY_NOT_ALLOWED = "Anonymity not allowed";
    public static final String FUNCTION_NOT_ALLOWED  = "Function not allowed";
    public static final String SIZE_EXCEEDED         = "Size Exceeded";
    public static final String TEST_ERROR            = "Test Error";
    public static final String NO_DESTINATIONS       = "No destinations";
    public static final String VERSION_UNSUPPORTED   = "132 Version not supported";
    public static final String SERVICE_UNAUTHORIZED  = "127 Service not authorized";
    public static final String DECLINED              = "Decline";
    public static final String TIMEOUT               = "timeout";
    public static final String UNSUPPORTED_MEDIA               = "415";
    public static final String FORBIDDEN               = "403";
    public static final String REQUEST_TIMEDOUT               = "408";
    public static final String INTERVAL_SMALL               = "423";
    public static final String BAD_REQUEST               = "400";

    /**
     * The logger
     */
    private static Logger logger = Logger.getLogger("ChatUtils");

    /**
     * Get supported feature tags for a group chat
     *
     * @return List of tags
     */
    public static List<String> getCpimSupportedFeatureTagsForGroupChat(boolean isCmcc) {
        List<String> tags = new ArrayList<String>(); 
        String additionalRcseTags = "";        
        String additionalRcseCmccTags = "";    
        additionalRcseTags += FeatureTags.FEATURE_CPM_SESSION + ",";
        if(true) {
            additionalRcseCmccTags += FeatureTags.FEATURE_CMCC_IARI_GROUP_MANAGE;
        }
       if(RcsSettings.getInstance().isSupportOP08()){
            additionalRcseTags += FeatureTags.FEATURE_CPM_FT + ",";
        
            additionalRcseTags += FeatureTags.FEATURE_RCSE_FT_THUMBNAIL + ",";
        }
        
        if (additionalRcseTags.length() > 0) {
            if (additionalRcseTags.endsWith(",")) {
                additionalRcseTags = additionalRcseTags.substring(0, additionalRcseTags.length()-1);
            }
            tags.add(FeatureTags.FEATURE_CPM_RCSE + "=\"" + additionalRcseTags + "\"");
        }
        if (additionalRcseCmccTags.length() > 0) {    
            if(isCmcc){
                tags.add(additionalRcseCmccTags);
            }
        }
        
        return tags;
    }    
    
    /**
     * Get supported feature tags for a chat
     *
     * @return List of tags
     */
    public static List<String> getCpimSupportedFeatureTagsForChat() {
        List<String> tags = new ArrayList<String>(); 
        String additionalRcseTags = "";
        additionalRcseTags += FeatureTags.FEATURE_CPM_SESSION + ",";
        additionalRcseTags += FeatureTags.FEATURE_RCSE_LARGE_MSG + ",";
        //additionalRcseTags += FeatureTags.FEATURE_CPM_FT + ",";
        
        if (additionalRcseTags.length() > 0) {
            if (additionalRcseTags.endsWith(",")) {
                additionalRcseTags = additionalRcseTags.substring(0, additionalRcseTags.length()-1);
            }
            tags.add(FeatureTags.FEATURE_RCSE + "=\"" + additionalRcseTags + "\"");
        }
        
        return tags;
    }    
    
    /**
     * Get supported feature tags for a group chat
     *
     * @return List of tags
     */
    public static List<String> getSupportedFeatureTagsForGroupChat() {
        List<String> tags = new ArrayList<String>(); 
        tags.add(FeatureTags.FEATURE_OMA_IM);
        
        String additionalRcseTags = "";
        if (RcsSettings.getInstance().isGeoLocationPushSupported()) {
            additionalRcseTags += FeatureTags.FEATURE_RCSE_GEOLOCATION_PUSH + ",";
        }
        if (RcsSettings.getInstance().isFileTransferSupported()) {
            additionalRcseTags += FeatureTags.FEATURE_RCSE_FT + ",";
        }
        if (RcsSettings.getInstance().isFileTransferHttpSupported()) {
            additionalRcseTags += FeatureTags.FEATURE_RCSE_FT_HTTP + ",";
       }
       if (RcsSettings.getInstance().isFileTransferStoreForwardSupported()) {
            additionalRcseTags += FeatureTags.FEATURE_RCSE_FT_SF;
        }
        if (additionalRcseTags.length() > 0) {
            if (additionalRcseTags.endsWith(",")) {
                additionalRcseTags = additionalRcseTags.substring(0, additionalRcseTags.length()-1);
            }
            tags.add(FeatureTags.FEATURE_RCSE + "=\"" + additionalRcseTags + "\"");
        }
        
        return tags;
    }    
    
    /**
     * Get supported feature tags for a chat
     *
     * @return List of tags
     */
    public static List<String> getSupportedFeatureTagsForChat() {
        List<String> tags = new ArrayList<String>(); 
        tags.add(FeatureTags.FEATURE_OMA_IM);
        
        String additionalRcseTags = "";
        if (RcsSettings.getInstance().isGeoLocationPushSupported()) {
            additionalRcseTags += FeatureTags.FEATURE_RCSE_GEOLOCATION_PUSH + ",";
        }
        if (RcsSettings.getInstance().isFileTransferSupported()) {
            additionalRcseTags += FeatureTags.FEATURE_RCSE_FT + ",";
        }
        if (RcsSettings.getInstance().isFileTransferHttpSupported()) {
            additionalRcseTags += FeatureTags.FEATURE_RCSE_FT_HTTP + ",";
        }
        if (RcsSettings.getInstance().isFileTransferStoreForwardSupported()) {
            additionalRcseTags += FeatureTags.FEATURE_RCSE_FT_SF;
        }
        if (additionalRcseTags.length() > 0) {
            if (additionalRcseTags.endsWith(",")) {
                additionalRcseTags = additionalRcseTags.substring(0, additionalRcseTags.length()-1);
            }
            tags.add(FeatureTags.FEATURE_RCSE + "=\"" + additionalRcseTags + "\"");
        }
        
        return tags;
    }    
    
    /**
     * Get contribution ID
     * 
     * @return String
     */
    public static String getContributionId(SipRequest request) {
        ExtensionHeader contribHeader = (ExtensionHeader)request.getHeader(ChatUtils.HEADER_CONTRIBUTION_ID);
        if (contribHeader != null) {
            return contribHeader.getValue();
        } else {
            return null;
        }
    }
    
    /**
     * Get contribution ID
     * 
     * @return String
     */
    public static String getCoversationId(SipRequest request) {
        ExtensionHeader conversHeader = (ExtensionHeader)request.getHeader(ChatUtils.HEADER_CONVERSATION_ID);
        if (conversHeader != null) {
            return conversHeader.getValue();
        } else {
            return null;
        }
    }

    /**
     * Get contribution ID
     * 
     * @return String
     */
    public static String getInReplyId(SipRequest request) {
        ExtensionHeader inReplyHeader = (ExtensionHeader)request.getHeader(ChatUtils.HEADER_INREPLY_TO_CONTRIBUTION_ID);
        if (inReplyHeader != null) {
            return inReplyHeader.getValue();
        } else {
            return null;
        }
    }
    
    /**
     * Is a group chat session invitation
     * 
     * @param request Request
     * @return Boolean
     */
    public static boolean isGroupChatInvitation(SipRequest request) {
        ContactHeader contactHeader = (ContactHeader)request.getHeader(ContactHeader.NAME);
        String param = contactHeader.getParameter("isfocus");
        if (param != null) {
            return true;
        } else {
            return false;
        }
    }

    /**
     * Get referred identity
     * 
     * @param request SIP request
     * @return SIP URI
     */
    public static String getReferredIdentity(SipRequest request) {
        String referredBy = SipUtils.getReferredByHeader(request);
        if (referredBy != null) {
            // Use the Referred-By header
            referredBy = PhoneUtils.extractNumberFromUri(referredBy);
            return referredBy;
        } else {
            // Use the Asserted-Identity header
            referredBy =  SipUtils.getAssertedIdentity(request);
            if (logger.isActivated()) {
                logger.debug("getReferredIdentity0: " + referredBy);
            }
            if(!(referredBy.equals(request.getFromUri()))){
                if(!RcsSettings.getInstance().isFromUriSupported()){
                    referredBy = PhoneUtils.extractNumberFromUri(referredBy);
                }
            }
            if (logger.isActivated()) {
                logger.debug("getReferredIdentity1: " + referredBy);
            }
            return referredBy;
        }
    }
    
    /**
     * Is a plain text type
     * 
     * @param mime MIME type
     * @return Boolean
     */
    public static boolean isTextPlainType(String mime) {
        if ((mime != null) && mime.toLowerCase().startsWith(InstantMessage.MIME_TYPE)) {
            return true;
        } else {
            return false;
        }
    }
    
    /**
     * Is a composing event type
     * 
     * @param mime MIME type
     * @return Boolean
     */
    public static boolean isApplicationIsComposingType(String mime) {
        if ((mime != null) && mime.toLowerCase().startsWith(IsComposingInfo.MIME_TYPE)) {
            return true;
        } else {
            return false;
        }
    }    

    /**
     * Formats the IP address
     * 
     * @param address
     * @return String
     */
    public static String formatIPAddress(String ipAddress) {
        if(ipAddress != null){
            ipAddress = ipAddress.replace("[","");
            ipAddress = ipAddress.replace("]","");
        }
        return ipAddress;
    }

    /**
     * Is a CPIM message type
     * 
     * @param mime MIME type
     * @return Boolean
     */
    public static boolean isMessageCpimType(String mime) {
        if ((mime != null) && mime.toLowerCase().startsWith(CpimMessage.MIME_TYPE)) {
            return true;
        } else {
            return false;
        }
    }
    
    /**
     * Is an IMDN message type
     * 
     * @param mime MIME type
     * @return Boolean
     */
    public static boolean isMessageImdnType(String mime) {
        if ((mime != null) && mime.toLowerCase().startsWith(ImdnDocument.MIME_TYPE)) {
            return true;
        } else {
            return false;
        }
    }

    /**
     * Is a geolocation event type
     * 
     * @param mime MIME type
     * @return Boolean
     */
    public static boolean isGeolocType(String mime) {
        if ((mime != null) && mime.toLowerCase().startsWith(GeolocInfoDocument.MIME_TYPE)) {
            return true;
        } else {
            return false;
        }
    }
    
    /**
     * Is a isPublicChatType event type
     * 
     * @param mime MIME type
     * @return Boolean
     */
    public static boolean isPublicChatType(String mime) {
        if ((mime != null) && mime.toLowerCase().startsWith(InstantMessage.PUBLIC_MIME_TYPE)) {
            return true;
        } else {
            return false;
        }
    }
    
    /**
     * Is a isPublicChatType event type
     * 
     * @param mime MIME type
     * @return Boolean
     */
    public static boolean isCloudChatType(String mime) {
        if ((mime != null) && mime.toLowerCase().startsWith(InstantMessage.CLOUD_MIME_TYPE)) {
            return true;
        } else {
            return false;
        }
    }
    
    /**
     * Is a isPublicChatType event type
     * 
     * @param mime MIME type
     * @return Boolean
     */
    public static boolean isEmoticonsChatType(String mime) {
        if ((mime != null) && mime.toLowerCase().startsWith(InstantMessage.EMOTICONS_MIME_TYPE)) {
            return true;
        } else {
            return false;
        }
    }

    /**
     * Check whether content type indicate as card message
     * 
     * @param mime MIME type
     * @return Boolean
     */
    public static boolean isCardMsgChatType(String mime) {
        if ((mime != null) && mime.toLowerCase().startsWith(InstantMessage.CARD_MIME_TYPE)) {
            return true;
        } else {
            return false;
        }
    }

    /**
     * Is a file transfer HTTP event type
     * 
     * @param mime MIME type
     * @return Boolean
     */
    public static boolean isFileTransferHttpType(String mime) {
        if ((mime != null) && mime.toLowerCase().startsWith(FileTransferHttpInfoDocument.MIME_TYPE)) {
            return true;
        } else {
            return false;
        }
    }
    
    /**
     * Generate a unique message ID
     * 
     * @return Message ID
     */
    public static String generateMessageId() {
        return "Msg" + IdGenerator.getIdentifier().replace('_', '-');
    }

    /**
     * Generate resource-list for a chat session
     * 
     * @param participants List of participants
     * @return XML document
     */
    public static String generateChatResourceList(List<String> participants) {
        StringBuffer uriList = new StringBuffer();
        for(int i=0; i < participants.size(); i++) {
            String contact = participants.get(i);
            uriList.append(" <entry uri=\"" +
                    PhoneUtils.formatNumberToSipUri(contact) + "\" cp:copyControl=\"to\"/>" 
                    + CRLF);
        }
        String xml = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" + CRLF +
            "<resource-lists xmlns=\"urn:ietf:params:xml:ns:resource-lists\" " +
            "xmlns:cp=\"urn:ietf:params:xml:ns:copycontrol\">" +
            "<list>" + CRLF +
            uriList.toString() +
            "</list></resource-lists>";
        return xml;
    }    

    /**
     * Generate resource-list for a chat session
     * 
     * @param participants List of participants
     * @return XML document
     */
    public static String generateSubscribeResourceList(List<String> participants) {
        StringBuffer uriList = new StringBuffer();
        
        for(int i=0; i < participants.size(); i++) {
            String contact = participants.get(i);
            uriList.append(" <entry uri=\"" +
                    "tel:"+contact+"\""
                    //PhoneUtils.formatNumberToSipUri(contact) 
                    //+"\" cp:copyControl=\"to\""
                    +"/>"
                    + CRLF);
        }
        
        /*
        for(int i=0; i < participants.size(); i++) {
            String contact = participants.get(i);
            uriList.append(" <entry uri=\"" +
                    PhoneUtils.formatNumberToSipUri(contact) + "\" cp:copyControl=\"to\"/>" 
                    + CRLF);
        }
        */
         String xml = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" + CRLF +
            "<resource-lists xmlns=\"urn:ietf:params:xml:ns:resource-lists\" " +
            "xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\">" +
            "<list name=\"list-1\">" + CRLF +
            uriList.toString() +
            "</list></resource-lists>";
        return xml;
    } 

    /**
     * Generate resource-list for a chat session
     * 
     * @param participants List of participants
     * @return XML document
     */
    public static String generateMultiChatResourceList(List<String> participants) {
        StringBuffer uriList = new StringBuffer();
        for(int i=0; i < participants.size(); i++) {
            String contact = participants.get(i);
            uriList.append(" <entry uri=\"" +
                    PhoneUtils.formatNumberToSipUri(contact) + "\" cp:capacity=\"to\"/>" 
                    + CRLF);
        }
        String xml = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" + CRLF +
            "<resource-lists xmlns=\"urn:ietf:params:xml:ns:resource-lists\" " +
            "xmlns:cp=\"urn:ietf:params:xml:ns:capacity\">" +
            "<list>" + CRLF +
            uriList.toString() +
            "</list></resource-lists>";
        return xml;
    }
    
    /**
     * Generate resource-list for a chat session
     * 
     * @param participants List of participants
     * @return XML document
     */
    public static String generateMultiChatBccResourceList(List<String> participants) {
        StringBuffer uriList = new StringBuffer();
        for(int i=0; i < participants.size(); i++) {
            String contact = participants.get(i);
                uriList.append(" <entry uri=\"" +
                        PhoneUtils.formatNumberToSipUri(contact) + "\" cp:copyControl=\"bcc\"/>" 
                        + CRLF);
        }
        String xml = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" + CRLF +
            "<resource-lists xmlns=\"urn:ietf:params:xml:ns:resource-lists\" " +
            "xmlns:cp=\"urn:ietf:params:xml:ns:copyControl\">" +
            "<list>" + CRLF +
            uriList.toString() +
            "</list></resource-lists>";
        return xml;
    }

    /**
     * Generate resource-list for a extended chat session
     * 
     * @param existingParticipant Replaced participant
     * @param replaceHeader Replace header
     * @param newParticipants List of new participants
     * @return XML document
     */
    public static String generateExtendedChatResourceList(String existingParticipant, String replaceHeader, List<String> newParticipants) {
        StringBuffer uriList = new StringBuffer();
        for(int i=0; i < newParticipants.size(); i++) {
            String contact = newParticipants.get(i);
            if (contact.equals(existingParticipant)) {
                uriList.append(" <entry cp:copyControl=\"to\" uri=\"" + PhoneUtils.formatNumberToSipUri(existingParticipant) +
                    StringUtils.encodeXML(replaceHeader) + "\"/>" + CRLF);
            } else {
                uriList.append(" <entry cp:copyControl=\"to\" uri=\"" + PhoneUtils.formatNumberToSipUri(contact) + "\"/>" + CRLF);
            }
        }
        
        String xml = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" + CRLF +
            "<resource-lists xmlns=\"urn:ietf:params:xml:ns:resource-lists\" " + 
            "xmlns:cp=\"urn:ietf:params:xml:ns:copycontrol\">" +
            "<list>" + CRLF +
            uriList.toString() +
            "</list></resource-lists>";
        return xml;
    }    
    
    public static Map<String, ParticipantStatus> changeParticipantsListToMap(ListOfParticipant participants,ParticipantStatus status){
        Map<String, ParticipantStatus> localParticipants = new HashMap<>();
        List<String> listParticipants = participants.getList();
        if (logger.isActivated()) {
            logger.debug("changeParticipantsListToMap: " + listParticipants.size());
        }
        for(String participant: listParticipants){
            localParticipants.put(participant,status);
        }
        return localParticipants;
    }
    
    public static ListOfParticipant getParticipantsList( Map<String, ParticipantStatus> participants){
        ListOfParticipant listParticipants = new ListOfParticipant();
        if (logger.isActivated()) {
            logger.debug("getParticipantsList: " + participants.size());
        }
        try {
            Set<String> listParts = participants.keySet();
            for(String participant: listParts){
                if (logger.isActivated()) {
                    logger.debug("getParticipantsList: add " + participant);
                }
                listParticipants.addParticipant(participant);
            }
        } catch (Exception e) {
            if (logger.isActivated()) {
                logger.debug("getParticipantsList exception");
            }
            e.printStackTrace();
        }
        return listParticipants;
    }
    
    public static Map<String, ParticipantStatus> getInvitedParticipants(SipRequest request){
        ListOfParticipant participants = new ListOfParticipant();
        if (logger.isActivated()) {
            logger.debug("getInvitedParticipants entry");
        }
        try {
            String content = request.getContent();
            String boundary = request.getBoundaryContentType();
            Multipart multi = new Multipart(content, boundary);
            if (multi.isMultipart()) {
                // Extract resource-lists
                String listPart = multi.getPart("application/resource-lists+xml");
                if (listPart != null) {
                    // Create list from XML
                    participants = new ListOfParticipant(listPart);
                    if (logger.isActivated()) {
                        logger.debug("getListOfParticipants1: " + participants);
                    }

                    // Include remote contact
                    String remote = getReferredIdentity(request);
                    participants.addParticipant(remote);
                }
            }
            if (logger.isActivated()) {
                logger.debug("getInvitedParticipants list size: " + participants.getList().size());
            }
        } catch (Exception e) {
            e.printStackTrace();
            if (logger.isActivated()) {
                logger.debug("getInvitedParticipants exception");
            }
        }
        return changeParticipantsListToMap(participants,ParticipantStatus.INVITED);
    }

    /**
     * Is IMDN service
     * 
     * @param request Request
     * @return Boolean
     */
    public static boolean isImdnService(SipRequest request) {
        
        CpimParser cpimParser = null;
        try {
            cpimParser = new CpimParser(request.getContent());
        } catch (Exception e1) {
            if (logger.isActivated()) {
                logger.debug("isImdnService failed to parse the msg" + request.getContent());
            }
            return false;
        }
        
        CpimMessage cpimMsg = cpimParser.getCpimMessage();
        String contentType = cpimMsg.getContentType();
        if (logger.isActivated()) {
            logger.debug("isImdnService type" + contentType);
        }
        
        if (contentType.equals(ImdnDocument.MIME_TYPE)) {
            return true;
        } else {
            return false;
        }
    }
    
    /**
     * Is IMDN notification "delivered" requested
     * 
     * @param request Request
     * @return Boolean
     */
    public static boolean isImdnDeliveredRequested(SipRequest request) {
        boolean result = false;
        try {
            // Read ID from multipart content
            String content = request.getContent();
            int index = content.indexOf(ImdnUtils.HEADER_IMDN_DISPO_NOTIF);
            if (index != -1) {
                index = index+ImdnUtils.HEADER_IMDN_DISPO_NOTIF.length()+1;
                String part = content.substring(index);
                String notif = part.substring(0, part.indexOf(CRLF));
                if (notif.indexOf(ImdnDocument.POSITIVE_DELIVERY) != -1) {
                    result = true;
                }
            }
        } catch(Exception e) {
            result = false;
        }
        return result;
    }
    
    public static String convertToHex(byte[] data) {
        StringBuilder buf = new StringBuilder();
        for (byte b : data) {
            int halfbyte = (b >>> 4) & 0x0F;
            int two_halfs = 0;
            do {
                buf.append((0 <= halfbyte) && (halfbyte <= 9) ? (char) ('0' + halfbyte) : (char) ('a' + (halfbyte - 10)));
                halfbyte = b & 0x0F;
            } while (two_halfs++ < 1);
        }
        return buf.toString();
    }

    public static String SHA1(String text) throws NoSuchAlgorithmException, UnsupportedEncodingException {
        MessageDigest md = MessageDigest.getInstance("SHA-1");
        md.update(text.getBytes("iso-8859-1"), 0, text.length());
        byte[] sha1hash = md.digest();
        return convertToHex(sha1hash);
    }

    public static String SHA1(byte[] data) throws NoSuchAlgorithmException, UnsupportedEncodingException {
        MessageDigest md = MessageDigest.getInstance("SHA-1");
        md.update(data, 0, data.length);
        byte[] sha1hash = md.digest();
        return convertToHex(sha1hash);
    }

    /**
     * Is IMDN notification "displayed" requested
     * 
     * @param request Request
     * @return Boolean
     */
    public static boolean isImdnDisplayedRequested(SipRequest request) {
        boolean result = false;
        try {
            // Read ID from multipart content
            String content = request.getContent();
            int index = content.indexOf(ImdnUtils.HEADER_IMDN_DISPO_NOTIF);
            if (index != -1) {
                index = index+ImdnUtils.HEADER_IMDN_DISPO_NOTIF.length()+1;
                String part = content.substring(index);
                String notif = part.substring(0, part.indexOf(CRLF));
                if (notif.indexOf(ImdnDocument.DISPLAY) != -1) {
                    result = true;
                }
            }
        } catch(Exception e) {
            result = false;;
        }
        return result;
    }
    
    /**
     * Returns the message ID from a SIP request
     * 
     * @param request Request
     * @return Message ID
     */
    public static String getMessageId(SipRequest request) {
        String result = null;
        try {
            // Read ID from multipart content
            String content = request.getContent();
            int index = content.indexOf(ImdnUtils.HEADER_IMDN_MSG_ID);
            if (index != -1) {
                index = index+ImdnUtils.HEADER_IMDN_MSG_ID.length()+1;
                String part = content.substring(index);
                String msgId = part.substring(0, part.indexOf(CRLF));
                result = msgId.trim();
            }
        } catch(Exception e) {
            result = null;
        }
        return result;
    }
    
    /**
     * Format to a SIP-URI for CPIM message
     * 
     * @param input Input
     * @return SIP-URI
     */
    public static String formatCpimSipUri(String input) {
        input = input.trim();
        
        if (input.startsWith("<")) {
            // Already a SIP-URI format
            return input;            
        }

        // It's a SIP address: remove display name
        if (input.startsWith("\"")) {
            int index1 = input.indexOf("\"", 1);
            if (index1 > 0) {
                input = input.substring(index1+2);
            }
            return input;
        }   


        if (input.startsWith("sip:") || input.startsWith("tel:")) {
            // Just add URI delimiter
            return "<" + input + ">";
        } else {
            // It's a number, format it
            return "<" + PhoneUtils.formatNumberToSipUri(input) + ">";
        }
    }

/**
     * Format to a SIP-URI for CPIM message
     * 
     * @param input Input
     * @return SIP-URI
     */
    public static String formatCpimSipUriAlias(String input) {
        input = input.trim();
        
        if (input.startsWith("<")) {
            // Already a SIP-URI format
            return input.substring(1);            
        }

        // It's a SIP address: remove display name
        if (input.startsWith("\"")) {
            int index1 = input.indexOf("\"", 1);
            if (index1 > 0) {
                input = input.substring(index1+2);
            }
            return input;
        }   
        

        if (input.startsWith("sip:") || input.startsWith("tel:")) {
            // Just add URI delimiter
            return "<" + input + ">";
        } else {
            // It's a number, format it
            return "<" + PhoneUtils.formatNumberToSipUri(input) + ">";
        }
    }

    /**
     * Build a CPIM message
     * 
     * @param from From
     * @param to To
     * @param content Content
     * @param contentType Content type
     * @return String
     */
    public static String buildCpimMessage(String from, String to, String content, String contentType) {
        int contentLength = content.getBytes().length;
        if(RcsSettings.getInstance().isBase64EncodingSupported()){
            content = Base64.encodeToString(content.getBytes(), Base64.NO_WRAP);
            contentLength = content.getBytes().length;
        }
        String cpim =
            CpimMessage.HEADER_FROM + ": " + ChatUtils.formatCpimSipUri(from) + CRLF + 
            CpimMessage.HEADER_TO + ": " + ChatUtils.formatCpimSipUri(to) + CRLF + 
            CpimMessage.HEADER_DATETIME + ": " + DateUtils.encodeDate(System.currentTimeMillis()) + CRLF + 
            CRLF +  
            CpimMessage.HEADER_CONTENT_TYPE + ": " + contentType + ";charset=utf-8" + CRLF ;
                                                                                                                
        if(RcsSettings.getInstance().isBase64EncodingSupported()){
            byte[] c = content.getBytes(Charset.forName("UTF-8"));
            cpim += SipUtils.HEADER_CONTENT_TRANSFER_ENCODING + ": " + "base64" + CRLF 
            + CRLF + content; 
        }
        else{
            cpim += CRLF + content;    
        }
           
        return cpim;
    }

    /**
     * Build a CPIM message
     * 
     * @param from From
     * @param to To
     * @param content Content
     * @param contentType Content type
     * @return String
     */
    public static String buildCpimMessageAlias(String from, String to, String content, String contentType) {
        int contentLength = content.getBytes().length;
        if(RcsSettings.getInstance().isBase64EncodingSupported()){
            content = Base64.encodeToString(content.getBytes(), Base64.NO_WRAP);
            contentLength = content.getBytes().length;
        }
        String nsMsrpHeader = CRLF;
        if(RcsSettings.getInstance().isPAccessNetworkInfoSupported()){
            String pAccessHeader = "" + ImsModule.IMS_USER_PROFILE.getAccessNetworkInfo();
            nsMsrpHeader +=  CpimMessage.HEADER_NS + ": " + CpimMessage.HEADER_MY_FEATURES + CRLF +
                                    CpimMessage.HEADER_PANI + ": " + pAccessHeader + CRLF;
        }
        String cpim =
            CpimMessage.HEADER_FROM + ": " + ChatUtils.formatCpimSipUriAlias(from) + CRLF + 
            CpimMessage.HEADER_TO + ": " + ChatUtils.formatCpimSipUri(to) + CRLF + 
            CpimMessage.HEADER_DATETIME + ": " + DateUtils.encodeDate(System.currentTimeMillis()) + nsMsrpHeader + 
            CRLF +  
            CpimMessage.HEADER_CONTENT_TYPE + ": " + contentType + ";charset=utf-8" + CRLF;
        
        if(RcsSettings.getInstance().isBase64EncodingSupported()){
            byte[] c = content.getBytes(Charset.forName("UTF-8"));
            cpim = cpim + SipUtils.HEADER_CONTENT_TRANSFER_ENCODING + ": " + "base64" + CRLF 
            +CRLF + 
            content;    
        }
        else{
            cpim = cpim +CRLF +
            content;    
        }    
           
        return cpim;
    }
    
    /**
     * Build a CPIM message with full IMDN headers
     * 
     * @param from From URI
     * @param to To URI
     * @param messageId Message ID
     * @param content Content
     * @param contentType Content type
     * @return String
     */
    public static String buildCpimMessageWithImdn(String from, String to, String messageId, String content, String contentType) {
        int contentLength = content.getBytes().length;
        if(RcsSettings.getInstance().isBase64EncodingSupported()){
            content = Base64.encodeToString(content.getBytes(), Base64.NO_WRAP);
            contentLength = content.getBytes().length;
        }
        String nsMsrpHeader = CRLF;
        if(RcsSettings.getInstance().isPAccessNetworkInfoSupported()){
            String pAccessHeader = "" + ImsModule.IMS_USER_PROFILE.getAccessNetworkInfo();
            nsMsrpHeader +=  CpimMessage.HEADER_NS + ": " + CpimMessage.HEADER_MY_FEATURES + CRLF +
                                    CpimMessage.HEADER_PANI + ": " + pAccessHeader + CRLF;
        }
        String cpim =
            CpimMessage.HEADER_FROM + ": " + ChatUtils.formatCpimSipUri(from) + CRLF + 
            CpimMessage.HEADER_TO + ": " + ChatUtils.formatCpimSipUri(to) + CRLF + 
            CpimMessage.HEADER_NS + ": " + ImdnDocument.IMDN_NAMESPACE + CRLF +
            ImdnUtils.HEADER_IMDN_MSG_ID + ": " + messageId + CRLF +
            CpimMessage.HEADER_DATETIME + ": " + DateUtils.encodeDate(System.currentTimeMillis()) + nsMsrpHeader + 
            ImdnUtils.HEADER_IMDN_DISPO_NOTIF + ": " + ImdnDocument.POSITIVE_DELIVERY + ", " + ImdnDocument.DISPLAY + CRLF +
            CRLF +  
            CpimMessage.HEADER_CONTENT_TYPE + ": " + contentType + ";charset=utf-8" + CRLF +
            CpimMessage.HEADER_CONTENT_LENGTH + ": " + contentLength + CRLF ;
                                                
        if(RcsSettings.getInstance().isBase64EncodingSupported()){
            byte[] c = content.getBytes(Charset.forName("UTF-8"));
            cpim = cpim + SipUtils.HEADER_CONTENT_TRANSFER_ENCODING + ": " + "base64" + CRLF 
            +CRLF + 
            content; 
        }
        else{
            cpim = cpim +CRLF +
            content;    
        }
        return cpim;
      
    }
    
    /**
     * Build a CPIM message with full IMDN headers
     * 
     * @param from From URI
     * @param to To URI
     * @param messageId Message ID
     * @param content Content
     * @param contentType Content type
     * @return String
     */
    public static String buildCpimMessageWithoutDisplay(String from, String to, String messageId, String content, String contentType) {
        int contentLength = content.getBytes().length;
        if(RcsSettings.getInstance().isBase64EncodingSupported()){
            content = Base64.encodeToString(content.getBytes(), Base64.NO_WRAP);
            contentLength = content.getBytes().length;
        }
        String nsMsrpHeader = CRLF;
        if(RcsSettings.getInstance().isPAccessNetworkInfoSupported()){
            String pAccessHeader = "" + ImsModule.IMS_USER_PROFILE.getAccessNetworkInfo();
            nsMsrpHeader +=  CpimMessage.HEADER_NS + ": " + CpimMessage.HEADER_MY_FEATURES + CRLF +
                                    CpimMessage.HEADER_PANI + ": " + pAccessHeader + CRLF;
        }
        String cpim =
            CpimMessage.HEADER_FROM + ": " + ChatUtils.formatCpimSipUri(from) + CRLF + 
            CpimMessage.HEADER_TO + ": " + ChatUtils.formatCpimSipUri(to) + CRLF + 
            CpimMessage.HEADER_NS + ": " + ImdnDocument.IMDN_NAMESPACE + CRLF +
            ImdnUtils.HEADER_IMDN_MSG_ID + ": " + messageId + CRLF +
            CpimMessage.HEADER_DATETIME + ": " + DateUtils.encodeDate(System.currentTimeMillis()) + nsMsrpHeader + 
            ImdnUtils.HEADER_IMDN_DISPO_NOTIF + ": " + ImdnDocument.POSITIVE_DELIVERY + CRLF +
            CRLF +  
            CpimMessage.HEADER_CONTENT_TYPE + ": " + contentType + ";charset=utf-8" + CRLF +
            CpimMessage.HEADER_CONTENT_LENGTH + ": " + contentLength + CRLF ;
                                                
        if(RcsSettings.getInstance().isBase64EncodingSupported()){
            byte[] c = content.getBytes(Charset.forName("UTF-8"));
            cpim = cpim + SipUtils.HEADER_CONTENT_TRANSFER_ENCODING + ": " + "base64" + CRLF 
            +CRLF + 
            content; 
        }
        else{
            cpim = cpim +CRLF +
            content;    
        }
        return cpim;
      
    }
    
    /**
     * Build a CPIM message with full IMDN headers
     * 
     * @param from From URI
     * @param to To URI
     * @param messageId Message ID
     * @param content Content
     * @param contentType Content type
     * @return String
     */
    public static String buildCpimMessageWithImdn(String from, String to, String messageId,String contentType) {
        if(RcsSettings.getInstance().isImDisplayedNotificationActivated()){
            return buildCpimMessageWithImdnDisplay(from,to,messageId,contentType);
        }
        String nsMsrpHeader = CRLF;
        if(RcsSettings.getInstance().isPAccessNetworkInfoSupported()){
            String pAccessHeader = "" + ImsModule.IMS_USER_PROFILE.getAccessNetworkInfo();
            nsMsrpHeader +=  CpimMessage.HEADER_NS + ": " + CpimMessage.HEADER_MY_FEATURES + CRLF +
                                    CpimMessage.HEADER_PANI + ": " + pAccessHeader + CRLF;
        }
        String cpim =
            CpimMessage.HEADER_FROM + ": " + ChatUtils.formatCpimSipUriAlias(from) + SipUtils.CRLF + 
            CpimMessage.HEADER_TO + ": " + ChatUtils.formatCpimSipUri(to) + SipUtils.CRLF + 
            CpimMessage.HEADER_NS + ": " + ImdnDocument.IMDN_NAMESPACE + SipUtils.CRLF +
            ImdnUtils.HEADER_IMDN_MSG_ID + ": " + messageId + SipUtils.CRLF +
            CpimMessage.HEADER_DATETIME + ": " + DateUtils.encodeDate(System.currentTimeMillis()) + nsMsrpHeader +
            ImdnUtils.HEADER_IMDN_DISPO_NOTIF + ": " + ImdnDocument.POSITIVE_DELIVERY  + SipUtils.CRLF +
            SipUtils.CRLF +
            CpimMessage.HEADER_CONTENT_TYPE + ": " + contentType +
            SipUtils.CRLF + SipUtils.CRLF;
        return cpim;
    }
    
    /**
     * Build a CPIM message with full IMDN headers
     * 
     * @param from From URI
     * @param to To URI
     * @param messageId Message ID
     * @param content Content
     * @param contentType Content type
     * @return String
     */
    public static String buildCpimMessageWithImdnDisplay(String from, String to, String messageId,String contentType) {
        String nsMsrpHeader = CRLF;
        if(RcsSettings.getInstance().isPAccessNetworkInfoSupported()){
            String pAccessHeader = "" + ImsModule.IMS_USER_PROFILE.getAccessNetworkInfo();
            nsMsrpHeader +=  CpimMessage.HEADER_NS + ": " + CpimMessage.HEADER_MY_FEATURES + CRLF +
                                    CpimMessage.HEADER_PANI + ": " + pAccessHeader + CRLF;
        }
        String cpim =
            CpimMessage.HEADER_FROM + ": " + ChatUtils.formatCpimSipUriAlias(from) + SipUtils.CRLF + 
            CpimMessage.HEADER_TO + ": " + ChatUtils.formatCpimSipUri(to) + SipUtils.CRLF + 
            CpimMessage.HEADER_NS + ": " + ImdnDocument.IMDN_NAMESPACE + SipUtils.CRLF +
            ImdnUtils.HEADER_IMDN_MSG_ID + ": " + messageId + SipUtils.CRLF +
            CpimMessage.HEADER_DATETIME + ": " + DateUtils.encodeDate(System.currentTimeMillis()) + nsMsrpHeader + 
            ImdnUtils.HEADER_IMDN_DISPO_NOTIF + ": " + ImdnDocument.POSITIVE_DELIVERY  + ", " + ImdnDocument.DISPLAY + SipUtils.CRLF +
            SipUtils.CRLF +
            CpimMessage.HEADER_CONTENT_TYPE + ": " + contentType +
            SipUtils.CRLF + SipUtils.CRLF;
        return cpim;
    }
    
    /**
     * Build a CPIM message with full IMDN headers
     * 
     * @param from From URI
     * @param to To URI
     * @param messageId Message ID
     * @param content Content
     * @param contentType Content type
     * @return String
     */
    public static String buildCMCCCpimMessageWithImdn(String from, String to, String messageId, String content, String contentType) {
        String cpim =
            CpimMessage.HEADER_FROM + ": " + ChatUtils.formatCpimSipUri(from) + CRLF + 
            CpimMessage.HEADER_TO + ": " + ChatUtils.formatCpimSipUri(to) + CRLF + 
            CpimMessage.HEADER_NS + ": " + ImdnDocument.IMDN_NAMESPACE + CRLF +
            ImdnUtils.HEADER_IMDN_MSG_ID + ": " + messageId + CRLF +
            CpimMessage.HEADER_DATETIME + ": " + DateUtils.encodeDate(System.currentTimeMillis()) + CRLF + 
            ImdnUtils.HEADER_IMDN_DISPO_NOTIF + ": " + ImdnDocument.POSITIVE_DELIVERY + ", " + ImdnDocument.DISPLAY + CRLF +
            CRLF +  
            CpimMessage.HEADER_CONTENT_TYPE + ": " + contentType + ";charset=utf-8" + CRLF +
            CpimMessage.HEADER_CONTENT_LENGTH + ": " + "0" +
            CRLF;    
        return cpim;
    }
    
    /**
     * Build a CPIM message For Spam INVITE
     * 
     * @param from From URI
     * @param to To URI
     * @param messageId Message ID
     * @param content Content
     * @param contentType Content type
     * @return String
     */
    public static String buildCpimMessageForSpam(String from, String to, String messageId, String content, String contentType, long time) {
    if (logger.isActivated()) {
            logger.debug("buildCpimMessageForSpam: " + content + " time: "
                    + time);
        }
        int contentLength = content.getBytes().length;
        if (RcsSettings.getInstance().isBase64EncodingSupported()) {
            content = Base64.encodeToString(content.getBytes(), Base64.NO_WRAP);
            contentLength = content.getBytes().length;
        }
        SimpleDateFormat sdf = new SimpleDateFormat("yyyy-MM-dd'T'HH:mm:ssZ");
        sdf.setTimeZone(TimeZone.getTimeZone("UTC"));
        String localTime = sdf.format(new Date(time));
        String cpim = CpimMessage.HEADER_FROM + ": "
                + ChatUtils.formatCpimSipUri(from) + CRLF
                + CpimMessage.HEADER_TO + ": " + ChatUtils.formatCpimSipUri(to)
                + CRLF + CpimMessage.HEADER_NS + ": "
                + ImdnDocument.IMDN_NAMESPACE + CRLF
                + ImdnUtils.HEADER_IMDN_MSG_ID + ": " + messageId + CRLF
                + CpimMessage.HEADER_DATETIME + ": " + localTime + CRLF
                + ImdnUtils.HEADER_IMDN_DISPO_NOTIF + ": "
                + ImdnDocument.DISPLAY + CRLF + CRLF
                + CpimMessage.HEADER_CONTENT_TYPE + ": " + contentType
                + ";charset=utf-8" + CRLF + CpimMessage.HEADER_CONTENT_LENGTH
                + ": " + contentLength + CRLF;

        if (RcsSettings.getInstance().isBase64EncodingSupported()) {
            byte[] c = content.getBytes(Charset.forName("UTF-8"));
            cpim = cpim + SipUtils.HEADER_CONTENT_TRANSFER_ENCODING + ": "
                    + "base64" + CRLF + CRLF + content;
        } else {
            cpim = cpim + CRLF + content;
        }
        if (logger.isActivated()) {
            logger.debug("buildCpimMessageForSpam: " + content + " localTime: "
                    + localTime);
        }
        return cpim;
    }
    
    /**
     * Build a CPIM message For Spam INVITE
     * 
     * @param from From URI
     * @param to To URI
     * @param messageId Message ID
     * @param content Content
     * @param contentType Content type
     * @return String
     */
    public static String buildCpimMessageForSpamNoIMDN(String from, String to, String content, String contentType) {
        String cpim =
            CpimMessage.HEADER_FROM + ": " + ChatUtils.formatCpimSipUri(from) + CRLF + 
            CpimMessage.HEADER_TO + ": " + ChatUtils.formatCpimSipUri(to) +            
            CpimMessage.HEADER_DATETIME + ": " + DateUtils.encodeDate(System.currentTimeMillis())+ CRLF + 
            CRLF + 
            content;    
                        
        return cpim;
    }
    
    /**
     * Build a CPIM message with IMDN delivered header
     * 
     * @param from From URI
     * @param to To URI
     * @param messageId Message ID
     * @param content Content
     * @param contentType Content type
     * @return String
     */
    public static String buildCpimMessageWithDeliveredImdn(String from, String to, String messageId, String content, String contentType) {
        boolean base64Encoded = RcsSettings.getInstance().isBase64EncodingSupported();
        int contentLength;

        if (content == null) {
            contentLength = 0;
        } else {
            if (base64Encoded){
                content = Base64.encodeToString(content.getBytes(), Base64.NO_WRAP);
            }
            contentLength = content.getBytes().length;
        }

        String cpim =
            CpimMessage.HEADER_FROM + ": " + ChatUtils.formatCpimSipUri(from) + CRLF + 
            CpimMessage.HEADER_TO + ": " + ChatUtils.formatCpimSipUri(to) + CRLF + 
            CpimMessage.HEADER_NS + ": " + ImdnDocument.IMDN_NAMESPACE + CRLF +
            ImdnUtils.HEADER_IMDN_MSG_ID + ": " + messageId + CRLF +
            CpimMessage.HEADER_DATETIME + ": " + DateUtils.encodeDate(System.currentTimeMillis()) + CRLF + 
            ImdnUtils.HEADER_IMDN_DISPO_NOTIF + ": " + ImdnDocument.POSITIVE_DELIVERY + CRLF +
            CRLF +  
            CpimMessage.HEADER_CONTENT_TYPE + ": " + contentType + ";charset=utf-8" + CRLF +
            CpimMessage.HEADER_CONTENT_LENGTH + ": " + contentLength + CRLF; 

        if (content != null) {
            if (base64Encoded) {
                cpim += CpimMessage.HEADER_CONTENT_ENCODING + ": " + "base64" + CRLF;
            }
            cpim += CRLF + content;
        }
        
        return cpim;
    }

    /**
     * Build a CPIM message with IMDN delivered header
     * 
     * @param from From URI
     * @param to To URI
     * @param messageId Message ID
     * @param content Content
     * @param contentType Content type
     * @return String
     */
    public static String buildCpimMessageWithDeliveredImdnAlias(String from, String to, String messageId, String content, String contentType) {
        int contentLength = content.getBytes().length;
        if(RcsSettings.getInstance().isBase64EncodingSupported()){
            content = Base64.encodeToString(content.getBytes(), Base64.NO_WRAP);
            contentLength = content.getBytes().length;
        }
        String headerDispositionInfo = ImdnDocument.POSITIVE_DELIVERY + ", " + ImdnDocument.DISPLAY ;
        if(RcsSettings.getInstance().supportOP01())
             headerDispositionInfo = ImdnDocument.POSITIVE_DELIVERY;
        String cpim =
            CpimMessage.HEADER_FROM + ": " + ChatUtils.formatCpimSipUriAlias(from) + CRLF + 
            CpimMessage.HEADER_TO + ": " + ChatUtils.formatCpimSipUri(to) + CRLF + 
            CpimMessage.HEADER_NS + ": " + ImdnDocument.IMDN_NAMESPACE + CRLF +
            ImdnUtils.HEADER_IMDN_MSG_ID + ": " + messageId + CRLF +
            CpimMessage.HEADER_DATETIME + ": " + DateUtils.encodeDate(System.currentTimeMillis()) + CRLF + 
            ImdnUtils.HEADER_IMDN_DISPO_NOTIF + ": " + headerDispositionInfo + CRLF +
            CRLF +  
            CpimMessage.HEADER_CONTENT_TYPE + ": " + contentType + "; charset=utf-8" + CRLF +
            CpimMessage.HEADER_CONTENT_LENGTH + ": " + contentLength + CRLF ;
        
            if(RcsSettings.getInstance().isBase64EncodingSupported()){
                byte[] c = content.getBytes(Charset.forName("UTF-8"));
                cpim = cpim + SipUtils.HEADER_CONTENT_TRANSFER_ENCODING + ": " + "base64" + CRLF 
                +CRLF + 
                content; 
            }
            else {
                cpim = cpim +CRLF +
            content;    
            }
        return cpim;
    }
    
    /**
     * Build a CPIM delivery report
     * 
     * @param from From
     * @param to To
     * @param imdn IMDN report
     * @return String
     */
    public static String buildCpimDeliveryReport(String from, String to, String imdn) {
        String cpim =
            CpimMessage.HEADER_FROM + ": " + ChatUtils.formatCpimSipUri(from) + CRLF + 
            CpimMessage.HEADER_TO + ": " + ChatUtils.formatCpimSipUri(to) + CRLF + 
            CpimMessage.HEADER_NS + ": " + ImdnDocument.IMDN_NAMESPACE + CRLF +
            ImdnUtils.HEADER_IMDN_MSG_ID + ": " + IdGenerator.getIdentifier() + CRLF +
            CpimMessage.HEADER_DATETIME + ": " + DateUtils.encodeDate(System.currentTimeMillis()) + CRLF + 
            CpimMessage.HEADER_CONTENT_DISPOSITION + ": " + ImdnDocument.NOTIFICATION + CRLF +
            CRLF +  
            CpimMessage.HEADER_CONTENT_TYPE + ": " + ImdnDocument.MIME_TYPE + CRLF +
            CpimMessage.HEADER_CONTENT_LENGTH + ": " + imdn.getBytes().length + CRLF + 
            CRLF + 
            imdn;    
           
        return cpim;
    }


    // Chuancheng IOT 5-5-1
    public static String buildCpimDeliveryReport(String from, String to, String imdn,String failedReport) {
        String cpim =
            CpimMessage.HEADER_FAILURE_REPORT + ": "+failedReport+"" + CRLF+
            CpimMessage.HEADER_FROM + ": " + ChatUtils.formatCpimSipUri(from) + CRLF + 
            CpimMessage.HEADER_TO + ": " + ChatUtils.formatCpimSipUri(to) + CRLF + 
            CpimMessage.HEADER_NS + ": " + ImdnDocument.IMDN_NAMESPACE + CRLF +
            ImdnUtils.HEADER_IMDN_MSG_ID + ": " + IdGenerator.getIdentifier() + CRLF +
            CpimMessage.HEADER_DATETIME + ": " + DateUtils.encodeDate(System.currentTimeMillis()) + CRLF + 
            CpimMessage.HEADER_CONTENT_DISPOSITION + ": " + ImdnDocument.NOTIFICATION + CRLF +
            CRLF +  
            CpimMessage.HEADER_CONTENT_TYPE + ": " + ImdnDocument.MIME_TYPE + CRLF +
            CpimMessage.HEADER_CONTENT_LENGTH + ": " + imdn.getBytes().length + CRLF + 
            CRLF + 
            imdn;    
           
        return cpim;
    }
    
    /**
     * Parse a CPIM delivery report
     * 
     * @param cpim CPIM document
     * @return IMDN document
     */
    public static ImdnDocument parseCpimDeliveryReport(String cpim) {
        ImdnDocument imdn = null;
        try {
            // Parse CPIM document
            CpimParser cpimParser = new CpimParser(cpim);
            CpimMessage cpimMsg = cpimParser.getCpimMessage();
            if (cpimMsg != null) {
                // Check if the content is a IMDN message            
                String contentType = cpimMsg.getContentType();
                if (logger.isActivated()) {
                    logger.debug("parseCpimDeliveryReport" + contentType);
                }
                if ((contentType != null) && ChatUtils.isMessageImdnType(contentType)) {
                    // Parse the IMDN document
                    if (logger.isActivated()) {
                        logger.debug("parseCpimDeliveryReport1");
                    }
                    imdn = parseDeliveryReport(cpimMsg.getMessageContent());
                }
            }
        } catch(Exception e) {
            e.printStackTrace();
            imdn = null;
        }        
        return imdn;
    }
    
    /**
     * Parse a CPIM delivery report
     * 
     * @param cpim CPIM document
     * @return IMDN document
     */
    public static ImdnDocument parseNewCpimDeliveryReport(String cpim) {
        ImdnDocument imdn = null;
        try {
            // Parse CPIM document
            CpimParser cpimParser = new CpimParser(cpim);
            CpimMessage cpimMsg = cpimParser.getCpimMessage();
            if (cpimMsg != null) {
                // Check if the content is a IMDN message            
                String contentType = cpimMsg.getContentType();
                if (logger.isActivated()) {
                    logger.debug("parseNewDeliveryReport" + contentType);
                }
                if ((contentType != null) && ChatUtils.isMessageImdnType(contentType)) {
                    // Parse the IMDN document
                    if (logger.isActivated()) {
                        logger.debug("parseNewDeliveryReport");
                    }
                    imdn = parseNewDeliveryReport(cpimMsg.getMessageContent());
                }
            }
        } catch(Exception e) {
            e.printStackTrace();
            imdn = null;
        }        
        return imdn;
    }

    /**
     * Parse a delivery report
     * 
     * @param xml XML document
     * @return IMDN document
     */
    public static ImdnDocument parseDeliveryReport(String xml) {
        try {           
            InputSource input = new InputSource(new ByteArrayInputStream(xml.getBytes()));
            ImdnParser parser = new ImdnParser(input);
            return parser.getImdnDocument();
        } catch(Exception e) {
            e.printStackTrace();
            return null;
        }        
    }
    
    /**
     * Parse a delivery report
     * 
     * @param xml XML document
     * @return IMDN document
     */
    public static ImdnDocument parseNewDeliveryReport(String xml) {
        try {
            byte[] precontent = xml.getBytes();
            byte[] content = precontent;
            if(RcsSettings.getInstance().singleRegistrationSupported()){
                content = Arrays.copyOfRange(precontent, 0, precontent.length-1);
            }
            
            InputSource input = new InputSource(new ByteArrayInputStream(content));
            ImdnParser parser = new ImdnParser(input);
            return parser.getImdnDocument();
        } catch(Exception e) {
            e.printStackTrace();
            return null;
        }        
    }

    /**
     * Build a delivery report
     * 
     * @param msgId Message ID
     * @param status Status
     * @return XML document
     */
    public static String buildDeliveryReport(String msgId, String status) {
        String method;
        if (status.equals(ImdnDocument.DELIVERY_STATUS_DISPLAYED) && RcsSettings.getInstance().isImDisplayedNotificationActivated()) {
            method = "display-notification";
        } else
        if (status.equals(ImdnDocument.DELIVERY_STATUS_DELIVERED)) {
            method = "delivery-notification";
        } 
        else
        if (status.equals(ImdnDocument.BURN_STATUS_BURNED)) {
                method = "display_burned";
            } 
        else {
            method = "processing-notification";
        }
        
        return "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" + CRLF +
            "<imdn xmlns=\"urn:ietf:params:xml:ns:imdn\">" + CRLF +
            "<message-id>" + msgId + "</message-id>" + CRLF +
            "<datetime>" + DateUtils.encodeDate(System.currentTimeMillis()) + "</datetime>" + CRLF +
            "<" + method + "><status><" + status + "/></status></" + method + ">" + CRLF +
            "</imdn>";
    }
    
    /**
    * Build a geoloc document
    * 
    * @param geoloc Geoloc info
    * @param contact Contact
    * @param msgId Message ID
    * @return XML document
    */
    public static String buildGeolocDocument(Geoloc geoloc, String contact, String msgId) {        
        String document= "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" + CRLF +
                "<rcsenveloppe xmlns=\"urn:gsma:params:xml:ns:rcs:rcs:geolocation\"" +
                " xmlns:rpid=\"urn:ietf:params:xml:ns:pidf:rpid\"" +
                " xmlns:gp=\"urn:ietf:params:xml:ns:pidf:geopriv10\"" +
                " xmlns:gml=\"http://www.opengis.net/gml\"" +
                " xmlns:gs=\"http://www.opengis.net/pidflo/1.0\"" +
                " entity=\""+ contact +"\">" + CRLF;
        String expire = DateUtils.encodeDate(geoloc.getExpiration());
        document += "<rcspushlocation id=\""+ msgId +"\" label=\""+ geoloc.getLabel() +"\" >" +
                "<rpid:place-type rpid:until=\""+ expire +"\">" +                
                "</rpid:place-type>" + CRLF + 
                "<rpid:time-offset rpid:until=\""+ expire +"\"></rpid:time-offset>" + CRLF +
                "<gp:geopriv>" + CRLF + 
                "<gp:location-info>" + CRLF +
                "<gs:Circle srsName=\"urn:ogc:def:crs:EPSG::4326\">" + CRLF +
                "<gml:pos>"+ geoloc.getLatitude()+" "+geoloc.getLongitude() +"</gml:pos>" + CRLF +
                "<gs:radius uom=\"urn:ogc:def:uom:EPSG::9001\">" + geoloc.getAccuracy() + "</gs:radius>" + CRLF +
                "</gs:Circle>" + CRLF +
                "</gp:location-info>" + CRLF + 
                "<gp:usage-rules>" + CRLF +
                "<gp:retention-expiry>"+ expire +"</gp:retention-expiry>" + CRLF +
                "</gp:usage-rules>" + CRLF + 
                "</gp:geopriv>" + CRLF + 
                "<timestamp>"+ DateUtils.encodeDate(System.currentTimeMillis()) +"</timestamp>" + CRLF + 
                "</rcspushlocation>" + CRLF;
        document += "</rcsenveloppe>" + CRLF;
        return document;
    }
    
    /**
     * Parse a geoloc document
     *
     * @param xml XML document
     * @return Geoloc info
     */
    public static Geoloc parseGeolocDocument(String xml) {
        try {
            InputSource geolocInput = new InputSource(new ByteArrayInputStream(xml.getBytes()));
            GeolocInfoParser geolocParser = new GeolocInfoParser(geolocInput);
            GeolocInfoDocument geolocDocument = geolocParser.getGeoLocInfo();
            if (geolocDocument != null) {
                /*GeolocPush geoloc = new GeolocPush(geolocDocument.getLabel(),
                        geolocDocument.getLatitude(),
                        geolocDocument.getLongitude(),
                        geolocDocument.getExpiration(),
                        geolocDocument.getRadius());*/
                return new Geoloc(geolocDocument.getLabel(), geolocDocument.getLatitude(),
                        geolocDocument.getLongitude(), geolocDocument.getExpiration(),
                        geolocDocument.getRadius());
                //return geoloc;
            }
        } catch(Exception e) {
            return null;
        }
        return null;
    }
    
    /**
     * Parse a file transfer over HTTP document
     *
     * @param xml XML document
     * @return File transfer document
     */
    public static FileTransferHttpInfoDocument parseFileTransferHttpDocument(byte[] xml) {
        try {
            InputSource ftHttpInput = new InputSource(new ByteArrayInputStream(xml));
            FileTransferHttpInfoParser ftHttpParser = new FileTransferHttpInfoParser(ftHttpInput);
            return ftHttpParser.getFtInfo();
        } catch(Exception e) {
            return null;
        }
    }

    /**
     * Parse a file transfer resume info
     *
     * @param xml XML document
     * @return File transfer resume info
     */
    public static FileTransferHttpResumeInfo parseFileTransferHttpResumeInfo(byte[] xml) {
        try {
            InputSource ftHttpInput = new InputSource(new ByteArrayInputStream(xml));
            FileTransferHttpResumeInfoParser ftHttpParser = new FileTransferHttpResumeInfoParser(ftHttpInput);
            return ftHttpParser.getResumeInfo();
        } catch(Exception e) {
            return null;
        }
    }
    
    /**
     * Create a text message
     * 
     * @param remote Remote contact
     * @param txt Text message
     * @param imdn IMDN flag
     * @return Text message
     */
    public static InstantMessage createTextMessage(String remote, String msg, boolean imdn) {
        String msgId = ChatUtils.generateMessageId();
        return new InstantMessage(msgId,
                remote,
                StringUtils.encodeUTF8(msg),
                imdn,null);
    }
    
    /**
     * Create a file transfer message
     * 
     * @param remote Remote contact
     * @param file File info
     * @param imdn IMDN flag
     * @return File message
     */
    public static FileTransferMessage createFileTransferMessage(String remote, String file, boolean imdn) {
        String msgId = ChatUtils.generateMessageId();
        return new FileTransferMessage(msgId,
                remote,
                file,
                imdn);
    }
    
    /**
     * Create a geoloc message
     * 
     * @param remote Remote contact
     * @param geoloc Geoloc info
     * @param imdn IMDN flag
     * @return Geoloc message
     */
    public static GeolocMessage createGeolocMessage(String remote, GeolocPush geoloc, boolean imdn) {
        String msgId = ChatUtils.generateMessageId();
        return new GeolocMessage(msgId,
                remote,
                geoloc,
                imdn);
    }
    
    /**
     * Get the first message
     * 
     * @param invite Request
     * @return First message
     */
    public static InstantMessage getFirstMessage(SipRequest invite) {
        InstantMessage msg = getFirstMessageFromCpim(invite);
        if (msg != null) {
            return msg;
        } else {
            return getFirstMessageFromSubject(invite);
        }
    }

    /**
     * Get the subject
     * 
     * @param invite Request
     * @return String
     */
    public static String getSubject(SipRequest invite) {
        return invite.getSubject();
    }

    /**
     * Get the subject
     * 
     * @param invite Request
     * @return String
     */
        public static String getFromAias(SipRequest invite) {
            return invite.getFrom();
        }

    /**
     * Get the first message from CPIM content
     * 
     * @param invite Request
     * @return First message
     */
    private static InstantMessage getFirstMessageFromCpim(SipRequest invite) {
        CpimMessage cpimMsg = ChatUtils.extractCpimMessage(invite);
        if (cpimMsg != null) {
            String remote = ChatUtils.getReferredIdentity(invite);
            String msgId = ChatUtils.getMessageId(invite);
            String content = cpimMsg.getMessageContent();
            Date date = cpimMsg.getMessageDate();
            String mime = cpimMsg.getContentType();
            if ((remote != null) && (msgId != null) && (content != null) && (mime != null)) {
                if (mime.contains(GeolocMessage.MIME_TYPE)) {
                    return null;/*new GeolocMessage(msgId,
                            remote,
                            ChatUtils.parseGeolocDocument(content),
                            ChatUtils.isImdnDisplayedRequested(invite),
                            date);*/
                } else
                if (mime.contains(FileTransferMessage.MIME_TYPE)) {
                    return new FileTransferMessage(msgId,
                            remote,
                            StringUtils.decodeUTF8(content),
                            ChatUtils.isImdnDisplayedRequested(invite),
                            date);
                } else {
                    return new InstantMessage(msgId,
                            remote,
                            StringUtils.decodeUTF8(content),
                            ChatUtils.isImdnDisplayedRequested(invite),
                            date,null);
                }
            } else {
                return null;
            }
        } else {
            return null;
        }
    }
    
    /**
     * Get the first message from the Subject header
     * 
     * @param invite Request
     * @return First message
     */
    private static InstantMessage getFirstMessageFromSubject(SipRequest invite) {
        String subject = invite.getSubject();
        if ((subject != null) && (subject.length() > 0)) {
            String remote = ChatUtils.getReferredIdentity(invite);
            if ((remote != null) && (subject != null)) {
                return new InstantMessage(ChatUtils.generateMessageId(),
                        remote,
                        StringUtils.decodeUTF8(subject),
                        ChatUtils.isImdnDisplayedRequested(invite),
                        new Date(),null);
            } else {
                return null;
            }
        } else {
            return null;
        }
    }    
    
    /**
     * Extract CPIM message from incoming INVITE request 
     * 
     * @param request Request
     * @return Boolean
     */
    public static CpimMessage extractCpimMessage(SipRequest request) {
        CpimMessage message = null;
        try {
            // Extract message from content/CPIM
            String content = request.getContent();
            String boundary = request.getBoundaryContentType();
            Multipart multi = new Multipart(content, boundary);
            if (multi.isMultipart()) {
                String cpimPart = multi.getPart(CpimMessage.MIME_TYPE);
                if (cpimPart != null) {
                    // CPIM part
                    CpimParser cpimParser = new CpimParser(cpimPart.getBytes());
                    message = cpimParser.getCpimMessage();
                }
            }
        } catch(Exception e) {
            message = null;
        }
        return message;
    }

    /**
     * Get list of participants from 'resource-list' present in XML document and
     * include the 'remote' as participant.
     * 
     * @return {@link ListOfParticipant} participant list
     * @author Deutsche Telekom AG
     */
    public static ListOfParticipant getListOfParticipants(SipRequest request) {
        ListOfParticipant participants = new ListOfParticipant();
        try {
            String content = request.getContent();
            String boundary = request.getBoundaryContentType();
            Multipart multi = new Multipart(content, boundary);
            if (multi.isMultipart()) {
                // Extract resource-lists
                String listPart = multi.getPart("application/resource-lists+xml");
                if (listPart != null) {
                    // Create list from XML
                    participants = new ListOfParticipant(listPart);
                    if (logger.isActivated()) {
                        logger.debug("getListOfParticipants1: " + participants);
                    }

                    // Include remote contact
                    String remote = getReferredIdentity(request);
                    participants.addParticipant(remote);
                }
            }
        } catch (Exception e) {
            // Nothing to do
        }
        return participants;
    }

    /**
     * Returns the data of the thumbnail file
     * 
     * @param filename Filename
     * @return Bytes or null in case of error
     */
    public static byte[] getFileThumbnail(String filename) {
        try {
            File file = new File(filename);
            byte [] data = new byte[(int)file.length()];
            FileInputStream fis = new FileInputStream(file);
            fis.read(data);
            fis.close();
            return data;
        } catch(Exception e) {
            return null;
        }
    }

    /**
     * Create a thumbnail from a filename
     *
     * @param filename Filename
     * @return Thumbnail
     */
    public static byte[] createFileThumbnail(String filename) {
        ByteArrayOutputStream out = new ByteArrayOutputStream();
        InputStream in = null;
        try {
            File file = new File(filename);
            in = new FileInputStream(file);
            Bitmap bitmap = BitmapFactory.decodeStream(in);
            int width = bitmap.getWidth();
            int height = bitmap.getHeight();
            long size = file.length();
            
            // Resize the bitmap
            int max = Math.max(width, height);
            if (max > 960) {
                float scale = 960f / max;
                width = Math.round(scale * width);
                height = Math.round(scale * height);
                bitmap = Bitmap.createScaledBitmap(bitmap, width, height, true);
            }

            float scale = 0.3f;
            Matrix matrix = new Matrix();
            matrix.postScale(scale, scale);

            // Recreate the new bitmap
            Bitmap resizedBitmap = Bitmap.createBitmap(
                    bitmap, 0, 0, width, height, matrix, true);

            // Compress the file to be under the limit (10KBytes)
            int quality = 90;
            int maxSize = MAX_THUMBNAIL_SIZE;
            if(RcsSettings.getInstance().supportOP01())
                maxSize = MAX_THUMBNAIL_SIZE_OP01;
            if (logger.isActivated()) {
                logger.debug("FTS createFileThumbnail4 start size: " + size);
            }
            if(size > maxSize) {
                while(size > maxSize && quality > 0) {
                    out = new ByteArrayOutputStream();
                    resizedBitmap.compress(CompressFormat.JPEG, quality, out);
                    out.flush();
                    out.close();
                    size = out.size();
                    if (logger.isActivated()) {
                        logger.debug("FTS createFileThumbnail5  size: " + size);
                    }
                    quality -= 10;
                }
            } else{
                out = new ByteArrayOutputStream();
                resizedBitmap.compress(CompressFormat.JPEG, 90, out);
                out.flush();
                out.close();
            }
            if(in != null){
                in.close();
            }
        } catch (Exception e) {
            e.printStackTrace();
            if (logger.isActivated()) {
                logger.debug("FTS createFileThumbnail0" +
                        " size; " + out.size() +
                        " byte :" + out.toByteArray());
            }
            //return null;
        } finally {
            try {
                if(in != null) {
                    in.close();
                }
                if(out != null) {
                    out.close();
                }
            } catch(Exception e) {
                e.printStackTrace();
                if (logger.isActivated()) {
                    logger.debug("FTS createFileThumbnail1");
                }
            }
        }
        if (out != null) {
            if (logger.isActivated()) {
                logger.debug("FTS createFileThumbnail3");
            }
            return out.toByteArray();
        } else {
            if (logger.isActivated()) {
                logger.debug("FTS createFileThumbnail2");
            }
            return null;
        }
    }

    /**
     * Create a thumbnail from a filename
     * 
     * @param filename Filename
     * @return Thumbnail
     */
    public static byte[] createVideoThumbnail(String filename) {
        ByteArrayOutputStream out = new ByteArrayOutputStream();
        if (logger.isActivated()) {
            logger.debug("FTS createVideoThumbnail");
        }
        InputStream in = null;
        try {
            File file = new File(filename);
            in = new FileInputStream(file);
            Bitmap bitmap = ThumbnailUtils.createVideoThumbnail(filename,Thumbnails.MINI_KIND);
            int width = bitmap.getWidth();
            int height = bitmap.getHeight();
            long size = file.length();

            // Resize the bitmap
            float scale = 0.4f;
            Matrix matrix = new Matrix();
            matrix.postScale(scale, scale);

            // Recreate the new bitmap
            Bitmap resizedBitmap = Bitmap.createBitmap(bitmap, 0, 0, width,
                    height, matrix, true);

            // Compress the file to be under the limit (10KBytes)
            int quality = 90;
            int maxSize = MAX_THUMBNAIL_SIZE;
            if(RcsSettings.getInstance().supportOP01())
                maxSize = MAX_THUMBNAIL_SIZE_OP01;
            if(size > maxSize && quality > 0) {
                while(size > maxSize) {
                    out = new ByteArrayOutputStream();
                    resizedBitmap.compress(CompressFormat.JPEG, quality, out);
                    out.flush();
                    out.close();
                    size = out.size();
                    quality -= 10;
                }
            } else{
                out = new ByteArrayOutputStream();
                resizedBitmap.compress(CompressFormat.JPEG, 90, out);
                out.flush();
                out.close();
            }
            if(in != null){
                in.close();
            }
        } catch (Exception e) {
            return null;
        }
        return out.toByteArray();
    }
    
    /**
     * Extract thumbnail from incoming INVITE request
     * 
     * @param request Request
     * @return Thumbnail
     */
    public static byte[] extractFileThumbnail(SipRequest request) {
        try {
            String boundary = request.getBoundaryContentType();
            byte[] rawContent = request.getRawContent();
            /* Use US-ASCII to get same offset of String and byte[] */
            String content = new String(rawContent, "US-ASCII");
            String boundaryLine = SipUtils.CRLF+Multipart.BOUNDARY_DELIMITER + boundary;
            int headerEnd = 0;
            int partBegin = 0;
          
            Multipart multi = new Multipart(content, boundary);
            if (multi.isMultipart()) {
                while ((partBegin = content.indexOf(boundaryLine, headerEnd)) >= 0) {
                    headerEnd = content.indexOf(SipUtils.CRLF+SipUtils.CRLF, partBegin);
                    String headers = content.substring(partBegin+boundaryLine.length(), headerEnd);
          
                    int begin = headers.indexOf(ContentTypeHeader.NAME);
                    int end = headers.indexOf(SipUtils.CRLF, begin);
                String mime;
                    if (end == -1) {
                        mime = headers.substring(begin+ContentTypeHeader.NAME.length()+1).trim();
                } else {
                        mime = headers.substring(begin+ContentTypeHeader.NAME.length()+1, end).trim();
                }    
                
                    if (mime.equalsIgnoreCase("image/jpeg") ||
                        mime.equalsIgnoreCase("image/png") ||
                        mime.equalsIgnoreCase("video/3gpp") ||
                        mime.equalsIgnoreCase("video/mpeg-4")) {
                
                        boolean encodeBase64 = false;
                       
                        begin = headers.indexOf(CpimMessage.HEADER_CONTENT_ENCODING);
                        end = headers.indexOf(SipUtils.CRLF, begin);
                        String encoding;
                        if (begin >= 0) {
                            if (end == -1) {
                                encoding = headers.substring(begin+CpimMessage.HEADER_CONTENT_ENCODING.length()+1).trim();
                    } else {
                                encoding = headers.substring(begin+CpimMessage.HEADER_CONTENT_ENCODING.length()+1, end).trim();
                    }
                            if (encoding.contains("base64"))
                                encodeBase64 = true;
                }
                        if (encodeBase64) {
                            return com.orangelabs.rcs.utils.Base64.decodeBase64(
                                    multi.getPart(mime).getBytes());
                    } else {
                            int partEnd = content.indexOf(boundaryLine, headerEnd);
                            byte[] thumbnail = new byte[partEnd-headerEnd-4];
                            System.arraycopy(rawContent, headerEnd+4, thumbnail, 0, thumbnail.length);
                            return thumbnail;
                        }
                    }
                }
                
                return null;
            }
        } catch(Exception e) {
            if (logger.isActivated()) {
                logger.debug("extractFileThumbnail exception");
                e.printStackTrace();
            }
            return null;
        }        
        
        return null;
    }

    /**
     * Is request is for FToHTTP
     *
     * @param request
     * @return true if FToHTTP
     */
    public static boolean isFileTransferOverHttp(SipRequest request) {
        CpimMessage message = extractCpimMessage(request);
        if (message != null && message.getContentType().startsWith(FileTransferHttpInfoDocument.MIME_TYPE)) {
            return true;
        } else {
            return false;
        }
    }

    /**
     * Get the HTTP file transfer info document
     *
     * @param request Request
     * @return FT HTTP info
     */
    public static FileTransferHttpInfoDocument getHttpFTInfo(SipRequest request) {
        InstantMessage message = getFirstMessage(request);
        if ((message != null) && (message instanceof FileTransferMessage)) {
            FileTransferMessage ftMsg = (FileTransferMessage)message;
            byte[] xml = ftMsg.getFileInfo().getBytes();
            return parseFileTransferHttpDocument(xml);
        } else {
            return null;
        }
    }
    
    public static int getEncodedMessageSize(String text) {
        int size = text.getBytes().length;

        if (RcsSettings.getInstance().isBase64EncodingSupported()){
            if (size % 3 == 0)
                size = size/3*4;
            else
                size = size/3*4+4;
        }

        return size;
    }
    
    /**
     * Create a chat message either text or geolocation.
     * 
     * @param msgId the message ID
     * @param apiMimeType The mime-type as exposed to client applications
     * @param content the content
     * @param contact The remote contact
     * @param displayName the display name
     * @param timestamp the timestamp
     * @param timestampSent The timestamp sent in payload for the chat message
     * @return ChatMessage
     */
    public static ChatMessage createChatMessage(String msgId, String apiMimeType, String content,
            ContactId contact, String displayName, long timestamp, long timestampSent) {
        if (MimeType.TEXT_MESSAGE.equals(apiMimeType)) {
            return new ChatMessage(msgId, contact, content, MimeType.TEXT_MESSAGE, timestamp,
                    timestampSent, displayName);

        } else if (MimeType.GEOLOC_MESSAGE.equals(apiMimeType)) {
            return new ChatMessage(msgId, contact, content, MimeType.GEOLOC_MESSAGE, timestamp,
                    timestampSent, displayName);
        }
        throw new IllegalArgumentException("Unable to create message, Invalid mimetype "
                + apiMimeType);
    }
    
    /**
     * Get reason code based on status 
     *
     * @param reasonCode String
     * @return ReasonCode
     */
    public static ReasonCode getReasonCode(String reasonCode){
        if (logger.isActivated()) {
            logger.info("getReasonCode " + reasonCode);
        }
        if(reasonCode.contains(ChatUtils.ANONYMITY_NOT_ALLOWED)){
            return ReasonCode.ANONYMITY_NOT_ALLOWED;
        } else if(reasonCode.contains(ChatUtils.FUNCTION_NOT_ALLOWED)){
            return ReasonCode.FUNCTION_NOT_ALLOWED;
        } else if(reasonCode.contains(ChatUtils.SIZE_EXCEEDED)){
            return ReasonCode.SIZE_EXCEEDED;
        } else if(reasonCode.contains(ChatUtils.TEST_ERROR)){
            return ReasonCode.TEST_ERROR;
        } else if(reasonCode.contains(ChatUtils.NO_DESTINATIONS)){
            return ReasonCode.NO_DESTINATIONS;
        } else if(reasonCode.contains(ChatUtils.VERSION_UNSUPPORTED)){
            return ReasonCode.VERSION_UNSUPPORTED;
        } else if(reasonCode.contains(ChatUtils.SERVICE_UNAUTHORIZED)){
            return ReasonCode.SERVICE_UNAUTHORIZED;
        } else if(reasonCode.contains(ChatUtils.DECLINED)){
            return ReasonCode.DECLINED;
        } else if(reasonCode.contains(ChatUtils.TIMEOUT)){
            return ReasonCode.TIMEOUT;
        } else if(reasonCode.contains(ChatUtils.BAD_REQUEST)){
            return ReasonCode.BAD_REQUEST;
        } else if(reasonCode.contains(ChatUtils.FORBIDDEN)){
            return ReasonCode.FORBIDDEN;
        } else if(reasonCode.contains(ChatUtils.REQUEST_TIMEDOUT)){
            return ReasonCode.REQUEST_TIMEDOUT;
        } else if(reasonCode.contains(ChatUtils.UNSUPPORTED_MEDIA)){
            return ReasonCode.UNSUPPORTED_MEDIA;
        } else if(reasonCode.contains(ChatUtils.INTERVAL_SMALL)){
            return ReasonCode.INTERVAL_SMALL;
        } else {
            return ReasonCode.UNSPECIFIED;
        }
    }
}
