/*******************************************************************************
 * Software Name : RCS IMS Stack
 *
 * Copyright (C) 2010-2016 Orange.
 * Copyright (C) 2014 Sony Mobile Communications Inc.
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
 *
 * NOTE: This file has been modified by Sony Mobile Communications Inc.
 * Modifications are licensed under the License.
 ******************************************************************************/
package com.mediatek.rcs.provisioning.https;

import android.app.PendingIntent;
import android.content.Context;
import android.net.ConnectivityManager;
import android.net.NetworkInfo;
import android.net.Uri;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.SystemProperties;
import android.telephony.TelephonyManager;
import android.telephony.SubscriptionManager;
import android.text.TextUtils;

import com.mediatek.rcs.provisioning.AcsConfigInfo;
import com.mediatek.rcs.provisioning.ProvisioningFailureReasons;
import com.mediatek.rcs.provisioning.ProvisioningInfo;
import com.mediatek.rcs.provisioning.ProvisioningInfo.Version;
import com.mediatek.rcs.provisioning.ProvisioningParser;
import com.mediatek.rcs.provisioning.R;
import com.mediatek.rcs.provisioning.TermsAndConditionsRequest;
import com.mediatek.rcs.utils.CloseableUtils;
import com.mediatek.rcs.utils.ContactId;
import com.mediatek.rcs.utils.DeviceUtils;
import com.mediatek.rcs.utils.LauncherUtils;
import com.mediatek.rcs.utils.NetworkUtils;
import com.mediatek.rcs.utils.RcsSettings;
import com.mediatek.rcs.utils.RcsSettings.GsmaRelease;
import com.mediatek.rcs.utils.RcsSettings.TermsAndConditionsResponse;
import com.mediatek.rcs.utils.StringUtils;
import com.mediatek.rcs.utils.TerminalInfo;
import com.mediatek.rcs.utils.logger.Logger;

import java.io.BufferedReader;
import java.io.DataInputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.net.CookieHandler;
import java.net.CookieManager;
import java.net.HttpCookie;
import java.net.HttpURLConnection;
import java.net.URL;
import java.util.List;
import java.util.Locale;
import java.util.Map;
import java.util.regex.Pattern;

import javax.net.ssl.HostnameVerifier;
import javax.net.ssl.HttpsURLConnection;
import javax.net.ssl.SSLSession;

import org.xml.sax.SAXException;

import static com.mediatek.rcs.utils.StringUtils.UTF8;

/**
 * Provisioning via network manager
 *
 * @author hlxn7157
 * @author G. LE PESSOT
 * @author Deutsche Telekom AG
 * @author yplo6403
 */
public class HttpsProvisioningManager {

    /**
     * Rate to convert from seconds to milliseconds
     */
    private static final long SECONDS_TO_MILLISECONDS_CONVERSION_RATE = 1000;

    private static final int HTTP_STATUS_ERROR_NETWORK_AUTHENTICATION_REQUIRED = 511;

    private static final String PROVISIONING_URI_FILENAME = "rcs_provisioning_uri.txt";

    private static final String PARAM_VERS = "vers";

    private static final String PARAM_RCS_VERSION = "rcs_version";

    private static final String PARAM_RCS_PROFILE = "rcs_profile";

    private static final String PARAM_RCS_STATE = "rcs_state";

    private static final String PARAM_CLIENT_VENDOR = "client_vendor";

    private static final String PARAM_CLIENT_VERSION = "client_version";

    private static final String PARAM_TERMINAL_VENDOR = "terminal_vendor";

    private static final String PARAM_TERMINAL_MODEL = "terminal_model";

    private static final String PARAM_TERMINAL_SW_VERSION = "terminal_sw_version";

    private static final String PARAM_IMSI = "IMSI";

    private static final String PARAM_IMEI = "IMEI";

    private static final String PARAM_SMS_PORT = "SMS_port";

    private static final String PARAM_MSISDN = "msisdn";

    private static final String PARAM_TOKEN = "token";

    // for ATT
    private static final String PARAM_DEFAULT_SMS_APP = "default_sms_app";

    private static final String PROVISIONING_OPERATIONS_THREAD_NAME = "ProvisioningOps";

    /**
     * Result content while waiting for OTP
     */
    private static final String RESULT_CONTENT = "Otp sent to terminal";

    /**
     * Timer internal for OTP
     */
    private static final int OTP_TIME_OUT = 120000;

    /**
     * Retry max count for OTP
     */
    private static final int RETRY_MAX_COUNT_OTP = 10;

    /**
     * First launch flag
     */
    private boolean mFirstProvAfterBoot = false;

    /**
     * User action flag
     */
    private boolean mUser = false;

    private int mRetryCount = 0;

    private HttpsProvisioningSMS mSmsManager;

    private HttpsProvisioningConnection mNetworkCnx;

    /**
     * Waiting for OTP flag
     */
    private boolean mWaitingForOTP;

    private final Context mCtx;

    /**
     * Handler to process messages & runnable associated with background thread.
     */
    private Handler mProvisioningOperationHandler;

    /**
     * Retry after 511 "Network authentication required" counter
     */
    private int mRetryAfter511ErrorCount = 0;

    private final PendingIntent mRetryIntent;

    private final RcsSettings mRcsSettings;

    //private RcsAccountManager mRcsAccountManager;

    private String mImsi;

    private String mImei;

    private final static int BUFFER_READER_SIZE = 1000;

    /**
     * Builds HTTPS request parameters that are related to Terminal, PARAM_RCS_VERSION &
     * PARAM_RCS_PROFILE.
     * <p>
     * EXCLUDE - PARAM_VERS & OPTIONAL ARGS
     * </p>
     * <p>
     * OPTIONAL ARGS = PARAM_IMSI, PARAM_IMEI, PARAM_SMS_PORT, PARAM_MSISDN & PARAM_TOKEN
     * </p>
     */
    private static Uri.Builder sHttpsReqUriBuilder;

    private static final Logger sLogger = Logger
            .getLogger(HttpsProvisioningManager.class.getName());

    private boolean  mIsConfiguring = false;

    private String mLocalCookie;

    private HostnameVerifier mHostnameVerifier = new HostnameVerifier() {
            @Override
            public boolean verify(String hostname, SSLSession session) {
                if (hostname != null) {
                    Pattern pattern =
                        Pattern.compile("(^config\\.rcs\\.mnc\\d+.mcc\\d+\\.pub\\.3gppnetwork\\.org$)|(^config\\-rcs\\.cmw500\\-test\\.com$)");
                    if (pattern.matcher(hostname).find()) {
                        return true;
                    }
                }

                HostnameVerifier hv = HttpsURLConnection.getDefaultHostnameVerifier();
                return hv.verify(hostname, session);
            }
        };

