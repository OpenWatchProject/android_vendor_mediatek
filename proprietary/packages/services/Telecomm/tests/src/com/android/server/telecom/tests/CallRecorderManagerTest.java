/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2017. All rights reserved.
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
 * Copyright (C) 2013 The Android Open Source Project
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
 */

package com.android.server.telecom.tests;

import android.content.ComponentName;
import android.telecom.DisconnectCause;
import android.telecom.PhoneAccountHandle;
import android.test.suitebuilder.annotation.LargeTest;

import com.android.server.telecom.CallState;

import mediatek.telecom.MtkCall;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertTrue;

/**
 * M: Performs Telecom call recorder tests.
 */
@RunWith(JUnit4.class)
public class CallRecorderManagerTest extends TelecomSystemTest {
    static final int RECORD_REPEAT_INTERVAL = 100;  // milliseconds, only for stress testing
    static final int RECORD_INTERVAL = 1000;  // milliseconds

    @Override
    @Before
    public void setUp() throws Exception {
        super.setUp();
    }

    @Override
    @After
    public void tearDown() throws Exception {
        super.tearDown();
    }

    @LargeTest
    @Test
    public void testSingleOutgoingCall() throws Exception {
        IdPair ids = startAndMakeActiveOutgoingCall("650-555-1212",
                mPhoneAccountA0.getAccountHandle(), mConnectionServiceFixtureA);

        startVoiceRecord();
        verifyRecordProperty(ids, true);

        Thread.sleep(RECORD_INTERVAL);

        mConnectionServiceFixtureA.sendSetDisconnected(ids.mConnectionId, DisconnectCause.LOCAL);
        verifyRecordProperty(ids, false);
    }

    @LargeTest
    @Test
    public void testIncomingThenOutgoingCalls() throws Exception {
        IdPair incoming = startAndMakeActiveIncomingCall("650-555-2323",
                mPhoneAccountA0.getAccountHandle(), mConnectionServiceFixtureA);

        startVoiceRecord();
        verifyRecordProperty(incoming, true);

        Thread.sleep(RECORD_INTERVAL);

        mConnectionServiceFixtureA.sendSetOnHold(incoming.mConnectionId);
        verifyRecordProperty(incoming, false);
        IdPair outgoing = startAndMakeActiveOutgoingCall("650-555-1212",
                mPhoneAccountA0.getAccountHandle(), mConnectionServiceFixtureA);

        startVoiceRecord();
        verifyRecordProperty(outgoing, true);

        Thread.sleep(RECORD_INTERVAL);

        stopVoiceRecord();
        verifyRecordProperty(outgoing, false);

        mConnectionServiceFixtureA.sendSetDisconnected(incoming.mConnectionId,
                DisconnectCause.LOCAL);
        mConnectionServiceFixtureA.sendSetDisconnected(outgoing.mConnectionId,
                DisconnectCause.LOCAL);
    }

    @LargeTest
    @Test
    public void testForStressTesting() throws Exception {
        IdPair ids = startAndMakeActiveOutgoingCall("650-555-1212",
                mPhoneAccountA0.getAccountHandle(), mConnectionServiceFixtureA);

        for (int i = 0; i < 100; i++) {
            startVoiceRecord();
            Thread.sleep(RECORD_REPEAT_INTERVAL);

            stopVoiceRecord();
            Thread.sleep(RECORD_REPEAT_INTERVAL);
        }

        if (can(mInCallServiceFixtureX.getCall(ids.mCallId).getProperties(),
                MtkCall.MtkDetails.MTK_PROPERTY_VOICE_RECORDING)) {
            stopVoiceRecord();
            verifyRecordProperty(ids, false);
        } else {
            startVoiceRecord();
            verifyRecordProperty(ids, true);

            Thread.sleep(RECORD_INTERVAL);

            stopVoiceRecord();
            verifyRecordProperty(ids, false);
        }

        mConnectionServiceFixtureA.sendSetDisconnected(ids.mConnectionId, DisconnectCause.LOCAL);
    }

    private void startVoiceRecord() {
        com.android.server.telecom.Call recordingCall = null;
        for (com.android.server.telecom.Call call : mTelecomSystem.getCallsManager().getCalls()) {
            if (canVoiceRecord(call)) {
                recordingCall = call;
                break;
            }
        }

        assertTrue(recordingCall != null);

        // If not PstnComponentName, call will start record fail at the beginning.
        // So change to PstnComponentName temporarily.
        PhoneAccountHandle handle = recordingCall.getTargetPhoneAccount();
        recordingCall.setTargetPhoneAccount(new PhoneAccountHandle(
                new ComponentName(
                        "com.android.phone",
                         "com.android.services.telephony.TelephonyConnectionService"),
                "id dummy"));

        mTelecomSystem.getCallsManager().startVoiceRecord();
        recordingCall.setTargetPhoneAccount(handle);
    }

    private void stopVoiceRecord() {
        mTelecomSystem.getCallsManager().stopVoiceRecord();
    }

    private boolean can(int capabilities, int capability) {
        return (capabilities & capability) == capability;
    }

    private boolean canVoiceRecord(com.android.server.telecom.Call call) {
        if (call == null || call.getTargetPhoneAccount() == null) {
            return false;
         }

        if (call.getState() == CallState.ACTIVE && call.getParentCall() == null) {
            return true;
        }

        return false;
    }

    private void verifyRecordProperty(IdPair idPair, boolean expectHasRecord) {
        verifyRecordProperty(idPair.mCallId, expectHasRecord);
    }

    private void verifyRecordProperty(String callId, boolean expectHasRecord) {
        boolean hasRecord = can(
                    mInCallServiceFixtureX.getCall(callId).getProperties(),
                    MtkCall.MtkDetails.MTK_PROPERTY_VOICE_RECORDING);
        int loopCount = 0;

        while (expectHasRecord != hasRecord && loopCount < 3) {
            mInCallServiceFixtureX.waitForUpdate();
            hasRecord = can(
                    mInCallServiceFixtureX.getCall(callId).getProperties(),
                    MtkCall.MtkDetails.MTK_PROPERTY_VOICE_RECORDING);
            loopCount++;
        }

        assertEquals(hasRecord, expectHasRecord);
    }
}
