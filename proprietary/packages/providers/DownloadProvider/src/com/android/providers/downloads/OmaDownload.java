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

//package com.mediatek.omadownload;
package com.android.providers.downloads;

import android.os.Handler;
import android.os.Message;
import static android.text.format.DateUtils.SECOND_IN_MILLIS;
import android.util.Log;
import android.util.Xml;
import android.webkit.URLUtil;


/*import org.apache.http.HttpEntity;
import org.apache.http.HttpResponse;
import org.apache.http.HttpStatus;
import org.apache.http.NoHttpResponseException;
import org.apache.http.client.methods.HttpPost;
import org.apache.http.conn.ConnectTimeoutException;
import org.apache.http.entity.StringEntity;
import org.apache.http.impl.client.DefaultHttpClient;
import org.apache.http.impl.client.DefaultHttpRequestRetryHandler;
import org.apache.http.params.HttpParams;
import org.apache.http.params.HttpProtocolParams;
import org.apache.http.protocol.HttpContext;*/

import org.xml.sax.Attributes;
import org.xml.sax.SAXException;
import org.xml.sax.helpers.DefaultHandler;

import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileReader;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.OutputStream;
import java.io.OutputStreamWriter;
import java.io.UnsupportedEncodingException;

import java.net.HttpURLConnection;

import java.net.MalformedURLException;
import java.net.URL;


public class OmaDownload {

    private static final String TAG = "OmaDownload";
    private static final double SUPPORTED_DDVERSION = 1.0;

