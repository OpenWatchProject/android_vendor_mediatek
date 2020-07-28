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
package com.mediatek.camera.setting;

import android.app.Activity;
import android.content.Context;
import android.content.SharedPreferences;
import android.content.SharedPreferences.Editor;
import android.graphics.Point;
import android.hardware.Camera.Size;
import android.media.CamcorderProfile;
import android.text.TextUtils;
import android.view.Display;
import android.view.View;
import android.view.WindowManager;
//TODO:CamcorderProfileEx
import com.mediatek.camera.ISettingCtrl;
import com.mediatek.camera.platform.Parameters;
import com.mediatek.camera.setting.preference.ListPreference;
import com.mediatek.camera.setting.preference.RecordLocationPreference;
import com.mediatek.camera.util.Log;

import java.text.DecimalFormat;
import java.text.DecimalFormatSymbols;
import java.util.ArrayList;
import java.util.Collections;
import java.util.List;
import java.util.Locale;

public class SettingUtils {
    private static final String TAG = "SettingUtils";

    public static final String PICTURE_RATIO_16_9                   = "1.7778";
    public static final String PICTURE_RATIO_5_3                    = "1.6667";
    public static final String PICTURE_RATIO_3_2                    = "1.5";
    public static final String PICTURE_RATIO_4_3                    = "1.3333";
    public static final String STEREO3D_ENABLE                      = "1";
    public static final String STEREO3D_DISABLE                     = "0";
    public static final String RESET_STATE_VALUE_DISABLE            = "disable-value";
    public static final String KEY_INTO_VIDEO_FACE_BEAUTY_NORMAL    = "face-beauty-normal";
    public static final String STEREO_CAMERA_ON                     = "on";
    public static final String STEREO_CAMERA_OFF                    = "off";

    public static final int VGA_SIZE = 640 * 480;
    public static final int QVGA_SIZE = 320 * 240;
    private static final String ENABLE_LIST_HEAD             = "[L];";
    private static final String ENABLE_LIST_SEPERATOR        = ";";


    private static final float ALPHA_ENABLE = 1.0F;
    private static final float ALPHA_DISABLE = 0.3F;
    private static final double DEFAULT_RATIO = 16d / 9;
    public static final double ASPECT_TOLERANCE = 0.02;
    // TODO
    public static final boolean SUPPORTED_SHOW_CONINUOUS_SHOT_NUMBER = true;
    public static final double[] RATIOS = new double[] { 1.3333, 1.5, 1.6667, 1.7778 };

    // public static final boolean SUPPORTED_SHOW_CONINUOUS_SHOT_NUMBER =
    // !FeatureSwitcher.isLcaRAM();

    public static final MappingFinder MAPPING_FINDER_PICTURE_SIZE = new PictureSizeMappingFinder();
    public static final MappingFinder MAPPING_FINDER_FLASH = new FlashMappingFinder();
    public static final MappingFinder MAPPING_FINDER_VIDEO_QUALITY =
            new VideoQualityMappingFinder();

    private static final int NOT_FOUND = -1;
    private static final int UNKNOWN = -1;
    public static final int CURRENT_VERSION = 5;
    public static final int CURRENT_LOCAL_VERSION = 2;
    public static final int INTO_VIDEO_FACE_BEAUTY_NORMAL           = 1;
    public static final int INTO_VIDEO_FACE_BEAUTY_NON_NORMAL       = 0;

    private static final DecimalFormat DECIMAL_FORMATOR = new DecimalFormat("######.####",
            new DecimalFormatSymbols(Locale.ENGLISH));

    private static double mCurrentFullScreenRatio = DEFAULT_RATIO;
    private static int mFullScreenWidth = 0;
    private static int mFullScreenHeight = 0;
    private static int mLimitResolution;
    private static String sZsdDefaultValue = null;
    private static String sAntiBandingDefaultValue = null;

    private SettingUtils() {
    }

    // add for pip, pip should restore top camera's(preview size, picture
    // size,zsd, anti flicker)
    public static void setPreferenceValue(Context context, SharedPreferences sharedPreferences,
            int settingId, String value) {
        String key = SettingConstants.getSettingKey(settingId);
        SharedPreferences.Editor editor = sharedPreferences.edit();
        editor.putString(key, value);
        editor.apply();
    }

    public static String getPreferenceValue(Context context, SharedPreferences sharedPreferences,
            int settingId, String defaultValue) {
        String key = SettingConstants.getSettingKey(settingId);
        String value = sharedPreferences.getString(key, defaultValue);
        if ((settingId == SettingConstants.ROW_SETTING_PICTURE_RATIO) && (value == null)) {
            value = PICTURE_RATIO_4_3;
        }
//        Log.d(TAG, "getPreferenceValue(" + settingId + ") return " + value);
        return value;
    }

    public static void setPictureSize(Context context, Parameters parameters,
            SharedPreferences sharedPreferences, String pictureSize,
                                      SettingItem previewRatioSetting) {
        if (pictureSize == null) {
            initialCameraPictureSize(context, parameters, sharedPreferences);
        } else {
            List<Size> supported = parameters.getSupportedPictureSizes();
            String defaultRatio = getDefaultValueFromXml(context, previewRatioSetting);
            String targetRatio = getPreferenceValue(context, sharedPreferences,
                    SettingConstants.ROW_SETTING_PICTURE_RATIO, defaultRatio);
            setCameraPictureSize(pictureSize, supported, parameters, targetRatio,
                    context);
        }
    }

