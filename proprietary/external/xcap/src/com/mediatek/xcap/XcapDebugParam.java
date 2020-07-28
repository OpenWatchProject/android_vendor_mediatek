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

/*
 *
 */

package com.mediatek.xcap.client;

import android.os.SystemProperties;
import android.util.Log;

import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.w3c.dom.Node;
import org.w3c.dom.NodeList;
import org.xml.sax.InputSource;
import org.xml.sax.SAXException;

import java.io.BufferedInputStream;
import java.io.DataInputStream;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.StringReader;

import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.parsers.ParserConfigurationException;

/**
 * XCAP debug configuration class.
 */
public class XcapDebugParam {
    private static final String TAG = "XcapDebugParam";
    private static final String TAG_ROOT = "DebugParam";
    private static final String TAG_XCAP_ROOT = "XcapRoot";
    private static final String TAG_XCAP_USER_AGENT = "XcapUserAgent";
    private static final String TAG_XCAP_XUI = "XcapXui";
    private static final String TAG_XCAP_HTTP_DIGEST_USERNAME = "HttpDigestUsername";
    private static final String TAG_XCAP_HTTP_DIGEST_PASSWORD = "HttpDigestPassword";
    private static final String TAG_XCAP_ENABLE_PREDEFINED_SIMSERV_QUERY_RESULT =
            "EnablePredefinedSimservQueryResult";
    private static final String TAG_XCAP_ENABLE_PREDEFINED_SIMSERV_SETTING =
            "EnablePredefinedSimservSetting";
    private static final String TAG_XCAP_ENABLE_SIMSERV_QUERY_WHOLE = "EnableSimservQueryWhole";
    private static final String TAG_XCAP_ENABLE_HTTP_LOG = "EnableHttpLog";
    private static final String TAG_XCAP_ENABLE_TRUST_ALL = "EnableXcapTrustAll";
    private static final String TAG_XCAP_DOCUMENT_NAME = "XcapDocumentName";
    private static final String TAG_XCAP_PUT_ELEMENT_MIME = "XcapPutElementMime";
    private static final String TAG_XCAP_AUID = "XcapAUID";
    private static final String TAG_XCAP_DISABLE_ETAG = "DisableETag";

    private static XcapDebugParam sInstance;

    //XCAP
    private static String mXcapRoot;
    private static String mXcapUserAgent;
    private static String mXcapXui;
    private static String mHttpDigestUsername;
    private static String mHttpDigestPassword;
    //if enable, push xml file to /data/ss.xml
    private static boolean mEnablePredefinedSimservQueryResult = false;
    //if enable, push xml file /data/simservs.xml
    private static boolean mEnablePredefinedSimservSetting = false;
    private static boolean mEnableSimservQueryWhole = false;
    private static boolean mEnableHttpLog = false;
    private static boolean mEnableXcapTrustAll = false;
    private static String mXcapDocumentName;
    private static String mXcapPutElementMime;
    private static String mXcapAUID;
    private static boolean mDisableETag = false;

    /**
     * Singleton.
     *
     * @return XcapDebugParam instance
     */
    public static XcapDebugParam getInstance() {
        if (sInstance == null) {
            sInstance = new XcapDebugParam();
        }
        return sInstance;
    }


    /**
     * Load debug configuration.
     */
    public void load() {
        Log.d(TAG, "XcapDebugParam is loading");

        readProperty();

        String xmlContent = readXmlFromFile("/data/misc/xcapconfig.xml");
        if (xmlContent == null || xmlContent.equals("")) {
            return;
        }

        try {
            DocumentBuilderFactory factory = DocumentBuilderFactory.newInstance();
            DocumentBuilder db = factory.newDocumentBuilder();
            InputSource is = new InputSource();
            is.setCharacterStream(new StringReader(xmlContent));
            Document doc;
            doc = db.parse(is);

            NodeList debugParamNode = doc.getElementsByTagName(TAG_ROOT);
            if (debugParamNode.getLength() > 0) {
                instantiateFromXmlNode(debugParamNode.item(0));
            }
        } catch (SAXException e) {
            e.printStackTrace();
        } catch (IOException e) {
            e.printStackTrace();
        } catch (ParserConfigurationException e) {
            e.printStackTrace();
        }
    }

    /**
     * Read XML file.
     *
     * @param file XML file path
     * @return String content of XML
     */
    public String readXmlFromFile(String file) {
        String text = "";

        try {
            FileInputStream fis = new FileInputStream(file);
            DataInputStream dis = new DataInputStream(fis);

            String buf;
            while ((buf = dis.readLine()) != null) {
                text += buf;
            }
            fis.close();
        } catch (IOException e) {
            return null;
        }

        return text;
    }

