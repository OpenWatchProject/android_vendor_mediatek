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

package com.orangelabs.rcs.core.ims.service.im.filetransfer.http;

import org.apache.http.HeaderIterator;

import java.io.IOException;
import java.net.MalformedURLException;
import java.net.URL;

import java.util.Enumeration;
import java.net.SocketException;
import java.net.NetworkInterface;
import android.text.format.Formatter;

import org.apache.http.Header;
import org.apache.http.HttpHost;
import org.apache.http.HttpResponse;
import org.apache.http.HttpVersion;
import org.apache.http.client.ClientProtocolException;
import org.apache.http.client.CookieStore;
import org.apache.http.client.methods.HttpRequestBase;
import org.apache.http.client.protocol.ClientContext;
import org.apache.http.conn.ClientConnectionManager;
import org.apache.http.conn.params.ConnManagerPNames;
import org.apache.http.conn.params.ConnPerRouteBean;
import org.apache.http.conn.params.ConnRoutePNames;
import org.apache.http.conn.scheme.PlainSocketFactory;
import org.apache.http.conn.scheme.Scheme;
import org.apache.http.conn.scheme.SchemeRegistry;
import org.apache.http.impl.client.BasicCookieStore;
import org.apache.http.impl.client.DefaultHttpClient;
import org.apache.http.impl.conn.SingleClientConnManager;
import org.apache.http.params.BasicHttpParams;
import org.apache.http.params.HttpParams;
import org.apache.http.params.HttpProtocolParams;
import org.apache.http.protocol.BasicHttpContext;
import org.apache.http.protocol.HttpContext;

import android.content.Context;
import android.net.ConnectivityManager;
import android.net.NetworkInfo;
import android.net.Proxy;

import com.orangelabs.rcs.platform.AndroidFactory;
import com.orangelabs.rcs.provider.settings.RcsSettings;
import com.orangelabs.rcs.utils.logger.Logger;
import java.net.InetAddress;


/**
 * Abstract HTTP transfer manager
 *
 * @author vfml3370
 */
public abstract class HttpTransferManager {
    /**
     * Max chunk size
     */
    public static final int CHUNK_MAX_SIZE = 10 * 1024;

    /**
     * HTTP traces enabled
     */
    public static boolean HTTP_TRACE_ENABLED = true;

    /**
     * HTTP server address
     */
    private static String serverAddr = RcsSettings.getInstance().getFtHttpServer();

    /**
     * HTTP server login
     */
    private String serverLogin = RcsSettings.getInstance().getFtHttpLogin();

    /**
     * HTTP server password
     */
    private String serverPwd = RcsSettings.getInstance().getFtHttpPassword();

    /**
     * HTTP transfer event listener
     */
    private HttpTransferEventListener listener;

    /**
     * HTTP context
     */
    private HttpContext httpContext = null;
    
    /**
     * HTTP response
     */
    private HttpResponse response = null;

    /**
     * HTTP client
     */
    private DefaultHttpClient httpClient = null;
    
    /**
     * Cancellation flag
     */
    private boolean isCancelled = false;    
    
    /**
     * Pause flag
     */
    private boolean isPaused = false;
    

    /**
     * The logger
     */
    private Logger logger = Logger.getLogger(this.getClass().getName());

    /**
     * Constructor
     *
     * @param listener HTTP event listener
     */
    public HttpTransferManager(HttpTransferEventListener listener) {
        this(listener, serverAddr);
        java.util.logging.Logger.getLogger("org.apache.http.wire").setLevel(java.util.logging.Level.ALL);

    }

    /**
     * Constructor
     *
     * @param listener HTTP event listener
     * @param address HTTP server address
     */
    public HttpTransferManager(HttpTransferEventListener listener, String address) {
        this.listener = listener;

        initServerAddress(address);
        java.util.logging.Logger.getLogger("org.apache.http.wire").setLevel(java.util.logging.Level.ALL);

    }