    public static void setPreviewSize(Context context, Parameters parameters, String prviewRatio) {
        double previewRatio = Double.parseDouble(prviewRatio);
        List<Size> sizes = parameters.getSupportedPreviewSizes();
        Size optimalSize = getOptimalPreviewSize(context, sizes, previewRatio,
                ParametersHelper.isDisplayRotateSupported(parameters));

        Size original = parameters.getPreviewSize();
        if (!original.equals(optimalSize)) {
            parameters.setPreviewSize(optimalSize.width, optimalSize.height);
        }
        setPreviewFrameRate(context, parameters, -1);
    }

    public static void setPipPreviewSize(Activity activity, Parameters parameters,
                        Parameters topParameters, ISettingCtrl iSettingCtrl, String value) {
        double previewRatio = Double.parseDouble(
                iSettingCtrl.getSettingValue(SettingConstants.KEY_PICTURE_RATIO));
        List<Size> bottomSizes = parameters.getSupportedPreviewSizes();
        List<Size> topSizes = topParameters.getSupportedPreviewSizes();

        Size bottomOptimalSize = getOptimalPreviewSize(activity,
                filterByBound(bottomSizes, 1920, 1088),
                previewRatio, ParametersHelper.isDisplayRotateSupported(parameters));
        Size topOptimalSize = getOptimalPreviewSize(activity,
                filterByBound(bottomSizes, 1920, 1088),
                previewRatio, ParametersHelper.isDisplayRotateSupported(parameters));
        Size selectedSize = (bottomOptimalSize.width > topOptimalSize.width) ?
                topOptimalSize : bottomOptimalSize;
        parameters.setPreviewSize(selectedSize.width, selectedSize.height);
        topParameters.setPreviewSize(selectedSize.width, selectedSize.height);

        // Set Preview Frame Rate
        setPipPreviewFrameRate(iSettingCtrl, parameters, topParameters);
    }

    public static Size getOptimalPreviewSize(Context context, Parameters parameters, String ratio) {
        double previewRatio = Double.parseDouble(ratio);
        List<Size> sizes = parameters.getSupportedPreviewSizes();
        Size optimalSize = getOptimalPreviewSize(context, sizes, previewRatio,
                ParametersHelper.isDisplayRotateSupported(parameters));
        return optimalSize;
    }

    // TODO why use this name ?
    public static String getDefaultValueFromXml(Context context, SettingItem settingItem) {
        int settingId = settingItem.getSettingId();
        String value = SettingDataBase.getDefaultValue(settingId);
        if (value == null) {
            ListPreference pref = settingItem.getListPreference();
            if (!SUPPORTED_SHOW_CONINUOUS_SHOT_NUMBER
                    && settingId == SettingConstants.ROW_SETTING_CONTINUOUS_NUM) {
                value = SettingConstants.DEFAULT_CONINUOUS_CAPTURE_NUM;
            } else {
                if (pref != null) {
                    value = pref.findSupportedDefaultValue();
                    if (value == null && pref.getEntryValues() != null
                            && pref.getEntryValues().length > 0) {
                        value = String.valueOf(pref.getEntryValues()[0]);
                    }
                }
                // last defense for some preference may be removed for supported
                // values less than 2.
                if (value == null) {
                    int resId = SettingDataBase.getDefaultSettingID(settingId);
                    if (resId != UNKNOWN) {
                        value = context.getString(resId);
                    }
                }
            }
            SettingDataBase.setDefaultValue(settingId, value);
        }
//        Log.d(TAG, "getDefaultValueFromXml(" + settingId + ") return " + value);
        return value;
    }

    public static boolean setCameraPictureSize(String candidate, List<Size> supported,
            Parameters parameters, String targetRatio, Context context) {
        int index = candidate.indexOf('x');
        if (index == NOT_FOUND) {
            return false;
        }
        List<String> supportedRatioSizes = buildSupportedPictureSizeByRatio(parameters,
                targetRatio);
        candidate = MAPPING_FINDER_PICTURE_SIZE.find(candidate, supportedRatioSizes);
        index = candidate == null ? NOT_FOUND : candidate.indexOf('x');
        if (index == NOT_FOUND) {
            return false;
        }
        int width = Integer.parseInt(candidate.substring(0, index));
        int height = Integer.parseInt(candidate.substring(index + 1));
        parameters.setPictureSize(width, height);
        return true;
    }

    public static boolean isSupported(Object value, List<?> supported) {
        return supported == null ? false : supported.indexOf(value) >= 0;
    }

    public static void initialCameraPictureSize(Context context, Parameters parameters,
            SharedPreferences sharedPreferences) {
        // / M: here we find the full screen picture size for default, not first
        // one in arrays.xml
        List<String> supportedRatios = buildPreviewRatios(context, parameters);
        String ratioString = null;
        if (supportedRatios != null && supportedRatios.size() > 0) {
            SharedPreferences.Editor editor = sharedPreferences.edit();
            ratioString = supportedRatios.get(supportedRatios.size() - 1);
            editor.putString(SettingConstants.KEY_PICTURE_RATIO, ratioString);
            editor.apply();
        }
        List<String> supportedSizes = buildSupportedPictureSizeByRatio(parameters, ratioString);
        if (supportedSizes != null && supportedSizes.size() > 0) {
            String findPictureSize = supportedSizes.get(supportedSizes.size() - 1);
            SharedPreferences.Editor editor = sharedPreferences.edit();
            editor.putString(SettingConstants.KEY_PICTURE_SIZE, findPictureSize);
            editor.apply();
            Point ps = SettingUtils.getSize(findPictureSize);
            parameters.setPictureSize(ps.x, ps.y);
        }
    }

