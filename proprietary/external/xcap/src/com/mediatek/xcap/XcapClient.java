/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

package com.mediatek.xcap.client;

import android.content.Context;
import android.net.Network;
import android.os.Build;
import android.os.IBinder;
import android.os.ServiceManager;
import android.os.SystemProperties;
import android.telephony.SubscriptionManager;
import android.text.TextUtils;
import android.util.Base64;
import android.util.Log;

import com.mediatek.gba.GbaManager;
import com.mediatek.gba.NafSessionKey;
import com.mediatek.internal.telephony.MtkSubscriptionManager;
import com.mediatek.xcap.auth.AkaDigestAuth;
import com.mediatek.xcap.header.WwwAuthHeader;

import java.io.BufferedOutputStream;
import java.io.IOException;
import java.io.OutputStream;
import java.io.UnsupportedEncodingException;
import java.net.HttpURLConnection;
import java.net.InetAddress;
import java.net.MalformedURLException;
import java.net.ProtocolException;
import java.net.Socket;
import java.net.SocketTimeoutException;
import java.net.URI;
import java.net.URL;
import java.net.UnknownHostException;
import java.security.GeneralSecurityException;
import java.security.KeyManagementException;
import java.security.KeyStore;
import java.security.KeyStoreException;
import java.security.NoSuchAlgorithmException;
import java.security.UnrecoverableKeyException;
import java.security.cert.CertificateException;

import java.util.Arrays;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Set;

//OkHttpClient common
import com.android.okhttp.CertificatePinner;
import com.android.okhttp.Dns;
import com.android.okhttp.Headers;
import com.android.okhttp.OkHttpClient;
import com.android.okhttp.Request;
import com.android.okhttp.Response;
import com.android.okhttp.Interceptor;

//additonal needs for POST
import com.android.okhttp.MediaType;
import com.android.okhttp.RequestBody;

//additional for SSL
import javax.net.ssl.HostnameVerifier;
import javax.net.ssl.HttpsURLConnection;
import javax.net.ssl.SSLContext;
import javax.net.ssl.SSLSession;
import javax.net.ssl.SSLSocketFactory;
import javax.net.ssl.TrustManager;
import javax.net.ssl.X509TrustManager;
import java.security.cert.Certificate;
import java.security.cert.X509Certificate;
import java.security.cert.CertificateException;
import java.util.concurrent.TimeUnit;

/**
 * XcapClient class.
 */
public class XcapClient {
    private static final String TAG = "XcapClient";

    // Sensitive log task
    private static final String PROP_FORCE_DEBUG_KEY = "persist.vendor.log.tag.tel_dbg";
    private static final boolean SENLOG = TextUtils.equals(Build.TYPE, "user");
    private static final boolean TELDBG = (SystemProperties.getInt(PROP_FORCE_DEBUG_KEY, 0) == 1);

    private static OkHttpClient mOkHttpClient = new OkHttpClient();
    private Request mRequest;
    private Response mResponse;

    private GbaManager mGbaManager;
    private static Map<Integer, String> mNafFqdnCache = new HashMap<Integer, String>();
    private String mUserAgent;

    // Default connection and socket timeout of 60 seconds. Tweak to taste.
    private static final int SOCKET_OPERATION_TIMEOUT = 60;
    private static final int SOCKET_READ_OPERATION_TIMEOUT = 60;
    private static final String AUTH_HDR = "WWW-Authenticate";

    private XcapDebugParam mDebugParam = XcapDebugParam.getInstance();
    private Network mNetwork;
    private Context mContext;
    private int     mPhoneId;
    private static int mRequestCount = 0;

    // Create a trust manager that does not validate certificate chains
    private TrustManager[] mTrustAllCerts = new TrustManager[] {new X509TrustManager() {
        @Override
        public void checkClientTrusted(java.security.cert.X509Certificate[] certs,
                String authType) {
        }

        @Override
        public void checkServerTrusted(java.security.cert.X509Certificate[] certs,
                String authType) {
        }

        @Override
        public java.security.cert.X509Certificate[] getAcceptedIssuers() {
                return new X509Certificate[0];
        }
    }};

