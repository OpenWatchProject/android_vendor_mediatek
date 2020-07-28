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

package com.orangelabs.rcs.provider.settings;

import android.net.ConnectivityManager;
import android.net.Uri;
import android.util.SparseArray;

import com.gsma.services.rcs.CommonServiceConfiguration.MessagingMethod;
import com.gsma.services.rcs.CommonServiceConfiguration.MessagingMode;
import com.gsma.services.rcs.CommonServiceConfiguration.MinimumBatteryLevel;


/**
 * RCS settings data constants
 *
 * @author jexa7410
 */
public class RcsSettingsData {
    /**
     * Database URI
     */
    static final Uri CONTENT_URI = Uri.parse("content://com.orangelabs.rcs.settings/settings");


public static final Uri FIRST_USER_ACCOUNT_CONTENT_URI = Uri
            .parse("content://com.orangelabs.rcs.settings/"
                    + RcsSettingsProvider.FIRST_USER_ACCOUNT_TABLE);
    /**
     * Column name
     */
    public static final String KEY_ID = "_id";

    /**
     * Column name
     */
    public static final String KEY_KEY = "key";

    /**
     * Column name
     */
    static final String KEY_VALUE = "value";

    // ---------------------------------------------------------------------------
    // Constants
    // ---------------------------------------------------------------------------

    /**
     * Default group chat conference URI
     */
    public static final String DEFAULT_GROUP_CHAT_URI = "sip:foo@bar";

    public static final String DEFAULT_MULTI_CHAT_URI= "sip:msgas@as99.message.rcs1.chinamobile.com";

    /**
     * Boolean value "true"
     */
    public static final String TRUE = Boolean.toString(true);

    /**
     * Boolean value "false"
     */
    public static final String FALSE = Boolean.toString(false);

    /**
     * GIBA authentication
     */
    public static final String GIBA_AUTHENT = "GIBA";

    /**
     * HTTP Digest authentication
     */
    public static final String DIGEST_AUTHENT = "DIGEST";

    /**
     * AKA  Digest authentication
     */
     public static final String AKA_AUTHENT = "AKA";

    /**
     * HTTP Digest authentication
     */
    public static final String VOLTE_AUTHENT = "VOLTE";


    /**
     * Any access
     */
    public static final int ANY_ACCESS = -1;

    /**
     * Mobile access
     */
    public static final int MOBILE_ACCESS = ConnectivityManager.TYPE_MOBILE;

    /**
     * Wi-Fi access
     */
    public static final int WIFI_ACCESS = ConnectivityManager.TYPE_WIFI;
/**
     * M:Save Access Network Info.@{T-Mobile
     */
    public static final String CURRENT_ACCESS_NETWORKINFO = "CurrentAccessNetworkInfo";
    /**
     * @}
     */

    /**
     * M:Save Last Access Network Info.@{T-Mobile
     */
    public static final String LAST_ACCESS_NETWORKINFO = "LastAccessNetworkInfo";
    /**
     * @}
     */
    /**
     * File type for certificate
     */
    public static final String CERTIFICATE_FILE_TYPE = ".crt";

    /**
     * No auto config mode
     */
    public static final int NO_AUTO_CONFIG = 0;

    /**
     * HTTPS auto config mode
     */
    public static final int HTTPS_AUTO_CONFIG = 1;

    /**
     * File transfer over MSRP
     */
    public static final String FT_PROTOCOL_MSRP = "MSRP";

    /**
     * File transfer over HTTP
     */
    public static final String FT_PROTOCOL_HTTP = "HTTP";

    // ---------------------------------------------------------------------------
    // UI settings
    // ---------------------------------------------------------------------------

    /**
     * Activate or not the RCS service
     */
    public static final String SERVICE_ACTIVATED = "ServiceActivated";

    /**
     * Ringtone which is played when a social presence sharing invitation is received
     */
    public static final String PRESENCE_INVITATION_RINGTONE = "PresenceInvitationRingtone";

    /**
     * Vibrate or not when a social presence sharing invitation is received
     */
    public static final String PRESENCE_INVITATION_VIBRATE = "PresenceInvitationVibrate";

    /**
     * Ringtone which is played when a content sharing invitation is received
     */
    public static final String CSH_INVITATION_RINGTONE = "CShInvitationRingtone";

    /**
     * Vibrate or not when a content sharing invitation is received
     */
    public static final String CSH_INVITATION_VIBRATE = "CShInvitationVibrate";

    /**
     * Make a beep or not when content sharing is available during a call
     */
    public static final String CSH_AVAILABLE_BEEP = "CShAvailableBeep";

    /**
     * Ringtone which is played when a file transfer invitation is received
     */
    public static final String FILETRANSFER_INVITATION_RINGTONE = "FileTransferInvitationRingtone";

    /**
     * Vibrate or not when a file transfer invitation is received
     */
    public static final String FILETRANSFER_INVITATION_VIBRATE = "FileTransferInvitationVibrate";

    /**
     * Ringtone which is played when a chat invitation is received
     */
    public static final String CHAT_INVITATION_RINGTONE = "ChatInvitationRingtone";

