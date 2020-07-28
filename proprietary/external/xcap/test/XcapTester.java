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

package com.mediatek.xcap.test;

import android.util.Log;

import com.mediatek.xcap.client.XcapClient;
import com.mediatek.xcap.client.XcapConstants;
import com.mediatek.xcap.client.uri.XcapUri;
import com.mediatek.xcap.client.uri.XcapUri.XcapDocumentSelector;
import com.mediatek.xcap.client.uri.XcapUri.XcapNodeSelector;

import org.apache.http.HttpResponse;
import org.apache.http.auth.UsernamePasswordCredentials;

import java.io.IOException;
import java.net.URI;
import java.net.URISyntaxException;

/**
 * XcapTester class.
 */
public class XcapTester {
    static final private String INITIAL_DOCUMENT =
            "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\r\n" +
                    "    <resource-lists xmlns=\"urn:ietf:params:xml:ns:resource-lists\">\r\n" +
                    "        <list name=\"friends\">\r\n" +
                    "        </list>\r\n" +
                    "    </resource-lists>\r\n";

    static final private String XCAP_ROOT = "http://172.23.4.42:80/xcap-root/";
    static final private String TEST_USER = "xcapTest@example.com";
    static final private String TEST_DOC = "testDoc";

    static final private String TAG = "XcapTester";

    private static void getDoc(XcapClient ra,
            URI documentURI,
            UsernamePasswordCredentials credentials) throws IOException {

        HttpResponse response;
        response = ra.get(documentURI, null);

        if (response != null) {
            if (response.getStatusLine().getStatusCode() == 200) {
                Log.d("info", "response 200, response = " + response.toString());
            } else {
                Log.d("server", "bad response from xcap server: " + response.toString());
            }
        } else {
            Log.d("server", "unable to retreive document in xcap server...");
        }
    }