    /**
     * Sort the supported picture sizes in ascending order.
     */
    public static void sortSizesInAscending(List<String> supportedPictureSizes) {
        String maxSizeInString = null;
        Point maxSize = null;
        int maxIndex = 0;
        // Loop (n - 1) times.
        for (int i = 0; i < supportedPictureSizes.size() - 1; i++) {
             maxSizeInString = supportedPictureSizes.get(0);
             maxSize = SettingUtils.getSize(maxSizeInString);
             maxIndex = 0;
             String tempSizeInString = null;
             Point tempSize = null;
             // Find the max size.
             for (int j = 0; j < supportedPictureSizes.size() - i; j++) {
                  tempSizeInString = supportedPictureSizes.get(j);
                  tempSize = SettingUtils.getSize(tempSizeInString);
                  if (tempSize.x * tempSize.y > maxSize.x * maxSize.y) {
                      maxSizeInString = tempSizeInString;
                      maxSize = tempSize;
                      maxIndex = j;
                  }
             }
             // Place the max size to the end position.
             supportedPictureSizes.set(maxIndex, tempSizeInString);
             supportedPictureSizes.set(supportedPictureSizes.size() - 1 - i, maxSizeInString);
        }
    }

    /**
     * Get the supported picture sizes that its ratio is equal with the given ratio.
     * @param parameters The camera parameters.
     * @param targetRatio The ratio used to filter picture sizes.
     * @return Return the appropriate picture sizes.
     */
    public static List<String> buildSupportedPictureSizeByRatio(Parameters parameters,
            String targetRatio) {
        ArrayList<String> list = new ArrayList<String>();
        double ratio = 0;
        if (targetRatio == null) {
            return list;
        }
        try {
            ratio = Double.parseDouble(targetRatio);
        } catch (NumberFormatException e) {
            Log.w(TAG, "buildSupportedPictureSize() bad ratio: " + targetRatio, e);
            return list;
        }
        List<Size> sizes = parameters.getSupportedPictureSizes();
        if (sizes != null) {
            for (Size size : sizes) {
                if (toleranceRatio(ratio, (double) size.width / size.height)) {
                    list.add(buildSize(size.width, size.height));
                }
            }
        }
//        Log.d(TAG, "buildSupportedPictureSize, ratio:" + targetRatio + "," +
//                " sizes:" + list.toString());
        return list;
    }

    public static List<String> buildPreviewRatios(Context context, Parameters parameters) {
        List<String> supportedRatios = new ArrayList<String>();
        String findString = null;
        if (context != null && parameters != null) {
            // Add standard preview ratio.
            supportedRatios.add(getRatioString(4d / 3));

            mCurrentFullScreenRatio = findFullscreenRatio(context);
            List<String> fullScreenPictureSizes = buildSupportedPictureSizeByRatio(parameters,
                    getRatioString(mCurrentFullScreenRatio));
            // Add full screen ratio if platform has full screen ratio picture sizes.
            if (fullScreenPictureSizes.size() > 0) {
                findString = getRatioString(mCurrentFullScreenRatio);
                if (!supportedRatios.contains(findString)) {
                    supportedRatios.add(findString);
                }
            }
        }
//        Log.d(TAG, "buildPreviewRatios(" + parameters + ") add supportedRatio " + findString);
        return supportedRatios;
    }

    public static double getFullScreenRatio() {
        return mCurrentFullScreenRatio;
    }

    public static int getFullScreenWidth() {
        return mFullScreenWidth;
    }

    public static int getFullScreenHeight() {
        return mFullScreenHeight;
    }

    public static void setLimitResolution(final int resolution) {
//        Log.d(TAG, "[setLimitResolution], resolution:" + resolution);
        mLimitResolution = resolution;
    }

    public static int getLimitResolution() {
        return mLimitResolution;
    }

    /**
     * Save the ZSD default value which is configured in parameters.
     * @param zsdDefaultValue The value configured in parameters.
     */
    public static void setZsdDefaultValue(String zsdDefaultValue) {
        sZsdDefaultValue = zsdDefaultValue;
    }

    /**
     * Get ZSD default value which is configured in parameters.
     * @return The ZSD default value.
     */
    public static String getZsdDefaultValue() {
        return sZsdDefaultValue;
    }

    /**
     * Save the anti flicker default value which is configured in parameters.
     * @param antiBandingDefaultValue The value configured in parameters.
     */
    public static void setAntiBandingDefaultValue(String antiBandingDefaultValue) {
        sAntiBandingDefaultValue = antiBandingDefaultValue;
    }

    /**
     * Get anti flicker default value which is configured in parameters.
     * @return The anti flicker default value.
     */
    public static String getAntiBandingDefaultValue() {
        return sAntiBandingDefaultValue;
    }

    public static void filterLimitResolution(List<String> pictureSizes) {
        if (mLimitResolution > 0) {
            for (int i = 0; i < pictureSizes.size(); i++) {
                String size = pictureSizes.get(i);
                int index = size.indexOf('x');
                int width = Integer.parseInt(size.substring(0, index));
                int height = Integer.parseInt(size.substring(index + 1));
                if (width * height > mLimitResolution) {
                    pictureSizes.remove(i);
                    i--;
                }
            }
        }
    }

