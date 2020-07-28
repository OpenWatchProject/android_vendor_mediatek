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
package com.mediatek.camera.v2.setting;


import com.mediatek.camera.v2.util.SettingKeys;

import java.util.HashMap;
import java.util.Map;

public class SettingConvertor {
    public interface EnumMode {
        public int getValue();
        public String getName();
    }

    public enum FaceDetectMode implements EnumMode {
        OFF(0),
        ON(1);

        private int value = 0;
        private FaceDetectMode(int value) {
            this.value = value;
        }

        public int getValue() {
            return this.value;
        }

        public String getName() {
            return this.toString();
        }
    }

    public enum SceneMode implements EnumMode {
        AUTO(0),
        FACE_PORTRAIT(1),
        ACTION(2),
        PORTRAIT(3),
        LANDSCAPE(4),
        NIGHT(5),
        NIGHT_PORTRAIT(6),
        THEATRE(7),
        BEACH(8),
        SNOW(9),
        SUNSET(10),
        STEADYPHOTO(11),
        FIREWORKS(12),
        SPORTS(13),
        PARTY(14),
        CANDLELIGHT(15),
        BARCODE(16),
        HIGH_SPEED_VIDEO(17),
        HDR(18),
        BACKLIGHT_PORTRAIT(32);

        private int value = 0;
        private SceneMode(int value) {
            this.value = value;
        }

        public int getValue() {
            return this.value;
        }

        public String getName() {
            return this.toString();
        }
    }

    public enum AWBMode implements EnumMode {
        OFF(0),
        AUTO(1),
        INCANDESCENT(2),
        FLUORESCENT(3),
        WARM_FLUORESCENT(4),
        DAYLIGHT(5),
        CLOUDY_DAYLIGHT(6),
        TWILIGHT(7),
        SHADE(8);

        private int value = 0;
        private AWBMode(int value) {
            this.value = value;
        }

        public int getValue() {
            return this.value;
        }

        public String getName() {
            return this.toString();
        }

    }

    public enum EffectMode implements EnumMode {
        NONE(0),
        MONO(1),
        NEGATIVE(2),
        SOLARIZE(3),
        SEPIA(4),
        POSTERIZE(5),
        WHITEBOARD(6),
        BLACKBOARD(7),
        AQUA(8);

        private int value = 0;
        private EffectMode(int value) {
            this.value = value;
        }

        public int getValue() {
            return this.value;
        }

        public String getName() {
            return this.toString();
        }
    }

    public enum EISMode implements EnumMode {
        OFF(0),
        ON(1);

        private int value = 0;
        private EISMode(int value) {
            this.value = value;
        }

        public int getValue() {
            return this.value;
        }

        public String getName() {
            return this.toString();
        }
    }

    public enum AnitbandingMode implements EnumMode {
        OFF(0),
        HZ_50(1),
        HZ_60(2),
        AUTO(3);

        private int value = 0;
        private AnitbandingMode(int value) {
            this.value = value;
        }

        public int getValue() {
            return this.value;
        }

        public String getName() {
            return this.toString();
        }
    }

    public enum ASDDetectMode implements EnumMode {
        OFF(0),
        ON(1);

        private int value = 0;
        private ASDDetectMode(int value) {
            this.value = value;
        }

        public int getValue() {
            return this.value;
        }

        public String getName() {
            return this.toString();
        }
    }

    public enum NoiceReductionMode implements EnumMode {
        OFF(0),
        ON(1);

        private int value = 0;
        private NoiceReductionMode(int value) {
            this.value = value;
        }

        public int getValue() {
            return this.value;
        }

        public String getName() {
            return this.toString();
        }
    }

    private static Map<String, Class<? extends EnumMode>> mEnumClasses
            = new HashMap<String, Class<? extends EnumMode>>();
    static {
        mEnumClasses.put(SettingKeys.KEY_CAMERA_FACE_DETECT, FaceDetectMode.class);
        mEnumClasses.put(SettingKeys.KEY_SCENE_MODE,         SceneMode.class);
        mEnumClasses.put(SettingKeys.KEY_WHITE_BALANCE,      AWBMode.class);
        mEnumClasses.put(SettingKeys.KEY_COLOR_EFFECT,       EffectMode.class);
        mEnumClasses.put(SettingKeys.KEY_ANTI_BANDING,       AnitbandingMode.class);
        mEnumClasses.put(SettingKeys.KEY_VIDEO_EIS,          EISMode.class);
        mEnumClasses.put(SettingKeys.KEY_ASD,                ASDDetectMode.class);
        mEnumClasses.put(SettingKeys.KEY_VIDEO_3DNR,         NoiceReductionMode.class);
    }

    public static String convertModeEnumToString(String key, int enumIndex) {
        Class<? extends EnumMode> enumMode = mEnumClasses.get(key);
        String name = null;
        if (enumMode != null) {
            EnumMode[] modes = enumMode.getEnumConstants();
            for (EnumMode mode : modes) {
                if (mode.getValue() == enumIndex) {
                    name = mode.getName().replace('_', '-').toLowerCase();
                    break;
                }
            }
        }
        return name;
    }

    public static String[] convertModeEnumToString(String key, int[] enumIndexs) {
         Class<? extends EnumMode> enumMode = mEnumClasses.get(key);
         if (enumMode != null) {
             EnumMode[] modes = enumMode.getEnumConstants();
             String[] names = new String[enumIndexs.length];
             for (int i = 0; i < enumIndexs.length; i++) {
                 int enumIndex = enumIndexs[i];
                 for (EnumMode mode : modes) {
                     if (mode.getValue() == enumIndex) {
                         names[i] = mode.getName().replace('_', '-').toLowerCase();
                         break;
                     }
                 }
             }
             return names;
         }
         return new String[0];
    }

    public static int convertStringToEnum(String key, String value) {
        int enumIndex = 0;
        Class<? extends EnumMode> enumMode = mEnumClasses.get(key);
        if (enumMode != null) {
            EnumMode[] modes = enumMode.getEnumConstants();
            for (EnumMode mode : modes) {
                String modeName = mode.getName().replace('_', '-').toLowerCase();
                if (modeName.equalsIgnoreCase(value)) {
                    enumIndex = mode.getValue();
                }
            }
        } else {
            enumIndex = Integer.parseInt(value);
        }
        return enumIndex;
    }
}