    /**
     * Vibrate or not when a chat invitation is received
     */
    public static final String CHAT_INVITATION_VIBRATE = "ChatInvitationVibrate";

    /**
     * Send or not the displayed notification
     */
    public static final String CHAT_DISPLAYED_NOTIFICATION = "ChatDisplayedNotification";

    /**
     * Predefined freetext
     */
    public static final String FREETEXT1 = "Freetext1";

    /**
     * Predefined freetext
     */
    public static final String FREETEXT2 = "Freetext2";

    /**
     * Predefined freetext
     */
    public static final String FREETEXT3 = "Freetext3";

    /**
     * Predefined freetext
     */
    public static final String FREETEXT4 = "Freetext4";

    /**
     * Battery level minimum
     */
    public static final String MIN_BATTERY_LEVEL = "MinBatteryLevel";

    // ---------------------------------------------------------------------------
    // Service settings
    // ---------------------------------------------------------------------------

    /**
     * Max file-icon size
     */
    public static final String MAX_FILE_ICON_SIZE = "FileIconSize";

    /**
     * Max photo-icon size
     */
    public static final String MAX_PHOTO_ICON_SIZE = "MaxPhotoIconSize";

    /**
     * Max length of the freetext
     */
    public static final String MAX_FREETXT_LENGTH = "MaxFreetextLength";

    /**
     * Max number of participants in a group chat
     */
    public static final String MAX_CHAT_PARTICIPANTS = "MaxChatParticipants";

    /**
     * Max length of a chat message
     */
    public static final String MAX_CHAT_MSG_LENGTH = "MaxChatMessageLength";

    /**
     * Max length of a group chat message
     */
    public static final String MAX_GROUPCHAT_MSG_LENGTH = "MaxGroupChatMessageLength";

    /**
     * Idle duration of a chat session
     */
    public static final String CHAT_IDLE_DURATION = "ChatIdleDuration";

        /**
     * Satus of joyn master switch
     */
    public static final String JOYN_DISABLE_STATUS = "JoynDisableStatus";


    /**
     * Max size of a file transfer
     */
    public static final String MAX_FILE_TRANSFER_SIZE = "MaxFileTransferSize";

    /**
     * Warning threshold for file transfer size
     */
    public static final String WARN_FILE_TRANSFER_SIZE = "WarnFileTransferSize";

    /**
     * Max size of an image share
     */
    public static final String MAX_IMAGE_SHARE_SIZE = "MaxImageShareSize";

    /**
     * Warning threshold for image share size
     */
    public static final String WARN_IMAGE_SHARE_SIZE = "WarnImageShareSize";

    /**
     * Max duration of a video share
     */
    public static final String MAX_VIDEO_SHARE_DURATION = "MaxVideoShareDuration";

    /**
     * Max number of simultaneous chat sessions
     */
    public static final String MAX_CHAT_SESSIONS = "MaxChatSessions";

    /**
     * Max number of simultaneous file transfer sessions
     */
    public static final String MAX_FILE_TRANSFER_SESSIONS = "MaxFileTransferSessions";

    /**
     * Max number of simultaneous IP call sessions
     */
    public static final String MAX_IP_CALL_SESSIONS = "MaxIpCallSessions";

    /**
     * Activate or not SMS fallback service
     */
    public static final String SMS_FALLBACK_SERVICE = "SmsFallbackService";

    /**
     * Auto accept file transfer invitation
     */
    public static final String AUTO_ACCEPT_FILE_TRANSFER = "AutoAcceptFileTransfer";

    /**
     * Auto accept chat invitation
     */
    public static final String AUTO_ACCEPT_CHAT = "AutoAcceptChat";

    /**
     * Auto accept group chat invitation
     */
    public static final String AUTO_ACCEPT_GROUP_CHAT = "AutoAcceptGroupChat";

    /**
     * Display a warning if Store & Forward service is activated
     */
    public static final String WARN_SF_SERVICE = "StoreForwardServiceWarning";

    /**
     * Define when the chat receiver sends the 200 OK back to the sender
     */
    public static final String IM_SESSION_START = "ImSessionStart";

    /**
     * Max entries for chat log
     */
    public static final String MAX_CHAT_LOG_ENTRIES = "MaxChatLogEntries";

    /**
     * Max entries for richcall log
     */
    public static final String MAX_RICHCALL_LOG_ENTRIES = "MaxRichcallLogEntries";

    /**
     * Max entries for IP call log
     */
    public static final String MAX_IPCALL_LOG_ENTRIES = "MaxIpcallLogEntries";

    /**
     * Max length of a geolocation label
     */
    public static final String MAX_GEOLOC_LABEL_LENGTH = "MaxGeolocLabelLength";

    /**
     * Geolocation expiration time
     */
    public static final String GEOLOC_EXPIRATION_TIME = "GeolocExpirationTime";

    /**
     * Minimum storage capacity
     */
    public static final String MIN_STORAGE_CAPACITY = "MinStorageCapacity";

    /**
     * Convergent messaging UX option
     */
    public static final String CONVERGENT_MESSAGING_UX = "ConvergentMessagingUx";

    /**
     * Voice breakout option
     */
    public static final String VOICE_BREAKOUT = "VoiceBreakout";

