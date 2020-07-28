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

package com.orangelabs.rcs.core.ims.service.presence.xdm;

import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.ByteArrayInputStream;
import java.io.DataOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.OutputStream;
import java.io.OutputStreamWriter;
import java.net.Authenticator;
import java.net.URL;
import java.sql.Struct;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.Hashtable;
import java.util.List;
import java.util.Map;
import java.util.Map.Entry;

import org.xbill.DNS.NULLRecord;
import org.xml.sax.InputSource;

import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;

import com.orangelabs.rcs.core.ims.protocol.http.HttpAuthenticationAgent;
import com.orangelabs.rcs.core.ims.protocol.http.HttpDeleteRequest;
import com.orangelabs.rcs.core.ims.protocol.http.HttpGetRequest;
import com.orangelabs.rcs.core.ims.protocol.http.HttpPutRequest;
import com.orangelabs.rcs.core.ims.protocol.http.HttpRequest;
import com.orangelabs.rcs.core.ims.protocol.http.HttpResponse;

import com.orangelabs.rcs.core.CoreException;
import com.orangelabs.rcs.core.TerminalInfo;
import com.orangelabs.rcs.core.ims.ImsModule;
import com.orangelabs.rcs.core.ims.service.presence.PhotoIcon;
import com.orangelabs.rcs.core.ims.service.presence.directory.Folder;
import com.orangelabs.rcs.core.ims.service.presence.directory.XcapDirectoryParser;
import com.orangelabs.rcs.platform.network.AndroidDefaultNetwork;
import com.orangelabs.rcs.platform.network.NetworkException;
import com.orangelabs.rcs.platform.network.NetworkFactory;
import com.orangelabs.rcs.platform.network.SocketConnection;
import com.orangelabs.rcs.provider.eab.ContactsManager;
import com.orangelabs.rcs.provider.settings.RcsSettings;
import com.orangelabs.rcs.provisioning.https.EasySSLSocketFactory;
import com.orangelabs.rcs.service.StartService;
import com.orangelabs.rcs.utils.Base64;
import com.orangelabs.rcs.utils.HttpUtils;
import com.orangelabs.rcs.utils.PeriodicRefresher;
import com.orangelabs.rcs.utils.PhoneUtils;
import com.orangelabs.rcs.utils.logger.Logger;

//apache imports

import org.apache.http.Header;
import org.apache.http.HttpHost;
//import org.apache.http.HttpResponse;
import org.apache.http.HttpVersion;
import org.apache.http.auth.AuthScope;
import org.apache.http.client.ClientProtocolException;
import org.apache.http.client.CookieStore;
import org.apache.http.client.CredentialsProvider;
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
import org.apache.http.util.EntityUtils;
import org.apache.http.client.CredentialsProvider;
import org.apache.http.impl.client.BasicCredentialsProvider;
import org.apache.http.auth.AuthScope;
import org.apache.http.cookie.Cookie;
import org.apache.http.cookie.CookieOrigin;
import org.apache.http.cookie.CookieSpec;
import org.apache.http.cookie.CookieSpecFactory;
import org.apache.http.client.params.ClientPNames;
import org.apache.http.impl.cookie.BrowserCompatSpec;
import org.apache.http.entity.StringEntity;
//import com.mediatek.gba.GbaHttpUrlCredential;
import com.orangelabs.rcs.platform.AndroidFactory;
import org.apache.http.client.methods.HttpPut;
import org.apache.http.client.methods.HttpGet;
import org.apache.http.client.methods.HttpDelete;

import java.net.Authenticator;
import java.net.HttpURLConnection;
import java.net.URI;
import java.net.URL;
import java.net.URLConnection;

import org.apache.http.util.EntityUtils;
import java.io.IOException;
//import com.mediatek.gba.GbaHttpUrlCredential;
import android.net.Network;
import android.net.NetworkInfo;
import android.net.ConnectivityManager;
import android.telephony.SubscriptionManager;
import android.net.NetworkCapabilities;

/**
 * XDM manager
 * 
 * @author Jean-Marc AUFFRET
 */
public class XdmManager {
    /**
     * XDM server address
     */
    private String xdmServerAddr;

    /**
     * XDM server login
     */
    private String xdmServerLogin;

    /**
     * XDM server password
     */
    private String xdmServerPwd;

    /**
     * Managed documents
     */
    private Hashtable<String, Folder> documents = new Hashtable<String, Folder>();

    /**
     * The logger
     */
    private Logger logger = Logger.getLogger(this.getClass().getName());

    // count of contacts stored on XCAP server
    private static int countGrantedContacts = 0;
    private static int countBlockedContacts = 0;
    private static int countRevokedContacts = 0;

    // max count of contacts stored at XDMS server
    private static final int MAX_COUNT_XDMS_CONTACTS = 2000;

    private ConnectivityManager connectivityMgr;
    private static String USER_AGENT = "";

    // network via which the XDM request needs to be sent
    private static AndroidDefaultNetwork mActiveNetworkConnection = null;

    private static List<String> grantedContacts = new ArrayList<String>();

    // is GBA XDM authenticateion type
    private static boolean isGBAxcapAuthenticationType = false;

    // GBA auth type name
    private final String AUTH_GBA = "GBA";
    private static boolean isProcessing = false;

    
    /**
     * HASHMAP with key as contact and value as XDMContact details
     */
    private static HashMap<String, XDMContact> xdmContactMap = new HashMap<String, XDMContact>();
    
    
    /**
     * XDM Operation type
     */
    private static enum operationType{
        DEFAULT,
        ADDITION_CONTACT_GRANTED_LIST,
        ADDITION_CONTACT_GRANTED_LIST_ALL,
        DELETE_CONTACT_GRANTED_LIST,
        ADDITION_CONTACT_BLOCKED_LIST,
        DELETE_CONTACT_BLOCKED_LIST,
        ADDITION_CONTACT_REVOKE_LIST,
        DELETE_CONTACT_REVOKE_LIST,
    }
    
    /**
     * 
     * Class XCAP operation
     *
     */
    private class XCAPOperation extends PeriodicRefresher {
        private Object syncLock = new Object();

        private int DEFAULT_TIMER = 0;// 0 min;
        private int MAX_TIMER = 1440;// 24 hrs

        // result of the xcap operation
        private Object result;

        // is the xcap operation successfull
        private boolean isSuccessfull = false;

        private List<XDMContact> xdmContactList = new ArrayList<XDMContact>(); // in
                                                                               // cases
                                                                               // of
                                                                               // add/delete
        // timer will be in seconds
        private int timer = DEFAULT_TIMER;// time after which this xcap
                                          // operations needs to be updated.

        // retry-after timer value
        private int retryAfterTimer = DEFAULT_TIMER;

        int timerIndex = 0;

        public operationType type = operationType.DEFAULT;

        // 2 min, 5 min, 15 min, 30 min, 1 hour, 2 hours, 4 hours, 8 hours, 16
        // hours, 24 hours and then every 24 hours thereafter.
        private int[] timerList = { DEFAULT_TIMER, 2, 5, 15, 30, 60, 120, 240,
                480, 960, MAX_TIMER }; // in minutes and then hrs [2min, 5, 15,
                                       // 30, 1 hr, 2 ,4,8,16,24]

        public XCAPOperation(List<XDMContact> contactList, operationType type) {
            logger.debug("XCAPOperation contactlist" + contactList);
            this.type = type;
            xdmContactList.addAll(contactList);
        }

        public XCAPOperation(List<XDMContact> contactList, int timer,
                operationType type) {
            xdmContactList = contactList;
            this.timerIndex = 0;
            this.timer = timerList[this.timerIndex];
            this.type = type;
        }

        public Object execute() {
            // synchronized (syncLock) {
            if (timer <= DEFAULT_TIMER) { // execute it now
                try {
                    
                     periodicProcessing();
                    
                    if (!isSuccessfull) {
                        if (logger.isActivated()) {
                            logger.debug("XCAP Operation failed. Timer started.");
                        }
                    } else {
                        if (logger.isActivated()) {
                            logger.debug("XCAP Operation finished successfully");
                        }
                    }
                } catch (Exception e) {
                    // TODO Auto-generated catch block
                    e.printStackTrace();
                    if (logger.isActivated()) {
                        logger.error("Exception at XcapOperation execution : "+e.getMessage());
                    }
                }
            }
            return result;
        }

        private void incrementTimer() {
            // if its the last element return last element
            if (timerIndex == (timerList.length - 1)) {
                timer = timerList[timerIndex];
            } else {
                // increment to the next timer value
                timerIndex++;
                timer = timerList[timerIndex] * 60; // in seconds
            }
        }

