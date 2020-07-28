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

package com.orangelabs.rcs.provisioning.https;

import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.OutputStreamWriter;
import java.lang.reflect.Method;
import java.net.Authenticator;
import java.net.HttpURLConnection;
import java.net.URI;
import java.net.URL;
import java.net.URLConnection;
import java.net.URISyntaxException;
import java.net.UnknownHostException;
import java.util.List;
import java.util.Map;
import java.util.Random;
import java.io.FileWriter;
import java.io.InterruptedIOException;

import javax.net.ssl.HostnameVerifier;
import javax.net.ssl.HttpsURLConnection;
import javax.net.ssl.SSLSession;
import com.gsma.services.rcs.CommonServiceConfiguration.MessagingMode;
//import com.mediatek.gba.GbaHttpUrlCredential;

import org.apache.http.Header;
import org.apache.http.HttpHost;
import org.apache.http.HttpResponse;
import org.apache.http.HttpVersion;
import org.apache.http.NoHttpResponseException;
import org.apache.http.auth.AuthScope;
import org.apache.http.client.ClientProtocolException;
import org.apache.http.client.CookieStore;
import org.apache.http.client.CredentialsProvider;
import org.apache.http.client.methods.HttpGet;
import org.apache.http.client.params.CookiePolicy;
import org.apache.http.client.params.HttpClientParams;
import org.apache.http.client.protocol.ClientContext;
import org.apache.http.conn.ClientConnectionManager;
import org.apache.http.conn.params.ConnManagerPNames;
import org.apache.http.conn.params.ConnPerRouteBean;
import org.apache.http.conn.params.ConnRoutePNames;
import org.apache.http.conn.scheme.PlainSocketFactory;
import org.apache.http.conn.scheme.Scheme;
import org.apache.http.conn.scheme.SchemeRegistry;
import org.apache.http.impl.client.BasicCookieStore;
import org.apache.http.impl.client.BasicCredentialsProvider;
import org.apache.http.impl.client.DefaultHttpClient;
import org.apache.http.impl.conn.SingleClientConnManager;
import org.apache.http.params.BasicHttpParams;
import org.apache.http.params.HttpParams;
import org.apache.http.params.HttpProtocolParams;
import org.apache.http.protocol.BasicHttpContext;
import org.apache.http.protocol.HttpContext;
import org.apache.http.protocol.ExecutionContext;
import org.apache.http.util.EntityUtils;

import android.app.AlarmManager;
import android.app.PendingIntent;
import android.content.Context;
import android.content.Intent;
import android.net.ConnectivityManager;
import android.net.NetworkInfo;
import android.net.Proxy;
import android.os.Environment;
import android.os.Looper;
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;
import android.text.TextUtils;

import com.orangelabs.rcs.core.ims.protocol.http.HttpGetRequest;
import com.orangelabs.rcs.core.ims.protocol.http.HttpRequest;
import com.orangelabs.rcs.platform.AndroidFactory;
import com.orangelabs.rcs.provider.settings.RcsSettings;
import com.orangelabs.rcs.provider.settings.RcsSettingsData;
import com.orangelabs.rcs.provisioning.ProvisioningFailureReasons;
import com.orangelabs.rcs.provisioning.ProvisioningInfo;
import com.orangelabs.rcs.provisioning.ProvisioningInfo.Version;
import com.orangelabs.rcs.provisioning.ProvisioningParser;
import com.orangelabs.rcs.provisioning.TermsAndConditionsRequest;
import com.orangelabs.rcs.service.LauncherUtils;
import com.orangelabs.rcs.service.StartService;
//import com.orangelabs.rcs.service.api.client.ImsApiIntents;
import com.orangelabs.rcs.utils.HttpUtils;
import com.orangelabs.rcs.utils.NetworkUtils;
import com.orangelabs.rcs.utils.StringUtils;
import com.orangelabs.rcs.utils.logger.Logger;
import android.net.Network;
import android.net.Uri;


import org.apache.http.client.CredentialsProvider;
import org.apache.http.client.HttpRequestRetryHandler;
import org.apache.http.impl.client.BasicCredentialsProvider;
import org.apache.http.auth.AuthScope;
import org.apache.http.cookie.Cookie;
import org.apache.http.cookie.CookieOrigin;
import org.apache.http.cookie.CookieSpec;
import org.apache.http.cookie.CookieSpecFactory;
import org.apache.http.client.params.ClientPNames;
import org.apache.http.params.HttpConnectionParams;
import org.apache.http.impl.cookie.BrowserCompatSpec;


/**
 * Provisioning via network manager
 *
 * @author hlxn7157
 * @author G. LE PESSOT
 * @author Deutsche Telekom AG
 */
public class HttpsProvisioningManager {

    /**
     * First launch flag
     */
    private boolean first = false;

    /**
     * User action flag
     */
    private boolean user = false;
    /**
     * Retry counter
     */
    private int retryCount = 0;

    /**
     * Check if a provisioning request is already pending
     */
    private boolean isPending = false;

    /**
     * The Service Context
     */
    private Context context;

    /**
     * Provisioning SMS manager
     */
    HttpsProvisioningSMS smsManager;

    /**
     * Provisioning Connection manager
     */
    HttpsProvisioningConnection networkConnection;

    /**
     * Provisioning Separate Connection manager
     */
    HttpsProvisioningNetwork separateNetwork;

    /**
     * Retry after 511 "Network authentication required" counter
     */
    private int retryAfter511ErrorCount = 0;

    /**
     * Retry intent
     */
    private PendingIntent retryIntent;

    private String UNKNOWN = "unknown"; // @tct-stack tukun change for IOT BB
                                        // Implementation guidline ID_1_5_1 http
                                        // User-Agent header

    /**
     * retry 511 flag "true then go MSISDN and IMSI based Provisioning"
     */
    private Boolean retryAfter511Flag = false;

    /**
     * The logger
     */
    private Logger logger = Logger.getLogger(this.getClass().getName());

    /**
     * Intent for broadcasting the provisioning status
     */
    final static public String PROVISIONING_INTENT = "orangelabs.rcs.provisioning.status";


    public static final String SHOW_403_NOTIFICATION = "com.orangelabs.rcs.SHOW_403_NOTIFICATION";

    /**
     * is request for update config is being sent
     */
    private static boolean isRequestSent = false;

    /**
     * Retry MSISDN dialog appear count
     */
    private int countMSISDNEntryDialog = 0;

    /**
     * MAX COUNT FOR WHICH THE MSISDN DIALOG CAN BE SHOWN
     */

    final private int MAXCOUNT_MSISDN_ENTRY = 10;

    private Looper mLooper = null;

    private boolean mobileDataOnly = false;

    /**
     * Constructor
     *
     * @param applicationContext
     * @param retryIntent pending intent to update periodically the configuration
     * @param first is provisioning service launch after (re)boot ?
     * @param user is provisioning service launch after user action ?
     */
    public HttpsProvisioningManager(Context applicationContext, final PendingIntent retryIntent,
            boolean first, boolean user) {
        this.context = applicationContext;
        this.retryIntent = retryIntent;
        this.first = first;
        this.user = user;
        this.smsManager = new HttpsProvisioningSMS(this);
        this.networkConnection = new HttpsProvisioningConnection(this);
        this.mLooper = Looper.myLooper();
    }

    /**
     * @return the context
     */
    protected Context getContext() {
        return context;
    }

    /**
     * Connection event
     *
     * @param action Connectivity action
     * @return true if the updateConfig has been done
     */
    protected boolean connectionEvent(String action) {
        if (logger.isActivated()) {
            logger.debug("Connection event -> " + action);
        }

        if (!action.equals(ConnectivityManager.CONNECTIVITY_ACTION))
            return false;
            
        if (!isPending) {
            isPending = true;

            NetworkInfo networkInfo = null;
            // Check received network info
            if (!needSeparateConnection()) {
                if (logger.isActivated()) {
                    logger.debug("connectionEvent -> Separated network not needed");
                }
                networkInfo = networkConnection.getConnectionMngr().getActiveNetworkInfo();
            } else {
                if (logger.isActivated()) {
                    logger.debug("connectionEvent -> Separated network needed, current["
                            + separateNetwork + "]");
                }
                if (separateNetwork == null) {
                    separateNetwork = new HttpsProvisioningNetwork(this);
                }
                long subID = (long)SubscriptionManager.getDefaultDataSubscriptionId();
                try {
                    separateNetwork.acquireNetwork(subID);
                } catch (NetworkException e) {
                    separateNetwork = null;
                    isPending = false;
                    e.printStackTrace();
                    return false;
                }
                networkInfo = separateNetwork.getNetworkInfo();
            }
            if (logger.isActivated()) {
                logger.debug("connectionEvent -> activeNetworkInfo[" + networkInfo + "]");
            }

            if (networkInfo != null && networkInfo.isConnected()) {
                if (mobileDataOnly) {
                    if (ConnectivityManager.isNetworkTypeMobile(networkInfo.getType())) {
                        mobileDataOnly = false;
                    } else {
                        isPending = false;
                        return false;
                    }
                }
                
                if (logger.isActivated()) {
                    logger.debug("Connected to data network");
                }

                unregisterNetworkStateListener();
                Thread t = new Thread() {
                    public void run() {
                        updateConfig();
                    }
                };
                t.start();

                isPending = false;
                return true;
            }
            isPending = false;
        }

        return false;
    }

    private boolean needSeparateConnection() {
        if (RcsSettings.getInstance().supportOP01()) {
            if (logger.isActivated()) {
                logger.debug("Need Separate connection: " + first);
            }
            return first;
        } else {
            if (logger.isActivated()) {
                logger.debug("Need Separate connection: " + "false");
            }
            return false;
        }
    }