    /**
     * Calculate device screen ratio.
     * @param context The activity context.
     * @return Return the device screen ratio.
     */
    public static double findFullscreenRatio(Context context) {
        double find = 4d / 3;
        WindowManager wm = (WindowManager) context.getSystemService(Context.WINDOW_SERVICE);
        Display display = wm.getDefaultDisplay();
        Point point = new Point();
        display.getRealSize(point);
        mFullScreenWidth = point.x;
        mFullScreenHeight = point.y;

        double fullscreen;
        if (point.x > point.y) {
            fullscreen = (double) point.x / point.y;
        } else {
            fullscreen = (double) point.y / point.x;
        }
        for (int i = 0; i < RATIOS.length; i++) {
            if (Math.abs(RATIOS[i] - fullscreen) < Math.abs(fullscreen - find)) {
                find = RATIOS[i];
            }
        }
//        Log.d(TAG, "findFullscreenRatiom, return ratio:" + find);
        return find;
    }

    public static void setPreviewFrameRate(Context context, Parameters parameters,
            int supportedFramerate) {
        List<Integer> frameRates = null;
        if (supportedFramerate > 0) {
            frameRates = new ArrayList<Integer>();
            frameRates.add(supportedFramerate);
        }

        if (frameRates == null) {
            // Reset preview frame rate to the maximum because it may be lowered
            // by
            // video camera application.
            frameRates = parameters.getSupportedPreviewFrameRates();
        }

        if (frameRates != null) {
            Integer max = Collections.max(frameRates);
            parameters.setPreviewFrameRate(max);
//            Log.d(TAG, "setPreviewFrameRate max = " + max + " frameRates = " + frameRates);
        }
    }

    private static Size getOptimalPreviewSize(Context context,
                              List<Size> sizes, double targetRatio, boolean needMatchPanelSize) {
        // Use a very small tolerance because we want an exact match.
        // final double EXACTLY_EQUAL = 0.001;
        if (sizes == null) {
            return null;
        }

        Size optimalSize = null;
        double minDiff = Double.MAX_VALUE;
        double minDiffWidth = Double.MAX_VALUE;

        // Because of bugs of overlay and layout, we sometimes will try to
        // layout the viewfinder in the portrait orientation and thus get the
        // wrong size of preview surface. When we change the preview size, the
        // new overlay will be created before the old one closed, which causes
        // an exception. For now, just get the screen size.
        WindowManager wm = (WindowManager) context.getSystemService(Context.WINDOW_SERVICE);
        Display display = wm.getDefaultDisplay();
        Point point = new Point();
        display.getRealSize(point);
        int panelHeight = Math.min(point.x, point.y);
        int panelWidth = Math.max(point.x, point.y);

        Size bestMatchPanelSize = needMatchPanelSize ?
                findBestMatchPanelSize(context, sizes, targetRatio, panelWidth, panelHeight) : null;
        if (bestMatchPanelSize != null) {
            return bestMatchPanelSize;
        }

        // Try to find an size match aspect ratio and size
        for (Size size : sizes) {
            double ratio = (double) size.width / size.height;
            if (Math.abs(ratio - targetRatio) > ASPECT_TOLERANCE) {
                continue;
            }
            if (Math.abs(size.height - panelHeight) < minDiff) {
                optimalSize = size;
                minDiff = Math.abs(size.height - panelHeight);
                minDiffWidth = Math.abs(size.width - panelWidth);
            } else if ((Math.abs(size.height - panelHeight) == minDiff)
                    && Math.abs(size.width - panelWidth) < minDiffWidth) {
                optimalSize = size;
                minDiffWidth = Math.abs(size.width - panelWidth);
            }
        }
        // Cannot find the one match the aspect ratio. This should not happen.
        // Ignore the requirement.
        // M: This will happen when native return video size and wallpaper
        // want to get specified ratio.
        if (optimalSize == null) {
            Log.w(TAG, "No preview size match the aspect ratio" + targetRatio + ","
                    + "then use the standard(4:3) preview size");
            minDiff = Double.MAX_VALUE;
            targetRatio = Double.parseDouble(PICTURE_RATIO_4_3);
            for (Size size : sizes) {
                double ratio = (double) size.width / size.height;
                if (Math.abs(ratio - targetRatio) > ASPECT_TOLERANCE) {
                    continue;
                }
                if (Math.abs(size.height - panelHeight) < minDiff) {
                    optimalSize = size;
                    minDiff = Math.abs(size.height - panelHeight);
                }
            }
        }
        return optimalSize;
    }

    public static String getRatioString(double ratio) {
        return DECIMAL_FORMATOR.format(ratio);
    }

    public static Point getSize(String sizeString) {
        Point size = new Point();
        int index = sizeString.indexOf('x');
        if (index != UNKNOWN) {
            size.x = Integer.parseInt(sizeString.substring(0, index));
            size.y = Integer.parseInt(sizeString.substring(index + 1));
        }
        return size;
    }

    public static String buildSize(Size size) {
        if (size != null) {
            return "" + size.width + "x" + size.height;
        } else {
            return "null";
        }
    }

    public static String buildSize(int width, int height) {
        return "" + width + "x" + height;
    }

