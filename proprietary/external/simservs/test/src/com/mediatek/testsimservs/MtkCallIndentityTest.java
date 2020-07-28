/*
* Copyright (C) 2016 The Android Open Source Project
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
* limitations under the License
*/
package com.mediatek.testsimservs;

import android.test.AndroidTestCase;
import android.util.Log;
import com.mediatek.simservs.client.*;
import com.mediatek.simservs.client.policy.*;
import com.mediatek.testsimservs.mock.*;
import com.mediatek.xcap.client.XcapConstants;
import com.mediatek.xcap.client.uri.XcapUri;
import com.mediatek.xcap.client.uri.XcapUri.XcapDocumentSelector;

public class MtkCallIndentityTest extends AndroidTestCase {

    final private static String TAG = "MTKSimservsTest";

    final private String TEST_USER = "sip:user@anritsu-cscf.com";
    final private String TEST_DOC = "simservs";
    final private String XCAP_ROOT = "http://192.168.1.2:8080/";

    MTKMockTIndetityPresentationRestriction mIndentityPresentationRestrition = null;
    DefaultBehaviour mDefaultBehaviour = null;
    SimServs mSimservs = SimServs.getInstance();
    //define CB xml
    final private String indentity_presentation_doc =
            "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" +
                    " <simservs xmlns=\"http://uri.etsi.org/ngn/params/xml/simservs/xcap\">\r\n" +
                    " <originating-identity-presentation-restriction active=\"false\">\r\n" +
                    "<default-behaviour>presentation-not-restricted</default-behaviour>\r\n" +
                    "</originating-identity-presentation-restriction>\r\n" +
                    " </simservs>\r\n";


    public void setUp() throws Exception {
        super.setUp();
        Log.d(TAG, "[setUp]");
        XcapDocumentSelector documentSelector = new XcapDocumentSelector(
                XcapConstants.AUID_RESOURCE_LISTS, TEST_USER, TEST_DOC);
        XcapUri xcapUri = new XcapUri();
        xcapUri.setXcapRoot(XCAP_ROOT).setDocumentSelector(documentSelector);
        mSimservs.setIntendedId("user@chinaTel.com");
        MtkMockLoader loader = new MtkMockLoader();
        // load indentity
        mIndentityPresentationRestrition = new MTKMockTIndetityPresentationRestriction(xcapUri, null,
                mSimservs.getIntendedId());
        mIndentityPresentationRestrition.setContent(indentity_presentation_doc);
        mIndentityPresentationRestrition.loadConfiguration(loader);

    }

    @Override
    public void tearDown() throws Exception {
        super.tearDown();
        Log.d(TAG, "[tearDown]");
        mIndentityPresentationRestrition = null;
    }

    public void test08_GetIndentityPresentationRestrition() {
        boolean isRestricted = mIndentityPresentationRestrition.isDefaultPresentationRestricted();
        assertEquals(isRestricted, false);
    }

    public void test09_SetIndentityPresentationRestritionBehavior() {
        String xml = mIndentityPresentationRestrition.setDefaultPresentationRestrictedForTest(true);
        int startIndexOi = xml.indexOf("<default-behaviour>");
        int endIndexOi = xml.indexOf("</default-behaviour>");
        String testString = xml.substring(startIndexOi+"<default-behaviour>".length(), endIndexOi);
        assertEquals(testString, "presentation-restricted");
    }
}