    // ---------------------------------------------------------------------------
    // User profile settings
    // ---------------------------------------------------------------------------

    /**
     * IMS username or username part of the IMPU (for HTTP Digest only)
     */
    public static final String USERPROFILE_IMS_USERNAME = "ImsUsername";

    /**
     * @tct-stack wuquan add for full public_user_identify but not only number part
     */
    public static final String USERPROFILE_IMS_USERNAME_FULL = "ImsUsername_full";

    /**
     * IMS display name
     */
    public static final String USERPROFILE_IMS_DISPLAY_NAME = "ImsDisplayName";

    /**
     * IMS home domain
     */
    public static final String USERPROFILE_IMS_HOME_DOMAIN = "ImsHomeDomain";

    /**
     * IMS private URI or IMPI (for HTTP Digest only)
     */
    public static final String USERPROFILE_IMS_PRIVATE_ID = "ImsPrivateId";

    /**
     * IMS password (for HTTP Digest only)
     */
    public static final String USERPROFILE_IMS_PASSWORD = "ImsPassword";

    /**
     * IMS realm (for HTTP Digest only)
     */
    public static final String USERPROFILE_IMS_REALM = "ImsRealm";

    /**
     * P-CSCF or outbound proxy addresses for mobile access
     */
    public static final String IMS_PROXY_ADDR_MOBILE = "ImsOutboundProxyAddrForMobile";



    /**
     * P-CSCF or outbound proxy ports for mobile access
     */
    public static final String IMS_PROXY_PORT_MOBILE = "ImsOutboundProxyPortForMobile";

    /**
     * P-CSCF or outbound proxy address for Wi-Fi access
     */
    public static final String IMS_PROXY_ADDR_WIFI = "ImsOutboundProxyAddrForWifi";

    /**
     * P-CSCF or outbound proxy port for Wi-Fi access
     */
    public static final String IMS_PROXY_PORT_WIFI = "ImsOutboundProxyPortForWifi";


/*
     *  number Of P-CSCF address
     */

    public static final String NUMBER_OF_PCSCF_ADDRESS= "NumberOfPCscfAddress";
    public static final String CURRENT_PCSCF_ADDRESS_COUNTER= "CurrentPCscfCounter";

    /**
     * XDM server address & port
     */
    public static final String XDM_SERVER = "XdmServerAddr";

    /**
     * XDM server login (for HTTP Digest only)
     */
    public static final String XDM_LOGIN= "XdmServerLogin";

    /**
     * XDM server password (for HTTP Digest only)
     */
    public static final String XDM_PASSWORD = "XdmServerPassword";

    /**
     * File transfer HTTP server address & port
     */
    public static final String FT_HTTP_SERVER = "FtHttpServerAddr";

    /**
     * File transfer HTTP server login
     */
    public static final String FT_HTTP_LOGIN= "FtHttpServerLogin";

    /**
     * File transfer HTTP server password
     */
    public static final String FT_HTTP_PASSWORD = "FtHttpServerPassword";

    /**
     * File transfer default protocol
     */
    public static final String FT_PROTOCOL = "FtProtocol";

    /**
     * IM conference URI for group chat session
     */
    public static final String IM_CONF_URI = "ImConferenceUri";

    /**
     * IM Multi conference URI for group chat session
     */
    public static final String IM_MULTI_CONF_URI = "ImMultiConferenceUri";

    /**
     * End user confirmation request URI for terms and conditions
     */
    public static final String ENDUSER_CONFIRMATION_URI = "EndUserConfReqUri";

    /**
     * Country code
     */
    public static final String COUNTRY_CODE = "CountryCode";

    /**
     * Country area code
     */
    public static final String COUNTRY_AREA_CODE = "CountryAreaCode";

    /**
     * Msisdn
     */
    public static final String MSISDN = "MSISDN";

    // ---------------------------------------------------------------------------
    // Stack settings
    // ---------------------------------------------------------------------------

    /**
     * Polling period used before each IMS service check (e.g. test subscription state for presence service)
     */
    public static final String IMS_SERVICE_POLLING_PERIOD = "ImsServicePollingPeriod";

    /**
     * Default SIP port
     */
    public static final String SIP_DEFAULT_PORT = "SipListeningPort";

    /**
     * Default SIP protocol for mobile
     */
    public static final String SIP_DEFAULT_PROTOCOL_FOR_MOBILE = "SipDefaultProtocolForMobile";

/**M : added for new params
    **/
    /**
     * Secure MSRP over Wi-Fi
     */
    public static final String SECURE_MSRP_OVER_MOBILE = "SecureMsrpOverMobile";


    /**
     * Secured RTP over Wi-Fi
     */
    public static final String SECURE_RTP_OVER_MOBILE = "SecureRtpOverMobile";

/**@*/

    /**
     * Default SIP protocol for Wi-Fi
     */
    public static final String SIP_DEFAULT_PROTOCOL_FOR_WIFI = "SipDefaultProtocolForWifi";

    /**
     * M: add for MSRPoTLS @{
     */
    /**
     * MSRP protocol
     */
    public static final String MSRP_PROTOCOL_FOR_MOBILE = "MsrpProtocolForMobile";

