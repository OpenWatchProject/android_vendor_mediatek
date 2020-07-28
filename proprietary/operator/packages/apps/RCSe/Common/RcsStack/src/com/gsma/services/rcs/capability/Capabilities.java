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

package com.gsma.services.rcs.capability;

import java.util.ArrayList;
import java.util.HashSet;
import java.util.List;
import java.util.Set;

import com.gsma.services.rcs.Logger;

import android.os.Parcel;
import android.os.Parcelable;

/**
 * Capabilities of a contact. This class encapsulates the different capabilities
 * which may be supported by the local user or a remote contact.
 *
 * @author Jean-Marc AUFFRET
 */
public class Capabilities implements Parcelable {
    /**
     * Image sharing support
     */
    private boolean imageSharing = false;

    public static final String TAG = "Capabilities";

    public void setImageSharingSupport(boolean imageSharing) {
        Logger.i(TAG, "setImageSharingSupport entry" + imageSharing);
        this.imageSharing = imageSharing;
    }

    /**
     * Video sharing support
     */
    private boolean videoSharing = false;

    public void setVideoSharingSupport(boolean videoSharing) {
        Logger.i(TAG, "setVideoSharingSupport entry" + videoSharing);
        this.videoSharing = videoSharing;
    }

    /**
     * IM session support
     */
    private boolean imSession = false;

    /**
     * File transfer support
     */
    private boolean fileTransfer = false;

    /**
     * Geolocation push support
     */
    private boolean geolocPush = false;

    /**
     * IP voice call support
     */
    private boolean ipVoiceCall = false;

    /**
     * IP video call support
     */
    private boolean ipVideoCall = false;

    /**
     * List of supported extensions
     */
    private Set<String> extensions = new HashSet<String>();

    /**
     * Automata flag
     */
    private boolean automata = false;

    /**
     * Automata flag
     */
    private boolean fileTransferHttpSupported = false;

    /**
     * Automata flag
     */
    private boolean rcsContact = false;

    private boolean burnAfterRead = false;

    /**
     * Integrated Mode of this contact
     */
    private boolean integratedMessagingMode = false;

    /**
     * The timestamp of the last capability response
     */
    private long timestamp = 0L;

    /**
     * Indicates the file transfer capability is supported.
     */
    public static final int CAPABILITY_FILE_TRANSFER = 0x00000001;

    /**
     * Indicates the IM capability is supported.
     */
    public static final int CAPABILITY_IM = 0x00000002;

    /**
     * Indicates the geoloc push capability is supported.
     */
    public static final int CAPABILITY_GEOLOC_PUSH = 0x00000004;

    /**
     * Indicates the image sharing capability is supported.
     */
    public static final int CAPABILITY_IMAGE_SHARING = 0x00000008;

    /**
     * Indicates the video sharing capability is supported.
     */
    public static final int CAPABILITY_VIDEO_SHARING = 0x00000010;


    /*
     * IR94 CAPABILITIES
     */
    private boolean IR94_VoiceCall = false;
    private boolean IR94_VideoCall = false;
    private boolean IR94_DuplexMode = false;
    /*
     * @ IR94 ENDS
     */

    /**
     * Standalone Msg.
     */
    private boolean standaloneMsg = false;

    /**
     * SocialPresence.
     */
    private boolean socialPresence = false;

     /**
     * FileTransferThumbnail.
     */
    private boolean fileTransferThumbnail = false;

     /**
     * FileTransferStoreForward.
     */
    private boolean fileTransferStoreForward = false;

    /**
     * GroupChatStoreForward.
     */
    private boolean groupChatStoreForward = false;

    /**
     * GroupChatStoreForward.
     */
    private boolean noResourceContact = false;

    /**
     * PresenceDiscovery.
     */
    private boolean presenceDiscovery = false;

    public void setIntegratedMessagingMode(boolean integratedMessagingMode) {
        Logger.i(TAG, "setIntegratedMessagingMode entry" + integratedMessagingMode);
        this.integratedMessagingMode = integratedMessagingMode;
    }

    public boolean isIntegratedMessagingMode() {
        Logger.i(TAG, "isIntegratedMessagingMode entry" + integratedMessagingMode);
        return integratedMessagingMode;
    }

    /**
     * Cs Video support
     */
    private boolean csVideoSupported = false;

