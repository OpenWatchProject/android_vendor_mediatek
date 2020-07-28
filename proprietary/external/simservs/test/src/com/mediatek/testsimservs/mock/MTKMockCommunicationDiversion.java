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

import com.mediatek.simservs.client.CommunicationDiversion;
import com.mediatek.simservs.xcap.XcapException;
import com.mediatek.xcap.client.uri.XcapUri;
import javax.xml.parsers.ParserConfigurationException;
import org.w3c.dom.*;

public class MTKMockCommunicationDiversion extends CommunicationDiversion {
    String xmlContent = "";
    MtkMockLoader mLoader = new MtkMockLoader();
    public MTKMockCommunicationDiversion(XcapUri documentUri, String parentUri, String intendedId)
            throws XcapException, ParserConfigurationException {
        super(documentUri, parentUri, intendedId);
    }
    public void setContent(String mtkContent) {
        xmlContent = mtkContent;
    }

    public void loadConfiguration(MtkMockLoader loader) throws XcapException,
            ParserConfigurationException {
            Document doc = mLoader.loadConfiguration(xmlContent, getNodeName(),mActived);
            initServiceInstance(doc);
    }
}
