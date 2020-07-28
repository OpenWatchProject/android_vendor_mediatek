/*******************************************************************************
 * Software Name : RCS IMS Stack
 *
 * Copyright (C) 2010-2016 Orange.
 * Copyright (C) 2014 Sony Mobile Communications AB.
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
 *
 * NOTE: This file has been modified by Sony Mobile Communications AB.
 * Modifications are licensed under the License.
 ******************************************************************************/

package com.orangelabs.rcs.core.ims.service.richcall;

import com.orangelabs.rcs.core.content.MmContent;
import com.orangelabs.rcs.core.ims.network.NetworkException;
import com.orangelabs.rcs.core.ims.protocol.PayloadException;
import com.orangelabs.rcs.core.ims.protocol.sip.SipException;
import com.orangelabs.rcs.core.ims.protocol.sip.SipRequest;
import com.orangelabs.rcs.core.ims.service.ImsServiceSession;
import com.orangelabs.rcs.core.ims.service.ImsSessionListener;
import com.orangelabs.rcs.core.ims.service.capability.CapabilityService;
//import com.orangelabs.rcs.provider.contact.ContactManager;
import com.orangelabs.rcs.provider.settings.RcsSettings;
import com.orangelabs.rcs.utils.IdGenerator;
import com.orangelabs.rcs.utils.PhoneUtils;
import com.gsma.services.rcs.contact.ContactId;
import com.orangelabs.rcs.core.ims.protocol.sip.SipException;

import android.net.Uri;

/**
 * Content sharing session
 * 
 * @author jexa7410
 */
public abstract class ContentSharingSession extends ImsServiceSession {
    /**
     * Content to be shared
     */
    private MmContent mContent;

    protected final RichcallService mRichcallService;

    protected final CapabilityService mCapabilityService;

    /**
     * Constructor
     * 
     * @param parent Richcall service
     * @param content Content to be shared
     * @param contact Remote contactId
     * @param rcsSettings RCS settings accessor
     * @param timestamp Local timestamp for the session
     * @param contactManager Contact manager accessor
     * @param capabilityService
     */
    public ContentSharingSession(RichcallService parent, MmContent content, ContactId contact,
             long timestamp, 
             CapabilityService capabilityService) {
        super(parent, contact,timestamp);
        mRichcallService = parent;
        mContent = content;
        mCapabilityService = capabilityService;
    }

    /**
     * Returns the content
     * 
     * @return Content
     */
    public MmContent getContent() {
        return mContent;
    }

    /**
     * Set the content
     * 
     * @param content Content
     */
    public void setContent(MmContent content) {
        mContent = content;
    }

    /**
     * Returns the "file-selector" attribute
     * 
     * @return String
     */
    public String getFileSelectorAttribute() {
        return "name:\"" + mContent.getName() + "\"" + " type:" + mContent.getEncoding() + " size:"
                + mContent.getSize();
    }

    /**
     * Returns the "file-location" attribute
     * 
     * @return Uri
     */
    public Uri getFileLocationAttribute() {
        Uri file = null;//mContent.getUri();
        if ((file != null) && file.getScheme().startsWith("http")) {
            return file;
        }
        return null;
    }

    /**
     * Returns the "file-transfer-id" attribute
     * 
     * @return String
     */
    public String getFileTransferId() {
        return "CSh";// + IdGenerator.generateMessageID();
    }

    @Override
    public void receiveBye(SipRequest bye)  {
        super.receiveBye(bye);
        ContactId remote = getRemoteContactId();
        for (ImsSessionListener listener : getListeners()) {
            listener.handleSessionAborted(TERMINATION_BY_REMOTE);
        }
        //mCapabilityService.requestContactCapabilities(remote);
    }

    @Override
    public void receiveCancel(SipRequest cancel) {
        super.receiveCancel(cancel);
        mCapabilityService.requestContactCapabilities(getRemoteContact());
    }
}
