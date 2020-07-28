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

package com.mediatek.cmas.ext;

import android.content.Context;
import android.content.ContextWrapper;
import android.telephony.CellBroadcastMessage;
import android.widget.TextView;
import android.util.Log;
import android.view.WindowManager;

public class DefaultCmasMessageInitiationExt extends ContextWrapper implements ICmasMessageInitiationExt {
    private static final String TAG = "CellBroadcastReceiver/DefaultCmasMessageInitiationExt";

    public DefaultCmasMessageInitiationExt(Context context) {
        super(context);
    }

    /**
     * Allow to answer Incoming Call on CMAS alert screen
     */
    public void registerBroadcastToCheckCallState(Context context){
    	Log.i(TAG, "Default registerBroadcastToCheckCallState");
    }

    public WindowManager.LayoutParams updateViewLpToTouchable(WindowManager.LayoutParams lp, boolean bTouchable) {
    	Log.i(TAG, "Default updateViewLpToTouchable");
        return lp;
    }

    public WindowManager.LayoutParams updateViewLayoutParams(WindowManager.LayoutParams lp) {
    	Log.i(TAG, "Default updateViewLayoutParams");
        return lp;
    }

    /**
     * Allow show MsgId on dialog
     */
    public String getMsgContent(String msgId, String content) {
    	Log.i(TAG, "Default getMsgContent");
        return content;
    }

    /**
     * Handle message with phone number, hyperlink, or url
     */
    public boolean setTextViewContent(TextView tv, String content, IAutoLinkClick autoLinkClick, int msgId) {
        Log.i(TAG, "Default setTextViewContent");
    	tv.setText(content);

        return true;
    }

    /**
     * Hide "has been updated" String in dialog
     */
    public boolean hideUpdatedString() {
        return false;
    }

    /**
     * check If support President Alert
     */
    public boolean needPresidentAlert() {
        return false;
    }

    public interface IAutoLinkClick {
        void onAutoLinkClicked(); //dismiss the dialog
    }

    /**
     * set Expiration Time for Sprint
     */
    public void setExpirationTime(Context hostContext, CellBroadcastMessage cbm) {
        Log.i(TAG, "Default setExpirationTime");
    }

    /**
     * get Expiration Time to show in alert dialog for Sprint
     */
    public long getExpirationTime(Context hostContext, int msgIdKey) {
        return 0L;
    }
}