    /**
     * Constructor
     *
     * @param imei International Mobile Equipment Identity
     * @param imsi International Mobile Subscriber Identity
     * @param context application context
     * @param localContentResolver Local content resolver
     * @param retryIntent pending intent to update periodically the configuration
     * @param first is provisioning service launch after (re)boot ?
     * @param user is provisioning service launch after user action ?
     * @param rcsSettings RCS settings accessor
     * @param messagingLog Message log accessor
     * @param contactManager Contact manager accessor
     */
    public HttpsProvisioningManager(String imei, String imsi, Context context,
                                    final PendingIntent retryIntent,
                                    boolean first, boolean user, RcsSettings rcsSettings) {
        mImsi = imsi;
        mImei = imei;
        mCtx = context;
        mRetryIntent = retryIntent;
        mFirstProvAfterBoot = first;
        mUser = user;
        mRcsSettings = rcsSettings;
    }

    /**
     * Initialize Provisioning Manager
     */
    /* package private */void initialize() {
        mProvisioningOperationHandler = allocateBgHandler(PROVISIONING_OPERATIONS_THREAD_NAME);
        mNetworkCnx = new HttpsProvisioningConnection(this, mCtx);
        mSmsManager = new HttpsProvisioningSMS(this, mCtx);
    }

    private Handler allocateBgHandler(String threadName) {
        HandlerThread thread = new HandlerThread(threadName);
        thread.start();
        return new Handler(thread.getLooper());
    }

    /**
     * Connection event
     *
     * @param action Connectivity action
     * @return true if the updateConfig has been done
     * @throws RcsAccountException
     * @throws IOException
     */
    /* package private */boolean connectionEvent(String action) throws IOException {
        try {
            if (!ConnectivityManager.CONNECTIVITY_ACTION.equals(action)) {
                return false;
            }
            if (!RcsSettings.ATT_SUPPORT) {
                NetworkInfo networkInfo = mNetworkCnx.getConnectionMngr().getActiveNetworkInfo();
                if (networkInfo == null) {
                    return false;
                }
                if (!networkInfo.isConnected()) {
                    if (sLogger.isActivated()) {
                        sLogger.debug("Disconnection from network");
                    }
                    return false;
                }
                if (sLogger.isActivated()) {
                    sLogger.debug("Connected to data network");
                }
            }
            // else {
            //     TelephonyManager tm = (TelephonyManager) mCtx.getSystemService(Context.TELEPHONY_SERVICE);
            //     int nwType = tm.getDataNetworkType();
            //     if (sLogger.isActivated()) {
            //         sLogger.debug("connectionEvent nwType:" + nwType);
            //     }
            //     if (nwType == TelephonyManager.NETWORK_TYPE_UNKNOWN) {
            //         return false;
            //     } else {
            //         // for AT&T, use nxtGenPhone APN
            //         mNetworkCnx.acquireNetwork();
            //     }

            //     networkInfo = mNetworkCnx.getConnectionMngr().getActiveNetworkInfo();
            //     if (networkInfo == null ||  !networkInfo.isConnected()) {
            //         if (sLogger.isActivated()) {
            //             sLogger.debug("Disconnection from network");
            //         }
            //         return false;
            //     }
            //     AcsService.onConnectionStateChanged(0);
            //     if (sLogger.isActivated()) {
            //         sLogger.debug("Connected to data network");
            //     }
            // }

            updateConfig();
            return true;
        } catch (Exception e) {
            retry();
            e.printStackTrace();
            return false;
        }
    }

    private HttpURLConnection executeHttpRequest(boolean secured, String request)
            throws IOException {
        if (sLogger.isActivatedDebug()) {
            String logRequest =
                (request == null) ? "null" : request.replaceAll(Logger.sLogNumPattern, "*");
            sLogger.debug("executeHttpRequest,secured:" + secured + ",request:" + logRequest);
        }

        String protocol = (secured) ? "https" : "http";
        URL url = new URL(protocol + "://" + request);
        HttpURLConnection cnx = (HttpURLConnection) url.openConnection();
        if (mLocalCookie != null) {
            cnx.setRequestProperty("Cookie", mLocalCookie);
            if (sLogger.isActivatedDebug()) {
                sLogger.debug("executeHttprequest cookie:" + mLocalCookie);
            }
        }

        cnx.setRequestProperty("Accept-Language", HttpsProvisioningUtils.getUserLanguage());
        if (RcsSettings.ATT_SUPPORT) {
            cnx.setConnectTimeout(20 * 1000);
            cnx.setReadTimeout(10 * 1000);
            if (secured) {
                HttpsURLConnection secureConn = (HttpsURLConnection)cnx;
                secureConn.setHostnameVerifier(mHostnameVerifier);
            }
        }

        return cnx;
    }

    /**
     * Get the HTTPS request arguments
     *
     * @param smsPort SMS port
     * @param token Provisioning token
     * @param msisdn MSISDN
     * @return {@link String} with the HTTPS request arguments.
     */
    String getHttpsRequestArguments(String smsPort, String token, ContactId msisdn) {
        if (sHttpsReqUriBuilder == null) {
            sHttpsReqUriBuilder = new Uri.Builder();
            sHttpsReqUriBuilder.appendQueryParameter(PARAM_RCS_VERSION,
                    HttpsProvisioningUtils.getRcsVersion());
            sHttpsReqUriBuilder.appendQueryParameter(PARAM_RCS_PROFILE,
                    HttpsProvisioningUtils.getRcsProfile());
            // if (mRcsSettings.isEnrichCallingServiceSupported()) {
            //     sHttpsReqUriBuilder.appendQueryParameter(PARAM_RCS_PROFILE,
            //             HttpsProvisioningUtils.getEnrichCallingProfile());
            // }
            sHttpsReqUriBuilder.appendQueryParameter(PARAM_CLIENT_VENDOR,
                    TerminalInfo.getClientVendor());
            sHttpsReqUriBuilder.appendQueryParameter(PARAM_CLIENT_VERSION,
                    TerminalInfo.getClientVersion(mCtx));
            sHttpsReqUriBuilder.appendQueryParameter(PARAM_TERMINAL_VENDOR,
                    TerminalInfo.getTerminalVendor());
            sHttpsReqUriBuilder.appendQueryParameter(PARAM_TERMINAL_MODEL,
                    TerminalInfo.getTerminalModel());
            sHttpsReqUriBuilder.appendQueryParameter(PARAM_TERMINAL_SW_VERSION,
                    TerminalInfo.getTerminalSoftwareVersion());
        }
        int provisioningVersion = mRcsSettings.getProvisioningVersion();
        if (mUser && Version.DISABLED_DORMANT.toInt() == provisioningVersion) {
            provisioningVersion = LauncherUtils.getProvisioningVersion(mCtx);
            mUser = false;
        }
        final Uri.Builder uriBuilder = sHttpsReqUriBuilder.build().buildUpon();
        uriBuilder.appendQueryParameter(PARAM_VERS, String.valueOf(provisioningVersion));
        uriBuilder.appendQueryParameter(PARAM_IMSI, mImsi);
        uriBuilder.appendQueryParameter(PARAM_IMEI, mImei);
        if (RcsSettings.ATT_SUPPORT) {
            uriBuilder.appendQueryParameter(PARAM_RCS_STATE, HttpsProvisioningUtils.getRcsState());
            String defaultSmsApp =
                mCtx.getString(R.string.default_sms_app).equals(mRcsSettings.getDefaultSmsApp()) ?
                "1" :
                "2";
            uriBuilder.appendQueryParameter(PARAM_DEFAULT_SMS_APP, defaultSmsApp);
            uriBuilder.appendQueryParameter("provisioning_version", "2.0");
        }
        /*
         * Add optional parameters only if available
         */
        if (smsPort != null) {
            uriBuilder.appendQueryParameter(PARAM_SMS_PORT, smsPort);
        }
        if (msisdn != null) {
            uriBuilder.appendQueryParameter(PARAM_MSISDN, msisdn.toString());
        }
        /*
         * RCS standard: "The token shall be stored on the device so it can be used in subsequent
         * configuration requests over non-3GPP access." In 3GPP access, the token is only
         * compulsory for non 3GPP access and is not used for 3GPP access. It shall then not be
         * inserted as a URI parameter for 3GPP access.
         */
        if (NetworkUtils.getNetworkAccessType(mCtx) == NetworkUtils.NETWORK_ACCESS_WIFI
            || RcsSettings.ATT_SUPPORT) {
            /*
             * According to the standard the token-parameter name should be part of the Uri even if
             * the token is null here but only for non 3gpp access.
             */
            uriBuilder.appendQueryParameter(PARAM_TOKEN, token == null ? "" : token);
        }

        return uriBuilder.toString();
    }