    /**
     * Execute an HTTP request
     *
     * @param protocol HTTP protocol
     * @param request HTTP request
     * @return HTTP response
     * @throws URISyntaxException
     * @throws IOException
     * @throws ClientProtocolException
     */
    protected HttpResponse executeRequest(String protocol, String request,
            DefaultHttpClient client, HttpContext localContext) throws URISyntaxException,
            ClientProtocolException, IOException {
        try {
            HttpGet get = new HttpGet();
            get.setURI(new URI(protocol + "://" + request));
            get.addHeader("Accept-Language", HttpsProvisioningUtils.getUserLanguage());
            if (logger.isActivated()) {
                logger.debug("HTTP request: " + get.getURI().toString());
            }

            //if GBA support is requied by the SIM for Config
            /*if(RcsSettings.getInstance().isGBASupported()){

                String nafAddress = RcsSettings.getInstance().getNetworkOperatorAddress();
                // for GBA
                CredentialsProvider credProvider = new BasicCredentialsProvider(); 
                credProvider.setCredentials(new AuthScope(AuthScope.ANY_HOST, AuthScope.ANY_PORT), new GbaCredentials(context, nafAddress));

                client.setCredentialsProvider(credProvider);
                int subId=0;
                subId = SubscriptionManager.getDefaultDataSubscriptionId();
                if (logger.isActivated()) {
                    logger.error("Provisioning manager subId" + subId);
                }
                if(subId == -1){
                    subId = SubscriptionManager.getDefaultSubscriptionId();
                }
                URL url1 = new URL(nafAddress);
                GbaHttpUrlCredential gbaCredential = new GbaHttpUrlCredential(context, url1.getHost(), subId);
                Authenticator.setDefault(gbaCredential.getAuthenticator());

            }*/



            HttpResponse response = client.execute(get, localContext);
            if (logger.isActivated()) {
                logger.debug("HTTP response: " + response.getStatusLine().toString());
            }
            return response;
        } catch (UnknownHostException e) {
            if (logger.isActivated()) {
                logger.debug("The server " + request + " can't be reached!");
            }
            return null;
        }
    }

    /**
     * Get the HTTPS request arguments
     *
     * @param imsi Imsi
     * @param imei Imei
     * @return {@link String} with the HTTPS request arguments.
     */
    protected String getHttpsRequestArguments(String imsi, String imei) {
        return getHttpsRequestArguments(imsi, imei, null, null, null);
    }

    /**
     * Get the HTTPS request arguments
     *
     * @param imsi Imsi
     * @param imei Imei
     * @param smsPort SMS port
     * @param token Provisioning token
     * @param msisdn MSISDN
     * @return {@link String} with the HTTPS request arguments.
     */
    private String getHttpsRequestArguments(String imsi, String imei, String smsPort, String token,
            String msisdn) {
        String vers = RcsSettings.getInstance().getProvisioningVersion();
        if (this.user && ProvisioningInfo.Version.DISABLED_DORMANT.equals(vers)) {
            vers = LauncherUtils.getProvisioningVersion(context);
            this.user = false;
        }

        //String Client_vendor = LauncherUtils.getClient(AndroidFactory.getApplicationContext());
        String Client_vendor = "";
        if (logger.isActivated()) {
            logger.debug("Vendor is " +Client_vendor);
        }
        String Client_version = "";
        if(!RcsSettings.getInstance().isUseDefaultClientVendor()) {
        	Client_vendor = RcsSettings.getInstance().getClientVendor();
    	}

        if (!Client_vendor.equals("")) {
            Client_version = LauncherUtils.getClientVersion(AndroidFactory.getApplicationContext());
        } else { // default cleint vendor and version
            Client_vendor = HttpsProvisioningUtils.getClientVendorFromContext(AndroidFactory
                    .getApplicationContext());
            Client_version = HttpsProvisioningUtils.getClientVersionFromContext(AndroidFactory
                    .getApplicationContext());
        }

               
        if (logger.isActivated()) {
            logger.debug("Vendor is " +Client_vendor);
        }
        String args = "?vers="
                + vers
                + "&rcs_version="
                + HttpsProvisioningUtils.getRcsVersion()
                + "&rcs_profile="
                + HttpsProvisioningUtils.getRcsProfile()
                + "&client_vendor="+Client_vendor
                // +"&client_version="+Client_version
                + "&client_version=RCSAndrd-1.4" + "&terminal_vendor="
                + HttpUtils.encodeURL(HttpsProvisioningUtils.getTerminalVendor())
                + "&terminal_model="
                + HttpUtils.encodeURL(HttpsProvisioningUtils.getTerminalModel())
                + "&terminal_sw_version="
                + HttpUtils.encodeURL(HttpsProvisioningUtils.getTerminalSoftwareVersion());
        if (imsi != null) {
            // Add optional parameter IMSI only if available
            args += "&IMSI=" + imsi;
        }
        if (imei != null) {
            // Add optional parameter IMEI only if available
            args += "&IMEI=" + imei;
        }
        if (smsPort != null) {
            // Add SMS port if available
            args += "&SMS_port=" + smsPort;
        }
        if (msisdn != null) {

            // replacing + sign with %2B as mentioned in GSM Sepcs
            msisdn = msisdn.replace("+", "%2B");
            msisdn = msisdn.replace(" ", "");

            // Add token if available
            args += "&msisdn=" + msisdn;
        }
        if (token != null) {
            // Add token if available
            args += "&token=" + token;
        }
        return args;
    }

