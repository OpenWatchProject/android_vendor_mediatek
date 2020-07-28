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

package com.mediatek.xcap.client.uri;

import android.os.SystemProperties;

import java.net.URLEncoder;

import java.net.URI;
import java.net.URISyntaxException;
import java.nio.charset.Charset;

/**
 * XcapUri class.
 */
public class XcapUri {

    private static final String RESOURCE_SELECTOR_SEPARATOR = "/~~/";
    private static final char PATH_SEPARATOR = '/';

    private String mXcapRoot = null;
    private String mDocumentSelector = null;
    private String mNodeSelector = null;

    // Based on the Concept of XPath
    // Example: XCAP root / Document Selector / Node Selector
    // Node Selector
    // ~~/resource-lists/list%5b@name=%22l1%22%5d

    /**
     * Constructor.
     *
     */
    public XcapUri() {

    }

    public String getXcapRoot() {
        return mXcapRoot;
    }

    /**
     * set Xcap Root.
     *
     * @param xcapRoot XCAP root URI
     * @return XcapUri
     * @throws IllegalArgumentException if illegal argument error
     */
    public XcapUri setXcapRoot(String xcapRoot) throws IllegalArgumentException {
        if (xcapRoot.charAt(xcapRoot.length() - 1) != '/') {
            throw new IllegalArgumentException("xcap root must end with /");
        }
        mXcapRoot = xcapRoot;
        return this;
    }

    public String getDocumentSelector() {
        return mDocumentSelector;
    }

    /**
     * set document selector.
     *
     * @param documentSelector XCAP document selector
     * @return XcapUri
     * @throws IllegalArgumentException if illegal argument error
     */
    public XcapUri setDocumentSelector(XcapDocumentSelector documentSelector)
            throws IllegalArgumentException {
        setDocumentSelector(documentSelector.toString());
        return this;
    }

    /**
     * set document selector.
     *
     * @param documentSelector string document selector
     * @return XcapUri
     * @throws IllegalArgumentException if illegal argument error
     */
    public XcapUri setDocumentSelector(String documentSelector)
            throws IllegalArgumentException {
        if (documentSelector.charAt(0) == '/') {
            throw new IllegalArgumentException("document selector must not start with /");
        }
        mDocumentSelector = documentSelector;
        return this;
    }

    public String getNodeSelector() {
        return mNodeSelector;
    }

    /**
     * set node selector.
     *
     * @param nodeSelector XCAP node selector
     * @return XcapUri
     * @throws IllegalArgumentException if illegal argument error
     */
    public XcapUri setNodeSelector(XcapNodeSelector nodeSelector)
            throws IllegalArgumentException {
        setNodeSelector(nodeSelector.toString());
        return this;
    }

    /**
     * set node selector.
     *
     * @param nodeSelector string node selector
     * @return XcapUri
     * @throws IllegalArgumentException if illegal argument error
     */
    public XcapUri setNodeSelector(String nodeSelector)
            throws IllegalArgumentException {
        if (mDocumentSelector.charAt(0) == '/') {
            throw new IllegalArgumentException("document selector must not start with /");
        }
        mNodeSelector = nodeSelector;
        return this;
    }

    /**
     * Convert to URI.
     *
     * @return URI
     * @throws URISyntaxException if URI syntax error
     */
    public URI toURI() throws URISyntaxException {
        final StringBuilder sb = new StringBuilder(mXcapRoot);
        if (("true".equals(SystemProperties.get("persist.vendor.mtk.xcap.rawurl")))||
           ("true".equals(SystemProperties.get("vendor.gsm.radio.ss.rawurl")))) {
            sb.append(mDocumentSelector);
            if (mNodeSelector != null) {
                sb.append(RESOURCE_SELECTOR_SEPARATOR).append(mNodeSelector);
            }
        } else {
            sb.append(mDocumentSelector.replaceAll("\\+", "%2B"));
            if (mNodeSelector != null) {
                sb.append(RESOURCE_SELECTOR_SEPARATOR).append(
                        mNodeSelector.replaceAll("\\+", "%2B"));
            }
        }

        return new URI(sb.toString());
    }

    /**
     * XcapDocumentSelector class.
     *
     */
    static public class XcapDocumentSelector {

        private static final String XCAP_USER_PATH = "users";
        private static final String XCAP_GLOBAL_PATH = "global";

        private StringBuilder mDocumentSelector = new StringBuilder();
        private String mAuid = null;
        private String mXui = null;
        private String mDocumentName = null;

        /**
         * Make XCAP document path.
         *
         * @param newSegment segment
         * @return XcapDocumentSelector
         */
        public XcapDocumentSelector queryPath(String newSegment) {
            if (mDocumentSelector.length() != 0) {
                mDocumentSelector.append(PATH_SEPARATOR);
            }
            mDocumentSelector.append(newSegment);
            return this;
        }

        /**
         * Constructor.
         *
         * @param auid application ID
         * @param xui  XUI
         * @param documentName document name
         */
        public XcapDocumentSelector(String auid, String xui, String documentName) {
            mAuid = auid;
            mXui = xui;
            mDocumentName = documentName;

            this.queryPath(auid).queryPath(XCAP_USER_PATH).queryPath(xui).queryPath(documentName);
        }

