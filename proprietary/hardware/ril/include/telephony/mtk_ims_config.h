//
// Created by mtk54098 on 5/18/18.
//

#ifndef RIL_IMS_CONFIG_ITEMS_H
#define RIL_IMS_CONFIG_ITEMS_H

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
/**
 * Define operator provisioned config items
 */
            IMS_CONFIG_PROVISIONED_START = 0,

/**
 * AMR CODEC Mode Value set, 0-7 in comma separated sequence.
 * Value is in String format.
 */
            IMS_CONFIG_VOCODER_AMRMODESET = IMS_CONFIG_PROVISIONED_START,

/**
 * Wide Band AMR CODEC Mode Value set,0-7 in comma separated sequence.
 * Value is in String format.
 */
            IMS_CONFIG_VOCODER_AMRWBMODESET = 1,

/**
 * SIP Session Timer value (seconds).
 * Value is in Integer format.
 */
            IMS_CONFIG_SIP_SESSION_TIMER = 2,

/**
 * Minimum SIP Session Expiration Timer in (seconds).
 * Value is in Integer format.
 */
            IMS_CONFIG_MIN_SE = 3,

/**
 * SIP_INVITE cancellation time out value (in milliseconds). Integer format.
 * Value is in Integer format.
 */
            IMS_CONFIG_CANCELLATION_TIMER = 4,

/**
 * Delay time when an iRAT transition from eHRPD/HRPD/1xRTT to LTE.
 * Value is in Integer format.
 */
            IMS_CONFIG_TDELAY = 5,

/**
 * Silent redial status of Enabled (True), or Disabled (False).
 * Value is in Integer format.
 */
            IMS_CONFIG_SILENT_REDIAL_ENABLE = 6,

/**
 * SIP T1 timer value in milliseconds. See RFC 3261 for define.
 * Value is in Integer format.
 */
            IMS_CONFIG_SIP_T1_TIMER = 7,

/**
 * SIP T2 timer value in milliseconds.  See RFC 3261 for define.
 * Value is in Integer format.
 */
            IMS_CONFIG_SIP_T2_TIMER = 8,

/**
* SIP TF timer value in milliseconds.  See RFC 3261 for define.
* Value is in Integer format.
*/
            IMS_CONFIG_SIP_TF_TIMER = 9,

/**
 * VoLTE status for VLT/s status of Enabled (1), or Disabled (0).
 * Value is in Integer format.
 */
            IMS_CONFIG_VLT_SETTING_ENABLED = 10,

/**
 * VoLTE status for LVC/s status of Enabled (1), or Disabled (0).
 * Value is in Integer format.
 */
            IMS_CONFIG_LVC_SETTING_ENABLED = 11,
/**
 * Domain Name for the device to populate the request URI for REGISTRATION.
 * Value is in String format.
 */
            IMS_CONFIG_DOMAIN_NAME = 12,
/**
* Device Outgoing SMS based on either 3GPP or 3GPP2 standards.
* Value is in Integer format. 3GPP2(0), 3GPP(1)
*/
            IMS_CONFIG_SMS_FORMAT = 13,
/**
* Turns IMS ON/OFF on the device.
* Value is in Integer format. ON (1), OFF(0).
*/
            IMS_CONFIG_SMS_OVER_IP = 14,
/**
 * Requested expiration for Published Online availability.
 * Value is in Integer format.
 */
            IMS_CONFIG_PUBLISH_TIMER = 15,
/**
 * Requested expiration for Published Offline availability.
 * Value is in Integer format.
 */
            IMS_CONFIG_PUBLISH_TIMER_EXTENDED = 16,
/**
 *
 * Value is in Integer format.
 */
            IMS_CONFIG_CAPABILITY_DISCOVERY_ENABLED = 17,
/**
 * Period of time the capability information of the  contact is cached on handset.
 * Value is in Integer format.
 */
            IMS_CONFIG_CAPABILITIES_CACHE_EXPIRATION = 18,
/**
 * Peiod of time the availability information of a contact is cached on device.
 * Value is in Integer format.
 */
            IMS_CONFIG_AVAILABILITY_CACHE_EXPIRATION = 19,
/**
 * Interval between successive capabilities polling.
 * Value is in Integer format.
 */
            IMS_CONFIG_CAPABILITIES_POLL_INTERVAL = 20,
/**
 * Minimum time between two published messages from the device.
 * Value is in Integer format.
 */
            IMS_CONFIG_SOURCE_THROTTLE_PUBLISH = 21,
/**
 * The Maximum number of MDNs contained in one Request Contained List.
 * Value is in Integer format.
 */
            IMS_CONFIG_MAX_NUMENTRIES_IN_RCL = 22,
