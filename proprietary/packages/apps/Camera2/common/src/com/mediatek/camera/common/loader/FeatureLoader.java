/*
 * Copyright Statement:
 *
 *   This software/firmware and related documentation ("MediaTek Software") are
 *   protected under relevant copyright laws. The information contained herein is
 *   confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 *   the prior written permission of MediaTek inc. and/or its licensors, any
 *   reproduction, modification, use or disclosure of MediaTek Software, and
 *   information contained herein, in whole or in part, shall be strictly
 *   prohibited.
 *
 *   MediaTek Inc. (C) 2016. All rights reserved.
 *
 *   BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 *   THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 *   RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 *   ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 *   WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 *   WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 *   NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 *   RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 *   INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 *   TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 *   RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 *   OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 *   SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 *   RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 *   STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 *   ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 *   RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 *   MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 *   CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *   The following software/firmware and/or related documentation ("MediaTek
 *   Software") have been modified by MediaTek Inc. All revisions are subject to
 *   any receiver's applicable license agreements with MediaTek Inc.
 */

package com.mediatek.camera.common.loader;

import android.content.Context;

import com.mediatek.camera.common.debug.LogHelper;
import com.mediatek.camera.common.debug.LogUtil.Tag;
import com.mediatek.camera.common.debug.profiler.IPerformanceProfile;
import com.mediatek.camera.common.debug.profiler.PerformanceTracker;
import com.mediatek.camera.common.device.CameraDeviceManagerFactory.CameraApi;
import com.mediatek.camera.common.mode.CameraApiHelper;
import com.mediatek.camera.common.mode.photo.PhotoModeEntry;
import com.mediatek.camera.common.mode.photo.intent.IntentPhotoModeEntry;
import com.mediatek.camera.common.mode.video.VideoModeEntry;
import com.mediatek.camera.common.mode.video.intentvideo.IntentVideoModeEntry;
import com.mediatek.camera.common.setting.ICameraSetting;
import com.mediatek.camera.feature.mode.dof.DofModeEntry;
import com.mediatek.camera.feature.mode.facebeauty.FaceBeautyModeEntry;
import com.mediatek.camera.feature.mode.hdr.HdrModeEntry;
import com.mediatek.camera.feature.mode.longexposure.LongExposureModeEntry;
import com.mediatek.camera.feature.mode.matrix.MatrixModeEntry;
import com.mediatek.camera.feature.mode.panorama.PanoramaModeEntry;
import com.mediatek.camera.feature.mode.slowmotion.SlowMotionEntry;
import com.mediatek.camera.feature.mode.vfacebeauty.VendorFaceBeautyModeEntry;
import com.mediatek.camera.feature.mode.vsdof.photo.SdofPhotoEntry;
import com.mediatek.camera.feature.setting.CameraSwitcherEntry;
import com.mediatek.camera.feature.setting.ContinuousShotEntry;
import com.mediatek.camera.feature.setting.aaaroidebug.AaaRoiDebugEntry;
import com.mediatek.camera.feature.setting.ais.AISEntry;
import com.mediatek.camera.feature.setting.antiflicker.AntiFlickerEntry;
import com.mediatek.camera.feature.setting.dng.DngEntry;
import com.mediatek.camera.feature.setting.dualcamerazoom.DualZoomEntry;
import com.mediatek.camera.feature.setting.fps60.Fps60Entry;
import com.mediatek.camera.feature.setting.eis.EISEntry;
import com.mediatek.camera.feature.setting.exposure.ExposureEntry;
import com.mediatek.camera.feature.setting.facedetection.FaceDetectionEntry;
import com.mediatek.camera.feature.setting.flash.FlashEntry;
import com.mediatek.camera.feature.setting.focus.FocusEntry;
import com.mediatek.camera.feature.setting.format.FormatEntry;
import com.mediatek.camera.feature.setting.hdr.HdrEntry;
import com.mediatek.camera.feature.setting.iso.ISOEntry;
import com.mediatek.camera.feature.setting.microphone.MicroPhoneEntry;
import com.mediatek.camera.feature.setting.noisereduction.NoiseReductionEntry;
import com.mediatek.camera.feature.setting.picturesize.PictureSizeEntry;
import com.mediatek.camera.feature.setting.postview.PostViewEntry;
import com.mediatek.camera.feature.setting.previewmode.PreviewModeEntry;
import com.mediatek.camera.feature.setting.scenemode.SceneModeEntry;
import com.mediatek.camera.feature.setting.selftimer.SelfTimerEntry;
import com.mediatek.camera.feature.setting.shutterspeed.ShutterSpeedEntry;
import com.mediatek.camera.feature.setting.slowmotionquality.SlowMotionQualityEntry;
import com.mediatek.camera.feature.setting.videoquality.VideoQualityEntry;
import com.mediatek.camera.feature.setting.whitebalance.WhiteBalanceEntry;
import com.mediatek.camera.feature.setting.zoom.ZoomEntry;
import com.mediatek.camera.feature.setting.zsd.ZSDEntry;

