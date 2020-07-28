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
import android.telecom.Call;

public class MtkCall {
    public static class MtkDetails {
        /**
         * {@see telecom#Call#Details} Next CAPABILITY value: 0x01000000
         */
        private static final int MTK_CAPABILITY_BASE = 0x01000000; /* Maximum << 7 */
        /**
         * Indicates the call is capable for recording.
         * @hide
         */
        public static final int MTK_CAPABILITY_CALL_RECORDING = MTK_CAPABILITY_BASE << 0;
        /**
         * Indicates the call is capable for legacy Explicit Call Transfer.
         * @hide
         */
        public static final int MTK_CAPABILITY_CONSULTATIVE_ECT = MTK_CAPABILITY_BASE << 1;
        /**
         * Indicates the call is capable for adding multiple participants to a conference at
         * at a time.
         * @hide
         */
        public static final int MTK_CAPABILITY_INVITE_PARTICIPANTS = MTK_CAPABILITY_BASE << 2;
        /**
         * IMS call transfer support blind or assured ECT.
         * Transfer the Active call to a number, then quit self.
         * Blind transfer would quit self immediately.
         * Assured transfer would make sure the new connection created, then quit self.
         * @hide
         */
        public static final int MTK_CAPABILITY_BLIND_OR_ASSURED_ECT = MTK_CAPABILITY_BASE << 3;

      /**
         * Indicate the call has capability to play video ringtone.
         * when remote user subscribes video ringtone service, the MO side could receive video
         * ringtone from network in dailing state.
         * @hide
         */
        public static final int MTK_CAPABILITY_VIDEO_RINGTONE = MTK_CAPABILITY_BASE << 4;

        private static final int MTK_PROPERTY_BASE = 0x00010000; /* Maximum << 15 */
        /**
         * Indicates the call is VoLTE call.
         * @hide
         */
        public static final int MTK_PROPERTY_VOLTE = MTK_PROPERTY_BASE << 0;
        public static final int MTK_PROPERTY_CDMA = MTK_PROPERTY_BASE << 1;
        public static final int MTK_PROPERTY_VOICE_RECORDING = MTK_PROPERTY_BASE << 2;

        /// M: For GTT (Global Text Telephony) @{
        /**
         * Local GTT (Global Text Telephony) property
         * @hide
         */
        public static final int PROPERTY_GTT_LOCAL = MTK_PROPERTY_BASE << 3;
        /// @}

        /// M: For GTT (Global Text Telephony) @{
        /**
         * Remote GTT (Global Text Telephony) property
         * @hide
         */
        public static final int PROPERTY_GTT_REMOTE = MTK_PROPERTY_BASE << 4;
        /// @}

        /**
         * Indicates this call is a conference participant(child).
         */
        public static final int PROPERTY_CONFERENCE_PARTICIPANT = MTK_PROPERTY_BASE << 5;

        public static final int PROPERTY_RTT_SUPPORT_REMOTE = MTK_PROPERTY_BASE << 6;
        public static final int PROPERTY_RTT_SUPPORT_LOCAL = MTK_PROPERTY_BASE << 7;

        @ProductApi
        public static String capabilitiesToStringShort(int capabilities) {
            StringBuilder builder = new StringBuilder();
            builder.append("[Capabilities:");
            if (can(capabilities, Call.Details.CAPABILITY_HOLD)) {
                builder.append(" hld");
            }
            if (can(capabilities, Call.Details.CAPABILITY_SUPPORT_HOLD)) {
                builder.append(" sup_hld");
            }
            if (can(capabilities, Call.Details.CAPABILITY_MERGE_CONFERENCE)) {
                builder.append(" mrg_cnf");
            }
            if (can(capabilities, Call.Details.CAPABILITY_SWAP_CONFERENCE)) {
                builder.append(" swp_cnf");
            }
            if (can(capabilities, Call.Details.CAPABILITY_RESPOND_VIA_TEXT)) {
                builder.append(" rsp_v_txt");
            }
            if (can(capabilities, Call.Details.CAPABILITY_MUTE)) {
                builder.append(" mut");
            }
            if (can(capabilities, Call.Details.CAPABILITY_MANAGE_CONFERENCE)) {
                builder.append(" mng_cnf");
            }
            if (can(capabilities, Call.Details.CAPABILITY_SUPPORTS_VT_LOCAL_RX)) {
                builder.append(" VTlrx");
            }
            if (can(capabilities, Call.Details.CAPABILITY_SUPPORTS_VT_LOCAL_TX)) {
                builder.append(" VTltx");
            }
            if (can(capabilities, Call.Details.CAPABILITY_SUPPORTS_VT_REMOTE_RX)) {
                builder.append(" VTrrx");
            }
            if (can(capabilities, Call.Details.CAPABILITY_SUPPORTS_VT_REMOTE_TX)) {
                builder.append(" VTrtx");
            }
            if (can(capabilities, Call.Details.CAPABILITY_CANNOT_DOWNGRADE_VIDEO_TO_AUDIO)) {
                builder.append(" !v2a");
            }
            if (can(capabilities, Call.Details.CAPABILITY_SPEED_UP_MT_AUDIO)) {
                builder.append(" spd_aud");
            }
            if (can(capabilities, Call.Details.CAPABILITY_CAN_UPGRADE_TO_VIDEO)) {
                builder.append(" a2v");
            }
            if (can(capabilities, Call.Details.CAPABILITY_CAN_PAUSE_VIDEO)) {
                builder.append(" paus_VT");
            }
            if (can(capabilities, Call.Details.CAPABILITY_CAN_PULL_CALL)) {
                builder.append(" pull");
            }
            if (can(capabilities, MTK_CAPABILITY_CALL_RECORDING)) {
                builder.append(" m_rcrd");
            }
            if (can(capabilities, MTK_CAPABILITY_CONSULTATIVE_ECT)) {
                builder.append(" m_ect");
            }
            if (can(capabilities, MTK_CAPABILITY_INVITE_PARTICIPANTS)) {
                builder.append(" m_invite");
            }
            if (can(capabilities, MTK_CAPABILITY_BLIND_OR_ASSURED_ECT)) {
                builder.append(" m_b|a_ect");
            }
            if (can(capabilities, MTK_CAPABILITY_VIDEO_RINGTONE)) {
                builder.append(" m_vt_tone");
            }
            builder.append("]");
            return builder.toString();
        }