    // Create all-trusting host name verifier
    private HostnameVerifier mHostnameVerifier = new HostnameVerifier() {
        @Override
        public boolean verify(String hostname, SSLSession session) {
            return true;
        }
    };

    public static final String METHOD_PUT = "PUT";
    public static final String METHOD_GET = "GET";
    public static final String METHOD_DELETE = "DELETE";

    final protected static char[] hexArray = "0123456789abcdef".toCharArray();

    private static final String[] PROPERTY_RIL_TEST_SIM = {
        "vendor.gsm.sim.ril.testsim",
        "vendor.gsm.sim.ril.testsim.2",
        "vendor.gsm.sim.ril.testsim.3",
        "vendor.gsm.sim.ril.testsim.4",
    };


    /**
     * Constructor.
     *
     */
    public XcapClient(Context context, int phoneId) {
        mContext = context;
        mPhoneId = phoneId;
        composeUserAgent();
        Log.i(TAG , "XcapClient context: " + context + " phoneId:" + phoneId);
        initialize();
    }

    /**
     * Constructor.
     *
     * @param userAgent XCAP client User Agent
     * @param phoneId   to indicate the command from which SIM
     */
    public XcapClient(Context context, String userAgent, int phoneId) {
        mContext = context;
        mPhoneId = phoneId;
        mUserAgent = userAgent;
        Log.i(TAG , "XcapClient context: " + context + " phoneId:" + phoneId);
        initialize();
    }

    /**
     * Constructor.
     *
     * @param network dedicated network
     * @param phoneId to indicate the command from which SIm
     */
    public XcapClient(Context context, Network network, int phoneId) {
        mContext = context;
        mPhoneId = phoneId;
        composeUserAgent();
        Log.i(TAG , "XcapClient context: " + context + " phoneId:" + phoneId);
        if (network != null) {
            mNetwork = network;
        }
        initialize();
    }

    /**
     * Constructor.
     *
     * @param userAgent XCAP client User Agent
     * @param network dedicated network
     * @param phoneId to indicate the command from which SIM
     */
    public XcapClient(Context context, String userAgent, Network network, int phoneId) {
        mContext = context;
        mPhoneId = phoneId;
        mUserAgent = userAgent;
        Log.i(TAG , "XcapClient context: " + context + " phoneId:" + phoneId);

        if (network != null) {
            mNetwork = network;
        }
        initialize();
    }

    private void composeUserAgent() {
        boolean isGbaEnabled = false;
        IBinder b = ServiceManager.getService("GbaService");
        if (b != null) {
            Log.i(TAG , "GbaService Enabled");
            isGbaEnabled = true;
        }

        if (mDebugParam.getXcapUserAgent() != null && !mDebugParam.getXcapUserAgent().isEmpty()) {
            mUserAgent = mDebugParam.getXcapUserAgent();
        } else {
            mUserAgent = "XCAP Client" + (isGbaEnabled ? " 3gpp-gba" : "");
        }
    }

    private void initialize() {
        mGbaManager = GbaManager.getDefaultGbaManager(mContext);
        mDebugParam.load();
    }

    /**
     * Close OkHttpClient, shutdown the connection and clean cache.
     */
    public void shutdown() {
        // try {
        //     if (mOkHttpClient != null) {
        //         mOkHttpClient.getDispatcher().getExecutorService().shutdown();
        //         mOkHttpClient.getConnectionPool().evictAll();
        //         mOkHttpClient.getCache().close();
        //     }
        // } catch (IOException e) {
        //     e.printStackTrace();
        // }
    }

    private Request addExtraHeaders(Request request, Headers rawHeaders) {
        if (rawHeaders == null) {
            return request;
        }

        Set<String> names = rawHeaders.names();
        for (String name : names) {
            List<String> values = rawHeaders.values(name);
            for (String value : values) {
                if (!name.isEmpty() && !value.isEmpty()) {
                    // Add the header if the param is valid
                    Log.d(TAG, "name: " + name + ", value: " + encryptString(value));
                    request = request.newBuilder().addHeader(name, value).build();
                    break;
                }
            }
        }
        return request;
    }

