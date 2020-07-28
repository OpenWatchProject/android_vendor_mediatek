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
package com.mediatek.camera.mode.pip.recorder;

import android.media.MediaRecorder;
import android.view.Surface;

import com.mediatek.camera.util.Log;
import com.mediatek.camera.util.Util;

import java.io.IOException;

/**
 * This class is used to wrap MediaRecorder or MediaCodec video recording.
 * <p>
 * When MediaRecorder.getSurface is enable, we will go MediaRecorder path,
 * otherwise we will choose MediaCodec path.
 *
 */
public class MediaRecorderWrapper implements MediaRecorder.OnInfoListener {
    private static final String TAG = "MediaRecorderWrapper";
    private MediaRecorder mMediaRecorder;
    private OnInfoListener mOnInfoListener;
    /**
     * Unspecified media recorder error.
     */
    public static final int MEDIA_RECORDER_INFO_UNKNOWN               = 1;
    /**
     * A maximum duration had been setup and has now been reached.
     */
    // public static final int MEDIA_RECORDER_INFO_MAX_DURATION_REACHED  = 800;
    /**
     * A maximum file size had been setup and has now been reached.
     */
    public static final int MEDIA_RECORDER_INFO_MAX_FILESIZE_REACHED  = 801;
    public static final int MEDIA_RECORDER_INFO_RECORDED_SIZE         = 895;
    // public static final int MEDIA_RECORDER_INFO_SESSIONID             = 896;
    // public static final int MEDIA_RECORDER_INFO_FPS_ADJUSTED          = 897;
    // public static final int MEDIA_RECORDER_INFO_BITRATE_ADJUSTED      = 898;
    public static final int MEDIA_RECORDER_INFO_WRITE_SLOW            = 899;
    public static final int MEDIA_RECORDER_INFO_START_TIMER           = 1998;
    public static final int MEDIA_RECORDER_INFO_CAMERA_RELEASE        = 1999;
    public static final String RECORDER_INFO_SUFFIX = "media-recorder-info=";
    /**
     * Interface definition for a callback to be invoked while recording.
     */
    public interface OnInfoListener {
        /**
         * Called when an informational event occurs while recording.
         *
         * @param mr the MediaRecorderWrapper that encountered the event
         * @param what    the type of event that has occurred:
         * @param extra   an extra code, specific to the event type
         */
        void onInfo(MediaRecorderWrapper mr, int what, int extra);
    }

    @Override
    public void onInfo(MediaRecorder mr, int what, int extra) {
        if (mOnInfoListener != null) {
            mOnInfoListener.onInfo(this, what, extra);
        }
    }

    /**
     * Default constructor.
     */
    public MediaRecorderWrapper() {
        Log.d(TAG, "MediaRecorderWrapper");
        mMediaRecorder = new MediaRecorder();
    }

    /**
     * Sets the format of the output file produced during recording. Call this
     * after setAudioSource()/setVideoSource() but before prepare().
     *
     * <p>It is recommended to always use 3GP format when using the H.263
     * video encoder and AMR audio encoder. Using an MPEG-4 container format
     * may confuse some desktop players.</p>
     *
     * @param fileFormat the output format to use. The output format
     * needs to be specified before setting recording-parameters or encoders.
     * @throws IllegalStateException if it is called after prepare() or before
     * setAudioSource()/setVideoSource().
     * @see android.media.MediaRecorder.OutputFormat
     */
    public void setOutputFormat(int fileFormat) throws IllegalStateException {
        Log.d(TAG, "Initialize >>> setOutputFormat = " + fileFormat);
        mMediaRecorder.setOutputFormat(fileFormat);
    }

    /**
     * Sets the maximum filesize (in bytes) of the recording session.
     * Call this after setOutFormat() but before prepare().
     * After recording reaches the specified filesize, a notification
     * will be sent to the {@link android.media.MediaRecorder.OnInfoListener}
     * with a "what" code of {@link #MEDIA_RECORDER_INFO_MAX_FILESIZE_REACHED}
     * and recording will be stopped. Stopping happens asynchronously, there
     * is no guarantee that the recorder will have stopped by the time the
     * listener is notified.
     *
     * @param filesizeInBytes the maximum filesize
     * in bytes (if zero or negative, disables the limit)
     *
     */
    public void setMaxFileSize(long filesizeInBytes) {
        Log.d(TAG, "Initialize >>> setMaxFileSize = " + filesizeInBytes);
        mMediaRecorder.setMaxFileSize(filesizeInBytes);
    }