    /**
     * MSRP protocol
     */
    public static final String MSRP_PROTOCOL_FOR_WIFI = "MsrpProtocolForWifi";
    public static final String PROVISION_VALIDITY = "ProvisionValidity";
    public static final String PROVISION_TIME = "ProvisionTime";
    public static final String PROVISION_VERSION = "ProvisioningVersion";
    public static final String SYSTEM_FINGERPRINT = "SystemFingerprint";
    /**
     * @}
     */


    /**
     * Roaming authorization parameter which indicates if the RCS service may be used or not in roaming
     */
    public static final String ROAMING_AUTHORIZED = "RoamingAuthorized";

    /**
     * Roaming authorization parameter which indicates if the RCS only roaming service may be used or not in roaming
     */
    public static final String RCS_ONLY_ROAMING_AUTHORIZED = "RcsOnlyRoamingAuthorized";

    /**
     * Video format for video share
     */
    public static final String CSH_VIDEO_FORMAT = "CShVideoFormat";


     /**
     * Video size for video share
     */
    public static final String CSH_VIDEO_SIZE = "CShVideoSize";

    /**
     * TLS Certifcate root
     */
    public static final String TLS_CERTIFICATE_ROOT = "TlsCertificateRoot";

    /**
     * TLS Certifcate intermediate
     */
    public static final String TLS_CERTIFICATE_INTERMEDIATE = "TlsCertificateIntermediate";

    /**
     * SIP transaction timeout used to wait a SIP response
     */
    public static final String SIP_TRANSACTION_TIMEOUT = "SipTransactionTimeout";

    /**
     * Default TCP port for MSRP session
     */
    public static final String MSRP_DEFAULT_PORT = "DefaultMsrpPort";

    /**
     * Default UDP port for RTP session
     */
    public static final String RTP_DEFAULT_PORT = "DefaultRtpPort";

    /**
     * MSRP transaction timeout used to wait MSRP response
     */
    public static final String MSRP_TRANSACTION_TIMEOUT = "MsrpTransactionTimeout";

    /**
     * Registration expire period
     */
    public static final String REGISTER_EXPIRE_PERIOD = "RegisterExpirePeriod";

    /**
     * Registration retry base time
     */
    public static final String REGISTER_RETRY_BASE_TIME = "RegisterRetryBaseTime";

    /**
     * Registration retry max time
     */
    public static final String REGISTER_RETRY_MAX_TIME = "RegisterRetryMaxTime";

    /**
     * Publish expire period
     */
    public static final String PUBLISH_EXPIRE_PERIOD = "PublishExpirePeriod";

    /**
     * Revoke timeout
     */
    public static final String REVOKE_TIMEOUT = "RevokeTimeout";

    /**
     * IMS authentication procedure for mobile access
     */
    public static final String IMS_AUTHENT_PROCEDURE_MOBILE = "ImsAuhtenticationProcedureForMobile";

    /**
     * IMS authentication procedure for Wi-Fi access
     */
    public static final String IMS_AUTHENT_PROCEDURE_WIFI = "ImsAuhtenticationProcedureForWifi";

    /**
     * Activate or not Tel-URI format
     */
    public static final String TEL_URI_FORMAT = "TelUriFormat";

    /**
     * Ringing session period. At the end of the period the session is cancelled
     */
    public static final String RINGING_SESSION_PERIOD = "RingingPeriod";

    /**
     * Subscribe expiration timeout
     */
    public static final String SUBSCRIBE_EXPIRE_PERIOD = "SubscribeExpirePeriod";

    /**
     * "Is-composing" timeout for chat service
     */
    public static final String IS_COMPOSING_TIMEOUT = "IsComposingTimeout";

    /**
     * SIP session refresh expire period
     */
    public static final String SESSION_REFRESH_EXPIRE_PERIOD = "SessionRefreshExpirePeriod";

    /**
     * Activate or not permanent state mode
     */
    public static final String PERMANENT_STATE_MODE = "PermanentState";

    /**
     * Activate or not the traces
     */
    public static final String TRACE_ACTIVATED = "TraceActivated";

    /**
     * Logger trace level
     */
    public static final String TRACE_LEVEL = "TraceLevel";

    /**
     * Activate or not the SIP trace
     */
    public static final String SIP_TRACE_ACTIVATED = "SipTraceActivated";

    /**
     * SIP trace file
     */
    public static final String SIP_TRACE_FILE = "SipTraceFile";

    /**
     * Activate or not the media trace
     */
    public static final String MEDIA_TRACE_ACTIVATED = "MediaTraceActivated";

    /**
     * Capability refresh timeout used to avoid too many requests in a short time
     */
    public static final String CAPABILITY_REFRESH_TIMEOUT = "CapabilityRefreshTimeout";

    /**
     * Capability refresh timeout used to decide when to refresh contact capabilities
     */
    public static final String CAPABILITY_EXPIRY_TIMEOUT = "CapabilityExpiryTimeout";

    /**
     * Polling period used to decide when to refresh contacts capabilities
     */
    public static final String CAPABILITY_POLLING_PERIOD = "CapabilityPollingPeriod";