    private void logRequestHeaders(Request request) {
        Map<String, List<String>> headerFields = request.headers().toMultimap();

        Log.d(TAG, "Request Headers:");

        for (Map.Entry<String, List<String>> entry : headerFields.entrySet()) {
            final String key = entry.getKey();
            final List<String> values = entry.getValue();
            if (values != null) {
                for (String value : values) {
                    if (!SENLOG) {
                        Log.d(TAG, key + ": " + encryptString(value));
                    }
                }
            }
        }
    }

    private void logResponseHeaders(Response response) {
        Map<String, List<String>> headerFields = response.headers().toMultimap();

        Log.d(TAG, "Response Headers:");

        for (Map.Entry<String, List<String>> entry : headerFields.entrySet()) {
            final String key = entry.getKey();
            final List<String> values = entry.getValue();
            if (values != null) {
                for (String value : values) {
                    if (!SENLOG) {
                        Log.d(TAG, key + ": " + value);
                    }
                }
            }
        }
    }

    private byte[] getNafSecureProtocolId(boolean isTlsEnabled, String cipher) {
        Log.d(TAG, "getNafSecureProtocolId: protocol=" + (isTlsEnabled ? "https" : "http")
                + ", isTlsEnabled=" + isTlsEnabled + ", cipher = " + cipher);
        return mGbaManager.getNafSecureProtocolId(isTlsEnabled, cipher);
    }

    private void handle401Exception(Response response, boolean isHttps, String cipher) throws IOException {
        WwwAuthHeader wwwAuthHeader = null;
        String auth_header = response.header(AUTH_HDR);

        mRequestCount = 1;

        if (auth_header != null) {
            wwwAuthHeader = WwwAuthHeader.parse(auth_header);
            Log.d(TAG, "handle401Exception: wwwAuthHeader=" + wwwAuthHeader);
        } else {
            Log.e(TAG, "handle401Exception: authentication header has something wrong");
            return;
        }
        byte[] uaId = getNafSecureProtocolId(isHttps, cipher);

        if (!SENLOG) {
            for (int j = 0; j < uaId.length; j++) {
                Log.d(TAG, "uaId[" + j + "] = "+ String.format("0x%02x", uaId[j]));
            }
        }

        String nafFqdn = null;
        String realm = wwwAuthHeader.getRealm();
        if (realm.length() > 0) {
            String[] segments = realm.split(";");
            nafFqdn = segments[0].substring(segments[0].indexOf("@") + 1);
            Log.d(TAG, "handle401Exception: nafFqdn=" + nafFqdn + ", mPhoneId=" + mPhoneId);
            mNafFqdnCache.put(mPhoneId, nafFqdn);
        } else {
            Log.e(TAG, "handle401Exception: realm is empty string !!!");
            return;
        }
        int subId = MtkSubscriptionManager.getSubIdUsingPhoneId(mPhoneId);
        NafSessionKey nafSessionKey = mGbaManager.runGbaAuthentication(nafFqdn, uaId,
                true, subId);
        if (nafSessionKey == null || nafSessionKey.getKey() == null) {
            Log.e(TAG, "handle401Exception: nafSessionKey Error!");
            if (nafSessionKey != null && (nafSessionKey.getException() != null) &&
                        (nafSessionKey.getException() instanceof IllegalStateException)) {
                String msg = ((IllegalStateException) nafSessionKey.getException())
                        .getMessage();

                if ("HTTP 403 Forbidden".equals(msg)) {
                    Log.i(TAG, "GBA hit 403");
                    throw new IOException("HTTP 403 Forbidden");
                } else if ("HTTP 400 Bad Request".equals(msg)) {
                    Log.i(TAG, "GBA hit 400");
                    throw new IOException("HTTP 400 Bad Request");
                }
            }
            return;
        } else {
            nafSessionKey.setAuthHeader(auth_header);
            mGbaManager.updateCachedKey(nafFqdn, uaId, subId, nafSessionKey);
            Log.d(TAG, "handle401Exception: nafSessionKey=" + nafSessionKey);
        }
    }