    /**
     * Send the first HTTPS request to require the one time password (OTP)
     *
     * @param msisdn the phone number
     * @param primaryUri the primary URI
     * @param secondaryUri the secondary URI
     * @return Instance of {@link HttpsProvisioningResult} or null in case of internal exception
     * @throws IOException
     */
    private HttpsProvisioningResult sendFirstRequestsToRequireOTP(ContactId msisdn,
            String primaryUri, String secondaryUri) throws IOException {
        HttpsProvisioningResult result = new HttpsProvisioningResult();
        boolean logActivated = sLogger.isActivated();
        if (sLogger.isActivatedDebug()) {
            sLogger.debug("Send 1rst HTTPS request to get OTP (retry_count="
                    + mRetryCount + ")(waitingOTP=" + mWaitingForOTP + ")");
        }
        HttpURLConnection urlConnection = null;
        try {
            if (!RcsSettings.ATT_SUPPORT) {
                /* Condition to check to show MSISDN popup after OTP time out */
                if (msisdn != null && mRetryCount > 0 && mWaitingForOTP) {
                    mWaitingForOTP = false;
                    ContactId contact = mRcsSettings.getUserProfileImsUserName();
                    /* Displays a popup with previously given MSISDN to edit and retry for OTP */
                    msisdn = HttpsProvisioningMsisdnInput.getInstance()
                        .displayPopupAndWaitResponse(mCtx, contact);
                    if (!contactIdEquals(contact, msisdn)) {
                        mRcsSettings.setUserProfileImsUserName(msisdn);
                    }
                    if (msisdn == null) {
                        if (logActivated) {
                            sLogger.warn("No MSISDN set by end user: cannot authenticate!");
                        }
                        result.code = HttpsProvisioningResult.UNKNOWN_MSISDN_CODE;
                        return result;
                    }
                }
            } else {
                mWaitingForOTP = false;
            }

            /* Generate the SMS port for provisioning */
            String smsPortForOTP = HttpsProvisioningSMS.generateSmsPortForProvisioning();
            if (RcsSettings.ATT_SUPPORT) {
                smsPortForOTP = "37273";
            }

            /*
             * Format first HTTPS request with extra parameters (IMSI and IMEI if available plus
             * SMS_port and token).
             */
            String token = mRcsSettings.getProvisioningToken();
            String args = getHttpsRequestArguments(smsPortForOTP, token, msisdn);

            /* Execute first HTTPS request with extra parameters */
            String request = primaryUri + args;
            urlConnection = executeHttpRequest(true, request);
            result.code = urlConnection.getResponseCode();

            Map<String, List<String>> headerFields = urlConnection.getHeaderFields();
            List<String> Cookies = headerFields.get("Set-Cookie");
            if (Cookies != null) {
                for (String cookie : Cookies) {
                    mLocalCookie = cookie;
                    if (sLogger.isActivatedDebug()) {
                        sLogger.debug("response cookie:" + cookie);
                    }
                }
            }

            if (HttpURLConnection.HTTP_OK != result.code && !StringUtils.isEmpty(secondaryUri)) {
                // To compatible with server not compliance RFC 2965
                List<HttpCookie> cookies = ((CookieManager)CookieManager.getDefault()).getCookieStore()
                        .getCookies();
                for (HttpCookie cookie : cookies) {
                    cookie.setVersion(0);
                }

                /* First server not available, try the secondaryUri */
                request = secondaryUri + args;
                urlConnection.disconnect();
                urlConnection = null;
                urlConnection = executeHttpRequest(true, request);
                result.code = urlConnection.getResponseCode();
            }
            switch (result.code) {
                case HttpURLConnection.HTTP_OK:
                    if (logActivated) {
                        sLogger.debug("HTTPS request returns with 200 OK");
                    }
                    result.content = readStream(urlConnection.getInputStream());
                    /*
                     * If the content is empty, means that the configuration XML is not present and
                     * the Token is invalid then we need to wait for the SMS with OTP.
                     */
                    if (TextUtils.isEmpty(result.content)
                            || result.content.contains(RESULT_CONTENT)) {
                        if (mRetryCount < HttpsProvisioningUtils.RETRY_MAX_COUNT) {
                            mRetryCount++;
                            if (logActivated) {
                                sLogger.debug("sendFirstRequestsToRequireOTP Retry count:"
                                              + mRetryCount);
                            }
                            mWaitingForOTP = true;
                            result.waitingForSMSOTP = true;
                            /* Register SMS provisioning receiver */
                            mSmsManager.registerSmsProvisioningReceiver(smsPortForOTP, primaryUri);

                            int retryDelay = OTP_TIME_OUT;
                            if (RcsSettings.ATT_SUPPORT) {
                                retryDelay = HttpsProvisioningUtils.RETRY_SCHEME[mRetryCount - 1];
                            }

                            /* Start retry alarm for OTP */
                            AcsService.startRetryAlarm(mCtx, mRetryIntent, retryDelay);
                        }
                    }
                    return result;

                case HttpURLConnection.HTTP_FORBIDDEN:
                case HttpURLConnection.HTTP_UNAUTHORIZED:
                    if (sLogger.isActivatedDebug()) {
                        sLogger.debug("Request to get OTP failed: Forbidden.");
                    }
                    if (RcsSettings.ATT_SUPPORT) {
                        return result;
                    }
                    if (mRetryCount < HttpsProvisioningUtils.RETRY_MAX_COUNT) {
                        mRetryCount++;
                        if (sLogger.isActivated()) {
                            sLogger.debug("Retry provisioning count: " + mRetryCount);
                        }

                        ContactId contact = mRcsSettings.getUserProfileImsUserName();
                        msisdn = HttpsProvisioningMsisdnInput.getInstance()
                            .displayPopupAndWaitResponse(mCtx, contact);
                        if (!contactIdEquals(contact, msisdn)) {
                            mRcsSettings.setUserProfileImsUserName(msisdn);
                        }
                        if (msisdn == null) {
                            if (logActivated) {
                                sLogger.warn("No MSISDN set by end user: cannot authenticate!");
                            }
                            result.code = HttpsProvisioningResult.UNKNOWN_MSISDN_CODE;
                            return result;
                        }
                        return sendFirstRequestsToRequireOTP(msisdn, primaryUri, secondaryUri);
                    }

                case HttpURLConnection.HTTP_UNAVAILABLE:
                    result.retryAfter = getRetryAfter(urlConnection);
                    /* Intentional fall through */
                    //$FALL-THROUGH$
                default:
                    if (logActivated) {
                        sLogger.debug("Request to get OTP failed: code=" + result.code);
                    }
                    return result;
            }
        } finally {
            if (urlConnection != null) {
                urlConnection.disconnect();
            }
            /* If not waiting for the SMS with OTP */
            if (!result.waitingForSMSOTP && HttpURLConnection.HTTP_FORBIDDEN != result.code) {
                mSmsManager.unregisterSmsProvisioningReceiver();
            }
        }
    }

