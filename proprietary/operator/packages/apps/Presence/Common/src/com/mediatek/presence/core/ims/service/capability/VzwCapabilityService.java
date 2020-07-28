/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2018. All rights reserved.
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

package com.mediatek.presence.core.ims.service.capability;

import java.util.ArrayList;
import java.util.List;
import java.util.HashSet;
import java.util.Iterator;
import android.os.Looper;
import android.os.Handler;
import java.util.concurrent.locks.ReentrantLock;

import android.database.Cursor;
import android.os.Build;
import android.os.HandlerThread;

import android.provider.ContactsContract.CommonDataKinds.Phone;

import com.mediatek.presence.addressbook.AddressBookEventListener;
import com.mediatek.presence.core.CoreException;
import com.mediatek.presence.core.ims.ImsModule;
import com.mediatek.presence.core.ims.protocol.sip.SipRequest;
import com.mediatek.presence.core.ims.service.ContactInfo;
import com.mediatek.presence.core.ims.service.ImsService;
import com.mediatek.presence.platform.AndroidFactory;
import com.mediatek.presence.provider.eab.ContactsManager;
import com.mediatek.presence.provider.settings.RcsSettings;
import com.mediatek.presence.utils.ContactNumberUtils;
import com.mediatek.presence.utils.PhoneUtils;
import com.mediatek.presence.utils.logger.Logger;

/**
 * Capability discovery service
 */
public class VzwCapabilityService extends CapabilityService {

    private CapabilityPollingManager mCapabilityPollingManager;
    private AvailabilityFetchManager mAvailabilityFetchManager;

    private HandlerThread mHandlerThread;

    private ImsModule mImsModule;

    public static final int SINGLE_SUBSCRIBE_TYPE_AVAILABILITY = 0;
    public static final int SINGLE_SUBSCRIBE_TYPE_CAPABILITY = 1;

    /**
     * Constructor
     *
     * @param parent
     *            IMS module
     * @throws CoreException
     */
    public VzwCapabilityService(ImsModule imsModule) throws CoreException {
        super(imsModule);
        logger = Logger.getLogger(mSlotId, "VzwCapabilityService");
        mImsModule = imsModule;
        mHandlerThread = new HandlerThread("VzWCapabilityService");
        mHandlerThread.start();
        mCapabilityPollingManager = new CapabilityPollingManager(
                    CapabilityPollingManager.class.getSimpleName(),
                    new Handler(mHandlerThread.getLooper()),
                    imsModule);
        mAvailabilityFetchManager = new AvailabilityFetchManager(imsModule);
    }

    /**
     * Start the IMS service
     */
    @Override
    public synchronized void start() {
        logger.debug("start()");
        if (isServiceStarted()) {
            // Already started
            return;
        }
        setServiceStarted(true);
        mCapabilityPollingManager.startService();
        mAvailabilityFetchManager.startService();
    }

    /**
     * Stop the IMS service
     */
    @Override
    public synchronized void stop() {
        logger.debug("stop()");
        if (!isServiceStarted()) {
            // Already stopped
            return;
        }
        setServiceStarted(false);
        mCapabilityPollingManager.stopService();
        mAvailabilityFetchManager.stopService();
    }

    /**
     * Check the IMS service
     */
    @Override
    public void check() {
    }

    /**
     *ImsService error handling
     */
    @Override
    public void onForbiddenReceived(ImsModule.EabActions action, String reason) {
        mCapabilityPollingManager.onForbiddenReceived(action, reason);
        mAvailabilityFetchManager.onForbiddenReceived(action, reason);
    }

    @Override
    public void onNotProvisionedReceived() {
        mCapabilityPollingManager.onNotProvisionedReceived();
        mAvailabilityFetchManager.onNotProvisionedReceived();
    }

    /**
     * Request contact capabilities.
     *
     * @param contact
     *            Contact
     */
    public synchronized void requestContactCapabilities(String contact) {
        List<String> contactList = new ArrayList<String>();
        contactList.add(contact);
        requestContactsCapabilities(contactList);
    }

    /**
     * Request contact capability with a requestId (AOSP procedure)
     *
     * @param contact contact
     * @param requestId When device receives SIP response, presence statck shall send
     *                  a callback with requestId to client
     */
    public synchronized void requestContactCapabilities(String contact, int requestId) {
        List<String> contactList = new ArrayList<String>();
        contactList.add(contact);
        requestContactsCapabilities(contactList, requestId);
    }

    /**
     * Request capabilities for a list of contacts
     *
     * @param contactList
     *            List of contacts
     */
    public void requestContactsCapabilities(List<String> contactList) {
        logger.debug("requestContactsCapabilities, contactList: " + contactList);
        mCapabilityPollingManager.putSubscription(contactList,
                mCapabilityPollingManager.CAPABILITY_POLLING_USER_APPLICATION);
    }

    /**
     * Request capabilities for a list of contacts with a requestId (AOSP procedure)
     *
     * @param contactList contactList
     * @param requestId When device receives SIP response, presence statck shall send
     *                  a callback with requestId to client
     */
    public void requestContactsCapabilities(List<String> contactList, int requestId) {
        logger.debug("requestContactsCapabilities, contactList: " + contactList);
        mCapabilityPollingManager.setRequestId(requestId);
        mCapabilityPollingManager.putSubscription(contactList,
                mCapabilityPollingManager.CAPABILITY_POLLING_USER_APPLICATION);
    }

    /**
     * Request contact availability.
     *
     * @param contact
     *            Contact
     */
    public synchronized void requestContactAvailability(String contact) {
        logger.debug("requestContactAvailability, contact: " + contact);
        mAvailabilityFetchManager.putSubscription(contact);
    }

    /**
     * Request contact availability (AOSP procedure)
     *
     * @param contact Contact
     * @param requestId When device receives SIP response, presence statck shall send
     *                  a callback with requestId to client
     */
    public synchronized void requestContactAvailability(String contact, int requestId) {
        logger.debug("requestContactAvailability, contact: " + contact);
        mAvailabilityFetchManager.setRequestId(requestId);
        mAvailabilityFetchManager.putSubscription(contact);
    }

    /**
     * Receive a notification (anonymous fecth procedure)
     *
     * @param notify
     *            Received notify
     */
    public void receiveNotification(final SipRequest notify) {
        mCapabilityPollingManager.putNotification(notify);
        mAvailabilityFetchManager.putNotification(notify);
    }
}