        @Override
        public void periodicProcessing() {
            HttpResponse xcapResponse;
            isSuccessfull = false;
            result = null;

            synchronized (syncLock) {
                switch (this.type) {

                case ADDITION_CONTACT_GRANTED_LIST:
                    // choose one contcats to be added to rnated list

                    if (xdmContactList.size() > 0) {
                        result = addContactToGrantedListOperation(xdmContactList.get(0)
                                .getContact());

                        if (result != null) {
                            isSuccessfull = ((HttpResponse) result)
                                    .isSuccessfullResponse();
                        }
                    }
                    break;
                case ADDITION_CONTACT_GRANTED_LIST_ALL:
                    // choose one contcats to be added to rnated list

                    if (xdmContactList.size() > 0) {
                        result = addContactToGrantedListOperation(xdmContactList);

                        if (result != null) {
                            isSuccessfull = ((HttpResponse) result)
                                    .isSuccessfullResponse();
                        }
                    }
                    break;
                case DELETE_CONTACT_GRANTED_LIST:
                    // choose one contcats to be added to rnated list
                    if (xdmContactList.size() > 0) {
                        result = removeContactFromGrantedListOperation(xdmContactList
                                .get(0).getContact());

                        if (result != null) {
                            isSuccessfull = ((HttpResponse) result)
                                    .isSuccessfullResponse();
                        }
                    }
                    break;
                case ADDITION_CONTACT_BLOCKED_LIST:
                    if (xdmContactList.size() > 0) {
                        result = addContactToBlockedListOperation(xdmContactList.get(0)
                                .getContact());

                        if (result != null) {
                            isSuccessfull = ((HttpResponse) result)
                                    .isSuccessfullResponse();
                        }
                    }
                    break;
                case DELETE_CONTACT_BLOCKED_LIST:
                    if (xdmContactList.size() > 0) {
                        result = removeContactFromBlockedListOperation(xdmContactList
                                .get(0).getContact());

                        if (result != null) {
                            isSuccessfull = ((HttpResponse) result)
                                    .isSuccessfullResponse();
                        }
                    }
                    break;
                case ADDITION_CONTACT_REVOKE_LIST:
                    if (xdmContactList.size() > 0) {
                        result = addContactToRevokedListOperation(xdmContactList.get(0)
                                .getContact());

                        if (result != null) {
                            isSuccessfull = ((HttpResponse) result)
                                    .isSuccessfullResponse();
                        }
                    }
                    break;
                case DELETE_CONTACT_REVOKE_LIST:
                    if (xdmContactList.size() > 0) {
                        result = removeContactFromRevokedListOperation(xdmContactList
                                .get(0).getContact());

                        if (result != null) {
                            isSuccessfull = ((HttpResponse) result)
                                    .isSuccessfullResponse();
                        }
                    }
                    break;
                case DEFAULT:
                    break;
                }

                syncLock.notify();

                if (!isSuccessfull) {

                    if (result != null) {
                        if (((HttpResponse) result).getHeader("retry-after") != null) {
                            timer = Integer.parseInt(((HttpResponse) result)
                                    .getHeader("retry-after"));

                        } else {
                            // reset the timer again
                            incrementTimer();
                        }
                    } else {
                        // reset the timer again
                        incrementTimer();
                    }
                    startTimer(timer);
                } else {
                    stopTimer();
                }
            }
        }
    };
    
    
    /**
     * Constructor
     * 
     * @param parent
     *            IMS module
     */
    public XdmManager(ImsModule parent) {
        xdmServerAddr = ImsModule.IMS_USER_PROFILE.getXdmServerAddr();
        xdmServerLogin = ImsModule.IMS_USER_PROFILE.getXdmServerLogin();
        xdmServerPwd = ImsModule.IMS_USER_PROFILE.getXdmServerPassword();

        // check GBA suport
        if (RcsSettings.getInstance().getXDMAuth().equalsIgnoreCase(AUTH_GBA)) {
            isGBAxcapAuthenticationType = true;
        }

        // initalize user agant
        USER_AGENT = "";
        isProcessing = false;

    }

    /**
     * Send HTTP PUT request
     * 
     * @param request
     *            HTTP request
     * @return HTTP response
     * @throws CoreException
     */
    private HttpResponse sendRequestToXDMS(HttpRequest request)
            throws CoreException {

        // send request via httpURL connection
        return sendHttpURLConnectionRequestToXDMS(request);

        // return sendApacheHttpRequestToXDMS(request);
        /*
         * return sendRequestToXDMS(request, new HttpAuthenticationAgent(
         * xdmServerLogin, xdmServerPwd));
         */
    }

    /**
     * Send HTTP PUT request with authentication
     * 
     * @param request
     *            HTTP request
     * @param authenticationAgent
     *            Authentication agent
     * @return HTTP response
     * @throws CoreException
     */
    private HttpResponse sendRequestToXDMS(HttpRequest request,
            HttpAuthenticationAgent authenticationAgent) throws CoreException {
        try {
            // Send first request
            HttpResponse response = sendHttpRequest(request,
                    authenticationAgent);

            // Analyze the response
            if (response.getResponseCode() == 401) {
                // 401 response received
                if (logger.isActivated()) {
                    logger.debug("401 Unauthorized response received");
                }

                if (authenticationAgent != null) {
                    // Update the authentication agent
                    authenticationAgent.readWwwAuthenticateHeader(response
                            .getHeader("www-authenticate"));
                }

                // Set the cookie from the received response
                String cookie = response.getHeader("set-cookie");
                request.setCookie(cookie);

                // Send second request with authentification header
                response = sendRequestToXDMS(request, authenticationAgent);
            } else if (response.getResponseCode() == 412) {
                // 412 response received
                if (logger.isActivated()) {
                    logger.debug("412 Precondition failed");
                }

                // Reset the etag
                documents.remove(request.getAUID());

                // Send second request with authentification header
                response = sendRequestToXDMS(request);
            } else {
                // Other response received
                if (logger.isActivated()) {
                    logger.debug(response.getResponseCode()
                            + " response received");
                }
            }
            return response;
        } catch (CoreException e) {
            throw e;
        } catch (Exception e) {
            throw new CoreException("Can't send HTTP request: "
                    + e.getMessage());
        }
    }

    // THIS METHOD IS USED TO SEND REQUEST WITH HTTPURL CONNECTION.
    // APACHE HTTP STACK IS NOT USED IN THIS METHOD.
    // WITH THE LATEST PATHCES OF GBA, HTTPURLCONNECTION ALSO SUPPORTS GBA
    private HttpResponse sendHttpURLConnectionRequestToXDMS(HttpRequest request) {
        HttpResponse xdmresponse = new HttpResponse();

        try {

            String nafAddress = xdmServerAddr;

            // GET phones default n/w

            // Set the connectivity manager
            connectivityMgr = (ConnectivityManager) AndroidFactory
                    .getApplicationContext().getSystemService(
                            Context.CONNECTIVITY_SERVICE);
            Network activeNetwork = null;
            activeNetwork = mActiveNetworkConnection.getNetwork();
            if (activeNetwork != null) {

                if (isGBAxcapAuthenticationType) {/*
                    // include GBA support
                    GbaHttpUrlCredential gbaCredential = new GbaHttpUrlCredential(
                            AndroidFactory.getApplicationContext(), nafAddress);
                    gbaCredential.setNetwork(activeNetwork);
                    Authenticator.setDefault(gbaCredential.getAuthenticator());
                */}

                // execute request
                xdmresponse = executeHttpConnection(activeNetwork, request);
            }

        } catch (Exception e) {
            if (logger.isActivated()) {
                logger.error("xdm request error : ", e);
            }
        }

        return xdmresponse;
    }

    private HttpResponse sendApacheHttpRequestToXDMS(HttpRequest request) {
        HttpResponse xdmresponse = new HttpResponse();

        try {
            SchemeRegistry schemeRegistry = new SchemeRegistry();
            schemeRegistry.register(new Scheme("http", PlainSocketFactory
                    .getSocketFactory(), 80));
            schemeRegistry.register(new Scheme("https",
                    new EasySSLSocketFactory(), 443));

            if (logger.isActivated()) {
                logger.debug("adding org.apache.http.wire filter for https log");
            }

            java.util.logging.Logger.getLogger("org.apache.http.wire")
                    .setLevel(java.util.logging.Level.ALL);

            // https prama
            HttpParams params = new BasicHttpParams();
            params.setParameter(ConnManagerPNames.MAX_TOTAL_CONNECTIONS, 30);
            params.setParameter(ConnManagerPNames.MAX_CONNECTIONS_PER_ROUTE,
                    new ConnPerRouteBean(30));
            params.setParameter(HttpProtocolParams.USE_EXPECT_CONTINUE, false);

            HttpClientParams.setCookiePolicy(params,
                    CookiePolicy.BROWSER_COMPATIBILITY);

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

            ClientConnectionManager cm = new SingleClientConnManager(params,
                    schemeRegistry);
            DefaultHttpClient client = new DefaultHttpClient(cm, params);

            CookieStore cookieStore = (CookieStore) new BasicCookieStore();
            HttpContext localContext = new BasicHttpContext();
            localContext.setAttribute(ClientContext.COOKIE_STORE, cookieStore);

            client.getCookieSpecs().register("easy", csf);
            client.getParams().setParameter(ClientPNames.COOKIE_POLICY, "easy");

            String serverAddress = "";
            serverAddress = xdmServerAddr;
            String requestUri = serverAddress + request.getUrl();

            org.apache.http.HttpResponse response = executeRequest("http",
                    requestUri, client, localContext, request);

            xdmresponse = convertResponsetoXDMFormat(response);

        } catch (Exception e) {
            if (logger.isActivated()) {
                logger.error("error : ", e);
            }
            return null;
        }

        return xdmresponse;
    }

    private HttpResponse convertResponsetoXDMFormat(
            org.apache.http.HttpResponse response) throws IOException {

        HttpResponse xdmResponse = null;
        if (response != null) {
            xdmResponse = new HttpResponse();

            // set stataus line
            xdmResponse.setStatusLine(response.getStatusLine().toString());

            // add content
            xdmResponse
                    .setContent(EntityUtils.toByteArray(response.getEntity()));

            // add headers
            Header[] headers = response.getAllHeaders();
            for (Header header : headers) {
                xdmResponse.addHeader(header.getName(), header.getValue());
            }

            return xdmResponse;
        } else {
            return null;
        }

    }

    private org.apache.http.HttpResponse executeRequest(String protocol,
            String request, DefaultHttpClient client, HttpContext localContext,
            HttpRequest xdmHttpRequest) {

        if (xdmHttpRequest.getMethod().equalsIgnoreCase("GET")) {
            return executeGetRequest(protocol, request, client, localContext,
                    xdmHttpRequest);
        } else if (xdmHttpRequest.getMethod().equalsIgnoreCase("PUT")) {
            return executePutRequest(protocol, request, client, localContext,
                    xdmHttpRequest);
        } else if (xdmHttpRequest.getMethod().equalsIgnoreCase("DELETE")) {
            return executeDeleteRequest(protocol, request, client,
                    localContext, xdmHttpRequest);
        } else {
            return null;
        }
    }

