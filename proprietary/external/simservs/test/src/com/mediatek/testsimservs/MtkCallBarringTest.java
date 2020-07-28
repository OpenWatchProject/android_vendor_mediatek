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

import java.util.List;

public class MtkCallBarringTest extends AndroidTestCase {

    final private static String TAG = "MTKSimservsTest";

    final private String TEST_USER = "sip:user@anritsu-cscf.com";
    final private String TEST_DOC = "simservs";
    final private String XCAP_ROOT = "http://192.168.1.2:8080/";

    MTKMockOutgoingCommunicationBarring mOcb = null;
    SimServs mSimservs = SimServs.getInstance();
    //define CB xml
    final private String call_barring_doc =
            "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" +
                    " <simservs xmlns=\"http://uri.etsi.org/ngn/params/xml/simservs/xcap\">\r\n" +
                    "        <outgoing-communication-barring active=\"true\">\r\n" +
                    "             <ruleset>\r\n" +
                    "                  <rule id=\"AO\">\r\n" +
                    "                         <cp:conditions>" +
                    "                               <media>audio</media>" +
                    "                               <roaming/>" +
                    "                         </cp:conditions>" +
                    "                       <actions>\r\n" +
                    "                            <allow>false</allow>\r\n" +
                    "                       </actions>\r\n" +
                    "                  </rule>\r\n" +
                    "             </ruleset>\r\n" +
                    "        </outgoing-communication-barring>\r\n" +
                    "        </simservs>\r\n";


    public void setUp() throws Exception {
        super.setUp();
        Log.d(TAG, "[setUp]");
        XcapDocumentSelector documentSelector = new XcapDocumentSelector(
                XcapConstants.AUID_RESOURCE_LISTS, TEST_USER, TEST_DOC);
        XcapUri xcapUri = new XcapUri();
        xcapUri.setXcapRoot(XCAP_ROOT).setDocumentSelector(documentSelector);
        mSimservs.setIntendedId("user@chinaTel.com");
        MtkMockLoader loader = new MtkMockLoader();
        // load OCB
        mOcb = new MTKMockOutgoingCommunicationBarring(xcapUri, null, mSimservs.getIntendedId());
        mOcb.setContent(call_barring_doc);
        mOcb.loadConfiguration(loader);

    }

    @Override
    public void tearDown() throws Exception {
        super.tearDown();
        Log.d(TAG, "[tearDown]");
        mOcb = null;
    }

    public void test01_GetOutgoingCallBarring() {
        try {
            RuleSet ruleSet = mOcb.getRuleSet();
            List<Rule> ruleList = ruleSet.getRules();
            Log.d(TAG, "[test01_GetOutgoingCallBarring] ruleList.size()-->" + ruleList.size());
            assertEquals(ruleList.size(), 1);
            for (int i = 0; i < ruleList.size(); i++) {
                Rule r = ruleList.get(i);
                Conditions cond = r.getConditions();
                Actions action = r.getActions();
                List<String> mediaList = null;
                if (cond != null) {
                    assertEquals(cond.comprehendRoaming(), true);
                    mediaList = cond.getMedias();
                    assertEquals(mediaList.size(), 1);
                    assertEquals(mediaList.get(0), "audio");
                }
            }
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    public void test02_NewRuleForOutgoingCallBarring() {
        try {
            RuleSet ruleSet = mOcb.getRuleSet();
            Rule rule = ruleSet.createNewRule("OI");
            Conditions cond = rule.createConditions();
            Actions act = rule.createActions();
            cond.addInternational();
            act.setAllow(false);
            // check rule list begin
            String xml = ruleSet.toXmlString();
            Log.d(TAG, "[test02_NewRuleForOutgoingCallBarring] xml" + xml);
            // check rule list end.
            int startIndexOi = xml.indexOf("<cp:rule id=\"OI\">");
            String oiString = xml.substring(startIndexOi, xml.length());
            int startOIindex = oiString.indexOf("international");
            assertEquals((startOIindex > 0), true);
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    public void test03_ModifyOutgoingCallBarring() {
        try {
            RuleSet ruleSet = mOcb.getRuleSet();
            // modify "OA" rule
            Conditions cond = ruleSet.getRules().get(0).getConditions();
            cond.addRuleDeactivated();
            // check new rule begin.
            String xml = ruleSet.toXmlString();
            Log.d(TAG, "[test03_ModifyOutgoingCallBarring] xml" + xml);
            // check new rule end.
            int startOAindex = xml.indexOf("rule-deactivated");
            assertEquals((startOAindex > 0), true);

        } catch (Exception e) {
            e.printStackTrace();
        }
    }
}