    public boolean isCsVideoSupported() {
        return csVideoSupported;
    }

    /**
     * Chatbot
     */
    private boolean chatbot = false;
    private boolean chatbotSa = false;

    /**
     * enriched calling
     */
    private boolean callcomposer = false;

    /**
     * Constructor
     *
     * @param imageSharing Image sharing support
     * @param videoSharing Video sharing support
     * @param imSession IM/Chat support
     * @param fileTransfer File transfer support
     * @param geolocPush Geolocation push support
     * @param ipVoiceCall IP voice call support
     * @param ipVideoCall IP video call support
     * @param extensions Set of supported extensions
     * @param automata Automata flag
     * @hide
     */
    public Capabilities(boolean imageSharing, boolean videoSharing, boolean imSession,
            boolean fileTransfer, boolean geolocPush,
            boolean ipVoiceCall, boolean ipVideoCall,
            Set<String> extensions, boolean automata, boolean fileTransferHttpSupport, boolean rcsContact, boolean integratedMessagingMode, boolean csVideoSupported,
            boolean isBurnAfterRead
            ) {
        Logger.i(TAG, "Capabilities entry , values are " + "Image:" + imageSharing
                + " video:" + videoSharing + " imSession:" + imSession
                + "\nFT:" + fileTransfer + " GeoPush:" + geolocPush
                + " ipVoice:" + ipVoiceCall + " ipVideo:" + ipVideoCall
                + "\next:" + extensions + " auto:" + automata
                + " FtHttp:" + fileTransferHttpSupport + " rcsContact:" + rcsContact
                + "\nIntMsgMode-" + integratedMessagingMode + " csVideo:" + csVideoSupported);
        this.imageSharing = imageSharing;
        this.videoSharing = videoSharing;
        this.imSession = imSession;
        this.fileTransfer = fileTransfer;
        this.geolocPush = geolocPush;
        this.ipVoiceCall = ipVoiceCall;
        this.ipVideoCall = ipVideoCall;
        this.extensions = extensions;
        this.automata = automata;
        this.fileTransferHttpSupported = fileTransferHttpSupport;
        this.rcsContact = rcsContact;
        this.integratedMessagingMode = integratedMessagingMode;
        this.csVideoSupported = csVideoSupported;
        this.burnAfterRead = isBurnAfterRead;
    }

    /**
     * Constructor
     *
     * @param imageSharing Image sharing support
     * @param videoSharing Video sharing support
     * @param imSession IM/Chat support
     * @param fileTransfer File transfer support
     * @param geolocPush Geolocation push support
     * @param ipVoiceCall IP voice call support
     * @param ipVideoCall IP video call support
     * @param extensions Set of supported extensions
     * @param automata Automata flag
     * @hide
     */
    public Capabilities(boolean imageSharing, boolean videoSharing, boolean imSession,
            boolean fileTransfer, boolean geolocPush,
            boolean ipVoiceCall, boolean ipVideoCall,
            Set<String> extensions, boolean automata, boolean fileTransferHttpSupport, boolean rcsContact, boolean integratedMessagingMode, boolean csVideoSupported,
            boolean isBurnAfterRead, long timestamp
            ) {
        Logger.i(TAG, "Capabilities entry , values are " + "Image:" + imageSharing
                + " video:" + videoSharing + " imSession:" + imSession
                + "\nFT:" + fileTransfer + " GeoPush:" + geolocPush
                + " ipVoice:" + ipVoiceCall + " ipVideo:" + ipVideoCall
                + "\next:" + extensions + " auto:" + automata
                + " FtHttp:" + fileTransferHttpSupport + " rcsContact:" + rcsContact
                + "\nIntMsgMode-" + integratedMessagingMode + " csVideo:" + csVideoSupported);
        this.imageSharing = imageSharing;
        this.videoSharing = videoSharing;
        this.imSession = imSession;
        this.fileTransfer = fileTransfer;
        this.geolocPush = geolocPush;
        this.ipVoiceCall = ipVoiceCall;
        this.ipVideoCall = ipVideoCall;
        this.extensions = extensions;
        this.automata = automata;
        this.fileTransferHttpSupported = fileTransferHttpSupport;
        this.rcsContact = rcsContact;
        this.integratedMessagingMode = integratedMessagingMode;
        this.csVideoSupported = csVideoSupported;
        this.burnAfterRead = isBurnAfterRead;
        this.timestamp = timestamp;
    }

