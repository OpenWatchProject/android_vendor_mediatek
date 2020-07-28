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
/*
 * Copyright (C) 2018 MediaTek Inc., this file is modified on 07/05/2018
 * by MediaTek Inc. based on Apache License, Version 2.0.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may not
 * use this file except in compliance with the License. See NOTICE for more details.
 */

package com.mediatek.presence.core.ims.service.im.chat;

import gov2.nist.javax2.sip.header.ContentType;
import java.text.CharacterIterator;
import java.text.StringCharacterIterator;

import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.UnsupportedEncodingException;
import java.nio.charset.Charset;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Date;
import java.util.List;
import java.util.TimeZone;


import android.graphics.Bitmap;
import android.graphics.Bitmap.CompressFormat;
import android.graphics.BitmapFactory;
import android.graphics.Matrix;
import android.media.ThumbnailUtils;
import android.provider.MediaStore.Video.Thumbnails;
import android.util.Base64;


import javax2.sip.header.ContactHeader;
import javax2.sip.header.ContentLengthHeader;
import javax2.sip.header.ContentTypeHeader;
import javax2.sip.header.ExtensionHeader;

import org.xml.sax.InputSource;

import com.mediatek.presence.core.ims.network.sip.FeatureTags;
import com.mediatek.presence.core.ims.network.sip.Multipart;
import com.mediatek.presence.core.ims.network.sip.SipUtils;
import com.mediatek.presence.core.ims.protocol.sip.SipRequest;

import com.mediatek.presence.platform.AndroidFactory;
import com.mediatek.presence.provider.settings.RcsSettings;
//import com.mediatek.presence.utils.Base64;

import com.mediatek.presence.utils.logger.Logger;

/**
 * Chat utility functions
 *
 * @author jexa7410
 */
public class ChatUtils {
    /**
     * Anonymous URI
     */
    public final static String ANOMYNOUS_URI = "sip:anonymous@anonymous.invalid";

    /**
     * Contribution ID header
     */
    public static final String HEADER_CONTRIBUTION_ID = "Contribution-ID";

    /**
     * Conversation ID header
     */
    public static final String HEADER_CONVERSATION_ID = "Conversation-ID";

    /**
     * In reply to Conversation ID header
     */
    public static final String HEADER_INREPLY_TO_CONTRIBUTION_ID = "InReplyTo-Contribution-ID";

    /**
     * CRLF constant
     */
    private static final String CRLF = "\r\n";

    private static final String mDefaultListName = "list-1";

    /**
     * The logger
     */
    private static Logger logger = Logger.getLogger("ChatUtils");

    public static String generateSubscribeResourceList(List<String> participants, String listName) {
        StringBuffer uriList = new StringBuffer();

        for(int i=0; i < participants.size(); i++) {
            String contact = participants.get(i);
            uriList.append(" <entry uri=\"" +
                    "tel:"+contact+"\""
                    +"/>"
                    + CRLF);
        }

         String xml = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" + CRLF +
            "<resource-lists xmlns=\"urn:ietf:params:xml:ns:resource-lists\" " +
            "xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\">" +
            "<list name=\""+ listName +"\">" + CRLF +
            uriList.toString() +
            "</list></resource-lists>";
        return xml;
    }

    /**
     * Generate resource-list for a chat session
     *
     * @param participants List of participants
     * @return XML document
     */
    public static String generateSubscribeResourceList(List<String> participants) {
        return generateSubscribeResourceList(participants, "list-1");
    }


}