    /**
     * CS video capability
     */
    public static final String CAPABILITY_CS_VIDEO = "CapabilityCsVideo";

    /**
     * Image sharing capability
     */
    public static final String CAPABILITY_IMAGE_SHARING = "CapabilityImageShare";

    /**
     * Video sharing capability
     */
    public static final String CAPABILITY_VIDEO_SHARING = "CapabilityVideoShare";

    /**
     * IP voice call capability
     */
    public static final String CAPABILITY_IP_VOICE_CALL = "CapabilityIPVoiceCall";

    /**
     * IP video call capability
     */
    public static final String CAPABILITY_IP_VIDEO_CALL = "CapabilityIPVideoCall";

    /**
     * Instant Messaging session capability
     */
    public static final String CAPABILITY_IM_SESSION = "CapabilityImSession";

    /**
     * Group Instant Messaging session capability
     */
    public static final String CAPABILITY_IM_GROUP_SESSION = "CapabilityImGroupSession";

    /**
     * File transfer capability
     */
    public static final String CAPABILITY_FILE_TRANSFER = "CapabilityFileTransfer";

    /**
     * File transfer via HTTP capability
     */
    public static final String CAPABILITY_FILE_TRANSFER_HTTP = "CapabilityFileTransferHttp";

    /**
     * Cloud File transfer capability
     */
    public static final String CAPABILITY_CLOUD_FILE_TRANSFER = "CapabilityCloudFileTransfer";

    /**
     * Cloud File transfer capability
     */
    public static final String CAPABILITY_CARD_MESSAGE = "CapabilityCardMessage";

    /**
     * Presence discovery capability
     */
    public static final String CAPABILITY_PRESENCE_DISCOVERY = "CapabilityPresenceDiscovery";

    /**
     * Social presence capability
     */
    public static final String CAPABILITY_SOCIAL_PRESENCE = "CapabilitySocialPresence";

    /**
     * Geolocation push capability
     */
    public static final String CAPABILITY_GEOLOCATION_PUSH = "CapabilityGeoLocationPush";

    /**
     * File transfer thumbnail capability
     */
    public static final String CAPABILITY_FILE_TRANSFER_THUMBNAIL = "CapabilityFileTransferThumbnail";

    /**
     * File transfer Store & Forward
     */
    public static final String CAPABILITY_FILE_TRANSFER_SF = "CapabilityFileTransferSF";

    /**
     * Group chat Store & Forward
     */
    public static final String CAPABILITY_GROUP_CHAT_SF = "CapabilityGroupChatSF";

    /**
     * RCS extensions capability
     */
    public static final String CAPABILITY_RCS_EXTENSIONS = "CapabilityRcsExtensions";

    /**
     * Instant messaging is always on (Store & Forward server)
     */
    public static final String IM_CAPABILITY_ALWAYS_ON = "ImAlwaysOn";

    /**
     * SIP Automata capability (@see RFC3840)
     */
    public static final String CAPABILITY_SIP_AUTOMATA = "CapabilitySipAutomata";

    /**
     * File transfer always on (Store & Forward server)
     */
    public static final String FT_CAPABILITY_ALWAYS_ON = "FtAlwaysOn";

    /**
     * Instant messaging use report
     */
    public static final String IM_USE_REPORTS = "ImUseReports";

    /**
     * Network access authorized
     */
    public static final String NETWORK_ACCESS = "NetworkAccess";

    /**
     * Registration status
     */
    public static final String REGISTRATION_STATUS = "RegistrationStatus";

    public static final String LAUNCH_STATUS = "LaunchStatus";

    /**
     * SIP stack timer T1
     */
    public static final String SIP_TIMER_T1 = "SipTimerT1";

    /**
     * SIP stack timer T2
     */
    public static final String SIP_TIMER_T2 = "SipTimerT2";

    /**
     * SIP stack timer T4
     */
    public static final String SIP_TIMER_T4 = "SipTimerT4";

    /**
     * Enable SIP keep alive
     */
    public static final String SIP_KEEP_ALIVE = "SipKeepAlive";

    /**
     * SIP keep alive period
     */
    public static final String SIP_KEEP_ALIVE_PERIOD = "SipKeepAlivePeriod";

    /**
     * RCS APN
     */
    public static final String RCS_APN = "RcsApn";



     /**
     * M: Add to achieve the RCS-e only APN feature. @{
     */
    /**
     * RCS APN enable switch
     */
    public static final String RCS_APN_SWITCH = "RcsApnSwitch";
    /**
     * @}
     */

    /**
     * RCS operator
     */
    public static final String RCS_OPERATOR = "RcsOperator";

    /**
     * GRUU support
     */
    public static final String GRUU = "GRUU";

    /**
     * IMEI used as device ID
     */
    public static final String USE_IMEI_AS_DEVICE_ID = "ImeiDeviceId";

    /**
     * CPU always_on support
     */
    public static final String CPU_ALWAYS_ON = "CpuAlwaysOn";

    /**
     * Auto configuration mode
     */
    public static final String AUTO_CONFIG_MODE = "Autoconfig";

    /**
     * Provisioning terms accepted
     */
    public static final String PROVISIONING_TERMS_ACCEPTED = "ProvisioningTermsAccepted";

