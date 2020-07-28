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

package com.mediatek.camera.common.utils;

import android.os.Handler;
import android.os.Message;

import com.mediatek.camera.common.debug.LogHelper;
import com.mediatek.camera.common.debug.LogUtil;

import java.util.concurrent.Semaphore;

import javax.annotation.Nonnull;
import javax.annotation.Nullable;

/**
 * Atom accessor for multi-thread operation.
 */
public final class AtomAccessor {
    private static final LogUtil.Tag TAG = new LogUtil.Tag(AtomAccessor.class.getSimpleName());
    private final Semaphore mSingleResource;
    private Object mResourceAccessLock = new Object();

    /**
     * Create a atom accessor.
     */
    public AtomAccessor() {
        LogHelper.d(TAG, "[AtomAccessor]");
        mSingleResource = new Semaphore(1, true);
    }

    /**
     * Send the message to front and runnable as an atom.
     * @param handler the handler used to post message and runnable.
     * @param msg the msg object.
     */
    public void sendAtomMessageAtFrontOfQueue(Handler handler, Message msg) {
        if (handler == null || msg == null) {
            return;
        }
        acquireResource();
        handler.sendMessageAtFrontOfQueue(msg);
        releaseResource();
    }

    /**
     * Send the message to front and runnable as an atom.
     * @param handler the handler used to post message and runnable.
     * @param msg the msg object.
     * @return whether wait success.
     */
    public boolean sendAtomMessageAtFrontOfQueueAndWait(Handler handler, Message msg) {
        acquireResource();
        handler.sendMessageAtFrontOfQueue(msg);
        return waitDoneAndReleaseResource(handler, null);
    }

    /**
     * Send the message and runnable as an atom.
     * @param handler the handler used to post message and runnable.
     * @param msg the msg object.
     */
    public void sendAtomMessage(Handler handler, Message msg) {
        if (handler == null || msg == null) {
            return;
        }
        acquireResource();
        handler.sendMessage(msg);
        releaseResource();
    }

    /**
     * Send the message and runnable as an atom.
     * @param handler the handler used to post message and runnable.
     * @param msg the msg object.
     * @param runnable the additional runnable need to post.
     * @return whether wait success.
     */
    public boolean sendAtomMessageAndWait(Handler handler, Message msg, Runnable runnable) {
        if (handler == null || msg == null) {
            return false;
        }
        acquireResource();
        handler.sendMessage(msg);
        return waitDoneAndReleaseResource(handler, runnable);
    }

    /**
     * Send the message and runnable as an atom.
     * @param handler the handler used to post message and runnable.
     * @param msg the msg object.
     * @return whether wait success.
     */
    public boolean sendAtomMessageAndWait(Handler handler, Message msg) {
        if (handler == null || msg == null) {
            return false;
        }
        acquireResource();
        handler.sendMessage(msg);
        return waitDoneAndReleaseResource(handler, null);
    }

    /**
     * Acquire the atom resource.
     */
    public void acquireResource() {
        synchronized (mResourceAccessLock) {
            mSingleResource.acquireUninterruptibly();
        }
    }

    /**
     * Release the atom resource.
     */
    public void releaseResource() {
        mSingleResource.release();
    }

    /**
     * Wait for the message is processed by post a runnable,
     * and release resource after post runnable.
     * @param handler the post notify wait done handler.
     * @param additionRunnable whether need additional runnable to post.
     * @return whether wait done success.
     */
    public boolean waitDoneAndReleaseResource(@Nonnull Handler handler,
                                              @Nullable Runnable additionRunnable) {
        if (handler == null) {
            return false;
        }
        final Object waitDoneObject = new Object();
        final Runnable unlockRunnable = new Runnable() {
            @Override
            public void run() {
                synchronized (waitDoneObject) {
                    waitDoneObject.notifyAll();
                }
            }
        };
        synchronized (waitDoneObject) {
            boolean postSuccess = handler.post(unlockRunnable);
            if (postSuccess && additionRunnable != null) {
                handler.post(additionRunnable);
            }
            mSingleResource.release();
            if (postSuccess) {
                try {
                    waitDoneObject.wait();
                } catch (InterruptedException e) {
                    e.printStackTrace();
                    return false;
                }
            }
            return postSuccess;
        }
    }

    /**
     * Wait for the message is processed by post a runnable.
     * @param handler the post notify wait done handler.
     * @return whether wait done success.
     */
    public boolean waitDone(@Nonnull Handler handler) {
        if (handler == null) {
            return false;
        }
        final Object waitDoneObject = new Object();
        final Runnable unlockRunnable = new Runnable() {
            @Override
            public void run() {
                synchronized (waitDoneObject) {
                    waitDoneObject.notifyAll();
                }
            }
        };
        synchronized (waitDoneObject) {
            boolean postSuccess = handler.post(unlockRunnable);
            if (postSuccess) {
                try {
                    waitDoneObject.wait();
                } catch (InterruptedException e) {
                    e.printStackTrace();
                    return false;
                }
            }
            return postSuccess;
        }
    }
}