    /**
     * Sets the path of the output file to be produced. Call this after
     * setOutputFormat() but before prepare().
     *
     * @param path The pathname to use.
     * @throws IllegalStateException if it is called before
     * setOutputFormat() or after prepare()
     */
    public void setOutputFile(String path) throws IllegalStateException {
        Log.d(TAG, "Initialize >>> setOutputFile = " + path);
        mMediaRecorder.setOutputFile(path);
    }

    /**
     * Sets the orientation hint for output video playback.
     * This method should be called before prepare(). This method will not
     * trigger the source video frame to rotate during video recording, but to
     * add a composition matrix containing the rotation angle in the output
     * video if the output format is OutputFormat.THREE_GPP or
     * OutputFormat.MPEG_4 so that a video player can choose the proper
     * orientation for playback. Note that some video players may choose
     * to ignore the compostion matrix in a video during playback.
     *
     * @param degrees the angle to be rotated clockwise in degrees.
     * The supported angles are 0, 90, 180, and 270 degrees.
     * @throws IllegalArgumentException if the angle is not supported.
     *
     */
    public void setOrientationHint(int degrees) throws IllegalArgumentException {
        Log.d(TAG, "Initialize >>> setOrientationHint = " + degrees);
        mMediaRecorder.setOrientationHint(degrees);
    }

    /**
     * Set and store the geodata (latitude and longitude) in the output file.
     * This method should be called before prepare(). The geodata is
     * stored in udta box if the output format is OutputFormat.THREE_GPP
     * or OutputFormat.MPEG_4, and is ignored for other output formats.
     * The geodata is stored according to ISO-6709 standard.
     *
     * @param latitude latitude in degrees. Its value must be in the
     * range [-90, 90].
     * @param longitude longitude in degrees. Its value must be in the
     * range [-180, 180].
     *
     * @throws IllegalArgumentException if the given latitude or
     * longitude is out of range.
     *
     */
    public void setLocation(long latitude, long longitude) throws IllegalArgumentException {
        Log.d(TAG, "Initialize >>> setLocation latitude = " + latitude
                    + " longitude = " + longitude);
        mMediaRecorder.setLocation(latitude, longitude);
    }

    /**
     * Sets the video encoder to be used for recording. If this method is not
     * called, the output file will not contain an video track. Call this after
     * setOutputFormat() and before prepare().
     *
     * @param videoEncoder the video encoder to use.
     * @throws IllegalStateException if it is called before
     * setOutputFormat() or after prepare()
     * @see android.media.MediaRecorder.VideoEncoder
     */
    public void setVideoEncoder(int videoEncoder) throws IllegalStateException {
        Log.d(TAG, "Initialize >>> setVideoEncoder video_encoder =  " + videoEncoder);
        mMediaRecorder.setVideoEncoder(videoEncoder);
    }

    /**
     * Sets the video source to be used for recording. If this method is not
     * called, the output file will not contain an video track. The source needs
     * to be specified before setting recording-parameters or encoders. Call
     * this only before setOutputFormat().
     *
     * @param videoSource the video source to use
     * @throws IllegalStateException if it is called after setOutputFormat()
     * @see android.media.MediaRecorder.VideoSource
     */
    public void setVideoSource(int videoSource) throws IllegalStateException {
        Log.d(TAG, "Initialize >>> setVideoSource video_source =  " + videoSource);
        mMediaRecorder.setVideoSource(videoSource);
    }

    /**
     * Sets the frame rate of the video to be captured.  Must be called
     * after setVideoSource(). Call this after setOutFormat() but before
     * prepare().
     *
     * @param rate the number of frames per second of video to capture
     * @throws IllegalStateException if it is called after
     * prepare() or before setOutputFormat().
     *
     * NOTE: On some devices that have auto-frame rate, this sets the
     * maximum frame rate, not a constant frame rate. Actual frame rate
     * will vary according to lighting conditions.
     */
    public void setVideoFrameRate(int rate) throws IllegalStateException {
        Log.d(TAG, "Initialize >>> setVideoFrameRate videoFrameRate =  " + rate);
        mMediaRecorder.setVideoFrameRate(rate);
    }