        /**
         * Constructor.
         *
         * @param auid application ID
         * @param documentName document name
         */
        public XcapDocumentSelector(String auid, String documentName) {
            StringBuilder documentSelector = new StringBuilder();

            mAuid = auid;
            mDocumentName = documentName;

            this.queryPath(auid).queryPath(XCAP_GLOBAL_PATH).queryPath(documentName);
        }

        /**
         * Convert to string.
         *
         * @return string
         */
        public String toString() {
            return mDocumentSelector.toString();
        }
    }

    /**
     * XcapNodeSelector class.
     *
     */
    static public class XcapNodeSelector {
        private final StringBuilder mNodeSelector = new StringBuilder();

        /**
         * Make node path.
         *
         * @param elementName XML element name
         * @return XcapNodeSelector instance
         */
        // Select by Element Name
        public XcapNodeSelector queryByNodeName(String elementName) {
            if (elementName == null) {
                return this;
            }

            if (mNodeSelector.length() != 0) {
                mNodeSelector.append(PATH_SEPARATOR);
            }
            mNodeSelector.append(elementName);
            return this;
        }

        /**
         * Make attribute path.
         *
         * @param attrName XML attribute name
         * @return XcapNodeSelector instance
         */
        // Select by Attribute Name
        public XcapNodeSelector queryByAttrName(String attrName) {
            if (mNodeSelector.length() != 0) {
                mNodeSelector.append(PATH_SEPARATOR);
            }
            mNodeSelector.append("@").append(attrName);
            return this;
        }

        /**
         * Make element path with attribute.
         *
         * @param elementName XML element name
         * @param attrName    XML attribute name
         * @return XcapNodeSelector instance
         */
        // Select by Element Name with Attribute
        public XcapNodeSelector queryByNodeName(String elementName, String attrName) {
            if (mNodeSelector.length() != 0) {
                mNodeSelector.append(PATH_SEPARATOR);
            }
            mNodeSelector.append(elementName);
            queryByAttrName(attrName);
            return this;
        }

        /**
         * Make element path with attribute name and value.
         *
         * @param elementName XML element name
         * @param attrName    XML attribute name
         * @param attrValue   XML attribute value
         * @return XcapNodeSelector instance
         */
        // Select by Attribute Name/Value
        public XcapNodeSelector queryByNodeName(String elementName, String attrName,
                String attrValue) {
            if (mNodeSelector.length() != 0) {
                mNodeSelector.append(PATH_SEPARATOR);
            }
            mNodeSelector.append(elementName).append("[@").append(attrName).append("=\"")
                    .append(attrValue).append("\"]");
            return this;
        }

        /**
         * Make path by node name.
         *
         * @param elementName XML element name
         * @param pos         position offset
         * @return XcapNodeSelector instance
         */
        // Positional Selectors
        public XcapNodeSelector queryByNodeName(String elementName, int pos) {
            if (mNodeSelector.length() != 0) {
                mNodeSelector.append(PATH_SEPARATOR);
            }
            mNodeSelector.append(elementName).append("[").append(pos).append("]");
            return this;
        }

        /**
         * Make path by node name.
         *
         * @param elementName XML element name
         * @param pos         position offset
         * @param attrName    XML attribute name
         * @param attrValue   XML attribute value
         * @return XcapNodeSelector instance
         */
        // Positional Selectors
        public XcapNodeSelector queryByNodeNameWithPos(String elementName, int pos,
                String attrName, String attrValue) {
            if (mNodeSelector.length() != 0) {
                mNodeSelector.append(PATH_SEPARATOR);
            }
            mNodeSelector.append(elementName).append("[").append(pos).append("]").append("[@")
                    .append(attrName).append("=\"").append(attrValue).append("\"]");
            return this;
        }

        /**
         * Constructor.
         *
         * @param elementName XML element name
         */
        public XcapNodeSelector(String elementName) {
            queryByNodeName(elementName);
        }

        /**
         * Constructor.
         *
         * @param elementName XML element name
         * @param attrName    XML attribute name
         */
        public XcapNodeSelector(String elementName, String attrName) {
            queryByNodeName(elementName, attrName);
        }

        /**
         * Constructor.
         *
         * @param elementName XML element name
         * @param attrName    XML attribute name
         * @param attrValue   XML attribute value
         */
        public XcapNodeSelector(String elementName, String attrName, String attrValue) {
            queryByNodeName(elementName, attrName, attrValue);
        }

        /**
         * Convert to string.
         *
         * @return string
         */
        public String toString() {
            return mNodeSelector.toString();
        }
    }

    /**
     * Encode path in UTF-8.
     *
     * @param path path input
     * @return path in UTF-8
     * @throws NullPointerException if null error
     */
    public static String encodePath(String path) throws NullPointerException {
        if (path == null) {
            throw new NullPointerException("string to encode is null");
        }
        return new String(URLEncoder.encode(path));
    }
}
