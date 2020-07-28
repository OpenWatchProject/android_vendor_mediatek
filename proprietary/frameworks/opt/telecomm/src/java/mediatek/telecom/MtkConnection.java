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

import android.os.Bundle;
import android.telecom.Call;
import android.telecom.Connection;
import android.telecom.PhoneAccountHandle;

public class MtkConnection {

    private static final int CAPABILITY_BASE = 0x08000000; /* Maximum << 4*/
    /**
     * Legacy Explicit Call Transfer. 1A1H, transfer to connect the hold and active call.
     * @hide
     */
    public static final int CAPABILITY_CONSULTATIVE_ECT = CAPABILITY_BASE << 0;
    /**
     * @hide
     */
    public static final int CAPABILITY_INVITE_PARTICIPANTS = CAPABILITY_BASE << 1;
    /**
     * IMS call transfer support blind or assured ECT.
     * Transfer the Active call to a number, then quit self.
     * Blind transfer would quit self immediately.
     * Assured transfer would make sure the new connection created, then quit self.
     * @hide
     */
    public static final int CAPABILITY_BLIND_OR_ASSURED_ECT = CAPABILITY_BASE << 2;

    public static final int CAPABILITY_CAPABILITY_CALL_RECORDING = CAPABILITY_BASE << 3;

    /**
     * Indicate the call has capability to play video ringtone.
     * when remote user subscribes video ringtone service, the MO side could receive video ringtone
     * from network in dailing state.
     * @hide
     */
    public static final int CAPABILITY_VIDEO_RINGTONE = CAPABILITY_BASE << 4;

    private static final int PROPERTY_BASE = 1 << 15; /* Maximum << 16 */
    /**
     * @hide
     */
    public static final int PROPERTY_VOLTE = PROPERTY_BASE << 0;

    /**
     * The call which has this property is a CDMA call, should act the CDMA feature set.
     */
    public static final int PROPERTY_CDMA = PROPERTY_BASE << 1;

    /**
     * The call which has this property is in voice recording state.
     */
    public static final int PROPERTY_VOICE_RECORDING = PROPERTY_BASE << 2;

    public static final int PROPERTY_CONFERENCE_PARTICIPANT = PROPERTY_BASE << 3;

    public static String propertiesToString(int properties) {
        StringBuilder sb = new StringBuilder(Connection.propertiesToString(properties));
        sb.setLength(sb.length() - 1);
        sb.append(mtkPropertiesToStringInternal(properties, true));
        sb.append("]");
        return sb.toString();
    }

    public static String propertiesToStringShort(int properties) {
        StringBuilder sb = new StringBuilder(Connection.propertiesToStringShort(properties));
        sb.setLength(sb.length() - 1);
        sb.append(mtkPropertiesToStringInternal(properties, false));
        sb.append("]");
        return sb.toString();
    }

    public static String capabilitiesToString(int capabilities) {
        StringBuilder sb = new StringBuilder(Connection.capabilitiesToString(capabilities));
        sb.setLength(sb.length() - 1);
        sb.append(mtkCapabilitiesToStringInternal(capabilities, true));
        sb.append("]");
        return sb.toString();
    }

    public static String capabilitiesToStringShort(int capabilities) {
        StringBuilder sb = new StringBuilder(Connection.capabilitiesToStringShort(capabilities));
        sb.setLength(sb.length() - 1);
        sb.append(mtkCapabilitiesToStringInternal(capabilities, false));
        sb.append("]");
        return sb.toString();
    }

    private static String mtkPropertiesToStringInternal(int properties, boolean isLong) {
        StringBuilder sb = new StringBuilder();
        if (can(properties, PROPERTY_VOLTE)) {
            sb.append(isLong ? " M_PROPERTY_VOLTE" : " m_volte");
        }
        if (can(properties, PROPERTY_CDMA)) {
            sb.append(isLong ? " M_PROPERTY_CDMA" : " m_cdma");
        }
        if (can(properties, PROPERTY_VOICE_RECORDING)) {
            sb.append(isLong ? " M_PROPERTY_VOICE_RECORDING" : " m_rcrding");
        }
        if (can(properties, PROPERTY_CONFERENCE_PARTICIPANT)) {
            sb.append(isLong ? " M_PROPERTY_CONFERENCE_PARTICIPANT" : " m_conf_child");
        }
        return sb.toString();
    }