    /**
     * Sets the width and height of the video to be captured.  Must be called
     * after setVideoSource(). Call this after setOutFormat() but before
     * prepare().
     *
     * @param width the width of the video to be captured
     * @param height the height of the video to be captured
     * @throws IllegalStateException if it is called after
     * prepare() or before setOutputFormat()
     */
    public void setVideoSize(int width, int height) throws IllegalStateException {
        Log.d(TAG, "Initialize >>> setVideoSize videoFrameWidth =  " + width
                   + " videoFrameHeight = " + height);
        mMediaRecorder.setVideoSize(width, height);
    }

    /**
     * Sets the video encoding bit rate for recording. Call this method before prepare().
     * Prepare() may perform additional checks on the parameter to make sure whether the
     * specified bit rate is applicable, and sometimes the passed bitRate will be
     * clipped internally to ensure the video recording can proceed smoothly based on
     * the capabilities of the platform.
     *
     * @param bitRate the video encoding bit rate in bits per second.
     */
    public void setVideoEncodingBitRate(int bitRate) {
        Log.d(TAG, "Initialize >>> setVideoEncodingBitRate bitRate =  " + bitRate);
        mMediaRecorder.setVideoEncodingBitRate(bitRate);
    }

    /**
     * Sets the audio encoder to be used for recording. If this method is not
     * called, the output file will not contain an audio track. Call this after
     * setOutputFormat() but before prepare().
     *
     * @param audioEncoder the audio encoder to use.
     * @throws IllegalStateException if it is called before
     * setOutputFormat() or after prepare().
     * @see android.media.MediaRecorder.AudioEncoder
     */
    public void setAudioEncoder(int audioEncoder) throws IllegalStateException {
        Log.d(TAG, "Initialize >>> setAudioEncoder audio_encoder =  " + audioEncoder);
        mMediaRecorder.setAudioEncoder(audioEncoder);
    }

    /**
     * Sets the audio source to be used for recording. If this method is not
     * called, the output file will not contain an audio track. The source needs
     * to be specified before setting recording-parameters or encoders. Call
     * this only before setOutputFormat().
     *
     * @param audioSource the audio source to use
     * @throws IllegalStateException if it is called after setOutputFormat()
     * @see android.media.MediaRecorder.AudioSource
     */
    public void setAudioSource(int audioSource) throws IllegalStateException {
        Log.d(TAG, "Initialize >>> setAudioSource audioSource =  " + audioSource);
        mMediaRecorder.setAudioSource(audioSource);
    }

    /**
     * Sets the audio encoding bit rate for recording. Call this method before prepare().
     * Prepare() may perform additional checks on the parameter to make sure whether the
     * specified bit rate is applicable, and sometimes the passed bitRate will be clipped
     * internally to ensure the audio recording can proceed smoothly based on the
     * capabilities of the platform.
     *
     * @param bitRate the audio encoding bit rate in bits per second.
     */
    public void setAudioEncodingBitRate(int bitRate) {
        Log.d(TAG, "Initialize >>> setAudioEncodingBitRate bitRate =  " + bitRate);
        mMediaRecorder.setAudioEncodingBitRate(bitRate);
    }

    /**
     * Sets the number of audio channels for recording. Call this method before prepare().
     * Prepare() may perform additional checks on the parameter to make sure whether the
     * specified number of audio channels are applicable.
     *
     * @param numChannels the number of audio channels. Usually it is either 1 (mono) or 2
     * (stereo).
     */
    public void setAudioChannels(int numChannels) {
        Log.d(TAG, "Initialize >>> setAudioChannels numChannels =  " + numChannels);
        mMediaRecorder.setAudioChannels(numChannels);
    }

    /**
     * Sets the audio sampling rate for recording. Call this method before prepare().
     * Prepare() may perform additional checks on the parameter to make sure whether
     * the specified audio sampling rate is applicable. The sampling rate really depends
     * on the format for the audio recording, as well as the capabilities of the platform.
     * For instance, the sampling rate supported by AAC audio coding standard ranges
     * from 8 to 96 kHz, the sampling rate supported by AMRNB is 8kHz, and the sampling
     * rate supported by AMRWB is 16kHz. Please consult with the related audio coding
     * standard for the supported audio sampling rate.
     *
     * @param samplingRate the sampling rate for audio in samples per second.
     */
    public void setAudioSamplingRate(int samplingRate) {
        Log.d(TAG, "Initialize >>> setAudioSamplingRate samplingRate =  " + samplingRate);
        mMediaRecorder.setAudioSamplingRate(samplingRate);
    }

    /**
     * Set extra parameters.
     */
    public void setParametersExtra() {
        setMediaRecorderParameters(mMediaRecorder);
    }