    synchronized HttpResponse executeHttpConnection(Network network, HttpRequest request) {
        if (logger.isActivated()) {
            logger.debug("executeHttpConnection");
        }

        HttpResponse resp = null;
        URL uri = null;
        HttpURLConnection con = null;
        String serverAddress = "http://" + xdmServerAddr;

        try {
            serverAddress += request.getUrl();
            uri = new URL(serverAddress);
            // con = (HttpURLConnection) uri.openConnection();
            con = (HttpURLConnection) network.openConnection(uri);
            String method = request.getMethod().toUpperCase();
            con.setRequestMethod(method); // type: POST, PUT, DELETE, GET
            con.setDoInput(true);
            con.setConnectTimeout(60000); // 60 secs
            con.setReadTimeout(60000); // 60 secs

            // add request header
            con.setRequestProperty("User-Agent", USER_AGENT);

            // IDENTITY
            String identity = "\"" + xdmServerLogin + "\"";
            con.setRequestProperty("X-3GPP-Intended-Identity", identity);

            // ADD etag
            Folder folder = (Folder) documents.get(request.getAUID());
           
            if ((folder != null) && (folder.getEntry() != null)
                    && (folder.getEntry().getEtag() != null)) {
                String eTag = "\"" + folder.getEntry().getEtag() + "\"";
                con.setRequestProperty("If-match", eTag);
            }

            // if there is some content
            if (request.getContent() != null) {
                con.setDoOutput(true);

                // CONTENT TYPE
                con.setRequestProperty("Content-Type", request.getContentType());
                DataOutputStream wr = new DataOutputStream(
                        con.getOutputStream());
                BufferedWriter writer = new BufferedWriter(
                        new OutputStreamWriter(wr, "UTF-8"));
                writer.write(request.getContent());
                writer.close();

            }

            // read for response
           int responseCode = con.getResponseCode();
           logger.info("xdm response received is  " + responseCode);

           String inputLine ="";
           String resContent = "";
           if(responseCode < HttpURLConnection.HTTP_OK || responseCode >= 300)
           {
               logger.info("xdm response received is  " + responseCode +" ; so return back");
               resp = convertHTTPUrlResponseToXDMFormat(con,resContent);
           }
           else{
            // read the input stream and send request
            BufferedReader in = new BufferedReader(
                    new InputStreamReader(con.getInputStream(),
                            "UTF-8"));
            
            while ((inputLine = in.readLine()) != null) {
                resContent += inputLine+"\n";
            }
            in.close();
            //reading stops
            
            logger.info("xdm response received is  " + con.getResponseCode());
            logger.debug("xdm content  received is  " + resContent);

            resp = convertHTTPUrlResponseToXDMFormat(con,resContent);

            // update etag
            if ((resp.getHeader("etag") != null) && (folder != null)
                    && (folder.getEntry() != null)) {
                folder.getEntry().setEtag(resp.getHeader("etag"));
            }
           }

        } catch (Exception e) {
            logger.error("xdm connection error : " + e.getMessage());
            return null;
        } finally {
            if (con != null)
                con.disconnect();
        }

        return resp;
    }

    private HttpResponse convertHTTPUrlResponseToXDMFormat(
            HttpURLConnection conn,String Content) {
        HttpResponse xdmResponse = null;

        logger.debug("convertHTTPUrlResponseToXDMFormat");

        if (conn != null) {
            try {

                /*long contentLength = Long.parseLong(conn.getHeaderField("content-Length"));
                logger.info("connection response is " + conn.getResponseCode() + " and content-length : "+contentLength);*/
                xdmResponse = new HttpResponse();
                // set the status
                xdmResponse.setStatusLine("HTTP/1.1 " + conn.getResponseCode()
                        + " " + conn.getResponseMessage());
                // set the headers
                Map<String, List<String>> headerList = conn.getHeaderFields();

                // populate the
                // response data
                // 1) etag
                // 2) content-type

                // its the key for response code as there is no key for it
                // sample data for headers

                /*
                 * {null=[HTTP/1.1 200 OK], authentication-info=[nextnonce
                 * ="78ffff7cffffff225dffffffffffff57", qop="auth",
                 * rspauth="54b38f0ae3a8614ff359293c2f96f2c1",
                 * cnonce="ce908499f83f6c9d", nc=00000001], connection=[close],
                 * content-type=[application/resource-lists+xml], date=[Fri, 18
                 */

                if(headerList!=null)
                {
                // GET -ETAG
                if ((headerList.get("etag") != null)
                        && (headerList.get("etag").get(0) != null)) {
                    String etag = headerList.get("etag").get(0);
                    xdmResponse.addHeader("etag", etag);
                }
                
                //set the retry after header
                if ((headerList.get("retry-after") != null)
                        && (headerList.get("retry-after").get(0) != null)) {
                    String retryAfterTimer = headerList.get("retry-after").get(0);
                    xdmResponse.addHeader("retry-after", retryAfterTimer);
                }
                }
                /*
                 * set the content
                 */
              
                if (!Content.equals("")) {
                        xdmResponse.setContent(Content.getBytes());
                } else {
                    logger.debug("xdm response has no content");
                }

            } catch (Exception err) {
                logger.error("exception at XDM conversion " + err.getMessage());
                err.printStackTrace();
                return null;
            }

        }

        return xdmResponse;
    }

    private org.apache.http.HttpResponse executeGetRequest(String protocol,
            String request, DefaultHttpClient client, HttpContext localContext,
            HttpRequest xdmHttpRequest) {
        try {

            HttpGet put = new HttpGet();

            put.setURI(new URI(protocol + "://" + request));

            // USER-AGENT
            String userAgent = USER_AGENT;
            put.addHeader("User-Agent", userAgent); // add user agent

            // X-3GPP-Intended-Identity:
            // String identity =
            // "\""+RcsSettings.getInstance().getUserProfileImsUserName_full()+"\"";
            String identity = "\"" + xdmServerLogin + "\"";
            put.addHeader("X-3GPP-Intended-Identity", identity); // add user
                                                                 // agent

           
            // add GBA support
            String nafAddress = xdmServerAddr;
            

            URL url1 = new URL(nafAddress);
           /* GbaHttpUrlCredential gbaCredential = new GbaHttpUrlCredential(
                    AndroidFactory.getApplicationContext(), url1.getHost());
            Authenticator.setDefault(gbaCredential.getAuthenticator());*/
            org.apache.http.HttpResponse response = client.execute(put,
                    localContext);

            return response;
        } catch (Exception e) {
            if (logger.isActivated()) {
                logger.debug("The server " + request + " can't be reached!");
            }
            return null;
        }
    }

    private org.apache.http.HttpResponse executeDeleteRequest(String protocol,
            String request, DefaultHttpClient client, HttpContext localContext,
            HttpRequest xdmHttpRequest) {
        try {

            HttpDelete deleteReq = new HttpDelete();

            deleteReq.setURI(new URI(protocol + "://" + request));

            // USER-AGENT
            String userAgent = USER_AGENT;
            deleteReq.addHeader("User-Agent", userAgent); // add user agent

            // X-3GPP-Intended-Identity:
            // String identity =
            // "\""+RcsSettings.getInstance().getUserProfileImsUserName_full()+"\"";
            String identity = "\"" + xdmServerLogin + "\"";
            deleteReq.addHeader("X-3GPP-Intended-Identity", identity); // add
                                                                       // user
                                                                       // agent

            if (xdmHttpRequest.getContent() != null) {
                // Set the content type
                deleteReq.setHeader("Content-Type",
                        xdmHttpRequest.getContentType());
                // deleteReq.setEntity(new
                // StringEntity(xdmHttpRequest.getContent(),"UTF-8"));
            } /*
               * else { put.setHeader("Content-Type", ""+0); }
               */

            // add GBA support
            String nafAddress = xdmServerAddr;
            // CredentialsProvider credProvider = new
            // BasicCredentialsProvider();
            // credProvider.setCredentials(new AuthScope(AuthScope.ANY_HOST,
            // AuthScope.ANY_PORT), new
            // GbaCredentials(AndroidFactory.getApplicationContext(),
            // nafAddress));
            // client.setCredentialsProvider(credProvider);

            URL url1 = new URL(nafAddress);
           /* GbaHttpUrlCredential gbaCredential = new GbaHttpUrlCredential(
                    AndroidFactory.getApplicationContext(), url1.getHost());
            Authenticator.setDefault(gbaCredential.getAuthenticator());*/

            org.apache.http.HttpResponse response = client.execute(deleteReq,
                    localContext);

            return response;
        } catch (Exception e) {
            if (logger.isActivated()) {
                logger.debug("The server " + request + " can't be reached!");
            }
            return null;
        }
    }

    private org.apache.http.HttpResponse executePutRequest(String protocol,
            String request, DefaultHttpClient client, HttpContext localContext,
            HttpRequest xdmHttpRequest) {
        try {

            HttpPut put = new HttpPut();

            put.setURI(new URI(protocol + "://" + request));

            // USER-AGENT
            String userAgent = USER_AGENT;
            put.addHeader("User-Agent", userAgent); // add user agent

            // X-3GPP-Intended-Identity:
            // String identity =
            // "\""+RcsSettings.getInstance().getUserProfileImsUserName_full()+"\"";
            String identity = "\"" + xdmServerLogin + "\"";
            put.addHeader("X-3GPP-Intended-Identity", identity); // add user
                                                                 // agent

            if (xdmHttpRequest.getContent() != null) {
                // Set the content type
                put.setHeader("Content-Type", xdmHttpRequest.getContentType());
                put.setEntity(new StringEntity(xdmHttpRequest.getContent(),
                        "UTF-8"));
            } /*
               * else { put.setHeader("Content-Type", ""+0); }
               */

            // add GBA support
            String nafAddress = xdmServerAddr;
            // CredentialsProvider credProvider = new
            // BasicCredentialsProvider();
            // credProvider.setCredentials(new AuthScope(AuthScope.ANY_HOST,
            // AuthScope.ANY_PORT), new
            // GbaCredentials(AndroidFactory.getApplicationContext(),
            // nafAddress));
            // client.setCredentialsProvider(credProvider);

            URL url1 = new URL(nafAddress);
            /*GbaHttpUrlCredential gbaCredential = new GbaHttpUrlCredential(
                    AndroidFactory.getApplicationContext(), url1.getHost());
            Authenticator.setDefault(gbaCredential.getAuthenticator());*/

            org.apache.http.HttpResponse response = client.execute(put,
                    localContext);

            return response;
        } catch (Exception e) {
            if (logger.isActivated()) {
                logger.debug("The server " + request + " can't be reached!");
            }
            return null;
        }
    }