    public Capabilities(boolean imageSharing, boolean videoSharing, boolean imSession,
            boolean fileTransfer, boolean geolocPush,
            boolean ipVoiceCall, boolean ipVideoCall,
            Set<String> extensions, boolean automata, boolean fileTransferHttpSupport, boolean rcsContact, boolean integratedMessagingMode, boolean csVideoSupported) {
        Logger.i(TAG, "Capabilities entry , values are " + "Image:" + imageSharing
                + " video:" + videoSharing + " imSession:" + imSession
                + "\nFT:" + fileTransfer + " GeoPush:" + geolocPush
                + " ipVoice:" + ipVoiceCall + " ipVideo:" + ipVideoCall
                + "\next:" + extensions + " auto:" + automata
                + " FtHttp:" + fileTransferHttpSupport + " rcsContact:" + rcsContact
                + "\nIntMsgMode-" + integratedMessagingMode + " csVideo:" + csVideoSupported);
        this.imageSharing = imageSharing;
        this.videoSharing = videoSharing;
        this.imSession = imSession;
        this.fileTransfer = fileTransfer;
        this.geolocPush = geolocPush;
        this.ipVoiceCall = ipVoiceCall;
        this.ipVideoCall = ipVideoCall;
        this.extensions = extensions;
        this.automata = automata;
        this.fileTransferHttpSupported = fileTransferHttpSupport;
        this.rcsContact = rcsContact;
        this.integratedMessagingMode = integratedMessagingMode;
        this.csVideoSupported = csVideoSupported;

    }

    public Capabilities(boolean imageSharing, boolean videoSharing, boolean imSession,
            boolean fileTransfer, boolean geolocPush,
            boolean ipVoiceCall, boolean ipVideoCall,
            Set<String> extensions, boolean automata, boolean fileTransferHttpSupport, boolean rcsContact, boolean integratedMessagingMode, boolean csVideoSupported,
            long timestamp) {
        Logger.i(TAG, "Capabilities entry , values are " + "Image:" + imageSharing
                + " video:" + videoSharing + " imSession:" + imSession
                + "\nFT:" + fileTransfer + " GeoPush:" + geolocPush
                + " ipVoice:" + ipVoiceCall + " ipVideo:" + ipVideoCall
                + "\next:" + extensions + " auto:" + automata
                + " FtHttp:" + fileTransferHttpSupport + " rcsContact:" + rcsContact
                + "\nIntMsgMode-" + integratedMessagingMode + " csVideo:" + csVideoSupported);
        this.imageSharing = imageSharing;
        this.videoSharing = videoSharing;
        this.imSession = imSession;
        this.fileTransfer = fileTransfer;
        this.geolocPush = geolocPush;
        this.ipVoiceCall = ipVoiceCall;
        this.ipVideoCall = ipVideoCall;
        this.extensions = extensions;
        this.automata = automata;
        this.fileTransferHttpSupported = fileTransferHttpSupport;
        this.rcsContact = rcsContact;
        this.integratedMessagingMode = integratedMessagingMode;
        this.csVideoSupported = csVideoSupported;
        this.timestamp = timestamp;

    }

    /**
     * Constructor
     *
     * @param source Parcelable source
     * @hide
     */
    public Capabilities(Parcel source) {
        this.imageSharing = source.readInt() != 0;
        this.videoSharing = source.readInt() != 0;
        this.imSession = source.readInt() != 0;
        this.fileTransfer = source.readInt() != 0;
        List<String> exts = new ArrayList<String>();
        source.readStringList(exts);
        this.extensions = new HashSet<String>(exts);
        this.geolocPush = source.readInt() != 0;
        this.ipVoiceCall = source.readInt() != 0;
        this.ipVideoCall = source.readInt() != 0;
        this.automata = source.readInt() != 0;
        this.fileTransferHttpSupported = source.readInt() != 0;
        this.rcsContact = source.readInt() != 0;
        this.integratedMessagingMode = source.readInt() != 0;
        this.csVideoSupported = source.readInt() != 0;
        this.burnAfterRead = source.readInt() != 0;
        this.IR94_VoiceCall = source.readInt() != 0;
        this.IR94_VideoCall = source.readInt() != 0;
        this.IR94_DuplexMode = source.readInt() != 0;
        this.timestamp = source.readLong();
        this.standaloneMsg = source.readInt() != 0;
        this.socialPresence= source.readInt() != 0;
        this.fileTransferThumbnail= source.readInt() != 0;
        this.fileTransferStoreForward= source.readInt() != 0;
        this.groupChatStoreForward= source.readInt() != 0;
        this.noResourceContact= source.readInt() != 0;
        this.presenceDiscovery = source.readInt() != 0;
        this.chatbot = source.readInt() != 0;
        this.chatbotSa = source.readInt() != 0;
        this.callcomposer = source.readInt() != 0;
    }

