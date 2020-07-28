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

package com.mediatek.op12.telephony.selfactivation;

public class SaRadioEnabledCause {
    private int mRadioEnabledCause = CAUSE_NONE;

    // Flags to record the radio enable cause.
    static final int CAUSE_NONE = 0;
    static final int CAUSE_NORMAL_CALL = 1;
    static final int CAUSE_EMERGENCY_CALL = 2;
    static final int CAUSE_ADD_DATA_SERVICE = 3;
    static final int CAUSE_SIM_CHANGED = 4;

    static final int MIN_CAUSE = CAUSE_NONE;
    static final int MAX_CAUSE = CAUSE_SIM_CHANGED;

    public SaRadioEnabledCause setCause(int cause) {
        enforceValidCause(cause);
        mRadioEnabledCause = cause;
        return this;
    }

    public SaRadioEnabledCause addCause(int cause) {
        enforceValidCause(cause);
        mRadioEnabledCause |= 1 << cause;
        return this;
    }

    public SaRadioEnabledCause removeCause(int cause) {
        enforceValidCause(cause);
        mRadioEnabledCause &= ~(1 << cause);
        return this;
    }

    public boolean hasCause(int cause) {
        enforceValidCause(cause);
        return ((mRadioEnabledCause & (1 << cause)) != 0);
    }

    private void enforceValidCause(int cause) {
        if (cause < MIN_CAUSE || cause > MAX_CAUSE) {
            throw new IllegalArgumentException("Invalid radio enabled cause: " + cause);
        }
    }

    public int[] getRadioEnabledCauses() {
        return enumerateBits(mRadioEnabledCause);
    }

    private int[] enumerateBits(long val) {
        int size = Long.bitCount(val);
        int[] result = new int[size];
        int index = 0;
        int resource = 0;
        while (val > 0) {
            if ((val & 1) == 1) result[index++] = resource;
            val = val >> 1;
            resource++;
        }
        return result;
    }

    @Override
    public String toString() {
        if (mRadioEnabledCause == CAUSE_NONE) {
            return "RadioEnabledCause: CAUSE_NONE";
        }
        int[] types = getRadioEnabledCauses();
        String causes = (types.length > 0 ? "RadioEnabledCause: " : "");
        for (int i = 0; i < types.length; ) {
            switch (types[i]) {
                case CAUSE_NORMAL_CALL:    causes += "NORMAL_CALL"; break;
                case CAUSE_EMERGENCY_CALL:    causes += "EMERGENCY_CALL"; break;
                case CAUSE_ADD_DATA_SERVICE:    causes += "ADD_DATA_SERVICE"; break;
                case CAUSE_SIM_CHANGED:    causes += "SIM_CHANGED"; break;
            }
            if (++i < types.length) causes += "&";
        }
        return causes;
    }
}