    /**
     * Send HTTP PUT request
     * 
     * @param request
     *            HTTP request
     * @param authenticationAgent
     *            Authentication agent
     * @return HTTP response
     * @throws IOException
     * @throws CoreException
     */
    private HttpResponse sendHttpRequest(HttpRequest request,
            HttpAuthenticationAgent authenticationAgent) throws IOException,
            CoreException {
        // Extract host & port
        String[] parts = xdmServerAddr.substring(7).split(":|/");
        String host = parts[0];
        int port = Integer.parseInt(parts[1]);
        String serviceRoot = "";
        if (parts.length > 2) {
            serviceRoot = "/" + parts[2];
        }

        // Open connection with the XCAP server
        SocketConnection conn = NetworkFactory.getFactory()
                .createSocketClientConnection();
        conn.open(host, port);
        InputStream is = conn.getInputStream();
        OutputStream os = conn.getOutputStream();

        // Create the HTTP request
        String requestUri = serviceRoot + request.getUrl();
        String httpRequest = request.getMethod() + " " + requestUri
                + " HTTP/1.1" + HttpUtils.CRLF + "Host: " + host + ":" + port
                + HttpUtils.CRLF + "User-Agent: "
                + TerminalInfo.getProductName() + " "
                + TerminalInfo.getProductVersion() + HttpUtils.CRLF;

        if (authenticationAgent != null) {
            // Set the Authorization header
            String authorizationHeader = authenticationAgent
                    .generateAuthorizationHeader(request.getMethod(),
                            requestUri, request.getContent());
            httpRequest += authorizationHeader + HttpUtils.CRLF;
        }

        String cookie = request.getCookie();
        if (cookie != null) {
            // Set the cookie header
            httpRequest += "Cookie: " + cookie + HttpUtils.CRLF;
        }

        httpRequest += "X-3GPP-Intended-Identity: \"" + xdmServerLogin + "\""
                + HttpUtils.CRLF;

        // Set the If-match header
        Folder folder = (Folder) documents.get(request.getAUID());
        if ((folder != null) && (folder.getEntry() != null)
                && (folder.getEntry().getEtag() != null)) {
            httpRequest += "If-match: \"" + folder.getEntry().getEtag() + "\""
                    + HttpUtils.CRLF;
        }

        if (request.getContent() != null) {
            // Set the content type
            httpRequest += "Content-type: " + request.getContentType()
                    + HttpUtils.CRLF;
            httpRequest += "Content-Length:" + request.getContentLength()
                    + HttpUtils.CRLF + HttpUtils.CRLF;
        } else {
            httpRequest += "Content-Length: 0" + HttpUtils.CRLF
                    + HttpUtils.CRLF;
        }

        // Write HTTP request headers
        os.write(httpRequest.getBytes());
        os.flush();

        // Write HTTP content
        if (request.getContent() != null) {
            os.write(request.getContent().getBytes("UTF-8"));
            os.flush();
        }

        if (logger.isActivated()) {
            if (request.getContent() != null) {
                logger.debug("Send HTTP request:\n" + httpRequest
                        + request.getContent());
            } else {
                logger.debug("Send HTTP request:\n" + httpRequest);
            }
        }

        // Read HTTP headers response
        StringBuffer respTrace = new StringBuffer();
        HttpResponse response = new HttpResponse();
        int ch = -1;
        String line = "";
        while ((ch = is.read()) != -1) {
            line += (char) ch;

            if (line.endsWith(HttpUtils.CRLF)) {
                if (line.equals(HttpUtils.CRLF)) {
                    // All headers has been read
                    break;
                }

                if (logger.isActivated()) {
                    respTrace.append(line);
                }

                // Remove CRLF
                line = line.substring(0, line.length() - 2);

                if (line.startsWith("HTTP/")) {
                    // Status line
                    response.setStatusLine(line);
                } else {
                    // Header
                    int index = line.indexOf(":");
                    String name = line.substring(0, index).trim().toLowerCase();
                    String value = line.substring(index + 1).trim();
                    response.addHeader(name, value);
                }

                line = "";
            }
        }

        // Extract content length
        int length = -1;
        try {
            String value = response.getHeader("content-length");
            length = Integer.parseInt(value);
        } catch (Exception e) {
            length = -1;
        }

        // Read HTTP content response
        if (length > 0) {
            byte[] content = new byte[length];
            int nb = -1;
            int pos = 0;
            byte[] buffer = new byte[1024];
            while ((nb = is.read(buffer)) != -1) {
                System.arraycopy(buffer, 0, content, pos, nb);
                pos += nb;

                if (pos >= length) {
                    // End of content
                    break;
                }
            }
            if (logger.isActivated()) {
                respTrace.append(HttpUtils.CRLF + new String(content));
            }
            response.setContent(content);
        }

        if (logger.isActivated()) {
            logger.debug("Receive HTTP response:\n" + respTrace.toString());
        }

        // Close the connection
        is.close();
        os.close();
        conn.close();

        // Save the Etag from the received response
        String etag = response.getHeader("etag");
        if ((etag != null) && (folder != null) && (folder.getEntry() != null)) {
            folder.getEntry().setEtag(etag);
        }

        return response;
    }

    /**
     * Initialize the XDM interface
     */
    public boolean initialize() {
        // Get the existing XCAP documents on the XDM server
        try {

            if (isProcessing) {
                return false;
            }

            isProcessing = true;

            // set apache logger property to prnit hTTP logs
            if (logger.isActivated()) {
                logger.debug("initialize");
            }

            // SET UPO THE N/W FOR XDM OPERATIONS
            if (!setUpDefaultNetwork()) {
                if (logger.isActivated()) {
                    logger.debug("cannot setip XDM default network. skip XDM operations");
                }
                isProcessing = false;
                return false;
            }

            // set up userAgent from user profile
            USER_AGENT = ImsModule.IMS_USER_PROFILE.getUserAgent();
            if (isGBAxcapAuthenticationType) {
                USER_AGENT += " 3gpp-gba";
            }
            if (logger.isActivated()) {
                logger.debug("xcap AUthentication type :"
                        + isGBAxcapAuthenticationType + " ; USER_AGENT : "
                        + USER_AGENT);
            }

            // START XDM OPERATIONS
            HttpResponse response = getXcapDocuments();
            if ((response != null) && response.isSuccessfullResponse()) {
                // Analyze the XCAP directory
                InputSource input = new InputSource(new ByteArrayInputStream(
                        response.getContent()));
                XcapDirectoryParser parser = new XcapDirectoryParser(input);
                documents = parser.getDocuments();

                // Check RCS list document
                Folder folder = (Folder) documents.get("rls-services");
                if ((folder == null) || (folder.getEntry() == null)) {
                    if (logger.isActivated()) {
                        logger.debug("The rls-services document does not exist");
                    }

                    // Set RCS list document
                    setRcsList();
                } else {
                    if (logger.isActivated()) {
                        logger.debug("The rls-services document already exists");
                    }
                    /*
                     * HttpResponse res = getRcsList(); String PresRules = new
                     * String(res.getContent()); if (logger.isActivated()) {
                     * logger.debug("rcs-list : " + PresRules); }
                     */
                }

                // Check resource list document
                folder = (Folder) documents.get("resource-lists");
                if ((folder == null) || (folder.getEntry() == null)) {
                    if (logger.isActivated()) {
                        logger.debug("The resource-lists document does not exist");
                    }

                    // Set resource list document
                    setResourcesList();
                } else {
                    if (logger.isActivated()) {
                        logger.debug("The resource-lists document already exists");
                    }

                    /*
                     * HttpResponse res = getResourcesList(); String PresRules =
                     * new String(res.getContent()); if (logger.isActivated()) {
                     * logger.debug("resourceList : " + PresRules); }
                     */

                }

                // Check presence rules document
                folder = (Folder) documents
                        .get("org.openmobilealliance.pres-rules");
                if ((folder == null) || (folder.getEntry() == null)) {
                    if (logger.isActivated()) {
                        logger.debug("The org.openmobilealliance.pres-rules document does not exist");
                    }

                    // Set presence rules document
                    setPresenceRules();
                } else {
                    if (logger.isActivated()) {
                        logger.debug("The org.openmobilealliance.pres-rules document already exists");
                    }

                    HttpResponse res = getPresenceRules();
                    String PresRules = new String(res.getContent());
                    if (logger.isActivated()) {
                        logger.debug("pres-rules : " + PresRules);
                    }

                    // set presence rules
                    setPresenceRules();
                }

                // set granted contacts list
                grantedContacts.clear();
                xdmContactMap.clear();
                // reset all the contact count
                countBlockedContacts = 0;
                countGrantedContacts = 0;
                countRevokedContacts = 0;
            }
            // in case of no document
            else if ((response != null) && (response.getResponseCode() == 404)) { // as
                                                                                  // per
                                                                                  // Operator_OP08
                                                                                  // req
                if (logger.isActivated()) {
                    logger.debug("Document dont exists at XDM server. Put the documents for first time");
                }
                if (logger.isActivated()) {
                    logger.debug("initialize");
                }
                setRcsList();
                setResourcesList();
                // put pres rules document
                setPresenceRules();
            } else {
                isProcessing = false;
                return false;
            }
        } catch (Exception e) {
            if (logger.isActivated()) {
                logger.error("Can't parse the XCAP directory document", e);
            }
            isProcessing = false;
            return false;
        } finally {
            isProcessing = false;
        }
        return true;
    }

    /**
     * Get XCAP managed documents
     * 
     * @return Response
     */
    public HttpResponse getXcapDocuments() {
        try {
            if (logger.isActivated()) {
                logger.info("Get XCAP documents");
            }

            // URL
            String url = "/org.openmobilealliance.xcap-directory/users/"
                    // + (ImsModule.IMS_USER_PROFILE.getPublicUri())
                    + ImsModule.IMS_USER_PROFILE.getPublicUri()
                    + "/directory.xml";

            // Create the request
            HttpGetRequest request = new HttpGetRequest(url);

            // Send the request
            HttpResponse response = sendRequestToXDMS(request);
            if (response.isSuccessfullResponse()) {
                if (logger.isActivated()) {
                    logger.info("XCAP documents has been read with success");
                }
            } else {
                if (logger.isActivated()) {
                    logger.info("Can't read XCAP documents: "
                            + response.getResponseCode() + " error");
                }
            }
            return response;
        } catch (CoreException e) {
            if (logger.isActivated()) {
                logger.error("Can't read XCAP documents: unexpected exception",
                        e);
            }
            return null;
        }
    }

    /**
     * Get RCS list
     * 
     * @return Response
     */
    public HttpResponse getRcsList() {
        try {
            if (logger.isActivated()) {
                logger.info("Get RCS list");
            }

            // URL
            String url = "/rls-services/users/"
                    + (ImsModule.IMS_USER_PROFILE.getPublicUri()) + "/index";

            // Create the request
            HttpGetRequest request = new HttpGetRequest(url);

            // Send the request
            HttpResponse response = sendRequestToXDMS(request);
            if (response.isSuccessfullResponse()) {
                if (logger.isActivated()) {
                    logger.info("RCS list has been read with success");
                }
            } else {
                if (logger.isActivated()) {
                    logger.info("Can't read RCS list: "
                            + response.getResponseCode() + " error");
                }
            }
            return response;
        } catch (CoreException e) {
            if (logger.isActivated()) {
                logger.error("Can't read RCS list: unexpected exception", e);
            }
            return null;
        }
    }

