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

public class MTKCommunicationTest extends AndroidTestCase {

    final private static String TAG = "MTKSimservsTest";

    final private String TEST_USER = "sip:user@anritsu-cscf.com";
    final private String TEST_DOC = "simservs";
    final private String XCAP_ROOT = "http://192.168.1.2:8080/";

    MTKMockOutgoingCommunicationBarring mOcb = null;
    MTKMockCommunicationDiversion mCd = null;
    SimServs mSimservs = SimServs.getInstance();

    //define CF xml
    final private String call_forwarding_doc =
            "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" +
                    " <simservs xmlns=\"http://uri.etsi.org/ngn/params/xml/simservs/xcap\">\r\n" +
                    "        <communication-diversion active=\"true\">\r\n" +
                    "             <ruleset>\r\n" +
                    "                  <rule id=\"call-forwarding-enhanced-rule\">\r\n" +
                    "                         <cp:conditions>" +
                    "                               <media>audio</media>" +
                    "                         </cp:conditions>" +
                    "                       <actions>\r\n" +
                    "                            <forward-to>\r\n" +
                    "                                <target>tel:18628336275</target>\r\n" +
                    "                                <notify-caller>false</notify-caller>\r\n" +
                    "                            </forward-to>\r\n" +
                    "                       </actions>\r\n" +
                    "                  </rule>\r\n" +
                    "             </ruleset>\r\n" +
                    "        </communication-diversion>\r\n" +
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

        // load CF
        mCd = new MTKMockCommunicationDiversion(xcapUri, null, mSimservs.getIntendedId());
        mCd.setContent(call_forwarding_doc);
        mCd.loadConfiguration(loader);
    }

    @Override
    public void tearDown() throws Exception {
        super.tearDown();
        Log.d(TAG, "[tearDown]");
        mOcb = null;
    }

    public void test04_GetCallForwarding() {
        Log.d(TAG, "[test04_GetCallForwarding]");
        try {
            RuleSet ruleSet = mCd.getRuleSet();
            List<Rule> ruleList = ruleSet.getRules();
            Log.d(TAG, "[test04_GetCallForwarding] ruleList.size()-->" + ruleList.size());
            assertEquals(ruleList.size(), 1);
            for (int i = 0; i < ruleList.size(); i++) {
                Rule r = ruleList.get(i);
                Conditions cond = r.getConditions();
                Actions action = r.getActions();
                List<String> mediaList = null;
                if (cond != null) {
                    assertEquals(cond.comprehendRoaming(), false);
                    mediaList = cond.getMedias();
                    assertEquals(mediaList.size(), 1);
                    assertEquals(mediaList.get(0), "audio");
                }
                if (action != null) {
                    ForwardTo forward = action.getFowardTo();
                    String target = forward.getTarget();
                    assertEquals(forward.getTarget(), "tel:18628336275");
                    assertFalse(forward.isNotifyCaller());
                }
            }

        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    public void test05_NewRuleForCallForwarding() {
        Log.d(TAG, "[test05_NewRuleForCallForwarding]");
        try {
            RuleSet ruleSet = mCd.getRuleSet();
            Rule rule = ruleSet.createNewRule("CFB");
            Conditions cond = rule.createConditions();
            Actions act = rule.createActions();
            cond.addBusy();
            cond.addRuleDeactivated();
            act.setFowardTo("tel:18828091540", true);
            // check rule list begin
            String xml = ruleSet.toXmlString();
            Log.d(TAG, "[test05_NewRuleForCallForwarding] xml" + xml);
            // check rule list end.
            int startIndexCd = xml.indexOf("<cp:rule id=\"CFB\">");
            String cdString = xml.substring(startIndexCd, xml.length());
            int startCFBindex = cdString.indexOf("busy");
            assertEquals((startCFBindex > 0), true);
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    public void test06_ModifyRuleVideoForCallForwarding() {
        Log.d(TAG, "[test06_ModifyRuleForCallForwarding]");
        try {
            RuleSet ruleSet = mCd.getRuleSet();
            // modify "CFB" rule
            Conditions cond = ruleSet.getRules().get(0).getConditions();
            cond.addMedia("video");
            // check new rule begin.
            String xml = ruleSet.toXmlString();
            Log.d(TAG, "[test06_ModifyRuleForCallForwarding] xml" + xml);
            // check new rule end.
            int startCfindex = xml.indexOf("video");
            assertEquals((startCfindex > 0), true);
        } catch (Exception e) {
            e.printStackTrace();
        }
    }
    public void test07_ModifyRuleNumberForCallForwarding() {
        Log.d(TAG, "[test06_ModifyRuleForCallForwarding]");
        try {
            String newNumber = "12345678";
            RuleSet ruleSet = mCd.getRuleSet();
            // modify "CFB" rule,set number as
            ruleSet.getRules().get(0).getActions().getFowardTo().setTarget(newNumber);
            // check new rule begin.
            String xml = ruleSet.toXmlString();
            Log.d(TAG, "[test06_ModifyRuleForCallForwarding] xml" + xml);
            // check new rule end.
            int startCfNumberindex = xml.indexOf("<target>");
            int endCfNumberindex = xml.indexOf("</target>");
            String putNewNumber = xml.substring(startCfNumberindex+"<target>".length(), endCfNumberindex);
            assertEquals(newNumber, putNewNumber);
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

}