/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * MediaTek Inc. (C) 2016. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

package com.mediatek.camera.common.device;

import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.util.Log;

import java.util.LinkedList;

/**
 * <p>A base handler used to record.</p>
 * <li>operation history</li>
 * <li>operation start, stop and pending time</li>
 */
public abstract class HistoryHandler extends Handler {
    private static final int MAX_HISTORY_SIZE = 400;
    private final LinkedList<Integer> mMsgHistory;
    protected long mMsgStartTime;
    protected long mMsgStopTime;

    @SuppressWarnings("unused")
    private HistoryHandler() {
        throw new AssertionError();
    }

    protected HistoryHandler(Looper looper) {
        super(looper);
        mMsgHistory = new LinkedList<Integer>();
        // We add a -1 at the beginning to mark the very beginning of the
        // history.
        mMsgHistory.offerLast(-1);
    }

    @Override
    public void handleMessage(Message msg) {
        mMsgHistory.offerLast(msg.what);
        while (mMsgHistory.size() > MAX_HISTORY_SIZE) {
            mMsgHistory.pollFirst();
        }
    }

    protected void printStartMsg(String tag, String operationStr, long pendingTime) {
        Log.i(tag, "[" + operationStr + "]+"
                + ", pending time = " + pendingTime + "ms.");
    }

    protected void printStopMsg(String tag, String operationStr, long executeTime) {
        Log.i(tag, "[" + operationStr + "]-"
                + ", executing time = " + executeTime + "ms.");
    }

    protected Integer getCurrentMessage() {
        return mMsgHistory.peekLast();
    }

    protected String generateHistoryString(int cameraId) {
        StringBuilder info = new StringBuilder();
        info.append("Begin is:").append("camera id:").append(cameraId);
        for (Integer msg : mMsgHistory) {
            info.append("_").append(msg);
        }
        info.append("End");
        return info.toString();
    }

    protected abstract void doHandleMessage(Message msg);
}