    /**
     * Utility function to convert byte array to hex string.
     *
     * @param bytes the byte array value.
     * @return the hex string value.
     *
     */
    private String bytesToHex(byte[] bytes) {
        char[] hexChars = new char[bytes.length * 2];

        for (int j = 0; j < bytes.length; j++) {
            int v = bytes[j] & 0xFF;
            hexChars[j * 2] = hexArray[v >>> 4];
            hexChars[j * 2 + 1] = hexArray[v & 0x0F];
        }

        return new String(hexChars);
    }

    private AkaDigestAuth getAkaDigestAuth(Request request, String method, String content) {
        NafSessionKey nafSessionKey = null;
        WwwAuthHeader wwwAuthHeader = null;
        String nafFqdn = mNafFqdnCache.get(mPhoneId);
        byte[] uaId = getNafSecureProtocolId(request.isHttps(), "");
        if (!SENLOG) {
            for (int j = 0; j < uaId.length; j++) {
                Log.d(TAG, "uaId[" + j + "] = "+ String.format("0x%02x", uaId[j]));
            }
        }
        int subId = MtkSubscriptionManager.getSubIdUsingPhoneId(mPhoneId);

        //check NVIOT or PCT test
        if (("1".equals(SystemProperties.get(PROPERTY_RIL_TEST_SIM[mPhoneId], "0"))) &&
                    mRequestCount > 1) {
            Log.d(TAG, "getAkaDigestAuth: force to run gba");
            return null;
        }

        if (nafFqdn != null) {
            nafSessionKey = mGbaManager.getCachedKey(nafFqdn, uaId, subId);
            Log.d(TAG, "getAkaDigestAuth: nafFqdn=" + nafFqdn + ", mPhoneId=" + mPhoneId
                    + "nafSessionKey=" + nafSessionKey);
            if (nafSessionKey != null && nafSessionKey.getAuthHeader() != null) {
                wwwAuthHeader = WwwAuthHeader.parse(nafSessionKey.getAuthHeader());
                Log.d(TAG, "getAkaDigestAuth: wwwAuthHeader=" + wwwAuthHeader);
            }
        }

        if (nafSessionKey == null || wwwAuthHeader == null) {
            return null;
        }

        String password = bytesToHex(
                Base64.encode(nafSessionKey.getKey(), Base64.NO_WRAP));
        String nc = String.format("%08x", mRequestCount);
        Log.d(TAG, "getAkaDigestAuth: password=" + password + ", nc=" + nc
                + ", url=" + ((!SENLOG) ? request.url() : "[hidden]"));
        return new AkaDigestAuth(wwwAuthHeader, nafSessionKey.getBtid(), null, password,
                request.url().getPath(), nc, method, content);
    }