    public void readProperty() {
        String val = null;
        val = SystemProperties.get("persist.vendor.xcap." + TAG_XCAP_ROOT.toLowerCase());
        mXcapRoot = (val != null && !val.isEmpty()) ? val : null;
        val = SystemProperties.get("persist.vendor.xcap." + TAG_XCAP_USER_AGENT.toLowerCase());
        mXcapUserAgent = (val != null && !val.isEmpty()) ? val : null;
        val = SystemProperties.get("persist.vendor.xcap." + TAG_XCAP_XUI.toLowerCase());
        mXcapXui = (val != null && !val.isEmpty()) ? val : null;
        mEnableSimservQueryWhole = SystemProperties.getBoolean(
            "persist.vendor.xcap." + "simservquerywhole", false);
        mEnableXcapTrustAll = SystemProperties.getBoolean(
            "persist.vendor.xcap." + TAG_XCAP_ENABLE_TRUST_ALL.toLowerCase(), false);
        val = SystemProperties.get("persist.vendor.xcap." + TAG_XCAP_DOCUMENT_NAME.toLowerCase());
        mXcapDocumentName = (val != null && !val.isEmpty()) ? val : null;
        val = SystemProperties.get("persist.vendor.xcap." + TAG_XCAP_PUT_ELEMENT_MIME.toLowerCase());
        mXcapPutElementMime = (val != null && !val.isEmpty()) ? val : null;
        val = SystemProperties.get("persist.vendor.xcap." + TAG_XCAP_AUID.toLowerCase());
        mXcapAUID = (val != null && !val.isEmpty()) ? val : null;
        mDisableETag = SystemProperties.getBoolean(
            "persist.vendor.xcap." + TAG_XCAP_DISABLE_ETAG.toLowerCase(), false);

        Log.d(TAG, "readProperty mXcapRoot: " + mXcapRoot + "\n" +
                "mXcapUserAgent: " + mXcapUserAgent + "\n" +
                "mXcapXui: " + mXcapXui + "\n" +
                "mEnableSimservQueryWhole: " + mEnableSimservQueryWhole + "\n" +
                "mEnableXcapTrustAll: " + mEnableXcapTrustAll + "\n" +
                "mXcapDocumentName: " + mXcapDocumentName + "\n" +
                "mXcapPutElementMime: " + mXcapPutElementMime + "\n" +
                "mXcapAUID: " + mXcapAUID + "\n" +
                "mDisableETag: " + mDisableETag + "\n");
    }

    private void reset() {
        mXcapRoot = null;
        mXcapUserAgent = null;
        mXcapXui = null;
        mHttpDigestUsername = null;
        mHttpDigestPassword = null;
        mEnablePredefinedSimservQueryResult = false;
        mEnablePredefinedSimservSetting = false;
        mEnableSimservQueryWhole = false;
        mEnableHttpLog = false;
        mEnableXcapTrustAll = false;
        mXcapDocumentName = "simservs.xml";
        mXcapPutElementMime = null;
        mXcapAUID = null;
        mDisableETag = false;
    }