    /**
     * Send the first HTTPS request to require the one time password (OTP)
     *
     * @param imsi IMSI
     * @param imei IMEI
     * @param requestUri Request URI
     * @param client Instance of {@link DefaultHttpClient}
     * @param localContext Instance of {@link HttpContext}
     * @return Instance of {@link HttpsProvisioningResult} or null in case of internal
     *         exception
     */
    protected HttpsProvisioningResult sendFirstRequestsToRequireOTP(String imsi, String imei,
            String msisdn, String primaryUri, String secondaryUri, DefaultHttpClient client,
            HttpContext localContext) {
        HttpsProvisioningResult result = new HttpsProvisioningResult();
        try {
            if (logger.isActivated()) {
                logger.debug("HTTP provisioning - Send first HTTPS request to require OTP ;"
                        + " requestCount : " + countMSISDNEntryDialog);
            }

            if (countMSISDNEntryDialog >= MAXCOUNT_MSISDN_ENTRY) {
                if (logger.isActivated()) {
                    logger.debug("HTTP provisioning - "
                            + "countMSISDNEntryDialog >=  MAXCOUNT_MSISDN_ENTRY");
                }
                return null;
            }

            // Generate the SMS port for provisioning
            String smsPortForOTP = HttpsProvisioningSMS.generateSmsPortForProvisioning();

            // is secondary device, then ask for the MSISDN
            if (LauncherUtils.isSecondaryDevice()) {
                if ((msisdn == null) || (msisdn.equals("")))
                    msisdn = RcsSettings.getInstance().getMsisdn();
                // set imsi to null
                imsi = null;
                if ((msisdn == null) || (msisdn.equals(""))) { // if no msisdn is present
                                                               // in the database entry
                                                               // ask for MSISDN
                    msisdn = HttpsProvionningMSISDNInput.getInstance().
                            displayPopupAndWaitResponse(context);
                    countMSISDNEntryDialog++;
                }
                if ((msisdn == null) || (msisdn.equals(""))) {
                    return null;
                }

                // set the smsPortForOTP
                smsPortForOTP = "0";
            }

            // Format first HTTPS request with extra parameters (IMSI and IMEI if
            // available plus SMS_port and token)
            String token = (!TextUtils.isEmpty(RcsSettings.getInstance().getProvisioningToken()) ?
                    RcsSettings.getInstance().getProvisioningToken() : "");
            String args = getHttpsRequestArguments(imsi, imei, smsPortForOTP, token, msisdn);

            // Execute first HTTPS request with extra parameters
            String request = primaryUri + args;
            int retryPeriod=0;
            if(!RcsSettings.getInstance().isGBASupported()){
            HttpResponse response = executeRequest("https", request, client, localContext);
            if (response == null && !StringUtils.isEmpty(secondaryUri)) {
                // First server not available, try the secondaryUri
                request = secondaryUri + args;
                response = executeRequest("http", request, client, localContext);
            }
            if (response == null) {
                return null;
            }

            result.code = response.getStatusLine().getStatusCode();
            result.content = new String(EntityUtils.toByteArray(response.getEntity()), "UTF-8");
               retryPeriod=getRetryAfter(response);
            } else {
                com.orangelabs.rcs.core.ims.protocol.http.HttpResponse response = executeRequestViaGBA("http", request, client, localContext);
                if (response == null && !StringUtils.isEmpty(secondaryUri)) {
                    // First server not available, try the secondaryUri
                    request = secondaryUri + args;
                    response = executeRequestViaGBA("https", request, client, localContext);
                }
                if (response == null) {
                    return null;
                }

                result.code = response.getResponseCode();
                if (response.getContent()!= null) {
                    result.content = new String(response.getContent(), "UTF-8");
                } else {
                    result.content = "";
                }
                //result.content = new String(response.getContent(), "UTF-8");
                retryPeriod=getRetryAfter(response);
            }
            

            if (logger.isActivated()) {
                logger.debug("result from https config request; result.code : " + result.code
                        + "; and result.content : " + result.content);
            }

            if (result.code != 200) {
                if (result.code == 403) {
                    if (logger.isActivated()) {
                        logger.debug("First HTTPS request to require OTP failed: "
                                + "Forbidden (request status code: 403) for msisdn "
                                + msisdn);
                    }

                    /* OP01 has special requirement for 403 */
                    if (RcsSettings.getInstance().supportOP01()) {
                        return result;
                    }
                    if ((msisdn != null) && (!TextUtils.isEmpty(msisdn))) {
                        String msisdnDB = RcsSettings.getInstance().getMsisdn();
                        // if msisdn from database and this msisdn from function is same
                        // then
                        // configuration for this msisdn has already happened before. and
                        // server
                        // is not allowing the msisdn to configure again.
                        if ((msisdnDB != null) && (msisdn.equals(msisdnDB))) {
                            /**
                             * M : added to handle when msisdn is currently unauthorized
                             * and 403 returns
                             *
                             * */
                            if (logger.isActivated()) {
                                logger.debug("msisdn is not empty or null. "
                                        + "msisdn is currently unauthorized and 403 returns");
                            }

                            if (LauncherUtils.isSecondaryDevice()) {
                                HttpsProvionningMSISDNInput.getInstance()
                                        .displayErrorMessageAndWait(context, true);
                            }

                            // if msisdn (read from database )is not null, empty, but
                            // still server return 403, then
                            // this msisdn is not autorized
                            return result;
                            /**@*/
                        } else {
                            // show a error message to the user
                            HttpsProvionningMSISDNInput.getInstance().displayErrorMessageAndWait(
                                    context, false);
                        }
                    }

                    // read from the database
                    msisdn = RcsSettings.getInstance().getMsisdn();

                    // if msisdn is null or is empty ask end user the msisdn
                    if ((msisdn == null) || (msisdn.equals(""))) {
                        countMSISDNEntryDialog++;
                        msisdn = HttpsProvionningMSISDNInput.getInstance()
                                .displayPopupAndWaitResponse(context);
                    }

                    if (msisdn == null) {
                        return null;
                    } else {
                        return sendFirstRequestsToRequireOTP(imsi, imei, msisdn, primaryUri,
                                secondaryUri, client, localContext);
                    }

                } else if (result.code == 503) {
                    if (logger.isActivated()) {
                        logger.debug("First HTTPS request to require OTP failed: "
                                + "Retry After (request status code: 503)");
                    }
                    result.retryAfter = retryPeriod;
                } else if (result.code == 511) {
                    if (logger.isActivated()) {
                        logger.debug("First HTTPS request to require OTP failed: "
                                + "Invalid token (request status code: 511)");
                    }
                    // @tct-stack wuquan add for 511 response according to IOT case
                    // ID_RCS_1_9_3 start.
                    /*
                     * refer to RCD07 RCS5_1 V3.0 page 2.3.3.3.3. + * user case 3 and 4 +
                     * 3. The OTP password is invalid. As a result, the RCS configuration
                     * server replies with an + HTTP 511 NETWORK AUTHENTICATION REQUIRED
                     * error response. It is up to the + RCS client to provide a user
                     * retry mechanism. When retrying, the RCS client shall re- + start
                     * the configuration process from the beginning. + 4. The token is
                     * invalid. As a result, the RCS configuration server replies with an
                     * HTTP 511 + NETWORK AUTHENTICATION REQUIRED error response. It is up
                     * to the RCS client to + provide a user retry mechanism. When
                     * retrying, the RCS client shall re-start the + configuration process
                     * from the beginning. Consequently, if a valid token was previously +
                     * stored, it shall be removed from the device +
                     */
                    // Reset version to "0"
                    RcsSettings.getInstance().setProvisioningVersion(Version.RESETED.toString());
                    // Reset config
                    LauncherUtils.resetRcsConfig(context);
                    return sendFirstRequestsToRequireOTP(imsi, imei, msisdn, primaryUri,
                            secondaryUri, client, localContext);
                }
            } else {
                if (logger.isActivated()) {
                    logger.debug("HTTPS request returns with 200 OK.");
                }

                // Register SMS provisioning receiver
                smsManager.registerSmsProvisioningReceiver(smsPortForOTP, primaryUri, client,
                        localContext);

                // Save the MSISDN
                RcsSettings.getInstance().setMsisdn(msisdn);
                /**
                 * M If the content is empty, means that the configuration XML is not
                 * present and the Token is invalid then we need to wait for the SMS with
                 * OTP
                 */
                if (TextUtils.isEmpty(result.content) || (!result.content.contains("<?xml"))) {

                    if (logger.isActivated()) {
                        logger.debug("waitingForSMSOTP , set  true");
                    }
                    // Wait for SMS OTP
                    result.waitingForSMSOTP = true;
                }

                /**
                 * M Secodnary device support
                 */
                if (LauncherUtils.isSecondaryDevice()) {
                    // Save the MSISDN
                    RcsSettings.getInstance().setMsisdn(msisdn);

                    // if waiting for SMS for OTP, then show the OTP dialog
                    if (result.waitingForSMSOTP) {
                        result.waitingForSMSOTP = false;
                        if (logger.isActivated()) {
                            logger.debug("waitingForSMSOTP , seondary device : show OTP Dialog");
                        }
                        // display the OTP dialog
                        // = //show a error message to the user
                        final String Otp = HttpsProvionningMSISDNInput.getInstance()
                                .displayOTPDialog(context);

                        if (logger.isActivated()) {
                            logger.debug("waitingForSMSOTP , OTP entered " + Otp);
                        }

                        if ((Otp != null)) {
                            final String otpRequestURI = primaryUri;
                            final DefaultHttpClient clientOtp = client;
                            final HttpContext localContextOtp = localContext;

                            Thread t = new Thread() {
                                public void run() {
                                    // update Config with OTP
                                    updateConfigWithOTP(Otp, otpRequestURI, clientOtp,
                                            localContextOtp);
                                }
                            };
                            t.start();
                        } else {
                            if (logger.isActivated()) {
                                logger.debug("waitingForSMSOTP , OTP entered empty");
                            }
                            // Save the MSISDN
                            RcsSettings.getInstance().setMsisdn("");
                        }
                    }

                    return result;
                }

                /**
                 * M wifi provisining
                 */
                // if context doest contains the XML tag.
                // this is a temp hex as server is currently sending back some content for
                // first time
                // which it should not
                /*
                 * if (!result.content.contains("<?xml")) { if (logger.isActivated()) {
                 * logger.warn("result.content doesnt contains XML "); } //needed as
                 * server is sending non xml data nas because of theta SMS OTP register is
                 * being unregistered result.waitingForSMSOTP = true; }
                 */
                /**@*/
            }

            // If not waiting for the sms with OTP
            if (!result.waitingForSMSOTP) {
                if (logger.isActivated()) {
                    logger.warn("not waiitng for SMS. Unregistering SMS receiver ");
                }

                // Unregister SMS provisioning receiver
                smsManager.unregisterSmsProvisioningReceiver();
            }

            return result;
        } catch (UnknownHostException e) {
            if (logger.isActivated()) {
                logger.warn("First HTTPS request to require OTP failed: "
                        + "Provisioning server not reachable");
            }
            return null;
        } catch (Exception e) {
            if (logger.isActivated()) {
                logger.error(
                        "First HTTPS request to require OTP failed: "
                        + "Can't get config via HTTPS", e);
            }
            return null;
        }
    }

    /**
     * Update provisioning config with OTP
     *
     * @param otp One time password
     * @param requestUri Request URI
     * @param client Instance of {@link DefaultHttpClient}
     * @param localContext Instance of {@link HttpContext}
     */
    protected void updateConfigWithOTP(
            String otp, String requestUri, DefaultHttpClient client, HttpContext localContext) {
        // Cancel previous retry alarm
        HttpsProvisioningService.cancelRetryAlarm(context, retryIntent);

        // Get config via HTTPS with OTP
        HttpsProvisioningResult result = sendSecondHttpsRequestWithOTP(otp, requestUri, client,
                localContext);

        // Process HTTPS provisioning result
        processProvisioningResult(result);
    }

    /**
     * Build the provisioning address with SIM information
     *
     * @return provisioning URI
     */
    private String buildProvisioningAddress(TelephonyManager tm) {

        // Get SIM info
        int subId=0;
        subId = SubscriptionManager.getDefaultDataSubscriptionId();
        if (logger.isActivated()) {
            logger.error("Provisioning manager subId" + subId);
        }
        if(subId == -1){
            subId = SubscriptionManager.getDefaultSubscriptionId();
        }
        String ope = tm.getSimOperator(subId);
        String operator_name = RcsSettings.getInstance().getNetworkOperator();
        // if(operator_name.equalsIgnoreCase("CMCC"))
        if (RcsSettings.getInstance().supportOP01())
            return RcsSettings.getInstance().getNetworkOperatorAddress();
        String configType = LauncherUtils.getConfigurationType(context);
        if (ope == null || ope.length() < 4) {
            if (logger.isActivated()) {
                logger.warn("Can not read network operator from SIM card!");
            }
            return null;
        }
        String mnc = ope.substring(3);
        String mcc = ope.substring(0, 3);
        if (configType.equalsIgnoreCase("Prepro")) {
            return "prepro.config." + mcc + mnc + ".rcse.vodafone.es";
        } else {
            while (mnc.length() < 3) { // Set mnc on 3 digits
                mnc = "0" + mnc;
            }
            if (configType.equalsIgnoreCase("Preprod")) {
                return "preprod.config.rcs." + "mnc" + mnc + ".mcc" + mcc + ".pub.3gppnetwork.org";
            } else if (configType.equalsIgnoreCase("Test")) {
                return "testbed.config.rcs." + "mnc" + mnc + ".mcc" + mcc + ".pub.3gppnetwork.org";
            } else {
                return "config.rcs." + "mnc" + mnc + ".mcc" + mcc + ".pub.3gppnetwork.org";
            }
            // return "testbed.config.rcs." + "mnc" + mnc + ".mcc" + mcc +
            // ".pub.3gppnetwork.org";
        }
    }