    private static String mtkCapabilitiesToStringInternal(int capabilities, boolean isLong) {
        StringBuilder sb = new StringBuilder();
        if (can(capabilities, CAPABILITY_BLIND_OR_ASSURED_ECT)) {
            sb.append(isLong ? " M_CAPABILITY_BLIND_OR_ASSURED_ECT" : " m_b|a_ect");
        }
        if (can(capabilities, CAPABILITY_CAPABILITY_CALL_RECORDING)) {
            sb.append(isLong ? " M_CAPABILITY_CAPABILITY_CALL_RECORDING" : " m_rcrd");
        }
        if (can(capabilities, CAPABILITY_CONSULTATIVE_ECT)) {
            sb.append(isLong ? " M_CAPABILITY_CONSULTATIVE_ECT" : " m_ect");
        }
        if (can(capabilities, CAPABILITY_INVITE_PARTICIPANTS)) {
            sb.append(isLong ? " M_CAPABILITY_INVITE_PARTICIPANTS" : " m_invite");
        }
        if (can(capabilities, CAPABILITY_VIDEO_RINGTONE)) {
            sb.append(isLong ? " M_CAPABILITY_VIDEO_RINGTONE" : " m_vt_tone");
        }
        return sb.toString();
    }

    public static boolean can(int capabilities, int capability) {
        return Connection.can(capabilities, capability);
    }

    /**
     * event string to notify Telecom that connection lost
     * Sent to Telecom via
     * {@link android.telecom.Connection#sendConnectionEvent(String, android.os.Bundle)}
     *
     * @hide
     */
    public static final String EVENT_CONNECTION_LOST =
            "mediatek.telecom.event.CONNECTION_LOST";

    /**
     * event string to notify Telecom that number updated.
     * Sent to Telecom via
     * {@link android.telecom.Connection#sendConnectionEvent(String, android.os.Bundle)}
     *
     * @hide
     */
    public static final String EVENT_NUMBER_UPDATED =
            "mediatek.telecom.event.NUMBER_UPDATED";

    /**
     * event string to notify Telecom that incoming info updated.
     * Sent to Telecom via
     * {@link android.telecom.Connection#sendConnectionEvent(String, android.os.Bundle)}
     *
     * @hide
     */
    public static final String EVENT_INCOMING_INFO_UPDATED =
            "mediatek.telecom.event.INCOMING_INFO_UPDATED";

    /**
     * event string to notify Telecom that phone account changed.
     * Sent to Telecom via
     * {@link android.telecom.Connection#sendConnectionEvent(String, android.os.Bundle)}
     * The new PhoneAccountHandle would be carried by {@link #EXTRA_PHONE_ACCOUNT_HANDLE}
     *
     * @hide
     */
    public static final String EVENT_PHONE_ACCOUNT_CHANGED =
            "mediatek.telecom.event.PHONE_ACCOUNT_CHANGED";

    /**
     * event string to notify Telecom that VT status updated.
     * Sent to Telecom via
     * {@link android.telecom.Connection#sendConnectionEvent(String, android.os.Bundle)}
     *
     * @hide
     */
    public static final String EVENT_3G_VT_STATUS_CHANGED =
            "mediatek.telecom.event.EVENT_3G_VT_STATUS_CHANGED";

    /**
     * event string to update volte call extra to Telecom
     * Sent to Telecom via
     * {@link android.telecom.Connection#sendConnectionEvent(String, android.os.Bundle)}
     *
     * @hide
     */
    public static final String EVENT_VOLTE_MARKED_AS_EMERGENCY =
            "mediatek.telecom.event.EVENT_VOLTE_MARKED_AS_EMERGENCY";

    /**
     * Event string to notify that the CS Fallback happened to the connection.
     * {@link android.telecom.Connection#sendConnectionEvent(String, android.os.Bundle)}
     */
    public static final String EVENT_CSFB =
            "mediatek.telecom.event.EVENT_CSFB";

    /**
     * Event string to notify that the SRVCC happened to the connection.
     * {@link android.telecom.Connection#sendConnectionEvent(String, android.os.Bundle)}
     */
    public static final String EVENT_SRVCC =
            "mediatek.telecom.event.EVENT_SRVCC";