    /**
     * Set RCS list
     * 
     * @return Response
     */
    public HttpResponse setRcsList() {
        try {
            if (logger.isActivated()) {
                logger.info("Set RCS list");
            }

            // URL
            String url = "/rls-services/users/"
                    + (ImsModule.IMS_USER_PROFILE.getPublicUri()) + "/index";

            // Content
            String user = ImsModule.IMS_USER_PROFILE.getPublicUri();
            String resList = xdmServerAddr + "/resource-lists/users/" + (user)
                    + "/index/~~/resource-lists/list%5B@name=%22rcs%22%5D";
            String content = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
                    + HttpUtils.CRLF
                    + "<rls-services xmlns=\"urn:ietf:params:xml:ns:rls-services\" xmlns:rl=\"urn:ietf:params:xml:ns:resource-lists\">"
                    + HttpUtils.CRLF + "<service uri=\"" + user
                    + ";pres-list=rcs\">" + HttpUtils.CRLF +

                    "<resource-list>" + resList + "</resource-list>"
                    + HttpUtils.CRLF +

                    "<packages>" + HttpUtils.CRLF
                    + " <package>presence</package>" + HttpUtils.CRLF
                    + "</packages>" + HttpUtils.CRLF +

                    "</service></rls-services>";

            // Create the request
            HttpPutRequest request = new HttpPutRequest(url, content,
                    "application/rls-services+xml");

            // Send the request
            HttpResponse response = sendRequestToXDMS(request);
            if (response.isSuccessfullResponse()) {
                if (logger.isActivated()) {
                    logger.info("RCS list has been set with success");
                }
            } else {
                if (logger.isActivated()) {
                    logger.info("Can't set RCS list: "
                            + response.getResponseCode() + " error");
                }
            }
            return response;
        } catch (CoreException e) {
            if (logger.isActivated()) {
                logger.error("Can't set RCS list: unexpected exception", e);
            }
            return null;
        }
    }

    /**
     * Get resources list
     * 
     * @return Response
     */
    public HttpResponse getResourcesList() {
        try {
            if (logger.isActivated()) {
                logger.info("Get resources list");
            }

            // URL
            String url = "/resource-lists/users/"
                    + (ImsModule.IMS_USER_PROFILE.getPublicUri()) + "/index";

            // Create the request
            HttpGetRequest request = new HttpGetRequest(url);

            // Send the request
            HttpResponse response = sendRequestToXDMS(request);
            if (response.isSuccessfullResponse()) {
                if (logger.isActivated()) {
                    logger.info("Resources list has been read with success");
                }
            } else {
                if (logger.isActivated()) {
                    logger.info("Can't read resources list: "
                            + response.getResponseCode() + " error");
                }
            }
            return response;
        } catch (CoreException e) {
            if (logger.isActivated()) {
                logger.error("Can't read resources list: unexpected exception",
                        e);
            }
            return null;
        }
    }

    /**
     * Set resources list
     * 
     * @return Response
     */
    public HttpResponse setResourcesList() {
        try {
            if (logger.isActivated()) {
                logger.info("Set resources list");
            }

            // URL
            String url = "/resource-lists/users/"
                    + (ImsModule.IMS_USER_PROFILE.getPublicUri()) + "/index";

            // Content
            String user = ImsModule.IMS_USER_PROFILE.getPublicUri();
            String resList = xdmServerAddr + "/resource-lists/users/" + (user)
                    + "/index/~~/resource-lists/list%5B";
            String content = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
                    + HttpUtils.CRLF
                    + "<resource-lists xmlns=\"urn:ietf:params:xml:ns:resource-lists\">"
                    + HttpUtils.CRLF +

                    "<list name=\"oma_buddylist\">" + HttpUtils.CRLF
                    + " <external anchor=\"" + resList
                    + "@name=%22rcs%22%5D\"/>" + HttpUtils.CRLF + "</list>"
                    + HttpUtils.CRLF +

                    "<list name=\"oma_grantedcontacts\">" + HttpUtils.CRLF
                    + " <external anchor=\"" + resList
                    + "@name=%22rcs%22%5D\"/>" + HttpUtils.CRLF + "</list>"
                    + HttpUtils.CRLF +

                    "<list name=\"oma_blockedcontacts\">" + HttpUtils.CRLF
                    + " <external anchor=\"" + resList
                    + "@name=%22rcs_blockedcontacts%22%5D\"/>" + HttpUtils.CRLF
                    + " <external anchor=\"" + resList
                    + "@name=%22rcs_revokedcontacts%22%5D\"/>" + HttpUtils.CRLF
                    + "</list>" + HttpUtils.CRLF +

                    "<list name=\"rcs\">" + HttpUtils.CRLF
                    + " <display-name>My presence buddies</display-name>"
                    + HttpUtils.CRLF + "</list>" + HttpUtils.CRLF +

                    "<list name=\"rcs_blockedcontacts\">" + HttpUtils.CRLF
                    + " <display-name>My blocked contacts</display-name>"
                    + HttpUtils.CRLF + "</list>" + HttpUtils.CRLF +

                    "<list name=\"rcs_revokedcontacts\">" + HttpUtils.CRLF
                    + " <display-name>My revoked contacts</display-name>"
                    + HttpUtils.CRLF + "</list>" + HttpUtils.CRLF +

                    "</resource-lists>";

            // Create the request
            HttpPutRequest request = new HttpPutRequest(url, content,
                    "application/resource-lists+xml");

            // Send the request
            HttpResponse response = sendRequestToXDMS(request);
            if (response.isSuccessfullResponse()) {
                if (logger.isActivated()) {
                    logger.info("Resources list has been set with success");
                }
            } else {
                if (logger.isActivated()) {
                    logger.info("Can't set resources list: "
                            + response.getResponseCode() + " error");
                }
            }
            return response;
        } catch (CoreException e) {
            if (logger.isActivated()) {
                logger.error("Can't set resources list: unexpected exception",
                        e);
            }
            return null;
        }
    }

    /**
     * Get presence rules
     * 
     * @return Response
     */
    public HttpResponse getPresenceRules() {
        try {
            if (logger.isActivated()) {
                logger.info("Get presence rules");
            }

            // URL
            String url = "/org.openmobilealliance.pres-rules/users/"
                    + (ImsModule.IMS_USER_PROFILE.getPublicUri())
                    + "/pres-rules";

            // Create the request
            HttpGetRequest request = new HttpGetRequest(url);

            // Send the request
            HttpResponse response = sendRequestToXDMS(request);
            if (response.isSuccessfullResponse()) {
                if (logger.isActivated()) {
                    logger.info("Get presence rules has been requested with success");
                }
            } else {
                if (logger.isActivated()) {
                    logger.info("Can't get the presence rules: "
                            + response.getResponseCode() + " error");
                }
            }
            return response;
        } catch (CoreException e) {
            if (logger.isActivated()) {
                logger.error(
                        "Can't get the presence rules: unexpected exception", e);
            }
            return null;
        }
    }

    /**
     * Set presence rules
     * 
     * @return Response
     */
    public HttpResponse setPresenceRules() {
        try {
            if (logger.isActivated()) {
                logger.info("Set presence rules");
            }

            // URL
            String url = "/org.openmobilealliance.pres-rules/users/"
                    + (ImsModule.IMS_USER_PROFILE.getPublicUri())
                    + "/pres-rules";

            // Content
            String user = ImsModule.IMS_USER_PROFILE.getPublicUri();
            String blockedList = xdmServerAddr
                    + "/resource-lists/users/"
                    + user
                    + "/index/~~/resource-lists/list%5B@name=%22oma_blockedcontacts%22%5D";
            String grantedList = xdmServerAddr
                    + "/resource-lists/users/"
                    + user
                    + "/index/~~/resource-lists/list%5B@name=%22oma_grantedcontacts%22%5D";
            String content = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
                    + HttpUtils.CRLF
                    + "<cr:ruleset xmlns:ocp=\"urn:oma:xml:xdm:common-policy\" xmlns:pr=\"urn:ietf:params:xml:ns:pres-rules\" xmlns:cr=\"urn:ietf:params:xml:ns:common-policy\">"
                    + HttpUtils.CRLF
                    +

                    /*
                     * "<cr:rule id=\"wp_prs_allow_own\">" + HttpUtils.CRLF +
                     * " <cr:conditions>" + HttpUtils.CRLF +
                     * "  <cr:identity><cr:one id=\"" +
                     * ImsModule.IMS_USER_PROFILE.getPublicUri() +
                     * "\"/></cr:identity>" + HttpUtils.CRLF +
                     * " </cr:conditions>" + HttpUtils.CRLF +
                     * " <cr:actions><pr:sub-handling>allow</pr:sub-handling></cr:actions>"
                     * + HttpUtils.CRLF + " <cr:transformations>" +
                     * HttpUtils.CRLF +
                     * "  <pr:provide-services><pr:all-services/></pr:provide-services>"
                     * + HttpUtils.CRLF +
                     * "  <pr:provide-persons><pr:all-persons/></pr:provide-persons>"
                     * + HttpUtils.CRLF +
                     * "  <pr:provide-devices><pr:all-devices/></pr:provide-devices>"
                     * + HttpUtils.CRLF + "  <pr:provide-all-attributes/>" +
                     * HttpUtils.CRLF + " </cr:transformations>" +
                     * HttpUtils.CRLF + "</cr:rule>" + HttpUtils.CRLF +
                     */
                    "<cr:rule id=\"rcs_allow_services_anonymous\">"
                    + HttpUtils.CRLF
                    + " <cr:conditions><ocp:anonymous-request/></cr:conditions>"
                    + HttpUtils.CRLF
                    + " <cr:actions><pr:sub-handling>allow</pr:sub-handling></cr:actions>"
                    + HttpUtils.CRLF
                    + " <cr:transformations>"
                    + HttpUtils.CRLF
                    + "  <pr:provide-services><pr:all-services/></pr:provide-services>"
                    + HttpUtils.CRLF + "  <pr:provide-all-attributes/>"
                    + HttpUtils.CRLF + " </cr:transformations>"
                    + HttpUtils.CRLF + "</cr:rule>" + HttpUtils.CRLF
                    /*
                     * +
                     * 
                     * "<cr:rule id=\"wp_prs_unlisted\">" + HttpUtils.CRLF +
                     * " <cr:conditions><ocp:other-identity/></cr:conditions>" +
                     * HttpUtils.CRLF +
                     * " <cr:actions><pr:sub-handling>confirm</pr:sub-handling></cr:actions>"
                     * + HttpUtils.CRLF + "</cr:rule>" + HttpUtils.CRLF +
                     * 
                     * "<cr:rule id=\"wp_prs_grantedcontacts\">" +
                     * HttpUtils.CRLF + " <cr:conditions>" + HttpUtils.CRLF +
                     * " <ocp:external-list>" + HttpUtils.CRLF +
                     * "  <ocp:entry anc=\"" + grantedList + "\"/>" +
                     * HttpUtils.CRLF + " </ocp:external-list>" + HttpUtils.CRLF
                     * + " </cr:conditions>" + HttpUtils.CRLF +
                     * " <cr:actions><pr:sub-handling>allow</pr:sub-handling></cr:actions>"
                     * + HttpUtils.CRLF + " <cr:transformations>" +
                     * HttpUtils.CRLF +
                     * "   <pr:provide-services><pr:all-services/></pr:provide-services>"
                     * + HttpUtils.CRLF +
                     * "   <pr:provide-persons><pr:all-persons/></pr:provide-persons>"
                     * + HttpUtils.CRLF +
                     * "   <pr:provide-devices><pr:all-devices/></pr:provide-devices>"
                     * + HttpUtils.CRLF + "   <pr:provide-all-attributes/>" +
                     * HttpUtils.CRLF + " </cr:transformations>" +
                     * HttpUtils.CRLF + "</cr:rule>" + HttpUtils.CRLF +
                     * 
                     * "<cr:rule id=\"wp_prs_blockedcontacts\">" +
                     * HttpUtils.CRLF + " <cr:conditions>" + HttpUtils.CRLF +
                     * "  <ocp:external-list>" + HttpUtils.CRLF +
                     * "  <ocp:entry anc=\"" + blockedList + "\"/>" +
                     * HttpUtils.CRLF + " </ocp:external-list>" + HttpUtils.CRLF
                     * + " </cr:conditions>" + HttpUtils.CRLF +
                     * " <cr:actions><pr:sub-handling>block</pr:sub-handling></cr:actions>"
                     * + HttpUtils.CRLF + "</cr:rule>" + HttpUtils.CRLF
                     */
                    + "</cr:ruleset>";

            // Create the request
            HttpPutRequest request = new HttpPutRequest(url, content,
                    "application/auth-policy+xml");

            // Send the request
            HttpResponse response = sendRequestToXDMS(request);
            if (response.isSuccessfullResponse()) {
                if (logger.isActivated()) {
                    logger.info("Presence rules has been set with success");
                }
            } else {
                if (logger.isActivated()) {
                    logger.info("Can't set presence rules: "
                            + response.getResponseCode() + " error");
                }
            }
            return response;
        } catch (CoreException e) {
            if (logger.isActivated()) {
                logger.error("Can't set presence rules: unexpected exception",
                        e);
            }
            return null;
        }
    }