    private static final OmaDownload OMADL_INSTANCE = new OmaDownload();
    private static final int DEFAULT_TIMEOUT = (int) (20 * SECOND_IN_MILLIS);
    /**
    * This method notifies the server for the status of the download operation.
    * It sends a status report to a
    Web server if installNotify attribute is specified in the download descriptor.
    @param  component   the component that contains attributes in the descriptor.
    @param  handler     the handler used to send and process messages. A message
                        indicates whether the media object is available to the user
                        or not (READY or DISCARD).
    */
    //protected static void installNotify (OmaDescription component, Handler handler) {
    protected static int installNotify(OmaDescription component, Handler handler) {

        //int ack = -1;
        int release = OmaStatusHandler.DISCARD;
        URL url = component.getInstallNotifyUrl();
        Log.v("@M_" + Constants.LOG_OMA_DL, "URL = " + url);

        if (url != null) {
            HttpURLConnection urlConnection = null;

            BufferedReader reader;
            InputStream is;
            int retryCount = 0;


            while (retryCount <= OmaStatusHandler.MAXIMUM_RETRY) {
                try {
                    urlConnection = (HttpURLConnection) url.openConnection();
                    urlConnection.setDoOutput(true);
                    urlConnection.setChunkedStreamingMode(0);
                    urlConnection.setInstanceFollowRedirects(false);
                    urlConnection.setConnectTimeout(DEFAULT_TIMEOUT);
                    urlConnection.setReadTimeout(DEFAULT_TIMEOUT);
                    urlConnection.connect();

                    OutputStream os = urlConnection.getOutputStream();
                    BufferedWriter writer = new BufferedWriter(
                                    new OutputStreamWriter(os, "UTF-8"));
                    String toWrite = OmaStatusHandler.statusCodeToString(component.getStatusCode());
                    Log.v("@M_" + Constants.LOG_OMA_DL, "toWrite in request = " + toWrite);
                    writer.write((toWrite) + "\n\r");
                    writer.flush();
                    writer.close();
                    os.close();
                    break;
                } catch (Exception e) {
                    Log.v("@M_" + Constants.LOG_OMA_DL, "Exception in request: ");
                    retryCount++;
                    e.printStackTrace();
                }
            }

            try {
                int code = urlConnection.getResponseCode();
                Log.v("@M_" + Constants.LOG_OMA_DL, "Response code = " + code);
                if (code == HttpURLConnection.HTTP_OK || code == HttpURLConnection.HTTP_ACCEPTED
                        || code == HttpURLConnection.HTTP_CREATED
                        || code == 207 /*HttpStatus.SC_MULTI_STATUS*/
                        || code == HttpURLConnection.HTTP_NOT_AUTHORITATIVE
                         || code == HttpURLConnection.HTTP_NO_CONTENT
                        || code == HttpURLConnection.HTTP_PARTIAL ||
                         code == HttpURLConnection.HTTP_RESET) {
                    if (component.getStatusCode() == OmaStatusHandler.SUCCESS) {
                        release = OmaStatusHandler.READY;
                    }
                }

                is = urlConnection.getInputStream();
                reader = new BufferedReader(new InputStreamReader(is));
                String line;
                while ((line = reader.readLine()) != null) {
                    Log.v("@M_" + Constants.LOG_OMA_DL, "Response: " + line);
                }
                reader.close();
                is.close();
            } catch (IOException e) {
                Log.v("@M_" + Constants.LOG_OMA_DL, "IOException in response: ");
                release = OmaStatusHandler.DISCARD;
                e.printStackTrace();
            } catch (Exception e) {
                Log.v("@M_" + Constants.LOG_OMA_DL, "Other Exception in response: ");
                if (component.getStatusCode() == OmaStatusHandler.SUCCESS) {
                    release = OmaStatusHandler.READY;
                }
                e.printStackTrace();
            }
        } else {
            if (component.getStatusCode() == OmaStatusHandler.SUCCESS) {
                release = OmaStatusHandler.READY;
            }
        }



        /*if (url != null) {
            DefaultHttpClient client = new DefaultHttpClient();
            HttpPost postRequest = new HttpPost(url.toString());

            try {
                HttpParams params = postRequest.getParams();
                HttpProtocolParams.setUseExpectContinue(params, false);
                postRequest.setEntity(new StringEntity(
                        OmaStatusHandler.statusCodeToString(component.getStatusCode()) + "\n\r"));
            } catch (UnsupportedEncodingException e1) {
                // TODO Auto-generated catch block
                e1.printStackTrace();
            }

            client.setHttpRequestRetryHandler(new DefaultHttpRequestRetryHandler() {
                @Override
                public boolean retryRequest
                (IOException exception, int executionCount, HttpContext context) {
                    // TODO Auto-generated method stub
                    Log.i("@M_" + Constants.LOG_OMA_DL, "Retry the request...");
                    return (executionCount <= OmaStatusHandler.MAXIMUM_RETRY);
                }
            });

            try {
                HttpResponse response = client.execute(postRequest);

                if (response.getStatusLine() != null) {
                    ack = response.getStatusLine().getStatusCode();

                    //200-series response code
                    if (ack == HttpStatus.SC_OK || ack == HttpStatus.SC_ACCEPTED
                            || ack == HttpStatus.SC_CREATED || ack == HttpStatus.SC_MULTI_STATUS
                            || ack == HttpStatus.SC_NON_AUTHORITATIVE_INFORMATION
                             || ack == HttpStatus.SC_NO_CONTENT
                            || ack == HttpStatus.SC_PARTIAL_CONTENT ||
                             ack == HttpStatus.SC_RESET_CONTENT) {
                        if (component.getStatusCode() == OmaStatusHandler.SUCCESS) {
                            release = OmaStatusHandler.READY;
                        }
                    }

                    final HttpEntity entity = response.getEntity();
                    if (entity != null) {
                        InputStream inputStream = null;
                        try {
                            inputStream = entity.getContent();
                            if (inputStream != null) {
                                BufferedReader br = new BufferedReader
                                (new InputStreamReader(inputStream));

                                String s;
                                while ((s = br.readLine()) != null) {
                                    Log.v("@M_" + Constants.LOG_OMA_DL, "Response: " + s);
                                }
                            }

                        } finally {
                            if (inputStream != null) {
                                inputStream.close();
                            }
                            entity.consumeContent();
                        }
                    }
                }
            } catch (ConnectTimeoutException e) {
                Log.e("@M_" + Constants.LOG_OMA_DL, e.toString());
                postRequest.abort();
                //After time out period, the client releases the media object for use.
                if (component.getStatusCode() == OmaStatusHandler.SUCCESS) {
                    release = OmaStatusHandler.READY;
                }
            } catch (NoHttpResponseException e) {
                Log.e("@M_" + Constants.LOG_OMA_DL, e.toString());
                postRequest.abort();
                //After time out period, the client releases the media object for use.
                if (component.getStatusCode() == OmaStatusHandler.SUCCESS) {
                    release = OmaStatusHandler.READY;
                }
            } catch (IOException e) {
                Log.e("@M_" + Constants.LOG_OMA_DL, e.toString());
                postRequest.abort();
            }
            if (client != null) {
               client.getConnectionManager().shutdown();
            }
        } else {
            if (component.getStatusCode() == OmaStatusHandler.SUCCESS) {
                release = OmaStatusHandler.READY;
            }
        }*/

        if (handler != null) {
            Message mg = Message.obtain();
            mg.arg1 = release;
            handler.sendMessage(mg);
        }
        Log.v("@M_" + Constants.LOG_OMA_DL, "installNotify return release = " + release);
        return release;
    }