    /**
     * Test suite.
     *
     * @throws IOException if I/O error
     * @throws URISyntaxException if URI syntax error
     */
    static public void syncTest() throws IOException, URISyntaxException {

        /** Put a XML document **/
        XcapClient ra = new XcapClient();
        UsernamePasswordCredentials credentials = new UsernamePasswordCredentials("userName",
                "password");

        XcapDocumentSelector documentSelector = new XcapDocumentSelector(
                XcapConstants.AUID_RESOURCE_LISTS, TEST_USER, TEST_DOC);
        Log.d(TAG, "document selector is " + documentSelector.toString());
        XcapUri xcapUri = new XcapUri();
        xcapUri.setXcapRoot(XCAP_ROOT).setDocumentSelector(documentSelector);

        URI documentURI = xcapUri.toURI();
        HttpResponse response;

        // put the document and get sync response
        response = ra.put(documentURI, "application/resource-lists+xml", INITIAL_DOCUMENT);
        // check put response
        if (response != null) {
            if (response.getStatusLine().getStatusCode() == 200
                    || response.getStatusLine().getStatusCode() == 201) {
                Log.d("info", "document created in xcap server...");
            } else {
                Log.d("server", "bad response from xcap server: " + response.toString());
            }
        } else {
            Log.d("server", "unable to create document in xcap server...");
        }
        // ====================================
        // get the document and check content is ok
        getDoc(ra, documentURI, credentials);
        // ====================================
        /** Get an element node **/
        XcapNodeSelector elementSelector = new XcapNodeSelector(XcapConstants.AUID_RESOURCE_LISTS)
                .queryByNodeName("list", "name", "friends");
        URI elementURI = xcapUri.setNodeSelector(elementSelector).toURI();

        // get the document and check content is ok
        response = ra.get(elementURI, null);

        // check get response
        if (response != null) {
            if (response.getStatusLine().getStatusCode() == 200) {
                Log.d("info", "document retreived in xcap server and content is the expected...");
                Log.d("info", "sync test suceed :)");
            } else {
                Log.d("server", "bad response from xcap server: " + response.toString());
            }
        } else {
            Log.d("server", "unable to retreive document in xcap server...");
        }
        // ====================================
        /** Append an element **/
        elementSelector = new XcapNodeSelector(XcapConstants.AUID_RESOURCE_LISTS)
                .queryByNodeName("list", "name", "friends")
                .queryByNodeName("entry");

        elementURI = xcapUri.setNodeSelector(elementSelector).toURI();
        String newFriend = "            <entry uri=\"sip:Seraph@example.com\">\r\n" +
                "                <display-name>Seraph Huang</display-name>\r\n" +
                "            </entry>\r\n";
        response = ra.put(elementURI, "application/xcap-el+xml", newFriend);

        // check put response
        if (response != null) {
            if (response.getStatusLine().getStatusCode() == 201) {
                Log.d("info", "element created in xcap server...");
            } else {
                Log.d("server", "bad response from xcap server: " + response.toString());
            }
        } else {
            Log.d("server", "unable to create element in xcap server...");
        }
        // ====================================
        // get the document and check content is ok
        getDoc(ra, documentURI, credentials);
        // ====================================
        /** Insert an element by position indiction **/
        elementSelector = new XcapNodeSelector(XcapConstants.AUID_RESOURCE_LISTS)
                .queryByNodeName("list", "name", "friends")
                .queryByNodeNameWithPos("entry", 1, "uri", "sip:Johnny@example.com");

        elementURI = xcapUri.setNodeSelector(elementSelector).toURI();
        String newFriend1 = "            <entry uri=\"sip:Johnny@example.com\">\r\n" +
                "                <display-name>Johnny Shih</display-name>\r\n" +
                "            </entry>\r\n";
        response = ra.put(elementURI, "application/xcap-el+xml", newFriend1);

        // check put response
        if (response != null) {
            if (response.getStatusLine().getStatusCode() == 201) {
                Log.d("info", "element created in xcap server...");
            } else {
                Log.d("server", "bad response from xcap server: " + response.toString());
            }
        } else {
            Log.d("server", "unable to create element in xcap server...");
        }
        // ====================================
        // get the document and check content is ok
        getDoc(ra, documentURI, credentials);
        // ====================================
        /** Modify a node attribute **/
        elementSelector = new XcapNodeSelector(XcapConstants.AUID_RESOURCE_LISTS)
                .queryByNodeName("list", "name", "friends")
                .queryByNodeName("entry", 1);
        elementURI = xcapUri.setNodeSelector(elementSelector).toURI();
        String newFriend2 = "            <entry uri=\"sip:Johnny1@example.com\">\r\n" +
                "                <display-name>Johnny Shih</display-name>\r\n" +
                "            </entry>\r\n";
        response = ra.put(elementURI, "application/xcap-el+xml", newFriend2);

        // check put response
        if (response != null) {
            if (response.getStatusLine().getStatusCode() == 200) {
                Log.d("info", "element created in xcap server...");
            } else {
                Log.d("server", "bad response from xcap server: " + response.toString());
            }
        } else {
            Log.d("server", "unable to create element in xcap server...");
        }
        // ====================================
        // get the document and check content is ok
        getDoc(ra, documentURI, credentials);
        // ====================================
        /** Fetch an attribute **/
        elementSelector = new XcapNodeSelector(XcapConstants.AUID_RESOURCE_LISTS)
                .queryByNodeName("list", "name", "friends")
                .queryByNodeName("entry", 1)
                .queryByAttrName("uri");
        elementURI = xcapUri.setNodeSelector(elementSelector).toURI();

        response = ra.get(elementURI, null);

        // check put response
        if (response != null) {
            if (response.getStatusLine().getStatusCode() == 200) {
                Log.d("info", "element created in xcap server...");
            } else {
                Log.d("server", "bad response from xcap server: " + response.toString());
            }
        } else {
            Log.d("server", "unable to create element in xcap server...");
        }
        // ====================================
        // get the document and check content is ok
        getDoc(ra, documentURI, credentials);
        // ====================================
        /** Delete an element **/
        elementSelector = new XcapNodeSelector(XcapConstants.AUID_RESOURCE_LISTS)
                .queryByNodeName("list", "name", "friends")
                .queryByNodeNameWithPos("entry", 1, "uri", "sip:Johnny1@example.com");

        elementURI = xcapUri.setNodeSelector(elementSelector).toURI();
        response = ra.delete(elementURI);

        // check put response
        if (response != null) {
            if (response.getStatusLine().getStatusCode() == 200) {
                Log.d("info", "element created in xcap server...");
            } else {
                Log.d("server", "bad response from xcap server: " + response.toString());
            }
        } else {
            Log.d("server", "unable to create element in xcap server...");
        }
        // ====================================
        // get the document and check content is ok
        getDoc(ra, documentURI, credentials);
        // ====================================
        /** Delete document **/
        response = ra.delete(documentURI);

        // check get response
        if (response != null) {
            if (response.getStatusLine().getStatusCode() == 200) {
                Log.d("info", "document retreived in xcap server and content is the expected...");
                Log.d("info", "sync test suceed :)");
            } else {
                Log.d("server", "bad response from xcap server: " + response.toString());
            }
        } else {
            Log.d("server", "unable to retreive document in xcap server...");
        }
        // ====================================
        // get the document and check content is ok
        getDoc(ra, documentURI, credentials);
        // ====================================
        ra.shutdown();
    }
}
