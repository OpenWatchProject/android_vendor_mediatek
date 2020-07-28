/*
 *   Copyright Statement:
 *
 *     This software/firmware and related documentation ("MediaTek Software") are
 *     protected under relevant copyright laws. The information contained herein is
 *     confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 *     the prior written permission of MediaTek inc. and/or its licensors, any
 *     reproduction, modification, use or disclosure of MediaTek Software, and
 *     information contained herein, in whole or in part, shall be strictly
 *     prohibited.
 *
 *     MediaTek Inc. (C) 2016. All rights reserved.
 *
 *     BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 *    THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 *     RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 *     ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 *     WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 *     WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 *     NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 *     RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 *     INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 *     TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 *     RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 *     OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 *     SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 *     RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 *     STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 *     ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 *     RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 *     MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 *     CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     The following software/firmware and/or related documentation ("MediaTek
 *     Software") have been modified by MediaTek Inc. All revisions are subject to
 *     any receiver's applicable license agreements with MediaTek Inc.
 */
package com.mediatek.camera.common.mode.video.recorder;

import android.hardware.Camera;
import android.location.Location;
import android.media.CamcorderProfile;
import android.media.MediaRecorder;
import android.view.Surface;


import java.io.FileDescriptor;

import javax.annotation.Nonnull;
/**
 * interface IRecorder used for all record related feature
 * to initialize and control the recording flow.
 * sub class should implement it and new a media recorder object instance.
 */
public interface IRecorder {
    /**
     * initialize media recorder and set recorder parameter for recorder.
     * @param spec a packet of parameters for media recorder.
     */
    void init(@Nonnull RecorderSpec spec);
    /**
     * Gets the surface to record from when using SURFACE video source.
     *
     * <p> May only be called after {@link #prepare}. Frames rendered to the Surface before
     * {@link #start} will be discarded.</p>
     *
     * throws IllegalStateException if it is called before {@link #prepare}, after
     * {@link #stop}, or is called when VideoSource is not set to SURFACE.
     * @see android.media.MediaRecorder.VideoSource
     * @return surface.
     */
    Surface getSurface();

