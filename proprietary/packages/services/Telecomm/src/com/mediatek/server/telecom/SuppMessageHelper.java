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

package com.mediatek.server.telecom;

import android.os.Bundle;
import android.text.TextUtils;
import android.util.SparseIntArray;
import com.android.server.telecom.R;
import mediatek.telecom.MtkConnection;

/**
 * Show toast for supplementary service notification.
 */
public class SuppMessageHelper {
    private static final int SUPP_SERVICE_MO_CALL = 0;
    private static final int SUPP_SERVICE_MT_CALL = 1;
    private static final int CALL_NUMBER_TYPE_INTERNATIONAL = 0x91;

    /**
     * Show toast for supplementary service notification.
     * @param extras The bundle of supplementary service notification.
     */
    public void onSsNotification(Bundle extras) {
        int notiType =
                extras.getInt(MtkConnection.EXTRA_SS_NOTIFICATION_NOTITYPE);
        int type = extras.getInt(MtkConnection.EXTRA_SS_NOTIFICATION_TYPE);
        int code = extras.getInt(MtkConnection.EXTRA_SS_NOTIFICATION_CODE);
        String number =
                extras.getString(MtkConnection.EXTRA_SS_NOTIFICATION_NUMBER);
        int index = extras.getInt(MtkConnection.EXTRA_SS_NOTIFICATION_INDEX);

        StringBuilder sb = new StringBuilder();
        if (notiType == SUPP_SERVICE_MO_CALL) {
            sb.append(getSuppServiceMoString(code, index));
        } else if (notiType == SUPP_SERVICE_MT_CALL) {
            sb.append(getSuppServiceMtString(code, index));
            if (type == CALL_NUMBER_TYPE_INTERNATIONAL) {
                if (number != null && number.length() != 0) {
                    sb.append(" +").append(number);
                }
            }
        }
        if (sb.length() > 0) {
            MtkTelecomGlobals.getInstance().showToast(sb.toString());
        }
    }

    // MO code
    private static final int MO_CODE_UNCONDITIONAL_CF_ACTIVE = 0;
    private static final int MO_CODE_SOME_CF_ACTIVE = 1;
    private static final int MO_CODE_CALL_FORWARDED = 2;
    private static final int MO_CODE_CALL_IS_WAITING = 3;
    private static final int MO_CODE_CUG_CALL = 4;
    private static final int MO_CODE_OUTGOING_CALLS_BARRED = 5;
    private static final int MO_CODE_INCOMING_CALLS_BARRED = 6;
    private static final int MO_CODE_CLIR_SUPPRESSION_REJECTED = 7;
    private static final int MO_CODE_CALL_DEFLECTED = 8;
    private static final int MO_CODE_CALL_FORWARDED_TO = 9;

    private static final SparseIntArray sMoCodeResIdPairs = new SparseIntArray() {{
        put(MO_CODE_UNCONDITIONAL_CF_ACTIVE, R.string.mo_code_unconditional_cf_active);
        put(MO_CODE_SOME_CF_ACTIVE, R.string.mo_code_some_cf_active);
        put(MO_CODE_CALL_FORWARDED, R.string.mo_code_call_forwarded);
        put(MO_CODE_CALL_IS_WAITING, R.string.call_waiting_indication);
        put(MO_CODE_CUG_CALL, R.string.mo_code_cug_call);
        put(MO_CODE_OUTGOING_CALLS_BARRED, R.string.mo_code_outgoing_calls_barred);
        put(MO_CODE_INCOMING_CALLS_BARRED, R.string.mo_code_incoming_calls_barred);
        put(MO_CODE_CLIR_SUPPRESSION_REJECTED, R.string.mo_code_clir_suppression_rejected);
        put(MO_CODE_CALL_DEFLECTED, R.string.mo_code_call_deflected);
        put(MO_CODE_CALL_FORWARDED_TO, R.string.mo_code_call_forwarding);
    }};

    // MT code
    private static final int MT_CODE_FORWARDED_CALL = 0;
    private static final int MT_CODE_CUG_CALL = 1;
    private static final int MT_CODE_CALL_ON_HOLD = 2;
    private static final int MT_CODE_CALL_RETRIEVED = 3;
    private static final int MT_CODE_MULTI_PARTY_CALL = 4;
    private static final int MT_CODE_ON_HOLD_CALL_RELEASED = 5;
    private static final int MT_CODE_FORWARD_CHECK_RECEIVED = 6;
    private static final int MT_CODE_CALL_CONNECTING_ECT = 7;
    private static final int MT_CODE_CALL_CONNECTED_ECT = 8;
    private static final int MT_CODE_DEFLECTED_CALL = 9;
    private static final int MT_CODE_ADDITIONAL_CALL_FORWARDED = 10;
    private static final int MT_CODE_FORWARDED_CF = 11;
    private static final int MT_CODE_FORWARDED_CF_UNCOND = 12;
    private static final int MT_CODE_FORWARDED_CF_COND = 13;
    private static final int MT_CODE_FORWARDED_CF_BUSY = 14;
    private static final int MT_CODE_FORWARDED_CF_NO_REPLY = 15;
    private static final int MT_CODE_FORWARDED_CF_NOT_REACHABLE = 16;