    private Response execute(URL url, String method, byte[] xml,
            Headers additionalRequestHeaders, String mimetype) throws IOException {
        int tryCount = 3;
        boolean success = false;
        boolean isTrustAll = mDebugParam.getEnableXcapTrustAll();

        // Prepare OkHttpClient instance and Interceptor
        if (mOkHttpClient.networkInterceptors().size() == 0) {
            Log.d(TAG, "mOkHttpClient add RequestInterceptor");
            mOkHttpClient.networkInterceptors().add((new RequestInterceptor()));
        }

        // Setup timeout
        mOkHttpClient.setConnectTimeout(SOCKET_OPERATION_TIMEOUT, TimeUnit.SECONDS);
        mOkHttpClient.setReadTimeout(SOCKET_READ_OPERATION_TIMEOUT, TimeUnit.SECONDS);

        // Specify dedicated XCAP network if it exists
        if (mNetwork != null) {
            mOkHttpClient.setSocketFactory(mNetwork.getSocketFactory());
            Log.d(TAG, "mOkHttpClient using dedicated network = " + mNetwork);
            NetworkDns.getInstance().setNetwork(mNetwork);
            mOkHttpClient.setDns(NetworkDns.getInstance());
        }

        // If trust all connections then install trust manager and host verifier
        if (isTrustAll) {
            try {
                final SSLContext sc = SSLContext.getInstance("SSL");
                sc.init(null, mTrustAllCerts, new java.security.SecureRandom());

                // Create an ssl socket factory with our all-trusting manager
                final SSLSocketFactory sslSocketFactory = sc.getSocketFactory();
                mOkHttpClient.setSslSocketFactory(sslSocketFactory);
                mOkHttpClient.setHostnameVerifier(mHostnameVerifier);
                Log.d(TAG, "mOkHttpClient set SSL");
            } catch (GeneralSecurityException se) {
                Log.e(TAG, "Execute TrustAll exception");
                se.printStackTrace();
            }
        }

        Log.d(TAG, "mOkHttpClient = " + mOkHttpClient);

        // Prepare the HTTP request and execute it
        mRequestCount += 1;
        while (tryCount > 0 && !success) {
            try {
                Log.d(TAG, method + " :" + encryptString(url.toString()));

                mRequest = new Request.Builder()
                        .url(url)
                        .header("Accept", "*/*")
                        .header("User-Agent", mUserAgent)
                        .build();
                mRequest = addExtraHeaders(mRequest, additionalRequestHeaders);

                // For some operators, NAF server will return HTTP 406 Not Acceptable error.
                // Even for Accept-Encoding: Identity. So remove Accept-Encoding for
                // better compatibility
                mRequest = mRequest.newBuilder().removeHeader("Accept-Encoding").build();

                // Add Authorization header
                String content = (xml == null) ? "" : new String(xml);
                AkaDigestAuth akaDigestAuth = getAkaDigestAuth(mRequest, method, content);
                if (akaDigestAuth != null) {
                    akaDigestAuth.calculateRequestDigest();
                    mRequest = mRequest.newBuilder().addHeader("Authorization",
                            akaDigestAuth.createAuthorHeaderValue()).build();
                }

                // Print all the request header for log debugging
                // if (Log.isLoggable(TAG, Log.DEBUG)) {
                logRequestHeaders(mRequest);
                // }

                // Different stuff for GET and POST
                if (METHOD_PUT.equals(method)) {
                    Log.d(TAG, "METHOD_PUT");
                    MediaType mediaType = MediaType.parse(mimetype);
                    RequestBody body = RequestBody.create(mediaType, xml);
                    mRequest = mRequest.newBuilder().put(body).build();
                } else if (METHOD_GET.equals(method)) {
                    Log.d(TAG, "METHOD_GET");
                    mRequest = mRequest.newBuilder().build();
                }

                // Get the response
                Log.d(TAG, "newCall execute");
                mResponse = mOkHttpClient.newCall(mRequest).execute();

                final int responseCode = mResponse.code();
                final String responseMessage = mResponse.message();
                Log.d(TAG, "HTTP: " + responseCode + " " + responseMessage);

                String cipher = "";

                try {
                    cipher = mResponse.handshake().cipherSuite();
                    Log.d(TAG, "cipherSuite: " + cipher);
                } catch (Exception e) {
                    Log.e(TAG, "No handshake stage");
                }

                try {
                    for (Certificate certificate : mResponse.handshake().peerCertificates()) {
                        Log.d(TAG, "certificate: " + CertificatePinner.pin(certificate));
                    }
                } catch (Exception e) {
                    Log.e(TAG, "No certificate stage");
                }

                // Print all the response header for log debugging
                // if (Log.isLoggable(TAG, Log.DEBUG)) {
                logResponseHeaders(mResponse);
                // }

                if (responseCode == 200 || responseCode == 403  || responseCode == 304 ||
                        responseCode == 412 || responseCode == 201) {
                    success = true;
                    break;
                } else if (responseCode == 409) {
                    success = true;
                    break;
                } else if (responseCode == 401) {
                    System.setProperty("gba.auth", "401");
                    Log.d(TAG, "HTTP status code is 401. Force to run GBA");
                    handle401Exception(mResponse, mRequest.isHttps(), cipher);
                } else {
                    Log.d(TAG, "HTTP status code is not 200 or 403 or 409");
                }
            } catch (MalformedURLException e) {
                Log.e(TAG, "MalformedURLException");
                e.printStackTrace();
                throw e;
            } catch (ProtocolException e) {
                Log.e(TAG, "ProtocolException");
                e.printStackTrace();
                throw e;
            } catch (IOException e) {
                e.printStackTrace();

                if (e instanceof SocketTimeoutException) {
                    if (tryCount - 1 > 0) {
                        Log.d(TAG, "SocketTimeoutException: wait for retry.");
                    } else {
                        throw e;
                    }
                } else if (e instanceof UnknownHostException) {
                    if (tryCount - 1 > 0) {
                        Log.d(TAG, "Trying to use default system DNS");
                        NetworkDns.getInstance().setNetwork(null);
                        mOkHttpClient.setDns(NetworkDns.getInstance());
                    } else {
                        Log.e(TAG, "Throw UnknownHostException");
                        throw e;
                    }
                } else if ("HTTP 403 Forbidden".equals(e.getMessage())) {
                    success = true;
                    throw new IOException("GBA hit HTTP 403 Forbidden");
                } else if ("HTTP 400 Bad Request".equals(e.getMessage())) {
                    success = true;
                    throw new IOException("GBA hit HTTP 400 Bad Request");
                } else {
                    throw e;
                }
            } finally {
                if (!success) {
                    try {
                        tryCount--;
                        if (tryCount > 0) {
                            Thread.sleep(5 * 1000);
                            Log.d(TAG, "retry once");
                        }
                    } catch (InterruptedException e) {
                        Log.d(TAG, "InterruptedException");
                        e.printStackTrace();
                    }
                }
            }
        }

        return mResponse;
        //OkHttp usage end
    }

