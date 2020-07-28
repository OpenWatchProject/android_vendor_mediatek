/* MediaTek Inc. (C) 2016. All rights reserved.
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

package com.mediatek.ims.internal;

import android.net.Uri;
import android.hardware.camera2.CameraCharacteristics;
import android.view.Surface;
import android.content.Context;
import android.util.Log;
import android.media.MediaPlayer;

import java.util.ArrayList;

/**
 * Provider source buffer for video call.
 * @hide
 */
public class VTDummySource extends VTSource {

    // private static Context sContext;
    // private static Resolution[] sCameraResolutions;
    private static final String TAG = "VT SRC_Dummy";
    // private MediaPlayer mMediaPlayer = null;

    /**
     * Set Context to VTSource.
     * @param context from IMS.
     * @hide
     */
    public static void setContext(Context context) {
        Log.d(TAG, "[STC] [setContext] context:" + context);
        sContext = context;
    }

    /**
     * Get current platform's all camera resolutions when boot then send to MA.
     * @return an array of all camera's resolution.
     * @hide
     */
    public static Resolution[] getAllCameraResolutions() {
        Log.d(TAG, "[STC] [getAllCameraResolutions] Start");

        if (sCameraResolutions == null) {
            ArrayList<Resolution> sensorResolutions = new ArrayList<>();
            try {
                // Dymmy back camera
                Resolution resolution = new Resolution();
                resolution.mId = 0;
                resolution.mMaxWidth = 5344;
                resolution.mMaxHeight = 5344;
                resolution.mDegree = 90;
                resolution.mFacing = 1;
                Log.w(TAG, "[getAllCameraResolutions] " + resolution);
                sensorResolutions.add(resolution);
                // Dymmy front camera
                Resolution resolution2 = new Resolution();
                resolution2.mId = 1;
                resolution2.mMaxWidth = 3264;
                resolution2.mMaxHeight = 3264;
                resolution2.mDegree = 270;
                resolution2.mFacing = 0;
                Log.w(TAG, "[getAllCameraResolutions] " + resolution2);
                sensorResolutions.add(resolution2);
            } catch (Exception  e) {
                Log.e(TAG, "[STC] [getAllCameraResolutions] getCameraIdList with exception:"
                        + e);
            }
            if (sensorResolutions.size() > 0) {
                sCameraResolutions = new Resolution[sensorResolutions.size()];
                sCameraResolutions = sensorResolutions.toArray(sCameraResolutions);
            }
            Log.d(TAG, "[STC] [getAllCameraResolutions] resolution size:"
                    + sensorResolutions.size());
        }
        Log.d(TAG, "[STC] [getAllCameraResolutions] Finish");
        return sCameraResolutions;
    }

    /**
     * New VTSource with 3G/4G mode;
     * 4G need rotate buffer to portrait.
     * 3G no need rotate buffer, keep it the same with sensor orientation.
     * @param mode current mode.
     * @param callId  call ID.
     * @hide
     */
    public VTDummySource() {
        Log.d(TAG, "[INT] [VTDummySource] Finish");
    }

    /**
     * Set replace picture path.
     * @param uri the replaced picture's uri.
     * @hide
     */
    public void setReplacePicture(Uri uri) {
    }

    /**
     * open camera, if another camera is running, switch camera.
     * @param cameraId indicate which camera to be opened.
     * @hide
     */
    public void open(String cameraId) {
        /*
        mMediaPlayer = new MediaPlayer();
        mMediaPlayer.reset();
        mMediaPlayer.setAudioStreamType(AudioManager.STREAM_MUSIC);
        Uri uri = Uri.parse("");
        mMediaPlayer.setDataSource(sContext, uri);
        mMediaPlayer.prepareAsync();
        */
    }

    /**
     * Close current opened camera.
     * @hide
     */
    public void close() {
    }

    /**
     * Release resource when do not use it.
     * @hide
     */
    public void release() {
    }

    /**
     * Set BufferQueueProducer to VTSource to put image data.
     * @param surface the surface used to receive record buffer.
     * @hide
     */
    public void setRecordSurface(Surface surface) {
        // mMediaPlayer.setSurface(surface);
    }

    /**
     * Update preview surface, if surface is null, do stop preview and clear cached preview surface.
     * @param surface the surface used to receive preview buffer.
     * @hide
     */
    public void setPreviewSurface(Surface surface) {
        // mMediaPlayer.setSurface(surface);
    }

    /**
     * Perform zoom by specified zoom value.
     * @param zoomValue the wanted zoom value.
     * @hide
     */
    public void setZoom(float zoomValue) {
    }

    /**
     * Get current using camera's characteristics.
     * @return an instance of camera's characteristics, if camera closed we return null.
     */
    public CameraCharacteristics getCameraCharacteristics() {
        return null;
    }

    /**
     * Start preview and recording.
     * @hide
     */
    public void startRecording() {
        // mMediaPlayer.start();
    }

    /**
     * Stop recording.
     * @hide
     */
    public void stopRecording() {
        // mMediaPlayer.stop();
    }

    /**
     * Stop preview and recording.
     * @hide
     */
    public void stopRecordingAndPreview() {
    }

    /**
     * If RJIL NW, replace output with picture data else drop camera data.
     * @hide
     */
    public void hideMe() {
    }

    /**
     * If RJIL NW, resume the camera output else stop dropping camera data.
     * @hide
     */
    public void showMe() {
    }

    /**
     * Set device orientation.
     * @hide
     */
    public void setDeviceOrientation(int degree) {
    }

}
