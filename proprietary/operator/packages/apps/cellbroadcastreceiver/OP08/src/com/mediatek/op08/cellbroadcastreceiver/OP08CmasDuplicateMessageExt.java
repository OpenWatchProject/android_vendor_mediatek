/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2012. All rights reserved.
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

package com.mediatek.op08.cellbroadcastreceiver;

import android.content.Context;
import android.database.Cursor;
import android.os.Handler;
import android.telephony.CellBroadcastMessage;
import android.util.Log;

import com.mediatek.cellbroadcastreceiver.CellBroadcastContentProvider;
import com.mediatek.cellbroadcastreceiver.Comparer;
import com.mediatek.cmas.ext.DefaultCmasDuplicateMessageExt;
import com.mediatek.cmas.ext.ICmasDuplicateMessageExt;


public class OP08CmasDuplicateMessageExt extends DefaultCmasDuplicateMessageExt {
    private static final String TAG = "CellBroadcastReceiver/Op08CmasDuplicateMessageExt";
    private boolean langChanged = false;
    Comparer compareList = new Comparer();

    public OP08CmasDuplicateMessageExt(Context context) {
        super(context);
    }

    @Override
    public void setCmasLocaleChange(boolean status) {
        Log.i("@M_" + TAG, "set locale change:" + status);
        langChanged = status;
        compareList.clear();
    }

    /**
     * Handle duplicate message after power on or airplane offline off
     */
    @Override
    public int handleDuplicateMessage(CellBroadcastContentProvider provider,
            CellBroadcastMessage cbm) {
        Cursor c = provider.getAllCellBroadcastCursor();
        Comparer oldList = Comparer.createFromCursor(c);

        if (oldList == null) {
            Log.d("@M_" + TAG, "oldList null so NEW CMAS");
            return ICmasDuplicateMessageExt.NEW_CMAS_PROCESS;
        }

        if (oldList.isIdentifyMsgOver12H(cbm)) {
            Log.d("@M_" + TAG, "OVER_12_HOURS_IDENTIFY_MSG");
            return ICmasDuplicateMessageExt.OVER_12_HOURS_IDENTIFY_MSG;
        }

        if (oldList.isReadForDuplicateMessage(cbm)) {

            if (langChanged && compareList.add(cbm)) {
                Log.d("@M_" + TAG, "[isReadForDuplicateMessage]:language Changed so PRESENT CMAS");
                return ICmasDuplicateMessageExt.PRESENT_CMAS_PROCESS;
            }
            /**
             * if the message is a duplicate message and the message read
             * before. So, for T-mobile, this message should Discard
             */
            Log.d("@M_" + TAG, "[isReadForDuplicateMessage]:Discard CMAS");
            return ICmasDuplicateMessageExt.DISCARD_CMAS_PROCESS;
        }

        /**
         * if the message is not duplicate or is a duplicate message but not
         * read before, the message should present, and the message which not
         * duplicate must add to the list
         */

        if (oldList.add(cbm)) {
            Log.d("@M_" + TAG, "the message is not duplicate so NEW CMAS");
            return ICmasDuplicateMessageExt.NEW_CMAS_PROCESS;
        }

        Log.d("@M_" + TAG, "the message is duplicate So Discard CMAS");
        return ICmasDuplicateMessageExt.DISCARD_CMAS_PROCESS;
    }

    /**
     * Send Delete Message with Delay: 12Hour Identify Message
     */
    @Override
    public void sendDelayedMsgToDelete(Handler mTimeoutHandler, CellBroadcastMessage cbm,
                                            final int MSG_OVER_12HOURS) {
        Log.d("@M_" + TAG, "Will remove message after " + Comparer.REGARD_AS_NEW_TIMEOUT);
        boolean result = mTimeoutHandler.sendMessageDelayed(mTimeoutHandler.obtainMessage(
                MSG_OVER_12HOURS, cbm.getServiceCategory(), cbm.getSerialNumber())
                                                    , Comparer.REGARD_AS_NEW_TIMEOUT); //12 hours
        Log.d("@M_" + TAG, "Handler result: " + result);
    }
}