    /**
     * Add a contact to the granted contacts list
     * 
     * @param contact
     *            Contact
     * @return Response
     */
    
    public HttpResponse addContactToGrantedList(String contact) {
        boolean isInitialize = StartService.getXdmInitializeFlag(AndroidFactory.getApplicationContext());
        if (logger.isActivated()) {
            logger.info("addContactToGrantedList:"+ contact + "isInitialize value:" + isInitialize);
        }
        if(isInitialize == false){
            return null;
        }
        if(contact!=null){ 
                XDMContact cont = new XDMContact(contact);
                List<XDMContact> contactList = new ArrayList<XDMContact>();
                contactList.add(cont);
                
                XCAPOperation addContactOperation = new XCAPOperation(contactList, operationType.ADDITION_CONTACT_GRANTED_LIST);
                return ((HttpResponse)addContactOperation.execute());
        }else{
            return null;
        }
    }
    
    
    public HttpResponse addContactToGrantedListOperation(String contact) {
        try {
            if (logger.isActivated()) {
                logger.info("Add " + contact + " to granted list");
            }

            if (getCountAllContacts() >= MAX_COUNT_XDMS_CONTACTS) {
                if (logger.isActivated()) {
                    logger.info("contacts count exceed MAX_COUNT_XDMS_CONTACTS");
                }
                return null;
            }

            // URL
            String url = "/resource-lists/users/"
                    + (ImsModule.IMS_USER_PROFILE.getPublicUri())
                    + "/index/~~/resource-lists/list%5B@name=%22rcs%22%5D/entry%5B@uri=%22"
                    + (contact) + "%22%5D";

            // Content
            String content = "<entry uri='" + contact+"'>";
            String display_name = "";
            
            display_name =ContactsManager.getInstance().getContactDisplayName(contact);
            if(display_name!=null){
                content += "<display-name>"+display_name+"</display-name>";
            }else{
                content += "<display-name>"+PhoneUtils.extractNumberFromUri(contact)+"</display-name>";
            }
            content+= "</entry>";
            
            if (logger.isActivated()) {
                logger.info("Add " + contact + " with display_name "+display_name+" to granted list");
            }
            
            // Create the request
            HttpPutRequest request = new HttpPutRequest(url, content,
                    "application/xcap-el+xml");

            // Send the request
            HttpResponse response = sendRequestToXDMS(request);
            if (response.isSuccessfullResponse()) {
                if (logger.isActivated()) {
                    logger.info(contact
                            + " has been added with success to granted list");
                }

                addGrantedContactInLocalList(contact);
                
            } else {
                if (logger.isActivated()) {
                    logger.info("Can't add " + contact + " to granted list: "
                            + response.getResponseCode() + " error");
                }
            }
            return response;
        } catch (Exception e) {
            if (logger.isActivated()) {
                logger.error("Can't add " + contact
                        + " to granted list: unexpected exception", e);
            }
            return null;
        }
    }

    /**
     * Add list of contacts to the granted contacts list
     * 
     * @param contact
     *            Contact
     * @return Response
     */
    
    public HttpResponse addContactToGrantedList(List<String> contactList) {
        if (logger.isActivated()) {
            logger.info("addContactToGrantedList " + contactList + " to granted list");
        }
        if(contactList!=null && contactList.size() > 0){ 
                List<XDMContact> xdmcontactList = new ArrayList<XDMContact>();
                for (String contact : contactList){
                    XDMContact cont = new XDMContact(contact);
                    xdmcontactList.add(cont);
                }
                XCAPOperation addContactOperation = new XCAPOperation(xdmcontactList, operationType.ADDITION_CONTACT_GRANTED_LIST_ALL);
                return ((HttpResponse)addContactOperation.execute());
        }else{
            return null;
        }
    }

    public HttpResponse addContactToGrantedListOperation(List<XDMContact> contactList) {
        try {
            if (logger.isActivated()) {
                logger.info("Add list:" + contactList + " to granted list" + contactList.size());
            }

            if (getCountAllContacts() >= MAX_COUNT_XDMS_CONTACTS) {
                if (logger.isActivated()) {
                    logger.info("contacts count exceed MAX_COUNT_XDMS_CONTACTS");
                }
                return null;
            }
            if(contactList == null){
                return null;
            }
            // URL
            String url = "/resource-lists/users/"
                    + (ImsModule.IMS_USER_PROFILE.getPublicUri())
                    + "/index/~~/resource-lists/list%5B@name=%22rcs%22%5D";

            // Content
            String content = "";
            content += "<list name=\"rcs\">";
            content += "<display-name>"+"rcs"+"</display-name>";
            for(XDMContact contact : contactList){
                content += "<entry uri=\"" + contact.getContact()+"\">";
                String display_name = "";
                
                display_name =ContactsManager.getInstance().getContactDisplayName(contact.getContact());
                if(display_name!=null){
                    content += "<display-name>"+display_name+"</display-name>";
                }else{
                    content += "<display-name>"+PhoneUtils.extractNumberFromUri(contact.getContact())+"</display-name>";
                }
                content+= "</entry>";
                
                if (logger.isActivated()) {
                    logger.info("Add " + contact.getContact() + " with display_name "+display_name+" to granted list");
                }
            }
            content+= "</list>";
            if (logger.isActivated()) {
                logger.info("content is " + content);
            }
            // Create the request
            HttpPutRequest request = new HttpPutRequest(url, content,
                    "application/xcap-el+xml");

            // Send the request
            HttpResponse response = sendRequestToXDMS(request);
            if (response.isSuccessfullResponse()) {
                if (logger.isActivated()) {
                    logger.info(contactList
                            + " has been added with success to granted list");
                }

                for(XDMContact contact : contactList){
                   addGrantedContactInLocalList(contact.getContact());
                }
                
            } else {
                if (logger.isActivated()) {
                    logger.info("Can't add " + contactList + " to granted list: "
                            + response.getResponseCode() + " error");
                }
            }
            return response;
        } catch (Exception e) {
            if (logger.isActivated()) {
                logger.error("Can't add " + contactList
                        + " to granted list: unexpected exception", e);
            }
            return null;
        }
    }

    /**
     * Remove a contact from the granted contacts list
     * 
     * @param contact
     *            Contact
     * @return Response
     */
    public HttpResponse removeContactFromGrantedList(String contact) {
        if(contact!=null){ 
                XDMContact cont = new XDMContact(contact);
                List<XDMContact> contactList = new ArrayList<XDMContact>();
                contactList.add(cont);
                
                XCAPOperation addContactOperation = new XCAPOperation(contactList, operationType.DELETE_CONTACT_GRANTED_LIST);
                return ((HttpResponse)addContactOperation.execute());
        }else{
            return null;
        }
    }
    
    public HttpResponse removeContactFromGrantedListOperation(String contact) {
        try {
            if (logger.isActivated()) {
                logger.info("Remove " + contact + " from granted list");
            }

            // URL
            String url = "/resource-lists/users/"
                    + (ImsModule.IMS_USER_PROFILE.getPublicUri())
                    + "/index/~~/resource-lists/list%5B@name=%22rcs%22%5D/entry%5B@uri=%22"
                    + (contact) + "%22%5D";

            // Create the request
            HttpDeleteRequest request = new HttpDeleteRequest(url);

            // Send the request
            HttpResponse response = sendRequestToXDMS(request);
            if (response.isSuccessfullResponse()) {
                if (logger.isActivated()) {
                    logger.info(contact
                            + " has been removed with success from granted list");
                }

               removeGrantedContactfromLocalList(contact); 
            } else {
                if (logger.isActivated()) {
                    logger.info("Can't remove " + contact
                            + " from granted list: "
                            + response.getResponseCode() + " error");
                }
            }
            return response;
        } catch (CoreException e) {
            if (logger.isActivated()) {
                logger.error("Can't remove " + contact
                        + " from granted list: unexpected exception", e);
            }
            return null;
        }
    }