    /**
     * Event string to notify that the RTT modify fail happened to the connection.
     * {@link android.telecom.Connection#sendConnectionEvent(String, android.os.Bundle)}
     */
    public static final String EVENT_RTT_UPDOWN_FAIL =
            "mediatek.telecom.event.EVENT_RTT_UPDOWN_FAIL";

    /**
     * Event string to notify that the emergency RTT call fails and modem will redial it.
     * {@link android.telecom.Connection#sendConnectionEvent(String, android.os.Bundle)}
     */
    public static final String EVENT_RTT_EMERGENCY_REDIAL =
            "mediatek.telecom.event.EVENT_RTT_EMERGENCY_REDIAL";

    /**
     * event string to notify Telecom user operation failed.
     * Sent to Telecom via
     * {@link android.telecom.Connection#sendConnectionEvent(String, android.os.Bundle)}
     * @hide
     */
    public static final String EVENT_OPERATION_FAILED =
            "mediatek.telecom.event.OPERATION_FAILED";

    /**
     * event string to notify Telecom call state change to alerting.
     * Sent to Telecom via
     * {@link android.telecom.Connection#sendConnectionEvent(String, android.os.Bundle)}
     * @hide
     */
    public static final String EVENT_CALL_ALERTING_NOTIFICATION =
            "mediatek.telecom.event.EVENT_CALL_ALERTING_NOTIFICATION";

     /**
     * event string to send SS notification to Telecom.
     * Sent to Telecom via
      * {@link android.telecom.Connection#sendConnectionEvent(String, android.os.Bundle)}
     * @hide
     */
    public static final String EVENT_SS_NOTIFICATION =
            "mediatek.telecom.event.SS_NOTIFICATION";
    public static final String EXTRA_SS_NOTIFICATION_NOTITYPE =
            "mediatek.telecom.extra.SS_NOTIFICATION_NOTITYPE";
    public static final String EXTRA_SS_NOTIFICATION_TYPE =
            "mediatek.telecom.extra.SS_NOTIFICATION_TYPE";
    public static final String EXTRA_SS_NOTIFICATION_CODE =
            "mediatek.telecom.extra.SS_NOTIFICATION_CODE";
    public static final String EXTRA_SS_NOTIFICATION_NUMBER =
            "mediatek.telecom.extra.SS_NOTIFICATION_NUMBER";
    public static final String EXTRA_SS_NOTIFICATION_INDEX =
            "mediatek.telecom.extra.SS_NOTIFICATION_INDEX";

    /**
     * The Bundle key for the {@link #EVENT_PHONE_ACCOUNT_CHANGED}.
     * The user can refer to the
     * {@link MtkConnection.ConnectionEventHelper
     * #buildParamsForPhoneAccountChanged(android.telecom.PhoneAccountHandle)}
     * for easier usage.
     */
    public static final String EXTRA_PHONE_ACCOUNT_HANDLE =
            "mediatek.telecom.extra.PHONE_ACCOUNT_HANDLE";

    public static final String EXTRA_FAILED_OPERATION =
            "mediatek.telecom.extra.FAILED_OPERATION";
    public static final String EXTRA_NEW_NUMBER =
            "mediatek.telecom.extra.NEW_NUMBER";
    public static final String EXTRA_UPDATED_INCOMING_INFO_TYPE =
            "mediatek.telecom.extra.UPDATED_INCOMING_INFO_TYPE";
    public static final String EXTRA_UPDATED_INCOMING_INFO_ALPHAID =
            "mediatek.telecom.extra.UPDATED_INCOMING_INFO_ALPHAID";
    public static final String EXTRA_UPDATED_INCOMING_INFO_CLI_VALIDITY =
            "mediatek.telecom.extra.UPDATED_INCOMING_INFO_CLI_VALIDITY";
    public static final String EXTRA_3G_VT_STATUS =
            "mediatek.telecom.extra.3G_VT_STATUS";