    private boolean contactIdEquals(ContactId a, ContactId b) {
        return a == b || (a != null && a.equals(b));
    }

    /**
     * Update provisioning config with OTP
     *
     * @param otp One time password
     * @param requestUri Request URI
     * @throws RcsAccountException thrown if RCS account failed to be created
     * @throws IOException
     */
    /* package private */void updateConfigWithOTP(String otp, String requestUri)
        throws //RcsAccountException,
        IOException {
        try {
            // Get config via HTTPS with OTP
            HttpsProvisioningResult result = sendSecondHttpsRequestWithOTP(otp, requestUri);

            if (RcsSettings.ATT_SUPPORT) {
                AcsService.onConnectionStateChanged(0);
            }

            // Cancel previous retry alarm
            AcsService.cancelRetryAlarm(mCtx, mRetryIntent);

            //
            // // Process HTTPS provisioning result
            processProvisioningResult(result);
        } finally {
            if (RcsSettings.ATT_SUPPORT) {
                mNetworkCnx.releaseNetwork();
                mIsConfiguring = false;
                AcsService.onConnectionStateChanged(1);
                int version = mRcsSettings.getProvisioningVersion();
                int state = (version > 0)
                    ? AcsConfigInfo.CONFIGURED
                    : AcsConfigInfo.PRE_CONFIGURATION;
                // since OPT, token may change, force notify change?
                AcsService.onConfigChange(state, version);
            }
        }


    }

    /**
     * Build the provisioning address with operator information
     *
     * @return provisioning URI
     */
    String buildProvisioningAddress() {
        int tmpmcc = 0;
        int tmpmnc = 0;
        int subId = SubscriptionManager.getDefaultDataSubscriptionId();
        TelephonyManager tm = (TelephonyManager)mCtx.getSystemService(Context.TELEPHONY_SERVICE);
        final String mccmnc = tm.getSimOperator(subId);
        try {
            tmpmcc = Integer.parseInt(mccmnc.substring(0, 3));
            tmpmnc = Integer.parseInt(mccmnc.substring(3));
            if (tmpmcc != mRcsSettings.getMobileCountryCode()) {
                mRcsSettings.setMobileCountryCode(tmpmcc);
            }
            if (tmpmnc != mRcsSettings.getMobileNetworkCode()) {
                mRcsSettings.setMobileNetworkCode(tmpmnc);
            }
        } catch (Exception e) {
            if (sLogger.isActivated()) {
                sLogger.error("getMccMnc: invalid string" + mccmnc);
            }
        }
        String mnc = String.format(Locale.US, "%03d", tmpmnc);
        String mcc = String.format(Locale.US, "%03d", tmpmcc);
        String addr = "config.rcs.mnc" + mnc + ".mcc" + mcc + ".pub.3gppnetwork.org";
        if (!RcsSettings.ATT_SUPPORT) {
            return addr;
        } else {
            String userAddr = mRcsSettings.getUserProvisioningAddr();
            if (userAddr != null && !userAddr.equals("")) {
                return userAddr;
            }
            return SystemProperties.get("persist.vendor.mtk_acs_url", addr);
        }
    }

    private static String readStream(InputStream in) throws IOException {
        StringBuilder sb = new StringBuilder();
        try {
            BufferedReader r = new BufferedReader(new InputStreamReader(in, UTF8),
                    BUFFER_READER_SIZE);
            for (String line = r.readLine(); line != null; line = r.readLine()) {
                sb.append(line);
            }
            return sb.toString();
        } finally {
            CloseableUtils.tryToClose(in);
        }
    }