    /**
     * user can get media recorder to special config it.
     * @return media recorder
     */
    MediaRecorder getMediaRecorder();
    /**
     * RecorderSpec provides a structure for recorder feature
     * to specify their config for media recorder.
     *
     * Once constructed by a feature, this class should be
     * treated as read only.
     *
     * The application then edits this spec according to
     * media recorder interface.
     */
    /**
     * Prepares the recorder to begin capturing and encoding data. This method
     * must be called after setting up the desired audio and video sources,
     * encoders, file format, etc., but before start().
     * throws IllegalStateException if it is called after
     * start() or before setOutputFormat().
     * throws IOException if prepare fails otherwise.
     */
    void prepare();
    /**
     * Begins capturing and encoding data to the file specified with
     * setOutputFile(). Call this after prepare().
     *
     * <p>Since API level 13, if applications set a camera via
     * {@link #setCamera(Camera)}, the apps can use the camera after this method
     * call. The apps do not need to lock the camera again. However, if this
     * method fails, the apps should still lock the camera back. The apps should
     * not start another recording session during recording.
     *
     * throws IllegalStateException if it is called before
     * prepare().
     */
    void start();
    /**
     * Stops recording. Call this after start(). Once recording is stopped,
     * you will have to configure it again as if it has just been constructed.
     * Note that a RuntimeException is intentionally thrown to the
     * application, if no valid audio/video data has been received when stop()
     * is called. This happens if stop() is called immediately after
     * start(). The failure lets the application take action accordingly to
     * clean up the output file (delete the output file, for instance), since
     * the output file is not properly constructed when this happens.
     * throws IllegalStateException if it is called before start()
     */
    void stop();
    /**
     * Restarts the MediaRecorder to its idle state. After calling
     * this method, you will have to configure it again as if it had just been
     * constructed.
     */
    void reset();
    /**
     * Releases resources associated with this MediaRecorder object.
     * It is good practice to call this method when you're done
     * using the MediaRecorder. In particular, whenever an Activity
     * of an application is paused (its onPause() method is called),
     * or stopped (its onStop() method is called), this method should be
     * invoked to unInitializeVideoUI the MediaRecorder object, unless the application
     * has a special need to keep the object around. In addition to
     * unnecessary resources (such as memory and instances of codecs)
     * being held, failure to call this method immediately if a
     * MediaRecorder object is no longer needed may also lead to
     * continuous battery consumption for mobile devices, and recording
     * failure for other applications if no multiple instances of the
     * same codec are supported on a device. Even if multiple instances
     * of the same codec are supported, some performance degradation
     * may be expected when unnecessary multiple instances are used
     * at the same time.
     */
    void release();
    /**
     * the class RecorderSpec is used for video feature to config media recorder.
     * the fields can be extend by future features.
     * each instance of IRecorder will just care about what it will use of the fields.
     */
    static class RecorderSpec {
        /**
         * isRecordAudio used for whether need
         * record audio.
         * if value is false @audioSource will not need to set.
         */
        public boolean isRecordAudio = true;
        /**
         * the audio source to be used for recording.
         * if this is not been set ,the output file will not
         * contain an audion track.
         */
        public int audioSource = 0;
        /**
         * the Camera to use for recording.
         * this is only use for API1,and will instead by
         * getSurface() in api2
         */
        public Camera camera = null;
        /**
         * the video source to be used for recording.
         * if this is not been set,the output file will
         * not contain an video track.
         * the value only supported 0,1,2.
         * other values will throw IllegalArgumentException
         */
        public int videoSource = 0;
        /**
         * Sets the path of the output file to be produced.
         * outFilePath will use if outFileDes is null.
         * and if both them are null will throw IllegalArgumentException.
         */
        public String outFilePath = null;
        /**
         * Pass in the file descriptor of the file to be written.
         */
        public FileDescriptor outFileDes = null;
        /**
         * Uses the settings from a CamcorderProfile object for recording.
         * this should not been null,otherwise will throw IllegalArgumentException.
         */
        public CamcorderProfile profile = null;
        /**
         * set location(latitude and longitude) into the output file.
         * latitude in degrees.Its value must be in the range [-90,90]
         * longitude in degrees.Its value must be in the range[-180,180]
         * if given latitude or longitude is out of range will
         *  throw IllegalArgumentException;
         */
        public Location location = null;
        /**
         * Sets the maximum duration (in ms) of the recording session.
         * if zero or negative,disable the duration limit.
         */
        public int maxDurationMs = 0;
        /**
         * Sets the maximum file size (in bytes) of the recording session.
         * if zero or negative disable the limit.
         */
        public long maxFileSizeBytes = 0;
        /**
         * Register a callback to be invoked when an informational event occurs while
         * recording.
         */
        public MediaRecorder.OnInfoListener infoListener = null;
        /**
         * Register a callback to be invoked when an error occurs while
         * recording.
         */
        public MediaRecorder.OnErrorListener errorListener = null;
        /**
         * Register a callback to invoked when release job has been done while recording.
         */
        public MediaRecorder.OnInfoListener releaseListener = null;
        /**
         * record orientation hint for output video playback.
         * the only supported angles are 0,90,180,270 degrees.
         * other values will throw IllegalArgumentException.
         */
        public int orientationHint;

        /**
         * Slow motion will config it and media recorder will know record slow motion by this.
         * captureRate at which frames should be captured in frames per second.
         */
        public int captureRate = 0;
        /**
         * slow motion will set videoFrameRate which means the final video fps.
         * because profile have videoFrameRate but we use it to set capture rate
         * and don't want to modify profile so add this
         */
        public int videoFrameRate = 0;
    }
}