    /**
     * M: Add to achieve the RCS-e set chat wall paper feature. @{
     */
    /**
     * RCS-e chat wall paper
     */
    public static final String RCSE_CHAT_WALLPAPER = "RcseChatWallpaper";
    /**
     * @}
     */

    /**
     * M: Add to achieve the RCS-e set compressing image feature. @{
     */
    /**
     * RCS-e compressing image
     */
    public static final String RCSE_COMPRESSING_IMAGE = "RcseCompressingImage";
    public static final String COMPRESS_IMAGE_HINT = "compress_image_hint";
    /**
     * @}
     */
    /**
     * M: Add to achieve the RCS-e Warning on Large image feature. @{
     */
    /**
     * RCS-e Large Size image
     */
    public static final String WARNING_LARGE_IMAGE_HINT = "Warning_large_image_hint";
    /**
     * @}
     */
    /** M: ftAutAccept @{ */
    /**
     * enable or disable auto-accept ft when roaming.
     */
    public static final String ENABLE_AUTO_ACCEPT_FT_ROMING = "ftAutAcceptWhenRoaming";

    /**
     * enable or disable auto-accept ft when no roaming.
     */
    public static final String ENABLE_AUTO_ACCEPT_FT_NOROMING = "ftAutAcceptWhenNoRoaming";

    /** @}*/
    /**
     * Provisioning version
     */
    public static final String PROVISIONING_VERSION = "ProvisioningVersion";

    /**
     * M: add T-Mobile supporting capability
     *
     * @{T-Mobile
     */

    /**
     * SMS Over IP capability
     */
    public static final String CAPABILITY_SMSOverIP = "CapabilitySMSOverIP";

    /**
     * ICSI Emergency capability
     */
    public static final String CAPABILITY_ICSI_EMERGENCY = "CapabilityICSIEmergency";

    /**
     * ICSI MMTEL capability
     */
    public static final String CAPABILITY_ICSI_MMTEL = "CapabilityICSIMMTel";


    /** T-Mobile@} */


    /**
     * Provisioning version
     */
    public static final String PROVISIONING_TOKEN = "ProvisioningToken";

    /**
     * Provisioning Token Validity
     */
    public static final String PROVISIONING_TOKEN_VALIDITY = "ProvisioningTokenValidity";

    /**
     * Secondary provisioning address
     */
    public static final String SECONDARY_PROVISIONING_ADDRESS = "SecondaryProvisioningAddress";

    /**
     * Use only the secondary provisioning address
     */
    public static final String SECONDARY_PROVISIONING_ADDRESS_ONLY = "SecondaryProvisioningAddressOnly";

    /**
     * Directory path for photos
     */
    public static final String DIRECTORY_PATH_PHOTOS = "DirectoryPathPhotos";

    /**
     * Directory path for videos
     */
    public static final String DIRECTORY_PATH_VIDEOS = "DirectoryPathVideos";

    /**
     * Directory path for files
     */
    public static final String DIRECTORY_PATH_FILES = "DirectoryPathFiles";

    /**
     * Disable XCAP operation flag
     */
    public static final String BLOCK_XCAP_OPERATION = "BlockXCAPOperation";

    /**
     * Secure MSRP over Wi-Fi
     */
    public static final String SECURE_MSRP_OVER_WIFI = "SecureMsrpOverWifi";

    public static final String GZIP_SUPPORT = "GzipSupport";

    public static final String IS_COMPOSING = "IsComposing";

    public static final String IS_LARGE_MODE = "IsLargeMode";

    /**
     * Secured RTP over Wi-Fi
     */
    public static final String SECURE_RTP_OVER_WIFI = "SecureRtpOverWifi";

    /**BB Drop 1 */

    public static final String MESSAGING_UX = "MessagingUx";


    public static final String MESSAGING_CAPABILITIES_VALIDITY = "msgCapValidity";


    public static final String DELIVERY_TIMEOUT = "DeliveryTimeout";


     public static final String FT_HTTP_CAP_ALWAYS_ON = "FtHttpCapAlwaysOn";


    public static final String JOYN_MESSAGING_DISABLED_FULLY_INTEGRATED = "JoynMessagingDisabledFullyIntegrated";


    public static final String JOYN_USER_ALIAS = "JoynUserAlias";

     /**
     * Key and associated values for GSMA release of the device as provisioned by the network
     */
    public static final String KEY_GSMA_RELEASE = "GsmaRelease";
    public static final String VALUE_GSMA_REL_ALBATROS = "0";
    public static final String VALUE_GSMA_REL_BLACKBIRD = "1";
    public static final String VALUE_GSMA_REL_CRANE = "2";

     /**
     * Authorization for user to use best effort IP Voice Call service /depending on network
     */
    public static final String BE_IPVOICECALL_AUTH = "beIPVoiceCallAuth";

    /**
     * Authorization for user to use best effort IP Video Call service /depending on network
     */
    public static final String BE_IPVIDEOCALL_AUTH = "beIPVideoCallAuth";

      /**
     *  Authorization for device in RCS-AA mode to initiate IP Voice Call even if remote doesn't show its voice service capabilities
     */
    public static final String IPVOICECALL_BREAKOUT ="beIPCallBreakOut";