    /**
     * Returns the list of granted contacts
     * 
     * @return List
     */
    public List<String> getGrantedContacts() {
        List<String> result = new ArrayList<String>();
        try {
            if (logger.isActivated()) {
                logger.info("Get granted contacts list");
            }

            // URL
            String url = "/resource-lists/users/"
                    + (ImsModule.IMS_USER_PROFILE.getPublicUri())
                    + "/index/~~/resource-lists/list%5B@name=%22rcs%22%5D";

            // Create the request
            HttpGetRequest request = new HttpGetRequest(url);

            // Send the request
            HttpResponse response = sendRequestToXDMS(request);
            if (response.isSuccessfullResponse()) {
                if (logger.isActivated()) {
                    logger.info("Granted contacts list has been read with success");
                }

                if(response.getContent()!=null)
                {
                    // Parse response
                    InputSource input = new InputSource(new ByteArrayInputStream(
                            response.getContent()));
                    XcapResponseParser parser = new XcapResponseParser(input);
                    
                    //GET THE 
                    result = parser.getUris();
                    xdmContactMap = parser.getXDMContactMap();
                }
                
                // set granted contacts
                grantedContacts = result;
                // update the count of granted contacts
                countGrantedContacts = result.size();
            } else {
                if (logger.isActivated()) {
                    logger.info("Can't get granted contacts list: "
                            + response.getResponseCode() + " error");
                }
            }
        } catch (Exception e) {
            if (logger.isActivated()) {
                logger.error(
                        "Can't get granted contacts list: unexpected exception",
                        e);
            }
        }
        return result;
    }

    /**
     * Add a contact to the blocked contacts list
     * 
     * @param contact
     *            Contact
     * @return Response
     */
    public HttpResponse addContactToBlockedList(String contact) {
        if(contact!=null){ 
            XDMContact cont = new XDMContact(contact);
            List<XDMContact> contactList = new ArrayList<XDMContact>();
            contactList.add(cont);
            
            XCAPOperation addContactOperation = new XCAPOperation(contactList, operationType.ADDITION_CONTACT_BLOCKED_LIST);
            return ((HttpResponse)addContactOperation.execute());
    }else{
        return null;
    }
    }
    
    public HttpResponse addContactToBlockedListOperation(String contact) {
        try {
            if (logger.isActivated()) {
                logger.info("Add " + contact + " to blocked list");
            }

            if (getCountAllContacts() >= MAX_COUNT_XDMS_CONTACTS) {
                if (logger.isActivated()) {
                    logger.info("contacts count exceed MAX_COUNT_XDMS_CONTACTS. Can't add contact");
                }
                return null;
            }

            // URL
            String url = "/resource-lists/users/"
                    + (ImsModule.IMS_USER_PROFILE.getPublicUri())
                    + "/index/~~/resource-lists/list%5B@name=%22rcs_blockedcontacts%22%5D/entry%5B@uri=%22"
                    + (contact) + "%22%5D";

            // Content
            String content = "<entry uri='" + contact + "'></entry>";

            // Create the request
            HttpPutRequest request = new HttpPutRequest(url, content,
                    "application/xcap-el+xml");

            // Send the request
            HttpResponse response = sendRequestToXDMS(request);
            if (response.isSuccessfullResponse()) {
                if (logger.isActivated()) {
                    logger.info(contact
                            + " has been added with success to blocked list");
                }

                // update count
                countBlockedContacts++;
            } else {
                if (logger.isActivated()) {
                    logger.info("Can't add " + contact + " to granted list: "
                            + response.getResponseCode() + " error");
                }
                
            }
            return response;
        } catch (CoreException e) {
            if (logger.isActivated()) {
                logger.error("Can't add " + contact
                        + " to blocked list: unexpected exception", e);
            }
            return null;
        }
    }

    /**
     * Remove a contact from the blocked contacts list
     * 
     * @param contact
     *            Contact
     * @return Response
     */
    public HttpResponse removeContactFromBlockedList(String contact) {
        if(contact!=null){ 
            XDMContact cont = new XDMContact(contact);
            List<XDMContact> contactList = new ArrayList<XDMContact>();
            contactList.add(cont);
            
            XCAPOperation addContactOperation = new XCAPOperation(contactList, operationType.DELETE_CONTACT_BLOCKED_LIST);
            return ((HttpResponse)addContactOperation.execute());
    }else{
        return null;
    }
    }
    
    
    public HttpResponse removeContactFromBlockedListOperation(String contact) {
        try {
            if (logger.isActivated()) {
                logger.info("Remove " + contact + " from blocked list");
            }

            // URL
            String url = "/resource-lists/users/"
                    + (ImsModule.IMS_USER_PROFILE.getPublicUri())
                    + "/index/~~/resource-lists/list%5B@name=%22rcs_blockedcontacts%22%5D/entry%5B@uri=%22"
                    + (contact) + "%22%5D";

            // Create the request
            HttpDeleteRequest request = new HttpDeleteRequest(url);

            // Send the request
            HttpResponse response = sendRequestToXDMS(request);
            if (response.isSuccessfullResponse()) {
                if (logger.isActivated()) {
                    logger.info(contact
                            + " has been removed with success from blocked list");
                }

                // update count
                countBlockedContacts--;
            } else {
                if (logger.isActivated()) {
                    logger.info("Can't remove " + contact
                            + " from blocked list: "
                            + response.getResponseCode() + " error");
                }
            }
            return response;
        } catch (CoreException e) {
            if (logger.isActivated()) {
                logger.error("Can't remove " + contact
                        + " from blocked list: unexpected exception", e);
            }
            return null;
        }
    }

    /**
     * Returns the list of blocked contacts
     * 
     * @return List
     */
    public List<String> getBlockedContacts() {
        List<String> result = new ArrayList<String>();
        try {
            if (logger.isActivated()) {
                logger.info("Get blocked contacts list");
            }

            // URL
            String url = "/resource-lists/users/"
                    + (ImsModule.IMS_USER_PROFILE.getPublicUri())
                    + "/index/~~/resource-lists/list%5B@name=%22rcs_blockedcontacts%22%5D";

            // Create the request
            HttpGetRequest request = new HttpGetRequest(url);

            // Send the request
            HttpResponse response = sendRequestToXDMS(request);
            if (response.isSuccessfullResponse()) {
                if (logger.isActivated()) {
                    logger.info("Blocked contacts list has been read with success");
                }

                if (response.getContent()!=null) {
                    // Parse response
                    InputSource input = new InputSource(
                            new ByteArrayInputStream(response.getContent()));
                    XcapResponseParser parser = new XcapResponseParser(input);
                    result = parser.getUris();
                }
                
                // update the count of blocked contacts
                countBlockedContacts = result.size();
            } else {
                if (logger.isActivated()) {
                    logger.info("Can't get blocked contacts list: "
                            + response.getResponseCode() + " error");
                }
            }
        } catch (Exception e) {
            if (logger.isActivated()) {
                logger.error(
                        "Can't get blocked contacts list: unexpected exception",
                        e);
            }
        }
        return result;
    }

    /**
     * Add a contact to the revoked contacts list
     * 
     * @param contact
     *            Contact
     * @return Response
     */
    public HttpResponse addContactToRevokedList(String contact) {
        if(contact!=null){ 
            XDMContact cont = new XDMContact(contact);
            List<XDMContact> contactList = new ArrayList<XDMContact>();
            contactList.add(cont);
            
            XCAPOperation addContactOperation = new XCAPOperation(contactList, operationType.ADDITION_CONTACT_REVOKE_LIST);
            return ((HttpResponse)addContactOperation.execute());
    }else{
        return null;
    }
    }
    
    public HttpResponse addContactToRevokedListOperation(String contact) {
        try {
            if (logger.isActivated()) {
                logger.info("Add " + contact + " to revoked list");
            }

            if (getCountAllContacts() >= MAX_COUNT_XDMS_CONTACTS) {
                if (logger.isActivated()) {
                    logger.info("contacts count exceed MAX_COUNT_XDMS_CONTACTS.Can't add contacts");
                }
                return null;
            }

            // URL
            String url = "/resource-lists/users/"
                    + (ImsModule.IMS_USER_PROFILE.getPublicUri())
                    + "/index/~~/resource-lists/list%5B@name=%22rcs_revokedcontacts%22%5D/entry%5B@uri=%22"
                    + (contact) + "%22%5D";

            // Content
            String content = "<entry uri='" + contact + "'></entry>";

            // Create the request
            HttpPutRequest request = new HttpPutRequest(url, content,
                    "application/xcap-el+xml");

            // Send the request
            HttpResponse response = sendRequestToXDMS(request);
            if (response.isSuccessfullResponse()) {
                if (logger.isActivated()) {
                    logger.info(contact
                            + " has been added with success to revoked list");
                }

                // update the count of revoked contacts
                countRevokedContacts++;
            } else {
                if (logger.isActivated()) {
                    logger.info("Can't add " + contact + " to revoked list: "
                            + response.getResponseCode() + " error");
                }
            }
            return response;
        } catch (CoreException e) {
            if (logger.isActivated()) {
                logger.error("Can't add " + contact
                        + " to revoked list: unexpected exception", e);
            }
            return null;
        }
    }

    /**
     * Remove a contact from the revoked contacts list
     * 
     * @param contact
     *            Contact
     * @return Response
     */
    public HttpResponse removeContactFromRevokedList(String contact) {
        if(contact!=null){ 
            XDMContact cont = new XDMContact(contact);
            List<XDMContact> contactList = new ArrayList<XDMContact>();
            contactList.add(cont);
            
            XCAPOperation addContactOperation = new XCAPOperation(contactList, operationType.DELETE_CONTACT_REVOKE_LIST);
            return ((HttpResponse)addContactOperation.execute());
    }else{
        return null;
    }
    }
    
    public HttpResponse removeContactFromRevokedListOperation(String contact) {
        try {
            if (logger.isActivated()) {
                logger.info("Remove " + contact + " from revoked list");
            }

            // URL
            String url = "/resource-lists/users/"
                    + (ImsModule.IMS_USER_PROFILE.getPublicUri())
                    + "/index/~~/resource-lists/list%5B@name=%22rcs_revokedcontacts%22%5D/entry%5B@uri=%22"
                    + (contact) + "%22%5D";

            // Create the request
            HttpDeleteRequest request = new HttpDeleteRequest(url);

            // Send the request
            HttpResponse response = sendRequestToXDMS(request);
            if (response.isSuccessfullResponse()) {
                if (logger.isActivated()) {
                    logger.info(contact
                            + " has been removed with success from revoked list");
                }
                // update count
                countRevokedContacts--;
            } else {
                if (logger.isActivated()) {
                    logger.info("Can't remove " + contact
                            + " from revoked list: "
                            + response.getResponseCode() + " error");
                }
            }
            return response;
        } catch (CoreException e) {
            if (logger.isActivated()) {
                logger.error("Can't remove " + contact
                        + " from revoked list: unexpected exception", e);
            }
            return null;
        }
    }