    /**
     * Describe the kinds of special objects contained in this Parcelable's
     * marshalled representation
     *
     * @return Integer
     * @hide
     */
    public int describeContents() {
        return 0;
    }

    /**
     * Write parcelable object
     *
     * @param dest The Parcel in which the object should be written
     * @param flags Additional flags about how the object should be written
     * @hide
     */
    public void writeToParcel(Parcel dest, int flags) {
        dest.writeInt(imageSharing ? 1 : 0);
        dest.writeInt(videoSharing ? 1 : 0);
        dest.writeInt(imSession ? 1 : 0);
        dest.writeInt(fileTransfer ? 1 : 0);
        if (extensions != null) {
            List<String> exts = new ArrayList<String>();
            exts.addAll(extensions);
            dest.writeStringList(exts);
        }
        dest.writeInt(geolocPush ? 1 : 0);
        dest.writeInt(ipVoiceCall ? 1 : 0);
        dest.writeInt(ipVideoCall ? 1 : 0);
        dest.writeInt(automata ? 1 : 0);
        dest.writeInt(fileTransferHttpSupported ? 1 : 0);
        dest.writeInt(rcsContact ? 1 : 0);
        dest.writeInt(integratedMessagingMode ? 1 : 0);
        dest.writeInt(csVideoSupported ? 1 : 0);
        dest.writeInt(burnAfterRead ? 1 : 0);
        dest.writeInt(IR94_VoiceCall ? 1 : 0);
        dest.writeInt(IR94_VideoCall ? 1 : 0);
        dest.writeInt(IR94_DuplexMode ? 1 : 0);
        dest.writeLong(timestamp);
        dest.writeInt(standaloneMsg ? 1 : 0);
        dest.writeInt(socialPresence ? 1 : 0);
        dest.writeInt(fileTransferThumbnail ? 1 : 0);
        dest.writeInt(fileTransferStoreForward ? 1 : 0);
        dest.writeInt(groupChatStoreForward ? 1 : 0);
        dest.writeInt(noResourceContact ? 1 : 0);
        dest.writeInt(presenceDiscovery? 1 : 0);
        dest.writeInt(chatbot ? 1 : 0);
        dest.writeInt(chatbotSa ? 1 : 0);
        dest.writeInt(callcomposer ? 1 : 0);
    }

    /**
     * Parcelable creator
     *
     * @hide
     */
    public static final Parcelable.Creator<Capabilities> CREATOR
            = new Parcelable.Creator<Capabilities>() {
        public Capabilities createFromParcel(Parcel source) {
            return new Capabilities(source);
        }

        public Capabilities[] newArray(int size) {
            return new Capabilities[size];
        }
    };

    /**
     * Timestamp of the last capability response (in milliseconds)
     *
     * @return the timestamp of the last capability response
     */
    public long getTimestamp() {
        return timestamp;
    }

    public void setTimestamp(long timestamp){
        this.timestamp = timestamp;
    }

    /**
     * Is image sharing supported
     *
     * @return Returns true if supported else returns false
     */
    public boolean isImageSharingSupported() {
        Logger.i(TAG, "isImageSharingSupported value " + imageSharing);
        return imageSharing;
    }

    /**
     * Is video sharing supported
     *
     * @return Returns true if supported else returns false
     */
    public boolean isVideoSharingSupported() {
        Logger.i(TAG, "isVideoSharingSupported value " + videoSharing);
        return videoSharing;
    }

    /**
     * Is IM session supported
     *
     * @return Returns true if supported else returns false
     */
    public boolean isImSessionSupported() {
        Logger.i(TAG, "isImSessionSupported value " + imSession);
        return imSession;
    }