    private static final SparseIntArray sMtCodeResIdPairs = new SparseIntArray() {{
        put(MT_CODE_FORWARDED_CALL, R.string.mt_code_forwarded_call);
        put(MT_CODE_CUG_CALL, R.string.mt_code_cug_call);
        put(MT_CODE_CALL_ON_HOLD, R.string.mt_code_call_on_hold);
        put(MT_CODE_CALL_RETRIEVED, R.string.mt_code_call_retrieved);
        put(MT_CODE_MULTI_PARTY_CALL, R.string.mt_code_multi_party_call);
        put(MT_CODE_ON_HOLD_CALL_RELEASED, R.string.mt_code_on_hold_call_released);
        put(MT_CODE_FORWARD_CHECK_RECEIVED, R.string.mt_code_forward_check_received);
        put(MT_CODE_CALL_CONNECTING_ECT, R.string.mt_code_call_connecting_ect);
        put(MT_CODE_CALL_CONNECTED_ECT, R.string.mt_code_call_connected_ect);
        put(MT_CODE_DEFLECTED_CALL, R.string.mt_code_deflected_call);
        put(MT_CODE_ADDITIONAL_CALL_FORWARDED, R.string.mt_code_additional_call_forwarded);
        put(MT_CODE_FORWARDED_CF, R.string.mt_code_forwarded_call);
        put(MT_CODE_FORWARDED_CF_UNCOND, R.string.mt_code_forwarded_call);
        put(MT_CODE_FORWARDED_CF_COND, R.string.mt_code_forwarded_call);
        put(MT_CODE_FORWARDED_CF_BUSY, R.string.mt_code_forwarded_call);
        put(MT_CODE_FORWARDED_CF_NO_REPLY, R.string.mt_code_forwarded_call);
        put(MT_CODE_FORWARDED_CF_NOT_REACHABLE, R.string.mt_code_forwarded_call);
    }};

    private static final SparseIntArray sMtCodeForwardedDetailResIdPairs = new SparseIntArray() {{
        put(MT_CODE_FORWARDED_CF, R.string.mt_code_forwarded_cf);
        put(MT_CODE_FORWARDED_CF_UNCOND, R.string.mt_code_forwarded_cf_uncond);
        put(MT_CODE_FORWARDED_CF_COND, R.string.mt_code_forwarded_cf_cond);
        put(MT_CODE_FORWARDED_CF_BUSY, R.string.mt_code_forwarded_cf_busy);
        put(MT_CODE_FORWARDED_CF_NO_REPLY, R.string.mt_code_forwarded_cf_no_reply);
        put(MT_CODE_FORWARDED_CF_NOT_REACHABLE, R.string.mt_code_forwarded_cf_not_reachable);
    }};

    private static String getSuppServiceMoString(int code, int index) {
        int resId = sMoCodeResIdPairs.get(code, R.string.incall_error_supp_service_unknown);
        StringBuilder sb = new StringBuilder();
        sb.append(MtkTelecomGlobals.getInstance().getContext().getString(resId));
        if (code == MO_CODE_CUG_CALL) {
            sb.append(" ").append(index);
        }
        return sb.toString();
    }

    private static String getSuppServiceMtString(int code, int index) {
        int resId = sMtCodeResIdPairs.get(code, R.string.incall_error_supp_service_unknown);
        StringBuilder sb = new StringBuilder();
        sb.append(MtkTelecomGlobals.getInstance().getContext().getString(resId));
        if (code == MT_CODE_CUG_CALL) {
            sb.append(" ").append(index);
        }
        if (sMtCodeForwardedDetailResIdPairs.get(code, -1) != -1) {
            int cfdResId = sMtCodeForwardedDetailResIdPairs.get(code);
            String cfdDetails = MtkTelecomGlobals.getInstance().getContext().getString(cfdResId);
            sb.append("(").append(cfdDetails).append(")");
        }
        return sb.toString();
    }
}