    public String getLocalIpAddress() {
    try {
        for (Enumeration<NetworkInterface> en = NetworkInterface.getNetworkInterfaces(); en.hasMoreElements();) {
            NetworkInterface intf = en.nextElement();
            for (Enumeration<InetAddress> enumIpAddr = intf.getInetAddresses(); enumIpAddr.hasMoreElements();) {
                InetAddress inetAddress = enumIpAddr.nextElement();
                if (!inetAddress.isLoopbackAddress()) {
                    String ip = Formatter.formatIpAddress(inetAddress.hashCode());
                 //   Log.i(TAG, "***** IP="+ ip);
                    return ip;
                }
            }
        }
    } catch (SocketException ex) {
   //     Log.e(TAG, ex.toString());
    }
    return null;
    }

    
    /**
     * Initialize with server address
     *
     * @param address server address
     */
    public void initServerAddress(String address) {
        try {
            // Extract protocol and port
            URL url = new URL(address);
            String protocol = url.getProtocol();
            int port = url.getPort();
            if (port == -1) {
                // Set default port
                if (protocol.equals("https")) {
                    port = 443;
                } else {
                    port = 80;
                }
            }
    
            // Format HTTP request
            ConnectivityManager connMgr = (ConnectivityManager) AndroidFactory.getApplicationContext().getSystemService(Context.CONNECTIVITY_SERVICE);
            SchemeRegistry schemeRegistry = new SchemeRegistry();
            if (protocol.equals("https")) {
                schemeRegistry.register(new Scheme("https", new com.orangelabs.rcs.provisioning.https.EasySSLSocketFactory(), port));
            } else {
                schemeRegistry.register(new Scheme("http", PlainSocketFactory.getSocketFactory(), port));
            }
            HttpParams params = new BasicHttpParams();
            params.setParameter(ConnManagerPNames.MAX_TOTAL_CONNECTIONS, 30);
            params.setParameter(ConnManagerPNames.MAX_CONNECTIONS_PER_ROUTE, new ConnPerRouteBean(30));
            params.setParameter(HttpProtocolParams.USE_EXPECT_CONTINUE, false);
            NetworkInfo networkInfo = connMgr.getActiveNetworkInfo();
            if (networkInfo != null && networkInfo.getType() == ConnectivityManager.TYPE_MOBILE) {
                String proxyHost = Proxy.getDefaultHost();
                if (proxyHost != null && proxyHost.length() > 1) {
                    int proxyPort = Proxy.getDefaultPort();
                    params.setParameter(ConnRoutePNames.DEFAULT_PROXY, new HttpHost(proxyHost, proxyPort));
                }
            }
            HttpProtocolParams.setVersion(params, HttpVersion.HTTP_1_1);
            ClientConnectionManager cm = new SingleClientConnManager(params, schemeRegistry);
            httpClient = new DefaultHttpClient(cm, params);
            
            
            // Create local HTTP context
            CookieStore cookieStore = (CookieStore) new BasicCookieStore();
            httpContext = new BasicHttpContext();
            httpContext.setAttribute(ClientContext.COOKIE_STORE, cookieStore);
        } catch (MalformedURLException e) {
            // Nothing to do
        }
    }

    /**
     * Returns the transfer event listener
     *
     * @return Listener
     */
    public HttpTransferEventListener getListener() {
        return listener;
    }

    /**
     * Returns HTTP server address
     *
     * @return Address
     */
    public String getHttpServerAddr() {
        return serverAddr;
    }

    /**
     * Returns HTTP server login
     *
     * @return Login
     */
    public String getHttpServerLogin() {
        return serverLogin;
    }

    /**
     * Returns HTTP server password
     *
     * @return Password
     */
    public String getHttpServerPwd() {
        return serverPwd;
    }

