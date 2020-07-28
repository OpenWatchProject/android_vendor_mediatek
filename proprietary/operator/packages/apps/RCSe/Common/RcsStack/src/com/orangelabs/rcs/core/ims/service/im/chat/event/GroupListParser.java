/*******************************************************************************
 * Software Name : RCS IMS Stack
 *
 * Copyright (C) 2010 France Telecom S.A.
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
 * limitations under the License.
 ******************************************************************************/
package com.orangelabs.rcs.core.ims.service.im.chat.event;

import javax.xml.parsers.SAXParser;
import javax.xml.parsers.SAXParserFactory;

import org.xml.sax.Attributes;
import org.xml.sax.InputSource;
import org.xml.sax.helpers.DefaultHandler;

import com.orangelabs.rcs.core.ims.service.im.chat.event.GroupListDocument.BasicGroupInfo;
import com.orangelabs.rcs.utils.logger.Logger;

/**
 * Conference-Info parser
 */
public class GroupListParser extends DefaultHandler {

    /*
     * Conference-Info SAMPLE:
     *   <conference-info entity="sip:+8613601039770@ims.mnc000.mcc460.3gppnetwork.org"
     *      state="list" xmlns="urn:ietf:params:xml:ns:conference-info">
     *     <conference-list>
     *       <grouplist-ver version="5"/>
     *       <grouplist>
     *         <group-info entity="sip:1252000199zC6c805000@as99.gc.rcs1.chinamobile.com">
     *           <conversationid>bd708545839a5ef4a9acfe47c8f4056d</conversationid>
     *           <name>9323,CJ</name>
     *         </group-info>
     *         <group-info entity="sip:1252000199cZkZ802x30@as99.gc.rcs1.chinamobile.com">
     *           <conversationid>03a45f943fee1f192e67f3ca9c280edb</conversationid>
     *           <name>Shuo,Test,jianhua</name>
     *         </group-info>
     *         <group-info entity="sip:1252000199zCMa80L-50@as99.gc.rcs1.chinamobile.com">
     *           <conversationid>2a4e4cebd7e92ab2dd00011c5f99ebf0</conversationid>
     *           <name>123456</name>
     *         </group-info>
     *       </grouplist>
     *     </conference-list>
     *   </conference-info>
     */
    public static final String TAG_CONFERENCE_INFO = "conference-info";
    public static final String TAG_GROUP_INFO = "group-info";
    public static final String TAG_CONVERSATION_ID = "conversationid";
    public static final String TAG_NAME = "name";
    private GroupListDocument groupList = null;

    private String entity = null;
    private String conversationid = null;
    private String name = null;

    private static String tagName = null;

    /**
     * The logger
     */
    private Logger logger = Logger.getLogger(this.getClass().getSimpleName());

    /**
     * Constructor
     *
     * @param inputSource
     *            Input source
     * @throws Exception
     */
    public GroupListParser(InputSource inputSource) throws Exception {
        SAXParserFactory factory = SAXParserFactory.newInstance();
        SAXParser parser = factory.newSAXParser();
        parser.parse(inputSource, this);
    }

    public GroupListDocument getGroupList() {
        return groupList;
    }

    @Override
    public void startDocument() {
        if (logger.isActivated()) {
            logger.debug("Start document");
        }
    }

    @Override
    public void characters(char buffer[], int start, int length) {
        if (logger.isActivated()) {
            logger.info("characters tagName: " + tagName +
                    " buffer start: " + start +
                    " buffer length: " + length);
        }
        if (tagName == null)
            return;

        if (tagName.equals(TAG_CONVERSATION_ID)) {
            conversationid = new String(buffer, start, length);
        } else if (tagName.equals(TAG_NAME)) {
            name = new String(buffer, start, length);
        }
    }

    @Override
    public void startElement(String namespaceURL, String localName, String qname, Attributes attr) {
        if (logger.isActivated()) {
            logger.info("startElement namespaceUrl: " + namespaceURL +
                    " localName: " + localName +
                    " qname: " + qname);
        }
        if (namespaceURL == null || namespaceURL.isEmpty()) {
            tagName = qname;
        } else {
            tagName = localName;
        }
        if (tagName.equals(TAG_CONFERENCE_INFO)) {
            String entity = attr.getValue("entity").trim();
            String state = attr.getValue("state").trim();
            groupList = new GroupListDocument(entity, state);
        } else if (tagName.equals(TAG_GROUP_INFO)) {
            entity = attr.getValue("entity").trim();
        }
    }

    @Override
    public void endElement(String namespaceURL, String localName, String qname) {
        if (logger.isActivated()) {
            logger.info("endElement namespaceUrl: " + namespaceURL +
                    "localName: " + localName +
                    "qname: " + qname);
        }
        this.tagName = null;

        String tagName;
        if (namespaceURL == null || namespaceURL.isEmpty()) {
            tagName = qname;
        } else {
            tagName = localName;
        }
        if (tagName.equals(TAG_GROUP_INFO)) {
            BasicGroupInfo groupinfo = new BasicGroupInfo.Builder()
                    .setUri(entity)
                    .setConversationId(conversationid)
                    .setSubject(name)
                    .build();
            groupList.addGroup(groupinfo);
        }
    }

    @Override
    public void endDocument() {
        if (logger.isActivated()) {
            logger.debug("End document");
        }
    }
}