    /**
     * Get configuration
     *
     * @return Result or null in case of internal exception
     */
    private HttpsProvisioningResult getConfig() {
        HttpsProvisioningResult result = new HttpsProvisioningResult();
        try {
            if (logger.isActivated()) {
                logger.debug("Request config via HTTPS");
            }

            // Get provisioning address
            TelephonyManager tm = (TelephonyManager) context
                    .getSystemService(Context.TELEPHONY_SERVICE);
            String primaryUri = null;
            String secondaryUri = null;

            if (LauncherUtils.isSecondaryDevice()) {
                // uncommnetb for voadfone numbers
                primaryUri = "config.rcs." + "mnc" + "001" + ".mcc" + "214"
                        + ".pub.3gppnetwork.org";
            } else {
                if (RcsSettings.getInstance().isSecondaryProvisioningAddressOnly()) {
                    primaryUri = RcsSettings.getInstance().getSecondaryProvisioningAddress();
                } else {
                    primaryUri = buildProvisioningAddress(tm);
                    secondaryUri = RcsSettings.getInstance().getSecondaryProvisioningAddress();
                }
            }

            // Check if a configuration file for HTTPS provisioning exists
            if (LauncherUtils.getDebugMode(context)) {
                String PROVISIONING_FILE = Environment.getDataDirectory().getPath()
                        + "/joyn_provisioning.txt";
                try {
                    File file = new File(PROVISIONING_FILE);
                    if (file.exists()) {
                        if (logger.isActivated()) {
                            logger.debug("Provisioning file found !");
                        }
                        FileInputStream fis = new FileInputStream(PROVISIONING_FILE);
                        DataInputStream in = new DataInputStream(fis);
                        BufferedReader br = new BufferedReader(new InputStreamReader(in));
                        primaryUri = br.readLine();
                        secondaryUri = null;
                        in.close();
                    }
                } catch (Exception e) {
                    // Nothing to do
                }
            }
            if (logger.isActivated()) {
                logger.debug("HCS/RCS Uri to connect: " + primaryUri + " or " + secondaryUri);
            }

            String imsi = tm.getSubscriberId();
            String imei = tm.getDeviceId();
            tm = null;

            // Format HTTP request
            SchemeRegistry schemeRegistry = new SchemeRegistry();
            schemeRegistry.register(new Scheme("http", PlainSocketFactory.getSocketFactory(), 80));
            schemeRegistry.register(new Scheme("https", new EasySSLSocketFactory(), 443));

            //java.util.logging.Logger.getLogger("org.apache.http.wire").setLevel(java.util.logging.Level.ALL);
            
            HttpParams params = new BasicHttpParams();
            params.setParameter(ConnManagerPNames.MAX_TOTAL_CONNECTIONS, 30);
            params.setParameter(ConnManagerPNames.MAX_CONNECTIONS_PER_ROUTE,
                    new ConnPerRouteBean(30));
            params.setParameter(HttpProtocolParams.USE_EXPECT_CONTINUE, false);
            NetworkInfo networkInfo;
            if (separateNetwork == null) {
                networkInfo = networkConnection.getConnectionMngr().getActiveNetworkInfo();
            } else {
                networkInfo = separateNetwork.getNetworkInfo();
            }

            if (networkInfo != null) {
                String proxyHost = Proxy.getDefaultHost();
                if (proxyHost != null && proxyHost.length() > 1) {
                    int proxyPort = Proxy.getDefaultPort();
                    params.setParameter(
                            ConnRoutePNames.DEFAULT_PROXY, new HttpHost(proxyHost, proxyPort));
                }
            }
            HttpProtocolParams.setVersion(params, HttpVersion.HTTP_1_1);

            // @tct-stack Begin: tukun change for IOT BB Implementation guidline ID_1_5_1
            // http User-Agent header
            String mUserAgent = buildUserAgentHeader();
            HttpProtocolParams.setUserAgent(params, mUserAgent);
            // @tct-stack End: tukun change for IOT BB Implementation guidline ID_1_5_1
            // http User-Agent header
            // Support broad variety of different cookie types (not just Netscape but RFC
            // 2109 and RFC2965 compliant ones, too)
            HttpClientParams.setCookiePolicy(params, CookiePolicy.BROWSER_COMPATIBILITY);

            CookieSpecFactory csf = new CookieSpecFactory() {
                        public CookieSpec newInstance(HttpParams params) {
                                return new BrowserCompatSpec() {                                                        
                                @Override
                                public boolean match(Cookie cookie, CookieOrigin origin) {
                                    return true;
                                }
                            };
                        }
            };

            ClientConnectionManager cm = new SingleClientConnManager(params, schemeRegistry);
            DefaultHttpClient client = new DefaultHttpClient(cm, params);
            CookieStore cookieStore = (CookieStore) new BasicCookieStore();
            HttpContext localContext = new BasicHttpContext();
            localContext.setAttribute(ClientContext.COOKIE_STORE, cookieStore);

            client.getCookieSpecs().register("easy", csf);
            client.getParams().setParameter(ClientPNames.COOKIE_POLICY, "easy");

            if (RcsSettings.getInstance().supportOP01()) {
                //set connection timeout
                HttpConnectionParams.setConnectionTimeout(client.getParams(), 60 * 1000);
                //set rx timeout
                HttpConnectionParams.setSoTimeout(client.getParams(), 60 * 1000);
                //set retry handler when time out
                client.setHttpRequestRetryHandler(new HttpRequestRetryHandler() {
                    private int count = 0;
                    public boolean retryRequest(
                        IOException exception, int executionCount, HttpContext context) {
                        count++;
                        if (count > 3)
                            return false;
                        if (exception == null || context == null)
                            throw new IllegalArgumentException("Parameter may not be null");
                        if (exception instanceof NoHttpResponseException) {
                            // Retry if the server dropped connection on us
                            return true;
                        }
                        if (exception instanceof InterruptedIOException) {
                            // Timeout
                            return true;
                        }
                        Boolean b = (Boolean)
                            context.getAttribute(ExecutionContext.HTTP_REQ_SENT);
                        boolean sent = (b != null && b.booleanValue());
                        if (!sent)
                            return true;
                        return false;
                    }
                });
            }

            // If network is not mobile network, use request with OTP
            if ((networkInfo != null &&
                RcsSettings.getInstance().isWIFI_OTPConfigSupported() &&
                !ConnectivityManager.isNetworkTypeMobile(networkInfo.getType()))
                || (retryAfter511Flag) || (LauncherUtils.isSecondaryDevice())) {
                // Proceed with non mobile network registration
                return sendFirstRequestsToRequireOTP(
                        imsi, imei, null,
                        primaryUri, secondaryUri,
                        client, localContext);
            }

            if (logger.isActivated()) {
                logger.debug("HTTP provisioning on mobile network");
            }

            // Execute first HTTP request
            String requestUri = primaryUri;
            int retryPeriod=0;
            if(!RcsSettings.getInstance().isGBASupported()){
            HttpResponse response = executeRequest("http", requestUri, client, localContext);
            if (logger.isActivated()) {
                logger.debug("HCS/RCS Not GBA: " + requestUri + " or " + secondaryUri);
            } 
            if (response == null && !StringUtils.isEmpty(secondaryUri)) {
                // First server not available, try the secondaryUri
                requestUri = secondaryUri;
                response = executeRequest("http", requestUri, client, localContext);
            }
            if (response == null) {
                return null;
            }

            result.code = response.getStatusLine().getStatusCode();
            result.content = new String(EntityUtils.toByteArray(response.getEntity()), "UTF-8");
             } else {
                 String args1 = getHttpsRequestArguments(imsi, imei);
                 String request1 = requestUri+ args1;
                 if (logger.isActivated()) {
                     logger.debug("HCS/RCS is gba: " + request1 + " or " + secondaryUri);
                 }
                 com.orangelabs.rcs.core.ims.protocol.http.HttpResponse response = executeRequestViaGBA("https", request1, client, localContext);
                 if (logger.isActivated()) {
                     logger.debug("HCS/RCS is gba: " + request1 + " or " + secondaryUri);
                 }
                 if (response == null && !StringUtils.isEmpty(secondaryUri)) {
                     // First server not available, try the secondaryUri
                     request1 = secondaryUri;
                     response = executeRequestViaGBA("https", request1, client, localContext);
                 }
                 if (response == null) {
                     return null;
                 }

                 result.code = response.getResponseCode();
                 if (response.getContent()!= null) {
                     result.content = new String(response.getContent(), "UTF-8");
                 } else {
                     result.content = "";
                 }
                 
             
                /* result.code=200;
                 result.content="";*/
                 if (logger.isActivated()) {
                     logger.debug("By-Pass first HTTP request sends request by HTTPS using arguments params");
                 }
              }
            
            
            if (result.code == 511) {
                // Blackbird guidelines ID_2_6 Configuration mechanism over PS without
                // Header Enrichment
                // Use SMS provisionning on PS data network if server reply 511 NETWORK
                // AUTHENTICATION REQUIRED
                return sendFirstRequestsToRequireOTP(
                        imsi, imei, null, primaryUri, secondaryUri, client, localContext);
            } else if(result.code == 308) {
                
                if (logger.isActivated()) {
                    logger.info("Response Code 308 received proceed further");
                }

                /*if (result.code == 308) {
                    
                    requestUri="config.rcs.mnc260.mcc310.pub.3gppnetwork.org";
                    if (logger.isActivated()) {
                        logger.debug("HCS/RCS is gba: " + requestUri + " or " + secondaryUri);
                    }
                    com.orangelabs.rcs.core.ims.protocol.http.HttpResponse response = executeRequestViaGBA("https", requestUri, client, localContext);
                    if (logger.isActivated()) {
                        logger.debug("HCS/RCS is gba: " + requestUri + " or " + secondaryUri);
                    }
                    if (response == null && !StringUtils.isEmpty(secondaryUri)) {
                        // First server not available, try the secondaryUri
                        requestUri = secondaryUri;
                        response = executeRequestViaGBA("http", requestUri, client, localContext);
                    }
                    if (response == null) {
                        return null;
                    }

                    result.code = response.getResponseCode();
                    if (response.getContent()!= null) {
                        result.content = new String(response.getContent(), "UTF-8");
                    } else {
                        result.content = "";
                    }
            if (result.code == 511) {
                // Blackbird guidelines ID_2_6 Configuration mechanism over PS without
                // Header Enrichment
                // Use SMS provisionning on PS data network if server reply 511 NETWORK
                // AUTHENTICATION REQUIRED
                return sendFirstRequestsToRequireOTP(
                        imsi, imei, null, primaryUri, secondaryUri, client, localContext);
                    }
                    
                    else if (result.code != 200) {
                if (result.code == 503) {
                            result.retryAfter = retryPeriod;
                        }
                        return result;
                    }
                }*/
            } else if (result.code== 307) {
                requestUri="";
            }
            else if (result.code != 200) {
                if (result.code == 503) {
                    result.retryAfter = retryPeriod;
                }
                return result;
            }

            if (result.content.contains("<?xml")) {
                boolean userAcceptTerm = isShowUserMessage(result.content);
                if (!userAcceptTerm) {
                    if(!RcsSettings.getInstance().isGBASupported())
                    result.code = 403;
                    return result;
                }

            }

            // Format second HTTPS request
            String args = getHttpsRequestArguments(imsi, imei);
            String request = requestUri + args;
            if (logger.isActivated()) {
                logger.info("Request provisioning: " + request);
            }

            if(!RcsSettings.getInstance().isGBASupported()){
                HttpResponse response = executeRequest("https", request, client, localContext);
            if (response == null) {
                return null;
            }
            result.code = response.getStatusLine().getStatusCode();
            if (result.code != 200) {
                if (result.code == 503) {
                    result.retryAfter = getRetryAfter(response);
                }
                return result;
            }
            result.content = new String(EntityUtils.toByteArray(response.getEntity()), "UTF-8");
             } else {
                 com.orangelabs.rcs.core.ims.protocol.http.HttpResponse response = executeRequestViaGBA("https", request, client, localContext);
                 if (response == null) {
                     return null;
                 }
                 result.code = response.getResponseCode();
                 if (result.code != 200) {
                     if (result.code == 503) {
                         result.retryAfter = getRetryAfter(response);
                     }
                     return result;
                 }
                 if (response.getContent()!= null) {
                     result.content = new String(response.getContent(), "UTF-8");
                 } else {
                     result.content = "";
                 }
             }
            
            // Execute second HTTPS request
            
            return result;
        } catch (UnknownHostException e) {
            if (logger.isActivated()) {
                logger.warn("Provisioning server not reachable");
            }
            return null;
        } catch (Exception e) {
            if (logger.isActivated()) {
                logger.error("Can't get config via HTTPS", e);
            }
            return null;
        }
    }

