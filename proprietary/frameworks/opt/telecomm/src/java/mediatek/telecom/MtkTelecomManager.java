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

package mediatek.telecom;

import android.annotation.ProductApi;
import android.content.Context;
import android.content.Intent;
import android.os.PersistableBundle;
import android.os.RemoteException;
import android.os.ServiceManager;
import android.telecom.PhoneAccount;
import android.telecom.PhoneAccountHandle;
import android.telecom.TelecomManager;
import android.telephony.CarrierConfigManager;
import android.telephony.TelephonyManager;
import android.util.Log;
import com.mediatek.internal.telecom.IMtkTelecomService;
import com.mediatek.telephony.MtkTelephonyManagerEx;

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

import mediatek.telephony.MtkCarrierConfigManager;


public class MtkTelecomManager {

    private static final String TAG = MtkTelecomManager.class.getSimpleName();

    public static final String MTK_TELECOM_SERVICE_NAME = "mtk_telecom";

    private static final MtkTelecomManager sInstance = new MtkTelecomManager();

    /// M: CC: Error message due to VoLTE SS checking @{
    /**
     * Here defines a special disconnect reason to distinguish that the disconnected call is
     * a VoLTE SS request without data connection open. (Telephony -> Telecomm)
     * see android.telecom.DisconnectCause.mDisconnectReason
     * @hide
     */
    public static final String DISCONNECT_REASON_VOLTE_SS_DATA_OFF =
            "disconnect.reason.volte.ss.data.off";
    /// @}

    /**
     * The connection event uploaded from Telecom to InCallUI via
     * {@see InCallService@onConnectionEvent}
     * Indicates the call recording state changed.
     * @hide
     */
    public static final String MTK_CONNECTION_EVENT_CALL_RECORDING_STATE_CHANGED =
            "mediatek.telecom.CONNECTION_EVENT_CALL_RECORDING_STATE_CHANGED";
    /**
     * The key of the Extra for Call recording state changing event via
     * {@see InCallService@onConnectionEvent}
     * @hide
     */
    public static final String EXTRA_CALL_RECORDING_STATE =
            "mediatek.telecom.CALL_RECORDING_STATE";
    /** @hide */
    public static final int CALL_RECORDING_STATE_IDLE = 0;
    /** @hide */
    public static final int CALL_RECORDING_STATE_ACTIVE = 1;

    /// M: ALPS04024049, fix issue show save record toast failed when call end. @{
    /** @hide */
    public static final int CALL_RECORDING_EVENT_SHOW_TOAST = 0;
    /// @}

    /**
     * [STK notify]STK wants to know the InCallScreen state.
     * broadcast the InCallActivity state change.
     * @hide
     */
    public static final String ACTION_INCALL_SCREEN_STATE_CHANGED =
            "mediatek.telecom.action.INCALL_SCREEN_STATE_CHANGED";

    /**
     * [STK notify]STK wants to know the InCallScreen show/disappear state.
     * broadcast the InCallActivity show or not.
     * this is a boolean extra.
     * if the Activity shows, the value will be true.
     * if the Activity disappears, the value will be false.
     * @hide
     */
    public static final String EXTRA_INCALL_SCREEN_SHOW =
            "mediatek.telecom.extra.INCALL_SCREEN_SHOW";

    public static final String ACTION_CALL_RECORD = "mediatek.telecom.action.CALL_RECORD";

    //-------------For VoLTE normal call switch to ECC------------------
    /**
     * Here defines a special key to distinguish the call is marked as Ecc by NW.
     * Its value is should be Boolean. see IConnectionServiceAdapter.updateExtras()
     * @deprecated
     * @hide
     */
    public static final String EXTRA_VOLTE_MARKED_AS_EMERGENCY =
            "mediatek.telecom.extra.VOLTE_MARKED_AS_EMERGENCY";

    //-------------For VoLTE PAU field------------------
    /**
     * Here defines a special key to pass "pau" information of the call.
     * Its value should be String. see IConnectionServiceAdapter.updateExtras()
     * @deprecated
     * @hide
     */
    public static final String EXTRA_VOLTE_PAU = "mediatek.telecom.extra.VOLTE_PAU";

    //-------------For VoLTE Conference Call
    /**
     * Optional extra for {@link android.content.Intent#ACTION_CALL} and
     * {@link android.content.Intent#ACTION_CALL_PRIVILEGED} containing a phone
     * number {@link ArrayList} that used to launch the volte conference call.
     * The phone number in the list may be normal phone number, sip phone
     * address or IMS call phone number. This extra takes effect only when the
     * {@link #EXTRA_START_VOLTE_CONFERENCE} is true.
     * @hide
     */
    public static final String EXTRA_VOLTE_CONFERENCE_NUMBERS =
            "mediatek.telecom.extra.VOLTE_CONFERENCE_NUMBERS";