    /**
     * Execute HTTP request
     *
     * @param request HTTP request
     * @return HTTP response
     * @throws IOException 
     * @throws ClientProtocolException 
     */
    public HttpResponse executeRequest(HttpRequestBase request) throws ClientProtocolException, IOException {
        if(response != null)
        {
            response.getEntity().consumeContent();
        }
        
        String host = null;
        InetAddress inetAddress = null;
        String localIP= null;
        int port=  0;
        
        if (logger.isActivated()) {
       host = request.getURI().getHost();
         inetAddress = InetAddress.getByName(host);
         localIP= getLocalIpAddress();
         port=   request.getURI().getPort();
        }
              
        String traceFirstRequest = "\r\n" ;
        if (HTTP_TRACE_ENABLED) {
            String trace = ">>> Send HTTP request:";
            if (logger.isActivated()) {
                traceFirstRequest += request.getMethod() + " " + request.getRequestLine().getUri();
            }
            HeaderIterator hi = request.headerIterator();
            Header[] headers = request.getAllHeaders();
            while(hi.hasNext()){
                System.out.println("there are headers");
                Header newHeader = hi.nextHeader();
                if (logger.isActivated()) {
                    traceFirstRequest += "\r\n" + newHeader.getName() + ": " + newHeader.getValue();      
                }         
            }
            if (logger.isActivated()) {
                for(int i =0 ;i < headers.length -1 ;i++){
                    System.out.println("there are headers1");
                    traceFirstRequest += "\r\n" + headers[i].getName() + ": " + headers[i].getValue();     
                }
            }
            if (logger.isActivated()) {
                //LogPcap.getInstance().logPcapUDP(localIP, inetAddress.toString(),0 ,port , traceFirstRequest.getBytes());
                System.out.println(traceFirstRequest);
            }
            System.out.println(trace);
        }
        
        if (httpClient != null) {
            response = httpClient.execute(request, httpContext);
            if (HTTP_TRACE_ENABLED) {
                String trace = "<<< Receive HTTP response:";
                String traceResponse = "\r\n";
                trace += "\n" + response.getStatusLine().toString();
                if (logger.isActivated()) {
                    traceResponse +=  response.getStatusLine().toString();
                }
                Header[] headers = response.getAllHeaders();
                for (Header header : headers) {
                    trace += "\n" + header.getName() + ": " + header.getValue();
                    if (logger.isActivated()) {
                        traceResponse += "\r\n" + header.getName() + ": " + header.getValue();
                    }
                }
                 if (logger.isActivated()) {
                     //LogPcap.getInstance().logPcapUDP(inetAddress.toString(), localIP,port , 0, traceResponse.getBytes());
                }
                System.out.println(trace);
                System.out.println(traceResponse);
            }
            return response;
        } else {
            throw new IOException("HTTP client not found");
        }
    }
    
    /**
     * Get HTTP client
     * 
     * @return HTTP client
     */
    public DefaultHttpClient getHttpClient(){
        return httpClient;
    }

    /**
     * Interrupts file transfer
     */
    public void interrupt() {
        if (logger.isActivated()) {
            logger.error("interrupting transfer");
        }
        isCancelled = true;
    }
    
    /**
     * Interrupts file transfer
     */
    public void pauseTransfer() {
        if (logger.isActivated()) {
            logger.error("Pausing transfer");
        }
        isPaused = true;
    }
    
    /**
     * Resuming upload so resetting cancelled boolean
     */
    public void resetParamForResume() {
        if (logger.isActivated()) {
            logger.error("resuming file transfer");
        }
        isCancelled = false;
        isPaused = false;
    }
    
    /**
     * Return whether or not the file transfer has been cancelled
     * 
     * @return Boolean
     */
    public boolean isCancelled() {
        return this.isCancelled;
    }
    
    /**
     * Return whether or not the file transfer has been cancelled
     * 
     * @return Boolean
     */
    public boolean isPaused() {
        return this.isPaused;
    }
}
