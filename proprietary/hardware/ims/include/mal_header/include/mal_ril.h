#ifndef __MAL_RIL2MAL_INTERFACE_H__
#define __MAL_RIL2MAL_INTERFACE_H__
typedef enum {
    MALRIL_PDP_FAIL_NONE = 0, /* No error, connection ok */
    MALRIL_PDP_FAIL_PDN_CONN_DOES_NOT_EXIST = 0x36,
} MALRIL_DataCallFailCause;
typedef enum {
    MALRIL_E_SUCCESS = 0,
    MALRIL_E_RADIO_NOT_AVAILABLE = 1,     /* If radio did not start or is resetting */
    MALRIL_E_GENERIC_FAILURE = 2,
    MALRIL_E_PASSWORD_INCORRECT = 3,      /* for PIN/PIN2 methods only! */
    MALRIL_E_SIM_PIN2 = 4,                /* Operation requires SIM PIN2 to be entered */
    MALRIL_E_SIM_PUK2 = 5,                /* Operation requires SIM PIN2 to be entered */
    MALRIL_E_REQUEST_NOT_SUPPORTED = 6,
    MALRIL_E_CANCELLED = 7,
    MALRIL_E_OP_NOT_ALLOWED_DURING_VOICE_CALL = 8, /* data ops are not allowed during voice
                                                   call on a Class C GPRS device */
    MALRIL_E_OP_NOT_ALLOWED_BEFORE_REG_TO_NW = 9,  /* data ops are not allowed before device
                                                   registers in network */
    MALRIL_E_SMS_SEND_FAIL_RETRY = 10,             /* fail to send sms and need retry */
    MALRIL_E_SIM_ABSENT = 11,                      /* fail to set the location where CDMA subscription
                                                   shall be retrieved because of SIM or RUIM
                                                   card absent */
    MALRIL_E_SUBSCRIPTION_NOT_AVAILABLE = 12,      /* fail to find CDMA subscription from specified
                                                   location */
    MALRIL_E_MODE_NOT_SUPPORTED = 13,              /* HW does not support preferred network type */
    MALRIL_E_FDN_CHECK_FAILURE = 14,               /* command failed because recipient is not on FDN list */
    MALRIL_E_ILLEGAL_SIM_OR_ME = 15,               /* network selection failed due to
                                                   illegal SIM or ME */
    MALRIL_E_MISSING_RESOURCE = 16,                /* no logical channel available */
    MALRIL_E_NO_SUCH_ELEMENT = 17,                  /* application not found on SIM */
    MALRIL_E_DIAL_MODIFIED_TO_USSD = 18,           /* DIAL request modified to USSD */
    MALRIL_E_DIAL_MODIFIED_TO_SS = 19,             /* DIAL request modified to SS */
    MALRIL_E_DIAL_MODIFIED_TO_DIAL = 20,           /* DIAL request modified to DIAL with different
                                                   data */
    MALRIL_E_USSD_MODIFIED_TO_DIAL = 21,           /* USSD request modified to DIAL */
    MALRIL_E_USSD_MODIFIED_TO_SS = 22,             /* USSD request modified to SS */
    MALRIL_E_USSD_MODIFIED_TO_USSD = 23,           /* USSD request modified to different USSD
                                                   request */
    MALRIL_E_SS_MODIFIED_TO_DIAL = 24,             /* SS request modified to DIAL */
    MALRIL_E_SS_MODIFIED_TO_USSD = 25,             /* SS request modified to USSD */
    MALRIL_E_SUBSCRIPTION_NOT_SUPPORTED = 26,      /* Subscription not supported by RIL */
    MALRIL_E_SS_MODIFIED_TO_SS = 27,               /* SS request modified to different SS request */
    MALRIL_E_LCE_NOT_SUPPORTED = 36,               /* LCE service not supported(36 in RILConstants.java) */
    MALRIL_E_NO_MEMORY = 37,                       /* Not sufficient memory to process the request */
    MALRIL_E_INTERNAL_ERR = 38,                    /* Hit unexpected vendor internal error scenario */
    MALRIL_E_SYSTEM_ERR = 39,                      /* Hit platform or system error */
    MALRIL_E_MODEM_ERR = 40,                       /* Hit unexpected modem error */
    MALRIL_E_INVALID_STATE = 41,                   /* Unexpected request for the current state */
    MALRIL_E_NO_RESOURCES = 42,                    /* Not sufficient resource to process the request */
    MALRIL_E_SIM_ERR = 43,                         /* Received error from SIM card */
    MALRIL_E_INVALID_ARGUMENTS = 44,               /* Received invalid arguments in request */
    MALRIL_E_INVALID_SIM_STATE = 45,               /* Can not process the request in current SIM state */
    MALRIL_E_INVALID_MODEM_STATE = 46,             /* Can not process the request in current Modem state */
    MALRIL_E_INVALID_CALL_ID = 47,                 /* Received invalid call id in request */
    MALRIL_E_NO_SMS_TO_ACK = 48,                   /* ACK received when there is no SMS to ack */
    MALRIL_E_NETWORK_ERR = 49,                     /* Received error from network */
    MALRIL_E_REQUEST_RATE_LIMITED = 50,            /* Operation denied due to overly-frequent requests */
    MALRIL_E_SIM_BUSY = 51,                        /* SIM is busy */
    MALRIL_E_SIM_FULL = 52,                        /* The target EF is full */
    MALRIL_E_NETWORK_REJECT = 53,                  /* Request is rejected by network */
    MALRIL_E_OPERATION_NOT_ALLOWED = 54,           /* Not allowed the request now */
    MALRIL_E_EMPTY_RECORD = 55,                    /* The request record is empty */
    MALRIL_E_INVALID_SMS_FORMAT = 56,              /* Invalid sms format */
    MALRIL_E_ENCODING_ERR = 57,                    /* Message not encoded properly */
    MALRIL_E_INVALID_SMSC_ADDRESS = 58,            /* SMSC address specified is invalid */
    MALRIL_E_NO_SUCH_ENTRY = 59,                   /* No such entry present to perform the request */
    MALRIL_E_NETWORK_NOT_READY = 60,               /* Network is not ready to perform the request */
    MALRIL_E_NOT_PROVISIONED = 61,                 /* Device doesnot have this value provisioned */
    MALRIL_E_NO_SUBSCRIPTION = 62,                 /* Device doesnot have subscription */
    MALRIL_E_NO_NETWORK_FOUND = 63,                /* Network cannot be found */
    MALRIL_E_DEVICE_IN_USE = 64,                   /* Operation cannot be performed because the device
                                                   is currently in use */
    MALRIL_E_ABORTED = 65,                         /* Operation aborted */
    // OEM specific error codes. To be used by OEM when they don't want to reveal
    // specific error codes which would be replaced by Generic failure.
    MALRIL_E_OEM_ERROR_1 = 501,
    MALRIL_E_OEM_ERROR_2 = 502,
    MALRIL_E_OEM_ERROR_3 = 503,
    MALRIL_E_OEM_ERROR_4 = 504,
    MALRIL_E_OEM_ERROR_5 = 505,
    MALRIL_E_OEM_ERROR_6 = 506,
    MALRIL_E_OEM_ERROR_7 = 507,
    MALRIL_E_OEM_ERROR_8 = 508,
    MALRIL_E_OEM_ERROR_9 = 509,
    MALRIL_E_OEM_ERROR_10 = 510,
    MALRIL_E_OEM_ERROR_11 = 511,
    MALRIL_E_OEM_ERROR_12 = 512,
    MALRIL_E_OEM_ERROR_13 = 513,
    MALRIL_E_OEM_ERROR_14 = 514,
    MALRIL_E_OEM_ERROR_15 = 515,
    MALRIL_E_OEM_ERROR_16 = 516,
    MALRIL_E_OEM_ERROR_17 = 517,
    MALRIL_E_OEM_ERROR_18 = 518,
    MALRIL_E_OEM_ERROR_19 = 519,
    MALRIL_E_OEM_ERROR_20 = 520,
    MALRIL_E_OEM_ERROR_21 = 521,
    MALRIL_E_OEM_ERROR_22 = 522,
    MALRIL_E_OEM_ERROR_23 = 523,
    MALRIL_E_OEM_ERROR_24 = 524,
    MALRIL_E_OEM_ERROR_25 = 525,
    MALRIL_E_POWER_ON_UIM_NOT_READY = 7000         /* C2K power on request fail uim not ready */
} MALRIL_Errno;
typedef enum {
    MALRIL_RADIO_TECH_UNKNOWN = 0,
    MALRIL_RADIO_TECH_GPRS = 1,
    MALRIL_RADIO_TECH_EDGE = 2,
    MALRIL_RADIO_TECH_UMTS = 3,
    MALRIL_RADIO_TECH_IS95A = 4,
    MALRIL_RADIO_TECH_IS95B = 5,
    MALRIL_RADIO_TECH_1xRTT =  6,
    MALRIL_RADIO_TECH_EVDO_0 = 7,
    MALRIL_RADIO_TECH_EVDO_A = 8,
    MALRIL_RADIO_TECH_HSDPA = 9,
    MALRIL_RADIO_TECH_HSUPA = 10,
    MALRIL_RADIO_TECH_HSPA = 11,
    MALRIL_RADIO_TECH_EVDO_B = 12,
    MALRIL_RADIO_TECH_EHRPD = 13,
    MALRIL_RADIO_TECH_LTE = 14,
    MALRIL_RADIO_TECH_HSPAP = 15, // HSPA+
    MALRIL_RADIO_TECH_GSM = 16, // Only supports voice
    MALRIL_RADIO_TECH_TD_SCDMA = 17,
    MALRIL_RADIO_TECH_IWLAN = 18
} MALRIL_RadioTechnology;
typedef enum {
    MALRIL_PARCEL_FIELD_UNKNOWN = 0,
    MALRIL_PARCEL_FIELD_INTERFACEID = 1,
    MALRIL_PARCEL_FIELD_MTU = 2,
} MALRIL_ParcelFieldType;
#define MALRIL_RESPONSE_SOLICITED                      (0)
#define MALRIL_RESPONSE_UNSOLICITED                    (1)
#define MALRIL_REQUEST_RADIO_POWER                     23
#define MALRIL_REQUEST_SETUP_DATA_CALL                 27
#define MALRIL_REQUEST_DEACTIVATE_DATA_CALL            41
#define MALRIL_REQUEST_LAST_DATA_CALL_FAIL_CAUSE       56
#define MALRIL_REQUEST_SET_INITIAL_ATTACH_APN          111
#define MALRIL_REQUEST_VENDOR_BASE                     2000
#define MALRIL_REQUEST_SETUP_DEDICATE_DATA_CALL        (MALRIL_REQUEST_VENDOR_BASE + 77)
#define MALRIL_REQUEST_DEACTIVATE_DEDICATE_DATA_CALL   (MALRIL_REQUEST_VENDOR_BASE + 78)
#define MALRIL_REQUEST_MODIFY_DATA_CALL                (MALRIL_REQUEST_VENDOR_BASE + 79)
#define MALRIL_REQUEST_ABORT_SETUP_DATA_CALL           (MALRIL_REQUEST_VENDOR_BASE + 80)
#define MALRIL_REQUEST_PCSCF_DISCOVERY_PCO             (MALRIL_REQUEST_VENDOR_BASE + 81)
#define MALRIL_REQUEST_AT_COMMAND_WITH_PROXY           (MALRIL_REQUEST_VENDOR_BASE + 106)
#define MALRIL_REQUEST_HANDOVER_IND                    (MALRIL_REQUEST_VENDOR_BASE + 112)
#define MALRIL_REQUEST_AT_COMMAND_WITH_PROXY_CDMA      (MALRIL_REQUEST_VENDOR_BASE + 123)
#define MALRIL_REQUEST_WIFI_DISCONNECT_IND             (MALRIL_REQUEST_VENDOR_BASE + 124)
#define MALRIL_REQUEST_MAL_PS_RGEGISTRATION_STATE      (MALRIL_REQUEST_VENDOR_BASE + 125)
#define MALRIL_REQUEST_OEM_HOOK_RAW_WITH_PROXY         (MALRIL_REQUEST_VENDOR_BASE + 126)
#define MALRIL_REQUEST_WIFI_CONNECT_IND                (MALRIL_REQUEST_VENDOR_BASE + 127)
#define MALRIL_REQUEST_SETUP_DATA_CALL_ALT             (MALRIL_REQUEST_VENDOR_BASE + 157)
#define MALRIL_REQUEST_LAST_DATA_CALL_FAIL_CAUSE_ALT   (MALRIL_REQUEST_VENDOR_BASE + 158)
#define MALRIL_REQUEST_PCSCF_DISCOVERY_PCO_ALT         (MALRIL_REQUEST_VENDOR_BASE + 159)
#define MALRIL_UNSOL_SIGNAL_STRENGTH                   1009
#define MALRIL_UNSOL_VENDOR_BASE                       3000
#define MALRIL_UNSOL_DEDICATE_BEARER_ACTIVATED         (MALRIL_UNSOL_VENDOR_BASE + 30)
#define MALRIL_UNSOL_DEDICATE_BEARER_MODIFIED          (MALRIL_UNSOL_VENDOR_BASE + 31)
#define MALRIL_UNSOL_DEDICATE_BEARER_DEACTIVATED       (MALRIL_UNSOL_VENDOR_BASE + 32)
#define MALRIL_UNSOL_SETUP_DATA_CALL_RESPONSE          (MALRIL_UNSOL_VENDOR_BASE + 72)
#define MALRIL_LOCAL_GSM_UNSOL_VENDOR_BASE             6000
#define MALRIL_LOCAL_GSM_UNSOL_MAL_DATA_CALL_LIST_CHANGED (MALRIL_LOCAL_GSM_UNSOL_VENDOR_BASE + 2)
#define MALRIL_LOCAL_C2K_UNSOL_VENDOR_BASE             7000
#define MALRIL_LOCAL_C2K_UNSOL_MAL_DATA_CALL_LIST_CHANGED (MALRIL_LOCAL_C2K_UNSOL_VENDOR_BASE + 3)
#endif /* __MAL_RIL2MAL_INTERFACE_H__ */
