/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensor. Without
 * the prior written permission of MediaTek inc. and/or its licensor, any
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
 * NON-INFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
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
package com.mediatek.camera.common.relation;

import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.CopyOnWriteArrayList;

/**
 * This class can be used for feature to monitor changed values of other features.
 */
public class StatusMonitor {
    private final ConcurrentHashMap<String, StatusResponder> mResponders
            = new ConcurrentHashMap<>();
    private final Object mResponderCreateDestroyLock = new Object();

    /**
     * A listener for monitoring the specified feature value.
     *
     *<p>Implement this listener and pass an instance to
     *{@link StatusMonitor#registerValueChangedListener} to notified of value changed</p>
     *
     */
    public interface StatusChangeListener {
        /**
         * Callback when feature value is changed.
         * @param key The string used to indicate value changed feature.
         * @param value The changed value of feature.
         */
        void onStatusChanged(String key, String value);
    }

    /**
     * This class is used to create a responder for the feature be monitored.
     *
     */
    public class StatusResponder {
        private final String mResponderName;
        private final CopyOnWriteArrayList<StatusChangeListener> mListeners
                = new CopyOnWriteArrayList<>();

        /**
         * StatusResponder constructor.
         * @param name The name of status responder.
         */
        private StatusResponder(String name) {
            mResponderName = name;
        }

        /**
         * Get the responder name.
         *
         * @return The name of responder.
         */
        public String getResponderName() {
            return mResponderName;
        }

        /**
         * When value of setting that this status responder is binding to is changed,
         * this method will be called.
         * @param key The setting key.
         * @param value The changed value of setting.
         */
        public void statusChanged(String key, String value) {
            for (StatusChangeListener listener : mListeners) {
                listener.onStatusChanged(key, value);
            }
        }

        /**
         * Add {@link StatusChangeListener} object into {@link StatusResponder} to callback
         * the changed value of the specified feature.
         * @param listener The instance of {@link StatusChangeListener}.
         */
        private void addListener(StatusChangeListener listener) {
            if (!mListeners.contains(listener)) {
                mListeners.add(listener);
            }
        }

        /**
         * Remove listener from {@link StatusResponder}.
         * @param listener The instance of {@link StatusChangeListener}.
         */
        private void removeListener(StatusChangeListener listener) {
            mListeners.remove(listener);
        }
    }

    /**
     * Feature registers listener to monitor the changed value of the specified feature.
     * A {@link StatusResponder} object will be created for receiving the changed value of the
     * specified feature and callback the value to the listeners.
     * @param key The string used to indicate the specified feature.
     * @param listener The instance of {@link StatusChangeListener} to receive callback.
     */
    public void registerValueChangedListener(String key,
            StatusChangeListener listener) {
        StatusResponder responder = getStatusResponderSync(key);
        responder.addListener(listener);
    }

    /**
     * Feature unregister listener to don't monitor the changed value of the specified feature.
     * @param key The string used to indicate the specified feature.
     * @param listener The instance of {@link StatusChangeListener}.
     */
    public void unregisterValueChangedListener(String key,
            StatusChangeListener listener) {
        removeStatusResponderSync(key, listener);
    }

    /**
     * Get a status responder for the specified feature, which is used to notify the
     * setting changed status to listeners.
     * @param key The string used to indicated setting.
     * @return The status responder for the specified setting indicated by input key.
     */
    public StatusResponder getStatusResponder(String key) {
        return getStatusResponderSync(key);
    }

    private StatusResponder getStatusResponderSync(String key) {
        synchronized (mResponderCreateDestroyLock) {
            StatusResponder responder = mResponders.get(key);
            if (responder == null) {
                responder = new StatusResponder(key);
                mResponders.put(key, responder);
            }
            return responder;
        }
    }

    private void removeStatusResponderSync(String key, StatusChangeListener statusListener) {
        synchronized (mResponderCreateDestroyLock) {
            StatusResponder responder = mResponders.get(key);
            if (responder != null) {
                responder.removeListener(statusListener);
            }
        }
    }
}
