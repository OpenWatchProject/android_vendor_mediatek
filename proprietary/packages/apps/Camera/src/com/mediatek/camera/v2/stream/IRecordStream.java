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
 * MediaTek Inc. (C) 2014. All rights reserved.
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

package com.mediatek.camera.v2.stream;

import java.io.FileDescriptor;
import java.util.List;
import android.media.CamcorderProfile;
import android.view.Surface;

/**
 *
 */
public interface IRecordStream {
    public static final int MEDIA_RECORDER_INFO_MAX_DURATION_REACHED = 800;
    public static final int MEDIA_RECORDER_INFO_MAX_FILESIZE_REACHED = 801;
    public static final int MEDIA_RECORDER_INFO_RECORDING_SIZE = 895;
    public static final int MEDIA_RECORDER_INFO_WRITE_SLOW = 899;
    public static final int MEDIA_RECORDER_INFO_START_TIMER = 1998;
    public static final int MEDIA_RECORDER_ENCODER_ERROR = -1103;
    public static final int MEDIA_RECORDER_INFO_CAMERA_RELEASE = 1999;

    public static final String RECORDER_INFO_SUFFIX = "media-recorder-info=";

       /**
        * HD is a record mode.
        */
    public final class HDRecordMode {
        private HDRecordMode() {
        }
        public static final int UNSUPPORT = -1;
        //Normal mode
        public static final int NORMAL = 0;
        //Indoor mode
        public static final int INDOOR = 1;
        // Outdoor mode
        public static final int OUTDOOR = 2;
    }
    /**
     * Interface to respond recording state change.
     *
     */
    public interface RecordStreamStatus {
        /**
         * Interface when recording start will call.
         * @param canUseExtra can use tk parameter for media recorder
         */
        public void onRecordingStarted(boolean canUseExtra);
        /**
         * Interface when recording stop will call.
         */
        public void onRecordingStoped();
        /**
         * Interface during recording will receive media recorder info.
         * @param what info type
         * @param extra extra info
         */
        public void onInfo(int what, int extra);
        /**
         * Interface during if some error happen in media recorder will call this.
         * @param what info type
         * @param extra extra info
         */
        public void onError(int what, int extra);
    }
    /**
     * Add recording listener for respond @ RecordStreamStatus.
     * @param status RecordStreamStatus
     */
    public void registerRecordingObserver(RecordStreamStatus status);
    /**
     * Remove recording listener.
     * @param status RecordStreamStatus
     */
    public void unregisterCaptureObserver(RecordStreamStatus status);
    /**
     * Set camcorderProfile to media recorder.
     * @param profile camcorderProfile
     */
    public void setRecordingProfile(CamcorderProfile profile);
    /**
     * Set max duration to media recorder.
     * @param maxDurationMs target parameter
     */
    public void setMaxDuration(int maxDurationMs);
    /**
     * Set max file size to media recorder.
     * @param maxFilesizeBytes target parameter
     */
    public void setMaxFileSize(long maxFilesizeBytes);
    /**
     * Set out put file path to media recorder.
     * @param path out path
     */
    public void setOutputFile(String path);
    /**
     * Set out put file path.
     * @param fd file descriptor.
     */
    public void setOutputFile(FileDescriptor fd);
    /**
     * Set capture rate to media recorder.
     * @param fps target fps
     */
    public void setCaptureRate(double fps);
    /**
     * Set location info.
     * @param latitude latitude info
     * @param longitude longitude info
     */
    public void setLocation(float latitude, float longitude);
    /**
     * Set orientation info to media recorder.
     * @param degrees target degree
     */
    public void setOrientationHint(int degrees);
    /**
     * Set parameters to media recorder.
     * @param paramters target parameters
     */
    public void setMediaRecorderParameters(List<String> paramters);
    // audio part
    /**
     * Set audio recording whether is enable.
     * @param enableAudio true means recording audio ,false means not recording audio
     */
    public void enalbeAudioRecording(boolean enableAudio);
    /**
     * Set audio source to media recorder.
     * @param audioSource target source
     */
    public void setAudioSource(int audioSource);
    /**
     * Sets up the HD record mode to be used for recording.
     * @param mode mode HD record mode to be used
     */
    public void setHDRecordMode(String mode);
    // video part
    /**
     * Set video source to media recorder.
     * @param videoSource target source
     */
    public void setVideoSource(int videoSource);
    // life cycle
    /**
     * Prepare media recorder.
     */
    public void prepareRecord();
    /**
     * Start media recorder.
     */
    public void startRecord();
    /**
     * Pause media recorder.
     */
    public void pauseRecord();
    /**
     * Resume media recorder.
     */
    public void resumeRecord();
    /**
     * Stop video recording to media recorder.
     * @param needSaveVideo true will save the video, false will delete the video.
     */
    public void stopRecord(boolean needSaveVideo);
    /**
     * Delete the video file, file path is defined by {@link #setOutputFile(String)}.
     * @return true for delete success, false for delete fail.
     */
    public boolean deleteVideoFile();
    // input surface
    /**
     * Get recorder input surface.
     * @return surface that use for input.
     */
    public Surface getRecordInputSurface();
    /**
     * Release recorder stream.
     */
    public void releaseRecordStream();
}
