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

package com.android.server.telecom.components;

import android.app.admin.DevicePolicyManager;
import android.content.res.Resources;

import android.content.Context;
import android.content.Intent;
import android.net.Uri;
import android.os.UserHandle;
import android.os.UserManager;
import android.telecom.DefaultDialerManager;
/// M: Using mtk log instead of AOSP log.
// import android.telecom.Log;
import android.telecom.PhoneAccount;
import android.telecom.TelecomManager;
import android.telecom.VideoProfile;
import android.telephony.PhoneNumberUtils;
import android.text.TextUtils;

import com.android.server.telecom.CallIntentProcessor;
import com.android.server.telecom.PhoneNumberUtilsAdapter;
import com.android.server.telecom.PhoneNumberUtilsAdapterImpl;
import com.android.server.telecom.R;
import com.android.server.telecom.TelecomServiceImpl;
import com.android.server.telecom.TelecomSystem;
import com.android.server.telecom.TelephonyUtil;
import com.android.server.telecom.UserUtil;

/// M: Mediatek import.
import com.mediatek.server.telecom.Log;
import com.mediatek.server.telecom.MtkUtil;

import java.util.List;

// TODO: Needed for move to system service: import com.android.internal.R;

/**
 * Handles system CALL actions and forwards them to {@link CallIntentProcessor}.
 * Handles all three CALL action types: CALL, CALL_PRIVILEGED, and CALL_EMERGENCY.
 *
 * Pre-L, the only way apps were were allowed to make outgoing emergency calls was the
 * ACTION_CALL_PRIVILEGED action (which requires the system only CALL_PRIVILEGED permission).
 *
 * In L, any app that has the CALL_PRIVILEGED permission can continue to make outgoing emergency
 * calls via ACTION_CALL_PRIVILEGED.
 *
 * In addition, the default dialer (identified via
 * {@link android.telecom.TelecomManager#getDefaultDialerPackage()} will also be granted the
 * ability to make emergency outgoing calls using the CALL action. In order to do this, it must
 * use the {@link TelecomManager#placeCall(Uri, android.os.Bundle)} method to allow its package
 * name to be passed to {@link UserCallIntentProcessor}. Calling startActivity will continue to
 * work on all non-emergency numbers just like it did pre-L.
 */
public class UserCallIntentProcessor {

    private final Context mContext;
    private final UserHandle mUserHandle;

    /// M: Identifier for intent extra for sending an empty Flash message for
    // CDMA networks. Currently, just ignore this signal.
    private static final String EXTRA_SEND_EMPTY_FLASH = "com.android.phone.extra.SEND_EMPTY_FLASH";

    /// M: VoLTE invitation conference fake number.
    private static final String FAKE_CONFERENCE_NUMBER = "1234567890987654321";

    /// M: Add Log TAG string.
    private static final String TAG = UserCallIntentProcessor.class.getSimpleName();

    public UserCallIntentProcessor(Context context, UserHandle userHandle) {
        mContext = context;
        mUserHandle = userHandle;
    }

    /**
     * Processes intents sent to the activity.
     *
     * @param intent The intent.
     * @param callingPackageName The package name of the calling app.
     * @param canCallNonEmergency {@code true} if the caller is permitted to call non-emergency
     *                            numbers.
     * @param isLocalInvocation {@code true} if the caller is within the system service (i.e. the
     *                            caller is {@link com.android.server.telecom.TelecomServiceImpl})
     *                            and we can skip the re-broadcast of the intent to Telecom.
     *                            When {@code false}, we need to re-broadcast the intent to Telcom
     *                            to trampoline it to the system service where the Telecom
     *                            service resides.
     */
    public void processIntent(Intent intent, String callingPackageName,
            boolean canCallNonEmergency, boolean isLocalInvocation) {
        /// M: VoLTE invitation conference. @{
        if (MtkUtil.isConferenceInvitation(intent.getExtras())) {
            // Fix the issue that invitation conference with more than 5 members from call log.
            // Show toast to notify user if the invitation conference members is more than 5.
            // The reason why not handle it in TeleService is because there is some timing issues.
            // If user makes invitation conference less than 5 members first, and then invite
            // some members to the existed conference, TeleService will handle it.
            // If user makes merge conference which is more than 5 members,
            // TeleService will handle it.
            List<String> numbers = MtkUtil.getConferenceInvitationNumbers(intent.getExtras());
            if (numbers != null) {
                int size = MtkUtil.removeDuplicatedNumbers(numbers).size();
                if (size > MtkUtil.INVITATION_CONFERENCE_MEMBERS_MAX) {
                    Log.w(this, "[processIntent] Exceed invitation conference members count");
                    MtkUtil.showMaxInvitationMembersToast(mContext);
                    return;
                }
            }

            // replace the calling number with fake number, the telephony framework didn't need
            // real number.
            intent.setData(Uri.fromParts(PhoneAccount.SCHEME_TEL, FAKE_CONFERENCE_NUMBER, null));
        }
        /// @}

        // Ensure call intents are not processed on devices that are not capable of calling.
        if (!isVoiceCapable()) {
            return;
        }

        String action = intent.getAction();

        if (Intent.ACTION_CALL.equals(action) ||
                Intent.ACTION_CALL_PRIVILEGED.equals(action) ||
                Intent.ACTION_CALL_EMERGENCY.equals(action)) {
            processOutgoingCallIntent(intent, callingPackageName, canCallNonEmergency,
                    isLocalInvocation);
        }
    }