    public static class ConnectionEventHelper {
        public static Bundle buildParamsForPhoneAccountChanged(PhoneAccountHandle handle) {
            Bundle bundle = new Bundle();
            bundle.putParcelable(EXTRA_PHONE_ACCOUNT_HANDLE, handle);
            return bundle;
        }
        public static Bundle buildParamsForOperationFailed(int operation) {
            Bundle bundle = new Bundle();
            bundle.putInt(EXTRA_FAILED_OPERATION, operation);
            return bundle;
        }
        public static Bundle buildParamsForNumberUpdated(String number) {
            Bundle bundle = new Bundle();
            bundle.putString(EXTRA_NEW_NUMBER, number);
            return bundle;
        }
        public static Bundle buildParamsForIncomingInfoUpdated(
                int type, String alphaid, int cliValidity) {
            Bundle bundle = new Bundle();
            bundle.putInt(EXTRA_UPDATED_INCOMING_INFO_TYPE, type);
            bundle.putString(EXTRA_UPDATED_INCOMING_INFO_ALPHAID, alphaid);
            bundle.putInt(EXTRA_UPDATED_INCOMING_INFO_CLI_VALIDITY, cliValidity);
            return bundle;
        }
        public static Bundle buildParamsFor3GVtStatusChanged(int status) {
            Bundle bundle = new Bundle();
            bundle.putInt(EXTRA_3G_VT_STATUS, status);
            return bundle;
        }
        public static Bundle buildParamsForSsNotification(
                int notiType, int type, int code, String number, int index) {
            Bundle bundle = new Bundle();
            bundle.putInt(EXTRA_SS_NOTIFICATION_NOTITYPE, notiType);
            bundle.putInt(EXTRA_SS_NOTIFICATION_TYPE, type);
            bundle.putInt(EXTRA_SS_NOTIFICATION_CODE, code);
            bundle.putString(EXTRA_SS_NOTIFICATION_NUMBER, number);
            bundle.putInt(EXTRA_SS_NOTIFICATION_INDEX, index);
            return bundle;
        }
    }

    public static final String OPERATION_DISCONNECT_CALL =
            "mediatek.telecom.operation.DISCONNECT_CALL";
    public static final String OPERATION_ANSWER_CALL =
            "mediatek.telecom.operation.ANSWER_CALL";

    /**
     * @hide
     */
    public static class MtkVideoProvider {
        /**
         * session modify for Canceling upgrade
         *
         * @hide
         */
        private static final int MTK_SESSION_MODIFY_BASE = 200;

        /**
         * session modify for Canceling upgrade fail
         *
         * @hide
         */
        public static final int SESSION_MODIFY_CANCEL_UPGRADE_FAIL = MTK_SESSION_MODIFY_BASE + 0;

        /**
         * session modify for Canceling upgrade fail to execute downgrade
         *
         * @hide
         */
        public static final int SESSION_MODIFY_CANCEL_UPGRADE_FAIL_AUTO_DOWNGRADE =
                MTK_SESSION_MODIFY_BASE + 1;

        /**
         * session modify for Canceling upgrade fail because of reject upgrade
         *
         * @hide
         */
        public static final int SESSION_MODIFY_CANCEL_UPGRADE_FAIL_REMOTE_REJECT_UPGRADE =
                MTK_SESSION_MODIFY_BASE + 2;

        /**
         * M: Camera operation crash. This occurs after a set of operation of camera.
         * This event means MAL has been crash and unrecoverable. This call should be
         * downgrade and disable the upgrade icon.
         * The {@link android.telecom.InCallService} can use this as a cue that the
         * camera is not available and won't recover any more during current call lifecycle.
         * Note: this has different meaning from {@link #SESSION_EVENT_CAMERA_FAILURE}
         * @hide
         */
        public static final int SESSION_EVENT_ERROR_CAMERA_CRASHED = 8003;
    }

    /**
     * Extended class for {@link Connection.RttModifyStatus}.
     * Provides constants to represent the results of responses to session modify requests sent via
     * {@link Call#sendRttRequest()}
     */
    public static final class MtkRttModifyStatus {
        private static final int MTK_SESSION_STATUS_CODE_BASE = 100;
        /**
         * Indicates the RTT session was terminated by the downgrade request of remote.
         * AOSP didn't provide the downgrade request APIs, and it's not that easy to add
         * a new API for it. So reuse the {@link Connection.Listener#sendRttInitiationFailure(int)}
         * API to carry the information that the remote side sent downgrade request which lead
         * to the terminating of current RTT session.
         */
        public static final int SESSION_DOWNGRADED_BY_REMOTE = MTK_SESSION_STATUS_CODE_BASE + 0;
    }
}