/**
 * Expiration timer for subscription of a Request Contained List, used in capability
 * polling.
 * Value is in Integer format.
 */
            IMS_CONFIG_CAPAB_POLL_LIST_SUB_EXP = 23,
/**
 * Applies compression to LIST Subscription.
 * Value is in Integer format. Enable (1), Disable(0).
 */
            IMS_CONFIG_GZIP_FLAG = 24,
/**
 * VOLTE Status for EAB/s status of Enabled (1), or Disabled (0).
 * Value is in Integer format.
 */
            IMS_CONFIG_EAB_SETTING_ENABLED = 25,
/**
 * Wi-Fi calling roaming status.
 * Value is in Integer format. ON (1), OFF(0).
 */
            IMS_CONFIG_VOICE_OVER_WIFI_ROAMING = 26,
/**
 * Wi-Fi calling modem - WfcModeFeatureValueConstants.
 * Value is in Integer format.
 */
            IMS_CONFIG_VOICE_OVER_WIFI_MODE = 27,
/**
 * VOLTE Status for voice over wifi status of Enabled (1), or Disabled (0).
 * Value is in Integer format.
 */
            IMS_CONFIG_VOICE_OVER_WIFI_SETTING_ENABLED = 28,
/**
 * Mobile data enabled.
 * Value is in Integer format. On (1), OFF(0).
 */
            IMS_CONFIG_MOBILE_DATA_ENABLED = 29,
/**
 * VoLTE user opted in status.
 * Value is in Integer format. Opted-in (1) Opted-out (0).
 */
            IMS_CONFIG_VOLTE_USER_OPT_IN_STATUS = 30,
/**
 * Proxy for Call Session Control Function(P-CSCF) address for Local-BreakOut(LBO).
 * Value is in String format.
 */
            IMS_CONFIG_LBO_PCSCF_ADDRESS = 31,
/**
 * Keep Alive Enabled for SIP.
 * Value is in Integer format. On(1), OFF(0).
 */
            IMS_CONFIG_KEEP_ALIVE_ENABLED = 32,
/**
 * Registration retry Base Time value in seconds.
 * Value is in Integer format.
 */
            IMS_CONFIG_REGISTRATION_RETRY_BASE_TIME_SEC = 33,
/**
 * Registration retry Max Time value in seconds.
 * Value is in Integer format.
 */
            IMS_CONFIG_REGISTRATION_RETRY_MAX_TIME_SEC = 34,
/**
 * Smallest RTP port for speech codec.
 * Value is in integer format.
 */
            IMS_CONFIG_SPEECH_START_PORT = 35,
/**
 * Largest RTP port for speech code.
 * Value is in Integer format.
 */
            IMS_CONFIG_SPEECH_END_PORT = 36,
/**
 * SIP Timer A's value in milliseconds. Timer A is the INVITE request
 * retransmit interval, for UDP only.
 * Value is in Integer format.
 */
            IMS_CONFIG_SIP_INVITE_REQ_RETX_INTERVAL_MSEC = 37,
/**
 * SIP Timer B's value in milliseconds. Timer B is the wait time for
 * INVITE message to be acknowledged.
 * Value is in Integer format.
 */
            IMS_CONFIG_SIP_INVITE_RSP_WAIT_TIME_MSEC = 38,
/**
 * SIP Timer D's value in milliseconds. Timer D is the wait time for
 * response retransmits of the invite client transactions.
 * Value is in Integer format.
 */
            IMS_CONFIG_SIP_INVITE_RSP_RETX_WAIT_TIME_MSEC = 39,
/**
 * SIP Timer E's value in milliseconds. Timer E is the value Non-INVITE
 * request retransmit interval, for UDP only.
 * Value is in Integer format.
 */
            IMS_CONFIG_SIP_NON_INVITE_REQ_RETX_INTERVAL_MSEC = 40,
/**
 * SIP Timer F's value in milliseconds. Timer F is the Non-INVITE transaction
 * timeout timer.
 * Value is in Integer format.
 */
            IMS_CONFIG_SIP_NON_INVITE_TXN_TIMEOUT_TIMER_MSEC = 41,
/**
 * SIP Timer G's value in milliseconds. Timer G is the value of INVITE response
 * retransmit interval.
 * Value is in Integer format.
 */
            IMS_CONFIG_SIP_INVITE_RSP_RETX_INTERVAL_MSEC = 42,
/**
 * SIP Timer H's value in milliseconds. Timer H is the value of wait time for
 * ACK receipt.
 * Value is in Integer format.
 */
            IMS_CONFIG_SIP_ACK_RECEIPT_WAIT_TIME_MSEC = 43,