    public static List<Point> splitSize(String str) {
        if (str == null) return null;

        TextUtils.StringSplitter splitter = new TextUtils.SimpleStringSplitter(',');
        splitter.setString(str);
        ArrayList<Point> sizeList = new ArrayList<Point>();
        for (String s : splitter) {
            Point size = strToSize(s);
            if (size != null) sizeList.add(size);
        }
        if (sizeList.size() == 0) return null;
        return sizeList;
    }

    public static Point strToSize(String str) {
        if (str == null) return null;
        int pos = str.indexOf('x');
        if (pos != -1) {
            String width = str.substring(0, pos);
            String height = str.substring(pos + 1);
            return new Point(Integer.parseInt(width), Integer.parseInt(height));
         }
         Log.e(TAG, "Invalid size parameter string=" + str);
         return null;
    }

    public static String pointToStr(Point size) {
        if (size != null) {
            return "" + size.x + "x" + size.y;
        } else {
            return "null";
        }
    }

    public static boolean equals(Object a, Object b) {
        return (a == b) || (a == null ? false : a.equals(b));
    }

    public interface MappingFinder {
        public String find(String current, List<String> supportedList);

        public int findIndex(String current, List<String> supportedList);
    }

    public static class VideoQualityMappingFinder implements MappingFinder {
        @Override
        public String find(String current, List<String> supportedList) {
            String supported = current;
            if (supportedList != null && !supportedList.contains(current)) {
                if (Integer.toString(CamcorderProfile.QUALITY_1080P).equals(current)) {
                 // Normal fine quality to high in PIP mode.
                    supported = Integer.toString(CamcorderProfile.QUALITY_720P);
                }
            }
            if (supportedList != null && !supportedList.contains(supported)) {
                supported = supportedList.get(0);
            }
            return supported;
        }

        @Override
        public int findIndex(String current, List<String> supportedList) {
            int index = UNKNOWN;
            if (supportedList != null) {
                String supported = find(current, supportedList);
                index = supportedList.indexOf(supported);
            }
//            Log.d(TAG, "VideoQualityMappingFinder [findIndex](" + current + ", " + supportedList
//                    + ") return " + index);
            return index;
        }
    }

    public static class FlashMappingFinder implements MappingFinder {
        @Override
        public String find(String current, List<String> supportedList) {
            String supported = current;
            if (supportedList != null && !supportedList.contains(current)) {
                if (Parameters.FLASH_MODE_ON.equals(current)) {
                    // if cannot find on, it video mode, match torch.
                    supported = Parameters.FLASH_MODE_TORCH;
                } else if (Parameters.FLASH_MODE_TORCH.equals(current)) {
                    // if cannot find torch, it video mode, match on.
                    supported = Parameters.FLASH_MODE_ON;
                }
            }
            if (supportedList != null && !supportedList.contains(supported)) {
                supported = supportedList.get(0);
            }
//            Log.d(TAG, "find(" + current + ") return " + supported);
            return supported;
        }

        @Override
        public int findIndex(String current, List<String> supportedList) {
            int index = UNKNOWN;
            if (supportedList != null) {
                String supported = find(current, supportedList);
                index = supportedList.indexOf(supported);
            }
//            Log.d(TAG, "findIndex(" + current + ", " + supportedList + ") return " + index);
            return index;
        }
    }

    public static class PictureSizeMappingFinder implements MappingFinder {
        @Override
        public String find(String current, List<String> supportedList) {
            String supported = current;
            int index = current.indexOf('x');
            if (index != UNKNOWN && supportedList != null && !supportedList.contains(current)) {
                // find other appropriate size
                int size = supportedList.size();
                Point findPs = SettingUtils.getSize(supportedList.get(size - 1));
                Point candidatePs = SettingUtils.getSize(current);
                for (int i = size - 2; i >= 0; i--) {
                    Point ps = SettingUtils.getSize(supportedList.get(i));
                    if (ps != null
                          && Math.abs(ps.x - candidatePs.x) < Math.abs(findPs.x - candidatePs.x)) {
                        findPs = ps;
                    }
                }
                supported = SettingUtils.buildSize(findPs.x, findPs.y);
            }
            if (supportedList != null && !supportedList.contains(supported)) {
                supported = supportedList.get(0);
            }
//            Log.d(TAG, "find(" + current + ") return " + supported);
            return supported;
        }

        @Override
        public int findIndex(String current, List<String> supportedList) {
            int index = UNKNOWN;
            if (supportedList != null) {
                String supported = find(current, supportedList);
                index = supportedList.indexOf(supported);
            }
//            Log.d(TAG, "findIndex(" + current + ", " + supportedList + ") return " + index);
            return index;
        }
    }