    /**
     * HTTP GET.
     *
     * @param  uri document URI
     * @param  additionalRequestHeaders HTTP headers
     * @return HTTP response
     * @throws IOException if I/O error
     */
    public Response get(URI uri, Headers additionalRequestHeaders) throws IOException {
        return execute(uri.toURL(), METHOD_GET, null, additionalRequestHeaders, null);
    }

    /**
     * HTTP PUT.
     *
     * @param  uri document URI
     * @param  mimetype MIME TYPE
     * @param  content content to upload
     * @return HTTP response
     * @throws IOException if I/O error
     */
    public Response put(URI uri, String mimetype, String content) throws IOException {
        Log.d(TAG, "PUT: " + encryptString(content));
        return put(uri, mimetype, content.getBytes("UTF-8"), null, null, null);

    }

    /**
     * HTTP PUT.
     *
     * @param  uri document URI
     * @param  mimetype MIME TYPE
     * @param  content content to upload
     * @param  additionalRequestHeaders HTTP headers
     * @return HTTP response
     * @throws IOException if I/O error
     */
    public Response put(URI uri, String mimetype, String content,
            Headers additionalRequestHeaders) throws IOException {
        Log.d(TAG, "PUT: " + encryptString(content));
        return put(uri, mimetype, content.getBytes("UTF-8"), additionalRequestHeaders, null, null);

    }

    /**
     * HTTP PUT.
     *
     * @param  uri document URI
     * @param  mimetype MIME TYPE
     * @param  content content to upload in string format
     * @param  additionalRequestHeaders HTTP headers
     * @param  eTag E-TAG
     * @param  condition use with E-TAG
     * @return HTTP response
     * @throws IOException if I/O error
     */
    public Response put(URI uri, String mimetype, String content,
            Headers additionalRequestHeaders, String eTag, String condition) throws IOException {
        Log.d(TAG, "PUT: " + encryptString(content));
        return put(uri, mimetype, content.getBytes("UTF-8"), additionalRequestHeaders, eTag,
                condition);

    }