/**
 * SIP Timer I's value in milliseconds. Timer I is the value of wait time for
 * ACK retransmits.
 * Value is in Integer format.
 */
            IMS_CONFIG_SIP_ACK_RETX_WAIT_TIME_MSEC = 44,
/**
 * SIP Timer J's value in milliseconds. Timer J is the value of wait time for
 * non-invite request retransmission.
 * Value is in Integer format.
 */
            IMS_CONFIG_SIP_NON_INVITE_REQ_RETX_WAIT_TIME_MSEC = 45,
/**
 * SIP Timer K's value in milliseconds. Timer K is the value of wait time for
 * non-invite response retransmits.
 * Value is in Integer format.
 */
            IMS_CONFIG_SIP_NON_INVITE_RSP_RETX_WAIT_TIME_MSEC = 46,
/**
 * AMR WB octet aligned dynamic payload type.
 * Value is in Integer format.
 */
            IMS_CONFIG_AMR_WB_OCTET_ALIGNED_PT = 47,
/**
 * AMR WB bandwidth efficient payload type.
 * Value is in Integer format.
 */
            IMS_CONFIG_AMR_WB_BANDWIDTH_EFFICIENT_PT = 48,
/**
 * AMR octet aligned dynamic payload type.
 * Value is in Integer format.
 */
            IMS_CONFIG_AMR_OCTET_ALIGNED_PT = 49,
/**
 * AMR bandwidth efficient payload type.
 * Value is in Integer format.
 */
            IMS_CONFIG_AMR_BANDWIDTH_EFFICIENT_PT = 50,
/**
 * DTMF WB payload type.
 * Value is in Integer format.
 */
            IMS_CONFIG_DTMF_WB_PT = 51,
/**
 * DTMF NB payload type.
 * Value is in Integer format.
 */
            IMS_CONFIG_DTMF_NB_PT = 52,
/**
 * AMR Default encoding mode.
 * Value is in Integer format.
 */
            IMS_CONFIG_AMR_DEFAULT_MODE = 53,
/**
 * SMS Public Service Identity.
 * Value is in String format.
 */
            IMS_CONFIG_SMS_PSI = 54,
/**
 * Video Quality - VideoQualityFeatureValuesConstants.
 * Value is in Integer format.
 */
            IMS_CONFIG_VIDEO_QUALITY = 55,
/**
 * LTE threshold.
 * Handover from LTE to WiFi if LTE < THLTE1 and WiFi > VOWT_A.
 */
            IMS_CONFIG_TH_LTE1 = 56,
/**
 * LTE threshold.
 * Handover from WiFi to LTE if LTE > THLTE3 or (WiFi < VOWT_B and LTE > THLTE2).
 */
            IMS_CONFIG_TH_LTE2 = 57,
/**
 * LTE threshold.
 * Handover from WiFi to LTE if LTE > THLTE3 or (WiFi < VOWT_B and LTE > THLTE2).
 */
            IMS_CONFIG_TH_LTE3 = 58,
/**
 * 1x threshold.
 * Handover from 1x to WiFi if 1x < TH1x
 */
            IMS_CONFIG_TH_1x = 59,
/**
 * WiFi threshold.
 * Handover from LTE to WiFi if LTE < THLTE1 and WiFi > VOWT_A.
 */
            IMS_CONFIG_VOWT_A = 60,
/**
 * WiFi threshold.
 * Handover from WiFi to LTE if LTE > THLTE3 or (WiFi < VOWT_B and LTE > THLTE2).
 */
            IMS_CONFIG_VOWT_B = 61,
/**
 * LTE ePDG timer.
 * Device shall not handover back to LTE until the T_ePDG_LTE timer expires.
 */
            IMS_CONFIG_T_EPDG_LTE = 62,
/**
 * WiFi ePDG timer.
 * Device shall not handover back to WiFi until the T_ePDG_WiFi timer expires.
 */
            IMS_CONFIG_T_EPDG_WIFI = 63,
/**
 * 1x ePDG timer.
 * Device shall not re-register on 1x until the T_ePDG_1x timer expires.
 */
            IMS_CONFIG_T_EPDG_1X = 64,
/**
 * MultiEndpoint status: Enabled (1), or Disabled (0).
 * Value is in Integer format.
 */
            IMS_CONFIG_VICE_SETTING_ENABLED = 65,

/**
 * RTT status: Enabled (1), or Disabled (0).
 * Value is in Integer format.
 */
            IMS_CONFIG_RTT_SETTING_ENABLED = 66,