    /**
     * Prepares the recorder to begin capturing and encoding data. This method
     * must be called after setting up the desired audio and video sources,
     * encoders, file format, etc., but before start().
     *
     * @throws IllegalStateException if it is called after
     * start() or before setOutputFormat().
     * @throws IOException if prepare fails otherwise.
     */
    public void prepare() throws IllegalStateException, IOException {
        Log.d(TAG, "prepare begin");
        mMediaRecorder.prepare();
        Log.d(TAG, "prepare end");
    }

    /**
     * Gets the surface to record from when using SURFACE video source.
     *
     * <p> May only be called after {@link #prepare}. Frames rendered to the Surface before
     * {@link #start} will be discarded.</p>
     *
     * @return Surface used to receive video frames.
     *
     * @throws IllegalStateException if it is called before {@link #prepare}, after
     * {@link #stop}, or is called when VideoSource is not set to SURFACE.
     * @see android.media.MediaRecorder.VideoSource
     */
    public Surface getSurface() throws IllegalStateException {
        return mMediaRecorder.getSurface();
    }

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
     * @throws IllegalStateException if it is called before
     * prepare().
     */
    public void start() throws IllegalStateException {
        Log.d(TAG, "start begin");
        mMediaRecorder.start();
        Log.d(TAG, "start end");
    }

    /**
      * Pauses the recording.
      * Call this method after MediaRecorderWrapper.start().
      * In addition, call MediaRecorderWrapper.
      * start() to resume the recorder after this method is called.
      *
      * @param recorder Recorder used to record video
      * @throws IllegalStateException If it is not called after MediaRecorder.start()
      */
     public void pause(MediaRecorderWrapper recorder) throws IllegalStateException {
         Log.d(TAG, "pause begin");
         if (recorder == null) {
             Log.e(TAG, "Null MediaRecorderWrapper!");
             return;
         }
         mMediaRecorder.pause();
         Log.d(TAG, "pause end");
     }

    /**
     * Resume to start video recording again.
     *
     * @param recorder the used recorder.
     */
    public void resume(MediaRecorderWrapper recorder) {
        Log.d(TAG, "resume begin");
        if (recorder == null) {
            Log.e(TAG, "[resume]Null MediaRecorderWrapper!");
            return;
        }
        mMediaRecorder.resume();
        Log.d(TAG, "resume end");
    }

    /**
     * Stops recording. Call this after start(). Once recording is stopped,
     * you will have to configure it again as if it has just been constructed.
     * Note that a RuntimeException is intentionally thrown to the
     * application, if no valid audio/video data has been received when stop()
     * is called. This happens if stop() is called immediately after
     * start(). The failure lets the application take action accordingly to
     * clean up the output file (delete the output file, for instance), since
     * the output file is not properly constructed when this happens.
     *
     * @throws IllegalStateException if it is called before start()
     */
    public void stop() throws IllegalStateException {
        Log.d(TAG, "stop begin");
        mMediaRecorder.stop();
        Log.d(TAG, "stop end");
    }

    /**
     * Releases resources associated with this MediaRecorder object.
     * It is good practice to call this method when you're done
     * using the MediaRecorder. In particular, whenever an Activity
     * of an application is paused (its onPause() method is called),
     * or stopped (its onStop() method is called), this method should be
     * invoked to release the MediaRecorder object, unless the application
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
    public void release() {
        Log.d(TAG, "release begin");
        mMediaRecorder.release();
        Log.d(TAG, "release end");
    }

    /**
     * Register a callback to be invoked when an informational event occurs while
     * recording.
     *
     * @param listener the callback that will be run
     */
    public void setOnInfoListener(OnInfoListener listener) {
        Log.d(TAG, "setOnInfoListener listener = " + listener);
        mOnInfoListener = listener;
        mMediaRecorder.setOnInfoListener(this);
    }

    private void setMediaRecorderParameters(MediaRecorder mediaRecorder) {
        try {
            Util.setParametersExtra(mediaRecorder, RECORDER_INFO_SUFFIX
                    + MEDIA_RECORDER_INFO_START_TIMER);
            Util.setParametersExtra(mediaRecorder, RECORDER_INFO_SUFFIX
                    + MEDIA_RECORDER_INFO_WRITE_SLOW);
            Util.setParametersExtra(mediaRecorder, RECORDER_INFO_SUFFIX
                    + MEDIA_RECORDER_INFO_CAMERA_RELEASE);
        } catch (Exception ex) {
            ex.printStackTrace();
        }
    }
}