    /**
     *  Authorization for device in RCS-CS mode to initiate IP Voice Call even if remote doesn't show its voice service capabilities
     */
    public static final String IPVOICECALL_BREAKOUT_CS ="beIPCallBreakOutCS";

    /**
     *  Authorization for device in RCS-CS mode to upgrade to video a CS call
     */
    public static final String IPVIDEOCALL_UPGRADE_FROM_CS="beIPVideoCallUpgradeFromCS";

    /**
     *  Authorization for device in RCS-AA or RCS-CS mode to upgrade to video even if no answer to capability request (fails with 480 or 408 error)
     */
    public static final String IPVIDEOCALL_UPGRADE_ON_CAPERROR = "beIPVideoCallUpgradeOnCapError";

    /**
     *  Authorization for device in RCS-CS mode to upgrade to video without first tearing down CS call
     */
    public static final String IPVIDEOCALL_UPGRADE_ATTEMPT_EARLY = "beIPVideoCallUpgradeAttemptEarly";

    /**
     *  label to be used when presenting to user the option for initiating an end-to-end ip call
     */
    public static final String IPCALL_E2E_LABEL = "e2eIPCallLabel";

    /**
     *  label to be used when presenting to user the option for initiating a breakout ip call
     */
    public static final String IPCALL_BREAKOUT_LABEL = "breakoutIPCallLabel";

     /**
     *  parameter to control whether "end-to-end ip call" button is presented to user (case false) or breakout button (case true)
     */
    public static final String IPCALL_E2E_VOICECAPABILITYHANDLING = "e2eVoiceCapabilityHandling";

    public static final String CPM_TECHNOLOGY = "messagingTechnology";

    public static final String CPM_FT_MSG = "cpmFt";

    public static final String CPM_PAGER_MODE = "cpmPagerMode";

    public static final String CPM_LARGE_MSG = "cpmLargeMsg";


    public static final String MULTIMEDIA_IN_CHAT = "multiMediaChat";


    public static final String FIRST_MSG_IN_INVITE = "firstMessageInvite";


   /**
   * Auto accept file transfer invitation on roaming
   */
    public static final String AUTO_ACCEPT_FILE_TRANSFER_ON_ROAMING = "AutoAcceptFileTransferOnRoaming";

    public static final String MAX_STANDALONE_SIZE = "MaxSizeStandalone";

    public static final String RCS_OPERATOR_ADDRESS = "NetworkOperatorAddress";

    public static final String MSGSTORE_ADDRESS = "msgStoreAddress";

    public static final String MSGSTORE_USER = "msgStoreUser";

    public static final String PUBLIC_URI = "publicUri";


    public static final String CONFIGURATION_STATE = "configurationState";


    public static final String IM_MASS_URI = "messageFactoryUri";

    public static final String PUBLIC_USER_IDENTITY_1 = "publicUserIdentity1";

    public static final String PUBLIC_USER_IDENTITY_2 = "publicUserIdentity2";

    public static final String PUBLIC_USER_IDENTITY_3 = "publicUserIdentity3";

    public static final String PUBLIC_USER_IDENTITY_PC = "publicUserIdentityPC";

    public static final String SERVICE_PERMITTED = "servicePermitted";

    public static final String MAX_PAGER_CONTENT_SIZE = "maxPagerContentSize";

    public static final String SPAM_ADDRESS = "SpamAddress";


    public static final String XDM_AUTH_TYPE = "xdmAuthType";


    public static final String IR94_VIDEO_AUTH = "ir94VideoSupported";

    public static final String STANDALONE_MSG_SUPPORT = "standaloneMsgAuth";

    public static final String RCS_VOLTE_SINGLE_REGISTRATION = "rcsVolteSingleRegistration";

    /**
     * Subscribe expiration timeout
     */
    public static final String MAX_SUBSCRIPTION_PRESENCE_LIST = "MaxSubscriptionPresenceList";

    /**
    * The device Shall follow a dual registration approach (transition solution) where
    * RCS services use a separate registration from the VoLTE/ViLTE one. In this case,
    * the RCS stack Shall work in RCS-CS mode as described in section 2.2.1.
    *
    */
    public static final int RCS_VOLTE_DUAL_REGISTRATION_MODE = 0;

    /**
     * The device Shall follow a single registration (target solution) for both RCS and
     * VoLTE/ViLTE services. Note in this configuration, the RCS stack works in RCSVoLTE
     * mode as described in section 2.2.1. Note this is the default value in case
     * the parameter is not present.
     */
    public static final int RCS_VOLTE_SINGLE_REGISTRATION_MODE = 1;

    /**
     * the device Shall follow a single registration for both RCS and VoLTE/ViLTE
     * services when in the home network, and Shall follow a dual registration when
     * roaming (transition solution).
     */
    public static final int RCS_VOLTE_DUAL_REGISTRATION_ROAMING_MODE = 2;


    public static final String CLIENT_OBJ_DATA_LIMIT = "clientObjDataLimit";


    public static final String CONTENT_SERVER_URI = "ClientServerURI";