    private boolean isShowUserMessage(String content) {

        ProvisioningParser parser = new ProvisioningParser(content);
        if (parser.parseMsg()) {
            ProvisioningInfo info = parser.getProvisioningInfo();
            if (info.getMessage() != null
                    && !RcsSettings.getInstance().isProvisioningTermsAccepted()) {
                return showTermsAndConditionsForResult(info);
            }
        }
        return false;
    }

    /**
     * Update provisioning config
     */
    protected void updateConfig() {
        if (isRequestSent) {
            return;
        }

        isRequestSent = true;

        // Cancel previous retry alarm
        HttpsProvisioningService.cancelRetryAlarm(context, retryIntent);

        // Get config via HTTPS
        HttpsProvisioningResult result = getConfig();

        // Process HTTPS provisioning result
        processProvisioningResult(result);

        isRequestSent = false;
    }

    /**
     * Send the second HTTPS request with the one time password (OTP)
     *
     * @param otp One time password
     * @param requestUri Request URI
     * @param client Instance of {@link DefaultHttpClient}
     * @param localContext Instance of {@link HttpContext}
     * @return Instance of {@link HttpsProvisioningResult} or null in case of internal
     *         exception
     */
    protected HttpsProvisioningResult sendSecondHttpsRequestWithOTP(
            String otp, String requestUri, DefaultHttpClient client, HttpContext localContext) {
        HttpsProvisioningResult result = new HttpsProvisioningResult();
        try {
            if (logger.isActivated()) {
                logger.debug("Send second HTTPS with OTP");
            }

            // Format second HTTPS request
            String args = "?OTP=" + otp;
            String request = requestUri + args;

            if (logger.isActivated()) {
                logger.info("Request provisioning with OTP: " + request);
            }

            // Execute second HTTPS request
            
            if(!RcsSettings.getInstance().isGBASupported()){
            HttpResponse response = executeRequest("https", request, client, localContext);
            if (response == null) {
                return null;
            }
            result.code = response.getStatusLine().getStatusCode();
            if (result.code != 200) {
                if (result.code == 503) {
                    result.retryAfter = getRetryAfter(response);
                } else if (result.code == 511) {
                    if (logger.isActivated()) {
                        logger.debug("Second HTTPS request with OTP failed: "
                                + "Invalid one time password (request status code: 511)");
                    }
                }
                return result;
            }
            result.content = new String(EntityUtils.toByteArray(response.getEntity()), "UTF-8");
             } else {
                 com.orangelabs.rcs.core.ims.protocol.http.HttpResponse response = executeRequestViaGBA("https", request, client, localContext); 
                 if (response == null) {
                     return null;
                 }
                 result.code = response.getResponseCode();
                 if (result.code != 200) {
                     if (result.code == 503) {
                         result.retryAfter = getRetryAfter(response);
                     } else if (result.code == 511) {
                         if (logger.isActivated()) {
                             logger.debug("Second HTTPS request with OTP failed: "
                                     + "Invalid one time password (request status code: 511)");
                         }
                     }
                     return result;
                 }
                 if (response.getContent()!= null) {
                     result.content = new String(response.getContent(), "UTF-8");
                 } else {
                     result.content = "";
                 }
             }
            
            

            return result;
        } catch (Exception e) {
            if (logger.isActivated()) {
                logger.error("Second HTTPS request with OTP failed: "
                        + "Can't get config via HTTPS", e);
            }
            return null;
        }
    }

    /**
     * Get retry-after value
     *
     * @return retry-after value
     */
    protected int getRetryAfter(com.orangelabs.rcs.core.ims.protocol.http.HttpResponse response) {
        String headers = response.getHeader("Retry-After");
        
            try {
                return Integer.parseInt(headers);
            } catch (NumberFormatException e) {
                return 0;
            }
    }
    
    
    /**
     * Get retry-after value
     *
     * @return retry-after value
     */
    protected int getRetryAfter(HttpResponse response) {
        Header[] headers = response.getHeaders("Retry-After");
        if (headers.length > 0) {
            try {
                return Integer.parseInt(headers[0].getValue());
            } catch (NumberFormatException e) {
                return 0;
            }
        }
        return 0;
    }