    /**
     * Clear settings' values in the shared preferences.
     * @param sharedPreferences The instance of shared preferences.
     * @param isNonePickIntent Whether camera is launched from launcher or not. If camera is
     *     launched from launcher, it should be true, if camera is launched from third party
     *     it should be false.
     */
    public static void restorePreferences(SharedPreferences sharedPreferences,
            boolean isNonePickIntent) {
        Editor editor = sharedPreferences.edit();
        if (!isNonePickIntent) {
            // 3rd party launch back camera should not changed hdr and asd sharepereference
            // value, because they do not access to hdr, asd.
            String[] clearKeys = new String[SettingConstants.KEYS_FOR_SETTING.length];
            for (int i = 0; i < clearKeys.length; i++) {
                clearKeys[i] = SettingConstants.KEYS_FOR_SETTING[i];
            }

            for (int i = 0; i < SettingConstants.UN_SUPPORT_BY_3RDPARTY.length; i++) {
                int settingIndex = SettingConstants.UN_SUPPORT_BY_3RDPARTY[i];
                clearKeys[settingIndex] = null;
            }

            for (int i = 0; i < SettingConstants.SUPPORT_BY_3RDPARTY_BUT_HIDDEN.length; i++) {
                int settingIndex = SettingConstants.SUPPORT_BY_3RDPARTY_BUT_HIDDEN[i];
                clearKeys[settingIndex] = null;
            }

            for (int i = 0; i < clearKeys.length; i++) {
                String clearKey = clearKeys[i];
                editor.remove(clearKey);
            }
        } else {
            editor.clear();
        }
        editor.apply();
    }

    public static void upgradeGlobalPreferences(SharedPreferences pref, int numberOfCameras) {
        upgradeOldVersion(pref);
        upgradeCameraId(pref, numberOfCameras);
    }

    public static void upgradeLocalPreferences(SharedPreferences pref) {
        int version;
        try {
            version = pref.getInt(SettingConstants.KEY_LOCAL_VERSION, 0);
        } catch (Exception ex) {
            Log.w(TAG, "[upgradeLocalPreferences]Exception = " + ex);
            version = 0;
        }
        if (version == CURRENT_LOCAL_VERSION) {
//            Log.d(TAG, "[upgradeLocalPreferences]version = " + version);
            return;
        }
        SharedPreferences.Editor editor = pref.edit();
        if (version == 1) {
            // We use numbers to represent the quality now. The quality
            // definition is identical to
            // that of CamcorderProfile.java.
            editor.remove("pref_video_quality_key");
        }
        editor.putInt(SettingConstants.KEY_LOCAL_VERSION, CURRENT_LOCAL_VERSION);
        editor.apply();
    }

    public static int readPreferredCameraId(SharedPreferences pref) {
        return Integer.parseInt(pref.getString(SettingConstants.KEY_CAMERA_ID, "0"));
    }

    public static void resetCameraId(SharedPreferences pref) {
        writePreferredCameraId(pref, 0);
    }

    public static void writePreferredCameraId(SharedPreferences pref, int cameraId) {
        Editor editor = pref.edit();
        editor.putString(SettingConstants.KEY_CAMERA_ID, Integer.toString(cameraId));
        editor.apply();
    }

    public static String readPreferredCamera3DMode(SharedPreferences pref) {
        return pref.getString(SettingConstants.KEY_STEREO3D_MODE, STEREO3D_DISABLE);
    }

    public static void writePreferredCamera3DMode(SharedPreferences pref, String camera3DMode) {
        Editor editor = pref.edit();
        editor.putString(SettingConstants.KEY_STEREO3D_MODE, camera3DMode);
        editor.apply();
    }

    public static String readPreferredStereoCamera(SharedPreferences pref) {
//        Log.d("Test", "readPreferredStereCamera " +
//              pref.getString(SettingConstants.KEY_DUAL_CAMERA_MODE, STEREO_CAMERA_OFF));
        return pref.getString(SettingConstants.KEY_DUAL_CAMERA_MODE, STEREO_CAMERA_OFF);
    }

    public static void writePreferredStereoCamera(SharedPreferences pref,
            String stereoCamera) {
//        Log.d(TAG, "writePreferredStereoCamera stereoCamera = " + stereoCamera);
        Editor editor = pref.edit();
        editor.putString(SettingConstants.KEY_DUAL_CAMERA_MODE, stereoCamera);
        editor.apply();
    }

    public static void updateSettingCaptureModePreferences(SharedPreferences pref) {
        SharedPreferences.Editor editor = pref.edit();
        editor.putString(SettingConstants.KEY_HDR, "off");
        editor.putString(SettingConstants.KEY_ASD, "off");
        editor.putString(SettingConstants.KEY_SLOW_MOTION, "off");
        editor.apply();
    }

    public static void updateFaceDetectionPreferences(SharedPreferences pref) {
        SharedPreferences.Editor editor = pref.edit();
        // the value should be "on"
        // because first launch camera the value should be on
        editor.putString(SettingConstants.KEY_CAMERA_FACE_DETECT, "on");
        // editor.putString(KEY_FB_EXTREME_BEAUTY, "true");//[Need Check,Maybe
        // not need this]
        editor.putInt(KEY_INTO_VIDEO_FACE_BEAUTY_NORMAL, INTO_VIDEO_FACE_BEAUTY_NORMAL);
        editor.apply();
    }

    public static void setEnabledState(View view, boolean enabled) {
        if (view != null) {
            float alpha = enabled ? ALPHA_ENABLE : ALPHA_DISABLE;
            view.setAlpha(alpha);
        }
    }

    public static boolean isDisableValue(String value) {
        boolean reset = false;
        if (RESET_STATE_VALUE_DISABLE.equals(value)) {
            reset = true;
        }
//        Log.d(TAG, "isResetValue(" + value + ") return " + reset);
        return reset;
    }