    /**
    * This method parses an xml file into a provided component.
    @param  ddUrl       the URL of the download descriptor file
    @param  file        the file containing the XML to be parsed
    @param  component   the component to which the parsed xml data should be added
    @return             the status code (success or other error code)
    */
    protected static int parseXml(URL ddUrl, File file, OmaDescription component) {

        BufferedReader sReader = null;

        //Initialize the status code in the component
        component.setStatusCode(OmaStatusHandler.SUCCESS);

        if (file == null || ddUrl == null) {
            component.setStatusCode(OmaStatusHandler.INVALID_DESCRIPTOR);
        } else {
            try {
                sReader = new BufferedReader(new FileReader(file));
            } catch (FileNotFoundException e) {
                // TODO Auto-generated catch block
                Log.e("@M_" + Constants.LOG_OMA_DL, e.toString());
            }

            try {
                Xml.parse(sReader, OMADL_INSTANCE.new DDHandler(ddUrl, component));
            } catch (IOException e) {
                // TODO Auto-generated catch block
                Log.e("@M_" + Constants.LOG_OMA_DL, e.toString());
            } catch (SAXException e) {
                // TODO Auto-generated catch block
                Log.e("@M_" + Constants.LOG_OMA_DL, e.toString());
                component.setStatusCode(OmaStatusHandler.INVALID_DESCRIPTOR);

                //parse install notify url
                String strLine;
                try {
                    sReader = new BufferedReader(new FileReader(file));

                    while ((strLine = sReader.readLine()) != null)   {
                        strLine = strLine.trim();

                        StringBuffer strBuffer = new StringBuffer(strLine);
                        String startTag = "<installNotifyURI>";
                        String endTag = "</installNotifyURI>";
                        int startTagPos = strBuffer.lastIndexOf(startTag);
                        int endTagPos = strBuffer.lastIndexOf(endTag);

                        if (startTagPos != -1 && endTagPos != -1) {
                            strLine = strLine.substring(startTagPos + startTag.length(), endTagPos);
                            Log.d("@M_" + Constants.LOG_OMA_DL, "install notify URI: " + strLine);
                            URL url = new URL(strLine);
                            component.setInstallNotifyUrl(url);
                            break;
                        }
                    }
                } catch (IOException e1) {
                    // TODO Auto-generated catch block
                    Log.e("@M_" + Constants.LOG_OMA_DL, e1.toString());
                }
            }
        }

        return component.getStatusCode();
    }

    class DDHandler extends DefaultHandler {

        private boolean mRootVisited = false;
        private boolean mTypeVisited = false;
        private boolean mSizeVisited = false;
        private boolean mObjectUrlVisited = false;
        private StringBuilder mBuilder = null;
        private OmaDescription mComponent = null;
        private URL mDDUrl = null;
        private static final  String TAG = "DDHandler";

        DDHandler(URL ddUrl, OmaDescription component) {
            super();
            mDDUrl = ddUrl;
            mComponent = component;
        }

        @Override
        public void characters(char[] ch, int start, int length)
                throws SAXException {
            super.characters(ch, start, length);
            mBuilder.append(ch, start, length);
        }