    /**
     * Returns the list of revoked contacts
     * 
     * @return List
     */
    public List<String> getRevokedContacts() {
        List<String> result = new ArrayList<String>();
        try {
            if (logger.isActivated()) {
                logger.info("Get revoked contacts list");
            }

            // URL
            String url = "/resource-lists/users/"
                    + (ImsModule.IMS_USER_PROFILE.getPublicUri())
                    + "/index/~~/resource-lists/list%5B@name=%22rcs_revokedcontacts%22%5D";

            // Create the request
            HttpGetRequest request = new HttpGetRequest(url);

            // Send the request
            HttpResponse response = sendRequestToXDMS(request);
            if (response.isSuccessfullResponse()) {
                if (logger.isActivated()) {
                    logger.info("Revoked contacts list has been read with success");
                }

                // Parse response
                InputSource input = new InputSource(new ByteArrayInputStream(
                        response.getContent()));
                XcapResponseParser parser = new XcapResponseParser(input);
                result = parser.getUris();

                // update the count of revoked contacts
                countRevokedContacts = result.size();
            } else {
                if (logger.isActivated()) {
                    logger.info("Can't get revoked contacts list: "
                            + response.getResponseCode() + " error");
                }
            }
        } catch (Exception e) {
            if (logger.isActivated()) {
                logger.error(
                        "Can't get revoked contacts list: unexpected exception",
                        e);
            }
        }
        return result;
    }

    /**
     * Returns the photo icon URL
     * 
     * @return URL
     */
    public String getEndUserPhotoIconUrl() {
        return xdmServerAddr
                + "/org.openmobilealliance.pres-content/users/"
                + HttpUtils
                        .encodeURL(ImsModule.IMS_USER_PROFILE.getPublicUri())
                + "/oma_status-icon/rcs_status_icon";
    }

    /**
     * Upload the end user photo
     * 
     * @param photo
     *            Photo icon
     * @return Response
     */
    public HttpResponse uploadEndUserPhoto(PhotoIcon photo) {
        try {
            if (logger.isActivated()) {
                logger.info("Upload the end user photo");
            }

            // Content
            String data = Base64.encodeBase64ToString(photo.getContent());
            String content = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
                    + HttpUtils.CRLF
                    + "<content xmlns=\"urn:oma:xml:prs:pres-content\">"
                    + HttpUtils.CRLF + "<mime-type>" + photo.getType()
                    + "</mime-type>" + HttpUtils.CRLF
                    + "<encoding>base64</encoding>" + HttpUtils.CRLF + "<data>"
                    + data + "</data>" + HttpUtils.CRLF + "</content>";

            // URL
            String url = "/org.openmobilealliance.pres-content/users/"
                    + (ImsModule.IMS_USER_PROFILE.getPublicUri())
                    + "/oma_status-icon/rcs_status_icon";

            // Create the request
            HttpPutRequest request = new HttpPutRequest(url, content,
                    "application/vnd.oma.pres-content+xml");

            // Send the request
            HttpResponse response = sendRequestToXDMS(request);
            if (response.isSuccessfullResponse()) {
                if (logger.isActivated()) {
                    logger.info("Photo has been uploaded with success");
                }
            } else {
                if (logger.isActivated()) {
                    logger.info("Can't upload the photo: "
                            + response.getResponseCode() + " error");
                }
            }
            return response;
        } catch (CoreException e) {
            if (logger.isActivated()) {
                logger.error("Can't upload the photo: unexpected exception", e);
            }
            return null;
        }
    }

    /**
     * Delete the end user photo
     * 
     * @param photo
     *            Photo icon
     * @return Response
     */
    public HttpResponse deleteEndUserPhoto() {
        try {
            if (logger.isActivated()) {
                logger.info("Delete the end user photo");
            }

            // URL
            String url = "/org.openmobilealliance.pres-content/users/"
                    + (ImsModule.IMS_USER_PROFILE.getPublicUri())
                    + "/oma_status-icon/rcs_status_icon";

            // Create the request
            HttpDeleteRequest request = new HttpDeleteRequest(url);

            // Send the request
            HttpResponse response = sendRequestToXDMS(request);
            if (response.isSuccessfullResponse()) {
                if (logger.isActivated()) {
                    logger.info("Photo has been deleted with success");
                }
            } else {
                if (logger.isActivated()) {
                    logger.info("Can't delete the photo: "
                            + response.getResponseCode() + " error");
                }
            }
            return response;
        } catch (CoreException e) {
            if (logger.isActivated()) {
                logger.error("Can't delete the photo: unexpected exception", e);
            }
            return null;
        }
    }

    /**
     * Download photo of a remote contact
     * 
     * @param url
     *            URL of the photo to be downloaded
     * @param etag
     *            Etag of the photo
     * @return Icon data as photo icon
     */
    public PhotoIcon downloadContactPhoto(String url, String etag) {
        try {
            if (logger.isActivated()) {
                logger.info("Download the photo at " + url);
            }

            // Remove the beginning of the URL
            url = url.substring(url
                    .indexOf("/org.openmobilealliance.pres-content"));

            // Create the request
            HttpGetRequest request = new HttpGetRequest(url);

            // Send the request
            HttpResponse response = sendRequestToXDMS(request);
            if (response.isSuccessfullResponse()) {
                if (logger.isActivated()) {
                    logger.info("Download photo with success");
                }

                // Parse response
                InputSource input = new InputSource(new ByteArrayInputStream(
                        response.getContent()));
                XcapPhotoIconResponseParser parser = new XcapPhotoIconResponseParser(
                        input);

                // Return data
                byte[] data = parser.getData();
                if (data != null) {
                    if (logger.isActivated()) {
                        logger.debug("Received photo: encoding="
                                + parser.getEncoding() + ", mime="
                                + parser.getMime() + ", encoded size="
                                + data.length);
                    }
                    byte[] dataArray = Base64.decodeBase64(data);

                    // Create a bitmap from the received photo data
                    Bitmap bitmap = BitmapFactory.decodeByteArray(dataArray, 0,
                            dataArray.length);
                    if (bitmap != null) {
                        if (logger.isActivated()) {
                            logger.debug("Photo width=" + bitmap.getWidth()
                                    + " height=" + bitmap.getHeight());
                        }

                        return new PhotoIcon(dataArray, bitmap.getWidth(),
                                bitmap.getHeight(), etag);
                    } else {
                        return null;
                    }
                } else {
                    if (logger.isActivated()) {
                        logger.warn("Can't download the photo: photo is null");
                    }
                    return null;
                }
            } else {
                if (logger.isActivated()) {
                    logger.warn("Can't download the photo: "
                            + response.getResponseCode() + " error");
                }
                return null;
            }
        } catch (Exception e) {
            if (logger.isActivated()) {
                logger.error("Can't download the photo: unexpected exception",
                        e);
            }
            return null;
        }
    }

    /**
     * Set presence info
     * 
     * @param info
     *            Presence info
     * @return Response
     */
    public HttpResponse setPresenceInfo(String info) {
        try {
            if (logger.isActivated()) {
                logger.info("Update presence info");
            }

            // URL
            String url = "/pidf-manipulation/users/"
                    + (ImsModule.IMS_USER_PROFILE.getPublicUri())
                    + "/perm-presence";

            // Create the request
            HttpPutRequest request = new HttpPutRequest(url, info,
                    "application/pidf+xml");

            // Send the request
            HttpResponse response = sendRequestToXDMS(request);
            if (response.isSuccessfullResponse()) {
                if (logger.isActivated()) {
                    logger.info("Presence info has been updated with succes");
                }
            } else {
                if (logger.isActivated()) {
                    logger.info("Can't update the presence info: "
                            + response.getResponseCode() + " error");
                }
            }
            return response;
        } catch (CoreException e) {
            if (logger.isActivated()) {
                logger.error(
                        "Can't update the presence info: unexpected exception",
                        e);
            }
            return null;
        }
    }

    // get the count of all the contacts stored at XCAP server
    private int getCountAllContacts() {
        return (countGrantedContacts + countBlockedContacts + countRevokedContacts);
    }

    public void deInitialize() {
        // release the network
        if (mActiveNetworkConnection != null) {
            mActiveNetworkConnection.releaseNetwork();
            mActiveNetworkConnection = null;
        }

        // clear granted contacts
        grantedContacts.clear();
        xdmContactMap.clear();
        countGrantedContacts =0;
        countBlockedContacts  =0;
        countRevokedContacts =0;

    }

    private boolean setUpDefaultNetwork() {
        boolean status = false;
        if (mActiveNetworkConnection == null) {
            try {
                mActiveNetworkConnection = new AndroidDefaultNetwork(
                        AndroidFactory.getApplicationContext());

                // long subID= (long)1 ;//LauncherUtils.getsubId();
                long subID = (long) SubscriptionManager.getDefaultDataSubscriptionId();
                mActiveNetworkConnection.acquireNetwork(subID);

                status = true;
            } catch (NetworkException e) {

                status = false;
                // TODO Auto-generated catch block
                e.printStackTrace();
            }
        } else {
            status = true;
        }

        return status;
    }

    public List<String> getGrantedContactsList() {
        return grantedContacts;
    }
    
    
    
    private void addGrantedContactInLocalList(String contact){
        if (logger.isActivated()) {
            logger.info("addGrantedContactInLocalList:" + contact);
        }
        if(!grantedContacts.contains(contact)){
            grantedContacts.add(contact);
            countGrantedContacts++;
            //update the hastable
            XDMContact xdmContact = new XDMContact(contact);
            
            String displayName = ContactsManager.getInstance().getContactDisplayName(contact);
            if(displayName!=null && displayName!="")
            {
                xdmContact.setDisplayName(displayName);
            }
            xdmContactMap.put(contact, xdmContact);
        }
    }
    
    private void removeGrantedContactfromLocalList(String contact)
    {
        if(grantedContacts.contains(contact)){
            grantedContacts.remove(contact);
            countGrantedContacts--;
            
            xdmContactMap.remove(contact);
        }
    }
    
    /**
     * 
     * @param contact : contact with SIP URI
     */
    public XDMContact getXDMContactDetails(String contact){
        if(contact!=null && contact!=""){
            return xdmContactMap.get(contact);
        }else{
            return null;
        }
    }
    
    
    
    
}