    /**
     * Optional extra for {@link android.content.Intent#ACTION_CALL} and
     * {@link android.content.Intent#ACTION_CALL_PRIVILEGED} containing an
     * boolean value that determines if it should launch a volte conference
     * call.
     * @hide
     */
    public static final String EXTRA_START_VOLTE_CONFERENCE =
            "mediatek.telecom.extra.EXTRA_START_VOLTE_CONFERENCE";

    /**
     * extra Boolean-key info in {@link android.telecom.TelecomManager#EXTRA_INCOMING_CALL_EXTRAS}
     * to indicate the incoming call is VoLTE conference invite.
     * @hide
     */
    public static final String EXTRA_INCOMING_VOLTE_CONFERENCE =
            "mediatek.telecom.extra.EXTRA_INCOMING_VOLTE_CONFERENCE";

    /// M: CC: GWSD
    /**
     * extra Boolean-key info in {@link android.telecom.TelecomManager#EXTRA_INCOMING_CALL_EXTRAS}
     * to indicate the incoming call is GWSD
     * @hide
     */
    public static final String EXTRA_INCOMING_GWSD =
            "mediatek.telecom.extra.EXTRA_INCOMING_GWSD";


    /**
     * Extras boolean-key info in {@link android.telecom.TelecomManager#EXTRA_INCOMING_CALL_EXTRAS}
     * to indicate the incoming call is a callback for previous RTT Emergency call.
     * Depends on operator, the call might automatically send request to upgrade to RTT mode
     * as soon as it was answered.
     */
    public static final String EXTRA_IS_RTT_EMERGENCY_CALLBACK =
            "mediatek.telecom.extra.IS_RTT_EMERGENCY_CALLBACK";

    /**
     * [Digits] Extras String-key info carrying the number of a virtual line to the IMS framework.
     * Since for the Digits multi-line feature, multiple virtual lines would mapping to the same
     * SIM, the IMS framework need this extra to distinguish lines.
     */
    public static final String EXTRA_VIRTUAL_LINE_NUMBER =
            "mediatek.telecom.extra.VIRTUAL_LINE_NUMBER";

    /**
     * [Digits] Extras Parcelable-key info carrying the PhoneAccountHandle of current call
     * to the IMS framework.
     * Since for the Digits multi-line feature, multiple virtual lines would mapping to the same
     * SIM, the IMS framework need this extra to distinguish lines.
     */
    public static final String EXTRA_CALLING_VIA_PHONE_ACCOUNT_HANDLE =
            "mediatek.telecom.extra.CALLING_VIA_PHONE_ACCOUNT_HANDLE";

    /**
     * Let the network to determine which ECT type(BLIND or ASSURED) to apply.
     */
    public static final int ECT_TYPE_AUTO = 0;
    /**
     * Try to apply a blind ECT. (ignored now)
     */
    public static final int ECT_TYPE_BLIND = 1;
    /**
     * Try to apply a assured ECT. (ignored now)
     */
    public static final int ECT_TYPE_ASSURED = 2;

    /**
     * RTT local/remote status/capability changed event.
     */
    public static final String EVENT_RTT_SUPPORT_CHANGED =
            "mediatek.telecom.event.RTT_SUPPORT_CHANGED";

    /**
     * Boolean extra indicates the local RTT capability.
     */
    public static final String EXTRA_RTT_SUPPORT_LOCAL =
            "mediatek.telecom.extra.RTT_SUPPORT_LOCAL";

    /**
     * Boolean extra indicates the remote RTT capability.
     */
    public static final String EXTRA_RTT_SUPPORT_REMOTE =
            "mediatek.telecom.extra.RTT_SUPPORT_REMOTE";

    /**
     * Boolean extra indicates the RTT local status.
     */
    public static final String EXTRA_RTT_STATUS_LOCAL =
            "mediatek.telecom.extra.RTT_STATUS_LOCAL";

    /**
     * Boolean extra indicates the RTT remote status.
     */
    public static final String EXTRA_RTT_STATUS_REMOTE =
            "mediatek.telecom.extra.RTT_STATUS_REMOTE";

    /**
     * Boolean extra indicates the call is or used to be video call.
     */
    public static final String EXTRA_WAS_VIDEO_CALL =
            "mediatek.telecom.extra.WAS_VIDEO_CALL";

    /**
     * Only for undemote to receive ACTION_NEW_OUTGOING_CALL intent.
     */
    public static final String ACTION_NEW_OUTGOING_CALL_UNDEMOTE =
            "mediatek.intent.action.NEW_OUTGOING_CALL";

    /**
     * @hide
     */
    private MtkTelecomManager() {
    }

    private IMtkTelecomService getTelecomService() {
        return IMtkTelecomService.Stub.asInterface(
                ServiceManager.getService(MTK_TELECOM_SERVICE_NAME));
    }

    private boolean isServiceConnected() {
        boolean isConnected = getTelecomService() != null;
        if (!isConnected) {
            Log.w(TAG, "Mtk Telecom Service not found.");
        }
        return isConnected;
    }

