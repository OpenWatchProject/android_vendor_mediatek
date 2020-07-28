package com.mediatek.ims;

/**
 * This class enables an application to get details on why a WFC.
 * registration/call failed.
 * @hide
 */
public class WfcReasonInfo {

    /**
     * Specific code of each types.
     */
    public static final int CODE_UNSPECIFIED = 999;

    /* Registration Successful, No error code. */
    public static final int CODE_WFC_SUCCESS = 99;

    /* Code for WFC OFF & default errors, if any */
    public static final int CODE_WFC_DEFAULT = 100;

    /**
     * RNS (RNS -> Connectivity Service (CS)).
     */
    // Wi-Fi signal lost
    public static final int CODE_WFC_WIFI_SIGNAL_LOST = 2001;

    // Unable to complete call
    public static final int CODE_WFC_UNABLE_TO_COMPLETE_CALL = 2003;

    // No available qualified mobile network
    public static final int CODE_WFC_NO_AVAILABLE_QUALIFIED_MOBILE_NETWORK = 2004;

    // Unable to complete call
    public static final int CODE_WFC_UNABLE_TO_COMPLETE_CALL_CD = 2005;

    // No RAT is available
    public static final int CODE_WFC_RNS_ALLOWED_RADIO_DENY = 2006;

    // (No LTE no Wi-fi) only 2G/3G is available
    public static final int CODE_WFC_RNS_ALLOWED_RADIO_NONE = 2007;

    /**
     * DNS error.
     */

    // Unable to receive response to NAPTR query
    public static final int CODE_WFC_DNS_RECV_NAPTR_QUERY_RSP_ERROR = 1201;

    // Unable to receive response to SRV query
    public static final int CODE_WFC_DNS_RECV_RSP_SRV_QUERY_ERROR = 1202;

    // Unable to receive response to A query
    public static final int CODE_WFC_DNS_RECV_RSP_QUERY_ERROR = 1203;

    // ePDG Wi-Fi calling- Device is unable to resolve FQDN for the ePDG to an
    // IP Address
    public static final int CODE_WFC_DNS_RESOLVE_FQDN_ERROR = 1041;

    /**
     * SIM error.
     */

    // Incorrect SIM card used (ex: no UICC with GBA support being used)
    // The error will be displayed when the SIM card inserted is not GBA capable
    // or is not a TMO SIM card
    public static final int CODE_WFC_INCORRECT_SIM_CARD_ERROR = 1301;

    /**
     * Connection Error (Unable to connect).
     */
    // Error shown when the device experiences a Null pointer Exception error
    // or an error local to the device before registration attempts are made
    public static final int CODE_WFC_LOCAL_OR_NULL_PTR_ERROR = 1401;

    // Device is not able to connect to ePDG or experiences a Null pointer error
    // or an error local to the device before registration attempts are made
    public static final int CODE_WFC_EPDG_CON_OR_LOCAL_OR_NULL_PTR_ERROR = 1081;

    // Device is unable to establish an IP sec tunnel with the ePDG for any
    // reason
    // other than timeout or certificate validation
    public static final int CODE_WFC_EPDG_IPSEC_SETUP_ERROR = 1082;

    // Device is unable to establish a TLS connection for reasons other than
    // certificate
    // verifications failures. This includes timeout to TCP connection
    public static final int CODE_WFC_TLS_CONN_ERROR = 1405;

    // Error 500: Internal server error
    // Device receives a 500 error message in response to the register message
    public static final int CODE_WFC_INTERNAL_SERVER_ERROR = 1406;

    // REG99: All other failures
    public static final int CODE_WFC_ANY_OTHER_CONN_ERROR = 1407;

    /**
     * Certificate Error (Invalid Certificate).
     */

    // Unable to validate the server certificate (this would happen if the URL
    // for certificate revocation cannot be resolved to IP or
    // the server cannot be reached in case of a firewall or captive portal)
    public static final int CODE_WFC_SERVER_CERT_VALIDATION_ERROR = 1501;

    // Unable to validate the server certificate for IP sec tunnel establishment
    public static final int CODE_WFC_SERVER_IPSEC_CERT_VALIDATION_ERROR = 1101;

    // The certificate during IP sec tunnel establishment is invalid:
    // certificate is revoked or has expired
    public static final int CODE_WFC_SERVER_IPSEC_CERT_INVALID_ERROR = 1111;

    // The certificate is invalid: certificate is revoked, certificate expired,
    public static final int CODE_WFC_SERVER_CERT_INVALID_ERROR = 1504;

    /**
     * Network returns 403 Forbidden to the Register message.
     */
    // Error 403:
    public static final int CODE_WFC_403_FORBIDDEN = 1600;

    // Error 403: Unknown user
    public static final int CODE_WFC_403_UNKNOWN_USER = 1601;

    // Error 403: Roaming not allowed
    public static final int CODE_WFC_403_ROAMING_NOT_ALLOWED = 1602;

    // Error 403: Mismatch identities
    public static final int CODE_WFC_403_MISMATCH_IDENTITIES = 1603;

    // Error 403: authentication scheme unsupported
    public static final int CODE_WFC_403_AUTH_SCHEME_UNSUPPORTED = 1604;

    // Error 403: handset is blacklisted
    public static final int CODE_WFC_403_HANDSET_BLACKLISTED = 1605;

    // Error 403: WiFi Calling Not Allowed from this Region
    public static final int CODE_WFC_403_NOT_ALLOWED_FROM_THIS_REGION = 1606;

    /**
     * No 911 address on file (Missing 911 address).
     */

    // Missing 911 address
    public static final int CODE_WFC_911_MISSING = 1701;

    // Error 503: Emergency Calls over Wi-Fi is not allowed in this location
    public static final int CODE_WFC_503_ECC_OVER_WIFI_NOT_ALLOWED = 1702;

    // Error 606: WiFi Calling IP is not Acceptable
    public static final int CODE_WFC_606_WIFI_CALLING_IP_NOT_ACCEPTABLE = 1703;

    /**
     * Returns status code corresponding to WFC status.
     * @param status indicate WFC status
     * @return WFC error code
     */
    public static int getImsStatusCodeString(int status) {
        // WFC status string resounce id
        int resId = 0;
        switch (status) {
            default:
                break;
        }
        return resId;
    }

}
