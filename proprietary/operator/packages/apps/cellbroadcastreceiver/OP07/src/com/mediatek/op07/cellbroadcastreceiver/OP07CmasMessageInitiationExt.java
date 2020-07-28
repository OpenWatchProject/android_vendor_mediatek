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

package com.mediatek.op07.cellbroadcastreceiver;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.telephony.TelephonyManager;
import android.text.SpannableString;
import android.text.method.LinkMovementMethod;
import android.util.Log;
import android.view.WindowManager;
import android.widget.TextView;

import com.mediatek.cellbroadcastreceiver.CMASAlertFullWindow;
import com.mediatek.cmas.ext.DefaultCmasMessageInitiationExt;



public class OP07CmasMessageInitiationExt extends DefaultCmasMessageInitiationExt {
    private static final String TAG = "OP07CmasMessageInitiationExt";
    private Context mContext;
    private WindowManager mWindowManager;


    public OP07CmasMessageInitiationExt(Context context) {
        super(context);
        mContext = context;
    }

    /**
     * Allow to answer Incoming Call on CMAS alert screen
     */
    @Override
    public WindowManager.LayoutParams updateViewLayoutParams(WindowManager.LayoutParams lp) {
        Log.d(TAG, "updateViewLpToTouchable:");
        int callState = TelephonyManager.CALL_STATE_IDLE;
        TelephonyManager mTm = (TelephonyManager) mContext.getSystemService(
                                                    mContext.TELEPHONY_SERVICE);
        if (mTm != null) {
            callState = mTm.getCallState();
            Log.d(TAG, "call callState = " + callState);
        }
        if (callState != TelephonyManager.CALL_STATE_IDLE) {
            Log.d(TAG, "call callState = " + callState);
            Log.i(TAG, "phone is activate, and allow operation on phone before CMAS alert dismiss");

            lp.flags = lp.flags & (~(WindowManager.LayoutParams.FLAG_SPLIT_TOUCH));
            lp.flags = lp.flags | WindowManager.LayoutParams.FLAG_NOT_FOCUSABLE
                | WindowManager.LayoutParams.FLAG_NOT_TOUCH_MODAL;
          }
        return lp;
    }

    @Override
    public WindowManager.LayoutParams updateViewLpToTouchable(WindowManager.LayoutParams lp,
                                        boolean bTouchable) {
        Log.i(TAG, "updateViewLpToTouchable:: bTouchable = " + bTouchable);
        if (bTouchable) {
            lp.flags = lp.flags & (~(WindowManager.LayoutParams.FLAG_SPLIT_TOUCH));
            lp.flags = lp.flags | WindowManager.LayoutParams.FLAG_NOT_FOCUSABLE
                | WindowManager.LayoutParams.FLAG_NOT_TOUCH_MODAL;
        } else {
            lp.flags = lp.flags & (~(WindowManager.LayoutParams.FLAG_NOT_FOCUSABLE))
                    & (~(WindowManager.LayoutParams.FLAG_NOT_TOUCH_MODAL));
            lp.flags = lp.flags | WindowManager.LayoutParams.FLAG_SPLIT_TOUCH;
        }

        return lp;
    }

    private BroadcastReceiver mBr = new BroadcastReceiver() {

        @Override
        public void onReceive(Context context, Intent intent) {
            // TODO Auto-generated method stub
            Log.d(TAG, "onReceiver: action = " + intent.getAction());
            if (intent.getAction().equals(TelephonyManager.ACTION_PHONE_STATE_CHANGED)) {
                TelephonyManager tm = (TelephonyManager) context.getSystemService(
                                                    context.TELEPHONY_SERVICE);
                int callState = tm.getCallState();
                Log.d(TAG, "onReceiver: callState = " + callState);
                CMASAlertFullWindow updateWindow = CMASAlertFullWindow.getInstance(context);
                if (callState == TelephonyManager.CALL_STATE_IDLE) {
                    //update all showing views to not touchable
                    updateWindow.updateShowingView(false);
                } else {
                    //update all showing views to touchable
                    updateWindow.updateShowingView(true);
                }
            }
        }
    };

    @Override
    public void registerBroadcastToCheckCallState(Context context) {

        IntentFilter filter = new IntentFilter();
        filter.addAction(TelephonyManager.ACTION_PHONE_STATE_CHANGED);
        context.registerReceiver(mBr, filter);
    }


    /**
     * Handle message with phone number, hyperlink, or url
     */
    @Override
    public boolean setTextViewContent(TextView tv, String content,
                                            IAutoLinkClick autoLinkClick, int msgId) {
        SpannableString text = new SpannableString(content);
        CMASLinkify.addLinks(text, CMASLinkify.ALL, autoLinkClick);

        tv.setText(text);
        tv.setMovementMethod(LinkMovementMethod.getInstance());

        return true;
    }

    public boolean hideUpdatedString() {
        Log.d(TAG, "hideUpdatedString = true");
        return true;
    }

    public boolean needPresidentAlert() {
        Log.d(TAG, "needPresidentAlert = true");
        return true;
    }
}