    /**
     * Get configuration
     *
     * @return Result or null in case of internal exception
     * @throws IOException
     */
    private HttpsProvisioningResult getConfig() throws IOException {
        HttpsProvisioningResult result = new HttpsProvisioningResult();
        boolean logActivated = sLogger.isActivated();
        if (logActivated) {
            sLogger.debug("Request config via HTTPS");
        }
        HttpURLConnection urlConnection = null;
        String primaryUri;
        String secondaryUri = null;
        try {
            /* Get provisioning address */
            String secondaryAddress = mRcsSettings.getSecondaryProvisioningAddress();
            if (secondaryAddress != null && mRcsSettings.isSecondaryProvisioningAddressOnly()) {
                primaryUri = secondaryAddress;
            } else {
                primaryUri = buildProvisioningAddress();
                secondaryUri = mRcsSettings.getSecondaryProvisioningAddress();
            }
            /*
             * Override primary URI if a file containing URI for HTTPS provisioning exists
             */
            File provFile = new File(mCtx.getFilesDir(), PROVISIONING_URI_FILENAME);
            if (provFile.exists()) {
                primaryUri = getPrimaryProvisionigServerUriFromFile(provFile);
                secondaryUri = null;
            }
            if (sLogger.isActivatedDebug()) {
                sLogger.debug("ACS Uri to connect: " + primaryUri + " or " + secondaryUri);
            }
            CookieManager cookieManager = new CookieManager();
            CookieHandler.setDefault(cookieManager);
            ContactId contactId = mRcsSettings.getUserProfileImsUserName();
            if (!RcsSettings.ATT_SUPPORT) {
                NetworkInfo networkInfo = mNetworkCnx.getConnectionMngr().getActiveNetworkInfo();
                /* If network is not mobile network, use request with OTP */
                if (networkInfo != null && networkInfo.getType() != ConnectivityManager.TYPE_MOBILE) {
                    // Proceed with non mobile network registration
                    return sendFirstRequestsToRequireOTP(contactId, primaryUri, secondaryUri);
                }
                if (logActivated) {
                    sLogger.debug("HTTP provisioning on mobile network");
                }
            } else {
                String token = mRcsSettings.getProvisioningToken();
                if (token == null) {
                    if (logActivated) {
                        sLogger.debug("toke is null, enter OTP flow");
                    }
                    return sendFirstRequestsToRequireOTP(contactId, primaryUri, secondaryUri);
                }
            }

            String requestUri = primaryUri;
            String token = mRcsSettings.getProvisioningToken();
            String request = requestUri + getHttpsRequestArguments(null, token, contactId);
            urlConnection = executeHttpRequest(true, request);
            result.code = urlConnection.getResponseCode();
            if (HttpURLConnection.HTTP_OK != result.code && !StringUtils.isEmpty(secondaryUri)) {
                urlConnection.disconnect();
                urlConnection = null;
                /* First server not available, try the secondaryUri */
                requestUri = secondaryUri;
                request = requestUri + getHttpsRequestArguments(null, token, contactId);
                urlConnection = executeHttpRequest(true, request);
                result.code = urlConnection.getResponseCode();
            }

            if (logActivated) {
                sLogger.debug("[getConfig] HTTP request result.code:" + result.code);
            }
            switch (result.code) {
                case HttpURLConnection.HTTP_OK:
                    result.content = readStream(urlConnection.getInputStream());
                    return result;

                case HTTP_STATUS_ERROR_NETWORK_AUTHENTICATION_REQUIRED:
                    /*
                     * Blackbird guidelines ID_2_6 Configuration mechanism over PS without Header
                     * Enrichment Use SMS provisioning on PS data network if server reply 511
                     * NETWORK AUTHENTICATION REQUIRED
                     */
                    mRcsSettings.setProvisioningToken(null);
                    return sendFirstRequestsToRequireOTP(contactId, primaryUri, secondaryUri);

                // HTTP_UNAVAILABLE:503
                case HttpURLConnection.HTTP_UNAVAILABLE:
                    result.retryAfter = getRetryAfter(urlConnection);
                    /* Intentional fall through */
                    //$FALL-THROUGH$
                default:
                    if (logActivated) {
                        sLogger.debug("[getConfig] HTTPS request failed with code " + result.code);
                    }
                    return result;
            }
        } finally {
            if (urlConnection != null) {
                urlConnection.disconnect();
            }
        }
    }

    private String getPrimaryProvisionigServerUriFromFile(File provFile) throws IOException {
        DataInputStream dataInputStream = null;
        try {
            if (sLogger.isActivated()) {
                sLogger.debug("Provisioning URI file found !");
            }
            FileInputStream fis = new FileInputStream(provFile);
            dataInputStream = new DataInputStream(fis);
            BufferedReader br = new BufferedReader(new InputStreamReader(dataInputStream));
            return br.readLine();

        } finally {
            CloseableUtils.tryToClose(dataInputStream);
        }
    }

    /**
     * Update provisioning config
     *
     * @throws RcsAccountException thrown if RCS account failed to be created
     * @throws IOException
     */
    /* package private */void updateConfig() throws // RcsAccountException,
        IOException, Exception {
        if (RcsSettings.ATT_SUPPORT) {
            if (mRetryCount > HttpsProvisioningUtils.RETRY_MAX_COUNT) {
                if (sLogger.isActivated()) {
                    sLogger.debug("exit no more retry, mRetryCount:" + mRetryCount);
                }
                // <LTE-ACS-195> exceeds 24h in no more retry state, clear token
                mRcsSettings.setProvisioningToken(null);
                return;
            }
        }

        HttpsProvisioningResult result = null;
        int preVersion = mRcsSettings.getProvisioningVersion();
        // TelephonyManager tm = (TelephonyManager) mCtx.getSystemService(Context.TELEPHONY_SERVICE);
        // int nwType = tm.getDataNetworkType();
        // if (nwType == TelephonyManager.NETWORK_TYPE_UNKNOWN) {
        //     if (sLogger.isActivated()) {
        //         sLogger.debug("connectionEvent nwType:" + nwType);
        //     }
        //     return;
        // }

        try {
            if (RcsSettings.ATT_SUPPORT) {
                // for AT&T, use nxtGenPhone APN
                mIsConfiguring = true;
                mNetworkCnx.acquireNetwork();
                AcsService.onConnectionStateChanged(0);
                if (sLogger.isActivated()) {
                    sLogger.debug("Connected to data network");
                }
            }

            // Cancel previous retry alarm
            AcsService.cancelRetryAlarm(mCtx, mRetryIntent);
            // Get config via HTTPS
            result = getConfig();
            // Process HTTPS provisioning result
            processProvisioningResult(result);
        } finally {
            if (RcsSettings.ATT_SUPPORT) {
                AcsService.onConnectionStateChanged(1);
                if (result == null || !result.waitingForSMSOTP) {
                    mIsConfiguring = false;
                    mNetworkCnx.releaseNetwork();
                    int version = mRcsSettings.getProvisioningVersion();
                    String versionCode = LauncherUtils.getVersionCode(mCtx);
                    if ((preVersion != version) || ("3".equals(versionCode))) {
                        int state = (version > 0)
                            ? AcsConfigInfo.CONFIGURED
                            : AcsConfigInfo.PRE_CONFIGURATION;
                        AcsService.onConfigChange(state, version);
                    }
                }
            }
        }
    }