    private void processOutgoingCallIntent(Intent intent, String callingPackageName,
            boolean canCallNonEmergency, boolean isLocalInvocation) {
        /// M: Do nothing for CDMA empty flash at present @{
        /// TODO: the empty flash might be sent to CDMA framework directly via broadcast,
        /// otherwise it would bring confusion to Telecom.
        if (intent.getBooleanExtra(EXTRA_SEND_EMPTY_FLASH, false)) {
            Log.w(this, "Empty flash obtained from the call intent.");
            return;
        }
        /// M: @}

        Uri handle = intent.getData();
        String scheme = handle.getScheme();
        String uriString = handle.getSchemeSpecificPart();

        // Ensure sip URIs dialed using TEL scheme get converted to SIP scheme.
        if (PhoneAccount.SCHEME_TEL.equals(scheme) && PhoneNumberUtils.isUriNumber(uriString)) {
            /// M: Support IMS number contains "@", like "tel:test@server". @{
            handle = Uri.fromParts(PhoneNumberUtils.isUriNumber(uriString)
                    && !MtkUtil.isImsCallIntent(intent) ?
                    PhoneAccount.SCHEME_SIP : PhoneAccount.SCHEME_TEL, uriString, null);
            /// @}
        }

        /// M: Check potential ECC first, to save one binder call for non-potential ECC @{
        PhoneNumberUtilsAdapter adapter = new PhoneNumberUtilsAdapterImpl();
        String number = adapter.getNumberFromIntent(intent, mContext);
        boolean isPotentialEmergencyNumber =
                adapter.isPotentialLocalEmergencyNumber(mContext, number);
        boolean processAsEmergency = false;
        if (isPotentialEmergencyNumber) {
            processAsEmergency = TelephonyUtil.shouldProcessAsEmergency(mContext, handle);
        }
        /// @}

        // Check DISALLOW_OUTGOING_CALLS restriction. Note: We are skipping this check in a managed
        // profile user because this check can always be bypassed by copying and pasting the phone
        // number into the personal dialer.
        if (!UserUtil.isManagedProfile(mContext, mUserHandle)) {
            // Only emergency calls are allowed for users with the DISALLOW_OUTGOING_CALLS
            // restriction.
            /// M: use the cached emergency checked value @{
            if (!processAsEmergency) {
            /// @}
                final UserManager userManager = (UserManager) mContext.getSystemService(
                        Context.USER_SERVICE);
                if (userManager.hasBaseUserRestriction(UserManager.DISALLOW_OUTGOING_CALLS,
                        mUserHandle)) {
                    showErrorDialogForRestrictedOutgoingCall(mContext,
                            R.string.outgoing_call_not_allowed_user_restriction);
                    Log.w(this, "Rejecting non-emergency phone call due to DISALLOW_OUTGOING_CALLS "
                            + "restriction");
                    return;
                } else if (userManager.hasUserRestriction(UserManager.DISALLOW_OUTGOING_CALLS,
                        mUserHandle)) {
                    final DevicePolicyManager dpm =
                            mContext.getSystemService(DevicePolicyManager.class);
                    if (dpm == null) {
                        return;
                    }
                    final Intent adminSupportIntent = dpm.createAdminSupportIntent(
                            UserManager.DISALLOW_OUTGOING_CALLS);
                    if (adminSupportIntent != null) {
                        mContext.startActivity(adminSupportIntent);
                    }
                    return;
                }
            }
        }

        /// M: use the cached emergency checked value @{
        if (!canCallNonEmergency && !processAsEmergency) {
        /// @}
            showErrorDialogForRestrictedOutgoingCall(mContext,
                    R.string.outgoing_call_not_allowed_no_permission);
            Log.w(this, "Rejecting non-emergency phone call because "
                    + android.Manifest.permission.CALL_PHONE + " permission is not granted.");
            return;
        }

        int videoState = intent.getIntExtra(
                TelecomManager.EXTRA_START_CALL_WITH_VIDEO_STATE,
                VideoProfile.STATE_AUDIO_ONLY);
        Log.d(this, "processOutgoingCallIntent videoState = " + videoState);

        /// M: Convert ECC to voice call. @{
        if (VideoProfile.isVideo(videoState)
                && processAsEmergency) {
            Log.d(this, "Emergency call, converting video call to voice...");
            videoState = VideoProfile.STATE_AUDIO_ONLY;
            intent.putExtra(TelecomManager.EXTRA_START_CALL_WITH_VIDEO_STATE,
                    videoState);
        }
        /// @}

        intent.putExtra(CallIntentProcessor.KEY_IS_PRIVILEGED_DIALER,
                isDefaultOrSystemDialer(callingPackageName));

        // Save the user handle of current user before forwarding the intent to primary user.
        intent.putExtra(CallIntentProcessor.KEY_INITIATING_USER, mUserHandle);

        /// M: Do a early check before call added to CallsManager.
        // AOSP don't allow applications dial ECC number via Intent.ACTION_CALL.
        // But AOSP did this process in (@see NewOutgoingCallIntentBroadcaster#processIntent).
        // This is a little bit late that the call already added and would be disconnected soon.
        // There would be a timing issue for a call added-then-removed very fast. @{
        if (blockAndLaunchSystemDialer(intent, mContext, isPotentialEmergencyNumber)) {
            return;
        }
        /// @}

        sendIntentToDestination(intent, isLocalInvocation, callingPackageName);
    }