    /**
     * Process provisioning result
     *
     * @param result Instance of {@link HttpsProvisioningResult}
     */
    private void processProvisioningResult(HttpsProvisioningResult result) {
        boolean separateMobileData = false;
        
        if (separateNetwork != null) {
            separateNetwork.releaseNetwork();
            separateNetwork = null;
            separateMobileData = true;
        }

        if (result != null) {
            if (result.code == 200) {
                // Reset after 511 counter
                retryAfter511ErrorCount = 0;
                retryAfter511Flag = false;

                if (result.waitingForSMSOTP) {
                    if (logger.isActivated()) {
                        logger.debug("Waiting for SMS with OTP.");
                    }
                    return;
                }

                if (logger.isActivated()) {
                    logger.debug("Provisioning request successful");
                }

                // if provisoing logging is enabled
                if (logger.isActivated()) {
                    TelephonyManager tm = (TelephonyManager) context
                            .getSystemService(Context.TELEPHONY_SERVICE);
                    String phone_imsi = "PROVISIONING_";
                    phone_imsi += tm.getSubscriberId();
                    if (phone_imsi.equals("")) {
                        phone_imsi = "joyn_provisioning";
                    }
                    // save provisioning information on the SD card
                    generateProvisioningOnSD(phone_imsi, result.content);

                }// PROVISONING LOG ENDS HERE

                // Parse the received content
                ProvisioningParser parser = new ProvisioningParser(result.content);
                // Save GSMA release set into the provider
                int gsmaRelease = RcsSettings.getInstance().getGsmaRelease();
                
                MessagingMode messagingMode = RcsSettings.getInstance().getMessagingMode();
                // Before parsing the provisioning, the GSMA release is set to
                // Albatros
                RcsSettings.getInstance().setGsmaRelease(RcsSettingsData.VALUE_GSMA_REL_ALBATROS);
                if (parser.parse(gsmaRelease,messagingMode,first)) {
                    // Successfully provisioned, 1st time reg finalized
                    first = false;
                    ProvisioningInfo info = parser.getProvisioningInfo();
                    // Save version
                    String version = info.getVersion();
                    long validity = info.getValidity();
                    if (logger.isActivated()) {
                        logger.debug("Provisioning version=" + version + ", validity=" + validity);
                    }
                    // Save the latest positive version of the configuration
                    LauncherUtils.saveProvisioningVersion(context, version);
                    // Save the validity of the configuration
                    LauncherUtils.saveProvisioningValidity(context, validity);

                    // set provision validty and version in database
                    RcsSettings.getInstance().setProvisionValidity(validity);
                    RcsSettings.getInstance().setProvisioningVersion(version);

                    // Save token
                    String token = info.getToken();
                    long tokenValidity = info.getTokenValidity();
                    if (logger.isActivated())
                        logger.debug("Provisioning Token="+token+", validity="+tokenValidity);

                    // if the token is not null , only then update the token
                    // value
                    if (token != null) {
                        RcsSettings.getInstance().setProvisioningToken(token);
                        RcsSettings.getInstance().setProvisioningTokenValidity(tokenValidity);
                    }

                    // Reset retry alarm counter
                    retryCount = 0;
                    if (ProvisioningInfo.Version.DISABLED_DORMANT.equals(version)) {
                        // -3 : Put RCS client in dormant state
                        if (logger.isActivated())
                            logger.debug("Provisioning: RCS client in dormant state");
                        // Start retry alarm
                        if (validity > 0) {
                            HttpsProvisioningService.startRetryAlarm(context, retryIntent,
                                    validity * 1000);
                        }
                        // Stop the RCS core service. Provisioning is still
                        // running.
                        LauncherUtils.stopRcsCoreService(context);
                    } else if (ProvisioningInfo.Version.DISABLED_NOQUERY.equals(version)) {
                        // -2 : Disable RCS client and stop configuration query
                        if (logger.isActivated())
                            logger.debug("Provisioning: disable RCS client");
                        // Disable and stop RCS service
                        RcsSettings.getInstance().setServiceActivationState(false);
                        LauncherUtils.stopRcsService(context);
                    } else if (ProvisioningInfo.Version.RESETED_NOQUERY.equals(version)) {
                        // -1 Forbidden: reset account + version = 0-1 (doesn't
                        // restart)
                        if (logger.isActivated())
                            logger.debug("Provisioning forbidden: reset account");
                        // Reset config
                        LauncherUtils.resetRcsConfig(context);
                        // Force version to "-1" (resetRcs set version to "0")
                        RcsSettings.getInstance().setProvisioningVersion(version);
                        // Disable the RCS service
                        RcsSettings.getInstance().setServiceActivationState(false);
                    } else if (ProvisioningInfo.Version.RESETED.equals(version)) {
                        if (logger.isActivated())
                            logger.debug("Provisioning forbidden: no account");
                        // Reset config
                        LauncherUtils.resetRcsConfig(context);
                    } else {
                        // Start retry alarm
                        if (validity > 0) {
                            HttpsProvisioningService.startRetryAlarm(context, retryIntent,
                                    validity * 1000);
                        }
                        // Terms request
                        if (info.getMessage() != null
                                && !RcsSettings.getInstance().isProvisioningTermsAccepted()) {
                            showTermsAndConditions(info);
                        }
                        // set first launch as false
                        RcsSettings.getInstance().setFristLaunchState(false);
                        // send notification that provisioning was successful
                        broadcastProvisioningStatus(true, "200 ok - provisioning sucess");
                        RcsSettings.getInstance().setConfigurationState(true);

                        boolean akaFlag = true;
                        if(RcsSettings.getInstance().isSupportOP08()||RcsSettings.getInstance().isSupportOP07() ){
                        if ((RcsSettings.getInstance().getImsAuhtenticationProcedureForMobile()==RcsSettingsData.AKA_AUTHENT)
                        ||akaFlag
                        ) {
                            if (logger.isActivated())
                                logger.debug("RcsSettingsData.AKA_AUTHENT : update RCS_setting fileds  ");
                            TelephonyManager tm = (TelephonyManager) context.getSystemService(
                                    Context.TELEPHONY_SERVICE);
                            
                             String impi = tm.getIsimImpi();
                             String[] impu = tm.getIsimImpu();
                             String domain = tm.getIsimDomain();
                             String MSISDN="";
                             try {
                                 MSISDN = extractUserNamePart(impu[0]);
                             } catch (Exception e) {
                                 MSISDN="";
                                 if (logger.isActivated()) {
                                     logger.error("updateSIMDetailsinDB is MSISDN null");
                                 }
                                 e.printStackTrace();
                             }
                             if(MSISDN == null) {
                                 MSISDN="";
                             }
                             RcsSettings.getInstance().setUserProfileImsUserName_full(impu[0]);
                             RcsSettings.getInstance().setUserProfileImsPrivateId(impi);
                             RcsSettings.getInstance().setUserProfileImsDomain(domain);
                             RcsSettings.getInstance().setUserProfileImsDisplayName(MSISDN);
                             RcsSettings.getInstance().setUserProfileImsUserName(MSISDN);
                             // No need to update, as provisioning is having correct value.
                             //RcsSettings.getInstance().setImConferenceUri("sip:adhoc@msg.pc.t-mobile.com");
                             
                             //xcap
                             RcsSettings.getInstance().setXdmServer("xcap.msg.pc.t-mobile.com");
                             RcsSettings.getInstance().setXdmLogin(impu[0]);
                             RcsSettings.getInstance().writeParameter(RcsSettingsData.XDM_AUTH_TYPE,"GBA");
                             
                             //presence services
                             RcsSettings.getInstance().writeParameter(RcsSettingsData.CAPABILITY_SOCIAL_PRESENCE, Boolean.toString(true));
                             RcsSettings.getInstance().writeParameter(RcsSettingsData.CAPABILITY_PRESENCE_DISCOVERY, Boolean.toString(true));
                             //RcsSettings.getInstance().writeParameter(RcsSettingsData.AUTO_ACCEPT_CHAT,RcsSettingsData.TRUE);
                             //RcsSettings.getInstance().writeParameter(RcsSettingsData.AUTO_ACCEPT_GROUP_CHAT,RcsSettingsData.TRUE);
                           //  RcsSettings.getInstance().writeParameter(RcsSettingsData.PERMANENT_STATE_MODE,RcsSettingsData.TRUE);
                             //RcsSettings.getInstance().setCPMSupported(true);
                             
                         }
                        }

                        // Start the RCS core service
                        LauncherUtils.launchRcsCoreService(context);
                    }
                } else {
                    if (logger.isActivated())
                        logger.debug("Can't parse provisioning document");
                    // Restore GSMA release saved before parsing of the
                    // provisioning
                    RcsSettings.getInstance().setGsmaRelease("" + gsmaRelease);
                    if (first) {
                        if (logger.isActivated())
                            logger.debug("As this is first launch and"
                                    + " we do not have a valid configuration yet, retry later");
                        // Reason: Invalid configuration
                        provisioningFails(ProvisioningFailureReasons.INVALID_CONFIGURATION,
                                "INVALID_CONFIGURATION");
                        retry();
                    } else {
                        if (logger.isActivated())
                            logger.debug("This is not first launch, "
                                    + "use old configuration to register");
                        tryLaunchRcsCoreService(context, -1);
                    }
                }
            } else if (result.code == 503) {
                // Server Unavailable
                if (logger.isActivated())
                    logger.debug("Server Unavailable. Retry after: " + result.retryAfter);
                if (first) {
                    // Reason: Unable to get configuration
                    provisioningFails(
                            ProvisioningFailureReasons.UNABLE_TO_GET_CONFIGURATION,
                            "UNABLE_TO_GET_CONFIGURATION");
                    if (result.retryAfter > 0) {
                        HttpsProvisioningService.startRetryAlarm(
                                context, retryIntent, result.retryAfter * 1000);
                    }
                } else {
                    tryLaunchRcsCoreService(context, result.retryAfter * 1000);
                }
            } else if (result.code == 403) {
                // Forbidden: reset account + version = 0
                if (logger.isActivated())
                    logger.debug("Provisioning forbidden: reset account");
                // Reset version to "0"
                RcsSettings.getInstance().setProvisioningVersion(Version.RESETED.toString());
                // Reset config
                LauncherUtils.resetRcsConfig(context);
                // Register listener to trigger separated network again
                // whenever Mobile Data become activated
                if (RcsSettings.getInstance().supportOP01() &&
                    NetworkUtils.getNetworkAccessType() == NetworkUtils.NETWORK_ACCESS_WIFI &&
                    !separateMobileData) {
                    Intent intent = new Intent(SHOW_403_NOTIFICATION);
                    context.sendBroadcast(intent, "com.gsma.services.permission.RCS");
                    mobileDataOnly = true;
                }
                // Reason: Provisioning forbidden
                provisioningFails(
                        ProvisioningFailureReasons.PROVISIONING_FORBIDDEN,
                        "PROVISIONING_FORBIDDEN");
            } else if (result.code == 511) {
                // Provisioning authentication required
                if (logger.isActivated())
                    logger.debug("Provisioning authentication required");
                // Reset provisioning token
                RcsSettings.getInstance().setProvisioningToken("");
                // Retry after reseting provisioning token
                if (!retryAfter511Error()) {
                    // Reason: Provisioning authentication required
                    provisioningFails(
                            ProvisioningFailureReasons.PROVISIONING_AUTHENTICATION_REQUIRED,
                            "PROVISIONING_AUTHENTICATION_REQUIRED");
                }
            } else if (result.code == 502) {
                // Provisioning authentication required
                if (logger.isActivated())
                    logger.debug("Provisioning authentication required");
                // Reset provisioning token
                RcsSettings.getInstance().setProvisioningToken("");
                // Retry after reseting provisioning token
                // Reason: Provisioning authentication required
                provisioningFails(
                        ProvisioningFailureReasons.PROVISIONING_AUTHENTICATION_REQUIRED,
                        "PROVISIONING_AUTHENTICATION_REQUIRED");
            } else {
                // Other error
                if (logger.isActivated())
                    logger.debug("Provisioning error " + result.code);
                // Start the RCS service
                if (first) {
                    // Reason: No configuration present
                    provisioningFails(
                            ProvisioningFailureReasons.CONNECTIVITY_ISSUE,
                            "CONNECTIVITY_ISSUE");
                    retry();
                } else {
                    tryLaunchRcsCoreService(context, -1);
                }
            }
        } else { // result is null
            // Start the RCS service
            if (first) {
                // Reason: No configuration present
                if (logger.isActivated())
                    logger.error("### Provisioning fails and first = true!");
                provisioningFails(
                        ProvisioningFailureReasons.CONNECTIVITY_ISSUE,
                        "CONNECTIVITY_ISSUE");
                retry();
            } else {
                tryLaunchRcsCoreService(context, -1);
            }
        }
    }

    /**
     * Try to launch RCS Core Service. RCS Service is only launched if version is
     * positive.
     *
     * @param context
     * @param timerRetry timer to trigger next provisioning request. Only applicable if
     *            greater than 0.
     */
    private void tryLaunchRcsCoreService(Context context, int timerRetry) {
        try {
            int version = Integer.parseInt(RcsSettings.getInstance().getProvisioningVersion());

            if (logger.isActivated()) {
                logger.debug("tryLaunchRcsCoreService ; version : " + version);
            }

            // Only launch service if version is positive
            if (version > 0) {
                // Start the RCS service
                LauncherUtils.launchRcsCoreService(context);
                if (timerRetry > 0) {
                    HttpsProvisioningService.startRetryAlarm(context, retryIntent, timerRetry);
                } else
                    retry();
            } else {
                // Only retry provisioning if service is disabled dormant (-3)
                if (ProvisioningInfo.Version.DISABLED_DORMANT.getVersion() == version) {
                    if (timerRetry > 0) {
                        HttpsProvisioningService.startRetryAlarm(context, retryIntent, timerRetry);
                    } else
                        retry();
                }
            }
        } catch (NumberFormatException e) {
        }
    }

    /**
     * Show the terms and conditions request
     *
     * @param info Provisioning info
     */
    private void showTermsAndConditions(ProvisioningInfo info) {
        final Intent intent = new Intent(Intent.ACTION_VIEW);
        intent.setClass(context, TermsAndConditionsRequest.class);

        // Required as the activity is started outside of an Activity context
        intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        intent.addFlags(Intent.FLAG_ACTIVITY_EXCLUDE_FROM_RECENTS);

        // Add intent parameters
        intent.putExtra(TermsAndConditionsRequest.ACCEPT_BTN_KEY, info.getAcceptBtn());
        intent.putExtra(TermsAndConditionsRequest.REJECT_BTN_KEY, info.getRejectBtn());
        intent.putExtra(TermsAndConditionsRequest.TITLE_KEY, info.getTitle());
        intent.putExtra(TermsAndConditionsRequest.MESSAGE_KEY, info.getMessage());

        context.startActivity(intent);
    }