    /**
     * Send the second HTTPS request with the one time password (OTP)
     *
     * @param otp One time password
     * @param requestUri Request URI
     * @return Instance of {@link HttpsProvisioningResult} or null in case of internal exception
     * @throws IOException
     */
    /* package private */HttpsProvisioningResult sendSecondHttpsRequestWithOTP(String otp,
            String requestUri) throws IOException {
        HttpsProvisioningResult result = new HttpsProvisioningResult();
        boolean logActivated = sLogger.isActivated();
        if (logActivated) {
            sLogger.debug("Send second HTTPS with OTP");
        }
        HttpURLConnection urlConnection = null;
        try {
            /* Format second HTTPS request */
            Uri.Builder uriBuilder = new Uri.Builder();
            if (RcsSettings.ATT_SUPPORT) {
                uriBuilder.appendQueryParameter(PARAM_IMSI, mImsi);
                uriBuilder.appendQueryParameter(PARAM_IMEI, mImei);
                ContactId contact = mRcsSettings.getUserProfileImsUserName();
                if (contact != null) {
                    uriBuilder.appendQueryParameter(PARAM_MSISDN, contact.toString());
                }
            }
            uriBuilder.appendQueryParameter("OTP", otp);
            String request = requestUri + uriBuilder.build().buildUpon().toString();
            /* Execute second HTTPS request */
            urlConnection = executeHttpRequest(true, request);
            result.code = urlConnection.getResponseCode();
            switch (result.code) {
                case HttpURLConnection.HTTP_OK:
                    result.content = readStream(urlConnection.getInputStream());
                    return result;

                case HttpURLConnection.HTTP_UNAVAILABLE:
                    result.retryAfter = getRetryAfter(urlConnection);
                    /* Intentional fall through */
                    //$FALL-THROUGH$
                default:
                    if (logActivated) {
                        sLogger.debug("Request with OTP failed code=" + result.code);
                    }
                    return result;
            }
        } finally {
            if (urlConnection != null) {
                urlConnection.disconnect();
            }
        }
    }

    /**
     * Get retry-after value
     *
     * @return retry-after value in milliseconds
     */
    private long getRetryAfter(HttpURLConnection response) {
        String header = response.getHeaderField("Retry-After");
        if (header == null) {
            return 0;
        }
        if (RcsSettings.ATT_SUPPORT) {
            if (header.equals("")) {
                header = "1800";    //30 minutes
            }
        }

        return Integer.valueOf(header) * SECONDS_TO_MILLISECONDS_CONVERSION_RATE;
    }