    /**
     * Set the IM session support
     *
     * @param supported Supported
     */
    public void setImSessionSupport(boolean supported) {
        this.imSession = supported;
    }

    /**
     * Is file transfer supported
     *
     * @return Returns true if supported else returns false
     */
    public boolean isFileTransferSupported() {
        Logger.i(TAG, "isFileTransferSupported value " + fileTransfer);
        return fileTransfer;
    }

    /**
     * Set the file transfer support
     *
     * @param supported Supported
     */
    public void setFileTransferSupport(boolean supported) {
        this.fileTransfer = supported;
    }

    /**
     * Is geolocation push supported
     *
     * @return Returns true if supported else returns false
     */
    public boolean isGeolocPushSupported() {
        Logger.i(TAG, "isGeolocPushSupported value " + geolocPush);
        return geolocPush;
    }

    /**
     * Set the Geolocation Push support
     *
     * @param supported Supported
     */
    public void setGeolocationPushSupport(boolean supported) {
        this.geolocPush = supported;
    }

    /**
     * Is IP voice call supported
     *
     * @return Returns true if supported else returns false
     */
    public boolean isIPVoiceCallSupported() {
        Logger.i(TAG, "isIPVoiceCallSupported value " + ipVoiceCall);
        return ipVoiceCall;
    }

    /**
     * Set the IP voice call support
     *
     * @param supported Supported
     */
    public void setIPVoiceCallSupport(boolean supported) {
        this.ipVoiceCall = supported;
    }

    /**
     * Is IP video call supported
     *
     * @return Returns true if supported else returns false
     */
    public boolean isIPVideoCallSupported() {
        Logger.i(TAG, "isIPVideoCallSupported value " + ipVideoCall);
        return ipVideoCall;
    }

    /**
     * Set the IP video call support
     *
     * @param supported Supported
     */
    public void setIPVideoCallSupport(boolean supported) {
        this.ipVideoCall = supported;
    }

    /**
     * Is extension supported
     *
     * @param tag Feature tag
     * @return Returns true if supported else returns false
     */
    public boolean isExtensionSupported(String tag) {
        Logger.i(TAG, "isExtensionSupported value " + extensions.contains(tag));
        return extensions.contains(tag);
    }

    /**
     * Get list of supported extensions
     *
     * @return List of feature tags
     */
    public Set<String> getSupportedExtensions() {
        Logger.i(TAG, "getSupportedExtensions value " + extensions);
        return extensions;
    }

    /**
     * Is automata
     *
     * @return Returns true if it's an automata else returns false
     */
    public boolean isAutomata() {
        Logger.i(TAG, "isAutomata value" + automata);
        return automata;
    }

    /**
     * If FT HTTP Supported
     *
     * @return Returns true if it's supported
     */
    public boolean isFileTransferHttpSupported() {
        Logger.i(TAG, "isFileTransferHttpSupported value" + fileTransferHttpSupported);
        return fileTransferHttpSupported;
    }

    /**
     * Set the file transfer over HTTP support
     *
     * @param supported Supported
     */
    public void setFileTransferHttpSupport(boolean supported) {
        this.fileTransferHttpSupported = supported;
    }

    /**
     * Is contact RCSe supported
     *
     * @return Returns true if it's rcs contact
     */
    public boolean isSupportedRcseContact() {
        Logger.i(TAG, "isSupportedRcseContact value" + rcsContact);
        return rcsContact;
    }

    /**
     * BURN AFTER READING
     *
     */

    public boolean isBurnAfterRead() {
        return burnAfterRead;
    }

    /**
    * Standalone Msg.
    * @return Returns true if standaloneMsg capbility
    */
    public boolean isStandaloneMsgSupported() {
        return standaloneMsg;
    }

    /**
    * Standalone Msg
    * @param standaloneMsg capbility
    */
    public void setStandaloneMsgSupport(boolean standaloneMsg) {
        this.standaloneMsg = standaloneMsg;
    }

     /**
    * SocialPresence
    * @return Returns true if SocialPresence capbility
    */
    public boolean isSocialPresenceSupported() {
        return socialPresence;
    }

    /**
    * SocialPresence
    * @param SocialPresence capbility
    */
    public void setSocialPresenceSupport(boolean supported) {
        socialPresence = supported;
    }

