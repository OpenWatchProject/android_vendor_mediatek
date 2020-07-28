package com.mediatek.server.telecom.tests;

import android.content.ComponentName;
import android.os.Bundle;
import android.os.Process;
import android.telecom.Call;
import android.telecom.DisconnectCause;
import android.telecom.Log;
import android.telecom.ParcelableCall;
import android.telecom.PhoneAccount;
import android.telecom.PhoneAccountHandle;
import android.telecom.VideoProfile;
import android.test.suitebuilder.annotation.LargeTest;
import android.test.suitebuilder.annotation.SmallTest;

import com.android.server.telecom.tests.TelecomSystemTest;

import mediatek.telecom.MtkPhoneAccount;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNotEquals;
import static org.junit.Assert.assertNotNull;
import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.eq;
import static org.mockito.Mockito.doNothing;
import static org.mockito.Mockito.doReturn;
import static org.mockito.Mockito.never;
import static org.mockito.Mockito.timeout;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.when;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;


@RunWith(JUnit4.class)
public class MtkCallsManagerTest extends TelecomSystemTest {

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
    public void testAnswerCallWhenDiffPhoneAccountHaveNonEccCalls() throws Exception {
        // Make 1A1H1W in PhoneAccountA0
        IdPair outgoingHeldByPhoneAccountA0 = startAndMakeActiveOutgoingCall("650-555-1212",
                mPhoneAccountA0.getAccountHandle(), mConnectionServiceFixtureA);
        IdPair outgoingActiveByPhoneAccountA0 = startAndMakeActiveOutgoingCall("650-555-1213",
                mPhoneAccountA0.getAccountHandle(), mConnectionServiceFixtureA);

        mConnectionServiceFixtureA.sendSetOnHold(outgoingHeldByPhoneAccountA0.mConnectionId);
        assertEquals(Call.STATE_HOLDING,
                mInCallServiceFixtureX.getCall(outgoingHeldByPhoneAccountA0.mCallId).getState());
        assertEquals(Call.STATE_HOLDING,
                mInCallServiceFixtureY.getCall(outgoingHeldByPhoneAccountA0.mCallId).getState());

        IdPair incomingByPhoneAccountA0 = startIncomingPhoneCall("650-555-1214",
                mPhoneAccountA0.getAccountHandle(), mConnectionServiceFixtureA);

        assertEquals(Call.STATE_RINGING,
                mInCallServiceFixtureX.getCall(incomingByPhoneAccountA0.mCallId).getState());
        assertEquals(Call.STATE_RINGING,
                mInCallServiceFixtureY.getCall(incomingByPhoneAccountA0.mCallId).getState());

        // Make 1 ringing call in PhoneAccountA1
        IdPair incomingByPhoneAccountA1 = startIncomingPhoneCall("650-555-1215",
                mPhoneAccountA1.getAccountHandle(), mConnectionServiceFixtureA);

        assertEquals(Call.STATE_RINGING,
                mInCallServiceFixtureX.getCall(incomingByPhoneAccountA1.mCallId).getState());
        assertEquals(Call.STATE_RINGING,
                mInCallServiceFixtureY.getCall(incomingByPhoneAccountA1.mCallId).getState());

        // InCallUI answer ringing call in PhoneAccountA1
        mInCallServiceFixtureX.mInCallAdapter
                .answerCall(incomingByPhoneAccountA1.mCallId, VideoProfile.STATE_AUDIO_ONLY);

        // Verify CallsManagerEx.disconnectCallInDiffPhoneAccountAndAnswerRinging
        verify(mConnectionServiceFixtureA.getTestDouble(), timeout(TEST_TIMEOUT))
                .disconnect(eq(outgoingHeldByPhoneAccountA0.mConnectionId), any());
        verify(mConnectionServiceFixtureA.getTestDouble(), timeout(TEST_TIMEOUT))
                .disconnect(eq(outgoingActiveByPhoneAccountA0.mConnectionId), any());
        verify(mConnectionServiceFixtureA.getTestDouble(), timeout(TEST_TIMEOUT))
                .disconnect(eq(incomingByPhoneAccountA0.mConnectionId), any());

        mConnectionServiceFixtureA.sendSetDisconnected(outgoingHeldByPhoneAccountA0.mConnectionId,
                DisconnectCause.LOCAL);
        assertEquals(Call.STATE_DISCONNECTED,
                mInCallServiceFixtureX.getCall(outgoingHeldByPhoneAccountA0.mCallId).getState());
        assertEquals(Call.STATE_DISCONNECTED,
                mInCallServiceFixtureY.getCall(outgoingHeldByPhoneAccountA0.mCallId).getState());

        mConnectionServiceFixtureA.sendSetDisconnected(incomingByPhoneAccountA0.mConnectionId,
                DisconnectCause.LOCAL);
        assertEquals(Call.STATE_DISCONNECTED,
                mInCallServiceFixtureX.getCall(incomingByPhoneAccountA0.mCallId).getState());
        assertEquals(Call.STATE_DISCONNECTED,
                mInCallServiceFixtureY.getCall(incomingByPhoneAccountA0.mCallId).getState());

        // Answer 1 ringing call in PhoneAccountA1 must waiting active call in PhoneAccountA0
        // disconnected.
        verify(mConnectionServiceFixtureA.getTestDouble(), never())
                .answer(eq(incomingByPhoneAccountA1.mConnectionId), any());
        mConnectionServiceFixtureA.sendSetDisconnected(
                outgoingActiveByPhoneAccountA0.mConnectionId, DisconnectCause.LOCAL);
        assertEquals(Call.STATE_DISCONNECTED,
                mInCallServiceFixtureX.getCall(outgoingActiveByPhoneAccountA0.mCallId).getState());
        assertEquals(Call.STATE_DISCONNECTED,
                mInCallServiceFixtureY.getCall(outgoingActiveByPhoneAccountA0.mCallId).getState());
        verify(mConnectionServiceFixtureA.getTestDouble(), timeout(TEST_TIMEOUT))
                .answer(eq(incomingByPhoneAccountA1.mConnectionId), any());

        mConnectionServiceFixtureA.sendSetDisconnected(incomingByPhoneAccountA1.mConnectionId,
                DisconnectCause.LOCAL);
    }

