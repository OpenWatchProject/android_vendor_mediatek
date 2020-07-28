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

package com.mediatek.gba;

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
 * GBA debug configuration class.
 */
public class GbaDebugParam {
    private static final String TAG = "GbaDebugParam";
    private static final String TAG_ROOT = "DebugParam";
    private static final String TAG_GBA_BSF_SERVER_URL = "BsfServerUrl";
    private static final String TAG_ENABLE_GBA_TRUST_ALL = "EnableGbaTrustAll";
    private static final String TAG_ENABLE_GBA_FORCE_RUN = "EnableGbaForceRun";
    private static final String TAG_GBA_TYPE = "GbaType";

    private static GbaDebugParam sInstance;

    private String mBsfServerUrl;
    private boolean mEnableGbaTrustAll = false;
    private boolean mEnableGbaForceRun = false;
    private String mGbaType;

    /**
     * Singleton.
     *
     * @return GbaDebugParam instance
     */
    public static GbaDebugParam getInstance() {
        if (sInstance == null) {
            sInstance = new GbaDebugParam();
        }
        return sInstance;
    }

    /**
     * Load debug configuration from XML file.
     */
    public void load() {

        readProperty();

        String xmlContent = readXmlFromFile("/data/misc/gbaconfig.xml");
        if (xmlContent == null) {
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

    public String getBsfServerUrl() {
        return mBsfServerUrl;
    }

    public boolean getEnableGbaTrustAll() {
        return mEnableGbaTrustAll;
    }

    public boolean getEnableGbaForceRun() {
        return mEnableGbaForceRun;
    }

    public String getGbaType() {
        return mGbaType;
    }

    private String readXmlFromFile(String file) {
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
        val = SystemProperties.get("persist.vendor.ss.gba." + TAG_GBA_BSF_SERVER_URL.toLowerCase());
        mBsfServerUrl = (val != null && !val.isEmpty()) ? val : null;
        mEnableGbaTrustAll = SystemProperties.getBoolean(
            "persist.vendor.ss.gba." + TAG_ENABLE_GBA_TRUST_ALL.toLowerCase(), false);
        mEnableGbaForceRun = SystemProperties.getBoolean(
            "persist.vendor.ss.gba." + TAG_ENABLE_GBA_FORCE_RUN.toLowerCase(), false);
        mGbaType = SystemProperties.get("persist.vendor.ss.gba." + TAG_GBA_TYPE.toLowerCase());

        GbaConstant.PiiLog(TAG, "mBsfServerUrl: " + mBsfServerUrl + "\n" +
                "mEnableGbaTrustAll: " + mEnableGbaTrustAll + "\n" +
                "mEnableGbaForceRun: " + mEnableGbaForceRun + "\n" +
                "mGbaType: " + mGbaType + "\n");
    }

    private void reset() {
        mBsfServerUrl = null;
        mEnableGbaTrustAll = false;
        mEnableGbaForceRun = false;
        mGbaType = null;
    }

    private void instantiateFromXmlNode(Node domNode) {
        Element domElement = (Element) domNode;
        NodeList node = domElement.getElementsByTagName(TAG_GBA_BSF_SERVER_URL);
        if (node.getLength() > 0) {
            mBsfServerUrl = ((Element) node.item(0)).getTextContent();
        }

        node = domElement.getElementsByTagName(TAG_ENABLE_GBA_TRUST_ALL);
        if (node.getLength() > 0) {
            String str = ((Element) node.item(0)).getTextContent();
            if ("true".equalsIgnoreCase(str)) {
                mEnableGbaTrustAll = true;
            } else {
                mEnableGbaTrustAll = false;
            }
        }

        node = domElement.getElementsByTagName(TAG_ENABLE_GBA_FORCE_RUN);
        if (node.getLength() > 0) {
            String str = ((Element) node.item(0)).getTextContent();
            if ("true".equalsIgnoreCase(str)) {
                mEnableGbaForceRun = true;
            } else {
                mEnableGbaForceRun = false;
            }
        }

        node = domElement.getElementsByTagName(TAG_GBA_TYPE);
        if (node.getLength() > 0) {
            mGbaType = ((Element) node.item(0)).getTextContent();
        }

    }

    /**
     * Print info.
     *
     * @return String type of info
     */
    public String toString() {
        return "mBsfServerUrl: " + mBsfServerUrl + "\n" +
                "mEnableGbaTrustAll: " + mEnableGbaTrustAll + "\n" +
                "mEnableGbaForceRun: " + mEnableGbaForceRun + "\n" +
                "mGbaType: " + mGbaType;
    }
}

