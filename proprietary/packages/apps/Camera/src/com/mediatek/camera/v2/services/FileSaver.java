/*
 * Copyright (C) 2013 The Android Open Source Project
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

package com.mediatek.camera.v2.services;

import android.content.ContentResolver;
import android.content.ContentValues;
import android.net.Uri;

public interface FileSaver {

    /**
     * An interface defining the callback when a file is saved.
     */
    public interface OnFileSavedListener {
        /**
         * The callback when the saving is done in the background.
         * @param uri The final content Uri of the saved file.
         */
        public void onMediaSaved(Uri uri);
    }

    /**
     * An interface defining the callback for task queue status changes.
     */
    public interface QueueListener {
        /**
         * The callback when the queue status changes.
         *
         * @param full Whether the queue is full.
         */
        public void onQueueStatus(boolean full);
    }

    public void addImage(byte data[], ContentValues values, OnFileSavedListener l,
            ContentResolver resolver);

    public void addVideo(String path, ContentValues values, OnFileSavedListener l,
            ContentResolver resolver);

    /**
     * Sets the queue listener.
     */
    public void setQueueListener(QueueListener l);

    /**
     * Checks whether the queue is full.
     */
    public boolean isQueueFull();
}