        @Override
        public void endElement(String uri, String localName, String name)
                throws SAXException {
            super.endElement(uri, localName, name);

            URL url = null;
            String s = null;

            if (mComponent != null) {
                s = mBuilder.toString().trim();

                if (localName.equalsIgnoreCase(OmaDescription.DDVERSION)) {
                    if (mComponent.getDDVersion() == null) {
                        mComponent.setDDVersion(s);
                        try {
                            double val = Double.parseDouble(s);

                            if (val != SUPPORTED_DDVERSION) {
                                //Invalid DDVersion
                                mComponent.setStatusCode(OmaStatusHandler.INVALID_DDVERSION);
                            }
                        } catch (NumberFormatException e) {
                            Log.e("@M_" + Constants.LOG_OMA_DL, e.toString());
                            mComponent.setStatusCode(OmaStatusHandler.INVALID_DDVERSION);
                        }
                    }
                } else if (localName.equalsIgnoreCase(OmaDescription.DESCRIPTION)) {
                    if (mComponent.getDescription() == null) {
                        mComponent.setDescription(s);
                    }
                } else if (localName.equalsIgnoreCase(OmaDescription.ICON_URL)) {
                    if (mComponent.getIconUrl() == null) {
                        try {
                            url = new URL(s);
                            mComponent.setIconUrl(url);
                        } catch (MalformedURLException e) {
                            Log.e("@M_" + Constants.LOG_OMA_DL, e.toString());
                            mComponent.setStatusCode(OmaStatusHandler.INVALID_DESCRIPTOR);
                        }
                    }
                } else if (localName.equalsIgnoreCase(OmaDescription.INFO_URL)) {
                    if (mComponent.getInfoUrl() == null) {
                        try {
                            url = new URL(s);
                            mComponent.setInfoUrl(url);
                        } catch (MalformedURLException e) {
                            Log.e("@M_" + Constants.LOG_OMA_DL, e.toString());
                            mComponent.setStatusCode(OmaStatusHandler.INVALID_DESCRIPTOR);
                        }
                    }
                } else if (localName.equalsIgnoreCase(OmaDescription.INSTALL_NOTIFY_URL)) {
                    if (mComponent.getInstallNotifyUrl() == null) {
                        try {
                            s = checkUrl(s);
                            url = new URL(s);
                            mComponent.setInstallNotifyUrl(url);
                        } catch (MalformedURLException e) {
                            Log.e("@M_" + Constants.LOG_OMA_DL, e.toString());
                            mComponent.setStatusCode(OmaStatusHandler.INVALID_DESCRIPTOR);
                        }
                    }
                } else if (localName.equalsIgnoreCase(OmaDescription.INSTALL_PARAM)) {
                    if (mComponent.getInstallParam() == null) {
                        mComponent.setInstallParam(s);
                    }
                } else if (localName.equalsIgnoreCase(OmaDescription.OBJECT_URL)) {
                    if (mComponent.getObjectUrl() == null) {
                        try {
                            s = checkUrl(s);
                            url = new URL(s);
                            mComponent.setObjectUrl(url);
                        } catch (MalformedURLException e) {
                            Log.e("@M_" + Constants.LOG_OMA_DL, e.toString());
                            mComponent.setStatusCode(OmaStatusHandler.INVALID_DESCRIPTOR);
                        }
                    }
                } else if (localName.equalsIgnoreCase(OmaDescription.NAME)) {
                    if (mComponent.getName() == null) {
                        mComponent.setName(s);
                    }
                } else if (localName.equalsIgnoreCase(OmaDescription.NEXT_URL)) {
                    if (mComponent.getNextUrl() == null) {
                        try {
                            url = new URL(s);
                            mComponent.setNextUrl(url);
                        } catch (MalformedURLException e) {
                            Log.e("@M_" + Constants.LOG_OMA_DL, e.toString());
                            mComponent.setStatusCode(OmaStatusHandler.INVALID_DESCRIPTOR);
                        }
                    }
                } else if (localName.equalsIgnoreCase(OmaDescription.SIZE)) {
                    if (mComponent.getSize() == -1) {
                        try {
                            int val = Integer.parseInt(s);

                            if (val > 0) {
                                mComponent.setSize(val);
                            } else {
                                //Invalid size of a media object
                                mComponent.setStatusCode(OmaStatusHandler.INVALID_DESCRIPTOR);
                            }
                        } catch (NumberFormatException e) {
                            Log.e("@M_" + Constants.LOG_OMA_DL, e.toString());
                            mComponent.setStatusCode(OmaStatusHandler.INVALID_DESCRIPTOR);
                        }
                    }
                } else if (localName.equalsIgnoreCase(OmaDescription.TYPE)) {
                        mComponent.setType(s);
                } else if (localName.equalsIgnoreCase(OmaDescription.VENDOR)) {
                    if (mComponent.getVendor() == null) {
                        mComponent.setVendor(s);
                    }
                } else if (localName.equalsIgnoreCase(OmaDescription.ROOT)) {
                    if (!mObjectUrlVisited || !mSizeVisited || !mTypeVisited) {
                        //Missing mandatory attributes
                        mComponent.setStatusCode(OmaStatusHandler.INVALID_DESCRIPTOR);
                    }
                }
                mBuilder.setLength(0);
            }
        }

        @Override
        public void startDocument() throws SAXException {
            super.startDocument();
            mBuilder = new StringBuilder();
        }

        @Override
        public void startElement(String uri, String localName, String name,
                Attributes attributes) throws SAXException {
            super.startElement(uri, localName, name, attributes);

            if (localName.equalsIgnoreCase(OmaDescription.ROOT)) {
                if (!mRootVisited) {
                    mRootVisited = true;
                } else {
                     //Double root elements
                    mComponent.setStatusCode(OmaStatusHandler.INVALID_DESCRIPTOR);
                }
            }
            if (localName.equalsIgnoreCase(OmaDescription.OBJECT_URL)) {
                mObjectUrlVisited = true;
            }
            if (localName.equalsIgnoreCase(OmaDescription.SIZE)) {
                mSizeVisited = true;
            }
            if (localName.equalsIgnoreCase(OmaDescription.TYPE)) {
                mTypeVisited = true;
            }
        }

        private String checkUrl(String url) {
            if (!URLUtil.isValidUrl(url)) {
                //http url, https url, file url, content url, etc.
                int index = mDDUrl.toString().lastIndexOf('/');
                if (index != -1) {
                    String sub = mDDUrl.toString().substring(0, index);
                    if (url.matches("\\s*/+.*")) {
                        //match a local file with a directory path, e.g. /OMA10/x.mp3
                        url = sub + url;
                    } else {                        //match a file
                        url = sub + "/" + url;
                    }
                }
            }
            return url;
        }
    }
}