    public static final String SOURCE_THROTTLE_PUBLISH = "SourceThrottlePublish";


    public static final String MAX_SUBSCRIPTION_PRESENCE = "MaxSubPrsence";


    public static final String SERVICE_URI_TEMPLATE = "ServiceUriTemplate";


    public static final String RLS_URI = "RlsUri";


    public static final String NON_VIP_POLL_PERIOD = "NonVipPollPeriodSetting";


    public static final String NON_VIP_MAX_POLL_PER_PERIOD = "NonVipMaxPollPerPeriod";


	public static final String CLIENT_VENDOR = "ClientVendor";


	public static final String USE_CLIENT_VENDER_SETTINGS = "UseClientVendorFromSettings";



	public static final String SERVICE_CURRENT_STATE = "CurrentServiceState";


    public static final String FORCE_NAT_TRAVERSAL = "forceNatTraversal";


    public static final String ENABLE_RCS_SWITCH = "enableRcseSwitch";
    /* package private */static final Integer DEFAULT_ENABLE_RCS_SWITCH = EnableRcseSwitch.ALWAYS_SHOW
            .toInt();
    /**
     * Max length for extensions using real time messaging (MSRP)
     */
    public static final String MAX_MSRP_SIZE_EXTENSIONS = "ExtensionsMaxMsrpSize";
    static final Integer DEFAULT_MAX_MSRP_SIZE_EXTENSIONS = 0;

    /**
     * Call composer inactivity timeout before session is closed
     */
    public static final String CALL_COMPOSER_INACTIVITY_TIMEOUT = "CallComposerInactivityTimeout";
    /* package private */static final Long DEFAULT_CALL_COMPOSER_INACTIVITY_TIMEOUT = 180000L;
    public static final String MAX_ALLOWED_DISPLAY_NAME_CHARS = "MaxAllowedDisplayNameChars";
    /* package private */static final Integer DEFAULT_MAX_ALLOWED_DISPLAY_NAME_CHARS = 256;

    /**
     * Validity of the RCS configuration.
     */
    public static final String CONFIGURATION_VALID = "ConfigurationValidity";
    /* package private */static final Boolean DEFAULT_CONFIGURATION_VALID = false;

    /**
     * Convergent messaging UX option
     */
    public static final String MESSAGING_MODE = "MessagingMode";
    /* package private */static final Integer DEFAULT_KEY_MESSAGING_MODE = MessagingMode.NONE
            .toInt();

     /**
     * Default messaging method
     */
    public static final String DEFAULT_MESSAGING_METHOD = "DefaultMessagingMethod";
    /* package private */static final Integer DEFAULT_KEY_DEFAULT_MESSAGING_METHOD = MessagingMethod.AUTOMATIC
            .toInt();


    public static final String MAX_AUDIO_MESSAGE_DURATION = "MaxAudioMessageDuration";
    /* package private */static final Long DEFAULT_MAX_AUDIO_DURATION = 600000L;


    public static final String AUTO_ACCEPT_FT_IN_ROAMING = "AutoAcceptFtInRoaming";
    /* package private */static final Boolean DEFAULT_AUTO_ACCEPT_FT_IN_ROAMING = false;

    /**
     * Auto accept file transfer enabled
     */
    public static final String AUTO_ACCEPT_FT_CHANGEABLE = "AutoAcceptFtChangeable";
    /* package private */static final Boolean DEFAULT_AUTO_ACCEPT_FT_CHANGEABLE = true;


    public static final String CHAT_RESPOND_TO_DISPLAY_REPORTS = "ChatRespondToDisplayReports";

     /**
     * Group Messaging toggle
     */
    public static final String GROUP_MESSAGING_TOGGLE = "GroupMessagingToggle";

     /**
     * EnableRcseSwitch describes whether or not to show the RCS enabled/disabled switch permanently
     */
    public enum EnableRcseSwitch {
        /**
         * the switch is shown permanently
         */
        ALWAYS_SHOW(1),
        /**
         * the switch is only shown during roaming
         */
        ONLY_SHOW_IN_ROAMING(0),
        /**
         * the switch is never shown
         */
        NEVER_SHOW(-1);

        private int mValue;

        private static SparseArray<EnableRcseSwitch> mValueToEnum = new SparseArray<>();

        static {
            for (EnableRcseSwitch entry : EnableRcseSwitch.values()) {
                mValueToEnum.put(entry.toInt(), entry);
            }
        }

        EnableRcseSwitch(int value) {
            mValue = value;
        }

        /**
         * @return value
         */
        public final int toInt() {
            return mValue;
        }

        /**
         * @param value the value representing the enable RCSe switch
         * @return EnableRcseSwitch
         */
        public static EnableRcseSwitch valueOf(int value) {
            EnableRcseSwitch entry = mValueToEnum.get(value);
            if (entry != null) {
                return entry;
            }
            throw new IllegalArgumentException("No enum const class "
                    + EnableRcseSwitch.class.getName() + "." + value);
        }

    }

    /**
     * The File Transfer protocol enumerated type.
     */
    public enum FileTransferProtocol {
        /**
         * MSRP protocol
         */
        MSRP,
        /**
         * HTTP protocol
         */
        HTTP
    }

}