    /**
     * @hide
     */
    public boolean isInVideoCall(Context context) {
        try {
            if (isServiceConnected()) {
                return getTelecomService().isInVideoCall((context.getOpPackageName()));
            }
        } catch (RemoteException e) {
            Log.e(TAG, "RemoteException calling isInVideoCall().", e);
        }
        return false;
    }

    /**
     * @hide
     */
    public List<PhoneAccount> getAllPhoneAccountsIncludingVirtual() {
        try {
            if (isServiceConnected()) {
                return getTelecomService().getAllPhoneAccountsIncludingVirtual();
            }
        } catch (RemoteException e) {
            Log.e(TAG, "RemoteException calling getAllPhoneAccountsIncludeVirtual()", e);
        }
        return Collections.EMPTY_LIST;
    }

    /**
     * @hide
     */
    public List<PhoneAccountHandle> getAllPhoneAccountHandlesIncludingVirtual() {
        try {
            if (isServiceConnected()) {
                return getTelecomService().getAllPhoneAccountHandlesIncludingVirtual();
            }
        } catch (RemoteException e) {
            Log.e(TAG, "RemoteException calling getAllPhoneAccountHandlesIncludingVirtual()", e);
        }
        return Collections.EMPTY_LIST;
    }

    /**
     * @hide
     */
    public static MtkTelecomManager getInstance() {
        return sInstance;
    }

    /**
     * @hide
     */
    public boolean isInVolteCall(Context context) {
        try {
            if (isServiceConnected()) {
                return getTelecomService().isInVolteCall(context.getOpPackageName());
            }
        } catch (RemoteException e) {
            Log.e(TAG, "RemoteException calling isInVolteCall().", e);
        }
        return false;
    }

    /**
     * The difference of this method with the same method in TelecomManager is that this method
     * take connecting as in call too, by this checking dialer can show "back to call" item even
     * CallState is in connecting status.
     *
     * Returns whether there is an ongoing phone call (can be in connecting, dialing, ringing, active
     * or holding states) originating from either a manager or self-managed {@link ConnectionService}.
     * <p>
     *
     * @return {@code true} if there is an ongoing call in either a managed or self-managed
     *      {@link ConnectionService}, {@code false} otherwise.
     */
    public boolean isInCall(Context context) {
        try {
            if (isServiceConnected()) {
                return getTelecomService().isInCall(context.getOpPackageName());
            }
        } catch (RemoteException e) {
            Log.e(TAG, "RemoteException calling isInCall().", e);
        }
        return false;
    }

    /**
     * Create a special Intent used to place conference invitation.
     *
     * Returns an Intent to place conference invitation call, and will put PhoneAccountHandle
     * into Intent if there is only one PhoneAccount that is able to make conference invitation.
     * @param context Context used to access TelephonyManager and CarrieConfigManager
     *
     * @return An intent with special extra values for conference invitation.
     */
    @ProductApi
    public static Intent createConferenceInvitationIntent(Context context) {
        Intent intent = new Intent(Intent.ACTION_CALL);
        intent.putExtra(EXTRA_START_VOLTE_CONFERENCE, true);

        final TelecomManager telecomManager =
                (TelecomManager)context.getSystemService(Context.TELECOM_SERVICE);
        final TelephonyManager telephonyManager =
                (TelephonyManager)context.getSystemService(Context.TELEPHONY_SERVICE);
        final CarrierConfigManager configManager =
                (CarrierConfigManager)context.getSystemService(Context.CARRIER_CONFIG_SERVICE);

        // Iterate the account list and find all VoLTE conference enabled accounts.
        List<PhoneAccount> phoneAccounts = telecomManager.getAllPhoneAccounts();
        List<PhoneAccount> volteConferenceAccounts =
                new ArrayList<PhoneAccount>(telephonyManager.getPhoneCount());
        for (PhoneAccount phoneAccount : phoneAccounts) {
            int subId = telephonyManager.getSubIdForPhoneAccount(phoneAccount);
            boolean votleEnabled = MtkTelephonyManagerEx.getDefault().isVolteEnabled(subId);
            boolean volteConferenceEnabled = false;
            if (votleEnabled) {
                PersistableBundle bundle = configManager.getConfigForSubId(subId);
                if (bundle != null) {
                    volteConferenceEnabled = bundle.getBoolean(
                            MtkCarrierConfigManager.MTK_KEY_VOLTE_CONFERENCE_ENHANCED_ENABLE_BOOL);
                }
            }
            if (volteConferenceEnabled && votleEnabled) {
                volteConferenceAccounts.add(phoneAccount);
            }
        }

        // If only one account is able to place conference invitation, should set it to intent.
        // Avoid running Telecom phone account selection flow, which may let user choose the
        // account that not able to place conference invitation.
        if (volteConferenceAccounts.size() == 1) {
            intent.putExtra(TelecomManager.EXTRA_PHONE_ACCOUNT_HANDLE,
                    volteConferenceAccounts.get(0).getAccountHandle());
        }
        return intent;
    }
}
