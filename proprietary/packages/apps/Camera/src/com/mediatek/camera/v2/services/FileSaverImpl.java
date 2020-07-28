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
import android.os.AsyncTask;
import android.provider.MediaStore.Files;
import android.provider.MediaStore.Images;
import android.provider.MediaStore.Images.ImageColumns;
import android.provider.MediaStore.Video;

import com.mediatek.camera.debug.LogHelper;
import com.mediatek.camera.debug.LogHelper.Tag;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;

/**
 * Functionality available to all modules and services.
 */
public class FileSaverImpl implements FileSaver {
    private static final Tag TAG = new Tag(FileSaverImpl.class.getSimpleName());
    private static final String TEMP_SUFFIX = ".tmp";
    private static final String MINI_IMAGE = "image/jpeg";
    /** The memory limit for unsaved image is 20MB. */
    private static final int SAVE_TASK_MEMORY_LIMIT = 20 * 1024 * 1024;

    /** Memory used by the total queued save request, in bytes. */
    private long mMemoryUse;
    private QueueListener mQueueListener;

    public FileSaverImpl() {
        mMemoryUse = 0;
    }

    @Override
    public void addImage(byte[] data, ContentValues values,
            OnFileSavedListener l, ContentResolver resolver) {
        if (isQueueFull()) {
            LogHelper.e(TAG, "Cannot add image when the queue is full");
            return;
        }
        mMemoryUse += data.length;
        new ImageSaveTask(data, values, l, resolver).execute();
    }

    @Override
    public void addVideo(String path, ContentValues values,
            OnFileSavedListener l, ContentResolver resolver) {
        new VideoSaveTask(path, values, l, resolver).execute();
    }

    @Override
    public void setQueueListener(QueueListener l) {
        mQueueListener = l;
    }

    @Override
    public boolean isQueueFull() {
        return (mMemoryUse >= SAVE_TASK_MEMORY_LIMIT);
    }

    private void onQueueAvailable() {
        if (mQueueListener != null) {
            mQueueListener.onQueueStatus(false);
        }
    }

    private class ImageSaveTask extends AsyncTask <Void, Void, Uri> {
        private byte[]                    mData;
        private final ContentValues       mValues;
        private final OnFileSavedListener mListener;
        private final ContentResolver     mResolver;

        public ImageSaveTask(byte[] data, ContentValues values,
                OnFileSavedListener l, ContentResolver resolver) {
            this.mData = data;
            this.mValues = new ContentValues(values);
            this.mListener = l;
            this.mResolver = resolver;
        }

        @Override
        protected void onPreExecute() {
            // do nothing
        }

        @Override
        protected Uri doInBackground(Void... params) {
            if (mData == null) {
                LogHelper.w(TAG, "[ImageSaveTask]mData is null,return!");
                return null;
            }
            String filePath = mValues.getAsString(ImageColumns.DATA);
            String tempFilePath = filePath + TEMP_SUFFIX;
            if (filePath == null) {
                LogHelper.w(TAG, "[ImageSaveTask]filePath is null, return");
                return null;
            }

            FileOutputStream out = null;
            try {
                // Write to a temporary file and rename it to the final name.
                // This
                // avoids other apps reading incomplete data.
                LogHelper.d(TAG, "[ImageSaveTask]save the data to SD Card");
                out = new FileOutputStream(tempFilePath);
                out.write(mData);
                out.close();
                new File(tempFilePath).renameTo(new File(filePath));
            } catch (IOException e) {
                LogHelper.e(TAG, "[ImageSaveTask]Failed to write image,ex:", e);
            } finally {
                if (out != null) {
                    try {
                        out.close();
                    } catch (IOException e) {
                        LogHelper.e(TAG, "[ImageSaveTask]IOException:", e);
                    }
                }
            }

            Uri uri = null;
            try {
                if (MINI_IMAGE.equals(mValues.getAsString(ImageColumns.MIME_TYPE))) {
                uri = mResolver.insert(Images.Media.EXTERNAL_CONTENT_URI, mValues);
                } else {
                    uri = mResolver.insert(Files.getContentUri("external"), mValues);
                }
                LogHelper.d(TAG, "[ImageSaveTask]insert image to database, uri:" + uri);
            } catch (IllegalArgumentException e) {
                LogHelper.e(TAG,
                        "[saveImageToDatabase]Failed to write MediaStore,IllegalArgumentException:",
                        e);
            } catch (UnsupportedOperationException e) {
                LogHelper.e(TAG,
                        "[saveImageToDatabase]Failed to" +
                        " write MediaStore,UnsupportedOperationException:",
                        e);
            }
            return uri;
        }

        @Override
        protected void onPostExecute(Uri result) {
            if (mListener != null) {
                mListener.onMediaSaved(result);
            }

            boolean previouslyFull = isQueueFull();
            mMemoryUse -= mData.length;
            if (isQueueFull() != previouslyFull) {
                onQueueAvailable();
            }
        }

    }

    private class VideoSaveTask extends AsyncTask <Void, Void, Uri> {
        private final String              mPath;
        private final ContentValues       mValues;
        private final OnFileSavedListener mListener;
        private final ContentResolver     mResolver;

        public VideoSaveTask(String path, ContentValues values, OnFileSavedListener l,
                             ContentResolver r) {
            this.mPath = path;
            this.mValues = values;
            this.mListener = l;
            this.mResolver = r;
        }

        @Override
        protected Uri doInBackground(Void... v) {
            String filePath = mValues.getAsString(Video.Media.DATA);
            File temp = new File(mPath);
            File file = new File(filePath);
            temp.renameTo(file);

            Uri uri = null;
            try {
                uri = mResolver.insert(Video.Media.EXTERNAL_CONTENT_URI, mValues);
            } catch (Exception e) {
                // We failed to insert into the database. This can happen if
                // the SD card is unmounted.
                LogHelper.e(TAG, "failed to add video to media store", e);
                uri = null;
            } finally {
                LogHelper.v(TAG, "Current video URI: " + uri);
            }
            return uri;
        }

        @Override
        protected void onPostExecute(Uri uri) {
            if (mListener != null) {
                mListener.onMediaSaved(uri);
            }
        }
    }
}
