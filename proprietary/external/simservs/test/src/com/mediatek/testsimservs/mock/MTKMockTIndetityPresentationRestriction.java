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

package com.mediatek.testsimservs.mock;

import com.mediatek.simservs.xcap.XcapException;
import com.mediatek.xcap.client.uri.XcapUri;
import org.w3c.dom.Document;
import com.mediatek.simservs.client.TerminatingIdentityPresentationRestriction;
import javax.xml.parsers.ParserConfigurationException;

public class MTKMockTIndetityPresentationRestriction
        extends  TerminatingIdentityPresentationRestriction {
    String xmlContent = "";
    MtkMockLoader mLoader = new MtkMockLoader();

    public MTKMockTIndetityPresentationRestriction(XcapUri documentUri, String parentUri,
                                                   String intendedId)
            throws Exception {
        super(documentUri, parentUri, intendedId);
    }

    public void setContent(String mtkContent) {
        xmlContent = mtkContent;
    }

    public void loadConfiguration(MtkMockLoader loader) throws XcapException,
            ParserConfigurationException {
        Document doc = mLoader.loadConfiguration(xmlContent, getNodeName(), mActived);
        initServiceInstance(doc);
    }
// FIXME:here I think we should modify later if we have a better way, orginal code do many
// things in one method, so we just modify the method, test point is setPresentationRestricted().
    public String setDefaultPresentationRestrictedForTest(boolean presentationRestricted) {
        mDefaultBehaviour.setPresentationRestricted(presentationRestricted);

        if (isDefaultPresentationRestricted()) {
            String defaultBehaviourXml = mDefaultBehaviour.toXmlString();
            return defaultBehaviourXml;
        } else {
            return toXmlString();
        }
    }
}