    /**
     * Show the terms and conditions request
     *
     * @param info Provisioning info
     */
    private boolean showTermsAndConditionsForResult(ProvisioningInfo info) {
        return HttpsProvisioningUserMessageDialogue.getInstance().
                displayPopupAndWaitResponse(context, info);
    }

    /**
     * Retry after 511 "Network authentication required" procedure
     *
     * @return <code>true</code> if retry is performed, otherwise <code>false</code>
     */
    private boolean retryAfter511Error() {
        if (retryAfter511ErrorCount < HttpsProvisioningUtils.RETRY_AFTER_511_ERROR_MAX_COUNT) {
            retryAfter511ErrorCount++;

            retryAfter511Flag = true;

            HttpsProvisioningService.startRetryAlarm(context, retryIntent,
                    HttpsProvisioningUtils.RETRY_AFTER_511_ERROR_TIMEOUT);
            if (logger.isActivated()) {
                logger.debug("Retry after 511 error (" + retryAfter511ErrorCount + "/"
                        + HttpsProvisioningUtils.RETRY_AFTER_511_ERROR_MAX_COUNT
                        + ") provisionning after "
                        + HttpsProvisioningUtils.RETRY_AFTER_511_ERROR_TIMEOUT + "ms");
            }
            return true;
        }

        if (logger.isActivated()) {
            logger.debug("No more retry after 511 error for provisionning");
        }

        // Reset after 511 counter
        retryAfter511ErrorCount = 0;
        retryAfter511Flag = false;

        return false;
    }

    /**
     * Provisioning fails.
     *
     * @param reason Reason of failure
     */
    public void provisioningFails(int reason, String Provision_Reason) {
        if (logger.isActivated()) {
            logger.debug("provisioningFails : reason :" + reason);
        }

        // If wifi is active network access type
        if (NetworkUtils.getNetworkAccessType() == NetworkUtils.NETWORK_ACCESS_WIFI) {
            // Register Wifi disabling listener
            networkConnection.registerWifiDisablingListener();
        }

        String failure_reason = "FAILURE REASON : " + Provision_Reason;
        broadcastProvisioningStatus(false, failure_reason);
    }

    /**
     * Retry procedure
     */
    private void retry() {
        if(!RcsSettings.getInstance().isSupportOP08()){    
        if (retryCount < HttpsProvisioningUtils.RETRY_MAX_COUNT) {
            retryCount++;
            int retryDelay = HttpsProvisioningUtils.RETRY_BASE_TIMEOUT + 2 * (retryCount - 1)
                    * HttpsProvisioningUtils.RETRY_BASE_TIMEOUT;
            HttpsProvisioningService.startRetryAlarm(context, retryIntent, retryDelay);
            if (logger.isActivated()) {
                logger.debug("Retry provisionning count: " + retryCount);
            }
        } else {
            if (logger.isActivated()) {
                logger.debug("No more retry for provisionning");
            }

            // broadcast provisoning failure status
            broadcastProvisioningStatus(false, "403 error, max retry finished");
        }
        } else {
            Random random = new Random();
            double w = Math.min(HttpsProvisioningUtils.RETRY_MAX_TIMEOUT, (HttpsProvisioningUtils.RETRY_BASE_TIMEOUT * Math.pow(2, retryCount)));
            double coeff = (random.nextInt(51) + 50) / 100.0; // Coeff between 50% and 100%
            int retryPeriod = (int)(coeff * w);
            HttpsProvisioningService.startRetryAlarm(context, retryIntent, retryPeriod);
            if (logger.isActivated()) {
                logger.debug("Wait " + retryPeriod + "s before retry Configuration (failures=" + retryCount + ", coeff="+ coeff + ")");
            }
        }
    }

    /**
     * Transmit to SMS unregister method
     */
    public void unregisterSmsProvisioningReceiver() {
        smsManager.unregisterSmsProvisioningReceiver();
    }

    /**
     * Transmit to Network unregister method
     */
    public void unregisterNetworkStateListener() {
        networkConnection.unregisterNetworkStateListener();
        networkConnection.unregisterSeparateNetworkStateListener();
    }

    /**
     * Transmit to Network unregister wifi method
     */
    public void unregisterWifiDisablingListener() {
        networkConnection.unregisterWifiDisablingListener();
    }

    /**
     * Transmit to Network register method
     */
    public void registerNetworkStateListener() {
        if (!needSeparateConnection())
            networkConnection.registerNetworkStateListener();
        else
            networkConnection.registerSeparateNetworkStateListener(mLooper);
    }

    /**
     * Retry procedure
     */
    public void resetCounters() {
        // Reset retry alarm counter
        retryCount = 0;

        // Reset after 511 counter
        retryAfter511ErrorCount = 0;

        retryAfter511Flag = false;

        countMSISDNEntryDialog = 0;
    }

    private void broadcastProvisioningStatus(boolean status, String reason) {
        Intent intent = new Intent(PROVISIONING_INTENT);
        intent.putExtra("status", status);
        intent.putExtra("reason", reason);
        context.sendBroadcast(intent, "com.gsma.services.permission.RCS");
    }

    private void generateProvisioningOnSD(String sFileName, String sBody) {
        try {
            if (logger.isActivated()) {
                logger.debug("generateProvisioningOnSD , filename :" + sFileName + "; path :-"
                        + Environment.getDataDirectory() + "/RCS_PROVISIONING");
            }

            File root = new File(Environment.getDataDirectory()
                        + "/RcsDebug", sFileName);

            if (!root.exists()) {
                root.mkdirs();
            }

            sFileName += ".txt";
            File gpxfile = new File(root, sFileName);
            FileWriter writer = new FileWriter(gpxfile);
            writer.append(sBody);
            writer.flush();
            writer.close();
        } catch (IOException e) {
            if (logger.isActivated()) {
                logger.debug("exception in generateProvisioningOnSD , error :"
                        + e.getMessage());
            }
        }
    }

    // @tct-stack Begin: tukun change for IOT BB Implementation guidline ID_1_5_1 http
    // User-Agent header
    /**
     * Build User-Agent string
     *
     * @return String
     */
    public String buildUserAgentHeader() {
        String userAgent = "IM-client/OMA1.0 " + encodeURL(getTerminalVendor()) + "/"
                + encodeURL(getTerminalModel()) + "-" + encodeURL(getTerminalSoftwareVersion())
                + " " + "ALCT/" + "3.2.3" + " " + "Orange-RCS/3.2.3 ";
        if(RcsSettings.getInstance().isGBASupported())
            userAgent += "3gpp-gba";
        return userAgent;
    }

    /**
     * Returns the terminal vendor
     *
     * @return String(4)
     */
    private String getTerminalVendor() {
        String result = UNKNOWN;
        String productmanufacturer = getSystemProperties("ro.product.manufacturer");
        if (productmanufacturer != null && productmanufacturer.length() > 0) {
            result = productmanufacturer;
        }
        return StringUtils.truncate(result, 4);
    }

    /**
     * Returns the terminal model
     *
     * @return String(10)
     */
    private String getTerminalModel() {
        String result = UNKNOWN;
        String devicename = getSystemProperties("ro.product.device");
        if (devicename != null && devicename.length() > 0) {
            result = devicename;
        }
        return StringUtils.truncate(result, 10);
    }

    /**
     * Returns the terminal software version
     *
     * @return String(10)
     */
    private String getTerminalSoftwareVersion() {
        String result = UNKNOWN;
        // String productversion = getSystemProperties("ro.product.version");
        String productversion = getSystemProperties("ro.mediatek.version.release");
        if (productversion != null && productversion.length() > 0) {
            result = productversion;
        }
        return StringUtils.truncate(result, 10);
    }

    /**
     * Returns a system parameter
     *
     * @param key Key parameter
     * @return Parameter value
     */
    private String getSystemProperties(String key) {
        String value = null;
        try {
            Class<?> c = Class.forName("android.os.SystemProperties");
            Method get = c.getMethod("get", String.class);
            value = (String) get.invoke(c, key);
            return value;
        } catch (Exception e) {
            return UNKNOWN;
        }
    }

    /**
     * Encode special characters in URL
     *
     * @param url URL to be encoded
     * @return Encoded URL
     */
    private String encodeURL(String url) {
        return Uri.encode(url);
    }

    // @tct-stack End: tukun change for IOT BB Implementation guidline ID_1_5_1 http
    // User-Agent header

    public void needUpdateConfig() {
        // TODO Auto-generated method stub
        Thread t = new Thread() {
            public void run() {
                updateConfig();
            }
        };
        t.start();

    }

 /**
         * Extract the username part of the SIP-URI
         * 
         * @param uri SIP-URI
         * @return Username
         */
        private static String extractUserNamePart(String uri) {
            if ((uri == null) || (uri.trim().length() == 0)) {
                return "";
            }

            try {
                uri = uri.trim();
                int index1 = uri.indexOf("sip:");
                if (index1 != -1) {
                    int index2 = uri.indexOf("@", index1);
                    String result = uri.substring(index1+4, index2);
                    return result;
                } else {
                    return uri;
                }
            } catch(Exception e) {
                return "";
            }
        }
    