    @LargeTest
    @Test
    public void testAnswerCallWhenDiffPhoneAccountHasEccCall() throws Exception {
        // Make 1 emergency call in PhoneAccountA0
        IdPair outgoingEccByPhoneAccountA0 = startAndMakeDialingEmergencyCall("650-555-1212",
                mPhoneAccountE0.getAccountHandle(), mConnectionServiceFixtureA);

        // Make 1 ringing call in PhoneAccountA1
        IdPair incomingByPhoneAccountA1 = startIncomingPhoneCall("650-555-1213",
                mPhoneAccountA1.getAccountHandle(), mConnectionServiceFixtureA);

        assertEquals(Call.STATE_RINGING,
                mInCallServiceFixtureX.getCall(incomingByPhoneAccountA1.mCallId).getState());
        assertEquals(Call.STATE_RINGING,
                mInCallServiceFixtureY.getCall(incomingByPhoneAccountA1.mCallId).getState());

        // InCallUI answer ringing call in PhoneAccountA1
        mInCallServiceFixtureX.mInCallAdapter
                .answerCall(incomingByPhoneAccountA1.mCallId, VideoProfile.STATE_AUDIO_ONLY);

        verify(mConnectionServiceFixtureA.getTestDouble(), never())
                .disconnect(eq(outgoingEccByPhoneAccountA0.mConnectionId), any());
        assertEquals(Call.STATE_DIALING,
                mInCallServiceFixtureX.getCall(outgoingEccByPhoneAccountA0.mCallId).getState());

        mConnectionServiceFixtureA.sendSetDisconnected(incomingByPhoneAccountA1.mConnectionId,
                DisconnectCause.LOCAL);
        mConnectionServiceFixtureA.sendSetDisconnected(outgoingEccByPhoneAccountA0.mConnectionId,
                DisconnectCause.LOCAL);
    }

