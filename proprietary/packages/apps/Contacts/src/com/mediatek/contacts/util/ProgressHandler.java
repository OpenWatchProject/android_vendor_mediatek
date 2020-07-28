/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
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
package com.mediatek.contacts.util;

import android.app.FragmentManager;
import android.os.Handler;
import android.os.Message;

import com.mediatek.contacts.widget.SimpleProgressDialogFragment;

public class ProgressHandler extends Handler {
    private static final String TAG = "ProgressHandler";

    private static final int PROGRESS_DIALOG_SHOW = 0;
    private static final int PROGRESS_DIALOG_DISMISS = 1;

    public void showDialog(FragmentManager fm) {
        SimpleProgressDialogFragment.show(fm);
    }

    public void showDialogDelayed(FragmentManager fm, int millis) {
        sendMessageDelayed(obtainMessage(PROGRESS_DIALOG_SHOW, fm), millis);
    }

    public void dismissDialog(FragmentManager fm) {
        removeMessages(PROGRESS_DIALOG_SHOW);
        sendMessage(obtainMessage(PROGRESS_DIALOG_DISMISS, fm));
    }

    @Override
    public void handleMessage(Message msg) {
        Log.d(TAG, "[handleMessage]msg.what = " + msg.what + ", msg.obj = " + msg.obj);
        switch (msg.what) {
        case PROGRESS_DIALOG_SHOW:
            SimpleProgressDialogFragment.show((FragmentManager) msg.obj);
            break;

        case PROGRESS_DIALOG_DISMISS:
            SimpleProgressDialogFragment.dismiss((FragmentManager) msg.obj);
            break;
        default:
            Log.w(TAG, "[handleMessage]unexpected message: " + msg.what);
            break;
        }
    }
}