    com.orangelabs.rcs.core.ims.protocol.http.HttpResponse executeHttpConnection(Network network, HttpRequest request)
    {
         if (logger.isActivated()) {
             logger.debug("executeHttpConnection");
         }
         
         com.orangelabs.rcs.core.ims.protocol.http.HttpResponse resp = null;
         URL uri = null;
         HttpURLConnection con = null;
         String serverAddress = null; //"http://"+xdmServerAddr;
         try{
            serverAddress = request.getUrl();
            
             uri = new URL(serverAddress);
            // con = (HttpURLConnection) uri.openConnection();
             con = (HttpURLConnection)network.openConnection(uri);
             if (logger.isActivated()) {
                 logger.info("URI to request "+ uri);
             }
             String method = request.getMethod().toUpperCase();
             if (logger.isActivated()) {
                 logger.info("Method to request "+ method);
             }
             //con.setRequestMethod(method); //type: POST, PUT, DELETE, GET
             //con.setDoInput(true);
             con.setConnectTimeout(60000); //60 secs
             con.setReadTimeout(60000); //60 secs
          
            //add request header
             if (logger.isActivated()) {
                 logger.info("User Agent "+ buildUserAgentHeader());
             }
            con.setRequestProperty("User-Agent",buildUserAgentHeader());
            
            
            
            //if there is some content 
            if(request.getContent()!=null)
            {
               con.setDoOutput(true);
                
               //CONTENT TYPE
               con.setRequestProperty("Content-Type", request.getContentType());
               DataOutputStream wr = new DataOutputStream(con.getOutputStream());
               BufferedWriter writer = new BufferedWriter(new OutputStreamWriter(wr, "UTF-8"));
               writer.write(request.getContent());
               writer.close();
            
            }
            
            
            //read for response
            int responseCode =  con.getResponseCode();  
            
            resp = convertHTTPUrlResponseToClientFormat(con);
        
    

         }catch(Exception e){
             logger.info( "connection error : " + e.getMessage());
             e.printStackTrace();
             return null;
         }finally{
             if(con!=null)
             con.disconnect();
         }
        

         return resp;
    }
    
    com.orangelabs.rcs.core.ims.protocol.http.HttpResponse executeHttpsConnection(Network network, HttpRequest request)
    {
         if (logger.isActivated()) {
             logger.debug("executeHttpsConnection");
         }
         
         com.orangelabs.rcs.core.ims.protocol.http.HttpResponse resp = null;
         URL uri = null;
         HttpsURLConnection con = null;
         String serverAddress = null; //"http://"+xdmServerAddr;
         try{
            serverAddress = request.getUrl();
             uri = new URL(serverAddress);
            // con = (HttpURLConnection) uri.openConnection();
             con = (HttpsURLConnection)network.openConnection(uri);
             // security risk to use Null HostnameVarifier
             // HostnameVerifier hostName = new NullHostNameVerifier();
             // HttpsURLConnection.setDefaultHostnameVerifier(hostName);
             try {
                 con.setSSLSocketFactory((new EasySSLSocketFactory()).getSSLContext().getSocketFactory());
             } catch(Exception e) {
                 if (logger.isActivated()) {
                     logger.error("Failed to initiate SSL for connection:", e);
                 }
             }
             String method = request.getMethod().toUpperCase();
             con.setRequestMethod(method); //type: POST, PUT, DELETE, GET
             //con.setDoInput(true);
             con.setConnectTimeout(60000); //60 secs
             con.setReadTimeout(60000); //60 secs
          
            //add request header
             if (logger.isActivated()) {
                 logger.info("User Agent "+buildUserAgentHeader());
             }
            con.setRequestProperty("User-Agent", buildUserAgentHeader());
            
            
            
            //if there is some content 
            if(request.getContent()!=null)
            {
               con.setDoOutput(true);
                
               //CONTENT TYPE
               con.setRequestProperty("Content-Type", request.getContentType());
               DataOutputStream wr = new DataOutputStream(con.getOutputStream());
               BufferedWriter writer = new BufferedWriter(new OutputStreamWriter(wr, "UTF-8"));
               writer.write(request.getContent());
               writer.close();
            
            }
            
            
            //read for response
            int responseCode =  con.getResponseCode();  
            
            resp = convertHTTPsUrlResponseToClientFormat(con);
        

            
         }catch(Exception e){
             logger.info( " connection error : " + e.getMessage());
             e.printStackTrace();
             return null;
         }finally{
             if(con!=null)
             con.disconnect();
         }
        

         return resp;
    }
    
    private com.orangelabs.rcs.core.ims.protocol.http.HttpResponse convertHTTPUrlResponseToClientFormat(HttpURLConnection conn){
        com.orangelabs.rcs.core.ims.protocol.http.HttpResponse xdmResponse = null;
        
        if(conn != null)
        {
            try{
                
                
                xdmResponse = new com.orangelabs.rcs.core.ims.protocol.http.HttpResponse();
                //set the status 
                xdmResponse.setStatusLine("HTTP/1.1 "+conn.getResponseCode() + " " +conn.getResponseMessage());                 
                //set the headers
                Map<String, List<String>> headerList =  conn.getHeaderFields();
                
           
                if((headerList.get("etag") != null) && (headerList.get("etag").get(0) != null)){
                 String etag = headerList.get("etag").get(0);
                 xdmResponse.addHeader("etag",etag);
                }
                
                
                
                /*
                 * set the content
                 */
                
                //read the content
               BufferedReader in = new BufferedReader(new InputStreamReader(conn.getInputStream() ,"UTF-8"));
               String inputLine;
               StringBuffer responseBuffer = new StringBuffer();
   
               while ((inputLine = in.readLine()) != null) {
                   responseBuffer.append(inputLine);
               }
               in.close();
               
               if(responseBuffer.length() > 0)
               {
                    xdmResponse.setContent(responseBuffer.toString().getBytes());
                    if (logger.isActivated()) {
                        logger.error("content for provisioning request:- "+ responseBuffer.toString().getBytes());
                    }
                   
               }
               
               
           }
            catch(Exception err)
            {
                logger.info( "exception  conversion " + err.getMessage());
                err.printStackTrace();
                return null;
            }  
        }
        return xdmResponse;
    }
    
    private com.orangelabs.rcs.core.ims.protocol.http.HttpResponse convertHTTPsUrlResponseToClientFormat(HttpsURLConnection conn){
        com.orangelabs.rcs.core.ims.protocol.http.HttpResponse xdmResponse = null;
        
        if (logger.isActivated()) {
            logger.debug("ParseHttpsConnectionResponse");
        }
        
        if(conn != null)
        {
            try{
                
                //conn.get
                xdmResponse = new com.orangelabs.rcs.core.ims.protocol.http.HttpResponse();
                
                
                //set the status 
                xdmResponse.setStatusLine("HTTP/1.1 "+conn.getResponseCode() + " " +conn.getResponseMessage());
                if (logger.isActivated()) {
                    logger.debug("Response code:"+conn.getResponseCode() +" Response Content:-" + conn.getContent() +" Response" +conn.getInputStream());
                }
                //set the headers
                Map<String, List<String>> headerList =  conn.getHeaderFields();
                
          

                if((headerList.get("etag") != null) && (headerList.get("etag").get(0) != null)){
                 String etag = headerList.get("etag").get(0);
                 xdmResponse.addHeader("etag",etag);
                }
                
                
                
                /*
                 * set the content
                 */
                
                //read the content
               BufferedReader in = new BufferedReader(new InputStreamReader(conn.getInputStream() ,"UTF-8"));
               String inputLine;
               StringBuffer responseBuffer = new StringBuffer();
   
               while ((inputLine = in.readLine()) != null) {
                   if (logger.isActivated()) {
                       logger.debug(" Response" +inputLine);
                   }
                   responseBuffer.append(inputLine);
               }
               in.close();
               
               if(responseBuffer.length() > 0)
               {
                    xdmResponse.setContent(responseBuffer.toString().getBytes());
                    if (logger.isActivated()) {
                        logger.error("content for provisioning request:- "+ responseBuffer.toString().getBytes());
                    }
                   
               }
               
               
           }
            catch(Exception err)
            {
                logger.info( "exception conversion " + err.getMessage());
                err.printStackTrace();
                return null;
            }  
        }
        return xdmResponse;
    }
        
    protected com.orangelabs.rcs.core.ims.protocol.http.HttpResponse executeRequestViaGBA(String protocol, String request,
            DefaultHttpClient client, HttpContext localContext) throws URISyntaxException,
            ClientProtocolException, IOException {
        try {
            if (logger.isActivated()) {
                logger.error("executeRequestViaGBA ");
            }
            
            String nafAddress = RcsSettings.getInstance().getNetworkOperatorAddress();

            if (logger.isActivated()) {
                logger.error("nafAddress for provisioning :  "+nafAddress);
            }

            int subId=0;
            subId=(int)SubscriptionManager.getDefaultDataSubscriptionId();
            if (logger.isActivated()) {
                logger.error("Provisioning manager subId " + subId);
            }
            /*if(subId == -1){
                subId=(int)SubscriptionManager.getDefaultSubId();
            }*/
            URL url1 = new URL("http://"+nafAddress);    
            HttpGetRequest request_http = new HttpGetRequest(protocol + "://" + request);
            Network activeNetwork =  null;
            if (!needSeparateConnection()) {
                activeNetwork = networkConnection.getConnectionMngr().getActiveNetwork();
                if (logger.isActivated())
                    logger.debug("network query" + activeNetwork);
            } else {
                activeNetwork = separateNetwork.getNetwork();
                if (logger.isActivated())
                    logger.debug("Seperate network query" + activeNetwork);
            }
            /*GbaHttpUrlCredential gbaCredential = new GbaHttpUrlCredential(context, url1.getHost(), subId);
            if (logger.isActivated())
                logger.debug("GbaHTTPCreadintial"+ gbaCredential);
            gbaCredential.setNetwork(activeNetwork);
            Authenticator.setDefault(gbaCredential.getAuthenticator());*/
            com.orangelabs.rcs.core.ims.protocol.http.HttpResponse response_gba= null;
            if (logger.isActivated())
                logger.debug("GbaHTTPCreadintial Set Authenticator");
            if(protocol.equals("http")){
                if (logger.isActivated())
                    logger.debug("For HTTP");
                response_gba = executeHttpConnection(activeNetwork,request_http);
            } else {
                if (logger.isActivated())
                    logger.debug("For HTTPS");
                response_gba = executeHttpsConnection(activeNetwork,request_http);
            }
            
            if (logger.isActivated()) {
                logger.debug("HTTP response111: " + response_gba.getStatusLine().toString());
            }
            return response_gba;

        } catch (Exception e) {
            if (logger.isActivated()) {
                logger.debug("The server " + request + " can't be reached!");
            }
            e.printStackTrace();
            return null;
        }
    }

    // security risk to use Null HostnameVarifier
    /**
     * Blank host verifier
     */
}