import java.util.Iterator;
import java.util.LinkedHashMap;
import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;

import javax.annotation.Nonnull;

/**
 * Used for load the features.
 */
public class FeatureLoader {
    private static final Tag TAG = new Tag(FeatureLoader.class.getSimpleName());
    private static final String CAMERA_SWITCH = "com.mediatek.camera.feature.setting.CameraSwitcherEntry";
    private static final String CONTINUOUSSHOT = "com.mediatek.camera.feature.setting.ContinuousShotEntry";
    private static final String DNG = "com.mediatek.camera.feature.setting.dng.DngEntry";
    private static final String DUAL_ZOOM =
            "com.mediatek.camera.feature.setting.dualcamerazoom.DualZoomEntry";
    private static final String SELFTIME = "com.mediatek.camera.feature.setting.selftimer.SelfTimerEntry";
    private static final String FACE_DETECTION = "com.mediatek.camera.feature.setting.facedetection.FaceDetectionEntry";
    private static final String FLASH = "com.mediatek.camera.feature.setting.flash.FlashEntry";
    private static final String HDR = "com.mediatek.camera.feature.setting.hdr.HdrEntry";
    private static final String PICTURE_SIZE = "com.mediatek.camera.feature.setting.picturesize.PictureSizeEntry";
    private static final String PREVIEW_MODE = "com.mediatek.camera.feature.setting.previewmode.PreviewModeEntry";
    private static final String VIDEO_QUALITY = "com.mediatek.camera.feature.setting.videoquality.VideoQualityEntry";
    private static final String ZOOM = "com.mediatek.camera.feature.setting.zoom.ZoomEntry";
    private static final String FOCUS = "com.mediatek.camera.feature.setting.focus.FocusEntry";
    private static final String EXPOSURE = "com.mediatek.camera.feature.setting.exposure.ExposureEntry";
    private static final String MICHROPHONE = "com.mediatek.camera.feature.setting.microphone.MicroPhoneEntry";
    private static final String NOISE_REDUCTION = "com.mediatek.camera.feature.setting.noisereduction.NoiseReductionEntry";
    private static final String EIS = "com.mediatek.camera.feature.setting.eis.EISEntry";
    private static final String FPS60 = "com.mediatek.camera.feature.setting.fps60.Fps60Entry";
    private static final String AIS = "com.mediatek.camera.feature.setting.ais.AISEntry";
    private static final String SCENE_MODE = "com.mediatek.camera.feature.setting.scenemode.SceneModeEntry";
    private static final String WHITE_BALANCE = "com.mediatek.camera.feature.setting.whitebalance.WhiteBalanceEntry";
    private static final String ANTI_FLICKER = "com.mediatek.camera.feature.setting.antiflicker.AntiFlickerEntry";
    private static final String ZSD = "com.mediatek.camera.feature.setting.zsd.ZSDEntry";
    private static final String ISO = "com.mediatek.camera.feature.setting.iso.ISOEntry";
    private static final String AE_AF_DEBUG = "com.mediatek.camera.feature.setting.aaaroidebug.AaaRoiDebugEntry";
    private static final String SDOF_PHOTO_MODE = "com.mediatek.camera.feature.mode.vsdof.photo.SdofPhotoEntry";
    private static final String SHUTTER_SPEED = "com.mediatek.camera.feature.setting.shutterspeed.ShutterSpeedEntry";
    private static final String LONG_EXPUSURE_MODE = "com.mediatek.camera.feature.mode.longexposure.LongExposureModeEntry";
    private static final String HDR_MODE = "com.mediatek.camera.feature.mode.hdr.HdrModeEntry";
    private static final String PANORAMA_MODE
            = "com.mediatek.camera.feature.mode.panorama.PanoramaModeEntry";
    private static final String PHOTO_MODE = "com.mediatek.camera.common.mode.photo.PhotoModeEntry";
    private static final String VIDEO_MODE = "com.mediatek.camera.common.mode.video.VideoModeEntry";
    private static final String INTENT_PHOTO_MODE
            = "com.mediatek.camera.common.mode.photo.intent.IntentPhotoModeEntry";
    private static final String INTENT_VIDEO_MODE
            = "com.mediatek.camera.common.mode.video.intentvideo.IntentVideoModeEntry";
    private static final String SLOW_MOTION_MODE
            = "com.mediatek.camera.feature.mode.slowmotion.SlowMotionEntry";
    private static final String MATRIX_MODE
            = "com.mediatek.camera.feature.mode.matrix.MatrixModeEntry";
    private static final String DOF_MODE = "com.mediatek.camera.feature.mode.dof.DofModeEntry";
    private static final String FORMATS = "com.mediatek.camera.feature.setting.format.FormatEntry";
    private static final String SLOW_MOTION_QUALITY =
            "com.mediatek.camera.feature.setting.videoquality.SlowMotionQualityEntry";
    private static final String POST_VIEW
            = "com.mediatek.camera.feature.setting.postview.PostViewEntry";
    private static final String FB_MODE
            = "com.mediatek.camera.feature.mode.facebeauty.FaceBeautyModeEntry";
    private static final String VFB_MODE
            = "com.mediatek.camera.feature.mode.vfacebeauty.VendorFaceBeautyModeEntry";
    private static ConcurrentHashMap<String, IFeatureEntry>
            sBuildInEntries = new ConcurrentHashMap<>();
    private static ConcurrentHashMap<String, IFeatureEntry>
            sPluginEntries = new ConcurrentHashMap<>();