    /**
     * Process provisioning result
     *
     * @param result Instance of {@link HttpsProvisioningResult}
     * @throws RcsAccountException thrown if RCS account failed to be created
     */
    private void processProvisioningResult(HttpsProvisioningResult result) {
        boolean logActivated = sLogger.isActivated();
        if (HttpURLConnection.HTTP_OK == result.code) {
            // Reset after 511 counter
            mRetryAfter511ErrorCount = 0;
            if (result.waitingForSMSOTP) {
                if (logActivated) {
                    sLogger.debug("Waiting for SMS with OTP.");
                }
                return;
            }
            if (logActivated) {
                sLogger.debug("Provisioning request successful");
            }
            // Parse the received content
            ProvisioningParser parser = new ProvisioningParser(result.content, mRcsSettings);
            /*
             * Save GSMA release set into the provider. The Node "SERVICES" is mandatory in GSMA
             * release Blackbird and not present in previous one (i.e. Albatros). It is the absence
             * of this node in the configuration which allows us to determine that current release
             * is Albatros
             */
            GsmaRelease gsmaRelease = mRcsSettings.getGsmaRelease();
            /*
             * Save client Messaging Mode set into the provider. The message mode NONE value is not
             * defined in the standard. It is the absence of the messagingUx parameter which allows
             * us to determine that client Message Mode is set to NONE.
             */
            //MessagingMode messagingMode = mRcsSettings.getMessagingMode();

            /* Before parsing the provisioning, the GSMA release is set to Albatros */
            //mRcsSettings.setGsmaRelease(GsmaRelease.ALBATROS);
            /* Before parsing the provisioning, the client Messaging mode is set to NONE */
            //mRcsSettings.setMessagingMode(MessagingMode.NONE);
            try {
                parser.parse(//gsmaRelease, messagingMode,
                             mFirstProvAfterBoot);
                // Successfully provisioned, 1st time reg finalized
                mFirstProvAfterBoot = false;
                ProvisioningInfo info = parser.getProvisioningInfo();
                int preVersion = mRcsSettings.getProvisioningVersion();
                // Save version
                int version = info.getVersion();
                long validity = info.getValidity();
                if (sLogger.isActivatedDebug()) {
                    sLogger.debug("Provisioning version=" + version
                                  + ", validity=" + validity
                                  + ",preVersion:" + preVersion);
                }
                // Save the latest positive version of the configuration
                // LauncherUtils.saveProvisioningVersion(mCtx, version);
                // Save the validity of the configuration
                LauncherUtils.saveProvisioningValidity(mCtx, validity);
                String versionCode = LauncherUtils.getVersionCode(mCtx);
                switch (versionCode) {
                    // 2: for 19.1
                    case "2":
                        if ((preVersion != version) || (version < 0)) {
                            mRcsSettings.setProvisioningVersion(version);
                            mRcsSettings.setConfigFileContent(result.content);
                        }
                        break;

                    // 1: for 18.3
                    case "1":
                        if ((preVersion <= version) || (version < 0)) {
                            mRcsSettings.setProvisioningVersion(version);
                            mRcsSettings.setConfigFileContent(result.content);
                        }
                        break;

                    // 3: for 19.1 Q4
                    case "3":
                    default:
                        mRcsSettings.setProvisioningVersion(version);
                        mRcsSettings.setConfigFileContent(result.content);
                        break;
                }

                // Save token
                String token = info.getToken();
                mRcsSettings.setProvisioningToken(token);
                // mRcsSettings.setFileTransferHttpSupported(mRcsSettings.getFtHttpServer() != null
                //         && mRcsSettings.getFtHttpLogin() != null
                //         && mRcsSettings.getFtHttpPassword() != null);
                // Reset retry alarm counter
                mRetryCount = 0;
                if (sLogger.isActivated()) {
                    sLogger.debug("processProvisioningResult, reset Retry provisioning count");
                }

                if (Version.DISABLED_DORMANT.toInt() == version) {
                    // -3 : Put RCS client in dormant state
                    if (logActivated) {
                        sLogger.debug("Provisioning: RCS client in dormant state");
                    }
                    // Start retry alarm
                    if (validity > 0) {
                        if (RcsSettings.ATT_SUPPORT) {
                            validity = validity * 4 / 5;
                        }
                        AcsService.startRetryAlarm(mCtx, mRetryIntent, validity);
                    }
                    // We parsed successfully the configuration
                    mRcsSettings.setConfigurationValid(true);
                    // Stop the RCS core service. Provisioning is still running.
                    //LauncherUtils.stopRcsCoreService(mCtx);

                    // if (preVersion != version) {
                    //     mRcsSettings.setConfigFileContent(result.content);
                    // }
                } else if (Version.DISABLED_NOQUERY.toInt() == version) {
                    // -2 : Disable RCS client and stop configuration query
                    if (logActivated) {
                        sLogger.debug("Provisioning: disable RCS client");
                    }
                    // We parsed successfully the configuration
                    mRcsSettings.setConfigurationValid(true);
                    // Disable and stop RCS service
                    // mRcsSettings.setServiceActivationState(false);
                    //LauncherUtils.stopRcsService(mCtx);

                    // if (preVersion != version) {
                    //     mRcsSettings.setConfigFileContent(result.content);
                    // }
                } else if (Version.RESETED_NOQUERY.toInt() == version) {
                    // -1 Forbidden: reset account + version = 0-1 (doesn't restart)
                    if (logActivated) {
                        sLogger.debug("Provisioning forbidden: reset account");
                    }
                    // Reset config
                    LauncherUtils.resetRcsConfig(mCtx, mRcsSettings);
                    // Force version to "-1" (resetRcs set version to "0")
                    mRcsSettings.setProvisioningVersion(version);
                    // Disable the RCS service
                    // mRcsSettings.setServiceActivationState(false);
                } else if (Version.RESETED.toInt() == version) {
                    if (logActivated) {
                        sLogger.debug("Provisioning forbidden: no account");
                    }
                    // Reset config
                    LauncherUtils.resetRcsConfig(mCtx, mRcsSettings);
                } else {
                    /* Start retry alarm */
                    if (validity > 0) {
                        if (RcsSettings.ATT_SUPPORT) {
                            validity = validity * 4 / 5;
                        }
                        AcsService.startRetryAlarm(mCtx, mRetryIntent, validity);
                    }
                    boolean tcNotAnswered = TermsAndConditionsResponse.NO_ANSWER == mRcsSettings
                            .getTermsAndConditionsResponse();
                    boolean requestTermsAndConditions = mRcsSettings.isProvisioningAcceptButton()
                            || mRcsSettings.isProvisioningRejectButton();
                    /* Check if Terms and conditions should be requested. */
                    if (requestTermsAndConditions && tcNotAnswered) {
                        TermsAndConditionsRequest.showTermsAndConditions(mCtx);
                    } else {
                        if (tcNotAnswered) {
                            if (logActivated) {
                                sLogger.debug("Terms and conditions implicitly accepted");
                            }
                            // mRcsAccountManager.createRcsAccount(
                            //         mCtx.getString(R.string.rcs_core_account_username), true);
                            mRcsSettings
                                .setTermsAndConditionsResponse(TermsAndConditionsResponse.ACCEPTED);
                        }
                        /* We parsed successfully the configuration */
                        mRcsSettings.setConfigurationValid(true);
                        // if (preVersion != version) {
                        //     mRcsSettings.setConfigFileContent(result.content);
                        // }
                        //LauncherUtils.launchRcsCoreService(mCtx, mRcsSettings);
                    }
                }
                //IntentUtils.sendBroadcastEvent(mCtx,
                //        RcsService.ACTION_SERVICE_PROVISIONING_DATA_CHANGED);

            } catch (SAXException e) {
                if (logActivated) {
                    sLogger.debug("Can't parse provisioning document");
                }
                // Restore GSMA release saved before parsing of the provisioning
                //mRcsSettings.setGsmaRelease(gsmaRelease);
                // Restore the client messaging mode saved before parsing of the provisioning
                //mRcsSettings.setMessagingMode(messagingMode);
                if (mFirstProvAfterBoot) {
                    if (logActivated) {
                        sLogger.debug("As this is first launch and we do not have a valid configuration yet, retry later");
                    }
                    // Reason: Invalid configuration
                    provisioningFails(ProvisioningFailureReasons.INVALID_CONFIGURATION);
                } else {
                    if (logActivated) {
                        sLogger.debug("This is not first launch, use old configuration to register");
                    }
                    //tryLaunchRcsCoreService(mCtx, -1);
                }
                retry();
            }
        } else if (HttpURLConnection.HTTP_UNAVAILABLE == result.code) {
            // Server Unavailable
            if (sLogger.isActivatedDebug()) {
                sLogger.debug("Server Unavailable. Retry after: " + result.retryAfter + "ms");
            }
            if (RcsSettings.ATT_SUPPORT) {
                provisioningFails(ProvisioningFailureReasons.UNABLE_TO_GET_CONFIGURATION);
                if (result.retryAfter > 0) {
                    AcsService.startRetryAlarm(mCtx, mRetryIntent, result.retryAfter);
                } else {
                    retry();
                }
            } else {
                if (mFirstProvAfterBoot) {
                    // Reason: Unable to get configuration
                    provisioningFails(ProvisioningFailureReasons.UNABLE_TO_GET_CONFIGURATION);
                    if (result.retryAfter > 0) {
                        AcsService.startRetryAlarm(mCtx, mRetryIntent, result.retryAfter);
                    }
                }
                //  else {
                //     tryLaunchRcsCoreService(mCtx, result.retryAfter);
                // }
            }
        } else if (HttpURLConnection.HTTP_FORBIDDEN == result.code) {
            // Forbidden: reset account + version = 0
            if (logActivated) {
                sLogger.debug("Provisioning forbidden: reset account");
            }
            if (RcsSettings.ATT_SUPPORT) {
                return;
            }
            // Reset version to "0"
            mRcsSettings.setProvisioningVersion(Version.RESETED.toInt());
            // Reset config
            LauncherUtils.resetRcsConfig(mCtx, mRcsSettings);
            // Reason: Provisioning forbidden
            provisioningFails(ProvisioningFailureReasons.PROVISIONING_FORBIDDEN);
        } else if (HttpURLConnection.HTTP_UNAUTHORIZED == result.code) {
            if (RcsSettings.ATT_SUPPORT) {
                if (logActivated) {
                    sLogger.debug("Provisioning 401...");
                }
                return;
            }
        } else if (HTTP_STATUS_ERROR_NETWORK_AUTHENTICATION_REQUIRED == result.code) {
            // Provisioning authentication required
            if (logActivated) {
                sLogger.debug("Provisioning authentication required");
            }
            // Reset provisioning token
            mRcsSettings.setProvisioningToken(null);
            // Retry after reseting provisioning token
            if (RcsSettings.ATT_SUPPORT) {
                provisioningFails(ProvisioningFailureReasons.CONNECTIVITY_ISSUE);
                retry();
            } else {
                if (!retryAfter511Error()) {
                    // Reason: Provisioning authentication required
                    provisioningFails(ProvisioningFailureReasons.PROVISIONING_AUTHENTICATION_REQUIRED);
                }
            }
        } else {
            // Other error
            if (logActivated) {
                sLogger.debug("Provisioning error " + result.code);
            }
            if (RcsSettings.ATT_SUPPORT) {
                provisioningFails(ProvisioningFailureReasons.CONNECTIVITY_ISSUE);
                retry();
            } else {
                // Start the RCS service
                if (mFirstProvAfterBoot) {
                    // Reason: No configuration present
                    provisioningFails(ProvisioningFailureReasons.CONNECTIVITY_ISSUE);
                    retry();
                }
                // else {
                //     tryLaunchRcsCoreService(mCtx, -1);
                // }
            }
        }
    }

