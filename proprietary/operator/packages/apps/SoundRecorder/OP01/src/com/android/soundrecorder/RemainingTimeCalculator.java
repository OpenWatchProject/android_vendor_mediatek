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
package com.android.soundrecorder;

import android.os.Environment;
import android.os.StatFs;
import android.os.SystemClock;
import android.os.storage.StorageManager;

import java.io.File;

/**
 * Class which is used to calculate remaining recording time.
 *
 */
public class RemainingTimeCalculator {
    private static final String TAG = "SR/RemainingTimeCalculator";
    public static final int UNKNOWN_LIMIT = 0;
    public static final int FILE_SIZE_LIMIT = 1;
    public static final int DISK_SPACE_LIMIT = 2;
    private static final int ONE_SECOND = 1000;
    private static final int BIT_RATE = 8;
    private static final float RESERVE_SAPCE = SoundRecorderService.LOW_STORAGE_THRESHOLD / 2;
    /** @} */
    private static final String RECORDING = "Recording";

    // which of the two limits we will hit (or have fit) first
    private int mCurrentLowerLimit = UNKNOWN_LIMIT;
    // Rate at which the file grows
    private int mBytesPerSecond;

    // the last time run timeRemaining()
    private long mLastTimeRunTimeRemaining;
    // the last remaining time
    private long mLastRemainingTime = -1;
    /** @} */
    private long mMaxBytes;
    // time at which number of free blocks last changed
    private long mBlocksChangedTime;
    // number of available blocks at that time
    private long mLastBlocks;
    // time at which the size of the file has last changed
    private long mFileSizeChangedTime = -1;
    // size of the file at that time
    private long mLastFileSize;

    // State for tracking file size of recording.
    private File mRecordingFile;
    private String mSDCardDirectory;
    private final StorageManager mStorageManager;
    // if recording has been pause
    private boolean mPauseTimeRemaining = false;
    private SoundRecorderService mService;
    private String mFilePath;

    /**
     * the construction of RemainingTimeCalculator.
     *
     * @param storageManager
     *            StorageManager
     * @param service service instance
     */
    public RemainingTimeCalculator(StorageManager storageManager, SoundRecorderService service) {
        /** M: initialize mStorageManager */
        mStorageManager = storageManager;
        /** M: initialize mSDCardDirectory using a function */
        getSDCardDirectory();
        /** M: initialize mService */
        mService = service;
    }

    /**
     * If called, the calculator will return the minimum of two estimates: how
     * long until we run out of disk space and how long until the file reaches
     * the specified size.
     *
     * @param file
     *            the file to watch
     * @param maxBytes
     *            the limit
     */
    public void setFileSizeLimit(File file, long maxBytes) {
        mRecordingFile = file;
        mMaxBytes = maxBytes;
    }

    /**
     * Resets the interpolation.
     */
    public void reset() {
        LogUtils.i(TAG, "<reset>");
        mCurrentLowerLimit = UNKNOWN_LIMIT;
        mBlocksChangedTime = -1;
        mFileSizeChangedTime = -1;
        /** M: reset new variable @{ */
        mPauseTimeRemaining = false;
        mLastRemainingTime = -1;
        mLastBlocks = -1;
        getSDCardDirectory();
        /** @} */
    }

    /**
     * M: return byte rate, using by SoundRecorder class when store state.
     *
     * @return byt e rate
     */
    public int getByteRate() {
        return mBytesPerSecond;
    }

    /**
     * M: in order to calculate more accurate remaining time, set
     * mPauseTimeRemaining as true when MediaRecorder pause recording.
     *
     * @param pause
     *            whether set mPauseTimeRemaining as true
     */
    public void setPauseTimeRemaining(boolean pause) {
        mPauseTimeRemaining = pause;
    }