    private void instantiateFromXmlNode(Node domNode) {
        Log.d(TAG, "instantiateFromXmlNode");

        Element domElement = (Element) domNode;

        NodeList node = domElement.getElementsByTagName(TAG_XCAP_ROOT);
        if (node.getLength() > 0) {
            mXcapRoot = ((Element) node.item(0)).getTextContent();
        }

        node = domElement.getElementsByTagName(TAG_XCAP_USER_AGENT);
        if (node.getLength() > 0) {
            mXcapUserAgent = ((Element) node.item(0)).getTextContent();
        }

        node = domElement.getElementsByTagName(TAG_XCAP_XUI);
        if (node.getLength() > 0) {
            mXcapXui = ((Element) node.item(0)).getTextContent();
        }

        node = domElement.getElementsByTagName(TAG_XCAP_HTTP_DIGEST_USERNAME);
        if (node.getLength() > 0) {
            mHttpDigestUsername = ((Element) node.item(0)).getTextContent();
        }

        node = domElement.getElementsByTagName(TAG_XCAP_HTTP_DIGEST_PASSWORD);
        if (node.getLength() > 0) {
            mHttpDigestPassword = ((Element) node.item(0)).getTextContent();
        }

        node = domElement.getElementsByTagName(TAG_XCAP_ENABLE_PREDEFINED_SIMSERV_QUERY_RESULT);
        if (node.getLength() > 0) {
            String str = ((Element) node.item(0)).getTextContent();
            if ("true".equalsIgnoreCase(str)) {
                mEnablePredefinedSimservQueryResult = true;
            } else {
                mEnablePredefinedSimservQueryResult = false;
            }
        }

        node = domElement.getElementsByTagName(TAG_XCAP_ENABLE_PREDEFINED_SIMSERV_SETTING);
        if (node.getLength() > 0) {
            String str = ((Element) node.item(0)).getTextContent();
            if ("true".equalsIgnoreCase(str)) {
                mEnablePredefinedSimservSetting = true;
            } else {
                mEnablePredefinedSimservSetting = false;
            }
        }

        node = domElement.getElementsByTagName(TAG_XCAP_ENABLE_SIMSERV_QUERY_WHOLE);
        if (node.getLength() > 0) {
            String str = ((Element) node.item(0)).getTextContent();
            if ("true".equalsIgnoreCase(str)) {
                mEnableSimservQueryWhole = true;
            } else {
                mEnableSimservQueryWhole = false;
            }
        }

        node = domElement.getElementsByTagName(TAG_XCAP_ENABLE_HTTP_LOG);
        if (node.getLength() > 0) {
            String str = ((Element) node.item(0)).getTextContent();
            if ("true".equalsIgnoreCase(str)) {
                mEnableHttpLog = true;
            } else {
                mEnableHttpLog = false;
            }
        }

        node = domElement.getElementsByTagName(TAG_XCAP_ENABLE_TRUST_ALL);
        if (node.getLength() > 0) {
            String str = ((Element) node.item(0)).getTextContent();
            if ("true".equalsIgnoreCase(str)) {
                mEnableXcapTrustAll = true;
            } else {
                mEnableXcapTrustAll = false;
            }
        }

        node = domElement.getElementsByTagName(TAG_XCAP_DOCUMENT_NAME);
        if (node.getLength() > 0) {
            mXcapDocumentName = ((Element) node.item(0)).getTextContent();
        }

        node = domElement.getElementsByTagName(TAG_XCAP_PUT_ELEMENT_MIME);
        if (node.getLength() > 0) {
            mXcapPutElementMime = ((Element) node.item(0)).getTextContent();
        }

        node = domElement.getElementsByTagName(TAG_XCAP_AUID);
        if (node.getLength() > 0) {
            mXcapAUID = ((Element) node.item(0)).getTextContent();
        }

        node = domElement.getElementsByTagName(TAG_XCAP_DISABLE_ETAG);
        if (node.getLength() > 0) {
            String str = ((Element) node.item(0)).getTextContent();
            if ("true".equalsIgnoreCase(str)) {
                mDisableETag = true;
            } else {
                mDisableETag = false;
            }
        }

    }

    public String getXcapRoot() {
        return mXcapRoot;
    }

    public String getXcapUserAgent() {
        return mXcapUserAgent;
    }

    public String getXcapXui() {
        return mXcapXui;
    }

    public String getHttpDigestUsername() {
        return mHttpDigestUsername;
    }

    public String getHttpDigestPassword() {
        return mHttpDigestPassword;
    }

    public boolean getEnablePredefinedSimservQueryResult() {
        return mEnablePredefinedSimservQueryResult;
    }

    public boolean getEnablePredefinedSimservSetting() {
        return mEnablePredefinedSimservSetting;
    }

    public boolean getEnableSimservQueryWhole() {
        return mEnableSimservQueryWhole;
    }

    public boolean getEnableHttpLog() {
        return mEnableHttpLog;
    }

    public boolean getEnableXcapTrustAll() {
        return mEnableXcapTrustAll;
    }

    public String getXcapDocumentName() {
        return mXcapDocumentName;
    }

    public String getXcapPutElementMime() {
        return mXcapPutElementMime;
    }

    public String getXcapAUID() {
        return mXcapAUID;
    }

    public boolean getDisableETag() {
        return mDisableETag;
    }

    /**
     * Print info.
     *
     * @return String type of info
     */
    public String toString() {
        return "mXcapRoot: " + mXcapRoot + "\n" +
                "mXcapUserAgent: " + mXcapUserAgent + "\n" +
                "mXcapXui: " + mXcapXui + "\n" +
                "mHttpDigestUsername: " + mHttpDigestUsername + "\n" +
                "mHttpDigestPassword: " + mHttpDigestPassword + "\n" +
                "mEnablePredefinedSimservQueryResult: " +
                mEnablePredefinedSimservQueryResult + "\n" +
                "mEnablePredefinedSimservSetting: " +
                mEnablePredefinedSimservSetting + "\n" +
                "mEnableSimservQueryWhole: " + mEnableSimservQueryWhole + "\n" +
                "mEnableHttpLog: " + mEnableHttpLog + "\n" +
                "mEnableXcapTrustAll: " + mEnableXcapTrustAll + "\n" +
                "mXcapDocumentName: " + mXcapDocumentName + "\n" +
                "mXcapPutElementMime: " + mXcapPutElementMime + "\n" +
                "mXcapAUID: " + mXcapAUID + "\n" +
                "mDisableETag: " + mDisableETag + "\n";
    }
}

