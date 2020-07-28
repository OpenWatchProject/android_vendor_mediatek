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

import android.telephony.PhoneNumberUtils;

import java.util.IllegalFormatException;
import java.util.Iterator;
import java.util.LinkedHashMap;
import java.util.Locale;
import java.util.Map;

/**
 * ALPS02136977. Prints debug logs for telephony call-control modules.
 * Format:
 * 1. [category][Module][OP or Notify][Action][call-number][local-call-ID] Msg. String
 * 2. [category][Module][Dump][call-number][local-call-ID]-[name:value],[name:value]-Msg. String
 * @hide
 */
public class FormattedLog {

    private String mLogString;

    /**
     * Definition for the operation type.
     */
    public enum OpType {
        OPERATION, NOTIFY, DUMP;
    }
    private static String opTypeToString(OpType type) {
        switch (type) {
        case OPERATION:
            return "OP";
        case NOTIFY:
            return "Notify";
        case DUMP:
            return "Dump";
        default:
            return null;
        }
    }

    /**
     * Builder for FormattedLog.
     */
    public static class Builder {
        private String mCategory;

        private String mServiceName;

        private OpType mOpType;

        private String mAction;

        private String mCallNumber;

        private String mCallId;

        private LinkedHashMap<String, String> mStatusInfo =
            new LinkedHashMap<String, String>();

        private StringBuilder mExtraMessage = new StringBuilder();

        /**
         * Constructor for FormattedLog builder.
         */
        public Builder() {
        }

        /**
         * Set category, ex: "CC".
         *
         * @param category string.
         * @return builder
         * @hide
         */
        /// M: ALPS02344365. Guard setter writing @{
        public synchronized Builder setCategory(String category) {
            mCategory = category;
            return this;
        }

        /**
         * Set service name, ex: "Telephony".
         *
         * @param serviceName service name.
         * @return builder
         * @hide
         */
        public synchronized Builder setServiceName(String serviceName) {
            mServiceName = serviceName;
            return this;
        }

        /**
         * Set OP type.
         *
         * @param type OpType.
         * @return builder
         * @hide
         */
        public synchronized Builder setOpType(OpType type) {
            mOpType = type;
            return this;
        }

        /**
         * Set action name.
         *
         * @param action action name.
         * @return builder
         * @hide
         */
        public synchronized Builder setActionName(String action) {
            mAction = action;
            return this;
        }

        /**
         * Set call number.
         *
         * @param number call number.
         * @return builder
         * @hide
         */
        public synchronized Builder setCallNumber(String number) {
            if (number != null && !number.equals("conferenceCall")
                && !PhoneNumberUtils.isUriNumber(number)) {
                mCallNumber = PhoneNumberUtils.extractNetworkPortionAlt(number);
            } else {
                mCallNumber = number;
            }
            return this;
        }

        /**
         * Set call ID.
         *
         * @param id call ID.
         * @return builder
         * @hide
         */
        public synchronized Builder setCallId(String id) {
            /// @}
            mCallId = id;
            return this;
        }

        /**
         * Set status info.
         *
         * @param name status name.
         * @param value status value.
         * @return builder
         * @hide
         */
        public synchronized Builder setStatusInfo(String name, String value) {
            if (name != null && value != null && !name.isEmpty() && !value.isEmpty()) {
                mStatusInfo.put(name, value);
            }
            return this;
        }

        /**
         * reset status info.
         *
         * @param name status name.
         * @return builder
         * @hide
         */
        public synchronized Builder resetStatusInfo(String name) {
            if (name != null && !name.isEmpty()) {
                mStatusInfo.remove(name);
            }
            return this;
        }

        /**
         * set extra message.
         *
         * @param msg extra message.
         * @return builder
         * @hide
         */
        public synchronized Builder setExtraMessage(String msg) {
            if (msg != null) {
                mExtraMessage = new StringBuilder();
                mExtraMessage.append(msg);
            }
            return this;
        }

        /**
         * append extra message.
         *
         * @param msg extra message.
         * @return builder
         * @hide
         */
        public synchronized Builder appendExtraMessage(String msg) {
            if (msg != null) {
                mExtraMessage.append(msg);
            }
            return this;
        }

        /**
         * Build the unified debug log messages, for "OP" or "Notify".
         * Format: [category][Module][OP or Notify][Action][call-number][local-call-ID] Msg. String
         * @return FormattedLog
         * @hide
         */
        public synchronized FormattedLog buildDebugMsg() {
            // phone number might be empty.
            if (mCallNumber == null) {
                mCallNumber = "unknown";
            }

            return new FormattedLog("[Debug][%s][%s][%s][%s][%s][%s] %s",
                                    mCategory, mServiceName, opTypeToString(mOpType),
                                    mAction, mCallNumber, mCallId, mExtraMessage);
        }

        /**
         * Logs unified debug log messages, for "Dump".
         * format:
         * [category][Module][Dump][call-number][localCallID]-[name:value],[name:value]-Msg. String
         * @return FormattedLog
         * @hide
         */
        public synchronized FormattedLog buildDumpInfo() {
            StringBuilder statusInfo = new StringBuilder();
            Iterator<Map.Entry<String, String>> entryIterator = mStatusInfo.entrySet().iterator();
            while (entryIterator.hasNext()) {
                Map.Entry<String, String> entry = entryIterator.next();

                statusInfo.append("[");
                statusInfo.append(entry.getKey());
                statusInfo.append(":");
                statusInfo.append(entry.getValue());
                statusInfo.append("]");

                if (entryIterator.hasNext()) {
                    statusInfo.append(",");
                }
            }

            // phone number might be empty.
            if (mCallNumber == null) {
                mCallNumber = "unknown";
            }

            return new FormattedLog("[Debug][%s][%s][Dump][%s][%s]-%s-%s",
                                    mCategory, mServiceName, mCallNumber, mCallId, statusInfo, mExtraMessage);
        }
    }

    private FormattedLog(String format, Object... args) {
        try {
            mLogString = (args == null || args.length == 0) ? format
                : String.format(Locale.US, format, args);
        } catch (IllegalFormatException ife) {
            mLogString = format + " (An error occurred while formatting the message.)";
        }
    }

    @Override
    public String toString() {
        return mLogString;
    }
}