    /**
     * HTTP PUT.
     *
     * @param  uri document URI
     * @param  mimetype MIME TYPE
     * @param  content content to upload in byte array format
     * @param  additionalRequestHeaders HTTP headers
     * @param  eTag E-TAG
     * @param  condition use with E-TAG
     * @return HTTP response
     * @throws IOException if I/O error
     */
    public Response put(URI uri, String mimetype, byte[] content,
            Headers additionalRequestHeaders, String eTag, String condition) throws IOException {
        return execute(uri.toURL(), METHOD_PUT, content, additionalRequestHeaders, mimetype);
    }

    /**
     * HTTP DELETE.
     *
     * @param  uri document URI
     * @return HTTP response
     * @throws IOException if I/O error
     */
    public Response delete(URI uri) throws IOException {
        return delete(uri, null, null, null);
    }

    /**
     * HTTP DELETE.
     *
     * @param  uri document URI
     * @param  additionalRequestHeaders HTTP headers
     * @return HTTP response
     * @throws IOException if I/O error
     */
    public Response delete(URI uri, Headers additionalRequestHeaders) throws IOException {
        return delete(uri, additionalRequestHeaders, null, null);
    }

    /**
     * HTTP DELETE.
     *
     * @param  uri document URI
     * @param  additionalRequestHeaders HTTP headers
     * @param  eTag E-TAG
     * @param  condition use with E-TAG
     * @return HTTP response
     * @throws IOException if I/O error
     */
    public Response delete(URI uri, Headers additionalRequestHeaders, String eTag,
            String condition) throws IOException {
        return execute(uri.toURL(), METHOD_DELETE, null, additionalRequestHeaders, null);
    }

    public static String encryptString(String message) {
        java.util.Base64.Encoder encoder = java.util.Base64.getEncoder();
        byte[] textByte = null;
        try {
            textByte = message.getBytes("UTF-8");
        } catch (Exception e) {
            e.printStackTrace();
            textByte = null;
        }

        if (textByte == null) return "";

        String encryptedString = encoder.encodeToString(textByte);
        return encryptedString;
    }

    /**
     * Class for application interceptor to handle request header before sending to network.
     *
     */
    class RequestInterceptor implements Interceptor {
        @Override
        public Response intercept(Interceptor.Chain chain) throws IOException {
            Log.d(TAG, "okhttp intercepting ...");
            Request request = chain.request();

            String requestEncoding = request.header("Accept-Encoding");
            if (!TextUtils.isEmpty(requestEncoding)) {
                Log.i(TAG, "found Accept-Encoding, remove it:" + requestEncoding);
                request = request.newBuilder().removeHeader("Accept-Encoding").build();
            }
            Response response = chain.proceed(request);
            return response;
        }
    }

    public static class NetworkDns implements Dns {

        private static NetworkDns sInstance;
        private Network mNetwork;

        public static NetworkDns getInstance() {
            if (sInstance == null) {
                sInstance = new NetworkDns();
            }
            return sInstance;
        }

        public void setNetwork(Network network) {
            mNetwork = network;
        }

        @Override
        public List<InetAddress> lookup(String hostname) throws UnknownHostException {
            if (mNetwork != null) {
                Log.d(TAG, "lookup hostname:" + ((!SENLOG) ? hostname : "[hidden]"));
                List<InetAddress> list = Arrays.asList(mNetwork.getAllByName(hostname));
                if (list != null && list.size() > 0) {
                    Log.d(TAG, "list size:" + list.size());
                    for (int i = 0; i < list.size(); i++) {
                        if (!SENLOG) Log.d(TAG, "InetAddress :" + list.get(i).toString());
                    }
                } else {
                    Log.d(TAG, "list is null, using SYSTEM Dns to lookup");
                    return Dns.SYSTEM.lookup(hostname);
                }
                return list;
            }
            return Dns.SYSTEM.lookup(hostname);
        }

        private NetworkDns() {
        }
    }
}