    /**
     * Update current mode key to feature entry, dual camera zoom need to set properties
     * in photo and video mode before open camera, this notify only update to setting feature.
     *
     * @param context        current application context.
     * @param currentModeKey current mode key.
     */
    public static void updateSettingCurrentModeKey(@Nonnull Context context,
                                                   @Nonnull String currentModeKey) {
        LogHelper.d(TAG, "[updateCurrentModeKey] current mode key:" + currentModeKey);
        if (sBuildInEntries.size() <= 0) {
            loadBuildInFeatures(context);
        }
    }

    /**
     * Notify setting feature before open camera, this event only need to notify setting feature.
     *
     * @param context   the context.
     * @param cameraId  want to open which camera.
     * @param cameraApi use which api.
     */
    public static void notifySettingBeforeOpenCamera(@Nonnull Context context,
                                                     @Nonnull String cameraId,
                                                     @Nonnull CameraApi cameraApi) {
        LogHelper.d(TAG, "[notifySettingBeforeOpenCamera] id:" + cameraId + ", api:" + cameraApi);
        //don't consider plugin feature? because plugin feature need more time to load
        if (sBuildInEntries.size() <= 0) {
            loadBuildInFeatures(context);
        }
        Iterator iterator = sBuildInEntries.entrySet().iterator();
        while (iterator.hasNext()) {
            Map.Entry item = (Map.Entry) iterator.next();
            IFeatureEntry entry = (IFeatureEntry) item.getValue();
            if (ICameraSetting.class.equals(entry.getType())) {
                entry.notifyBeforeOpenCamera(cameraId, cameraApi);
            }
        }
    }

    /**
     * Load plugin feature entries, should be called in non-ui thread.
     *
     * @param context the application context.
     * @return the plugin features.
     */
    public static ConcurrentHashMap<String, IFeatureEntry> loadPluginFeatures(
            final Context context) {
        return sPluginEntries;
    }

    /**
     * Load build in feature entries, should be called in non-ui thread.
     *
     * @param context the application context.
     * @return the build-in features.
     */
    public static ConcurrentHashMap<String, IFeatureEntry> loadBuildInFeatures(Context context) {
        if (sBuildInEntries.size() > 0) {
            return sBuildInEntries;
        }
        IPerformanceProfile profile = PerformanceTracker.create(TAG,
                "Build-in Loading");
        profile.start();
        sBuildInEntries = new ConcurrentHashMap<>(loadClasses(context));
        profile.stop();
        return sBuildInEntries;
    }