    public static String buildEnableList(String[] list, String current) {
        String listStr = null;
        if (list != null) {
            listStr = ENABLE_LIST_HEAD + current + ENABLE_LIST_SEPERATOR;
            List<String> uniqueList = new ArrayList<String>();
            for (int i = 0, len = list.length; i < len; i++) {
                if (uniqueList.contains(list[i])) {
                    continue;
                }
                uniqueList.add(list[i]);
                if (i == (len - 1)) {
                    listStr += list[i];
                } else {
                    listStr += (list[i] + ENABLE_LIST_SEPERATOR);
                }
            }
        }
//        Log.d(TAG, "buildEnableList(" + current + ") return " + listStr);
        return listStr;
    }

    public static boolean isBuiltList(String listString) {
        boolean isList = false;
        if (listString != null && listString.startsWith(ENABLE_LIST_HEAD)) {
            isList = true;
        }
//        Log.d(TAG, "isBuiltList(" + listString + ") return " + isList);
        return isList;
    }

    public static List<String> getEnabledList(String listString) {
        ArrayList<String> list = new ArrayList<String>();
        if (isBuiltList(listString)) {
            String[] temp = listString.split(ENABLE_LIST_SEPERATOR);
            for (int i = 2, len = temp.length; i < len; i++) {
                if (!list.contains(temp[i])) {
                    list.add(temp[i]);
                }
            }
        }
//        Log.d(TAG, "getEnabledList(" + listString + ") return " + list);
        return list;
    }

    public static String getDefaultValue(String listString) {
        String value = null;
        if (isBuiltList(listString)) {
            String[] temp = listString.split(ENABLE_LIST_SEPERATOR);
            if (temp != null && temp.length > 1) {
                value = temp[1];
            }
        }
//        Log.d(TAG, "getDefaultValue(" + listString + ") return " + value);
        return value;
    }

    public static boolean contains(int[] list, int value) {
        boolean find = false;
        if (list != null) {
            for (int i = 0, len = list.length; i < len; i++) {
                if (list[i] == value) {
                    find = true;
                    break;
                }
            }
        }
//        Log.d(TAG, "contains(" + list + ", " + value + ") return " + find);
        return find;
    }

    public static boolean contains(String[] list, String value) {
        boolean find = false;
        if (list != null && value != null) {
            for (int i = 0, len = list.length; i < len; i++) {
                if (value.equals(list[i])) {
                    find = true;
                    break;
                }
            }
        }
//        Log.d(TAG, "contains(" + list + ", " + value + ") return " + find);
        return find;
    }

    public static boolean contains(CharSequence[] list, String value) {
        boolean find = false;
        if (list != null && value != null) {
            for (int i = 0, len = list.length; i < len; i++) {
                if (value.equals(list[i])) {
                    find = true;
                    break;
                }
            }
        }
//        Log.d(TAG, "contains(" + list + ", " + value + ") return " + find);
        return find;
    }

    public static int index(String[] list, String value) {
        int findIndex = UNKNOWN;
        if (list != null && value != null) {
            for (int i = 0, len = list.length; i < len; i++) {
                if (value.equals(list[i])) {
                    findIndex = i;
                    break;
                }
            }
        }
//        Log.v(TAG, "index(" + list + ", " + value + ") return " + findIndex);
        return findIndex;
    }

    public static int index(CharSequence[] list, String value) {
        int findIndex = UNKNOWN;
        if (list != null && value != null) {
            for (int i = 0, len = list.length; i < len; i++) {
                if (value.equals(list[i])) {
                    findIndex = i;
                    break;
                }
            }
        }
//        Log.d(TAG, "index(" + list + ", " + value + ") return " + findIndex);
        return findIndex;
    }

    /**
     * Compare the ratios of two point.
     * @param target The target point.
     * @param candidate The candidate point.
     * @return Return true if gap between the ratios of two point is below
     *     {@code ASPECT_TOLERANCE}, return false if gap between the ratios
     *     of two point is above {@code ASPECT_TOLERANCE}.
     */
    public static boolean toleranceRatio(Point target, Point candidate) {
        return toleranceRatio((double) target.x / (double) target.y,
                (double) candidate.x / (double) candidate.y);
    }

    private static List<Size> filterByBound(List<Size> sizes, int boundWidth, int boundHeight) {
        List<Size> filterSizes = new ArrayList<Size>();
        for (Size size:sizes) {
            if (size.width <= boundWidth && size.height <= boundHeight) {
                filterSizes.add(size);
            }
        }
        return filterSizes;
    }

    private static void setPictureSizeToSharedPreference(Context context, int width, int height,
            SharedPreferences sharedPreferences) {
        String findPictureSize = buildSize(width, height);
        SharedPreferences.Editor editor = sharedPreferences.edit();
        editor.putString(SettingConstants.KEY_PICTURE_SIZE, findPictureSize);
        editor.apply();
    }

    private static boolean toleranceRatio(double target, double candidate) {
        boolean tolerance = true;
        if (candidate > 0) {
            tolerance = Math.abs(target - candidate) <= ASPECT_TOLERANCE;
        }
        return tolerance;
    }