    /**
     * Try to launch RCS Core Service. RCS Service is only launched if version is positive.
     *
     * @param context the context
     * @param timerRetry timer in milliseconds to trigger next provisioning request. Only applicable
     *            if greater than 0.
     */
    // /* package private */void tryLaunchRcsCoreService(Context context, long timerRetry) {
    //     int version = mRcsSettings.getProvisioningVersion();
    //     // Only launch service if version is positive
    //     if (version > 0) {
    //         // Start the RCS service
    //         //LauncherUtils.launchRcsCoreService(context, mRcsSettings);

    //         if (timerRetry > 0) {
    //             HttpsProvisioningService.startRetryAlarm(context, mRetryIntent, timerRetry);
    //         } else {
    //             retry();
    //         }
    //     } else {
    //         // Only retry provisioning if service is disabled dormant (-3)
    //         if (Version.DISABLED_DORMANT.toInt() == version) {
    //             if (timerRetry > 0) {
    //                 HttpsProvisioningService.startRetryAlarm(context, mRetryIntent, timerRetry);
    //             } else
    //                 retry();
    //         }
    //     }
    // }

    /**
     * Retry after 511 "Network authentication required" procedure
     *
     * @return <code>true</code> if retry is performed, otherwise <code>false</code>
     */
    private boolean retryAfter511Error() {
        if (mRetryAfter511ErrorCount < HttpsProvisioningUtils.RETRY_AFTER_511_ERROR_MAX_COUNT) {
            mRetryAfter511ErrorCount++;
            AcsService.startRetryAlarm(mCtx, mRetryIntent,
                    HttpsProvisioningUtils.RETRY_AFTER_511_ERROR_TIMEOUT);
            if (sLogger.isActivatedDebug()) {
                sLogger.debug("Retry after 511 error (" + mRetryAfter511ErrorCount + "/"
                        + HttpsProvisioningUtils.RETRY_AFTER_511_ERROR_MAX_COUNT
                        + ") provisioning after "
                        + HttpsProvisioningUtils.RETRY_AFTER_511_ERROR_TIMEOUT + "ms");
            }
            return true;
        }
        if (sLogger.isActivated()) {
            sLogger.debug("No more retry after 511 error for provisioning");
        }
        // Reset after 511 counter
        mRetryAfter511ErrorCount = 0;
        return false;
    }

    /**
     * Provisioning fails.
     *
     * @param reason Reason of failure
     */
    public void provisioningFails(int reason) {
        // If wifi is active network access type
        if (NetworkUtils.getNetworkAccessType(mCtx) == NetworkUtils.NETWORK_ACCESS_WIFI) {
            sLogger.debug("provisioningFails reason:" + reason);
        }
    }

    /**
     * Retry procedure
     */
    /* package private */void retry() {
        if (mRetryCount < HttpsProvisioningUtils.RETRY_MAX_COUNT) {
            mRetryCount++;
            int retryDelay;
            if (!RcsSettings.ATT_SUPPORT) {
                retryDelay = HttpsProvisioningUtils.RETRY_BASE_TIMEOUT + 2 * (mRetryCount - 1)
                    * HttpsProvisioningUtils.RETRY_BASE_TIMEOUT;
            } else {
                retryDelay = HttpsProvisioningUtils.RETRY_SCHEME[mRetryCount - 1];
            }

            AcsService.startRetryAlarm(mCtx, mRetryIntent, retryDelay);
            if (sLogger.isActivated()) {
                sLogger.debug("Retry provisioning count: " + mRetryCount);
            }
        } else {
            if (RcsSettings.ATT_SUPPORT) {
                // <LTE-ACS-195>
                mRetryCount++;
                int retryDelay = 24 * 60 * 60 * 1000;
                AcsService.startRetryAlarm(mCtx, mRetryIntent, retryDelay);
                if (sLogger.isActivated()) {
                    sLogger.debug("reach retry max count, start 24h timer to clear token");
                }
                return;
            }

            if (sLogger.isActivated()) {
                sLogger.debug("No more retry for provisioning");
            }
        }
    }

    /**
     * Transmit to SMS unregister method
     */
    public void unregisterSmsProvisioningReceiver() {
        mSmsManager.unregisterSmsProvisioningReceiver();
    }

    /**
     * Transmit to Network unregister method
     */
    // public void unregisterNetworkStateListener() {
    //     mNetworkCnx.unregisterNetworkStateListener();
    // }

    /**
     * Transmit to Network register wifi method
     */
    // public void registerWifiDisablingListener() {
    //     mNetworkCnx.registerWifiDisablingListener();
    // }

    /**
     * Transmit to Network unregister wifi method
     */
    // public void unregisterWifiDisablingListener() {
    //     mNetworkCnx.unregisterWifiDisablingListener();
    // }

    /**
     * Transmit to Network register method
     */
    // public void registerNetworkStateListener() {
    //     mNetworkCnx.registerNetworkStateListener();
    // }

    // public boolean increaseCounters() {
    //     if (sLogger.isActivated()) {
    //         sLogger.debug("increaseCounters, cur mRetryCount:" + mRetryCount);
    //     }

    //     if (mRetryCount < HttpsProvisioningUtils.RETRY_MAX_COUNT) {
    //         mRetryCount++;
    //         return true;
    //     }

    //     return false;
    // }

    public int getRetryCounters() {
        return mRetryCount;
    }

    public boolean isRetryingOrInNoMoreRetryState() {
        return (mRetryCount > 0) ? true : false;
    }

    public boolean isInConfiguringState() {
        return mIsConfiguring;
    }

    /**
     * Retry procedure
     */
    public void resetCounters() {
        // Reset retry alarm counter
        mRetryCount = 0;
        if (sLogger.isActivated()) {
            sLogger.debug("resetCounters Retry provisioning count: " + mRetryCount);
        }
        // Reset after 511 counter
        mRetryAfter511ErrorCount = 0;
    }

    /**
     * Schedule a background task on Handler for execution
     */
    /* package private */
    void scheduleProvisioningOperation(Runnable task) {
        mProvisioningOperationHandler.post(task);
    }

    /**
     * Causes the provisioning handler to quit without processing any more messages in the message
     * queue
     */
    /* package private */void quitProvisioningOperation() {
        mProvisioningOperationHandler.getLooper().quit();
    }

    /**
     * Checks if first provisioning after (re)boot.
     *
     * @return true if first provisioning after (re)boot.
     */
    public boolean isFirstProvisioningAfterBoot() {
        return mFirstProvAfterBoot;
    }

    void tryReleaseNetwork() {
        if (sLogger.isActivatedDebug()) {
            sLogger.debug("tryReleaseNetwork");
        }

        mNetworkCnx.releaseNetwork();
    }

    void updateSimInfo(String imsi, String imei) {
        mImsi = imsi;
        mImei = imei;
        if (sLogger.isActivatedDebug()) {
            sLogger.debug("updateSimInfo");
        }
    }
}