    private boolean isDefaultOrSystemDialer(String callingPackageName) {
        if (TextUtils.isEmpty(callingPackageName)) {
            return false;
        }

        final String defaultDialer = DefaultDialerManager.getDefaultDialerApplication(mContext,
                mUserHandle.getIdentifier());
        if (TextUtils.equals(defaultDialer, callingPackageName)) {
            return true;
        }

        final TelecomManager telecomManager =
                (TelecomManager) mContext.getSystemService(Context.TELECOM_SERVICE);
        return TextUtils.equals(telecomManager.getSystemDialerPackage(), callingPackageName);
    }

    /**
     * Returns whether the device is voice-capable (e.g. a phone vs a tablet).
     *
     * @return {@code True} if the device is voice-capable.
     */
    private boolean isVoiceCapable() {
        return mContext.getApplicationContext().getResources().getBoolean(
                com.android.internal.R.bool.config_voice_capable);
    }

    /**
     * Potentially trampolines the intent to Telecom via TelecomServiceImpl.
     * If the caller is local to the Telecom service, we send the intent to Telecom without
     * sending it through TelecomServiceImpl.
     */
    private boolean sendIntentToDestination(Intent intent, boolean isLocalInvocation,
            String callingPackage) {
        intent.putExtra(CallIntentProcessor.KEY_IS_INCOMING_CALL, false);
        intent.setFlags(Intent.FLAG_RECEIVER_FOREGROUND);
        if (isLocalInvocation) {
            // We are invoking this from TelecomServiceImpl, so TelecomSystem is available.  Don't
            // bother trampolining the intent, just sent it directly to the call intent processor.
            // TODO: We should not be using an intent here; this whole flows needs cleanup.
            Log.i(this, "sendIntentToDestination: send intent to Telecom directly.");
            synchronized (TelecomSystem.getInstance().getLock()) {
                TelecomSystem.getInstance().getCallIntentProcessor().processIntent(intent,
                        callingPackage);
            }
        } else {
            // We're calling from the UserCallActivity, so the TelecomSystem is not in the same
            // process; we need to trampoline to TelecomSystem in the system server process.
            Log.i(this, "sendIntentToDestination: trampoline to Telecom.");
            TelecomManager tm = (TelecomManager) mContext.getSystemService(Context.TELECOM_SERVICE);
            tm.handleCallIntent(intent);
        }
        return true;
    }

    private static void showErrorDialogForRestrictedOutgoingCall(Context context, int stringId) {
        final Intent intent = new Intent(context, ErrorDialogActivity.class);
        intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        intent.putExtra(ErrorDialogActivity.ERROR_MESSAGE_ID_EXTRA, stringId);
        context.startActivityAsUser(intent, UserHandle.CURRENT);
    }

    /**
     * M: Do a early check before call added to CallsManager.
     * AOSP don't allow applications dial ECC number via Intent.ACTION_CALL.
     * But AOSP did this process in (@see NewOutgoingCallIntentBroadcaster#processIntent).
     * This is a little bit late that the call already added and would be disconnected soon.
     * There would be a timing issue for a call added-then-removed very fast.
     * (refer to ALPS03127194)
     */
    private static boolean blockAndLaunchSystemDialer(Intent intent, Context context,
            boolean isPotentialEmergencyNumber) {
        final boolean isPrivilegedDialer = intent.getBooleanExtra(
                CallIntentProcessor.KEY_IS_PRIVILEGED_DIALER, false);
        final boolean isActionCall = TextUtils.equals(Intent.ACTION_CALL, intent.getAction());
        if (isActionCall && isPotentialEmergencyNumber && !isPrivilegedDialer) {
            launchSystemDialer(intent.getData(), context);
            return true;
        }
        return false;
    }

    /**
     * M: Copied from (@see NewOutgoingCallIntentBroadcaster#launchSystemDialer)
     */
    private static void launchSystemDialer(Uri handle, Context context) {
        Intent systemDialerIntent = new Intent();
        final Resources resources = context.getResources();
        systemDialerIntent.setClassName(
                TelecomServiceImpl.getSystemDialerPackage(context),
                resources.getString(R.string.dialer_default_class));
        systemDialerIntent.setAction(Intent.ACTION_DIAL);
        systemDialerIntent.setData(handle);
        systemDialerIntent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        Log.v(TAG, "calling startActivity for default dialer: %s", systemDialerIntent);
        context.startActivityAsUser(systemDialerIntent, UserHandle.CURRENT);
    }
}
