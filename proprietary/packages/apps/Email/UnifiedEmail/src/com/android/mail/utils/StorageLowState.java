/*
 * Copyright (C) 2012 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
package com.android.mail.utils;

import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;

import java.util.ArrayList;
import java.util.List;

/**
 * Holds the storage state of the system. Allows for registering a single handler which can
 * adjust application state to deal with STORAGE_LOW and STORAGE_OK state:
 * Reference: {@link Intent#ACTION_DEVICE_STORAGE_LOW}, {@link Intent#ACTION_DEVICE_STORAGE_OK}
 */
public final class StorageLowState {
    /// M: Log tag
    public static final String TAG = LogUtils.TAG;
    /** M: Indicate why we check storage state */
    public static final int CHECK_FOR_GENERAL = 0;
    public static final int CHECK_FOR_VIEW_ATTACHMENT = 1;
    public static final int CHECK_FOR_DOWNLOAD_ATTACHMENT = 2;

    /**
     * Methods that are called when a device enters/leaves storage low mode.
     */
    public interface LowStorageHandler {
        /**
         * Method to be called when the device enters storage low mode.
         */
        void onStorageLow();

        /**
         * Method to be run when the device recovers from storage low mode.
         */
        void onStorageOk();

        /**
         * M: Method to be run when the device is in low storage state after we check, this function
         * should do some light weight work, while {@link #onStorageLow()} do some heavy weight jobs
         */
        void onStorageLowAfterCheck(final int checkReason);
    }
    /** True if the system has entered STORAGE_LOW state. */
    private static boolean sIsStorageLow = false;
    /** If non-null, this represents a handler that is notified of changes to state. */
    /** M: There may be more then one handlers. @{ */
    private static List<LowStorageHandler> sHandlers = new ArrayList<LowStorageHandler>();
    /** @} */

    /** Private constructor to avoid class instantiation. */
    private StorageLowState() {
        // Do nothing.
    }

    /**
     * M: check storage status for General purpose.
     * see {@link #checkIfStorageLow(Context, int)}
     * @param context
     * @return
     */
    public static boolean checkIfStorageLow(Context context) {
        return checkIfStorageLow(context, CHECK_FOR_GENERAL);
    }

    /**
     * M: Return whether if device is in storage low state. We should pass in why we check storage,
     * We could do different things in the handler.
     * This will notify {@link LowStorageHandler#onStorageLowAfterCheck} in storage low handler
     * if storage is low.
     * @return whether if device in low storage
     */
    public static boolean checkIfStorageLow(Context context, int checkReason) {
        LogUtils.d(TAG, "checkIfStorageLow");
        /** M: There may be more then one handlers, to notify all. @{ */
        if (sIsStorageLow) {
            for (LowStorageHandler handler : sHandlers) {
                handler.onStorageLowAfterCheck(checkReason);
            }
        }
        /** @} */
        return sIsStorageLow;
    }

    /**
     * Checks if the device is in storage low state. If the state changes, the handler is notified
     * of it. The handler is not notified if the state remains the same as before.
     */
    public static void checkStorageLowMode(Context context) {
        // Identify if we are in low storage mode. This works because storage low is a sticky
        // intent, so we are guaranteed a non-null intent if that broadcast was sent and not
        // cleared subsequently.
        final IntentFilter filter = new IntentFilter(Intent.ACTION_DEVICE_STORAGE_LOW);
        final Intent result = context.registerReceiver(null, filter);
        setIsStorageLow(result != null);
    }

    /**
     * Notifies {@link StorageLowState} that the device has entered storage low state.
     */
    public static void setIsStorageLow(boolean newValue) {
        LogUtils.d(TAG, "setIsStorageLow to: " + newValue);
        if (sIsStorageLow == newValue) {
            // The state is unchanged, nothing to do.
            return;
        }
        sIsStorageLow = newValue;

        /** M: There may be more then one handlers, to notify all. @{ */
        for (LowStorageHandler handler : sHandlers) {
            if (newValue) {
                handler.onStorageLow();
            } else {
                handler.onStorageOk();
            }
        }
        /** @} */
    }

    /**
     * M: Registers handlers that can adjust application state to deal with storage low and
     * storage ok intents.
     * Reference: {@link Intent#ACTION_DEVICE_STORAGE_LOW}, {@link Intent#ACTION_DEVICE_STORAGE_OK}
     * @param handler a handler that can deal with changes to the storage state.
     */
    public static void registerHandler(LowStorageHandler handler) {
        if (sHandlers.contains(handler)) {
            return;
        }
        LogUtils.d(TAG, "register LowStorage Handler: %s", handler);
        sHandlers.add(handler);
        // If we are currently in low storage mode, let the handler deal with it immediately.
        if (sIsStorageLow) {
            /// M: change to onStorageLowAfterCheck
            handler.onStorageLowAfterCheck(CHECK_FOR_GENERAL);
            /// M: some case the registerhandler is behind the low storage set, if this
            //  we should call onStorageLow derectly here.
            handler.onStorageLow();
        }
    }
}