    private static void upgradeOldVersion(SharedPreferences pref) {
        int version;
        try {
            version = pref.getInt(SettingConstants.KEY_VERSION, 0);
        } catch (Exception ex) {
            version = 0;
        }
        if (version == CURRENT_VERSION)
            return;

        SharedPreferences.Editor editor = pref.edit();
        if (version == 0) {
            // We won't use the preference which change in version 1.
            // So, just upgrade to version 1 directly
            version = 1;
        }
        if (version == 1) {
            // Change jpeg quality {65,75,85} to {normal,fine,superfine}
            String quality = pref.getString(SettingConstants.KEY_JPEG_QUALITY, "85");
            if (quality.equals("65")) {
                quality = "normal";
            } else if (quality.equals("75")) {
                quality = "fine";
            } else {
                quality = "superfine";
            }
            editor.putString(SettingConstants.KEY_JPEG_QUALITY, quality);
            version = 2;
        }
        if (version == 2) {
            editor.putString(SettingConstants.KEY_RECORD_LOCATION,
                    pref.getBoolean(SettingConstants.KEY_RECORD_LOCATION, false) ?
                             RecordLocationPreference.VALUE_ON
                            : RecordLocationPreference.VALUE_NONE);
            version = 3;
        }
        if (version == 3) {
            // Just use video quality to replace it and
            // ignore the current settings.
            editor.remove("pref_camera_videoquality_key");
            editor.remove("pref_camera_video_duration_key");
        }

        editor.putInt(SettingConstants.KEY_VERSION, CURRENT_VERSION);
        editor.apply();
    }

    private static void upgradeCameraId(SharedPreferences pref, int numberOfCameras) {
        // The id stored in the preference may be out of range if we are running
        // inside the emulator and a webcam is removed.
        // Note: This method accesses the global preferences directly, not the
        // combo preferences.
        int cameraId = readPreferredCameraId(pref);
        if (cameraId == 0)
            return; // fast path

        if (cameraId < 0 || cameraId >= numberOfCameras) {
            writePreferredCameraId(pref, 0);
        }
    }

    private static void setPipPreviewFrameRate(ISettingCtrl iSettingCtrl, Parameters parameters,
                                               Parameters topParameters) {
        String zsdValue = iSettingCtrl.getSettingValue(SettingConstants.KEY_CAMERA_ZSD);
        List<Integer> pipFrameRates = null;
        List<Integer> pipTopFrameRates = null;
        if ("on".equals(zsdValue)) {
            pipFrameRates = parameters.getPIPFrameRateZSDOn();
            if (topParameters != null) {
                pipTopFrameRates = topParameters.getPIPFrameRateZSDOn();
            }
//            Log.d(TAG, "getPIPFrameRateZSDOn pipFrameRates " + pipFrameRates
//                    + " pipTopFrameRates = " + pipTopFrameRates);
        } else {
            pipFrameRates = parameters.getPIPFrameRateZSDOff();
            if (topParameters != null) {
                pipTopFrameRates = topParameters.getPIPFrameRateZSDOff();
            }
//            Log.d(TAG, "getPIPFrameRateZSDOff pipFrameRates = " + pipFrameRates
//                    + " pipTopFrameRates = " + pipTopFrameRates);
        }
        // close dynamic frame rate, if dynamic frame rate is supported
        closeDynamicFrameRate(parameters);
        closeDynamicFrameRate(topParameters);

        if (pipFrameRates != null) {
            Integer backFramerate = Collections.max(pipFrameRates);
            parameters.setPreviewFrameRate(backFramerate);

        }
        if (topParameters != null) {
            Integer frontFramerate = Collections.max(pipTopFrameRates);
            topParameters.setPreviewFrameRate(frontFramerate);
        }
    }

    private static void closeDynamicFrameRate(Parameters parameters) {
        if (parameters == null) {
//            Log.d(TAG, "closeDynamicFrameRate but why parameters is null");
            return;
        }
        boolean support = parameters.isDynamicFrameRateSupported();
        if (support) {
            parameters.setDynamicFrameRate(false);
        }
//        Log.d(TAG, "closeDynamicFrameRate support = " + support);
    }

    // 1.find a preview size which aspect ratio is closest to target aspect ratio
    // 2.when panel size's aspect ratio is the same with target aspect ratio,
    // choose a preview size no less than panel size
    //  or else find a preview size closest to panel size
    private static Size findBestMatchPanelSize(Context context, List<Size> sizes,
            double targetRatio, int panelWidth, int panelHeight) {
        double minDiff = Double.MAX_VALUE;
        double minDiffWidth = Double.MAX_VALUE;
        double panelAspectRatio = (double) panelWidth / panelHeight;
        Size bestMatchSize = null;
        for (Size size : sizes) {
            double ratio = (double) size.width / size.height;
            // filter out the size which not tolerated by target ratio
            if (Math.abs(ratio - targetRatio) > ASPECT_TOLERANCE) {
                continue;
            }
            // when target aspect ratio is the same with panel size aspect ratio
            // find no less than panel size's preview size
            if (Math.abs(panelAspectRatio - targetRatio) <= ASPECT_TOLERANCE &&
                    (panelHeight > size.height || panelWidth > size.width)) {
                // filter out less than panel size
                continue;
            }
            // find the size closest to panel size
            if (Math.abs(size.height - panelHeight) < minDiff) {
                bestMatchSize = size;
                minDiff = Math.abs(size.height - panelHeight);
                minDiffWidth = Math.abs(size.width - panelWidth);
            } else if ((Math.abs(size.height - panelHeight) == minDiff)
                    && Math.abs(size.width - panelWidth) < minDiffWidth) {
                bestMatchSize = size;
                minDiffWidth = Math.abs(size.width - panelWidth);
            }
        }
//        Log.d(TAG, "findBestMatchPanelSize :" + buildSize(bestMatchSize));
        return bestMatchSize;
    }
}
