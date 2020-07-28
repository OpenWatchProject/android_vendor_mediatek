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
import android.os.Bundle;

import java.util.ArrayList;

public class MtkTelecomHelper {

    public static class MtkInCallServiceHelper {
        private static final String KEY_ACTION = "key_action";

        private static final String ACTION_HANGUP_ALL = "hangupAll";
        private static final String ACTION_EXPLICIT_CALL_TRANSFER = "explicitCallTransfer";
        private static final String ACTION_INVITE_CONFERENCE_PARTICIPANTS = "inviteConferenceParticipants";
        private static final String ACTION_HANGUP_HOLD = "hangupAllHold";
        private static final String ACTION_HANGUP_ACTIVE_AND_ANSWER_WAITING = "hangupActiveAndAnswerWaiting";
        private static final String ACTION_SET_SORTED_INCOMING_CALL_LIST = "setSortedIncomingCallList";
        private static final String ACTION_START_VOICE_RECORDING = "startVoiceRecording";
        private static final String ACTION_STOP_VOICE_RECORDING = "stopVoiceRecording";
        private static final String ACTION_BLIND_OR_ASSURED_ECT = "blindAssuredEct";
        /// Reject call with cause.
        private static final String ACTION_REJECT_CALL_WITH_CAUSE = "rejectcallwithcause";

        private static final String PARAM_STRING_CALL_ID = "param_string_call_id";
        private static final String PARAM_STRING_ARRAY_LIST_NUMBERS = "param_string_array_list_numbers";
        private static final String PARAM_STRING_ARRAY_CALL_IDS = "param_string_array_call_ids";
        private static final String PARAM_STRING_PHONE_NUMBER = "param_string_phone_number";
        private static final String PARAM_INT_TYPE = "param_int_type";

        @ProductApi
        public static Bundle buildParamsForHangupHold() {
            return obtainBuilder(ACTION_HANGUP_HOLD).build();
        }

        @ProductApi
        public static Bundle buildParamsForHangupAll() {
            return obtainBuilder(ACTION_HANGUP_ALL).build();
        }

        public static Bundle buildParamsForHangupActiveAndAnswerWaiting() {
            return obtainBuilder(ACTION_HANGUP_ACTIVE_AND_ANSWER_WAITING).build();
        }

        @ProductApi
        public static Bundle buildParamsForExplicitCallTransfer(String callId) {
            return obtainBuilder(ACTION_EXPLICIT_CALL_TRANSFER)
                    .putStringParam(PARAM_STRING_CALL_ID, callId)
                    .build();
        }

        @ProductApi
        public static Bundle buildParamsForInviteConferenceParticipants(
                String callId, ArrayList<String> numbers) {
            return obtainBuilder(ACTION_INVITE_CONFERENCE_PARTICIPANTS)
                    .putStringParam(PARAM_STRING_CALL_ID, callId)
                    .putStringArrayListParam(PARAM_STRING_ARRAY_LIST_NUMBERS, numbers)
                    .build();
        }

        @ProductApi
        public static Bundle buildParamsForSetSortedIncomingCallList(ArrayList<String> callIds) {
            return obtainBuilder(ACTION_SET_SORTED_INCOMING_CALL_LIST)
                    .putStringArrayListParam(PARAM_STRING_ARRAY_CALL_IDS, callIds)
                    .build();
        }

        @ProductApi
        public static Bundle buildParamsForBlindOrAssuredEct(String callId, String phoneNumber,
                                                             int type) {
            return obtainBuilder(ACTION_BLIND_OR_ASSURED_ECT)
                    .putStringParam(PARAM_STRING_CALL_ID, callId)
                    .putStringParam(PARAM_STRING_PHONE_NUMBER, phoneNumber)
                    .putIntParam(PARAM_INT_TYPE, type)
                    .build();
        }

        @ProductApi
        public static Bundle buildParamsForStartVoiceRecording() {
            return obtainBuilder(ACTION_START_VOICE_RECORDING).build();
        }

        @ProductApi
        public static Bundle buildParamsForStopVoiceRecording() {
            return obtainBuilder(ACTION_STOP_VOICE_RECORDING).build();
        }

        /// Reject call with cause. @{
        public static Bundle buildParamsForRejectCallWithCause(String callId, int cause) {
            return obtainBuilder(ACTION_REJECT_CALL_WITH_CAUSE)
                    .putStringParam(PARAM_STRING_CALL_ID, callId)
                    .putIntParam(PARAM_INT_TYPE, cause)
                    .build();
        }
        /// @}