    /**
     * M: Returns how long (in seconds) we can continue recording. Because the
     * remaining time is calculated by estimation, add man-made control to
     * remaining time, and make it not increase when available blocks is
     * reducing.
     *
     * @param isFirstTimeGetRemainingTime
     *            if the first time to getRemainingTime
     * @param isForStartRecording for start recording case
     * @return the remaining time that Recorder can record
     */
    public long timeRemaining(boolean isFirstTimeGetRemainingTime, boolean isForStartRecording) {
        /**
         * M:Modified for SD card hot plug-in/out. Should to check the savePath
         * of the current file rather than default write path.@{
         */
        if (isForStartRecording) {
            getSDCardDirectory();
            mFilePath = null;
        } else {
            mFilePath = mService.getCurrentFilePath();
        }
        if (mFilePath != null) {
            int index = mFilePath.indexOf(RECORDING, 0) - 1;
            mSDCardDirectory = mFilePath.substring(0, index);
        }
        LogUtils.i(TAG, "timeRemaining --> mFilePath is :" + mFilePath);
        /**@}*/
        // Calculate how long we can record based on free disk space
        // LogUtils.i(TAG,"<timeRemaining> mBytesPerSecond = " +
        // mBytesPerSecond);
        boolean blocksNotChangeMore = false;
        StatFs fs = null;
        long blocks = 0;
        long blockSize = 0;
        try {
            fs = new StatFs(mSDCardDirectory);
            blocks = fs.getAvailableBlocks() - 1;
            blockSize = fs.getBlockSize();
        } catch (IllegalArgumentException e) {
            fs = null;
            LogUtils.d(TAG, "stat " + mSDCardDirectory + " failed...");
            return SoundRecorderService.ERROR_PATH_NOT_EXIST;
        }
        long now = SystemClock.elapsedRealtime();
        if ((-1 == mBlocksChangedTime) || (blocks != mLastBlocks)) {
            // LogUtils.i(TAG, "<timeRemaining> blocks has changed from " +
            // mLastBlocks + " to "
            // + blocks);
            blocksNotChangeMore = (blocks <= mLastBlocks) ? true : false;
            // LogUtils.i(TAG, "<timeRemaining> blocksNotChangeMore = " +
            // blocksNotChangeMore);
            mBlocksChangedTime = now;
            mLastBlocks = blocks;
        } else if (blocks == mLastBlocks) {
            blocksNotChangeMore = true;
        }

        /*
         * The calculation below always leaves one free block, since free space
         * in the block we're currently writing to is not added. This last block
         * might get nibbled when we close and flush the file, but we won't run
         * out of disk.
         */

        // at mBlocksChangedTime we had this much time
        float resultTemp = ((float) (mLastBlocks * blockSize - RESERVE_SAPCE)) / mBytesPerSecond;

        // if recording has been pause, we should add pause time to
        // mBlocksChangedTime
        // LogUtils.i(TAG, "<timeRemaining> mPauseTimeRemaining = " +
        // mPauseTimeRemaining);
        if (mPauseTimeRemaining) {
            mBlocksChangedTime += (now - mLastTimeRunTimeRemaining);
            mPauseTimeRemaining = false;
        }
        mLastTimeRunTimeRemaining = now;

        // so now we have this much time
        resultTemp -= ((float) (now - mBlocksChangedTime)) / ONE_SECOND;
        long resultDiskSpace = (long) resultTemp;
        mLastRemainingTime = (-1 == mLastRemainingTime) ? resultDiskSpace : mLastRemainingTime;
        if (blocksNotChangeMore && (resultDiskSpace > mLastRemainingTime)) {
            // LogUtils.i(TAG, "<timeRemaining> result = " + resultDiskSpace
            // + " blocksNotChangeMore = true");
            resultDiskSpace = mLastRemainingTime;
            // LogUtils.i(TAG, "<timeRemaining> result = " + resultDiskSpace);
        } else {
            mLastRemainingTime = resultDiskSpace;
            // LogUtils.i(TAG, "<timeRemaining> result = " + resultDiskSpace);
        }

        if ((null == mRecordingFile) && !isFirstTimeGetRemainingTime) {
            mCurrentLowerLimit = DISK_SPACE_LIMIT;
            // LogUtils.i(TAG,
            // "<timeRemaining> mCurrentLowerLimit = DISK_SPACE_LIMIT "
            // + mCurrentLowerLimit);
            return resultDiskSpace;
        }

        // If we have a recording file set, we calculate a second estimate
        // based on how long it will take us to reach mMaxBytes.
        if (null != mRecordingFile) {
            mRecordingFile = new File(mRecordingFile.getAbsolutePath());
            long fileSize = mRecordingFile.length();

            if ((-1 == mFileSizeChangedTime) || (fileSize != mLastFileSize)) {
                mFileSizeChangedTime = now;
                mLastFileSize = fileSize;
            }
            long resultFileSize = (mMaxBytes - fileSize) / mBytesPerSecond;
            resultFileSize -= (now - mFileSizeChangedTime) / ONE_SECOND;
            resultFileSize -= 1; // just for safety
            mCurrentLowerLimit = (resultDiskSpace < resultFileSize) ? DISK_SPACE_LIMIT
                    : FILE_SIZE_LIMIT;
            // LogUtils.i(TAG, "<timeRemaining> mCurrentLowerLimit = " +
            // mCurrentLowerLimit);
            return Math.min(resultDiskSpace, resultFileSize);
        }
        return 0;
    }

    /**
     * Indicates which limit we will hit (or have hit) first, by returning one
     * of FILE_SIZE_LIMIT or DISK_SPACE_LIMIT or UNKNOWN_LIMIT. We need this to
     * display the correct message to the user when we hit one of the limits.
     *
     * @return current limit is FILE_SIZE_LIMIT or DISK_SPACE_LIMIT
     */
    public int currentLowerLimit() {
        return mCurrentLowerLimit;
    }

    /**
     * Sets the bit rate used in the interpolation.
     *
     * @param bitRate
     *            the bit rate to set in bits/second.
     */
    public void setBitRate(int bitRate) {
        mBytesPerSecond = bitRate / BIT_RATE;
        LogUtils.i(TAG, "<setBitRate> mBytesPerSecond = " + mBytesPerSecond);
    }

    /** M: define a function to initialize the SD Card Directory. */
    private void getSDCardDirectory() {
        if (null != mStorageManager) {
            mSDCardDirectory =
                    Environment.getExternalStorageDirectory().getAbsolutePath();
        }
    }

    /**
     * the remaining disk space that Record can record.
     *
     * @return the remaining disk space
     */
    public long diskSpaceRemaining() {
        StatFs fs = new StatFs(mSDCardDirectory);
        long blocks = fs.getAvailableBlocks() - 1;
        long blockSize = fs.getBlockSize();
        return (long) ((blocks * blockSize) - RESERVE_SAPCE);
    }
}