    private static LinkedHashMap<String, IFeatureEntry> loadClasses(Context context) {
        LinkedHashMap<String, IFeatureEntry> entries = new LinkedHashMap<>();
        DeviceSpec deviceSpec = CameraApiHelper.getDeviceSpec(context);

        IFeatureEntry postviewEntry = new PostViewEntry(context, context.getResources());
        postviewEntry.setDeviceSpec(deviceSpec);
        entries.put(POST_VIEW, postviewEntry);

        IFeatureEntry cameraSwitchEntry = new CameraSwitcherEntry(context, context.getResources());
        cameraSwitchEntry.setDeviceSpec(deviceSpec);
        entries.put(CAMERA_SWITCH, cameraSwitchEntry);

        IFeatureEntry continuousShotEntry = new ContinuousShotEntry(context,
                context.getResources());
        continuousShotEntry.setDeviceSpec(deviceSpec);
        entries.put(CONTINUOUSSHOT, continuousShotEntry);

        IFeatureEntry dngEntry = new DngEntry(context, context.getResources());
        dngEntry.setDeviceSpec(deviceSpec);
        entries.put(DNG, dngEntry);

        IFeatureEntry dualZoomEntry = new DualZoomEntry(context, context.getResources());
        dualZoomEntry.setDeviceSpec(deviceSpec);
        entries.put(DUAL_ZOOM, dualZoomEntry);

        IFeatureEntry selfTimeEntry = new SelfTimerEntry(context, context.getResources());
        selfTimeEntry.setDeviceSpec(deviceSpec);
        entries.put(SELFTIME, selfTimeEntry);

        IFeatureEntry faceDetectionEntry = new FaceDetectionEntry(context, context.getResources());
        faceDetectionEntry.setDeviceSpec(deviceSpec);
        entries.put(FACE_DETECTION, faceDetectionEntry);

        IFeatureEntry flashEntry = new FlashEntry(context, context.getResources());
        flashEntry.setDeviceSpec(deviceSpec);
        entries.put(FLASH, flashEntry);

        IFeatureEntry hdrEntry = new HdrEntry(context, context.getResources());
        hdrEntry.setDeviceSpec(deviceSpec);
        entries.put(HDR, hdrEntry);

        IFeatureEntry hdrModeEntry = new HdrModeEntry(context, context.getResources());
        hdrModeEntry.setDeviceSpec(deviceSpec);
        entries.put(HDR_MODE, hdrModeEntry);

        IFeatureEntry panoramaModeEntry = new PanoramaModeEntry(context, context.getResources());
        panoramaModeEntry.setDeviceSpec(deviceSpec);
        entries.put(PANORAMA_MODE, panoramaModeEntry);

        IFeatureEntry pictureSizeEntry = new PictureSizeEntry(context, context.getResources());
        pictureSizeEntry.setDeviceSpec(deviceSpec);
        entries.put(PICTURE_SIZE, pictureSizeEntry);

        IFeatureEntry previewModeEntry = new PreviewModeEntry(context, context.getResources());
        previewModeEntry.setDeviceSpec(deviceSpec);
        entries.put(PREVIEW_MODE, previewModeEntry);

        IFeatureEntry videoQualityEntry = new VideoQualityEntry(context, context.getResources());
        videoQualityEntry.setDeviceSpec(deviceSpec);
        entries.put(VIDEO_QUALITY, videoQualityEntry);

        IFeatureEntry zoomEntry = new ZoomEntry(context, context.getResources());
        zoomEntry.setDeviceSpec(deviceSpec);
        entries.put(ZOOM, zoomEntry);

        IFeatureEntry focusEntry = new FocusEntry(context, context.getResources());
        focusEntry.setDeviceSpec(deviceSpec);
        entries.put(FOCUS, focusEntry);

        IFeatureEntry exposureEntry = new ExposureEntry(context, context.getResources());
        exposureEntry.setDeviceSpec(deviceSpec);
        entries.put(EXPOSURE, exposureEntry);

        IFeatureEntry microPhoneEntry = new MicroPhoneEntry(context, context.getResources());
        microPhoneEntry.setDeviceSpec(deviceSpec);
        entries.put(MICHROPHONE, microPhoneEntry);

        IFeatureEntry noiseReductionEntry = new NoiseReductionEntry(context, context.getResources());
        noiseReductionEntry.setDeviceSpec(deviceSpec);
        entries.put(NOISE_REDUCTION, noiseReductionEntry);

        IFeatureEntry EisPhoneEntry = new EISEntry(context, context.getResources());
        EisPhoneEntry.setDeviceSpec(deviceSpec);
        entries.put(EIS, EisPhoneEntry);

        IFeatureEntry Fps60PhoneEntry = new Fps60Entry(context, context.getResources());
        Fps60PhoneEntry.setDeviceSpec(deviceSpec);
        entries.put(FPS60, Fps60PhoneEntry);

        IFeatureEntry aisEntry = new AISEntry(context, context.getResources());
        aisEntry.setDeviceSpec(deviceSpec);
        entries.put(AIS, aisEntry);

        IFeatureEntry sceneModeEntry = new SceneModeEntry(context, context.getResources());
        sceneModeEntry.setDeviceSpec(deviceSpec);
        entries.put(SCENE_MODE, sceneModeEntry);

        IFeatureEntry whiteBalanceEntry = new WhiteBalanceEntry(context, context.getResources());
        whiteBalanceEntry.setDeviceSpec(deviceSpec);
        entries.put(WHITE_BALANCE, whiteBalanceEntry);

        IFeatureEntry antiFlickerEntry = new AntiFlickerEntry(context, context.getResources());
        antiFlickerEntry.setDeviceSpec(deviceSpec);
        entries.put(ANTI_FLICKER, antiFlickerEntry);

        IFeatureEntry zsdEntry = new ZSDEntry(context, context.getResources());
        zsdEntry.setDeviceSpec(deviceSpec);
        entries.put(ZSD, zsdEntry);

        IFeatureEntry isoEntry = new ISOEntry(context, context.getResources());
        isoEntry.setDeviceSpec(deviceSpec);
        entries.put(ISO, isoEntry);

        IFeatureEntry aeAfDebugEntry = new AaaRoiDebugEntry(context, context.getResources());
        aeAfDebugEntry.setDeviceSpec(deviceSpec);
        entries.put(AE_AF_DEBUG, aeAfDebugEntry);

        IFeatureEntry sDofPhotoEntry = new SdofPhotoEntry(context, context.getResources());
        sDofPhotoEntry.setDeviceSpec(deviceSpec);
        entries.put(SDOF_PHOTO_MODE, sDofPhotoEntry);

        IFeatureEntry shutterSpeedEntry = new ShutterSpeedEntry(context, context.getResources());
        shutterSpeedEntry.setDeviceSpec(deviceSpec);
        entries.put(SHUTTER_SPEED, shutterSpeedEntry);

        IFeatureEntry longExposureEntry = new LongExposureModeEntry(context,
                context.getResources());
        longExposureEntry.setDeviceSpec(deviceSpec);
        entries.put(LONG_EXPUSURE_MODE, longExposureEntry);

        IFeatureEntry photoEntry = new PhotoModeEntry(context, context.getResources());
        photoEntry.setDeviceSpec(deviceSpec);
        entries.put(PHOTO_MODE, photoEntry);

        IFeatureEntry videoEntry = new VideoModeEntry(context, context.getResources());
        videoEntry.setDeviceSpec(deviceSpec);
        entries.put(VIDEO_MODE, videoEntry);

        IFeatureEntry intentVideoEntry = new IntentVideoModeEntry(context, context.getResources());
        intentVideoEntry.setDeviceSpec(deviceSpec);
        entries.put(INTENT_VIDEO_MODE, intentVideoEntry);

        IFeatureEntry intentPhotoEntry = new IntentPhotoModeEntry(context, context.getResources());
        intentPhotoEntry.setDeviceSpec(deviceSpec);
        entries.put(INTENT_PHOTO_MODE, intentPhotoEntry);

        IFeatureEntry slowMotionEntry = new SlowMotionEntry(context, context.getResources());
        slowMotionEntry.setDeviceSpec(deviceSpec);
        entries.put(SLOW_MOTION_MODE, slowMotionEntry);

        IFeatureEntry formatsEntry = new FormatEntry(context, context.getResources());
        formatsEntry.setDeviceSpec(deviceSpec);
        entries.put(FORMATS, formatsEntry);

        IFeatureEntry slowMotionQualityEntry = new SlowMotionQualityEntry(context,
                context.getResources());
        slowMotionQualityEntry.setDeviceSpec(deviceSpec);
        entries.put(SLOW_MOTION_QUALITY, slowMotionQualityEntry);

        IFeatureEntry matrixModeEntry = new MatrixModeEntry(context,
                context.getResources());
        matrixModeEntry.setDeviceSpec(deviceSpec);
        entries.put(MATRIX_MODE, matrixModeEntry);

//        IFeatureEntry matrixDisplayEntry = new MatrixDisplayEntry(context,
//                context.getResources());
//        matrixDisplayEntry.setDeviceSpec(deviceSpec);
//        entries.put(MATRIX_SETTING, matrixDisplayEntry);

        IFeatureEntry dofEntry = new DofModeEntry(context,
                context.getResources());
        dofEntry.setDeviceSpec(deviceSpec);
        entries.put(DOF_MODE, dofEntry);

        IFeatureEntry fbEntry = new FaceBeautyModeEntry(context,
                context.getResources());
        fbEntry.setDeviceSpec(deviceSpec);
        entries.put(FB_MODE, fbEntry);

        IFeatureEntry vfbEntry = new VendorFaceBeautyModeEntry(context,
                context.getResources());
        vfbEntry.setDeviceSpec(deviceSpec);
        entries.put(VFB_MODE, vfbEntry);

        return entries;
    }
}