        @ProductApi
        public static String propertiesToStringShort(int properties) {
            StringBuilder builder = new StringBuilder();
            builder.append("[Properties:");
            if (hasProperty(properties, Call.Details.PROPERTY_CONFERENCE)) {
                builder.append(" cnf");
            }
            if (hasProperty(properties, Call.Details.PROPERTY_GENERIC_CONFERENCE)) {
                builder.append(" gen_cnf");
            }
            if (hasProperty(properties, Call.Details.PROPERTY_WIFI)) {
                builder.append(" wifi");
            }
            if (hasProperty(properties, Call.Details.PROPERTY_HIGH_DEF_AUDIO)) {
                builder.append(" HD");
            }
            if (hasProperty(properties, Call.Details.PROPERTY_EMERGENCY_CALLBACK_MODE)) {
                builder.append(" ecbm");
            }
            if (hasProperty(properties, Call.Details.PROPERTY_IS_EXTERNAL_CALL)) {
                builder.append(" xtrnl");
            }
            if (hasProperty(properties, Call.Details.PROPERTY_HAS_CDMA_VOICE_PRIVACY)) {
                builder.append(" priv");
            }
            if (hasProperty(properties, MTK_PROPERTY_CDMA)) {
                builder.append(" m_cdma");
            }
            if (hasProperty(properties, MTK_PROPERTY_VOICE_RECORDING)) {
                builder.append(" m_rcrding");
            }
            if (hasProperty(properties, MTK_PROPERTY_VOLTE)) {
                builder.append(" m_volte");
            }
            if (hasProperty(properties, PROPERTY_GTT_LOCAL)) {
                builder.append(" m_gtt_l");
            }
            if (hasProperty(properties, PROPERTY_GTT_REMOTE)) {
                builder.append(" m_gtt_r");
            }
            if (hasProperty(properties, PROPERTY_CONFERENCE_PARTICIPANT)) {
                builder.append(" m_cnf_chld");
            }
            if (hasProperty(properties, PROPERTY_RTT_SUPPORT_LOCAL)) {
                builder.append(" m_rtt_l");
            }
            if (hasProperty(properties, PROPERTY_RTT_SUPPORT_REMOTE)) {
                builder.append(" m_rtt_r");
            }
            builder.append("]");
            return builder.toString();
        }

        public static String deltaPropertiesToStringShort(int previousProp, int newProp) {
            int xorProperties = previousProp ^ newProp;
            int addedProperties = newProp & xorProperties;
            int removedProperties = previousProp & xorProperties;
            StringBuilder sb = new StringBuilder();
            sb.append("Delta Properties Added: ").append(propertiesToStringShort(addedProperties))
                    .append(", Removed: ").append(propertiesToStringShort(removedProperties));
            return sb.toString();
        }

        public static String deltaCapabilitiesToStringShort(int previousCap, int newCap) {
            int xorProperties = previousCap ^ newCap;
            int addedProperties = newCap & xorProperties;
            int removedProperties = previousCap & xorProperties;
            StringBuilder sb = new StringBuilder();
            sb.append("Delta Properties Added: ").append(capabilitiesToStringShort(addedProperties))
                    .append(", Removed: ").append(capabilitiesToStringShort(removedProperties));
            return sb.toString();
        }

        /**
         * Whether the supplied capabilities  supports the specified capability.
         *
         * @param capabilities A bit field of capabilities.
         * @param capability The capability to check capabilities for.
         * @return Whether the specified capability is supported.
         */
        public static boolean can(int capabilities, int capability) {
            return Call.Details.can(capabilities, capability);
        }

        /**
         * Whether the supplied properties includes the specified property.
         *
         * @param properties A bit field of properties.
         * @param property The property to check properties for.
         * @return Whether the specified property is supported.
         */
        public static boolean hasProperty(int properties, int property) {
            return Call.Details.hasProperty(properties, property);
        }
    }
}