     /**
    * FileTransferThumbnail
    * @return Returns true if FileTransferThumbnail capbility
    */
    public boolean isFileTransferThumbnailSupported() {
        return fileTransferThumbnail;
    }

    /**
    * FileTransferThumbnail
    * @param FileTransferThumbnail capbility
    */
    public void setFileTransferThumbnailSupport(boolean supported) {
        fileTransferThumbnail = supported;
    }

     /**
    * FileTransferStoreForward
    * @return Returns true if FileTransferStoreForward capbility
    */
    public boolean isFileTransferStoreForwardSupported() {
        return fileTransferStoreForward;
    }

    /**
    * FileTransferStoreForward
    * @param FileTransferStoreForward capbility
    */
    public void setFileTransferStoreForwardSupport(boolean supported) {
        fileTransferStoreForward = supported;
    }

     /**
    * GroupChatStoreForward.
    * @return Returns true if GroupChatStoreForward capbility
    */
    public boolean isGroupChatStoreForwardSupported() {
        return groupChatStoreForward;
    }

    /**
    * GroupChatStoreForward
    * @param GroupChatStoreForward capbility
    */
    public void setGroupChatStoreForwardSupport(boolean supported) {
        groupChatStoreForward = supported;
    }

     /**
    * NoResourceContact
    * @return Returns true if NoResourceContact capbility
    */
    public boolean isNoResourceContactSupported() {
        return noResourceContact;
    }

    /**
    * NoResourceContact
    * @param NoResourceContact capbility
    */
    public void setNoResourceContactSupport(boolean supported) {
        noResourceContact = supported;
    }

     /**
    * PresenceDiscovery
    * @return Returns true if PresenceDiscovery capbility
    */
    public boolean isPresenceDiscoverySupported() {
        return noResourceContact;
    }

    /**
    * PresenceDiscovery
    * @param PresenceDiscovery capbility
    */
    public void setPresenceDiscoverySupport(boolean supported) {
        presenceDiscovery = supported;
    }

    /**
        * IR94 voice and video call
    */
    public void setIR94_VoiceCall(boolean supported) {
        this.IR94_VoiceCall = supported;
    }

    public boolean isIR94_VoiceCallSupported() {
        return IR94_VoiceCall;
    }

    public void setIR94_VideoCall(boolean supported) {
        this.IR94_VideoCall = supported;
    }

    public boolean isIR94_VideoCallSupported() {
        return IR94_VideoCall;
    }

    public void setIR94_DuplexMode(boolean supported) {
        this.IR94_DuplexMode = supported;
    }

    public boolean isIR94_DuplexModeSupported() {
        return IR94_DuplexMode;
    }

    private int isCapabilitySupported(boolean supported, int capabilityFlag) {
        return (supported) ? capabilityFlag : 0;
    }

    /**
     * Gets the combination of supported capabilities.
     *
     * @return the combination of supported capabilities
     */
    private int getSupportedCapabilities() {
        int result = isCapabilitySupported(fileTransfer, CAPABILITY_FILE_TRANSFER);
        result |= isCapabilitySupported(imSession, CAPABILITY_IM);
        result |= isCapabilitySupported(geolocPush, CAPABILITY_GEOLOC_PUSH);
        result |= isCapabilitySupported(imageSharing, CAPABILITY_IMAGE_SHARING);
        result |= isCapabilitySupported(videoSharing, CAPABILITY_VIDEO_SHARING);
        return result;
    }

    /**
     * Tests for the support of capabilities on this instance.
     *
     * @return True if the capabilities are supported.
     */
    public boolean hasCapabilities(int capabilities) {
        return (getSupportedCapabilities() & capabilities) == capabilities;
    }

    public void setChatbotSupport(boolean chatbot) {
        this.chatbot = chatbot;
    }

    public boolean isChatbotSupported() {
        return chatbot;
    }

    public void setChatbotSaSupport(boolean chatbotSa) {
        this.chatbotSa = chatbotSa;
    }

    public boolean isChatbotSaSupported() {
        return chatbotSa;
    }

    public void setCallcomposerSupport(boolean callcomposer) {
        this.callcomposer = callcomposer;
    }

    public boolean isCallcomposerSupported() {
        return callcomposer;
    }
}