/**
 * Expand the operator config items as needed here, need to change
 * PROVISIONED_CONFIG_END after that.
 */
            IMS_CONFIG_PROVISIONED_END = IMS_CONFIG_RTT_SETTING_ENABLED,

/*******************************************************************/

// Expand the operator config items as needed here.
/**
 * Define MTK operator provisioned config items
 */
            IMS_CONFIG_MTK_PROVISIONED_START = 1000,

/**
 * FQDN address for WFC ePDG.
 * Value is in String format.
 * Operator: Op06
 * Specification: R19.0.1, VoWiFi Provisioning utilising SMS
 */
            IMS_CONFIG_EPDG_ADDRESS = IMS_CONFIG_MTK_PROVISIONED_START,

/**
 * Publish retry timer when receiving error code =
 * 408, 500, 503, and 603
 * Operator: Op12
 */
            IMS_CONFIG_PUBLISH_ERROR_RETRY_TIMER =
                               IMS_CONFIG_MTK_PROVISIONED_START + 1,

// VzW opt-in MDN requirement
            IMS_CONFIG_VOICE_OVER_WIFI_MDN = IMS_CONFIG_MTK_PROVISIONED_START + 2,

/**
 * Voce domain preference.
 * Value is in Integer format.
 * Operator: Op236
 */
            IMS_CONFIG_VOICE_DOMAIN_PREFERENCE = IMS_CONFIG_MTK_PROVISIONED_START + 3,

/**
 * Expand the MTK operator config items as needed here, need to change
 * PROVISIONED_CONFIG_END after that.
 */
            IMS_CONFIG_MTK_PROVISIONED_END =
                               IMS_CONFIG_VOICE_DOMAIN_PREFERENCE,

/*******************************************************************/
}ImsConfigItem;

typedef enum {
    INTEGER = 0,
    STRING = 1,
    FLOAT = 2,
    JSON = 3,
} ImsCfgMimeType;


typedef enum {
    FEATURE_TYPE_UNKNOWN = -1,
    FEATURE_TYPE_VOICE_OVER_LTE = 0,
    FEATURE_TYPE_VIDEO_OVER_LTE = 1,
    FEATURE_TYPE_VOICE_OVER_WIFI = 2,
    FEATURE_TYPE_VIDEO_OVER_WIFI = 3,
    FEATURE_TYPE_UT_OVER_LTE = 4,
    FEATURE_TYPE_UT_OVER_WIFI =5,
    FEATURE_TYPE_VOICE_OVER_NR = 6,
    FEATURE_TYPE_VIDEO_OVER_NR = 7,
} ImsFeatureType;

typedef enum {
    NETWORK_TYPE_UNKNOWN = 0,
    NETWORK_TYPE_GPRS    = 1,
    NETWORK_TYPE_EDGE    = 2,
    NETWORK_TYPE_UMTS    = 3,
    NETWORK_TYPE_CDMA    = 4,
    NETWORK_TYPE_EVDO_0  = 5,
    NETWORK_TYPE_EVDO_A  = 6,
    NETWORK_TYPE_1xRTT   = 7,
    NETWORK_TYPE_HSDPA   = 8,
    NETWORK_TYPE_HSUPA   = 9,
    NETWORK_TYPE_HSPA    = 10,
    NETWORK_TYPE_IDEN    = 11,
    NETWORK_TYPE_EVDO_B  = 12,
    NETWORK_TYPE_LTE     = 13,
    NETWORK_TYPE_EHRPD   = 14,
    NETWORK_TYPE_HSPAP   = 15,
    NETWORK_TYPE_GSM     = 16,
} NetworkType;

typedef enum {
    FEATURE_VALUE_OFF = 0,
    FEATURE_VALUE_ON = 1,
} ImsFeatureValue;

typedef enum {
    FEATURE_RESOURCE_OFF = 0,
    FEATURE_RESOURCE_ON = 1,
} ImsFeatureResource;

#define IMS_CONFIG_VALUE_NO_DEFAULT "n/a"

// IMS config unit

#define IMS_CONFIG_UNIT_NONE -1
#define IMS_CONFIG_UNIT_NANOSECONDS  0
#define IMS_CONFIG_UNIT_MICROSECONDS 1
#define IMS_CONFIG_UNIT_MILLISECONDS 2
#define IMS_CONFIG_UNIT_SECONDS 3
#define IMS_CONFIG_UNIT_MINUTES 4
#define IMS_CONFIG_UNIT_HOURS 5
#define IMS_CONFIG_UNIT_DAYS 6

#ifdef __cplusplus
}
#endif
#endif //RIL_IMS_CONFIG_ITEMS_H