    @SmallTest
    @Test
    public void testDisallowOutgoingCallsDuringConference() throws Exception {
        int newCarrierCapabilities =
                MtkPhoneAccount.CARRIER_CAPABILITY_DISABLE_MO_CALL_DURING_CONFERENCE;
        Bundle extras = new Bundle();
        extras.putInt(
                MtkPhoneAccount.EXTRA_PHONE_ACCOUNT_CARRIER_CAPABILITIES, newCarrierCapabilities);

        // Re-register mPhoneAccountA0 with pre-defined extras we need
        PhoneAccount newPhoneAccount =
            PhoneAccount.builder(mPhoneAccountA0.getAccountHandle(), mPhoneAccountA0.getLabel())
            .addSupportedUriScheme("tel")
            .setCapabilities(mPhoneAccountA0.getCapabilities())
            .setExtras(extras)
            .build();
        mTelecomSystem.getPhoneAccountRegistrar().registerPhoneAccount(newPhoneAccount);
        PhoneAccount temPhoneAccount =
                mTelecomSystem.getPhoneAccountRegistrar().getPhoneAccountOfCurrentUser(
                        mPhoneAccountA0.getAccountHandle());
        assertEquals(newPhoneAccount, temPhoneAccount);
        assertEquals(temPhoneAccount.getExtras(), extras);

        // Make a conference call and then add another outgoing call in the same PhoneAccount
        ParcelableCall conferenceCall = makeConferenceCall();
        // ConnectionServiceFixture will not set PhoneAccount for FakeConference, update it here.
        for (com.android.server.telecom.Call call : mTelecomSystem.getCallsManager().getCalls()) {
            if (call.isConference()) {
                call.setTargetPhoneAccount(newPhoneAccount.getAccountHandle());
            }
        }
        int callCount = mTelecomSystem.getCallsManager().getCalls().size();
        // Make another MO call
        startOutgoingPhoneCallWaitForBroadcaster("650-555-1214", newPhoneAccount.getAccountHandle(),
                mConnectionServiceFixtureA, Process.myUserHandle(), VideoProfile.STATE_AUDIO_ONLY, false);
        // The new MO call should fail
        assertEquals(callCount, mTelecomSystem.getCallsManager().getCalls().size());
        for (com.android.server.telecom.Call call : mTelecomSystem.getCallsManager().getCalls()) {
            assertNotEquals("650-555-1214", call.getHandle());
        }

        // Change disallow outgoing call during conference to false, and try make MO call again.
        newCarrierCapabilities &=
                ~MtkPhoneAccount.CARRIER_CAPABILITY_DISABLE_MO_CALL_DURING_CONFERENCE;
        extras.putInt(
                MtkPhoneAccount.EXTRA_PHONE_ACCOUNT_CARRIER_CAPABILITIES, newCarrierCapabilities);
        // Re-register mPhoneAccountA0 with pre-defined extras we need
        newPhoneAccount =
                PhoneAccount.builder(mPhoneAccountA0.getAccountHandle(), mPhoneAccountA0.getLabel())
                .addSupportedUriScheme("tel")
                .setCapabilities(mPhoneAccountA0.getCapabilities())
                .setExtras(extras)
                .build();
        mTelecomSystem.getPhoneAccountRegistrar().registerPhoneAccount(newPhoneAccount);
        // Make another MO call
        startOutgoingPhoneCallWaitForBroadcaster("650-555-1215", newPhoneAccount.getAccountHandle(),
                mConnectionServiceFixtureA, Process.myUserHandle(), VideoProfile.STATE_AUDIO_ONLY, false);
        // The new MO call should success
        com.android.server.telecom.Call newMoCall = null;
        for (com.android.server.telecom.Call call : mTelecomSystem.getCallsManager().getCalls()) {
            if (call.getTargetPhoneAccount().equals(newPhoneAccount.getAccountHandle()) &&
                    call.getHandle() != null && call.getHandle().toString().contains("650-555-1215")) {
                newMoCall = call;
                break;
            }
        }
        assertEquals(callCount + 1, mTelecomSystem.getCallsManager().getCalls().size());
        assertNotNull(newMoCall);
    }
}
