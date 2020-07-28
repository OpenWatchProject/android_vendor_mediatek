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
package com.mediatek.rcse.plugin.message;

import android.os.Bundle;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.IMessenger;
import android.os.Looper;
import android.os.Message;
import android.os.Parcelable;
import android.os.RemoteException;
import android.text.TextUtils;

import com.mediatek.rcse.api.Logger;
import com.mediatek.rcse.service.binder.WindowBinder;

import java.util.ArrayList;

/**
 * This controller is used in Plugin to communicate with ControllerImpl in UI process.
 */
public class PluginController {
    /**
     * The Constant TAG.
     */
    private static final String TAG = PluginController.class.getSimpleName();
    /**
     * The Constant PLUGIN_CONTROLLER_THREAD_NAME.
     */
    private static final String PLUGIN_CONTROLLER_THREAD_NAME =
            "RCS-e Plugin Controller";
    /**
     * The s instance.
     */
    private static PluginController sInstance = null;
    /**
     * The m messenger.
     */
    private static IMessenger sMessenger = null;
    /**
     * The m handler.
     */
    private PluginControllerHandler mHandler = null;
    /**
     * The Constant INDEX_ZERO.
     */
    public static final int INDEX_ZERO = 0;
    /**
     * The m handler thread.
     */
    private final HandlerThread mHandlerThread = new HandlerThread(
            PLUGIN_CONTROLLER_THREAD_NAME);

    /**
     * Initialize.
     *
     * @param messenger the messenger
     */
    static void initialize(IMessenger messenger) {
        if (null == sInstance || sMessenger == null) {
            Logger.d(TAG, "initialize() sInstance is null");
            sInstance = new PluginController(messenger);
        } else {
            Logger.w(TAG, "initialize() sInstance is not null");
        }
    }
    /**
     * Destroy instance.
     */
    static void destroyInstance() {
        sInstance = null;
    }
    /**
     * Obtain a message of the event type using specific tag and a list of message _id.
     *
     * @param eventType The type of event to be transfered
     * @param data The extra data attached to this message
     * @return The message
     */
    public static Message obtainMessage(int eventType, ArrayList<Integer> data) {
        Message message = obtainMessage(eventType, (String) null);
        Logger.d(TAG, "obtainMessage() array list: " + data);
        if (null != data) {
            Bundle bundle = message.getData();
            bundle.putIntegerArrayList(WindowBinder.REMOTE_KEY_DATA, data);
        }
        return message;
    }
    /**
     * Obtain a message of the event type using specific tag and a list of Parcelable.
     *
     * @param eventType The type of event to be transfered
     * @param tag The tag of object to receive this message
     * @param data The extra data attached to this message
     * @return The message
     */
    public static Message obtainMessage(int eventType, String tag,
            ArrayList<? extends Object> data) {
        Message message = obtainMessage(eventType, tag);
        Logger.d(TAG, "obtainMessage() array list: " + data);
        if (null != data && !data.isEmpty()) {
            Bundle bundle = message.getData();
            if (data.get(INDEX_ZERO) instanceof Parcelable) {
                bundle.putParcelableArrayList(WindowBinder.REMOTE_KEY_DATA,
                        (ArrayList<? extends Parcelable>) data);
            } else if (data.get(INDEX_ZERO) instanceof Integer) {
                bundle.putIntegerArrayList(WindowBinder.REMOTE_KEY_DATA,
                        (ArrayList<Integer>) data);
            }
            else if (data.get(INDEX_ZERO) instanceof String) {
				 Logger.d(TAG, "obtainMessage() stringa");
				bundle.putStringArrayList(WindowBinder.REMOTE_KEY_DATA, (ArrayList<String>) data);
			
			}
        }
        return message;
    }
    /**
     * Obtain a message of the event type using specific contact and a String.
     *
     * @param eventType The type of event to be transfered
     * @param contact The contact to receive this message
     * @param data The extra data attached to this message
     * @return The message
     */
    public static Message obtainMessage(int eventType, String contact,
            String data) {
        Message message = obtainMessage(eventType, (String) null);
        Logger.d(TAG, "obtainMessage() contact: " + contact + ", data: " + data);
        if (!TextUtils.isEmpty(data) && !TextUtils.isEmpty(contact)) {
            Bundle bundle = message.getData();
            bundle.putString(WindowBinder.REMOTE_KEY_DATA, data);
            bundle.putString(WindowBinder.REMOTE_KEY_CONTACT, contact);
        } else {
            Logger.w(TAG, "obtainMessage() empty contact or data");
        }
        return message;
    }
    /**
     * Obtain a message of the event type using specific tag and an integer value.
     *
     * @param eventType The type of event to be transfered
     * @param tag The tag of object to receive this message
     * @param data The extra data attached to this message
     * @return The message
     */
    public static Message obtainMessage(int eventType, String tag, int data) {
        Message message = obtainMessage(eventType, tag);
        Logger.d(TAG, "obtainMessage() int : " + data);
        Bundle bundle = message.getData();
        bundle.putInt(WindowBinder.REMOTE_KEY_DATA, data);
        return message;
    }
    /**
     * Obtain a message of the event type using specific tag and a boolean value.
     *
     * @param eventType The type of event to be transfered
     * @param tag The tag of object to receive this message
     * @param data The extra data attached to this message
     * @return The message
     */
    public static Message obtainMessage(int eventType, String tag, boolean data) {
        Message message = obtainMessage(eventType, tag);
        Logger.d(TAG, "obtainMessage() boolean : " + data);
        Bundle bundle = message.getData();
        bundle.putBoolean(WindowBinder.REMOTE_KEY_DATA, data);
        return message;
    }
    /**
     * Obtain a message of the event type using specific tag.
     *
     * @param eventType The type of event to be transfered
     * @param tag The tag of object to receive this message
     * @return The message
     */
    public static Message obtainMessage(int eventType, String tag) {
        Logger.d(TAG, "obtainMessage() eventType: " + eventType + " , tag: "
                + tag);
        initialize(sMessenger);
        Message message;
        try {
            message = sInstance.mHandler.obtainMessage(eventType);
            if (null != tag) {
                Bundle bundle = message.getData();
                bundle.putString(WindowBinder.REMOTE_KEY_TAG, tag);
            }
            return message;
        } catch (NullPointerException e) {
            Logger.d(TAG, "obtainMessage() exception: " + e);
        }
        return new Message();
    }
    /**
     * Instantiates a new plugin controller.
     *
     * @param messenger the messenger
     */
    private PluginController(IMessenger messenger) {
        sMessenger = messenger;
        mHandlerThread.start();
        mHandler = new PluginControllerHandler(mHandlerThread.getLooper());
    }

    /**
     * The Class PluginControllerHandler.
     */
    private class PluginControllerHandler extends Handler {
        /**
         * The Constant TAG.
         */
        private static final String TAG = "PluginControllerHandler";

        /**
         * Instantiates a new plugin controller handler.
         *
         * @param looper the looper
         */
        PluginControllerHandler(Looper looper) {
            super(looper);
        }
        /* (non-Javadoc)
         * @see android.os.Handler#handleMessage(android.os.Message)
         */
        @Override
        public void handleMessage(Message msg) {
            try {
                Logger.d(TAG, "handleMessage() incoming message: " + msg);
                Message remoteMessage = Message.obtain();
                remoteMessage.copyFrom(msg);
                sMessenger.send(remoteMessage);
            } catch (Exception e) {
                e.printStackTrace();
            }
        }
    }
}