        private static Builder obtainBuilder(String action) {
            return new Builder(action);
        }

        private static class Builder {
            Bundle mBundle;

            Builder(String action) {
                mBundle = new Bundle();
                mBundle.putString(KEY_ACTION, action);
            }

            Builder putStringParam(String key, String value) {
                mBundle.putString(key, value);
                return this;
            }

            Builder putStringArrayListParam(String key, ArrayList<String> value) {
                mBundle.putStringArrayList(key, value);
                return this;
            }

            Builder putIntParam(String key, int value) {
                mBundle.putInt(key, value);
                return this;
            }

            Bundle build() {
                return mBundle;
            }
        }
    }

    public static class MtkInCallAdapterHelper {

        public static void handleExtCommand(Bundle params, ICommandProcessor processor) {
            final String action = params.getString(MtkInCallServiceHelper.KEY_ACTION, "");
            switch (action) {
                case MtkInCallServiceHelper.ACTION_HANGUP_ALL:
                    processor.hangupAll();
                    break;
                case MtkInCallServiceHelper.ACTION_EXPLICIT_CALL_TRANSFER:
                    String callId = params.getString(MtkInCallServiceHelper.PARAM_STRING_CALL_ID);
                    processor.explicitCallTransfer(callId);
                    break;
                case MtkInCallServiceHelper.ACTION_INVITE_CONFERENCE_PARTICIPANTS:
                    callId = params.getString(MtkInCallServiceHelper.PARAM_STRING_CALL_ID);
                    ArrayList<String> numbers = params.getStringArrayList(
                            MtkInCallServiceHelper.PARAM_STRING_ARRAY_LIST_NUMBERS);
                    processor.inviteConferenceParticipants(callId, numbers);
                    break;
                case MtkInCallServiceHelper.ACTION_BLIND_OR_ASSURED_ECT:
                    callId = params.getString(MtkInCallServiceHelper.PARAM_STRING_CALL_ID);
                    String phoneNumber = params.getString(
                            MtkInCallServiceHelper.PARAM_STRING_PHONE_NUMBER);
                    int type = params.getInt(MtkInCallServiceHelper.PARAM_INT_TYPE);
                    processor.blindOrAssuredEct(callId, phoneNumber, type);
                    break;
                case MtkInCallServiceHelper.ACTION_HANGUP_ACTIVE_AND_ANSWER_WAITING:
                    processor.hangupActiveAndAnswerWaiting();
                    break;
                case MtkInCallServiceHelper.ACTION_HANGUP_HOLD:
                    processor.hangupHold();
                    break;
                case MtkInCallServiceHelper.ACTION_SET_SORTED_INCOMING_CALL_LIST:
                    ArrayList<String> callIds = params.getStringArrayList(
                            MtkInCallServiceHelper.PARAM_STRING_ARRAY_CALL_IDS);
                    processor.setSortedIncomingCallList(callIds);
                    break;
                case MtkInCallServiceHelper.ACTION_START_VOICE_RECORDING:
                    processor.startVoiceRecording();
                    break;
                case MtkInCallServiceHelper.ACTION_STOP_VOICE_RECORDING:
                    processor.stopVoiceRecording();
                    break;
                /// Reject call with cause.
                case MtkInCallServiceHelper.ACTION_REJECT_CALL_WITH_CAUSE:
                    callId = params.getString(MtkInCallServiceHelper.PARAM_STRING_CALL_ID);
                    int cause = params.getInt(MtkInCallServiceHelper.PARAM_INT_TYPE);
                    processor.rejectWithCause(callId, cause);
                    break;
                default:
                    // TODO: Log unknown action
            }
        }

        public static interface ICommandProcessor {
            void hangupAll();

            void explicitCallTransfer(String callId);

            void inviteConferenceParticipants(String callId, ArrayList<String> numbers);

            void blindOrAssuredEct(String callId, String phoneNumber, int type);

            void hangupActiveAndAnswerWaiting();

            void hangupHold();

            void setSortedIncomingCallList(ArrayList<String> callIds);

            void startVoiceRecording();

            void stopVoiceRecording();

            /// Reject call with cause.
            void rejectWithCause(String callId, int cause);
        }
    }
}
